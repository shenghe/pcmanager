#pragma once
#include "vulfix/Defines.h"
#include "Utils.h"
#include "ItemFilter.h"

class DBFilterBase;
class CXmlManager;

BOOL ParseUserPatcherXml( LPCTSTR szXmlfile, T_UserPatchInfo &patchInfo );

struct T_Date
{
	T_Date()
		: nYear(0), nMonth(0), nDay(0)
	{
	}
	void Reset()
	{
		nYear = nMonth = nDay = 0;
	}
	
	void Update(INT _year, INT _month, INT _day)
	{
		if(Compare(_year, _month, _day)<0)
		{
			nYear = _year;
			nMonth = _month;
			nDay = _day;
		}
	}

	void Update(const T_Date& date)
	{
		Update(date.nYear, date.nMonth, date.nDay);
	}
	
	int Compare(INT _year, INT _month, INT _day)
	{
		if(_year>nYear 
			|| (_year==nYear&&_month>nMonth)
			|| (_year==nYear&&_month==nMonth&&_day>nDay))
		{
			return -1;
		}
		else if(_year==nYear&&_month==nMonth&&_day==nDay)
			return 0;
		else
			return 1;
	}

public:	
	INT nYear, nMonth, nDay;
};

class CDBOSLeak
{
	friend class CImplVulfix;
public:
	CDBOSLeak() : m_dwSysLang(ESYSLang_CN), m_pObserver(NULL) {} 
	virtual ~CDBOSLeak();
	
	virtual void Reset();
	BOOL Load(LPCTSTR szfilename, DBFilterBase *filter, DWORD dwFlags);
	int GetUnfixedLeakList(CSimpleArray<LPTUpdateItem> &arrList, CSimpleArray<TItemFixed*> &arrInstalled, CSimpleArray<LPTUpdateItem> &arrInvalid, CSimpleArray<TReplacedUpdate*> &arrReplacedUpdates );
	const CSimpleArray<int> &GetExpiredIds() const;
	void SetObserver(IVulfixObserver *pObserver)
	{
		m_pObserver = pObserver;
	}
	void Cancel();
	
    void GetMirrorHostName(CString& strHostName);

	T_Date m_LatestPackageDate;
	
protected:	
	BOOL _ReadExpiredKBId( CXmlManager &xml, IXMLDOMNode *lpRoot );
	bool _ReadExpiredKBIdItem( CXmlManager &xml, IXMLDOMNode *lpNode, CSimpleArray<int>& arrKBID );
	
	BOOL _ReadReplaceds( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase *filter );
	bool _ReadReplacedItem( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase * &filter );
	
	BOOL _ReadUpdates( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase *filter );
	bool _ReadUpdateItem( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase * &filter );
	
	BOOL _ReadUpdateItem(CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase *filter, TUpdateItem &updateItem );

	void _UpdatePackageLatestDate( LPCTSTR szPubDate );
	BOOL _ReadUpdateItemOptions(CXmlManager &xml, IXMLDOMNode *lpNode, int nKBId, DBFilterBase *filter, TUpdateItem &updateItem);
	BOOL _ReadNodeCondition( CXmlManager &nd, IXMLDOMNode *lpNode, TOptionCondition &condition );

	BOOL _ReadNodePrequisite( CXmlManager &xml, IXMLDOMNode *lpNode, TOptionCondition &condition );
	BOOL _ReadUpdateItemOptionProcessInfo( CXmlManager &xml, IXMLDOMNode *lpNode, DWORD dwLang, TUpdateItem &upitem );
	
	void _Notify( TVulFixEventType evt, DWORD nNum )
	{
		if(m_pObserver)
			m_pObserver->OnVulfixEvent( evt, m_nDBType, nNum, 0 );
	}
	
	BOOL _IsUpdateReplaced( int nKBId );

    BOOL _ReadMirrorHostName( CXmlManager &xml, IXMLDOMNode *lpRoot );
	
protected:
	DWORD m_dwSysLang;

	CSimpleArray<int> m_arrExpiredKBid;
	CSimpleArray<LPTUpdateItem> m_updateItems, m_invalidItems;
	CSimpleArray<TItemFixed*> m_installedItems;
	CSimpleArray<TReplacedUpdate*> m_arrReplacedUpdates;
	
	IVulfixObserver *m_pObserver;
	INT m_nDBType, m_nTotalItem, m_nCurrentItem;
	BOOL m_bFindAll, m_bCanceled;

	CObjLock	m_objLock;

    CString     m_strMirrorHostName;
};

class CDBOfficeLeak : public CDBOSLeak
{
	friend class CImplVulfix;
public:
	~CDBOfficeLeak();
	virtual void Reset();
	BOOL Load(LPCTSTR szfilename, DBFilterBase *filter, DWORD dwFlags);

protected:
	BOOL _ReadOffices( CComPtr<IXMLDOMNode> lpRoot );
	BOOL _ReadOffices( CXmlManager &xml, IXMLDOMNode *lpNode );
	bool _ReadOfficeItem( CXmlManager &xml, IXMLDOMNode *lpNode, CSimpleArray<TOfficeVersion>& officeTypes );

protected:
	CSimpleArray<TOfficeVersion> m_officeTypes;
};

class CDBSoftLeak : public CDBOSLeak
{
public:
	~CDBSoftLeak();
	virtual void Reset();
	BOOL Load(LPCTSTR szfilename, DBFilterBase *filter, DWORD dwFlags);
	int GetUnfixedLeakList(CSimpleArray<LPTVulSoft> &arrList );
	
protected:
	LPTVulSoft ndReadVulnerability( CXmlManager &xml, IXMLDOMNode *lpNode);
	bool _ndReadVulnerabilityInfo( CXmlManager &xml, IXMLDOMNode * lpNode, LPTVulSoft lpItem );
	
	BOOL ndReadSubitems( CXmlManager &xml, IXMLDOMNode *lpNode, TVulSoftSubitem &subitem );
	BOOL _CheckCLSIDMatched( LPCTSTR szCLSID, LPCTSTR szFrom, LPCTSTR szTo );
	void _ParseState(  LPTVulSoft lpItem, bool bMatched );
protected:
	CSimpleArray<LPTVulSoft> m_arrUpdateItem;
};

#pragma once
#include "vulfix/Defines.h"
#include "LeakDB.h"
#include "ImplVulIgnore.h"

class CImplVulfix : public IVulfix
{
public:
	CImplVulfix();
	virtual ~CImplVulfix(void);
	
public:
	virtual void SetObserver(IVulfixObserver *pObserver);
	virtual HRESULT Scan(DWORD dwFlags);
	virtual HRESULT Cancel();
	virtual const CSimpleArray<LPTUpdateItem>& GetResults();
	virtual const CSimpleArray<LPTVulSoft>& GetSoftVuls();

	virtual const CSimpleArray<TItemFixed*>& GetFixedVuls();
	virtual const CSimpleArray<LPTUpdateItem>& GetIgnoredVuls();
	virtual const CSimpleArray<LPTUpdateItem>& GetInvalidVuls();
	virtual const CSimpleArray<TReplacedUpdate*>& GetReplacedVuls();
	
	virtual BOOL Ignore(int nKBID, BOOL bIgnore);
	virtual BOOL PersistIgnored();
	
	virtual HRESULT EnableVulCOM(int nId, BOOL bEnable=FALSE);

	virtual BOOL GetLatestPackgeDate(INT &nYear, INT &nMonth, INT &nDay);
	virtual BOOL GetLatestInstalledKBDate(FILETIME &latestInstallTime);
	virtual INT ExpressScanSystem();

protected:
	void Init();
	void Reset();
	void Notify(TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2);

protected:
	static DWORD WINAPI _ThreadFuncDownload(LPVOID lpVoid);
	static DWORD WINAPI _ThreadFuncInstall(LPVOID lpVoid);
	DWORD _ThreadFuncDownloadImpl();
	DWORD _ThreadFuncInstallImpl();

protected:
	BOOL m_inited;
	BOOL m_Canceled;	//
	
	// 
	CDBOSLeak m_dbOS;
	CDBOfficeLeak m_dbOffice;
	CDBSoftLeak m_dbSoft;

	// 
	CSimpleArray<LPTUpdateItem> m_arrLeaks, m_arrIgnoredVuls, m_arrInvalid;
	CSimpleArray<TItemFixed*> m_arrFixedVuls;
	CSimpleArray<LPTVulSoft> m_arrSoftLeaks;
	CSimpleArray<TReplacedUpdate*> m_arrReplacedUpdates;	// 过期的不需要安装的补丁

	CSimpleArray<LPTUpdateItem> m_arrIgnoredVulsFromSoft;	// New UpdateItem from TVulSoft 
	
	CImplVulIgnore &m_objIgnore;
	IVulfixObserver *m_Observer;
	DBFilterBase *m_pFilterOS;
};

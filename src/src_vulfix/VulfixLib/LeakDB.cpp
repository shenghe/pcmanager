#include "StdAfx.h"
#include "LeakDB.h"
#include "ItemFilter.h"
#include "XmlManager.h"
#include "EnvUtils.h"
#include "HotfixUtils.h"
#include "LibLoader.h"

BOOL ParseUserPatcherXml( LPCTSTR szXmlfile, T_UserPatchInfo &patchInfo )
{	
	CXmlManager xml;
	if( !CLibLoader::LoadFileXml(szXmlfile, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode> lpNdPatcher;
	if( !xml.selectSingleNode(_T("UserPatcher"), &lpNdPatcher) ||
		 NULL == lpNdPatcher)
		return FALSE;

	xml.ReadSubXmlNodeString( lpNdPatcher, _T("Name"),			patchInfo.strName );
	xml.ReadSubXmlNodeString( lpNdPatcher, _T("ProductKey"),	patchInfo.strProductKey );
	xml.ReadSubXmlNodeString( lpNdPatcher, _T("PatchKey"),		patchInfo.strPatchKey );
	xml.ReadSubXmlNodeString( lpNdPatcher, _T("PatchValue"),	patchInfo.strPatchValue );
	
	CComBSTR bstrName = _T("File");
	CComPtr<IXMLDOMNodeList> lpFileNodes;
	if( SUCCEEDED(lpNdPatcher->selectNodes(bstrName, &lpFileNodes)) || lpFileNodes )
	{
		long nListLength = 0;
		lpFileNodes->get_length(&nListLength);
		for (int i = 0; i < nListLength; i++)
		{
			CComPtr<IXMLDOMNode> lpSubNode;
			if ( SUCCEEDED(lpFileNodes->get_item(i, &lpSubNode)) && lpSubNode)
			{
				CString strFrom, strTo;
				xml.ReadSubXmlNodeString( lpSubNode, _T("from"),strFrom );
				xml.ReadSubXmlNodeString( lpSubNode, _T("to"),	strTo );
				patchInfo.AddPatch( strFrom, strTo );
			}
		}
	}
	return patchInfo.files.GetSize()>0;
}

CDBOSLeak::~CDBOSLeak()
{
	Reset();
}

BOOL CDBOSLeak::Load( LPCTSTR szfilename, DBFilterBase *filter, DWORD dwFlags)
{
	//CObjGuard guard(m_objLock);

	Reset();
	ATLASSERT(filter);
	
	m_bCanceled = FALSE;
	m_bFindAll = !(dwFlags & VULSCAN_EXPRESS_SCAN);
	m_nDBType = VTYPE_WINDOWS;
	
	CXmlManager xml;
	if( !CLibLoader::LoadFile(szfilename, xml) )
		return FALSE;
	
	CComPtr<IXMLDOMNode> lpRoot;
	if( !xml.selectSingleNode(_T("Hotfix"), &lpRoot) )
		return FALSE;
	
	m_dwSysLang = GetLangID();

	_ReadMirrorHostName(xml, lpRoot);
    _ReadExpiredKBId(xml, lpRoot);
	_ReadReplaceds(xml, lpRoot, filter);
	
	BOOL bRet = _ReadUpdates(xml, lpRoot, filter);
	
	// append installed info from reg , and not found in db.lib 
	M_KBInfo &arrInfo = filter->GetInstalledInfo();
	for( M_KBInfo::iterator it=arrInfo.begin(); it!=arrInfo.end(); ++it)
	{
		if(!it->second.fromDB)
		{
			m_installedItems.Add( new TItemFixed( it->second ) );
		}
	}
	return bRet;
}

int CDBOSLeak::GetUnfixedLeakList( CSimpleArray<LPTUpdateItem> &arrList, CSimpleArray<TItemFixed*> &arrInstalled, CSimpleArray<LPTUpdateItem> &arrInvalid, CSimpleArray<TReplacedUpdate*> &arrReplacedUpdates )
{
	CopySimpleArray(m_updateItems, arrList);
	CopySimpleArray(m_installedItems, arrInstalled);
	CopySimpleArray(m_invalidItems, arrInvalid);
	CopySimpleArray(m_arrReplacedUpdates, arrReplacedUpdates);
	return arrList.GetSize();
}

const CSimpleArray<int> & CDBOSLeak::GetExpiredIds() const
{
	return m_arrExpiredKBid;
}

void CDBOSLeak::Cancel()
{
	m_bCanceled = TRUE;
}

void CDBOSLeak::Reset()
{
	ClearResult( m_updateItems );
	ClearResult( m_installedItems );
	ClearResult( m_invalidItems );
	ClearResult( m_arrReplacedUpdates );
	m_arrExpiredKBid.RemoveAll();

	m_nTotalItem = m_nCurrentItem = 0;
	m_LatestPackageDate.Reset();
	m_bCanceled = FALSE;
}

BOOL CDBOSLeak::_ReadExpiredKBId( CXmlManager &xml, IXMLDOMNode *lpRoot )
{
	m_arrExpiredKBid.RemoveAll();
	
	CComPtr<IXMLDOMNode> lpRootExpired;
	if( !xml.selectSingleNode(lpRoot, _T("Expired"), &lpRootExpired) )
		return FALSE;
	
	XmlLoopCallback<CDBOSLeak,CSimpleArray<int> > funcReadExpiredKBID(this, &CDBOSLeak::_ReadExpiredKBIdItem, m_arrExpiredKBid);
	xml.loopSubNode( lpRootExpired, _T("KBID"), funcReadExpiredKBID);
	return TRUE;
}

bool CDBOSLeak::_ReadExpiredKBIdItem( CXmlManager &xml, IXMLDOMNode *lpNode, CSimpleArray<int>& arrKBID )
{
	CString strKBID;
	xml.ReadXmlNodeString(lpNode, strKBID);
	if( !strKBID.IsEmpty() )
		arrKBID.Add( _ttoi(strKBID) );
	return true;
}

BOOL CDBOSLeak::_ReadReplaceds( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase *filter )
{
	CComPtr<IXMLDOMNode> lpUPs;
	if( !xml.selectSingleNode(lpNode, _T("ReplacedUPs"), &lpUPs) )
		return FALSE;
	
	XmlLoopCallback<CDBOSLeak,DBFilterBase* > funcReadReplaced(this, &CDBOSLeak::_ReadReplacedItem, filter);
	xml.loopSubNode(lpUPs, _T("ReplacedUP"), funcReadReplaced);
	return TRUE;
}

bool CDBOSLeak::_ReadReplacedItem( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase * &filter )
{
	CComPtr<IXMLDOMNode> lpNdOptions;
	if( xml.selectSingleNode(lpNode, _T("Subitems"), &lpNdOptions) )
	{	
		CComBSTR bstrName = _T("Subitem");
		CComPtr<IXMLDOMNodeList> lpNodes;
		if( SUCCEEDED(lpNdOptions->selectNodes(bstrName, &lpNodes)) || lpNodes )
		{
			long nListLength = 0;
			lpNodes->get_length(&nListLength);
			for (int i = 0; i < nListLength; i++)
			{
				CComPtr<IXMLDOMNode> lpOptionNode;
				if ( SUCCEEDED(lpNodes->get_item(i, &lpOptionNode)) && lpOptionNode)
				{
					TOptionCondition condition;
					DWORD dwLang = 0;
					if( _ReadNodePrequisite(xml, lpOptionNode, condition) 
						&& singleton<CSysEnv>::Instance().CheckPrequisites( condition, dwLang ) )
					{
						TReplacedUpdate *pUpdateItem = new TReplacedUpdate;

						xml.ReadSubXmlNodeKBID( lpNode, _T("KBID"), pUpdateItem->nKBID, 0 );	
						xml.ReadSubXmlNodeKBID( lpNode, _T("KBID2"), pUpdateItem->nKBID2, 0 );
						xml.ReadSubXmlNodeString( lpNode, _T("PubDate"), pUpdateItem->strPubdate, NULL );
						m_arrReplacedUpdates.Add( pUpdateItem );
						break;
					}
				}
			}
		}
	}
	return true;
}

BOOL CDBOSLeak::_ReadUpdates( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase *filter )
{
	CComPtr<IXMLDOMNode> lpUpdates;
	if( !xml.selectSingleNode(lpNode, _T("Updates"), &lpUpdates) )
		return FALSE;
	
	CComPtr<IXMLDOMNamedNodeMap> lpNodeAttribs;
	if( SUCCEEDED(lpUpdates->get_attributes( &lpNodeAttribs )) && lpNodeAttribs )
	{
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("num"), m_nTotalItem, 0 );
	}
	_Notify(EVulfix_ScanBegin, m_nTotalItem);
	m_nCurrentItem = 0;
	
	XmlLoopCallback<CDBOSLeak,DBFilterBase* > funcReadUpdate(this, &CDBOSLeak::_ReadUpdateItem, filter);
	xml.loopSubNode(lpUpdates, _T("Item"), funcReadUpdate);
	return TRUE;
}

bool CDBOSLeak::_ReadUpdateItem( CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase * &filter )
{
	LPTUpdateItem pupdateItem = new TUpdateItem;
	pupdateItem->nID = -1;	
	BOOL bUpdateValid = _ReadUpdateItem(xml, lpNode, filter, *pupdateItem);
	INT nKBID = pupdateItem->nID;
	
	BOOL bContinue = TRUE;
	
	if( bUpdateValid )
	{
		pupdateItem->isIgnored = filter->IsIgnored( pupdateItem->nID );
		if( !m_bFindAll && pupdateItem->nWarnLevel>0 && !pupdateItem->isIgnored )
			bContinue = FALSE;
		m_updateItems.Add( pupdateItem );
	}
	else if( pupdateItem->isInstalled )
	{
		TItemFixed *pfixed = new TItemFixed;
		pfixed->nID = nKBID;
		pfixed->fromDB = TRUE;
		pfixed->strName = pupdateItem->strName;
		pfixed->strWebPage = pupdateItem->strWebpage;
		SAFE_DELETE(pupdateItem);
		
		// 
		M_KBInfo &arrInfo = filter->GetInstalledInfo();
		M_KBInfo::iterator it = arrInfo.find( nKBID );
		if(it!=arrInfo.end())
		{
			it->second.fromDB = TRUE;
			pfixed->strInstallDate = it->second.strInstallDate;
		}
		m_installedItems.Add( pfixed );
	}
	else 
	{
		m_invalidItems.Add( pupdateItem );
	}
	_Notify(EVulfix_ScanProgress, ++m_nCurrentItem);
	return bContinue && !m_bCanceled;
}

//读取每个节点的数据结构。
BOOL CDBOSLeak::_ReadUpdateItem(CXmlManager &xml, IXMLDOMNode *lpNode, DBFilterBase *filter, TUpdateItem &updateItem )
{
	ATLASSERT( lpNode && filter );
	if(!lpNode || !filter) return FALSE;
	
	USES_CONVERSION;
	xml.ReadSubXmlNodeKBID(lpNode, _T("KBID"), updateItem.nID);
	xml.ReadSubXmlNodeString( lpNode, _T("Name"), updateItem.strName );	
	xml.ReadSubXmlNodeString( lpNode, m_dwSysLang==ESYSLang_CN ? _T("Webpage_CN") : _T("Webpage_EN"), updateItem.strWebpage );
	xml.ReadSubXmlNodeString( lpNode, _T("Pubdate"), updateItem.strPubdate );
	xml.ReadSubXmlNodeInt	( lpNode, _T("WarnLevel"),	updateItem.nWarnLevel, 0 );

	///> 读取flag
	INT nFlags = 0;
	xml.ReadSubXmlNodeInt( lpNode, _T("VFlags"),	nFlags, 0 );
	updateItem.dwVFlags = nFlags;

	nFlags = 0;
	xml.ReadSubXmlNodeInt( lpNode, _T("ExFlags"), nFlags, 0 );
	if ( (INT)VFLAG_IGNORE_INSTALLED == nFlags )
	{
		SetFlagOn( VFLAG_IGNORE_INSTALLED, updateItem.dwVFlags );
	}

	if( updateItem.nWarnLevel>0 && !updateItem.strPubdate.IsEmpty() )
		_UpdatePackageLatestDate(updateItem.strPubdate);
	
	CComPtr<IXMLDOMNode> lpNodeOptions;
	if ( xml.selectSingleNode(lpNode, _T("Subitems"), &lpNodeOptions)
		&& _ReadUpdateItemOptions(xml, lpNodeOptions, updateItem.nID, filter, updateItem)
		)
	{		
		xml.ReadSubXmlNodeString( lpNode, _T("Msid"), updateItem.strMsid );		
		xml.ReadSubXmlNodeString( lpNode, _T("Suggestion"), updateItem.strSuggestion );		
		xml.ReadSubXmlNodeString( lpNode, _T("Affects"), updateItem.strAffects );
		xml.ReadSubXmlNodeString( lpNode, _T("Description"), updateItem.strDescription );
		xml.ReadSubXmlNodeString( lpNode, _T("InstallParam"), updateItem.strInstallparam );
		xml.ReadSubXmlNodeInt( lpNode, _T("Exclusive"), updateItem.isExclusive, FALSE );

		if( !updateItem.detail.strWebPage.IsEmpty() )
			updateItem.strWebpage = updateItem.detail.strWebPage;

		if( IsFlagOn( VFLAG_SERVICE_PATCH, updateItem.dwVFlags ) )	// SP 的补丁必须可选 
			updateItem.nWarnLevel = 0;

		return TRUE;
	}
	return FALSE;
}

//读取每个Option 中的内容
BOOL CDBOSLeak::_ReadUpdateItemOptions(CXmlManager &xml, IXMLDOMNode *lpNode, int nKBId, DBFilterBase *filter, TUpdateItem &updateItem)
{
	if(!lpNode || nKBId==0 || !filter) return FALSE;
	
	CComBSTR bstrName = _T("Subitem");
	CComPtr<IXMLDOMNodeList> lpNodes;
	if( FAILED(lpNode->selectNodes(bstrName, &lpNodes)) || !lpNodes )
		return FALSE;
	
	long nListLength = 0;
	lpNodes->get_length(&nListLength);
	for (int i = 0; i < nListLength; i++)
	{
		CComPtr<IXMLDOMNode> lpOptionNode;
		if ( SUCCEEDED(lpNodes->get_item(i, &lpOptionNode)) && lpOptionNode)
		{
			TOptionCondition condition;
			int nType = 0;
			DWORD dwLang = 0;
			
			// 只读取满足条件的内容 			
			if( _ReadNodeCondition(xml, lpOptionNode, condition) 
				&& singleton<CSysEnv>::Instance().CheckPrequisites( condition, dwLang ) )
			{
				xml.ReadSubXmlNodeString( lpOptionNode, _T("Installed"), condition.m_strExpInstalled );
				BOOL bInstalled = filter->IsInstalled( condition, nKBId );
				if( bInstalled )
					updateItem.isInstalled = TRUE;

				if( ( !bInstalled || IsFlagOn( VFLAG_IGNORE_INSTALLED, updateItem.dwVFlags ) ) 
					&& !_IsUpdateReplaced(nKBId) 
					&& filter->IsInstallable(condition, nKBId, nType, dwLang) )
				{
					updateItem.m_nType = m_nDBType;
					updateItem.strCondition = condition.m_strCondition;
					return _ReadUpdateItemOptionProcessInfo(xml, lpOptionNode, dwLang, updateItem);
				}
			}
		}
	}
	return FALSE;
}

BOOL CDBOSLeak::_ReadNodeCondition( CXmlManager &xml, IXMLDOMNode *lpNode, TOptionCondition &condition )
{
	if( !xml.ReadSubXmlNodeString( lpNode, _T("Condition"), condition.m_strCondition )  )
		return FALSE;
	
	CComPtr<IXMLDOMNode> lpNodeConition;
	if( xml.selectSingleNode( lpNode, _T("Condition"), &lpNodeConition) )
	{
		return _ReadNodePrequisite(xml, lpNodeConition, condition);
	}
	return FALSE;
}

BOOL CDBOSLeak::_ReadNodePrequisite( CXmlManager &xml, IXMLDOMNode *lpNode, TOptionCondition &condition )
{
	CComPtr<IXMLDOMNamedNodeMap> lpNodeAttribs;
	if( SUCCEEDED(lpNode->get_attributes( &lpNodeAttribs )) && lpNodeAttribs )
	{
		CString strOs;
		if( xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("os"), strOs) )
			condition.nWinVer = TransSysCode( strOs );
		else 
			condition.nWinVer = -1; 

		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("ossp"), condition.nWinSP, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("ie"), condition.nIEVer, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("iesp"), condition.nIESP, -1 );

		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("js"), condition.nJS, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("mediaplayer"), condition.nMediaplayer, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("dx"), condition.nDX, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("oe"), condition.nOE, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("db"), condition.nDataAccess, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("framework"), condition.nFramework, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("xml"), condition.nXML, -1 );
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("msn"), condition.nMSN, -1 );

		// 
		xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("office"), condition.strOffice );
		return TRUE;
	}
	return FALSE;
}

BOOL ReplaceUrlHostName(LPCTSTR szUrl, LPCTSTR szNewHost, CString &strNewUrl)
{
#define HTTP_PREFIX _T("http://")

	ATLASSERT(szUrl && szNewHost);
	int nStart = sizeof(HTTP_PREFIX) / sizeof(TCHAR) - 1;
	if( _tcsnicmp(HTTP_PREFIX, szUrl, nStart)==0 )
	{
		LPCTSTR lpPath = _tcschr(szUrl + nStart, _T('/'));
		if(lpPath)
		{
			strNewUrl = HTTP_PREFIX;
			strNewUrl += szNewHost;
			strNewUrl += lpPath;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDBOSLeak::_ReadUpdateItemOptionProcessInfo( CXmlManager &xml, IXMLDOMNode *lpNode, DWORD dwLang, TUpdateItem &upitem )
{
	LPCTSTR szTagName = _T("CN");
	if( (m_nDBType==VTYPE_OFFICE && dwLang==ESYSLang_EN) 
		|| (m_nDBType!=VTYPE_OFFICE && m_dwSysLang==ESYSLang_EN) ) 
	{
		szTagName = _T("EN");
	}

	ATLTRACE(_T("[knvul]ReadItemInfo kb=%d, dbtype=%d, syslang=%d, dwlang=%d,%s\n"), upitem.nID, m_nDBType, m_dwSysLang, dwLang, szTagName);
	
	CComPtr<IXMLDOMNode> lpNodeProcess;
	if( xml.selectSingleNode( lpNode, szTagName, &lpNodeProcess) )
	{
		xml.ReadSubXmlNodeString( lpNodeProcess, _T("Webpage"), upitem.detail.strWebPage );
		xml.ReadSubXmlNodeString( lpNodeProcess, _T("DownURL"), upitem.detail.strDownurl );
		xml.ReadSubXmlNodeInt( lpNodeProcess, _T("Size"), upitem.detail.nPatchSize, 0);
		xml.ReadSubXmlNodeString( lpNodeProcess, _T("InstallParam"), upitem.detail.strInstallParam );
		
		INT userPatcher = 0;
		xml.ReadSubXmlNodeInt( lpNodeProcess, _T("UserPatcher"), userPatcher, 0);
		if( userPatcher )
			upitem.bUserPatcher = TRUE;
        if (!m_strMirrorHostName.IsEmpty())
        {
			ReplaceUrlHostName(upitem.detail.strDownurl, m_strMirrorHostName, upitem.detail.strMirrorurl);
        }
		//ReplaceUrlHostName(upitem.detail.strDownurl, _T("localhost"), upitem.detail.strDownurl);
		return TRUE;
	}
	return FALSE;
}

BOOL CDBOSLeak::_IsUpdateReplaced( int nKBId )
{
	for(int i=0; i<m_arrReplacedUpdates.GetSize(); ++i)
	{
		TReplacedUpdate *pReplacedUpdate = m_arrReplacedUpdates[i];
		if(pReplacedUpdate->nKBID==nKBId)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CDBOSLeak::_UpdatePackageLatestDate( LPCTSTR szPubDate )
{
	INT nYear=0, nMonth=0, nDay=0;
	if( ParseDateString(szPubDate, nYear, nMonth, nDay))
	{
		m_LatestPackageDate.Update(nYear, nMonth, nDay);
	}
}

BOOL CDBOSLeak::_ReadMirrorHostName( CXmlManager &xml, IXMLDOMNode *lpRoot )
{
    CComPtr<IXMLDOMNode> lpRootServerInfo;
    if( !xml.selectSingleNode(lpRoot, _T("ServerInfo"), &lpRootServerInfo) )
        return FALSE;

    CComPtr<IXMLDOMNode> lpMirrorHost;
    if( !xml.selectSingleNode(lpRootServerInfo, _T("MirrorHost"), &lpMirrorHost))
        return FALSE;

    xml.ReadXmlNodeString(lpMirrorHost, m_strMirrorHostName);

    return TRUE;
}

void CDBOSLeak::GetMirrorHostName(CString& strHostName)
{
    strHostName = m_strMirrorHostName;
}

CDBOfficeLeak::~CDBOfficeLeak()
{
	Reset();
}

void CDBOfficeLeak::Reset()
{
	m_officeTypes.RemoveAll();
	CDBOSLeak::Reset();
}

BOOL CDBOfficeLeak::Load( LPCTSTR szfilename, DBFilterBase *filter, DWORD dwFlags )
{
	//CObjGuard guard(m_objLock);
	Reset();
	m_bFindAll = !(dwFlags & VULSCAN_EXPRESS_SCAN);
	m_nDBType = VTYPE_OFFICE;
	
	CXmlManager xml;
	if( !CLibLoader::LoadFile(szfilename, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode> lpRoot;
	if( !xml.selectSingleNode(_T("Hotfix"), &lpRoot) )
		return FALSE;
	
	m_dwSysLang = GetLangID();
	_ReadMirrorHostName(xml, lpRoot);
	if(!_ReadOffices(xml, lpRoot))
		return FALSE;
	
	singleton<CSysEnv>::Instance().InitOffice( m_officeTypes );
	if(!singleton<CSysEnv>::Instance().IsOfficeLangSupported())
		return FALSE;

	if(!_ReadUpdates(xml, lpRoot, filter) )
		return FALSE;
	return TRUE;
}

BOOL CDBOfficeLeak::_ReadOffices( CXmlManager &xml, IXMLDOMNode *lpNode )
{
	CComPtr<IXMLDOMNode> lpNdRootOffices;
	if( !xml.selectSingleNode(lpNode, _T("Offices"), &lpNdRootOffices) )
		return FALSE;
	
	XmlLoopCallback<CDBOfficeLeak,CSimpleArray<TOfficeVersion> > funcReadUpdate(this, &CDBOfficeLeak::_ReadOfficeItem, m_officeTypes);
	xml.loopSubNode(lpNdRootOffices, _T("Office"), funcReadUpdate);
	return TRUE;
}

bool CDBOfficeLeak::_ReadOfficeItem( CXmlManager &xml, IXMLDOMNode *lpNode, CSimpleArray<TOfficeVersion>& officeTypes )
{
	CComPtr<IXMLDOMNamedNodeMap> lpNodeAttribs;
	if( SUCCEEDED(lpNode->get_attributes( &lpNodeAttribs )) && lpNodeAttribs )
	{
		TOfficeVersion version;
		if( xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("id"), version.type, -1 ) 
			&& xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("name"), version.strName ) 
			&& xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("from"), version.strVFrom ) 
			&& xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("to"), version.strVTo ) )
		{
			version.strName = version.strName.MakeLower();
			officeTypes.Add( version );
		}
	}
	return true;	
}
/////////
/// 第三方软件

CDBSoftLeak::~CDBSoftLeak()
{
	Reset();
}

BOOL CDBSoftLeak::Load( LPCTSTR szfilename, DBFilterBase *filter, DWORD dwFlags)
{
	//CObjGuard guard(m_objLock);
	Reset();
	m_bFindAll = !(dwFlags & VULSCAN_EXPRESS_SCAN);
	m_nDBType = VTYPE_SOFTLEAK;
	
	CXmlManager xml;
	if( !CLibLoader::LoadFile(szfilename, xml) )
		return FALSE;

	m_dwSysLang = GetLangID();
		
	CComPtr<IXMLDOMNode> lpRoot;
	if( !xml.selectSingleNode(_T("Hotfix"), &lpRoot) )
		return FALSE;
	
	// 
	CComPtr<IXMLDOMNode> lpUpdates;
	if( !xml.selectSingleNode(lpRoot, _T("Updates"), &lpUpdates) )
		return FALSE;
	
	CComPtr<IXMLDOMNamedNodeMap> lpNodeAttribs;
	if( SUCCEEDED(lpUpdates->get_attributes( &lpNodeAttribs )) && lpNodeAttribs )
	{
		xml.ReadXmlNodeAttribInt( lpNodeAttribs, _T("num"), m_nTotalItem, 0 );
	}
	_Notify(EVulfix_ScanBegin, m_nTotalItem);
	m_nCurrentItem = 0;
		
	// 
	CComBSTR bstrNodeName = _T("Item");
	CComPtr<IXMLDOMNodeList> lpIXMLDOMNodeList;
	if (FAILED(lpUpdates->selectNodes(bstrNodeName, &lpIXMLDOMNodeList)) || !lpIXMLDOMNodeList)
	{
		return FALSE;
	}
	long nListLength = 0;
	lpIXMLDOMNodeList->get_length(&nListLength);
	for (int i = 0; i < nListLength && !m_bCanceled; i++)
	{
		CComPtr<IXMLDOMNode> ndSubItem;
		lpIXMLDOMNodeList->get_item(i, &ndSubItem);
		if (!ndSubItem) continue;
		
		//读取每个节，并填充数据结构。
		// TODO : 会不会忽略没有匹配到子条件的项呢???
		LPTVulSoft lpItem = ndReadVulnerability(xml, ndSubItem);
		if(lpItem)
		{
			bool bMatchedCLSID = _ndReadVulnerabilityInfo(xml, ndSubItem, lpItem);
			if(lpItem->matchedItem.nAction==SOFTACTION_NONE && lpItem->nDisableCom)
				lpItem->matchedItem.nAction = SOFTACTION_DISABLECOM;
			_ParseState(lpItem, bMatchedCLSID);
			if(lpItem->matchedItem.nAction==SOFTACTION_DISABLECOM)
			{
				// 禁用COM , 但是没有找到可禁用的COM, 那么应该不显示 
				if(!lpItem->state.bFounded)
				{
					delete lpItem;
					lpItem = NULL;
				}
			}
			if(lpItem)
			{
				m_arrUpdateItem.Add(lpItem);
				if(!m_bFindAll)
					break;
			}
		}
		_Notify(EVulfix_ScanProgress, ++m_nCurrentItem);
	}
	return TRUE;
}

int CDBSoftLeak::GetUnfixedLeakList( CSimpleArray<LPTVulSoft> &arrList )
{
	CopySimpleArray( m_arrUpdateItem, arrList );
	return arrList.GetSize();
}

void CDBSoftLeak::Reset()
{
	ClearResult( m_arrUpdateItem );
	m_bCanceled = FALSE;
}

//读取每个节点的数据结构。
LPTVulSoft CDBSoftLeak::ndReadVulnerability( CXmlManager &xml, IXMLDOMNode *lpNode)
{
	if(!lpNode) return NULL;
	INT nId = 0;
	if( !xml.ReadSubXmlNodeInt( lpNode, _T("VID"), nId, -1 ) )
		return NULL;
	
	USES_CONVERSION;
	LPTVulSoft lpItem = new TVulSoft;
	lpItem->nID = nId;

	CComPtr<IXMLDOMNode> lpSubnode;
	if( xml.selectSingleNode(lpNode, _T("Subitems"), &lpSubnode) )
	{
		if( ndReadSubitems(xml, lpSubnode, lpItem->matchedItem ) )
		{
			return lpItem;
		}
	}
	delete lpItem;
	return NULL;
}

BOOL CDBSoftLeak::ndReadSubitems( CXmlManager &xml, IXMLDOMNode *lpNode, TVulSoftSubitem &subitem )
{
	if(!lpNode) return FALSE;
	
	USES_CONVERSION;
	CComBSTR bstrName = _T("Subitem");
	CComPtr<IXMLDOMNodeList> lpNodes;
	if (SUCCEEDED(lpNode->selectNodes(bstrName, &lpNodes)) && lpNodes)
	{
		long nListLength = 0;
		lpNodes->get_length(&nListLength);
		for (int i = 0; i < nListLength; i++)
		{
			CComPtr<IXMLDOMNode> lpNdSubitem;
			lpNodes->get_item(i, &lpNdSubitem);
			if(!lpNdSubitem) continue;
			
			// 只读取满足条件的内容 
			if( _ReadNodeCondition(xml, lpNdSubitem, subitem.conditon) 
				&& EvaluateCondition( subitem.conditon.m_strCondition ) )
			{
				xml.ReadSubXmlNodeString( lpNdSubitem, _T("Name"), subitem.strName );
				xml.ReadSubXmlNodeInt( lpNdSubitem, _T("Action"), subitem.nAction, 0 );
				xml.ReadSubXmlNodeString( lpNdSubitem, _T("Webpage"), subitem.strWebpage );
				xml.ReadSubXmlNodeString( lpNdSubitem, _T("Downpage"), subitem.strDownpage );
				xml.ReadSubXmlNodeString( lpNdSubitem, _T("DownURL"), subitem.strDownUrl );
				xml.ReadSubXmlNodeInt( lpNdSubitem, _T("Size"), subitem.nSize, 0 );
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool CDBSoftLeak::_ndReadVulnerabilityInfo( CXmlManager &xml, IXMLDOMNode * nd, LPTVulSoft lpItem )
{
	ATLASSERT(lpItem);
	xml.ReadSubXmlNodeString( nd, _T("Name"), lpItem->strName );
	xml.ReadSubXmlNodeString( nd, _T("Pubdate"), lpItem->strPubdate );
	xml.ReadSubXmlNodeString( nd, _T("OrigURL"), lpItem->strOrigUrl );
	xml.ReadSubXmlNodeString( nd, _T("Summary"), lpItem->strDescription );
	xml.ReadSubXmlNodeInt( nd, _T("WarnLevel"), lpItem->nLevel, 0 );
	xml.ReadSubXmlNodeString( nd, _T("Affects"), lpItem->strAffects );
	xml.ReadSubXmlNodeString( nd, _T("Description"), lpItem->strDetailDescription );
	xml.ReadSubXmlNodeString( nd, _T("SideAffect"), lpItem->strSideaffect );
	xml.ReadSubXmlNodeInt( nd, _T("DisableCOM"), lpItem->nDisableCom, 0 );
	
	int nMatched = 0;
	CComPtr<IXMLDOMNode> lpSubnode;
	if( xml.selectSingleNode(nd, _T("CLSIDS"), &lpSubnode) )
	{		
		CComBSTR bstrName = _T("CLSID");
		CComPtr<IXMLDOMNodeList> lpNodes;
		if (SUCCEEDED(lpSubnode->selectNodes(bstrName, &lpNodes)) && lpNodes)
		{
			long nListLength = 0;
			lpNodes->get_length(&nListLength);
			for (int i = 0; i < nListLength; i++)
			{
				CComPtr<IXMLDOMNode> lpNdSubitem;
				lpNodes->get_item(i, &lpNdSubitem);
				if(!lpNdSubitem) continue;
				
				// 
				CString strCLSID;
				xml.ReadXmlNodeString(lpNdSubitem, strCLSID);
				if(strCLSID.IsEmpty())
					continue;
				
				CString strFrom, strTo;
				// 
				CComPtr<IXMLDOMNamedNodeMap> lpNodeAttribs;
				if( SUCCEEDED(lpNdSubitem->get_attributes( &lpNodeAttribs )) && lpNodeAttribs )
				{
					xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("vfrom"), strFrom);
					xml.ReadXmlNodeAttribString( lpNodeAttribs, _T("vto"), strTo);
				}

				if(_CheckCLSIDMatched(strCLSID, strFrom, strTo))
					++nMatched;
				lpItem->arrCLSID.Add( strCLSID );
			}
		}
		lpSubnode = NULL;
	}
	
	return nMatched>0;
}

BOOL CDBSoftLeak::_CheckCLSIDMatched( LPCTSTR szCLSID, LPCTSTR szFrom, LPCTSTR szTo )
{
	CString strFilename;
	LARGE_INTEGER lgVersion;
	if( GetCLSIDFilename(szCLSID, strFilename) && GetFileVersion(strFilename, lgVersion) )
	{
		return VersionInRange(lgVersion, szFrom, szTo);
	}
	return FALSE;
}

void CDBSoftLeak::_ParseState( LPTVulSoft lpItem, bool bMatched )
{
	// 
	lpItem->state.action = lpItem->matchedItem.nAction;
	lpItem->state.bMatched = bMatched;
	lpItem->state.comState = 0;
	
	// 2. 分析clsid 是否被禁用
	int nFoundedClsid = 0;
	int nDisabledClsid = 0;
	
	for(int i=0; i<lpItem->arrCLSID.GetSize(); ++i)
	{
		BOOL bEnabled;
		if( GetComState(lpItem->arrCLSID[i], bEnabled) )
		{
			if( !bEnabled )
				++ nDisabledClsid;
			++ nFoundedClsid;
		}
	}
	
	T_SoftComState comstate;
	if(nFoundedClsid>0 && nDisabledClsid>=nFoundedClsid)
		comstate = COM_ALL_DISABLED;
	else if(nDisabledClsid>0)
		comstate = COM_PART_DISABLED;
	else 
		comstate = COM_NOT_DISABLED;
	lpItem->state.comState = comstate;
	lpItem->state.bFounded = nFoundedClsid>0;
}

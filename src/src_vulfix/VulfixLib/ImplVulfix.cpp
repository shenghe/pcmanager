#include "stdafx.h"
#include "Utils.h"
#include "ItemFilter.h"
#include "LeakDB.h"
#include "ImplVulfix.h"
#include "HotfixUtils.h"
#include "EnvUtils.h"
#include "LibLoader.h"

CImplVulfix::CImplVulfix()
	: m_Observer(NULL), m_objIgnore( singleton<CImplVulIgnore>::Instance() )
{
	m_inited = FALSE;
	m_Canceled = FALSE;
	m_pFilterOS = NULL;
}	

CImplVulfix::~CImplVulfix(void)
{
	if( m_pFilterOS )
	{
		delete m_pFilterOS;
		m_pFilterOS = NULL;
	}
}

void CImplVulfix::SetObserver( IVulfixObserver *pObserver )
{
	m_Observer = pObserver;
}

HRESULT CImplVulfix::Scan(DWORD dwFlags)
{
	TIME_CHECK( _T("CImplVulfix::Scan ") );
	m_Canceled = FALSE;
	T_ComInit __init__com__;
	HRESULT hr ; 	
	do
	{
		Reset();
		GetLangID();
		
		CSysEnv& sysEnv = singleton<CSysEnv>::Instance();
		sysEnv.Init();
		if( FAILED( hr=sysEnv.IsSupported(FALSE) ) )
			break;
		
		Init();
		m_objIgnore.LoadIgnoreDB();

		CString filenameSystem, filenameOffice, filenameSoft;
		GetXmlDBFileName(VTYPE_WINDOWS, filenameSystem, IsWin64());
		GetXmlDBFileName(VTYPE_OFFICE, filenameOffice, FALSE);
		GetXmlDBFileName(VTYPE_SOFTLEAK, filenameSoft, FALSE);
		if( !PathFileExists(filenameSystem) && !PathFileExists(filenameOffice) && !PathFileExists(filenameSoft) )
		{
			hr = KERR_LOAD_FILE;
			break;
		}
		
		m_pFilterOS = CreateOSFilter(sysEnv.m_WinVer, dwFlags);
		InitOSFilter( m_pFilterOS, sysEnv.m_WinVer, dwFlags);
		if( m_pFilterOS->WaitComplete() )
		{
			m_dbOS.SetObserver( m_Observer );
			m_dbOffice.SetObserver( m_Observer );
			m_dbSoft.SetObserver( m_Observer );
			m_pFilterOS->SetIIgnore( &m_objIgnore );
			
			CString filename;
			try
			{
				FixLocale();

				//BOOL bWin64 = IsWin64();
				//PVOID OldValue = NULL;
				//if(bWin64)
				//	Wow64DisableWow64FsRedirection(&OldValue);
				
				!m_Canceled && m_dbOffice.Load( filenameOffice, m_pFilterOS, dwFlags );
				!m_Canceled && sysEnv.IsLangSupported() && sysEnv.IsOsSupported() && m_dbOS.Load( filenameSystem, m_pFilterOS, dwFlags );
				!m_Canceled && m_dbSoft.Load( filenameSoft, NULL, dwFlags);

				//if(bWin64)
				//	Wow64RevertWow64FsRedirection(OldValue);
			}
			catch (...)
			{
				hr = KERR_LOAD_FILE;
			}
			
			CSimpleArray<LPTUpdateItem> arrLeaks;
			CSimpleArray<TReplacedUpdate*> arrReplaced;			
			m_dbOS.GetUnfixedLeakList( arrLeaks, m_arrFixedVuls, m_arrInvalid, arrReplaced );
			m_dbOffice.GetUnfixedLeakList( arrLeaks, m_arrFixedVuls, m_arrInvalid, arrReplaced );

			const CSimpleArray<int> &arrExpired = m_dbOS.GetExpiredIds();
			CSimpleArray<int> arrReplacedId;
			for(int i=0; i<arrReplaced.GetSize(); ++i)
			{
				arrReplacedId.Add( arrReplaced[i]->nKBID );
			}
			
			// select soft ignored vuls 
			CSimpleArray<LPTVulSoft> arrSoftLeaks;
			m_dbSoft.GetUnfixedLeakList( arrSoftLeaks );
			for(int i=0; i<arrSoftLeaks.GetSize(); ++i)
			{
				LPTVulSoft ps = arrSoftLeaks[i];
				ps->isIgnored = m_objIgnore.IsIgnored( ps->nID );
				if( ps->isIgnored )
				{
					LPTUpdateItem pu = new TUpdateItem;
					pu->m_nType = VTYPE_SOFTLEAK;
					pu->nID = ps->nID;
					pu->strName = ps->matchedItem.strName;
					pu->strDescription = ps->strDescription;
					pu->strWebpage = ps->matchedItem.strWebpage;
					pu->nWarnLevel = ps->nLevel;
					pu->strPubdate = ps->strPubdate;

					m_arrIgnoredVuls.Add( pu );
					m_arrIgnoredVulsFromSoft.Add( pu );
				}
				else
					m_arrSoftLeaks.Add( ps );
			}
			
			// select installable, ignored , expired 
			for(int i=0; i<arrLeaks.GetSize(); ++i )
			{
				LPTUpdateItem &pi = arrLeaks[i];
				pi->isExpired = arrExpired.Find( pi->nID )!=-1;

				if(pi->isExpired)
					m_arrInvalid.Add( pi );
				else if(pi->isIgnored)
					m_arrIgnoredVuls.Add( pi );
				else
				{
					bool bReplaced = arrReplacedId.Find( pi->nID )!=-1;
					if(!bReplaced)
						m_arrLeaks.Add( pi );
				}
			}
			
			// find correct replace relationship 
			for(int i=0; i<arrReplaced.GetSize(); ++i)
			{
				TReplacedUpdate* pu = arrReplaced[i];
				BOOL bInstalled = FindArrayIndex( m_arrFixedVuls, pu->nKBID )!=-1;
				if( !bInstalled )
				{
					if( FindArrayIndex(m_arrFixedVuls, pu->nKBID2)!=-1 
						|| FindArrayIndex(m_arrLeaks, pu->nKBID2)!=-1
						|| FindArrayIndex(m_arrIgnoredVuls, pu->nKBID2)!=-1 )
						m_arrReplacedUpdates.Add( pu );
				}
			}

			// - 保存最后无漏洞时间, 使得下次不再提示有风险 
			// -- 非快速扫描的结果才有效果 
			if( RequireUsingInterface() && !(dwFlags & VULSCAN_EXPRESS_SCAN) )
			{
				BOOL hasMustLeak = FALSE;
				const CSimpleArray<LPTUpdateItem> &arrLeaks = GetResults();
				for(int i=0; i<arrLeaks.GetSize(); ++i)
				{
					if(arrLeaks[i]->nWarnLevel>0)
					{
						hasMustLeak = TRUE;
						break;
					}
				}
				CString strVal;
				if(!hasMustLeak)
				{
					T_Date date;
					GetLatestPackgeDate(date.nYear, date.nMonth, date.nDay);
					strVal.Format(_T("%04d-%02d-%02d"), date.nYear, date.nMonth, date.nDay);
				}
				WriteVulConfig(_T("VulScan"), _T("LastSafePkgDate"), strVal);
			}			
		}
		
		hr = KERR_NONE;
	} while (FALSE);
	return hr;
}

HRESULT CImplVulfix::Cancel()
{
	m_Canceled = TRUE;
	if(m_pFilterOS)
		m_pFilterOS->Cancel();
	m_dbOS.Cancel();
	m_dbOffice.Cancel();
	m_dbSoft.Cancel();
	return S_OK;
}

const CSimpleArray<LPTUpdateItem>& CImplVulfix::GetResults()
{
	return m_arrLeaks;
}

const CSimpleArray<LPTVulSoft>& CImplVulfix::GetSoftVuls()
{
	return m_arrSoftLeaks;
}

const CSimpleArray<TItemFixed*>& CImplVulfix::GetFixedVuls()
{
	return m_arrFixedVuls;
}

const CSimpleArray<LPTUpdateItem>& CImplVulfix::GetIgnoredVuls()
{
	return m_arrIgnoredVuls;
}

const CSimpleArray<LPTUpdateItem>& CImplVulfix::GetInvalidVuls()
{
	return m_arrInvalid;
}

const CSimpleArray<TReplacedUpdate*>& CImplVulfix::GetReplacedVuls()
{
	return m_arrReplacedUpdates;
}

BOOL CImplVulfix::Ignore( int nKBID, BOOL bIgnore )
{
	return m_objIgnore.Ignore( nKBID, bIgnore );
}

BOOL CImplVulfix::PersistIgnored()
{
	return m_objIgnore.SaveIgnoreDB();
}

HRESULT CImplVulfix::EnableVulCOM( int nId, BOOL bEnable/*=FALSE*/ )
{
	HRESULT hr = E_FAIL;
	do 
	{
		INT i = FindArrayIndex( m_arrSoftLeaks, nId );
		if( i==-1 ) break;

		LPTVulSoft pitem = m_arrSoftLeaks[i];
		if(!pitem->nDisableCom) break;
		
		for(int j=0; j<pitem->arrCLSID.GetSize(); ++j)
		{
			EnableCom( pitem->arrCLSID[j], bEnable );
		}
		hr = S_OK;
	} while (FALSE);
	return hr;
}

BOOL CImplVulfix::GetLatestPackgeDate( INT &nYear, INT &nMonth, INT &nDay )
{
	T_Date date = m_dbOS.m_LatestPackageDate;
	date.Update( m_dbOffice.m_LatestPackageDate );
	nYear = date.nYear;
	nMonth = date.nMonth;
	nDay = date.nDay;
	return TRUE;
}

BOOL CImplVulfix::GetLatestInstalledKBDate( FILETIME &latestInstallTime )
{
	// 只适合在不使用接口的时候使用 !! 
	CWUAUpdateInfoVistaLaterReg &wuaInfo = singleton<CWUAUpdateInfoVistaLaterReg>::Instance();
	latestInstallTime = wuaInfo.m_LastestUpdateTime;
	return TRUE;
}

INT CImplVulfix::ExpressScanSystem()
{
	// Not use IUpdate 
	HRESULT hr = Scan( VULSCAN_EXPRESS_SCAN );
	if(FAILED(hr))
		return 0;
	
	// check date 
	T_Date date;
	GetLatestPackgeDate(date.nYear, date.nMonth, date.nDay);

	// 上次扫描为安全的库版本号 
	CString strLastSafePkgDate;
	ReadVulConfig(_T("VulScan"), _T("LastSafePkgDate"), strLastSafePkgDate);
	T_Date dateSafe;

	SHOWMSG(_T("PkgDate %04d-%02d-%02d \r\nLastScan.PkgDate %s"), date.nYear, date.nMonth, date.nDay, strLastSafePkgDate);
	if(!strLastSafePkgDate.IsEmpty() && ParseDateString(strLastSafePkgDate, dateSafe.nYear, dateSafe.nMonth, dateSafe.nDay) )
	{
		if(date.Compare(dateSafe.nYear, dateSafe.nMonth, dateSafe.nDay)<=0)
			return -1;
	}
	
	FILETIME ft;
	SYSTEMTIME systime={0};
	if( GetLatestInstalledKBDate(ft) && FileTimeToSystemTime(&ft, &systime) )
	{
		SHOWMSG(_T("ExpressScanSystem.DateCompare %d-%d-%d - %d-%d-%d"), date.nYear, date.nMonth, date.nDay, systime.wYear, systime.wMonth, systime.wDay);
		return date.Compare(systime.wYear, systime.wMonth, systime.wDay);
	}
	return 0;
}

void CImplVulfix::Init()
{
	if(!m_inited)
	{
		m_inited = TRUE;
	}
}

void CImplVulfix::Reset()
{
	m_arrLeaks.RemoveAll();
	m_arrFixedVuls.RemoveAll();
	m_arrIgnoredVuls.RemoveAll();
	m_arrInvalid.RemoveAll();
	m_arrReplacedUpdates.RemoveAll();
	m_arrSoftLeaks.RemoveAll();

	ClearResult( m_arrIgnoredVulsFromSoft );
	m_dbOS.Reset();
	m_dbOffice.Reset();
	m_dbSoft.Reset();
}

void CImplVulfix::Notify( TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2 )
{
	if(m_Observer)
		m_Observer->OnVulfixEvent(evt, nKbId, dwParam1, dwParam2);
}

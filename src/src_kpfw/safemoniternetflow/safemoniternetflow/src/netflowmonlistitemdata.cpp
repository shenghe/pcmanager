#include "stdafx.h"
#include "bkres/bkres.h"
#include "kpfw/netfluxdef.h"
#include "netflowmonlistitemdata.h"
#include "kcompare.h"
#include "kpfw/kpfw_def.h"
#include "kpfw/netflowformat.h"
#include "src/beikesafemsgbox.h"
#include "kis_filever.h"
#include "sysmprocess.h"
#include "kpfw/msg_logger.h"
#include "KAccessControl.h"
#include "kmaindlg.h"

void GetUserTrustModeString(INT nTrustMode, ATL::CString& strTrust )
{
	switch( nTrustMode )
	{
	case enumTM_UnVerfiy:
		strTrust = BkString::Get(Str_TrustMode_UnVerfiy);
		break;
	case enumTM_Unknown:
		strTrust = BkString::Get(Str_TrustMode_Unknown);
		break;

	case enumTM_Safe:
		strTrust = BkString::Get(Str_TrustMode_Safe);
		break;

	case enumTM_Danger:
		strTrust =BkString::Get(Str_TrustMode_Danger);
		break;

	default:
		strTrust = BkString::Get(Str_TrustMode_Unknown);
		break;
	}

	return;
}

void KNetFlowMonListItemData::SetProcessPath(LPCTSTR szPath)
{
    m_strProcessPath = szPath;
}



KNetFlowMonListItemData::~KNetFlowMonListItemData()
{

}

void KNetFlowMonListItemData::SetTrust(LPCTSTR szTrust)
{
    m_strTrust = szTrust;
}

KNetFlowMonListItemData::KNetFlowMonListItemData(KProcFluxItem flowItem)
    :m_bIconExtracted(FALSE),
      m_hSmallIcon(NULL),
      m_ProcessID(0),
	  m_defaultIcon(NULL),
	  m_bIsShowOpenFileLink(TRUE)
{
    m_strProcessName					= L"";
    m_strTrust							= L"";
    m_strSmallIcon 						= L"";
    m_strProcessPath 					= L"";
    m_strSendSpeedText 					= L"";
    m_strReceiveSpeedText 				= L"";
    m_strTotalReceiveText 				= L"";
    m_strTotalSendText 					= L"";
	m_strPID 							= L"";
    m_ProcessFlowItem 					= flowItem;
	m_dwDisable 						= flowItem.m_nDisable;
	m_dwRecvLimit 						= flowItem.m_nRecvLimit;
	m_dwSendLimit 						= flowItem.m_nSendLimit;
	//CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"%s", m_ProcessFlowItem.m_strProcPath);
	WinMod::CWinPath path(m_ProcessFlowItem.m_strProcPath);
    m_strProcessPath = path.GetPathWithoutUnicodePrefix();
	CString strPath = m_strProcessPath.Left(4);
	if (0 == strPath.CompareNoCase(L"\\??\\"))
	{
		m_strProcessPath = m_strProcessPath.Mid(4);
		//CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"HasInvalid pre %s", m_strProcessPath);
	}
    
    m_strProcessName 					= PathFindFileName(m_ProcessFlowItem.m_strProcPath);
    GetUserTrustModeString(m_ProcessFlowItem.m_nTrustMode, m_strTrust);

    m_ProcessID 						= m_ProcessFlowItem.m_nProcessID;

	CString strNameTerm = m_strProcessName;
	strNameTerm.MakeLower();
	if (strNameTerm == _T("system") || strNameTerm == _T("system idle"))
	{
		m_bIsShowOpenFileLink = FALSE;
		m_strProcessPath = _T("此程序为系统虚拟程序，没有路径");
	}

	if (m_strProcessPath == _T(""))
	{
		m_bIsShowOpenFileLink = FALSE;
		m_strProcessPath = _T("暂时未能获取此程序路径");
	}
}

KNetFlowMonListItemData::KNetFlowMonListItemData()
    : m_bIconExtracted(FALSE),
      m_hSmallIcon(NULL),
      m_ProcessID(0),
	  m_defaultIcon(NULL)
{
    m_strProcessName 					= L"";
    m_strTrust 							= L"";
    m_strSmallIcon 						= L"";
    m_strProcessPath 					= L"";
    m_strSendSpeedText 					= L"";
    m_strReceiveSpeedText 				= L"";
    m_strTotalReceiveText 				= L"";
    m_strTotalSendText 					= L"";
	m_strPID 							= L"";
}

bool KNetFlowMonListItemData::Match(KNetFlowMonListItemData* pItem)
{
    if(this->m_ProcessID == pItem->GetProcessId())
    {
        return true;
    }

    return false;
}

DWORD KNetFlowMonListItemData::GetProcessId()
{
    return m_ProcessID;
}

ATL::CString KNetFlowMonListItemData::GetProcessName()
{
	return m_strProcessName;
}

ATL::CString KNetFlowMonListItemData::GetTrust()
{
	return m_strTrust;
}

ULONGLONG KNetFlowMonListItemData::GetDownloadSpeed()
{
	return m_ProcessFlowItem.m_nFlux.nRecvSpeed;
}

ULONGLONG KNetFlowMonListItemData::GetUploadSpeed()
{
	return m_ProcessFlowItem.m_nFlux.nSendSpeed;
}

ULONGLONG KNetFlowMonListItemData::GetTotalDownload()
{
	return m_ProcessFlowItem.m_nFlux.nTotalRecv;
}	

ULONGLONG KNetFlowMonListItemData::GetTotalUpload()
{
	return m_ProcessFlowItem.m_nFlux.nTotalSend;
}

BOOL KNetFlowMonListItemData::_KillProcess( DWORD processId )
{
	HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION |
		PROCESS_TERMINATE,
		FALSE, processId );

	if (!hProcess)
	{
		netmon_log(L"Open process %d failed, now adjust process have debug privilege.",
			m_strProcessName);
		bool ret = KAccessControl::SetCurrentProcessPrivilege(SE_DEBUG_NAME, true);
		if (!ret)
		{
			netmon_log(L"SetCurrentProcessPrivilege Debug failed.ErrorCode:%d", 
				GetLastError());
			
			return FALSE;
		}

		hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION |
			PROCESS_TERMINATE,
			FALSE, processId );
	}

	if (hProcess)
	{
		BOOL ret = ::TerminateProcess(hProcess, 0);
		if (!ret)
		{
			netmon_log(L"TerminateProcess %s failed.ErrorCode:%d", 
				m_strProcessName,
				GetLastError());
		}

		return ret;
	}
	else
	{
		netmon_log(L"Open Process %s failed again.ErrorCode:%d", 
			m_strProcessName,
			GetLastError());
	}

	return FALSE;
}

CString KNetFlowMonListItemData::GetProcessDescription()
{	
	if (m_strProcessDescription.GetLength() == 0)
	{
		if (GetProcessId() == 0)
		{
			m_strProcessDescription = BkString::Get(Str_SystemIdleProcessDescription);
		}
		else if (GetProcessId() == 4)
		{
			m_strProcessDescription = BkString::Get(Str_SystemProcessDescription);
		}
		else
		{
			kis::KFileVersion fileVer(m_strProcessPath);
			m_strProcessDescription = fileVer.GetFileDescription();
			
			// 如果一个pe文件没有version资源则获取不到描述，
			// windows资源管理器默认在描述信息位置显示文件名
			// 我们也照此处理。
			if (m_strProcessDescription.GetLength() == 0)
			{
				m_strProcessDescription = m_strProcessName;
			}
		}
	}

	return m_strProcessDescription;
}

enumProcessType KNetFlowMonListItemData::_GetProcessType( KProcFluxItem processFlowItem )
{
	enumProcessType result = enumProcessTypeOther;

	if (_IsSystemProcess(processFlowItem.m_strProcPath) ||
		_IsSystemProcess(processFlowItem.m_nProcessID))
	{
		result = enumProcessTypeOS;
	}

	if (_IsKingsoftWeiShiProcess(processFlowItem.m_strProcPath))
	{
		result = enumProcessTypeKingsoft;
	}
	
	return result;
}

BOOL KNetFlowMonListItemData::KillCurrentSelProcess()
{
	CBkSafeMsgBox2 dlg;
	CString strTipInfo;
	UINT nType = 0;
	BOOL bIsNeedDelFile = FALSE;
	CString strKpfwProcess;
	switch(_GetProcessType(m_ProcessFlowItem))
	{
	case enumProcessTypeOS:

		strTipInfo.Format(BkString::Get(DefString8), GetProcessName().AllocSysString());
		nType = MB_OK | MB_ICONEXCLAMATION;
		dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), nType);
		break;

	case enumProcessTypeService:
	case enumProcessTypeOther:
	case enumProcessTypeKingsoft:
		strKpfwProcess = GetProcessName().AllocSysString();
		strKpfwProcess.MakeLower();
		if (strKpfwProcess == _T("ksafesvc.exe"))
		{
			strTipInfo.Format(BkString::Get(DefString32), GetProcessName().AllocSysString());
			nType = MB_OK | MB_ICONEXCLAMATION;
			dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), nType);
			break;
		}
		if (strKpfwProcess == _T("ksafetray.exe"))
		{
			strTipInfo.Format(BkString::Get(DefString59), GetProcessName().AllocSysString());
			nType = MB_OK | MB_ICONEXCLAMATION;
			dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), nType);
			break;
		}

		strTipInfo.Format(BkString::Get(DefString12), GetProcessName().AllocSysString());
		nType = MB_OKCANCEL | MB_ICONQUESTION;
		if (IDOK == dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), nType))
			bIsNeedDelFile = TRUE;

		break;
	}

	if (bIsNeedDelFile)
		_KillProcess(GetProcessId());

	return TRUE;
}



BOOL KNetFlowMonListItemData::_IsKingsoftWeiShiProcess( LPCWSTR szProcessPath )
{
	KSysApps kingsoftApps;
	kingsoftApps.AddWeishiPath();

	return kingsoftApps.Search(szProcessPath);
	
}

BOOL KNetFlowMonListItemData::_IsSystemProcess( DWORD pid )
{
	// 4 为Sytstem进程
	// 0 为System Idle Process进程
	return pid == 4 || pid == 0;
}

BOOL KNetFlowMonListItemData::_IsSystemProcess( LPCWSTR szProcessPath )
{
	KSysApps systemApps;
	systemApps.AddSystemPath();

	return systemApps.Search(szProcessPath);
}

BOOL KNetFlowMonListItemData::_IsNetMonEnabled()
{
	return KMainDlg::GetNetMonIsEnabled();
}

void KNetFlowMonListItemData::OpenDirector( BOOL bSelectFile /* = TRUE */)
{
	if (TRUE == PathFileExists(GetProcessFullPath()))
	{
		CString	strParam = L"";
		if (TRUE == bSelectFile)
		{
			strParam = _T("/select,\"") + GetProcessFullPath() + _T("\"");
		}
		else
		{
			strParam = _T("\"") + GetProcessFullPath() + _T("\"");
		}
		if (FALSE == strParam.IsEmpty())
			::ShellExecute( NULL, _T("open"), _T("explorer.exe"), strParam, NULL, SW_SHOW);
	}

}

void KNetFlowMonListItemData::OpenFileAttribute()
{
	if (TRUE == PathFileExists(GetProcessFullPath()))
	{
		SHELLEXECUTEINFO FileInfo = {0};
		FileInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		//FileInfo.hwnd = GetViewHWND();
		FileInfo.lpFile = GetProcessFullPath();
		FileInfo.lpVerb = L"properties";
		FileInfo.fMask = SEE_MASK_INVOKEIDLIST;
		FileInfo.nShow = SW_SHOW;
		::ShellExecuteEx(&FileInfo);
	}
}

COLORREF KNetFlowMonListItemData::GetLevelColor()
{
	COLORREF result = CLR_INVALID;

	switch(m_ProcessFlowItem.m_nTrustMode)
	{
	case enumTM_UnVerfiy:
	case enumTM_Unknown:
		result = RGB(0, 95, 203);
		break;

	case enumTM_Safe:
		result = RGB(45, 202, 9);
		break;

	case enumTM_Danger:
		result = RGB(200, 0, 0);
		break;

	default: 
		result = RGB(0, 95, 203);
		break;
	}

	return result;
}
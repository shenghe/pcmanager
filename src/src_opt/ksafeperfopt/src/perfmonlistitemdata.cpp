#include "stdafx.h"
#include "bkres/bkres.h"
#include "runoptimize/perfmondef.h"
#include "perfmonlistitemdata.h"
#include "kcompare.h"
#include "kpfw/kpfw_def.h"
#include "kpfw/netflowformat.h"
#include "src/beikesafemsgbox.h"
#include "kis_filever.h"
#include "sysmprocess.h"
#include "kpfw/msg_logger.h"
#include "common/kaccesscontrol.h"
#include "kmaindlg.h"
#include "winmod/winwow64.h"

#define BUFSIZE 512

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

void KPerfMonListItemData::SetProcessPath(LPCTSTR szPath)
{
    m_strProcessPath = szPath;
}

KPerfMonListItemData::~KPerfMonListItemData()
{

}

void KPerfMonListItemData::SetTrust(LPCTSTR szTrust)
{
    m_strTrust = szTrust;
}

KPerfMonListItemData::KPerfMonListItemData(KProcessPerfData* procInfoItem)
	: m_bIconExtracted(FALSE)
	, m_hSmallIcon(NULL)
	, m_nProcessID(0)
	, m_defaultIcon(NULL)
	, m_bIsShowOpenFileLink(TRUE)
{
	m_strSmallIcon 						= L"";
	m_strPID 							= L"";
	m_pProcInfoItem 					= procInfoItem;

	m_nProcessID = m_pProcInfoItem->GetProcessID();
	m_strProcessPath = m_pProcInfoItem->GetProcessPath();
	m_strProcessName = m_pProcInfoItem->GetProcessName();
	m_fCpuUsage	 = m_pProcInfoItem->GetCpuUsage();
	m_uMemUsage  = m_pProcInfoItem->GetMemUsage();
	m_uIOSpeed	 = m_pProcInfoItem->GetIOSpeed();
	GetUserTrustModeString(m_pProcInfoItem->GetProcessTrustMode(), m_strTrust);
}

KPerfMonListItemData::KPerfMonListItemData()
		: m_bIconExtracted(FALSE)
		, m_hSmallIcon(NULL)
		, m_nProcessID(0)
		, m_defaultIcon(NULL)
		, m_bIsShowOpenFileLink(TRUE)
		, m_pProcInfoItem(NULL)
{
	
}

bool KPerfMonListItemData::Match(KPerfMonListItemData* pItem)
{
    if(this->m_nProcessID == pItem->GetProcessId())
    {
        return true;
    }

    return false;
}

DWORD KPerfMonListItemData::GetProcessId()
{
    return m_nProcessID;
}

ATL::CString KPerfMonListItemData::GetProcessName()
{
	CString strNameTerm = m_strProcessName;
	strNameTerm.MakeLower();
	if ( ( strNameTerm != _T("system") )
		&& ( strNameTerm != _T("system idle") )
		&& ( strNameTerm != _T("idle") )
		&& ( strNameTerm != _T("")))
	{
		if (m_strProcessPath.CompareNoCase(BkString::Get(PERF_MONITOR_STR_516)) == 0
			|| m_strProcessPath.CompareNoCase(BkString::Get(PERF_MONITOR_STR_517)) == 0
			|| m_strProcessPath.CompareNoCase(_T("")) == 0
			|| PathFileExists(m_strProcessPath) == FALSE
			)
		{
			return m_strProcessName;
		}
		else
		{
			return PathFindFileName(m_strProcessPath);
		}
	}

	if (m_nProcessID == 0)
	{
		m_strProcessName = _T("System Idle");
	}

	return m_strProcessName;
}

BOOL KPerfMonListItemData::_IsWow64( void )
{
	BOOL bIsWow64 = TRUE;
	if (!WinMod::CWinModule_kernel32::IsWow64Process(::GetCurrentProcess(), &bIsWow64))
		bIsWow64 = FALSE;

	return bIsWow64;
}

ATL::CString KPerfMonListItemData::GetProcessFullPath()
{/*
	if (m_strProcessPath.CompareNoCase(BkString::Get(PERF_MONITOR_STR_516)) == 0
		|| m_strProcessPath.CompareNoCase(BkString::Get(PERF_MONITOR_STR_517)) == 0
		|| m_strProcessPath.CompareNoCase(_T("")) == 0
		)
	{
		if (_IsWow64() == FALSE)
		{
			TCHAR szPathName[MAX_PATH] = {0};
			bool bRet = KAccessControl::SetCurrentProcessPrivilege(SE_DEBUG_NAME, true);
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_nProcessID);
			if (!hProcess)
				hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_nProcessID);

			if (hProcess)
			{    
				TCHAR szBuf[MAX_PATH] = {0};
				if ( 0 != GetModuleFileNameEx(hProcess, NULL, szBuf, MAX_PATH) )
				{
					if (FixDosPathToNormalPath(szBuf, szPathName) == 2)
						m_strProcessPath = szPathName;
					else
						m_strProcessPath = szBuf;
				}
			}
			CloseHandle(hProcess);
		}
	}
*/
	return m_strProcessPath;
}

ATL::CString KPerfMonListItemData::GetTrust()
{
	return m_strTrust;
}

ATL::CString KPerfMonListItemData::GetProcessCpuUsage()
{
	CString strCpuUsage;
	if (m_fCpuUsage >= 0.0 && m_fCpuUsage <= 100.0)
		strCpuUsage.Format(_T("%d%%"), int(m_fCpuUsage));
	
	return strCpuUsage;
}

int KPerfMonListItemData::GetProcessCpuUsage_INT()
{
	return (int)m_fCpuUsage;
}

ATL::CString KPerfMonListItemData::GetProcessMemUsage()
{
	CString strMemUsage;
	NetFlowToString2(m_uMemUsage, strMemUsage);
	return strMemUsage;
}

ULONGLONG KPerfMonListItemData::GetProcessMemUsage_ULL()
{
	return m_uMemUsage;
}

ATL::CString KPerfMonListItemData::GetProcessIOSpeed()
{
	CString strIOPerSecond;
	strIOPerSecond.Format(_T("%I64u次/秒"), m_uIOSpeed);
	return strIOPerSecond;
}

ULONGLONG KPerfMonListItemData::GetProcessIOSpeed_ULL()
{
	return m_uIOSpeed;
}

BOOL KPerfMonListItemData::_KillProcess( DWORD processId )
{
	HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION |
		PROCESS_TERMINATE,
		FALSE, processId );

	if (!hProcess)
	{
// 		netmon_log(L"Open process %d failed, now adjust process have debug privilege.",
// 			m_strProcessName);
		bool ret = KAccessControl::SetCurrentProcessPrivilege(SE_DEBUG_NAME, true);
		if (!ret)
		{
// 			netmon_log(L"SetCurrentProcessPrivilege Debug failed.ErrorCode:%d", 
// 				GetLastError());

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
	CloseHandle(hProcess);
	return FALSE;
}

CString KPerfMonListItemData::GetProcessDescription()
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

enumProcessType KPerfMonListItemData::_GetProcessType()
{
	enumProcessType result = enumProcessTypeOther;

	if (_IsSystemProcess(GetProcessFullPath()) ||
		_IsSystemProcess(m_nProcessID))
	{
		result = enumProcessTypeOS;
	}

	if (_IsKingsoftWeiShiProcess(GetProcessFullPath()))
	{
		result = enumProcessTypeKingsoft;
	}
	
	return result;
}

CString KPerfMonListItemData::FormatLongExeName(const CString& strIn)
{
	DWORD dwLength = strIn.GetLength();
	if (dwLength > 61)
	{
		CString strOut = strIn.Left(61);
		strOut.Append(_T("..."));
		return strOut;
	}
	else
		return strIn;
}

BOOL KPerfMonListItemData::KillCurrentSelProcess()
{
	CBkSafeMsgBox2 dlg;
	CString strTipInfo;
	UINT nType = 0;
	BOOL bIsNeedDelFile = FALSE;
	CString strKpfwProcess;
	CString strProcName = FormatLongExeName(GetProcessName());
	switch(_GetProcessType())
	{
	case enumProcessTypeOS:

		strTipInfo.Format(BkString::Get(DefString8), strProcName);
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
			strTipInfo.Format(BkString::Get(DefString34), GetProcessName().AllocSysString());
			nType = MB_OK | MB_ICONEXCLAMATION;
			dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), nType);
			break;
		}

		strTipInfo.Format(BkString::Get(DefString12), strProcName);
		nType = MB_OKCANCEL | MB_ICONQUESTION;
		if (IDOK == dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), nType))
			bIsNeedDelFile = TRUE;

		break;
	}

	if (bIsNeedDelFile)
		_KillProcess(GetProcessId());

	return TRUE;
}



BOOL KPerfMonListItemData::_IsKingsoftWeiShiProcess( LPCWSTR szProcessPath )
{
	KSysApps kingsoftApps;
	kingsoftApps.AddWeishiPath();

	return kingsoftApps.Search(szProcessPath);
	
}

BOOL KPerfMonListItemData::_IsSystemProcess( DWORD pid )
{
	// 4 为Sytstem进程
	// 0 为System Idle Process进程
	return pid == 4 || pid == 0;
}

BOOL KPerfMonListItemData::_IsSystemProcess( LPCWSTR szProcessPath )
{
	KSysApps systemApps;
	systemApps.AddSystemPath();

	return systemApps.Search(szProcessPath);
}

void KPerfMonListItemData::OpenDirector( BOOL bSelectFile /* = TRUE */)
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

void KPerfMonListItemData::OpenFileAttribute()
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

COLORREF KPerfMonListItemData::GetLevelColor()
{
	COLORREF result = CLR_INVALID;

	switch(m_pProcInfoItem->GetProcessTrustMode())
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
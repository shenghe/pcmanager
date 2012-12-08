#include "StdAfx.h"
#include "examcheckitemsex.h"
#include <shlobj.h>
#include "lm.h"
#include "shellapi.h"
#pragma comment(lib, "Netapi32.lib")
#include "iefix/RegisterOperate.h"
using namespace RegisterOperate;


CExamCheckItemsEx::CExamCheckItemsEx(void)
{
}

CExamCheckItemsEx::~CExamCheckItemsEx(void)
{
}

//GUEST账户
int CExamCheckItemsEx::GuestUserIsEnable()
{
	int nRet = -1;
	
	LPUSER_INFO_3 pBuf = NULL;
	LPUSER_INFO_3 pTmpBuf;
	NET_API_STATUS nStatus = ERROR_MORE_DATA;
	DWORD dwLevel = 3;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	
	do
	{
		try
		{
			nStatus = NetUserEnum(NULL,
				dwLevel,
				FILTER_NORMAL_ACCOUNT, // global users
				(LPBYTE*)&pBuf,
				dwPrefMaxLen,
				&dwEntriesRead,
				&dwTotalEntries,
				&dwResumeHandle);

			if ((nStatus != NERR_Success) && (nStatus != ERROR_MORE_DATA))
				continue;

			if ((pTmpBuf = pBuf) == NULL)
				continue;

			for (int i = 0; (i < dwEntriesRead); i++)
			{
				if (pTmpBuf == NULL)
					break;
				if(StrCmpI(pTmpBuf->usri3_name , TEXT("Guest")) == 0 && pTmpBuf->usri3_priv == USER_PRIV_GUEST ) 
				{

					if(!( pTmpBuf->usri3_flags & UF_ACCOUNTDISABLE ) )
					{
						nRet = 0;
						break ;
					}	
				}
				pTmpBuf++;
			}

			if (pBuf != NULL)
			{
				NetApiBufferFree(pBuf);
				pBuf = NULL;
			}
		}
		catch (...)
		{
			nRet = -1;
			break;
		}
	}while (nStatus == ERROR_MORE_DATA); 
	
	if (pBuf != NULL)
		NetApiBufferFree(pBuf);

	return nRet;
}
int CExamCheckItemsEx::SetGuestUserDisable()
{
	int nRet = -1;

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi={0};
	TCHAR pszShutDownGuestCmd[] = {TEXT("net user guest /active:no")};

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);
	TCHAR pszCmdLine[MAX_PATH] = {0};
	_tcsncpy_s(pszCmdLine, pszShutDownGuestCmd, sizeof(pszShutDownGuestCmd));

	if (TRUE == CreateProcess(NULL, pszCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		nRet = 0;

	return nRet;
}
int CExamCheckItemsEx::StartupGuest()
{
	int nRet = -1;
	TCHAR pszStartGuestCmd[] = {TEXT("net user guest /active:yes")};
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi={0};

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);
	TCHAR pszCmdLine[MAX_PATH] = {0};
	_tcsncpy_s(pszCmdLine, pszStartGuestCmd, sizeof(pszStartGuestCmd));

	if (TRUE == CreateProcess(NULL, pszCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		nRet = 0;

	return nRet;
}
//共享资源
int CExamCheckItemsEx::IsHaveShareResource()
{
	int nRet = -1;

	PSHARE_INFO_502 BufPtr,p;
	NET_API_STATUS res;
	DWORD er=0,tr=0,resume=0, i;
	m_arrResource.RemoveAll();

	do 
	{
		res = NetShareEnum( NULL , 502, (LPBYTE *) &BufPtr, -1, &er, &tr, &resume);
		if(res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
		{
			p=BufPtr;
			for(i=1; i <= er; i++)
			{
				switch( p->shi502_type ) 
				{
				case STYPE_DISKTREE:
					{
						SHARE_RESOURCES res;
						res.pszName = p->shi502_netname;
						res.pszPath = p->shi502_path;

						if (0 == StrCmp(TEXT("print$"), p->shi502_netname))
						{
							p++;
							continue;
						}
						m_arrResource.Add(res);
						CString strMsg;
						strMsg.Format(TEXT("type=%d Name = %s"), p->shi502_type, p->shi502_netname);
						OutputDebugString(strMsg);
					}
					break ;
				default:
					p ++ ;
					continue ;
					break ;
				}
				p++;
				nRet = 0;
			}
			NetApiBufferFree(BufPtr);
		}
	}
	while (res==ERROR_MORE_DATA); // end do

	return nRet;
}
int CExamCheckItemsEx::RemoveAllShareResource()
{
	int nRet = -1;
	int nCount = m_arrResource.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		CString strRes = m_arrResource[i].pszName;
		if (TRUE == strRes.IsEmpty())
			continue;
		if(NERR_Success != NetShareDel(NULL , strRes.GetBuffer(-1), 0)) 
		{
			nRet = -1;
			continue;
		}
		nRet = 0;
		strRes.ReleaseBuffer(-1);
	}
	if (0 == nRet)
		m_arrResource.RemoveAll();
	return nRet;
}
//获取资源个数
int CExamCheckItemsEx::GetShareResourceCount()
{
	return m_arrResource.GetSize();
}
//枚举每一个资源
BKSafeExamItemEx::SHARE_RESOURCES CExamCheckItemsEx::EnumAnResources(int nIndex)
{
	CString strValue(TEXT(""));
	SHARE_RESOURCES res;
	if (nIndex < 0 || nIndex >= GetShareResourceCount())
		return res;

	return m_arrResource[nIndex];
}

int CExamCheckItemsEx::SetItemShareResourceSafe(LPCTSTR pszRes)
{
	int nRet = -1;
	if (NULL == pszRes)
		return nRet;
	int nCount = m_arrResource.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		CString strRes = m_arrResource[i].pszPath;
		if (TRUE == strRes.IsEmpty())
			continue;
		if (0 == strRes.CompareNoCase(pszRes))
		{
			m_arrResource.RemoveAt(i);
			nRet = 0;
			break;
		}
	}

	return nRet;
}

int CExamCheckItemsEx::SetItemShareResourceUnSafe(LPCTSTR pszRes)
{
	int nRet = -1;
	if (NULL == pszRes)
		return nRet;
	SHARE_RESOURCES res;
	res.pszPath = pszRes;
	res.pszName = pszRes;
	m_arrResource.Add(res);
	nRet = 0;

	return nRet;
}

int CExamCheckItemsEx::RemoveShareResource(LPCTSTR pszResource)
{
	int nRet = -1;
	if (NULL == pszResource)
	{//默认不提供需要清除的共享资源就直接把所有的都删除掉
		nRet = RemoveAllShareResource();
		return nRet;
	}
	//////////////////////////////////////////////////////////////////////////
	int nCount = m_arrResource.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		CString strRes = m_arrResource[i].pszName;
		if (TRUE == strRes.IsEmpty())
			continue;
		if (0 == strRes.CompareNoCase(pszResource))
		{
			LPWSTR pwszRes = const_cast<LPWSTR>(strRes.GetBuffer(-1));
			strRes.ReleaseBuffer(-1);
			if(NULL == pwszRes || NERR_Success != NetShareDel(NULL , pwszRes, 0)) 
			{
				nRet = -1;
				break;
			}
			m_arrResource.RemoveAt(i);
			nRet = 0;
			break;
		}
	}
	return nRet;

}
//远程桌面,1表示关闭远程桌面服务
int CExamCheckItemsEx::RemoteDesktopIsEnable()
{
	int nRet = -1;
	SERVICE_STATUS svrStatus;
	svrStatus.dwCurrentState = SERVICE_RUNNING;
	svrStatus = _GetSrvStatus(L"TermService");
	if(svrStatus.dwCurrentState == SERVICE_STOPPED)
		return nRet;

	DWORD dwValue = 1 ;
	DWORD dwSize = sizeof(DWORD) ;
	DWORD dwType = REG_DWORD ;
	SHGetValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server", 
		L"fDenyTSConnections", &dwType , &dwValue , &dwSize ) ;

	if( dwValue == 0 )
		nRet = 0;

	return nRet;
}
int CExamCheckItemsEx::SetRemoteDesktopDisable()
{
	int nRet = -1;
	DWORD dwValue = 1 ;
	SHDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services"));
	if(ERROR_SUCCESS == SHSetValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server" , 
									L"fDenyTSConnections",  REG_DWORD, &dwValue, sizeof(DWORD))) 
	{
		nRet = 0;
	}

	return nRet;
}

int CExamCheckItemsEx::StartupRemoteDesktop()
{
	int nRet = -1;
	DWORD dwValue = 0 ;
	HKEY hNewKey = NULL;
	SERVICE_STATUS svrStatus;
	//启动远程桌面服务
	TCHAR pszSrvName[] = L"TermService";
	svrStatus = _GetSrvStatus(pszSrvName);
	if(svrStatus.dwCurrentState == SERVICE_STOPPED)
		_StartSrv(pszSrvName);

	
	CRegisterOperate::GetRegOperatePtr()->CreateAnKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services"), hNewKey);
	if (NULL == hNewKey)
		return nRet;
	if(ERROR_SUCCESS == SHSetValue(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server" , 
		L"fDenyTSConnections",  REG_DWORD, &dwValue, sizeof(DWORD))) 
	{
		nRet = 0;
	}

	return nRet;
}

//组策略
int CExamCheckItemsEx::GpeditIsDisable()
{
	int nRet = -1;
	DWORD dwValue = 0;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(DWORD);
	if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, TEXT("Software\\Policies\\Microsoft\\MMC"), TEXT("RestrictToPermittedSnapins"), &dwType , &dwValue, &dwSize))
	{
		if (1 == dwValue)
			nRet = 0;
	}
	return nRet;
}
int CExamCheckItemsEx::SetGpeditEnable()
{
	int nRet = -1;
	DWORD dwValue = 0;
	if (ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Policies\\Microsoft\\MMC"), TEXT("RestrictToPermittedSnapins"), REG_DWORD, &dwValue, sizeof(DWORD)))
	{
			nRet = 0;
	}
	return nRet;
}


SERVICE_STATUS CExamCheckItemsEx::_GetSrvStatus(LPCTSTR pszSrvName)
{
	SERVICE_STATUS status = {0};
	try
	{
		SC_HANDLE schSCManager;
		SC_HANDLE schService;
		LPQUERY_SERVICE_CONFIG lpsc; 
		DWORD dwBytesNeeded, cbBufSize, dwError; 

		status.dwCurrentState = SERVICE_RUNNING;
		// Get a handle to the SCM database. 
		schSCManager = OpenSCManager( 
			NULL,                    // local computer
			NULL,                    // ServicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (NULL == schSCManager) 
		{
			return status;
		}

		// Get a handle to the service.
		schService = OpenService( 
			schSCManager,            // SCM database 
			pszSrvName,               // name of service 
			SERVICE_CHANGE_CONFIG);  // need change config access 

		if (schService == NULL)
		{ 
			CloseServiceHandle(schSCManager);
			return status;
		}
		//检测服务状态是禁用还是手动或者自动
		if( !QueryServiceConfig( 
			schService, 
			NULL, 
			0, 
			&dwBytesNeeded))
		{
			dwError = GetLastError();
			if( ERROR_INSUFFICIENT_BUFFER == dwError )
			{
				cbBufSize = dwBytesNeeded;
				lpsc = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, cbBufSize);
			}
			else
			{
				return status;
			}
		}

		if( !QueryServiceConfig( 
			schService, 
			lpsc, 
			cbBufSize, 
			&dwBytesNeeded) ) 
		{
			return status;
		}
		if (NULL != lpsc && SERVICE_DISABLED == lpsc->dwStartType)
		{
			status.dwCurrentState = SERVICE_STOPPED;
			LocalFree(lpsc); 
			return status;
		}
		if (NULL != lpsc)
			LocalFree(lpsc); 
		//
		if (FALSE == QueryServiceStatus(schService, &status))
		{
			status.dwCurrentState = SERVICE_RUNNING;
		}

	}
	catch (...)
	{
	}

	return status;
}

BOOL CExamCheckItemsEx::_StartSrv(LPCTSTR pszSrvName)
{
	BOOL bIsSuccess = FALSE;

	if (NULL == pszSrvName)
		return bIsSuccess;

	DWORD lastError = 0;
	SERVICE_STATUS status = {0};
	try
	{
		SC_HANDLE schSCManager;
		SC_HANDLE schService;

		status.dwCurrentState = SERVICE_RUNNING;
		// Get a handle to the SCM database. 
		schSCManager = OpenSCManager( 
			NULL,                    // local computer
			NULL,                    // ServicesActive database 
			SC_MANAGER_ALL_ACCESS);  // full access rights 

		if (NULL == schSCManager) 
		{
			return bIsSuccess;
		}

		// Get a handle to the service.
		schService = OpenService( 
			schSCManager,            // SCM database 
			pszSrvName,          // name of service 
			SERVICE_START);           // need stop access 

		if (schService == NULL)
		{ 
			CloseServiceHandle(schSCManager);
			return bIsSuccess;
		}

		bIsSuccess = StartService(
			schService, 
			0,
			NULL);

		if (!bIsSuccess)
		{
			if (ERROR_SERVICE_ALREADY_RUNNING == GetLastError())
			{
				bIsSuccess = TRUE;
			}
		}
	}
	catch (...)
	{
	}

	return bIsSuccess;
}


int CExamCheckItemsEx::ShowHideFileIsEnable()
{
	DWORD dwValue = 0;
	int nRet = -1;
	TCHAR pszValue[MAX_PATH] = {0};
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	SHGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL",L"CheckedValue", &dwType, &dwValue, &dwSize);

	if(1 != dwValue)
	{
		nRet = 0;
		return nRet;
	} 
	
	dwSize = sizeof(pszValue);
	dwType = REG_SZ;
	SHGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL", L"Type", &dwType, pszValue, &dwSize);
	if(StrCmpI(pszValue, L"radio"))
	{
		nRet = 0;
		return nRet;
	}
	dwValue = 0;
	dwSize = sizeof(DWORD);
	dwType = REG_DWORD;
	SHGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\NOHIDDEN", L"CheckedValue", &dwType, &dwValue, &dwSize);
	if(dwValue != 2)
	{
		nRet = 0;
		return nRet;
	}
	ZeroMemory(pszValue, sizeof(pszValue));
	dwSize = sizeof(pszValue);
	dwType = REG_SZ;
	SHGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\NOHIDDEN", L"Type", &dwType, pszValue, &dwSize);
	if(StrCmpI(pszValue, L"radio"))
	{
		nRet = 0;
		return nRet;
	}

	return nRet;
}

int CExamCheckItemsEx::ShowHideFileEnable()
{
	DWORD dwValue = 2;
	DWORD dwSize = sizeof(DWORD);

	SHSetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\NOHIDDEN",\
				L"CheckedValue", REG_DWORD, &dwValue, dwSize);
	dwValue = 1;
	SHSetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL",\
				L"CheckedValue", REG_DWORD, &dwValue, dwSize);

	SHSetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL",\
				L"Type", REG_SZ, L"radio", sizeof(L"radio"));
	SHSetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\NOHIDDEN",\
				L"Type", REG_SZ, L"radio", sizeof(L"radio"));

	return 0;

}

int CExamCheckItemsEx::DriverOrDirectoryOpenIsEnable()
{
	int nRet = 0;
	TCHAR pszValue[32] = {0};
	DWORD dwSize = sizeof(pszValue);
	//driver
	//HKEY_CLASSES_ROOT\Directory\shell
	DWORD dwType = REG_SZ;
	SHGetValue(HKEY_CLASSES_ROOT, L"Directory\\shell", L"", &dwType, pszValue, &dwSize);
	if (0 != StrCmpI(pszValue, L"none"))
		return nRet;

	//directory
	dwSize = sizeof(pszValue);
	ZeroMemory(pszValue, dwSize);
	SHGetValue(HKEY_CLASSES_ROOT, L"Drive\\shell", L"", &dwType, pszValue, &dwSize);
	if (0 != StrCmpI(pszValue, L"none"))
		return nRet;

	nRet = 1;
	
	return nRet;
}
int CExamCheckItemsEx::DriverOrDirectoryOpenEnable()
{
	int nRet = -1;
	TCHAR pszValue[] = L"none";
	DWORD dwSize = sizeof(pszValue);
	//driver
	//HKEY_CLASSES_ROOT\Directory\shell
	SHSetValue(HKEY_CLASSES_ROOT, L"Directory\\shell",\
		L"", REG_SZ, pszValue, dwSize);

	//directory
	SHSetValue(HKEY_CLASSES_ROOT, L"Drive\\shell",\
		L"", REG_SZ, pszValue, dwSize);

	nRet = 0;

	return nRet;
}
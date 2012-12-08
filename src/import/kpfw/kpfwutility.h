////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : kpfwutility.h
//      Version   : 1.0
//      Comment   : netmon工具实现模块
//      
//      Create at : 2008-11-12
//      Create by : chenguicheng
//      
///////////////////////////////////////////////////////////////////////////////

#include <shellapi.h>
#include <atlpath.h>
#include "kis/kpfw/regkeystring.h"


class KpfwUtility
{
public:

	KpfwUtility(){};
	~KpfwUtility(){};


	BOOL GetKpfwAutoRun( BOOL *pValue )
	{
		CRegKey regKey;
		LRESULT lRes = 0;
		TCHAR tszPath[MAX_PATH * 2] = {0};
		ULONG uLength = MAX_PATH * 2;
		BOOL	bRetCode = FALSE;

		if( regKey.Open( HKEY_LOCAL_MACHINE, KCOMMON_AUTO_RUN_REY_PATH) != ERROR_SUCCESS )
			return FALSE;

		lRes = regKey.QueryStringValue( KCOMMON_AUTO_RUN_REY_VALUE, tszPath, &uLength );
		
		if(lRes == ERROR_FILE_NOT_FOUND)
		{
			*pValue = FALSE;
			bRetCode = TRUE;
		}
		else if( lRes == ERROR_SUCCESS )
		{
			*pValue = TRUE;
			bRetCode = TRUE;
		}
		else
			bRetCode = FALSE;

		ASSERT( bRetCode );

		return bRetCode;
	}

	BOOL KpfwAutoRun(BOOL bAdd, ATL::CString strName)
	{
		CRegKey regKey;
		LRESULT lRes = 0;

		if( regKey.Open( HKEY_LOCAL_MACHINE, KCOMMON_AUTO_RUN_REY_PATH) != ERROR_SUCCESS )
			return FALSE;

		if ( !bAdd )
		{
			lRes = regKey.DeleteValue( KCOMMON_AUTO_RUN_REY_VALUE );
		}
		else
		{
			ATL::CString strAutoRun = L"\"" + strName + L"\"" + L" -autorun";
			lRes = regKey.SetStringValue( KCOMMON_AUTO_RUN_REY_VALUE, strAutoRun, REG_SZ );
			ASSERT(lRes == ERROR_SUCCESS );
		}

		return lRes == ERROR_SUCCESS;
	}


	HRESULT KillProcess(ULONG uProcessId )
	{
		HANDLE hProcess = NULL;
		TOKEN_PRIVILEGES	tkp;
		BOOL bRet = FALSE;
		HANDLE hToken = NULL;
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, (DWORD)uProcessId );
		if ( hProcess )
		{
			bRet = OpenProcessToken( GetCurrentProcess(), 
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_ADJUST_SESSIONID, 
				&hToken);

			bRet = LookupPrivilegeValue(NULL,
				SE_DEBUG_NAME, 
				&tkp.Privileges[0].Luid); 

			tkp.PrivilegeCount = 1; 
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			bRet = AdjustTokenPrivileges( hToken, 
				FALSE, &tkp, 0, 
				(PTOKEN_PRIVILEGES)NULL, 0); 

			TerminateProcess( hProcess, 1 );
		}

		if (hProcess)
			CloseHandle(hProcess);

		if ( hToken )
			CloseHandle( hToken );

		return TRUE;

	}


	HRESULT StartProcess( ATL::CString& strFile, ATL::CString& strArgv )
	{
		HINSTANCE hInstance = NULL;
		HRESULT hr = E_FAIL;

		hInstance = ShellExecute( NULL, 
			TEXT("open"), 
			strFile, 
			strArgv, 
			NULL, 
			SW_SHOWNORMAL);

		HINSTANCE hInstance2 = (HINSTANCE)32;
		if ( hInstance < hInstance2 )
		{
			ASSERT( FALSE );
			return hr;
		}

		hr = S_OK;
		return hr;
	}
};



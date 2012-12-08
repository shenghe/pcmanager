#include "StdAfx.h"
#include "bkservicecallImp.h"
#include "winmod/winpath.h"
#include "winmod/winosver.h"
#include "winmod/winprocessenumerator.h"
#include "common/procprivilege.h"
#include "common/utility.h"
#include "signverifer.h"

#ifdef _DEBUG
#define ENABLE_SIGN_CHECK
#else
#define ENABLE_SIGN_CHECK
#endif

CBkServiceCallImp::CBkServiceCallImp(void) :
    m_hToken( NULL )
{
}

CBkServiceCallImp::~CBkServiceCallImp(void)
{
}

BOOL    CBkServiceCallImp::VerifyProcess( DWORD dwProcId )
{
    BOOL    bRet = FALSE;
    DWORD   dwRet = 0;
    HRESULT hr = S_OK;
    HANDLE  hProcess = NULL;

    hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        dwProcId 
        );
    if ( hProcess )
    {
        TCHAR   wszBuf[ MAX_PATH + 1 ] = { 0 };

        dwRet = GetModuleFileNameEx( hProcess, NULL, wszBuf, MAX_PATH );
        if ( dwRet )
        {
            CString             instpath;
            WinMod::CWinPath    path;

            path.m_strPath = wszBuf;

            WinMod::CWinPathApi::ExpandLongPathName( path.m_strPath );

            hr = CAppPath::Instance().GetLeidianAppPath( instpath );
            if ( SUCCEEDED( hr ) )
            {
                path.RemoveFileSpec();

                path.m_strPath.MakeLower();

                instpath.MakeLower();

                if ( wcsstr( path, instpath ) )
                {
                    bRet = TRUE;
                }
            }
        }
    }

    if ( hProcess )
    {
        CloseHandle( hProcess );
        hProcess = NULL;
    }

    return bRet;
}

BOOL    CBkServiceCallImp::VerifyExePath( LPCWSTR lpwszExePath )
{
    BOOL    bRet = FALSE;
    HRESULT hr = S_OK;
    CString instpath;
    WinMod::CWinPath    path;

    path.m_strPath = lpwszExePath;

    bRet = path.IsExisting();
    if ( !bRet )
    {
        return bRet;
    }

    hr = CAppPath::Instance().GetLeidianAppPath( instpath );
    if ( SUCCEEDED( hr ) )
    {
        path.RemoveFileSpec();

        path.m_strPath.MakeLower();
        instpath.MakeLower();

        if ( wcsstr( path, instpath ) )
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

HRESULT CBkServiceCallImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    // 检测是否为可信的进程
    if ( !VerifyProcess( dwProcId ) )
    {
        return E_ACCESSDENIED;
    }

    if ( NULL == m_hToken )
    {
        hr = WinMod::CWinProcessApi::ObtainImpersonateToken( dwProcId, m_hToken );
        if ( FAILED( hr ) )
        {
            m_hToken = NULL;
        }
    }
    return hr;
}

HRESULT CBkServiceCallImp::Uninitialize()
{
    if ( m_hToken )
    {
        CloseHandle( m_hToken );
        m_hToken = NULL;
    }
    return S_OK;
}

HRESULT CBkServiceCallImp::Execute( 
    const CString& strExePath, 
    const CString& strCmdline, 
    BOOL bShow
    )
{
    HRESULT hr = S_OK;
    PROCESS_INFORMATION pi;
    STARTUPINFO         si;
    CProcPrivilege      pri;
    CString strCmdLineDuplicate;
    LPVOID pvEnvironment = NULL;
	BOOL bRet = FALSE;

//    if ( !VerifyExePath( strExePath ) )
//    {
//        hr = E_ACCESSDENIED;
//        goto Exit0;
//    }
#ifdef ENABLE_SIGN_CHECK
    hr = CSignVerifer::Instance().VerifyFile( strExePath );
    if ( FAILED ( hr ) )
    {
        goto Exit0;
    }
#endif
    

    if ( NULL == m_hToken )
    {
        hr = E_HANDLE;
        goto Exit0;
    }

    memset( &si, 0, sizeof( si ) );
    memset( &pi, 0, sizeof( pi ) );

    si.cb = sizeof( STARTUPINFO );
    si.lpDesktop = L"winsta0\\default";

    hr = pri.SetPri( TRUE, SE_ASSIGNPRIMARYTOKEN_NAME );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = pri.SetPri( TRUE, SE_INCREASE_QUOTA_NAME );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = ::CreateEnvironmentBlock( &pvEnvironment, m_hToken, FALSE );
    if ( FAILED( hr ) )
    {
        pvEnvironment = NULL;
    }

    if ( strCmdline == L"" )
    {
        strCmdLineDuplicate = strExePath;
    }
    else
    {
        strCmdLineDuplicate.Format(L"\"%s\" %s", strExePath, strCmdline);
    }

    bRet = CreateProcessAsUser( 
        m_hToken, 
        NULL, 
        ( LPWSTR )( LPCWSTR )strCmdLineDuplicate, 
        NULL, 
        NULL, 
        FALSE, 
        NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
        pvEnvironment,
        NULL,
        &si, 
        &pi
        );
    if ( !bRet )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }
Exit0:
    if ( pvEnvironment )
    {
        ::DestroyEnvironmentBlock(pvEnvironment);
        pvEnvironment = NULL;
    }

    if ( bRet )
    {
        if ( pi.hThread )
        {
            CloseHandle( pi.hThread );
        }

        if ( pi.hProcess )
        {
            CloseHandle( pi.hProcess );
        }
    }

    return hr;
}
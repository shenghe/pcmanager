// InstallExt.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "InstallExt.h"
#include <atlpath.h>
#include <shellapi.h>
#include <atlbase.h>
#include <Winbase.h>


InstallExt::InstallExt() //:
    //m_nWaitDo(em_none),
    //m_bShowWarning(FALSE)
{
    //m_dwCurUserPolicy     = 1;
    //m_bufferLocMPolicy[0] = 1;

    //m_bExist = FALSE;
}

 

DWORD WINAPI ThreadFunc(LPVOID /*lp*/)
{
    
    LPCTSTR pszWindowTitle[] = 
    {
        _T("硬件安装"),
        _T("Hardware Installation"),
        _T("没有找到数字签名"),
        _T("Digital Signature Not Found")
    };

    UINT ControlID[] = { 0x14B7, 0x14B7, 0x14B7, 0x14B7 };

    while ( true )
    {   
        for (int i = 0; i < sizeof(pszWindowTitle) / sizeof(LPCTSTR); i++)
        {
            HWND hWnd = FindWindow( _T("#32770"), pszWindowTitle[i] );

            if (hWnd != NULL)
            {
                SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(ControlID[i], BN_CLICKED), NULL);
            }
        }        

        Sleep(10);
    }

    return 0;
}


HRESULT InstallExt::DoInstall()
{
    TCHAR szModulePath[MAX_PATH] = { 0 };    
    TCHAR szCmdLine[MAX_PATH * 2] = { 0 };    

    ATL::CString strExeFile;
    ATL::CString strDirectory;
    ATL::CString strInfFile;

    GetModuleFileName( NULL, szModulePath, MAX_PATH);

    CPath path = szModulePath;
    path.RemoveFileSpec();

    strDirectory = path.m_strPath;

    path.Append(_T("snetcfg.exe"));

    strExeFile = path.m_strPath;

    path.m_strPath = strDirectory;
    path.Append(_T("kantiarp.inf"));

    GetShortPathName(path.m_strPath, szModulePath, MAX_PATH);
    
    strInfFile = szModulePath;

	_sntprintf_s(szCmdLine, MAX_PATH * 2, MAX_PATH * 2 - 1, 
        _T(" -l %s -c s -i ms_KAntiarp"), strInfFile );
    
    strExeFile += szCmdLine;

	//HANDLE hThread = CreateThread(NULL, 1024, ThreadFunc, 0, 0, 0);
	//if ( hThread != NULL )
	//	CloseHandle( hThread );

    return RunInsatll(strExeFile.GetBuffer(MAX_PATH + 1), strDirectory.GetBuffer(MAX_PATH + 1) );

}


void InstallExt::WaitEXEExit()
{
    HANDLE hMutex = NULL;

    do 
    {

        hMutex = ::CreateMutex(NULL, TRUE, _T("161F3D86-2025-4644-A353-812D6EA7FC8C"));

        if (hMutex != NULL && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            CloseHandle(hMutex);

            break;
        }

        if (hMutex != NULL)
            CloseHandle (hMutex);
        
        if ( MessageBox(NULL, _T("金山ARP防火墙 正在运行。请先退出。然后重试。"), _T("金山ARP防火墙"),  MB_RETRYCANCEL) == IDCANCEL)
            break;

    } while( TRUE );

    Sleep(100);

}

HRESULT InstallExt::DoUninstall()
{
    WaitEXEExit();

    TCHAR szModulePath[MAX_PATH] = { 0 };    

    ATL::CString strExeFile;
    ATL::CString strDirectory;

    GetModuleFileName( NULL, szModulePath, MAX_PATH);

    CPath path = szModulePath;
    path.RemoveFileSpec();

    strDirectory = path.m_strPath;

    path.Append(_T("snetcfg.exe"));

    strExeFile = path.m_strPath;


    strExeFile +=  _T(" -u ms_KAntiarp");

    //MessageBox(NULL, strExeFile, strDirectory, MB_OK);

    return RunInsatll(strExeFile.GetBuffer(MAX_PATH + 1), strDirectory.GetBuffer(MAX_PATH + 1) );

}

HRESULT InstallExt::RunInsatll(LPTSTR pszCmdLine, LPTSTR pszCurPath)
{
    DWORD dwResult = E_FAIL;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	
    ZeroMemory( &si, sizeof(si) );
    ZeroMemory( &pi, sizeof(pi) );

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if( !CreateProcess( NULL, // No module name (use command line). 
        pszCmdLine,       // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        TRUE,             // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        pszCurPath,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
        ) 
    {
        return dwResult;
    }

    WaitForSingleObject( pi.hProcess, INFINITE );
	
	GetExitCodeProcess( pi.hProcess, &dwResult );

    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	return dwResult;
}

//
//void InstallExt::GetOldRegValue()
//{
//    CRegKey regc;
//    CRegKey regl;
//
//    CRegKey reg;
//
//    regc.Create(HKEY_CURRENT_USER , _T("SOFTWARE\\Microsoft\\Driver Signing"));
//    regl.Create(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Driver Signing"));
//    
//    
//    if (regc.m_hKey != NULL)
//    {
//        regc.QueryDWORDValue(_T("Policy"), m_dwCurUserPolicy);
//        
//        DWORD dwValue = 0;
//
//        regc.SetDWORDValue(_T("Policy"), dwValue);
//
//
//    }
//
//    if (regl.m_hKey != NULL)
//    {
//        ULONG ulong = 1;
//        BYTE  byte[1] = { 0 };
//
//
//        regl.QueryBinaryValue(_T("Policy"), m_bufferLocMPolicy, &ulong);
//        regl.SetBinaryValue(_T("Policy"), byte, 1);
//    }
//
//}
//
//
//void InstallExt::SetOldRegValue()
//{
//    CRegKey regc;
//    CRegKey regl;
//
//    CRegKey reg;
//
//    regc.Create(HKEY_CURRENT_USER , _T("SOFTWARE\\Microsoft\\Driver Signing"));
//    regl.Create(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Driver Signing"));
//
//
//    if (regc.m_hKey != NULL)
//    {
//        regc.SetDWORDValue(_T("Policy"), m_dwCurUserPolicy);
//    }
//
//    if (regl.m_hKey != NULL)
//    {
//        regl.SetBinaryValue(_T("Policy"), m_bufferLocMPolicy, 1);
//    }
//
//}

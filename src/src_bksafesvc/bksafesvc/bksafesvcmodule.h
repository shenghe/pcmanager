#pragma once
#include "kxebase/kxebase.h"
#include "miniutil/bksvcmdl.h"
#include "winmod/winpath.h"
#include "safeapi/bootchecktool.h"
#include "unknownreport.h"
#include "unkownrescan.h"
#include "insrecycle.h"
#include "globalwhitelist.h"
#include "signverifer.h"
#include "avpperf.h"
#include "KDump/KDump.h"
#include "kwssp.h"
#include "netmonspproxy.h"
#include "bkrescanspmgr.h"
#include "svc/kbasicspwrapper.h"
#define BKSAFESVC_APP_NAME L"KSafeSvc"
#define COM_MUTEX_NAME  _T("Global\\84A34B8D-CBFD-46e2-8781-DEC9CF4CCB13")

#define MAX_HOST_FILE_SIZE  0x2800

class CbksafesvcModule : 
    public CBkServiceModuleT<CbksafesvcModule, IDS_SERVICENAME, IDS_SERVICEDISP, IDS_SERVICEDESP>
{
public:
    CbksafesvcModule() : m_hComMutex( NULL )
    {
        m_filterHost.Add( L".beike.cn" );
        m_filterHost.Add( L" beike.cn" );
        m_filterHost.Add( L"\tbeike.cn" );
        m_filterHost.Add( L".ijinshan.com" );
        m_filterHost.Add( L" ijinshan.com" );
        m_filterHost.Add( L"\tijinshan.com" );
        m_filterHost.Add( L".duba.net" );
        m_filterHost.Add( L" duba.net" );
        m_filterHost.Add( L"\tduba.net" );

		m_bStopKwsp			= FALSE;
		m_bKxeBaseInited	= FALSE;

		m_status.dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
    }
public :
    static void InitLibId() throw()
    {
        ATL::CAtlModule::m_libid = LIBID_bksafesvcLib;
    }

    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BKSAFESVC, "{E44A3E87-876D-46BB-8831-836A4C74918B}")

    BOOL Init()
    {
        DWORD dwRet = ::GetModuleFileName(NULL, m_strAppFileFullName.GetBuffer(MAX_PATH + 1), MAX_PATH);
        if (0 == dwRet || MAX_PATH == dwRet)
        {
            m_strAppFileFullName.ReleaseBuffer(0);
            return FALSE;
        }

        m_strAppFileFullName.ReleaseBuffer();

        m_strAppFileName = m_strAppFileFullName.Mid(m_strAppFileFullName.ReverseFind(L'\\') + 1);
        m_strAppPath = m_strAppFileFullName.Left(m_strAppFileFullName.ReverseFind(L'\\') + 1);

        return TRUE;
    }

    HRESULT OnInitializeSecurity() throw()
    {
        HRESULT hRet = ::CoInitializeSecurity(
            NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_PKT,
            RPC_C_IMP_LEVEL_IDENTIFY,
            NULL, EOAC_NONE, NULL );

        return hRet;
    }

    BOOL OnParseCommandLine(CBkCmdLine &_CmdLine, int nShowCmd, HRESULT &hRet)
    {
        BOOL bRet = FALSE;

        return bRet;
    }

	int KxeBaseInit(void)
	{
		int nRet = kxe_base_init_client();
		if (nRet != 0)
		{
			return nRet;
		}

		nRet = kxe_base_start();
		if (nRet != 0)
		{
			kxe_base_fini();
			return nRet;
		}

		m_bKxeBaseInited = TRUE;
		return 0;
	}

	int KxeBaseUnInit(void)
	{
		if (!m_bKxeBaseInited)
			return 0;

		kxe_base_stop_ipc();
		kxe_base_stop();
		kxe_base_fini();

		m_bKxeBaseInited = FALSE;

		return 0;

	}

    void OnStart() throw()
    {
        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Begin Start Service");

		/*服务初始化最开始先注册Dump*/
		BOOL bRet = KDump::Instance().Install();
        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Install Dump Result:%d", bRet);

        // fix host
        FixHostFile();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Begin KxeBaseInit");
		int nRet = KxeBaseInit();
        if (nRet != 0)
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_CRITICAL, L"[OnStart] KxeBaseInit Failed:%x", nRet);
        }
        else
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] KxeBaseInit Success");
        }
        

//         if ( SUCCEEDED( CUnkownReport::Instance().Initialize() ) )
//         {
//             CUnkownReport::Instance().StartRunning();
//         }

//         if ( SUCCEEDED( CUnkownRescan::Instance().Initialize() ) )
//         {
//             CUnkownRescan::Instance().StartRunning();
//         }

        if ( SUCCEEDED(CBKRescanSpMgr::Instance().Initialize()))
        {
            CBKRescanSpMgr::Instance().StartRescanSP();
        }


//        CGlobalWhiteList::Instance().LoadGlobalWhiteList();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Begin Start Basic SP");
		nRet = m_BasicSPWrapper.Initialize();
        if (nRet != 0)
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_CRITICAL, L"[OnStart] Basic SP Start Failed:%x", nRet);
        }
        else
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Basic SP Start Success");
        }

//        CSignVerifer::Instance().Initialize();

//        Skylark::CAvpEchoPerf::InitAvpEchoPerf();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Begin Start Kwssp");
		HRESULT hr = Kwssp::Instance().Start();
        if (FAILED(hr))
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_CRITICAL, L"[OnStart] Kwssp Start Failed:%x", hr);
        }
        else
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Kwssp  Start Success");
        }

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] Begin Start KNetmonSp");
		hr = KNetmonSp::Instance().Start();
        if (FAILED(hr))
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_CRITICAL, L"[OnStart] KNetmonSp Start Failed:%x", hr);
        }
        else
        {
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStart] KNetmonSp  Start Success");
        }
		/*tray中会等待此事件*/
        CreateComMutex();
    }

    void OnStop()
    {
		if ( !m_bStopKwsp )
		{
			m_bStopKwsp = TRUE;

            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Begin Stop m_BasicSPWrapper");
			m_BasicSPWrapper.Uninitialize();
            CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Stop m_BasicSPWrapper Success");

		}

        if ( m_hComMutex )
        {
            CloseHandle( m_hComMutex );
            m_hComMutex = NULL;
        }

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Begin Stop Kwssp");
		Kwssp::Instance().Stop();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Begin Stop KNetmonSp");
		KNetmonSp::Instance().Stop();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Begin Stop RescanSp");
        CBKRescanSpMgr::Instance().StopRescanSP();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Begin Uninit RescanSp");
        CBKRescanSpMgr::Instance().Uninitialize();

        //         CUnkownRescan::Instance().NotifyStop();
        //CUnkownReport::Instance().NotifyStop();

        //         CUnkownRescan::Instance().WaitExit(INFINITE);
        //         CUnkownRescan::Instance().Uninitialize();

        //CUnkownReport::Instance().WaitExit(INFINITE);
        //CUnkownReport::Instance().Uninitialize();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Begin Uninit KxeBase");
		KxeBaseUnInit();

        CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[OnStop] Uninit Over");
    }

    void OnFinal()
    {
        if (m_bNeedRestart)
        {
            CBkCmdLine cmdLine;

            cmdLine.SetParam(CMDLINE_PARAM_NAME_RESTART, 1);
            cmdLine.Execute(AppFileFullName(), FALSE, FALSE);
        }
    }

    LPCTSTR AppName()
    {
        return BKSAFESVC_APP_NAME;
    }

    LPCTSTR AppPath()
    {
        return m_strAppPath;
    }

    LPCTSTR AppFileFullName()
    {
        return m_strAppFileFullName;
    }

    LPCTSTR AppFileName()
    {
        return m_strAppFileName;
    }

    ULONG OnObjectCreate()
    {
        return ::InterlockedIncrement(&ms_lObjectCount);
    }

    ULONG OnObjectDestroy()
    {
        return ::InterlockedDecrement(&ms_lObjectCount);
    }

    void Restart()
    {
        m_bNeedRestart = TRUE;

        OnStop();
    }

    BOOL Install()
    {
        if (!__super::Install())
        {
            return FALSE;
        }

        ::Sleep(500);

        return TRUE;
    }

    BOOL Uninstall() throw()
    {
        BOOL bRet = FALSE;
            
        bRet = __super::Uninstall();

        return bRet;
    }
private:
    HRESULT CreateComMutex()
    {
        HRESULT hr = S_OK;
        DWORD   dwErr  = ERROR_SUCCESS;

        CDacl dacl;
        dacl.AddAllowedAce( Sids::World(), SYNCHRONIZE );
        dacl.AddAllowedAce( Sids::Admins(), MUTEX_ALL_ACCESS  );

        CSecurityDesc sd;
        sd.SetDacl( dacl );

        CSecurityAttributes sa;
        sa.Set( sd );

		CString strWndClassName = COM_MUTEX_NAME;

		std::wstring strValue;

		if (KisPublic::Instance()->Init())
		{
			int nRet = KisPublic::Instance()->KQueryOEMLPCWSTR(KIS::KOemKsfe::oemc_OtherCheckName, strValue);
			if (nRet)
				strWndClassName.Append(strValue.c_str());

		}

        m_hComMutex = CreateMutex( &sa, FALSE, strWndClassName );
        dwErr = GetLastError();
        if ( ERROR_ALREADY_EXISTS == dwErr )
        {
            return S_FALSE;
        }

        return HRESULT_FROM_WIN32( dwErr );
    }

    HRESULT LoadHostFile( 
        LPCWSTR lpwszHostPath, 
        ULARGE_INTEGER& ullFileSize, 
        CAtlArray<CString>& host 
        )
    {
        HRESULT hr = S_OK;
        HANDLE  hFile = INVALID_HANDLE_VALUE;
        char    *pszFileBuffer = NULL;

        //USES_CONVERSION;

        host.RemoveAll();
        hFile = CreateFile(
            lpwszHostPath, 
            GENERIC_READ, 
            FILE_SHARE_READ, 
            NULL, 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, 
            NULL 
            );
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto Exit0;
        }

        ullFileSize.LowPart = ::GetFileSize( hFile, &ullFileSize.HighPart );
        if ( 0 == ullFileSize.LowPart )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto Exit0;
        }

        if ( ullFileSize.LowPart >= MAX_HOST_FILE_SIZE )
        {
            hr = S_FALSE;
            goto Exit0;
        }

        pszFileBuffer = new char[ ullFileSize.LowPart + 1 ];
        if ( NULL == pszFileBuffer )
        {
            hr = E_OUTOFMEMORY;
            goto Exit0;
        }

        ZeroMemory( pszFileBuffer, ullFileSize.LowPart + 1 );

        do 
        {
            DWORD dwRetSize = 0;
            int nFind = -1;

            if ( ( FALSE == ReadFile(hFile, pszFileBuffer, ullFileSize.LowPart, &dwRetSize, NULL) ) 
                || ( 0 == dwRetSize) )
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
                break;
            }

            CString strValue;//(  );
            strValue = CA2W( pszFileBuffer );
            if (FALSE == strValue.IsEmpty())
            {
                if ( '\n' != pszFileBuffer[ strlen( pszFileBuffer ) - 1 ] )
                {
                    strValue += L"\r\n";
                }
            }

            nFind = strValue.Find(L'\n');
            while(nFind >= 0)
            {
                CString strTmp;
                strTmp = strValue.Left(nFind+1);
                host.Add(strTmp);
                strValue = strValue.Mid(nFind+1);
                nFind = -1;
                nFind = strValue.Find(L'\n');
            }
        } while (FALSE);

Exit0:
        if ( pszFileBuffer )
        {
            delete[] pszFileBuffer;
        }

        if (INVALID_HANDLE_VALUE != hFile)
        {
            CloseHandle(hFile);
            hFile = NULL;
        }

        return hr;
    }

    HRESULT WriteLineToFile( FILE* fp, LPCWSTR lpwszValue )
    {
        int     nRet = -1;

        //USES_CONVERSION;

        nRet = fputs( CW2A( lpwszValue ), fp );
        if ( EOF == nRet )
        {
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT SaveHostFile( 
        LPCWSTR lpwszHostPath, 
        const CAtlArray<CString>& host 
        )
    {
        HRESULT             hr = S_OK;
        BOOL                bRet = FALSE;
        FILE*               fp = NULL;
        WinMod::CWinPath    hostpath, tmppath;

        hostpath.m_strPath = lpwszHostPath;
        tmppath = hostpath;
        tmppath.RemoveFileSpec();
        tmppath.Append( L"hosts.old" );

        if ( hostpath.IsExisting() )
        {
            if ( tmppath.IsExisting() )
            {
                ::SetFileAttributes( tmppath, FILE_ATTRIBUTE_NORMAL );
                bRet = DeleteFile( tmppath );
                if ( !bRet )
                {
                    return HRESULT_FROM_WIN32( GetLastError() );
                }
            }

            bRet = ::MoveFile( hostpath, tmppath );
            if ( !bRet )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }
        }

        tmppath.RemoveFileSpec();
        tmppath.Append( L"hosts.new" );
        if ( tmppath.IsExisting() )
        {
            ::SetFileAttributes( tmppath, FILE_ATTRIBUTE_NORMAL );
            bRet = DeleteFile( tmppath );
            if ( !bRet )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }
        }

        do
        {
            fp = _wfopen( tmppath, L"a+b" );
            if ( NULL == fp )
            {
                hr = E_FAIL;
            }

            for ( size_t i = 0; i < host.GetCount(); i++ )
            {
                hr = WriteLineToFile( fp, host[ i ] );
                if ( FAILED( hr ) )
                {
                    break;
                }

                if (host[ i ].Right(1) != "\n")
                {
                    hr = WriteLineToFile( fp, L"\r\n" );
                    if ( FAILED( hr ) )
                    {
                        break;
                    }
                }
            }
        }while( 0 );

        if ( fp )
        {
            fclose( fp );
            fp = NULL;
        }

        if ( SUCCEEDED( hr ) )
        {
            bRet = ::CopyFile( tmppath, hostpath, FALSE );
            if ( !bRet )
            {
                return HRESULT_FROM_WIN32( GetLastError() );
            }

            //::SetFileAttributes( hostpath, FILE_ATTRIBUTE_READONLY );
            ::DeleteFile( tmppath );
        }

        return hr;
    }

    HRESULT FixHostFile()
    {
        BOOL                bFix = FALSE;
        HRESULT             hr = S_OK;
        DWORD               dwRet = 0;
        WCHAR               wszSystemRoot[ MAX_PATH + 1 ] = { 0 };
        ULARGE_INTEGER      ullFileSize;
        WinMod::CWinPath    hostpath;
        CAtlArray<CString>  host, newhost;

        dwRet = GetSystemDirectory( wszSystemRoot, MAX_PATH );
        if ( 0 == dwRet )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto Exit0;
        }

        hostpath.m_strPath = wszSystemRoot;
        hostpath.Append( L"drivers\\etc\\hosts" );
        //hostpath.Append( L"etc" );
        //hostpath.Append( L"hosts" );

        hr = LoadHostFile( hostpath, ullFileSize, host );
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }

        if ( S_FALSE == hr )
        {
            //CString strValue;

            //newhost.RemoveAll();
            //strValue = L"127.0.0.1 localhost";
            //newhost.Add( strValue );

            //bFix = TRUE;
            goto Exit0;
        }
        else
        {
            for ( size_t  i = 0; i < host.GetCount(); i++ )
            {
                int     iPound = -1;
                int     iHost = -1;
                CString strValue;

                if ( 0 == host[ i ].GetLength() )
                {
                    continue;
                }

                iPound = host[ i ].Find( L'#' );
                for ( size_t j = 0; j < m_filterHost.GetCount(); j++ )
                {
                    iHost = host[ i ].Find( m_filterHost[ j ] );
                    if ( iHost > -1 )
                    {
                        break;
                    }
                }

                if ( -1 == iHost )
                {
                    newhost.Add( host[ i ] );
                    continue;
                }

                if ((iPound > -1) && (iPound < iHost)) 
                {
                    newhost.Add( host[ i ] );
                    continue;
                }

                bFix = TRUE;
            }
        }

        if ( bFix )
        {
            SaveHostFile( hostpath, newhost );
        }
Exit0:
        return hr;
    }
protected:

    static LONG ms_lObjectCount;

    CString m_strAppPath;
    CString m_strAppFileName;
    CString m_strAppFileFullName;
    BOOL m_bNeedRestart;

	BOOL					m_bStopKwsp;
	KBasicSPWrapper         m_BasicSPWrapper;
	BOOL					m_bKxeBaseInited;
private:
    HANDLE  m_hComMutex;
    CAtlArray<CString>  m_filterHost;
};

extern CbksafesvcModule* _AtlModule;
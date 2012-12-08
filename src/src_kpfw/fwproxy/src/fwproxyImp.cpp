///////////////////////////////////////////////////////////////	
//	
// FileName  :  fwproxyImp.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-08-20  16:12
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "stdafx.h"
#include <winioctl.h>
#include "ktdifilt_public.h"
#include "fwproxyimp.h"
#include "fwinstaller.h"
#include "kpfw/msg_logger.h"

#include <process.h>
#pragma comment(lib, "shlwapi.lib")

//////////////////////////////////////////////////////////////////////////

CFwProxy::CFwProxy()
	: m_hDriver(INVALID_HANDLE_VALUE)
    , m_hExitEvent(NULL)
    , m_piEvent(NULL)
	, m_hCommEvent(NULL)
	, m_hActiveEvent(NULL)
	, m_bInit(FALSE)
{
}

//////////////////////////////////////////////////////////////////////////

CFwProxy::~CFwProxy()
{
    UnInitialize();
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::Initialize(IFwEvent* piEvent)
{
	HRESULT hr = E_FAIL;
	int nRetCode;
	
	/*if (IsWow64())
		return E_FAIL;*/

	if( m_bInit )
		return S_OK;

    m_hExitEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!m_hExitEvent)
        goto clean0;

    nRetCode = OpenDevice();
    if (!nRetCode)
	{
		DBG("OpenDriverDevice Failed");
        goto clean0;
	}

	if (piEvent != NULL )
	{
		m_piEvent = piEvent;
		
		if (!TestCanLoad())
		{
			CheckVersion();
			goto clean0;
		}

		nRetCode = RegisterEvent();
		if (!nRetCode)
		{
			goto clean0;
		}
		nRetCode = BeginWorkerThreads();
	}
	
	hr = S_OK;

clean0:
    if (FAILED(hr))
    {
        UnInitialize();
    }

	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::UnInitialize()
{

	CloseDevice();

    if (m_hExitEvent)
    {
        ::CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;
    }
	
	if( m_hCommEvent != NULL )
	{
		CloseHandle( m_hCommEvent );
		m_hCommEvent = NULL;
	}

	if( m_hActiveEvent != NULL )
	{
		CloseHandle( m_hActiveEvent );
		m_hActiveEvent = NULL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

int CFwProxy::OpenDevice()
{
	int retval = false;
	m_hDriver = ::CreateFile(
		KTDIFLT_LINK_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);
	if (m_hDriver == INVALID_HANDLE_VALUE)
	{
		goto clean0;
	}

	retval = true;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

void CFwProxy::CloseDevice()
{
	if (m_hDriver != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hDriver);
		m_hDriver = INVALID_HANDLE_VALUE;
	}
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::GetFlux(PULONGLONG pInbound, PULONGLONG pOutbound,
							   PULONGLONG pInboundLocal, PULONGLONG pOutboundLocal)
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONGLONG dwFluxs[4];
	ULONG dwRetCode;

	if (!pInbound && !pOutbound)
		goto clean0;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
	               m_hDriver,
	               IOCTL_KDTIFLT_GET_FLUXS,
	               NULL,
	               0,
	               dwFluxs,
	               sizeof(dwFluxs),
	               &dwRetCode,
	               NULL
	           );
	if (!fRetcode)
		goto clean0;

	if (dwRetCode != sizeof(ULONGLONG) * 4)
		goto clean0;

	if (pInbound) 
    {
		*pInbound = dwFluxs[0];
	}

	if (pOutbound) 
    {
		*pOutbound = dwFluxs[1];
	}
	if (pInboundLocal) 
	{
		*pInboundLocal = dwFluxs[2];
	}

	if (pOutboundLocal) 
	{
		*pOutboundLocal = dwFluxs[3];
	}
	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::EnumConnect(PCONN_INFO pConnInfo, ULONG dwInfoCount, PULONG pdwRetCount)
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (!pConnInfo || !pdwRetCount)
		goto clean0;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
	               m_hDriver,
	               IOCTL_KTDIFLT_ENUM_TCP_CONN,
	               NULL,
	               0,
	               pConnInfo,
	               sizeof(*pConnInfo) * dwInfoCount,
	               &dwRetCode,
	               NULL
	           );
	if (!fRetcode)
		goto clean0;

	*pdwRetCount = dwRetCode / sizeof(*pConnInfo);

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::EnumProcessTraffic(PROCESS_TRAFFIC* pProcessTraffic, ULONG dwProcessCount, PULONG pdwRetCount)
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (!pProcessTraffic || !pdwRetCount)
		goto clean0;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_GET_PROCESS_FLUXS,
		NULL,
		0,
		pProcessTraffic,
		sizeof(PROCESS_TRAFFIC) * dwProcessCount,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	*pdwRetCount = dwRetCode / sizeof(PROCESS_TRAFFIC);

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::EnableFirewall()
{
    HRESULT hr = E_FAIL;
    BOOL fRetcode;
    ULONG dwRetCode;

    if (INVALID_HANDLE_VALUE == m_hDriver)
        goto clean0;

    fRetcode = ::DeviceIoControl(
        m_hDriver,
        IOCTL_KTDIFLT_ENABLE_FIREWALL,
        NULL,
        0,
        NULL,
        0,
        &dwRetCode,
        NULL
        );
    if (!fRetcode)
        goto clean0;

    hr = S_OK;

clean0:
    return hr;
}

STDMETHODIMP CFwProxy::DisableFirewall()
{
    HRESULT hr = E_FAIL;
    BOOL fRetcode;
    ULONG dwRetCode;

    if (INVALID_HANDLE_VALUE == m_hDriver)
        goto clean0;

    fRetcode = ::DeviceIoControl(
        m_hDriver,
        IOCTL_KTDIFLT_DISABLE_FIREWALL,
        NULL,
        0,
        NULL,
        0,
        &dwRetCode,
        NULL
        );
    if (!fRetcode)
        goto clean0;

    hr = S_OK;

clean0:
    return hr;
}

STDMETHODIMP CFwProxy::EnableNetwork()
{
    HRESULT hr = E_FAIL;
    BOOL fRetcode;
    ULONG dwRetCode;

    if (INVALID_HANDLE_VALUE == m_hDriver)
        goto clean0;

    fRetcode = ::DeviceIoControl(
        m_hDriver,
        IOCTL_KTDIFLT_ENABLE_NETWORK,
        NULL,
        0,
        NULL,
        0,
        &dwRetCode,
        NULL
        );
    if (!fRetcode)
        goto clean0;

    hr = S_OK;

clean0:
    return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::DisableNetwork()
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_DISABLE_NETWORK,
		NULL,
		0,
		NULL,
		0,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	hr = S_OK;

clean0:
	return hr;
}

STDMETHODIMP CFwProxy::EnableFluxStastic()
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_ENABLE_FLUX_STASTIC,
		NULL,
		0,
		NULL,
		0,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	hr = S_OK;

clean0:
	return hr;
}

STDMETHODIMP CFwProxy::DisableFluxStastic()
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_DISABLE_FLUX_STASTIC,
		NULL,
		0,
		NULL,
		0,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::ClearResultCache()
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_CLEAR_RESULT_CACHE,
		NULL,
		0,
		NULL,
		0,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

int CFwProxy::BeginWorkerThreads()
{
	BOOL bResult = TRUE;
    m_hWorkerThread = (HANDLE)_beginthreadex( 
        NULL, 
        0, 
        WorkerThreadRoutine, 
        this, 
        0, 
        NULL
        );
    if ( m_hWorkerThread == NULL )
    {
        EndWorkerThreads();
		bResult = FALSE;
    }

    return bResult;
}

//////////////////////////////////////////////////////////////////////////

int CFwProxy::EndWorkerThreads()
{
    if (m_hExitEvent)
    {
        ::SetEvent(m_hExitEvent);
    }

    if (m_hWorkerThread)
    {
        if (::WaitForSingleObject(m_hWorkerThread, 100) == WAIT_TIMEOUT)
        {
            ::TerminateThread(m_hWorkerThread, 1);
        }

        ::CloseHandle(m_hWorkerThread);
        m_hWorkerThread = NULL;
    }


    return true;
}

//////////////////////////////////////////////////////////////////////////

UINT CFwProxy::WorkerThreadRoutine( PVOID pParam )
{
	UINT retval = 0;
	CFwProxy* pThis = (CFwProxy*)pParam;
	DWORD dwWait;
	HANDLE hWaits[3] = { 0 };
	RESPONSE_APP_REQUEST_INFO requestInfo;
	APP_NOTIFY_INFO appNotifyInfo;

	if ( pThis == NULL )
		goto clean0;

	try
	{
		hWaits[0] = pThis->m_hExitEvent;
		hWaits[1] = pThis->m_hCommEvent;
		hWaits[2] = pThis->m_hActiveEvent;

		for (;;) 
		{
			dwWait = ::WaitForMultipleObjects(3, hWaits, FALSE, INFINITE);
			switch( dwWait )
			{
			case WAIT_OBJECT_0:
				goto clean0;
			case WAIT_OBJECT_0 + 1:
				{
					memset( (char*)&appNotifyInfo, 0, sizeof(appNotifyInfo) );
					if( !pThis->GetCommonRequestInfo( &appNotifyInfo ) )
						continue;

					if( pThis->m_piEvent != NULL )
					{
						pThis->m_piEvent->OnApplicationNotify( &appNotifyInfo );
					}
				}
				break;
			case WAIT_OBJECT_0 + 2:
				{
					memset( (char*)&requestInfo, 0, sizeof(requestInfo) );
					if( !pThis->GetRequestInfo( &requestInfo ) )
						continue;

					if( pThis->m_piEvent != NULL )
					{
						pThis->m_piEvent->OnApplicationRequest( &requestInfo );
					}
				}
				break;
			default:
				goto clean0;
			}
		}
	}
	catch (...)
	{
	}

clean0:
	_endthreadex(retval);
	return retval;
}

//////////////////////////////////////////////////////////////////////////

int CFwProxy::RegisterEvent()
{
	BOOL bResult = FALSE;
	DWORD dwReturnBytes = 0;
	TDI_EVENT_REGISTER eventRegister = {0};
	
	do 
	{
		m_hCommEvent = ::CreateSemaphore( NULL, 0, 1024*4, 0);
		if( m_hCommEvent == NULL )
			break;

		m_hActiveEvent = ::CreateSemaphore( NULL, 0, 1024*4, 0);
		if( m_hActiveEvent == NULL )
			break;

		eventRegister.hActiveEvent = (ULONGLONG)m_hActiveEvent;
		eventRegister.hCommEvent = (ULONGLONG)m_hCommEvent;
		eventRegister.m_uMarVer = KFW_MAJOR_VER;
		eventRegister.m_uMinVer = KFW_MINOR_VER;
		
		if( !DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_REGISTER_EVENT, 
			(LPVOID)&eventRegister, sizeof(eventRegister),
			0,0,
			&dwReturnBytes, 0 ) )
		{
			CheckVersion();
			break;
		}
		SetDirverVersionError(FALSE);
		bResult = TRUE;

	} while (FALSE);

	return bResult;
}
BOOL	CFwProxy::CheckVersion()
{
	KFW_STATE state = {0};
	if (GetKfwState(&state) == S_OK)
	{
		// OutputDebugStringW(TEXT("driver GetKfwState ok:\n"));
		if ((state.usMajorVer != KFW_MAJOR_VER) || 
			(state.usMinorVer != KFW_MINOR_VER))
		{
			// OutputDebugStringW(TEXT("driver find version dismatch:\n"));
			SetDirverVersionError(TRUE);	
		}
		else
		{
			// OutputDebugStringW(TEXT("driver find version match:\n"));
			SetDirverVersionError(FALSE);	
		}
	}
	else
	{
		// OutputDebugStringW(TEXT("driver GetKfwState failed\n"));
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::SendResponse(
	ULONGLONG responseContext,
	ULONG dwResult)
{
	DWORD dwReturnBytes = 0;
	RESPONSE_RESULT response = {0,0};
	response.pResponseContext = responseContext;
	response.pResult = dwResult;

	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_RESPONSE_EVENT, 
		(LPVOID)&response, sizeof(response),
		(LPVOID)&response, sizeof(response),
		&dwReturnBytes, 0 );

	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
BOOL CFwProxy::GetRequestInfo( RESPONSE_APP_REQUEST_INFO *requestInfo )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_GET_ACTIVE_EVENT, 
				(LPVOID)requestInfo, sizeof(RESPONSE_APP_REQUEST_INFO),
				(LPVOID)requestInfo, sizeof(RESPONSE_APP_REQUEST_INFO),
				&dwReturnBytes, 0 );
	
	return bResult;
}

//////////////////////////////////////////////////////////////////////////

BOOL CFwProxy::GetCommonRequestInfo( APP_NOTIFY_INFO *appNotifyInfo )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_GET_COMMON_EVENT, 
		(LPVOID)appNotifyInfo, sizeof(APP_NOTIFY_INFO),
		(LPVOID)appNotifyInfo, sizeof(APP_NOTIFY_INFO),
		&dwReturnBytes, 0 );

	return bResult;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::GetKfwState( KFW_STATE *kfwState )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_GET_VERSION, 
		0, 0,
		(LPVOID)kfwState, sizeof(KFW_STATE),
		&dwReturnBytes, 0 );

	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::GetModuleInfo( PModuleInfo pModuleInfo, ULONG dwCount )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_GET_MODULE_INFO, 
		(PVOID)pModuleInfo, sizeof(ModuleInfo) * dwCount,
		(PVOID)pModuleInfo, sizeof(ModuleInfo) * dwCount,
		&dwReturnBytes, 0 );

	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::SetModuleInfo( PModuleInfo pModuleInfo, ULONG dwCount )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_SET_MODULE_INFO, 
		(PVOID)pModuleInfo, sizeof(ModuleInfo) * dwCount,
		0, 0,
		&dwReturnBytes, 0 );

	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::GetModuleList( PModuleInfo pModuleInfo, ULONG dwModuleCount, PULONG pdwRetCount)
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (!pModuleInfo || !pdwRetCount)
		goto clean0;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_GET_MODULE_LIST,
		NULL,
		0,
		pModuleInfo,
		sizeof(ModuleInfo) * dwModuleCount,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	*pdwRetCount = dwRetCode / sizeof(ModuleInfo);

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////
 
STDMETHODIMP CFwProxy::AddRecorder( PDATA_RECORDER pRecorder, ULONG dwCount )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_ADD_RECORDER, 
		(PVOID)pRecorder, sizeof(DATA_RECORDER) * dwCount,
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::RemoveRecorder( ULONG dwModlueID )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_REMOVE_RECORDER, 
		(PVOID)&dwModlueID, sizeof(ULONG),
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::ClearRecorder()
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_CLEAR_RECORDER, 
		0,0,
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) CFwProxy::OpenPort( PROTO_TYPE pt, USHORT uPort )
{
	BOOL bResult = FALSE;
	DWORD dwRetCode = 0;

	if ( INVALID_HANDLE_VALUE == m_hDriver  )
		return bResult;

	switch( pt )
	{
	case enumPT_TCP:
		bResult = ::DeviceIoControl(
			m_hDriver,
			IOCTL_KTDIFLT_TCP_PORT_OPEN,
			(PVOID)&uPort,
			sizeof(USHORT),
			0,
			0,
			&dwRetCode,
			NULL
			);
		break;
	case enumPT_UDP:
		bResult = ::DeviceIoControl(
			m_hDriver,
			IOCTL_KTDIFLT_UDP_PORT_OPEN,
			(PVOID)&uPort,
			sizeof(USHORT),
			0,
			0,
			&dwRetCode,
			NULL
			);
		break;
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) CFwProxy::ClosePort( PROTO_TYPE pt, USHORT uPort )
{
	BOOL bResult = FALSE;
	DWORD dwRetCode = 0;

	if ( INVALID_HANDLE_VALUE == m_hDriver  )
		return bResult;

	switch( pt )
	{
	case enumPT_TCP:
		bResult = ::DeviceIoControl(
			m_hDriver,
			IOCTL_KTDIFLT_TCP_PORT_CLOSE,
			(PVOID)&uPort,
			sizeof(USHORT),
			0,
			0,
			&dwRetCode,
			NULL
			);
		break;
	case enumPT_UDP:
		bResult = ::DeviceIoControl(
			m_hDriver,
			IOCTL_KTDIFLT_UDP_PORT_CLOSE,
			(PVOID)&uPort,
			sizeof(USHORT),
			0,
			0,
			&dwRetCode,
			NULL
			);
		break;
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) CFwProxy::GetOpenPort( PVOID pBitVec, ULONG uVecLen )
{
	BOOL bResult = FALSE;
	DWORD dwRetCode = 0;

	if ( INVALID_HANDLE_VALUE == m_hDriver || uVecLen != (65536 * sizeof(PVOID)*2) )
		return bResult;

	bResult = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_GET_OPEN_PORT,
		0,
		0,
		(PVOID)pBitVec,
		uVecLen,
		&dwRetCode,
		NULL
		);

	return bResult;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::SetRequestFlag( ULONG uRequestFlag )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_SET_REQUEST_FLAG, 
		(PVOID)&uRequestFlag, sizeof(ULONG),
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;
}

STDMETHODIMP CFwProxy::ClearFlux()
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_CLEAR_FLUX, 
		0,0,
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::SetProcessSpeed( ULONGLONG uPid, ULONG uSendSpeed, ULONG uRecvSpeed , ULONG nDisable)
{
	SET_PROCESS_LIMIT setSpeed = {uPid, uSendSpeed, uRecvSpeed, nDisable};
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_SET_PROCESS_SPEED, 
		(PVOID)&setSpeed, sizeof(SET_PROCESS_LIMIT),
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;

}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::EnumProgramTraffic(PROGRAM_TRAFFIC* pProgramTraffic, ULONG dwProgramCount, PULONG pdwRetCount)
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (!pProgramTraffic || !pdwRetCount)
		goto clean0;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_GET_PROGRAM_FLUXS,
		NULL,
		0,
		pProgramTraffic,
		sizeof(PROGRAM_TRAFFIC) * dwProgramCount,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	*pdwRetCount = dwRetCode / sizeof(PROGRAM_TRAFFIC);

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::EnumProgramTrafficFromCache(
	PROGRAM_TRAFFIC* pProgramTraffic, 
	ULONG dwProgramCount, 
	PULONG pdwRetCount
	)
{
	HRESULT hr = E_FAIL;
	BOOL fRetcode;
	ULONG dwRetCode;

	if (!pProgramTraffic || !pdwRetCount)
		goto clean0;

	if (INVALID_HANDLE_VALUE == m_hDriver)
		goto clean0;

	fRetcode = ::DeviceIoControl(
		m_hDriver,
		IOCTL_KTDIFLT_GET_PROGRAM_FLUXS_FROM_CACHE,
		NULL,
		0,
		pProgramTraffic,
		sizeof(PROGRAM_TRAFFIC) * dwProgramCount,
		&dwRetCode,
		NULL
		);
	if (!fRetcode)
		goto clean0;

	*pdwRetCount = dwRetCode / sizeof(PROGRAM_TRAFFIC);

	hr = S_OK;

clean0:
	return hr;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::SetProgramTrafficTimer( ULONG uTimer )
{
	DWORD dwReturnBytes = 0;
	BOOL bResult = DeviceIoControl( m_hDriver, IOCTL_KTDIFLT_SET_REFRESH_CACHE_TIMER, 
		(PVOID)&uTimer, sizeof(ULONG),
		0,0,
		&dwReturnBytes, 0 );
	return bResult ? S_OK : E_FAIL;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::EnumProgramTrafficFromFile( PROGRAM_TRAFFIC_FROM_FILE* pProgramTraffic, ULONG dwProgramCount, ULONG uStartItem, PULONG pdwRetCount )
{
	HRESULT hResult = E_FAIL;
	HANDLE hFile = NULL;
	WCHAR wsPath[MAX_PATH] = {0};
	DWORD dwRead = 0;
	DWORD dwFileSize = 0;
	DWORD dwCurrentOffset = 0;
	DWORD dwMoveOffset = uStartItem*sizeof(PROGRAM_TRAFFIC);
	PVOID pBuffer = NULL;;
	DWORD dwBufferSize = dwProgramCount * sizeof(PROGRAM_TRAFFIC);
	DWORD dwRetCount = 0;

	do 
	{
		if( pProgramTraffic == NULL || dwProgramCount == 0 || pdwRetCount == NULL )
			return FALSE;
		
		pBuffer = new char[dwBufferSize];
		if( pBuffer == NULL )
			break;

		if( !GetCachePathFromReg(wsPath) )
			break;

		hFile = CreateFile( wsPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile == INVALID_HANDLE_VALUE )
			break;

		dwFileSize = GetFileSize( hFile, NULL );
		if( dwFileSize / sizeof(PROGRAM_TRAFFIC) == 0 || (dwFileSize % sizeof(PROGRAM_TRAFFIC) != 0) )
			break;
		
		dwCurrentOffset = SetFilePointer( hFile, dwMoveOffset, NULL, FILE_BEGIN  );
		if( dwCurrentOffset != dwMoveOffset )
			break;

		if( !ReadFile( hFile, (PVOID)pBuffer, dwBufferSize, &dwRead, NULL ) )
			break;
		
		if( (dwRead % sizeof(PROGRAM_TRAFFIC)) != 0 )
			break;

		if( uStartItem == 0 )
			GetModulesFromReg();
		
		dwRetCount = dwRead / sizeof(PROGRAM_TRAFFIC);
		for( DWORD i=0; i<dwRetCount; i++ )
		{
			memcpy( (char*)&pProgramTraffic[i].progTraffic, &((PROGRAM_TRAFFIC*)pBuffer)[i], sizeof(PROGRAM_TRAFFIC) );
			GetModulePath( pProgramTraffic[i].progTraffic.dwModuleID,  pProgramTraffic[i].wsFilePath );
		}

		*pdwRetCount = dwRetCount;
		hResult = S_OK;
	} while (FALSE);

	if( pBuffer != NULL )
		delete []pBuffer;

	return hResult;
}

//////////////////////////////////////////////////////////////////////////

BOOL CFwProxy::GetCachePathFromReg( WCHAR *szPath )
{
	DWORD dwResult;
	DWORD dwType = REG_SZ;
	DWORD dwCbData = sizeof(WCHAR) * MAX_PATH;

	dwResult = SHGetValue( HKEY_LOCAL_MACHINE, L"Software\\KingSoft\\KISCommon", L"ProgramPath", &dwType, (LPVOID)szPath, &dwCbData );
	if( dwResult != ERROR_SUCCESS )
		return FALSE;
	_tcscat_s( szPath, MAX_PATH, PROGRAM_CACHE_FILE );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

VOID CFwProxy::GetModulesFromReg()
{
	TCHAR szValueName[16];
	DWORD dwType;
	WCHAR szPath[MAX_PATH] = {0};
	DWORD dwValueNameSize = 16;
	DWORD dwDataSize = MAX_PATH*sizeof(WCHAR);
	DWORD dwModuleId[2] = {0};
	CRegIterator regIterator;
	
	m_mapModules.clear();

	for( regIterator.begin( HKEY_LOCAL_MACHINE, 
							L"SYSTEM\\CurrentControlSet\\Services\\KTdiFilt\\Modules", 
							szValueName, &dwValueNameSize, &dwType, (LPBYTE)szPath, &dwDataSize );
		regIterator.next();
		)
	{
		_stscanf_s( szValueName, L"%08X", dwModuleId );
		m_mapModules[dwModuleId[0]] = szPath;

		memset((char*)szValueName, 0, sizeof(szValueName) );
		memset((char*)szPath, 0, sizeof(szPath) );
		dwValueNameSize = 16;
		dwDataSize = MAX_PATH*sizeof(WCHAR);
	}
}

//////////////////////////////////////////////////////////////////////////

VOID CFwProxy::GetModulePath( ULONG uModuleID, WCHAR* wsPath )
{
	map<ULONG, wstring>::iterator item = m_mapModules.find( uModuleID );
	if( item != m_mapModules.end() )
		_tcsncpy( wsPath, item->second.c_str(), MAX_PATH );
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwProxy::DeleteProgramTrafficFile()
{
	WCHAR wsPath[MAX_PATH] = {0};
	if( GetCachePathFromReg( wsPath ) )
	{
		if( DeleteFile( wsPath ) )
			return S_OK;
	}
	return E_FAIL;
}

BOOL CFwProxy::IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE hProcess,PBOOL Wow64Process);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"),"IsWow64Process");

	BOOL bIsWow64 = FALSE;

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
		}
	}
	return bIsWow64;
}

STDMETHODIMP CFwProxy::GetLastBootTime(ULONGLONG& nTime)
{
	ULONGLONG nLastBootTime = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nLastBootTime);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_LAST_BOOT_TIME, &nType, (PVOID)&nLastBootTime, &nSize);

	if( hResult != ERROR_SUCCESS )
		return 0;

	if ((nSize == sizeof(ULONGLONG)) && (nType == REG_BINARY))
	{
		nTime = nLastBootTime;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CFwProxy::SetForceDisableTdi(ULONG nValue)
{
	HRESULT hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_FORCE_DISABLE_TDI, REG_BINARY, (PVOID)&nValue, sizeof(nValue) );
	if( hResult != ERROR_SUCCESS )
		return hResult;
	return S_OK;
}

int CFwProxy::SetDirverVersionError(ULONG bError)
{
	HRESULT hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_TDI_VERSION_ERROR, REG_BINARY, (PVOID)&bError, sizeof(bError) );
	if( hResult != ERROR_SUCCESS )
		return hResult;
	return S_OK;
}

STDMETHODIMP CFwProxy::SetHasUnCompatibleDriver(ULONG nValue)
{
	HRESULT hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_TDI_HAS_UNCOMPATIBLE_DRIVER, REG_BINARY, (PVOID)&nValue, sizeof(nValue) );
	if( hResult != ERROR_SUCCESS )
		return hResult;
	return S_OK;
}

STDMETHODIMP CFwProxy::ClearLastBootTime()
{
	ULONGLONG nBootTime = 0;
	HRESULT hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_LAST_BOOT_TIME, REG_BINARY, (PVOID)&nBootTime, sizeof(nBootTime) );
	if( hResult != ERROR_SUCCESS )
		return hResult;
	return S_OK;
}

STDMETHODIMP CFwProxy::AddDumpCount(ULONG nAdd)
{
	ULONG nDumpCount = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nDumpCount);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_DUMP_COUNT, &nType, (PVOID)&nDumpCount, &nSize);

	nDumpCount += nAdd;

	hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_DUMP_COUNT, REG_BINARY, (PVOID)&nDumpCount, sizeof(nDumpCount) );
	if( hResult != ERROR_SUCCESS )
		return hResult;
	return S_OK;
}

STDMETHODIMP CFwProxy::ClearUnNormalRunCount()
{
	ULONGLONG nUnNormalRunCount = 0;
	HRESULT hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_UNNORMAL_RUN_COUTN, REG_BINARY, (PVOID)&nUnNormalRunCount, sizeof(nUnNormalRunCount) );
	if( hResult != ERROR_SUCCESS )
		return E_FAIL;;
	return S_OK;
}

STDMETHODIMP CFwProxy::ClearDumpCount()
{
	ULONG nDumpCount = 0;
	HRESULT hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_DUMP_COUNT, REG_BINARY, (PVOID)&nDumpCount, sizeof(nDumpCount) );
	if( hResult != ERROR_SUCCESS )
		return E_FAIL;
	return S_OK;
}

ULONG CFwProxy::GetForceDisableTDI()
{
	ULONG nForceDisableTDI = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nForceDisableTDI);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_FORCE_DISABLE_TDI, &nType, (PVOID)&nForceDisableTDI, &nSize);
	if( hResult != ERROR_SUCCESS )
		return 0;
	if (nType == REG_BINARY && nSize == sizeof(ULONG))
		return nForceDisableTDI;
	return 0;
}
ULONG CFwProxy::GetHasUnCompatibleDriver()
{
	ULONG nHasCompatibleDriver= 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nHasCompatibleDriver);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_TDI_HAS_UNCOMPATIBLE_DRIVER, &nType, (PVOID)&nHasCompatibleDriver, &nSize);
	if( hResult != ERROR_SUCCESS )
		return 0;
	if (nType == REG_BINARY && nSize == sizeof(ULONG))
		return nHasCompatibleDriver;
	return 0;
}

ULONG CFwProxy::GetDumpCount()
{
	ULONG nDumpCount = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nDumpCount);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_DUMP_COUNT, &nType, (PVOID)&nDumpCount, &nSize);
	if( hResult != ERROR_SUCCESS )
		return 0;
	if (nType == REG_BINARY && nSize == sizeof(ULONG))
		return nDumpCount;
	return 0;
}

ULONGLONG CFwProxy::GetUnNormalBootCount()
{
	ULONGLONG nUnNormalRunCount = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nUnNormalRunCount);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		DUMP_STATE_UNNORMAL_RUN_COUTN, &nType, (PVOID)&nUnNormalRunCount, &nSize);
	if( hResult != ERROR_SUCCESS )
		return 0;
	if (nType == REG_BINARY && nSize == sizeof(ULONGLONG))
		return nUnNormalRunCount;
	return 0;
}

STDMETHODIMP CFwProxy::ClearBootState()
{
	ClearUnNormalRunCount();
	ClearDumpCount();
	return S_OK;
}

BOOL CFwProxy::TestCanLoad()
{
	if (GetForceDisableTDI())
	{
		return FALSE;
	}

	if (GetHasUnCompatibleDriver())
	{
		return FALSE;
	}

	if ((GetDumpCount() > 2) || (GetUnNormalBootCount() >= 5))
		return FALSE;
	return TRUE;
}

#include "StdAfx.h"
#include "KFileDownload.h"


KFileDownload::KFileDownload()
{
	m_lpUserData = NULL;
	m_pObserver = NULL;

	m_hModuleKFileDownload = NULL;
	m_pIFileDownloader = NULL;

	m_hThread = NULL;
	m_nCorrurent = 0;
	m_nCurrentStat = 0;	
}

KFileDownload::~KFileDownload(void)
{
	if(m_hModuleKFileDownload)
	{
		m_fdFactory.ReleaseObject(&m_pIFileDownloader);
	}
}

void KFileDownload::Release()
{
	delete this;
}

VOID KFileDownload::SetObserver( IHttpAsyncObserver *pObserver )
{
	m_pObserver = pObserver;
}

void KFileDownload::SetDownloadInfo( LPCTSTR szUrl, LPCTSTR szFilePath )
{
	m_strUrl = szUrl;
	m_strFilePath = szFilePath;
}

LPVOID KFileDownload::SetUserData( LPVOID lpUserData )
{
	m_lpUserData = lpUserData;
	return m_lpUserData;
}

BOOL KFileDownload::Fetch( INT nCorrurent/*=0*/ )
{
	ATLASSERT(m_pIFileDownloader==NULL);

	if(m_pIFileDownloader!=NULL)
	{		
		return FALSE;
	}
	else
	{
		CObjGuard _guard_(m_lckFileDownloaderObject);
		m_fdFactory.CreateObject(&m_pIFileDownloader);
		
		if(!m_pIFileDownloader)
			return FALSE;

		if(nCorrurent<=0)
			nCorrurent = 4;
		else if(nCorrurent>8)
			nCorrurent = 8;

		USES_CONVERSION;

		CPubConfig::T_ProxySetting proxyconfig;
		CPubConfig pubconfig;
		pubconfig.LoadProxyConfig(proxyconfig);

		m_pIFileDownloader->SetThreadNumber(nCorrurent);

		switch(proxyconfig.nProxyMode)
		{
		case PROXY_MODE_IE:
			m_pIFileDownloader->SetProxyMethod(enumPROXY_METHOD_USE_IE);
			break;

		case PROXY_MODE_USER:
			m_pIFileDownloader->SetProxyMethod(enumPROXY_METHOD_CUSTOM);

			{			
				CStringA strHost = CT2CA(proxyconfig.strHost);
				if(proxyconfig.bNeedAuthorization)
				{
					CStringA strUser, strPasswd;

					strUser = CT2CA(proxyconfig.strLogin);
					strPasswd = CT2CA(proxyconfig.strPassword);

					m_pIFileDownloader->SetProxyInfo(enumPROXY_TYPE_HTTP, strHost, proxyconfig.nPort, strUser, strPasswd);
				}
				else
				{
					m_pIFileDownloader->SetProxyInfo(enumPROXY_TYPE_HTTP, strHost, proxyconfig.nPort);
				}
			}
			break;

		default:
			m_pIFileDownloader->SetProxyMethod(enumPROXY_METHOD_DIRECT);
			break;
		}

		m_pIFileDownloader->SetThreadNumber(nCorrurent);
		m_pIFileDownloader->SetSendTimeout(10);
		m_pIFileDownloader->SetRecvTimeout(10);
		m_pIFileDownloader->SetConnectTimeout(10);
		m_pIFileDownloader->SetCallback(this);
	}
	
	unsigned long uErrorCode = 0;

	CStringA strUrl = CT2CA(m_strUrl);
	CStringA strFile = CT2CA(m_strFilePath);
	
	HRESULT hr = m_pIFileDownloader->Download(strUrl, strFile, TRUE, uErrorCode);
	
	// Release 
	{
		CObjGuard _guard_(m_lckFileDownloaderObject);
		m_fdFactory.ReleaseObject(&m_pIFileDownloader);
	}

	if(PathFileExistsA(strFile))
	{
		_Notify(ProcessState_Finished);
		return TRUE;
	}
	else
	{
		_Notify(m_nCurrentStat==ProcessState_UserCanceled ? ProcessState_UserCanceled : ProcessState_Failed);
		return FALSE;
	}
}

BOOL KFileDownload::Start( INT nCorrurent/*=0*/ )
{
	Stop();
	Wait();
	
	ATLASSERT(m_hThread==NULL);
	_Reset();

	m_nCorrurent = nCorrurent;
		
	unsigned int nThreadID = 0;
	m_hThread = (HANDLE) _beginthreadex(NULL, 0, _RunThreadFun, this, 0, &nThreadID);
	return m_hThread!=NULL;
}

void KFileDownload::Stop()
{
	CObjGuard _guard_(m_lckFileDownloaderObject);
	if(m_pIFileDownloader)
	{
		m_pIFileDownloader->NotifyExit();
	}
}

void KFileDownload::Wait()
{
	if(m_hThread)
		WaitForSingleObject(m_hThread, INFINITE);
	SAFE_CLOSE_HANDLE(m_hThread);
}

DLERRCode KFileDownload::LastError()
{
	return m_errCode;
}

LPVOID KFileDownload::GetUserData()
{
	return m_lpUserData;
}

void KFileDownload::GetDownloadInfo( CString &strUrl, CString &strFilePath )
{
	strUrl = m_strUrl;
	strFilePath = m_strFilePath;
}

RemoteFileInfo * KFileDownload::GetRemoteFileInfo()
{
	return &m_FileInfo;
}

IDownStat * KFileDownload::GetDownloadStat()
{
	return &m_DownStat;
}

BOOL KFileDownload::_Notify( ProcessState state, LPARAM lParam/*=0*/ )
{
	m_nCurrentStat = ProcessState_UserCanceled;
	if(m_pObserver)
		return m_pObserver->OnHttpAsyncEvent(this, state, lParam);
	return TRUE;
}

void __stdcall KFileDownload::OnNotifyDownloadState( IN FDSCode nState )
{
	switch(nState)
	{
	case enumDS_READY:
		break;
	case enumDS_QUERYING_INFO:
		_Notify(ProcessState_EndRequestDone);
		break;
	case enumDS_CREATING_LOCAL_FILE:
		break;
	case enumDS_DOWNLOADING:
		_Notify(ProcessState_ReceiveData);
		break;
	case enumDS_USER_STOP:		
		_Notify(ProcessState_UserCanceled);
		break;
	case enumDS_SUCCESS:
		m_FileInfo.fileDownloaded = m_FileInfo.fileSize;
		m_DownStat.m_iDownloaded = m_FileInfo.fileSize;
		//_Notify(ProcessState_Finished);
		//此刻还没有重命名文件 
		break;
	case enumDS_FAILED:
		_Notify(ProcessState_Failed);
		break;
	}
}

void __stdcall KFileDownload::OnNotifyDownloadProgress( IN unsigned long ulFileSize, IN unsigned long ulDownloadSize, IN int nSpeed, IN int nThreadNumber, IN int nThreadRunning, IN const char* pszDescription )
{
	m_FileInfo.fileSize = ulFileSize;
	m_FileInfo.fileDownloaded = ulDownloadSize;
	
	m_DownStat.m_iDownloaded = ulDownloadSize;
	m_DownStat.m_nSpeed = nSpeed;

	_Notify(ProcessState_ReceiveData);
}

void __stdcall KFileDownload::OnNotifyDownloadError( IN FDECode nError )
{
	switch(nError)
	{
	case enumDE_SUCCEEDED:
		break;
	case enumDE_UNKNOWN:
	case enumDE_NORMAL:
	case enumDE_FILE_EXISTS:
	case enumDE_QUERY_INFO:
	case enumDE_CREATE_CONFIG_FILE:
	case enumDE_CREATE_LOCAL_FILE:
	case enumDE_QUERY_IE_PROXY:
	case enumDE_PROTOCOL_NOT_SUPPORT:
		break;
	}
}

bool __stdcall KFileDownload::OnQueryExitNeeded( void )
{
	return FALSE;
}

void KFileDownload::_Reset()
{
	SAFE_CLOSE_HANDLE(m_hThread);
	m_FileInfo.Reset();
	m_errCode = DLERR_SUCCESS;
	m_DownStat.Reset();

	{
		CObjGuard _guard_(m_lckFileDownloaderObject);
		m_fdFactory.ReleaseObject(&m_pIFileDownloader);
	}
}

unsigned int __stdcall KFileDownload::_RunThreadFun( void* lpThis )
{
	ATLASSERT(lpThis);
	KFileDownload *pThis = (KFileDownload*) lpThis;
	return pThis->Fetch(pThis->m_nCorrurent);
}
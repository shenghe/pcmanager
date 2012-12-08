#include "StdAfx.h"
#include "LocalFileDownload.h"
#include "FileStream.h"
#include <atlfile.h>

CLocalFileDownload::CLocalFileDownload(void)
{
	m_lpUserData = NULL;
	m_pObserver = NULL;
}

CLocalFileDownload::~CLocalFileDownload(void)
{
}

void CLocalFileDownload::Release()
{
	delete this;
}

VOID CLocalFileDownload::SetObserver( IHttpAsyncObserver *pObserver )
{
	m_pObserver = pObserver;
}

void CLocalFileDownload::SetDownloadInfo( LPCTSTR szUrl, LPCTSTR szFilePath )
{
	m_strUrl = szUrl;
	m_strFilePath = szFilePath;
}

LPVOID CLocalFileDownload::SetUserData( LPVOID lpUserData )
{
	m_lpUserData = lpUserData;
	return m_lpUserData;
}

BOOL CLocalFileDownload::Fetch( INT nCorrurent/*=0*/ )
{
	m_bStopped = FALSE;
	
	try
	{
		CFileInStream fin(m_strUrl);
		if(!fin.Create())
		{
			m_errCode = DLLER_SERVER_FILENOTFOUND;
			return FALSE;
		}

		CString strTmpFile;
		strTmpFile.Format(_T("%s%s"), m_strFilePath, _T(".tc"));
		CAtlFile file;	
		if( FAILED( file.Create(strTmpFile, GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS) ) )
		{
			m_errCode = DLERR_CREATEFILE;
			return FALSE;
		}
		
		m_errCode = DLERR_SUCCESS;
		m_FileInfo.Reset(fin.GetFileSize(), 0, TRUE);
		m_DownStat.OnDownBegin();
		
		int64 lastDownloaded = 0, downloadedPercent = m_FileInfo.fileSize/100;

		const int nBufferSize = 1024;
		BYTE *pBuffer = new BYTE[nBufferSize];
		while(!m_bStopped)
		{
			DWORD dwReaded = 0;
			fin.Read(pBuffer, nBufferSize, &dwReaded);
			if(dwReaded==0)
				break;
			
			DWORD dwWrited = 0;
			if( FAILED(file.Write(pBuffer, dwReaded, &dwWrited)) || dwWrited!=dwReaded)
			{
				m_errCode = DLERR_WRITEFILE;
				break;
			}

			m_FileInfo.fileDownloaded += dwReaded;			
			if((m_FileInfo.fileDownloaded-lastDownloaded) > downloadedPercent)
			{
				m_DownStat.OnDownData(GetTickCount(), (m_FileInfo.fileDownloaded-lastDownloaded));
				lastDownloaded = m_FileInfo.fileDownloaded;
				_Notify(ProcessState_ReceiveData);
			}
		}
		fin.CloseFile();
		file.Close();
		SAFE_DELETE_ARRAY(pBuffer);
		m_DownStat.OnDownEnd();
		
		if(m_FileInfo.fileDownloaded==m_FileInfo.fileSize)
		{
			MoveFileEx(strTmpFile, m_strFilePath, MOVEFILE_REPLACE_EXISTING);
			m_errCode = DLERR_SUCCESS;
		}
		else
		{
			DeleteFile(strTmpFile);
			m_errCode = DLLER_NETWORK;
		}
	}
	catch (...)
	{
		m_errCode = DLERR_WRITEFILE;
	}
	return DLERR_SUCCESS==m_errCode;
}

BOOL CLocalFileDownload::Start( INT nCorrurent/*=0*/ )
{
	ATLASSERT(FALSE);
	return FALSE;
}

void CLocalFileDownload::Stop()
{
	m_bStopped = TRUE;
}

void CLocalFileDownload::Wait()
{
	ATLASSERT(FALSE);
}

DLERRCode CLocalFileDownload::LastError()
{
	return m_errCode;
}

LPVOID CLocalFileDownload::GetUserData()
{
	return m_lpUserData;
}

void CLocalFileDownload::GetDownloadInfo( CString &strUrl, CString &strFilePath )
{
	strUrl = m_strUrl;
	strFilePath = m_strFilePath;
}


RemoteFileInfo * CLocalFileDownload::GetRemoteFileInfo()
{
	return &m_FileInfo;
}

IDownStat * CLocalFileDownload::GetDownloadStat()
{
	return &m_DownStat;
}

BOOL CLocalFileDownload::_Notify( ProcessState state, LPARAM lParam/*=0*/ )
{
	if(m_pObserver)
		return m_pObserver->OnHttpAsyncEvent(this, state, lParam);
	return TRUE;
}
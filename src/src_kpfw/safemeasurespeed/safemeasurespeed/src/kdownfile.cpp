#include "stdafx.h"
#include "kdownfile.h"
#include "Wininet.h"

#pragma comment(lib, "Wininet.lib")

CString strGetTempDirectory()
{
	WCHAR szFilePath[MAX_PATH] = {0};
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FLAG_CREATE | CSIDL_COMMON_APPDATA, NULL, 0, szFilePath)))
		return CString(szFilePath);
	else 
		return _T("");
}

int KDownFile::GetDownSpeed( IStatusCallback* pCallBack )
{
	//assert(m_pDownLoad == NULL);
	if (m_pDownLoad == NULL)
	{
		ATLVERIFY( SUCCEEDED(CreateDownloadObject(__uuidof(IDownload), (VOID**)&m_pDownLoad)) );
		if (m_pDownLoad == NULL)
			return -1;
	}


	CString strUrlAddr = _T("http://dl.ijinshan.com/safe/speet.dat");
//	CString strUrlAddr = _T("http://cd001.www.duba.net/duba/install/2011/ever/KAV101109_DOWN_99_10.exe");
//	CString strUrlAddr = _T("http://down.360safe.com/se/360se_3.5.exe");//http://cd001.www.duba.net/duba/install/2011/ever/KAV101109_DOWN_99_10.exe");
	CString strTempDirectory = strGetTempDirectory();
	if (strTempDirectory.IsEmpty())
		m_strDownFileSavePath.Format(_T("temp1.exe"));
	else
		m_strDownFileSavePath.Format(_T("%s\\temp1.exe"), strTempDirectory);
	

	if (::PathFileExists(m_strDownFileSavePath))
		::DeleteFile(m_strDownFileSavePath);

	m_strDownFileSavePathT.Format(_T("%s_kt"), m_strDownFileSavePath);
	if (::PathFileExists(m_strDownFileSavePathT))
		::DeleteFile(m_strDownFileSavePathT);
	
	m_pCallBack = pCallBack;


	m_pDownLoad->Stop();
	m_pDownLoad->SetObserver(this);
	m_pDownLoad->SetDownloadInfo(strUrlAddr, m_strDownFileSavePath);
//	m_pDownLoad->SetDownloadInfo(strUrlAddr2, strDownFileSavePath2);
	m_pDownLoad->Start(3);

	return 0;
}



BOOL KDownFile::OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam )
{
	IDownStat* pStat = pDownload->GetDownloadStat();
	if (pStat == NULL)	return FALSE;

	int nProcess = 0 ;
	RemoteFileInfo* pFileInfo = pDownload->GetRemoteFileInfo();
	if (pFileInfo != NULL)
		nProcess = (int)(pFileInfo->fileDownloaded* 100.0 / pFileInfo->fileSize);
	

	if (m_pCallBack != NULL)
	{
		int64 nAvgsize = pStat->AverageSpeed();		
		m_pCallBack->OnStatusCallback(state, pStat->Speed(), nAvgsize, nProcess);
	}
	
	if (state == ProcessState_Failed && m_pCallBack != NULL)
		m_pCallBack->OnStatusCallback(state, 0, 0, nProcess);
	
	return TRUE;
}

void KDownFile::StopDown( void )
{
	m_pDownLoad->Stop();
	DeleteFile(m_strDownFileSavePath);
}

void KDownFile::WaitDown( void )
{
	m_pDownLoad->Wait();
}	
#include "stdafx.h"
#include "DTManager.h"
#include "../../src_commonlib/libDownload2/MD5Checksum.h"
#include <atlfile.h>
#include <libdownload/libDownload.h>

CDTManager::CDTManager(void):_calls(NULL)
{
}

CDTManager::~CDTManager(void)
{
	POSITION p=_tks.GetStartPosition();
	while(p)
	{
		StopTask(_tks.GetNextKey(p));
	}
}

void CDTManager::Init( CAtlMap<DtStateChangeNotifyCallBack,void*>* calls )
{
	_calls=calls;
}

void* CDTManager::NewTask(CAtlArray<CString>* urls,CString md5,CString localfile)
{
	IDownload* down;
	CreateDownloadObject(__uuidof(IDownload),(void**)&down);
	_tks[down]=urls;
	size_t ppp=0;

	CString url;
	int64 len;
	int cu=0;
	if(GetContiInfo(localfile+L"c",url,len,cu))
	{
		for (;ppp<urls->GetCount()&&url!=(*urls)[ppp];ppp++)
		{
		};
		if(ppp==urls->GetCount())
		{
			ppp=0;
			DeleteFile(localfile+DOWNLOAD_TEMP_FILE_SUFFIX);
			DeleteFile(localfile+DOWNLOAD_TEMP_FILE_INFO_SUFFIX);
			DeleteFile(localfile+L"c");
		}
	}

	down->SetDownloadInfo((*urls)[ppp],localfile);
	LastDownInfo* lst=new LastDownInfo;
	lst->pos=ppp;
	lst->cur=cu;
	lst->urls=urls;
	lst->tk=down;
	_md5s[down]=md5;
	down->SetUserData((void*)lst);
	down->SetObserver(this);
	down->Start(cu);
	return down;
}

void CDTManager::ResumeTask( void* context )
{
	IDownload* down=(IDownload*)context;
	down->Start();
}




void __cdecl pause ( void * tk)
{
	CDTManager::LastDownInfo* lst=(CDTManager::LastDownInfo*)tk;
	lst->tk->Stop();
	CString url;
	CString lf;
	lst->tk->GetDownloadInfo(url,lf);
	CDTManager::SetContiInfo(lf+L"c",url,lst->tk->GetRemoteFileInfo()->fileSize,lst->cur);
	_endthread();
}

void CDTManager::PauseTask( void* context )
{
	IDownload* down=(IDownload*)context;
	LastDownInfo* lst=(LastDownInfo*)down->GetUserData();
	_beginthread(pause,0,lst);
}

void CDTManager::StopTask( void* context )
{
	IDownload* down=(IDownload*)context;
	down->Stop();
	LastDownInfo* lst=(LastDownInfo*)down->GetUserData();
	CString url;
	CString lf;
	down->GetDownloadInfo(url,lf);
	CDTManager::SetContiInfo(lf+L"c",url,lst->tk->GetRemoteFileInfo()->fileSize,lst->cur);
}

size_t CDTManager::GetTaskCount()
{
	return _tks.GetCount();
}

int CDTManager::GetTasks( CAtlArray<void*>& tasks )
{
	for (POSITION p=_tks.GetStartPosition();p;)
	{
		tasks.Add(_tks.GetNextKey(p));
	}
	return (int)tasks.GetCount();
}

void CDTManager::QueryTask( void* tk,InfoCallBack func,void* para )
{
	IDownload* down=(IDownload*)tk;
	if(func)
	{
		func(down->GetRemoteFileInfo()->fileDownloaded,down->GetRemoteFileInfo()->fileSize,down->GetDownloadStat()->Speed(),down->GetDownloadStat()->TimeUsed(),para);
	}
}

void __cdecl restry ( void * tk)
{
	CDTManager::LastDownInfo* lst=(CDTManager::LastDownInfo*)tk;
	lst->tk->Stop();
	if(lst->cur==0)
	{
		lst->tk->GetRemoteFileInfo()->Reset();
		lst->cur=1;
		lst->tk->Start(1);
		_endthread();
		return;
	}
	size_t index=lst->pos;
	if(index+1<lst->urls->GetCount())
	{
		lst->tk->GetRemoteFileInfo()->Reset();
		CString url;
		CString lf;
		lst->tk->GetDownloadInfo(url,lf);
		url=lst->urls->GetAt(++index);
		lst->pos=index;
		lst->tk->SetDownloadInfo(url,lf);
		DeleteFile(lf+DOWNLOAD_TEMP_FILE_SUFFIX);
		DeleteFile(lf+DOWNLOAD_TEMP_FILE_INFO_SUFFIX);
		DeleteFile(lf+L"c");
		if(url.Find(L"ijinshan.com")>=0)
		{
			lst->cur=1;
			lst->tk->Start(1);
		}
		else
		{
			lst->cur=0;
			lst->tk->Start();
		}
	}
	_endthread();
}


void __cdecl rest ( void * tk)
{
	CDTManager::LastDownInfo* lst=(CDTManager::LastDownInfo*)tk;
	lst->tk->Stop();
	CString url;
	CString lf;
	lst->tk->GetDownloadInfo(url,lf);
	lst->tk->GetRemoteFileInfo()->Reset();
	DeleteFile(lf+DOWNLOAD_TEMP_FILE_SUFFIX);
	DeleteFile(lf+DOWNLOAD_TEMP_FILE_INFO_SUFFIX);
	DeleteFile(lf+L"c");
	lst->cur=0;
	lst->pos=0;
	lst->tk->SetDownloadInfo((*(lst->urls))[lst->pos],lf);
	lst->tk->Start(lst->cur);
	_endthread();
}

BOOL CDTManager::OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam )
{
	if(_calls==NULL)
		return TRUE;

	if(ProcessState_Failed==state)
	{
		LastDownInfo* lst=(LastDownInfo*)pDownload->GetUserData();
		size_t index=lst->pos;
		if(index+1<_tks[pDownload]->GetCount())
		{
			_beginthread(restry,0,lst);
			return TRUE;
		}
	}
	DTManager_Stat st;

	if(ProcessState_EndRequestDone==state)
	{
		LastDownInfo* lst=(LastDownInfo*)pDownload->GetUserData();
		CString url;
		CString localfile;
		pDownload->GetDownloadInfo(url,localfile);

		if(pDownload->GetRemoteFileInfo()->bRandomAccess)
		{
			CString last_url;
			int64 len;
			int cu;
			if(GetContiInfo(localfile+L"c",last_url,len,cu))
			{
				if(len!=pDownload->GetRemoteFileInfo()->fileSize)
				{
					_beginthread(rest,0,lst);
				}
			}
			else
				SetContiInfo(localfile+L"c",url,pDownload->GetRemoteFileInfo()->fileSize,lst->cur);
		};
	}

	if(state==ProcessState_Finished)
	{
		CString url;
		CString localfile;
		pDownload->GetDownloadInfo(url,localfile);
		if((_md5s[pDownload])==L""||url.Find(L"ijinshan.com")<0)
			st=TASK_DONE;
		else
		{
			CString strCheckSum;
			try
			{
				strCheckSum =CMD5Checksum::GetMD5( localfile );
				strCheckSum.MakeUpper();
			}
			catch (...)
			{
				strCheckSum = _T("ERROR");
			};
			if((_md5s[pDownload]).CompareNoCase(strCheckSum) == 0 )
				st=TASK_DONE;
			else
				st=TASK_ERROR_MD5;
		}

	}
	else if (state==ProcessState_PartFinished)
	{
		st=TASK_DOWNING;
	}
	else if (state==ProcessState_StartConnectToServer)
	{
		st=TASK_CONECTING;
	}
	else if (state==ProcessState_Failed)
	{
		st=TASK_ERROR;
	}
	else
		st=TASK_DOWNING;

	for (POSITION p=_calls->GetStartPosition();p;)
	{
		CAtlMap<DtStateChangeNotifyCallBack,void*>::CPair* pr=_calls->GetNext(p);
		DtStateChangeNotifyCallBack func=pr->m_key;
		func(st,pDownload,pr->m_value);

	}
	return FALSE;
}

void __cdecl stop ( void * tk)
{
	CDTManager::LastDownInfo* lst=(CDTManager::LastDownInfo*)tk;
	lst->tk->Stop();
	CString url;
	CString lf;
	lst->tk->GetDownloadInfo(url,lf);
	DeleteFile(lf+DOWNLOAD_TEMP_FILE_SUFFIX);
	DeleteFile(lf+DOWNLOAD_TEMP_FILE_INFO_SUFFIX);
	DeleteFile(lf+L"c");
	delete lst->tk;
	delete lst;
}


void CDTManager::CancelTask( void* context )
{
	IDownload* down=(IDownload*)context;
	LastDownInfo* lst=(LastDownInfo*)down->GetUserData();
	_beginthread(stop,0,lst);
}

bool CDTManager::GetContiInfo( CString file,CString& url,int64& len,int& cur )
{
	CAtlFile f;
	if(ERROR_SUCCESS!=f.Create(file,FILE_GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,OPEN_EXISTING)!=ERROR_SUCCESS)
	{
		return false;
	}
	if(ERROR_SUCCESS!=f.Read(&len,sizeof len)&&f.Read(&cur,sizeof cur)!=ERROR_SUCCESS)
		return false;
	f.Read(&cur,sizeof cur);
	ULONGLONG flen=0;
	f.GetSize(flen);
	wchar_t* buf=new wchar_t[((size_t)flen)/2+1];
	ZeroMemory(buf,((size_t)flen)/2+1);
	f.Read(buf,(DWORD)flen);
	url=buf;
	delete [] buf;
	f.Close();
	return true;
}

bool CDTManager::SetContiInfo( CString file,CString& url,int64& len,int& cur )
{
	CAtlFile f;
	if(ERROR_SUCCESS!=f.Create(file,FILE_GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,CREATE_ALWAYS)!=ERROR_SUCCESS)
	{
		return false;
	}
	f.Write(&len,sizeof len);
	f.Write(&cur,sizeof cur);
	f.Write(url.GetBuffer(),url.GetLength()*2+2);
	f.Close();
	return true;
}

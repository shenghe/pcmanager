#pragma once
#include <atlstr.h>
#include <atlcoll.h>
#include <libDownload/libDownload.h>
#include <softmgr/IDTManager.h>


class CDTManager:public IHttpAsyncObserver,public IDTManager
{
public:
	CDTManager(void);
	virtual ~CDTManager(void);

	virtual void Init(CAtlMap<DtStateChangeNotifyCallBack,void*>* calls);
	
	virtual void* NewTask(CAtlArray<CString>* urls,CString md5,CString localfile);

	virtual void ResumeTask(void* context);

	virtual void PauseTask(void* context);

	virtual void StopTask(void* context);

	virtual void CancelTask(void* context);

	virtual size_t GetTaskCount();

	virtual int GetTasks(CAtlArray<void*>& tasks );
	virtual int GetTasks(GetTaskCallback func,void* para )
	{
		CAtlArray<void*> a;
		int ret=GetTasks(a);
		for (size_t i=0;i<a.GetCount();i++)
		{
			func(a[i],para);
		}
		return ret;
	}

	virtual void QueryTask(void* tk,InfoCallBack func,void* para);

	struct LastDownInfo 
	{
		size_t pos;//位置
		int cur;//线程数量
		CAtlArray<CString>* urls;
		IDownload* tk;
		CString md5;
	};

	static bool GetContiInfo(CString file,CString& url,int64& len,int& cur);
	static bool SetContiInfo( CString file,CString& url,int64& len,int& cur );

protected:

	virtual BOOL OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam );
	
private:
	CAtlMap<void*,CString> _md5s;
	CAtlMap<void*,CAtlArray<CString>*> _tks;
	CAtlMap<DtStateChangeNotifyCallBack,void*>* _calls;
};

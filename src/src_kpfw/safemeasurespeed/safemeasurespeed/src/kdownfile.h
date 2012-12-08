#pragma once
#include "libdownload/libDownload.h"

//回调函数
class IStatusCallback
{
public:
	/*
	* @funcn							--- OnStatusCallback
	* @param				enumState	---	当前状态
	* @param				nSpeed		---	当前下载的即时速度
	* @param				nAvgSpeed	---	下载文件的平均速度
	* @param				nProcess	---	当前下载的进度
	*/
	virtual void OnStatusCallback(ProcessState enumState, int64 nSpeed, int64 nAvgSpeed, int nProcess) = 0;

};


class KDownFile : public IHttpAsyncObserver
{
public:
	KDownFile(){	m_pDownLoad = NULL;		}
	~KDownFile()
	{	
		DeleteFile(m_strDownFileSavePath);	
		DeleteFile(m_strDownFileSavePathT);	
	}

public:
	//获取文件下载速度
	int GetDownSpeed(IStatusCallback* pCallBack);

	/*
	* @funcn							--- OnHttpAsyncEvent
	* @brief							--- http的回调函数
	* @param pDownload					--- 指定的对象
	* @param state						--- 指定的状态
	*/
	virtual BOOL OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam );

public:
	void StopDown(void);
	void WaitDown(void);
private:
	ULONG		m_uSpeed;			//即时下载速度
	ULONG		m_uAvgSpeed;		//平均下载速度
	IDownload*	m_pDownLoad;
	IStatusCallback* m_pCallBack;
	CString		m_strUrlAddr;
	CString		m_strDownFileSavePath;
	CString		m_strDownFileSavePathT;
	DWORD		m_dwBegin;
	DWORD		m_dwEnd;
	
};
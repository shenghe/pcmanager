#pragma once

#include <process.h>
#include <vector>
#include "Defines.h"
#include "FileStream.h"
#include <pubconfig/pubconfig.h>

enum EAsyncInState{
	EAsync_Idle,		// 继续等待
	EAsync_Opened,		// 已经被打开 
	EAsync_Data,		// 获取到数据了
	EAsync_Finished,	// 
	EAsync_Failed,		// 
};


class IAsyncIn
{
public:
	// operation 
	virtual BOOL Open(UrlInfo *location, int64 startPosition=0, int64 endPosition=0, int64 filesize=0) = 0;
	virtual BOOL Continue(int64 startPosition, int64 endPosition) = 0;
	virtual BOOL Close() = 0;
	virtual EAsyncInState Process() = 0;
	virtual BOOL Release() = 0;	// 释放!
	virtual BOOL SetProxyConfig(CPubConfig::T_ProxySetting &proxyconfig) = 0;
	
	// Get Info
	virtual HANDLE GetWaitHandle() = 0;
	virtual RemoteFileInfo GetRemoteFileInfo() = 0;
	virtual PVOID GetLastBuffer() = 0;		// 读取上次process 的buffer, 如果是EAsync_Data, EAsync_Finished
	virtual DWORD GetLastAsyncError() = 0;		// 上次异步错误 
	
	// Set Running Param 
	virtual VOID SetRetry(INT nMaxRetry) = 0;
};

class CRequestPool;
class CHttpAsyncIn : public IAsyncIn 
{
	friend class CRequestPool;
public:
	// 
	CHttpAsyncIn();
	~CHttpAsyncIn();
	
public:
	// operation 
	virtual BOOL Open(UrlInfo *location, int64 startPosition=0, int64 endPosition=0, int64 filesize=0);
	virtual BOOL Continue(int64 startPosition, int64 endPosition);
	virtual BOOL OpenRange(int64 startPosition, int64 endPosition);
	virtual BOOL ModifyRange(int64 startPosition, int64 endPosition);
	virtual BOOL Close();
	virtual EAsyncInState Process();
	virtual BOOL Release();	// 释放!
	virtual BOOL SetProxyConfig(CPubConfig::T_ProxySetting &proxyconfig);
	
	// Get Info
	virtual HANDLE GetWaitHandle();
	virtual RemoteFileInfo GetRemoteFileInfo();
	virtual PVOID GetLastBuffer();		// 读取上次process 的buffer, 如果是EAsync_Data, EAsync_Finished
	virtual DWORD GetLastAsyncError();		// 上次异步错误 
	virtual const UrlInfo *GetLocationInfo() const;
	
	// Set Running Param 
	virtual VOID SetRetry(INT nMaxRetry);
	virtual VOID SetFileSizeExpect(int64 filesize);

	// 测试最后一位!! 
	virtual BOOL IsLastValidBufferZeroBit() const;
	
protected:
	void _Reset();
	EAsyncInState _Process();
	BOOL _TryRetry(BOOL bSetRetry=TRUE);
	
	static VOID CALLBACK RequestCallback(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	VOID RequestCallback_(HINTERNET hInternet, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	BOOL _ConnectServer();
	void _CreateAdditionalHeaders( LPTSTR lpszHeaders, DWORD dwLen );
	BOOL _ReadRemoteFileInfo();
		
	DWORD _QueryHttpStatus( HINTERNET hRequest ) const;
	BOOL _QueryContentLength( HINTERNET hRequest, int64 &nContentLength ) const;
	BOOL _QueryRandomAccess( HINTERNET hRequest, int64 iContentLength ) const;
	BOOL _QueryHttpHeaderInfo(BOOL bReqeust);
	BOOL _QueryHttpRangeInfo( HINTERNET hRequest, int64 &iBegin, int64 &iEnd, int64 &iEntityLength );
	
protected:
	ProcessState m_state;
	HINTERNET m_hSession, m_hConnect, m_hRequest;
	UrlInfo m_ResourceLocation;
	RemoteFileInfo m_fileInfo;
	INTERNET_BUFFERSA InetBuff;
	CHAR *m_pBuffer;
		
	HANDLE m_hWaitEvent;
	HANDLE m_hWaitableTimer;
	DWORD  m_dwAsyncError;
	BOOL   m_bHandleValid;
	
	int64 m_iPositionBegin, m_iPositionEnd, m_iPositionCurrent;
	int64 m_iFilesizeExpect;
	INT m_nRetry, m_nMaxRetry;
	int64 m_iDownloaded;
	
	CObjLock m_objLock;


	// for test 
	DWORD	m_dwLastAsyncError;
	DWORD	m_dwLastReceivedByte;

	// 
	DWORD	m_dwLastValidBufferSize;
	BYTE	m_dwLastValidBufferFirstBit;

	CPubConfig::T_ProxySetting m_currentProxySetting;
};

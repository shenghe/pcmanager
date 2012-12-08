//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

#include <Unknwn.h>

//////////////////////////////////////////////////////////////////////////

#define KIS_LOG_DLL_NAME	L"KisLog.DLL"
#define MAX_LOG_FILE_SIZE	25 * 1024 * 1024	//KB,50M

//////////////////////////////////////////////////////////////////////////
//模块名称, 对应为SQLite表
#define LOG_APP			L"APP"
#define LOG_IP			L"IP"
#define LOG_SCAN		L"SCAN"
#define LOG_ARP			L"ARP"
#define LOG_WEB			L"WEB"
#define LOG_ADV_IP		L"ADV_IP"



//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

typedef enum
{
	LOG_MESSAGE_TYPE_NONE = 0,
	LOG_MESSAGE_TYPE_INFO
}LOG_MESSAGE_TYPE;

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////

extern const __declspec(selectany) GUID CLSID_ILogWriter =
{ 0xc5330f6, 0x987f, 0x4da8, { 0xb6, 0x87, 0x12, 0x76, 0xe3, 0xa7, 0x58, 0x70 } };

[
	uuid("0C5330F6-987F-4da8-B687-1276E3A75870")
]
interface IKisLogWriter : IUnknown
{	
	virtual BOOL STDMETHODCALLTYPE Init( const char *pLogPath,	const char *pcsModuleName,
		BOOL bWait	//TRUE:等待日志写完; FALSE:放入队列中，由工作线程写入
		) = 0;	
	virtual BOOL STDMETHODCALLTYPE UnInit() = 0;
	virtual BOOL STDMETHODCALLTYPE BeginTransaction() = 0;
	virtual BOOL STDMETHODCALLTYPE CommitTransaction() = 0;
	virtual BOOL STDMETHODCALLTYPE AddLog( LOG_MESSAGE_TYPE MessageType, const BYTE *pbData, int nDataLen ) = 0;
	virtual BOOL STDMETHODCALLTYPE RemoveLog( int nStartId , int nEndId ) = 0;
	virtual int STDMETHODCALLTYPE GetCount() = 0;
};

//////////////////////////////////////////////////////////////////////////

extern const __declspec(selectany) GUID CLSID_ILogReader =
{ 0xd6804d0, 0xfdda, 0x4bbe, { 0x92, 0x33, 0xd4, 0xc1, 0xdf, 0xad, 0x63, 0x94 } };

[
	uuid("0D6804D0-FDDA-4bbe-9233-D4C1DFAD6394")
]
interface IKisLogReader : IUnknown
{	
public:
	virtual ~IKisLogReader(){}
	virtual BOOL STDMETHODCALLTYPE Init( const char *szLogPath, const char* szModuleName ) = 0;

	virtual BOOL STDMETHODCALLTYPE UnInit() = 0;

	virtual BOOL STDMETHODCALLTYPE GetLog(
		__out LOG_MESSAGE_TYPE *pnMsgType,
		__out int *nID,
		__out SYSTEMTIME *pLogTime,
		__out BYTE *pbData, 
		__in __out int *pnDataLen
		) = 0;	
	virtual int STDMETHODCALLTYPE GetCount() = 0;
};

//////////////////////////////////////////////////////////////////////////
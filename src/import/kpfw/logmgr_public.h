///////////////////////////////////////////////////////////////	
//	
// FileName  :  logmgr_public.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-16  15:33
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#ifndef LOGMGR_PUBLIC_INC_
#define LOGMGR_PUBLIC_INC_

#include <unknwn.h>
#include "netwatch.h"

enum KpfwLogType
{
	LOG_TYPE_APP = 1,
	LOG_TYPE_LANMAN,
	LOG_TYPE_ARP,
	LOG_TYPE_IP,
	LOG_TYPE_ADV,
	LOG_TYPE_MODULE,
	LOG_TYPE_HTTP,
	LOG_TYPE_PE_DOWNLOAD,
	LOG_TYPE_ANTI_SCAN
};


//////////////////////////////////////////////////////////////////////////
interface ILogListOper
{
	virtual ~ILogListOper() {};
	virtual STDMETHODIMP PushStr(IN LPCWSTR lpstrName, IN LPCWSTR lpStrValue) = 0;
	virtual STDMETHODIMP PushInt(IN LPCWSTR lpstrName, IN ULONG lpStrValue) = 0;
	virtual STDMETHODIMP Get(OUT LPWSTR lpstrName, OUT int* nNameSize, OUT LPWSTR lpstrValue, OUT int* nValueSize) = 0;
	virtual STDMETHODIMP_(BOOL) Next() = 0;
	
};

interface ILogIterator
{
	virtual STDMETHODIMP Get(OUT ULONG *uLogID, OUT DWORD* tmLog, OUT void* pData, OUT DWORD* pnDataSize) = 0;
	virtual STDMETHODIMP_(BOOL) Next() = 0;
};


//////////////////////////////////////////////////////////////////////////

extern const __declspec(selectany) GUID CLSID_ILogMgrImp =
{ 0x16d52845, 0x2b4e, 0x4c84, { 0xb8, 0x6e, 0xbb, 0xe7, 0xe, 0x36, 0xe0, 0xf9 } };

[
	uuid(16D52845-2B4E-4c84-B86E-BBE70E36E0F9)
]
interface ILogMgr : IUnknown
{
	virtual STDMETHODIMP_(BOOL) Init( WCHAR* szLogPath, DWORD dwDBMaxSize = 0 ) = 0;
	virtual STDMETHODIMP_(BOOL) UnInit() = 0;
	virtual STDMETHODIMP_(BOOL) CreateLogList( ILogListOper** logList) = 0;
	virtual STDMETHODIMP_(BOOL) DelLogList( ILogListOper* logList) = 0;
	virtual STDMETHODIMP_(BOOL) AddLog( ULONG nLogType, void* pData, ULONG nDataSize) = 0;
	virtual STDMETHODIMP_(BOOL) AddKeyLog( ULONG nLogType, ULONG uKey, void* pData, ULONG nDataSize) = 0;
	virtual STDMETHODIMP_(BOOL) CreateIterator( ULONG nLogType, ILogIterator** logIterator, DWORD dwStartDate = 0, DWORD dwEndDate = 0 ) = 0;
	virtual STDMETHODIMP_(BOOL) CreateKeyIterator( ULONG nLogType, ULONG uKey, ILogIterator** logIterator, DWORD dwStartDate = 0, DWORD dwEndDate = 0 ) = 0;
	virtual STDMETHODIMP_(BOOL) DeleteIterator( ILogIterator* pLogIterator ) = 0;
};

//////////////////////////////////////////////////////////////////////////

#endif  // !LOGMGR_PUBLIC_INC_
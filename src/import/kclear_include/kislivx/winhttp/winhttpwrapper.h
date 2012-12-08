#pragma once
#include "../uplive_common.h"
#include "winhttpapi.h"
#include "../iseqstream.h"

enum enumKHttpStatus
{
	enumHttpStatus_Progress			= 1,
	enumHttpStatus_ErrorCode		= 2,
	enumHttpStatus_StatusCode		= 3,
	enumHttpStatus_ContentLength	= 4,
};

interface IStatusCallback
{
	virtual BOOL OnStatusCallback(enumKHttpStatus uStatus, int nValue) = 0;
};

class WinHttpWrapper
{
public:
	WinHttpWrapper(void);
	~WinHttpWrapper(void);
public:
	BOOL Initialize();
	BOOL Uninitialize();
public:
	long SendHttpRequest(LPCWSTR lpszUrl,
						 IOutStream * _pISeqOutStream,
						 IStatusCallback * _pIStatusCallback = NULL);
public:

protected:
	WinHttpApi WinHttp_;
};

//////////////////////////////////////////////////////////////////////////
// WinHttpHandle
class WinHttpHandle
{
public:
	WinHttpHandle(IN WinHttpApi& _WinHttp);
	~WinHttpHandle();
public:
	BOOL Attach(HINTERNET handle);
	
	HINTERNET Detach();
	
	void Close();

	HRESULT SetOption(DWORD option, const void* value, DWORD length);

	HRESULT QueryOption(DWORD option, void* value, DWORD& length) const;
protected:
	HINTERNET m_handle;
protected:
	WinHttpApi& WinHttp_;
};

//////////////////////////////////////////////////////////////////////////
// WinHttpSession
class WinHttpSession 
	: public WinHttpHandle
{
public:
	WinHttpSession(IN WinHttpApi& _WinHttp);
public:
	HRESULT Initialize();
};

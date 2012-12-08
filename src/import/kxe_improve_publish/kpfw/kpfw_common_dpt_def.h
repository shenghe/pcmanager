////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : kpfw_common_dpt_def.h
//      Version   : 1.0
//      Comment   : 定义网镖服务和界面之间的转HTTP_RPC接口定义文件
//      
//      Create at : 2009-07-09
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include <string>
#include "../../src/import/kpfw/kpfw_common_interface_ex.h"
#include "include/idl.h"
#include "include/kxe_serialize.h"
#include "kxeproduct/kxeproduct_id.h"

//2010 框架 不支持 __int64 要自己写代码转换！！！
#define MAKEINT64(l, h)		((INT64) (((DWORD)((INT64)(l) & 0xffffffff)) | ((INT64)((DWORD)((INT64)(h) & 0xffffffff))) << 32))
#define LOINT64(l)          ((DWORD)((INT64)(l) & 0xffffffff))
#define HIINT64(l)          ((DWORD)((INT64)(l) >> 32))


// kxe_common_param  / ret
typedef struct tagHRESULTRet
{
	HRESULT hr;

	inline tagHRESULTRet& operator=(const HRESULT& val)
	{
		hr = val;
		return *this;
	}
} Kxe_HRESULT_Ret;

KXE_JSON_DDX_BEGIN(Kxe_HRESULT_Ret)
KXE_JSON_DDX_MEMBER("hr", hr);
KXE_JSON_DDX_END()


typedef struct tagInit64Param 
{
	DWORD dwHigh;
	DWORD dwLow;

} Kxe_Init64_Param;


KXE_JSON_DDX_BEGIN(Kxe_Init64_Param)
KXE_JSON_DDX_MEMBER("dwHigh", dwHigh);
KXE_JSON_DDX_MEMBER("dwLow", dwLow);
KXE_JSON_DDX_END()


typedef struct tagVOIDParam
{
} Kxe_VOID_Param;

KXE_JSON_DDX_BEGIN(Kxe_VOID_Param)
KXE_JSON_DDX_END()


typedef struct tagStringParam
{
	std::wstring	str;

	inline tagStringParam& operator=(const std::wstring& val)
	{
		str = val;
		return *this;
	}

	inline tagStringParam& operator=(LPCWSTR& val)
	{
		str = val;
		return *this;
	}
} Kxe_String_Param;

KXE_JSON_DDX_BEGIN(Kxe_String_Param)
KXE_JSON_DDX_MEMBER("str", str);
KXE_JSON_DDX_END()


typedef struct tagStringHResultRet
{
	std::wstring	str;
	HRESULT			hr;
} Kxe_StringHResult_Ret;

KXE_JSON_DDX_BEGIN(Kxe_StringHResult_Ret)
KXE_JSON_DDX_MEMBER("str", str);
KXE_JSON_DDX_MEMBER("hr", hr);
KXE_JSON_DDX_END()


typedef struct tagBOOLParam
{
	BOOL bParam;

	inline tagBOOLParam& operator=(BOOL& val)
	{
		bParam = val;
		return *this;
	}
} Kxe_BOOL_Param;

KXE_JSON_DDX_BEGIN(Kxe_BOOL_Param)
KXE_JSON_DDX_MEMBER("bParam", bParam);
KXE_JSON_DDX_END()


typedef struct tagINTParam
{
	INT	nParam;
	inline tagINTParam& operator=(INT& val)
	{
		nParam = val;
		return *this;
	}
} Kxe_INT_Param;

KXE_JSON_DDX_BEGIN(Kxe_INT_Param)
KXE_JSON_DDX_MEMBER("nParam", nParam);
KXE_JSON_DDX_END()


typedef struct tagINTRet
{
	INT	nRet;
	inline tagINTRet& operator=(const INT& val)
	{
		nRet = val;
		return *this;
	}
} Kxe_INT_Ret;

KXE_JSON_DDX_BEGIN(Kxe_INT_Ret)
KXE_JSON_DDX_MEMBER("nRet", nRet);
KXE_JSON_DDX_END()


typedef struct tagINTHResultRet
{
	INT		nRet;
	HRESULT hr;
} Kxe_INTHResult_Ret;

KXE_JSON_DDX_BEGIN(Kxe_INTHResult_Ret)
KXE_JSON_DDX_MEMBER("nRet", nRet);
KXE_JSON_DDX_MEMBER("hr",	hr);
KXE_JSON_DDX_END()


typedef struct tagBOOLRet
{
	BOOL bRet;
	inline tagBOOLRet& operator=(const BOOL& val)
	{
		bRet = val;
		return *this;
	}
} Kxe_BOOL_Ret;

KXE_JSON_DDX_BEGIN(Kxe_BOOL_Ret)
KXE_JSON_DDX_MEMBER("bRet", bRet);
KXE_JSON_DDX_END()


typedef struct tagBOOLHResultRet
{
	BOOL		bRet;
	HRESULT		hr;
} Kxe_BOOLHResult_Ret;

KXE_JSON_DDX_BEGIN(Kxe_BOOLHResult_Ret)
KXE_JSON_DDX_MEMBER("bRet", bRet);
KXE_JSON_DDX_MEMBER("hr",	hr);
KXE_JSON_DDX_END()

typedef struct tagDWORDRet
{
	DWORD dwRet;
	inline tagDWORDRet& operator=(const DWORD& val)
	{
		dwRet = val;
		return *this;
	}
} Kxe_DWORD_Ret;

KXE_JSON_DDX_BEGIN(Kxe_DWORD_Ret)
KXE_JSON_DDX_MEMBER("dwRet", dwRet);
KXE_JSON_DDX_END()



typedef struct tagDWORDHResultRet
{
	DWORD		dwRet;
	HRESULT		hr;
} Kxe_DWORDHResult_Ret;

KXE_JSON_DDX_BEGIN(Kxe_DWORDHResult_Ret)
KXE_JSON_DDX_MEMBER("dwRet", dwRet);
KXE_JSON_DDX_MEMBER("hr",	hr);
KXE_JSON_DDX_END()


typedef struct tagDWORDParam
{
	DWORD dwRet;
	inline tagDWORDParam& operator=(DWORD& val)
	{
		dwRet = val;
		return *this;
	}
} Kxe_DWORD_Param;

KXE_JSON_DDX_BEGIN(Kxe_DWORD_Param)
KXE_JSON_DDX_MEMBER("dwRet", dwRet);
KXE_JSON_DDX_END()


typedef struct tagULONGParam
{
	ULONG	ulParam;
} Kxe_ULONG_Param;

KXE_JSON_DDX_BEGIN(Kxe_ULONG_Param)
KXE_JSON_DDX_MEMBER("ulParam", ulParam);
KXE_JSON_DDX_END()

///////////////////////////////////


//kxe_xxx_param / ret

//KProcFluxItemColl->

typedef struct tagKpfwFluxParam
{
	Kxe_Init64_Param	nRec;
	Kxe_Init64_Param	nSend;

	inline tagKpfwFluxParam& operator=(const KPFWFLUX& val)
	{
		nRec = *((Kxe_Init64_Param*)&val.nRec);
		nSend = *((Kxe_Init64_Param*)&val.nSend);
		return *this;
	}
} Kxe_KpfwFlux_Param;

KXE_JSON_DDX_BEGIN(Kxe_KpfwFlux_Param)
KXE_JSON_DDX_MEMBER("nRec", nRec);
KXE_JSON_DDX_MEMBER("nSend", nSend);
KXE_JSON_DDX_END()


typedef struct tagTrustModeParam
{
	std::wstring		strFile;
	DWORD				dwRet;
} Kxe_TrustMode_Param;

KXE_JSON_DDX_BEGIN(Kxe_TrustMode_Param)
KXE_JSON_DDX_MEMBER("strFile",		strFile);
KXE_JSON_DDX_MEMBER("dwRet",		dwRet);
KXE_JSON_DDX_END()
//FluxInfo ->


//LogSize -->
typedef struct tagLogSizeRet
{
	Kxe_Init64_Param	sizeRet;
	HRESULT	hr;
} Kxe_LogSize_Ret;

KXE_JSON_DDX_BEGIN(Kxe_LogSize_Ret)
KXE_JSON_DDX_MEMBER("sizeRet",		sizeRet);
KXE_JSON_DDX_MEMBER("hr",			hr);
KXE_JSON_DDX_END()
//<-- LogSize
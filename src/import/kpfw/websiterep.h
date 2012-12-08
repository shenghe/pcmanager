////////////////////////////////////////////////////////////////////////////////
//      
//      File for WebSitRep
//      
//      File      : WebSiteRep
//      Version   : 1.0
//      Comment   : 网址可信认证接口定义
//      
//      Create at : 2008-10-9
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 

#ifndef __WEB_SITE_REP_H__
#define __WEB_SITE_REP_H__

#include "scom/SCOM/SCOMDef.h"

#define WEB_SITE_TRUST_DLL		TEXT("security\\kpfw\\kwebsiterep.dll")

//////////////////////////////////////////////////////////////////////////
enum eWebSiteTrustLevel 
{
	Web_Trust_Unknown = 0,				// 未知
	Web_Trust_Normal = 1,				// 白
	Web_Trust_Doultful = 2,				// 灰
	Web_Trust_Danger  = 3,				// 黑
};

[
	uuid("8C354CA4-2400-4333-841E-1FF5020A411B")
]
interface IUrlProvider: public IUnknown
{
	virtual STDMETHODIMP_(INT) GetUrlCount()				PURE;

	virtual STDMETHODIMP_(const WCHAR*) GetUrlMd5(INT index)	PURE;

	virtual STDMETHODIMP SetUrlInfo(const char* purl, const char* phit_key, eWebSiteTrustLevel nTrustLevel) PURE;

	virtual STDMETHODIMP_(const WCHAR*) GetUrlBase64(INT index)	PURE;

	virtual STDMETHODIMP_(const char*) GetUrl(INT index)	PURE;
};

[
	uuid("05BD2ADA-964B-4765-BD35-1638A275597A")
]
interface IUrlTrustRep: public IUnknown
{
	virtual STDMETHODIMP SetUrlProvider(IUrlProvider* pProvider)	PURE;

	virtual STDMETHODIMP SetTimeout(DWORD dwMilliSecs)				PURE;

	virtual STDMETHODIMP SendHttpRequest(LPCWSTR lpServer = NULL, DWORD nPort = -1)	PURE;

	virtual STDMETHODIMP UpdateUrlPackage(LPCSTR strVersion, INT topn, LPWSTR strUpdateFile)	PURE;
};

_declspec(selectany) extern const KSCLSID CLSID_KisUrlProvider = { 0xf626ff8f, 0x1c73, 0x426d, { 0x8f, 0x84, 0xfc, 0xc3, 0x6e, 0x7, 0x56, 0xf5 } };
_declspec(selectany) extern const KSCLSID CLSID_KisUrlTrustRepImpl = { 0x3e10d61d, 0x7a5c, 0x4fbe, { 0xad, 0x84, 0x5b, 0x6c, 0xb7, 0xf0, 0x3e, 0xb4 } };

//////////////////////////////////////////////////////////////////////////
// ip地址转换为地理地址应用

[
	uuid("7F042131-0324-482a-B3B5-9032DEDB61DE")
]
interface IIpProvider: public IUnknown
{
	virtual STDMETHODIMP_(INT) GetIPCount()				PURE;

	virtual STDMETHODIMP_(LPCWSTR) GetIpAddr(INT index)	PURE;

	virtual STDMETHODIMP SetIpInfo(DWORD ipAddr, LPCWSTR strGeoAddr) PURE;
};

[
	uuid("7854BD27-8D83-47b0-9CB3-FDA984FD89C7")
]
interface IIpToGeoAddr: public IUnknown
{
	virtual STDMETHODIMP SetIpProvider(IIpProvider* pProvider)	PURE;

	virtual STDMETHODIMP SetTimeout(DWORD dwMilliSecs)				PURE;

	virtual STDMETHODIMP SendHttpRequest(LPCWSTR lpServer = NULL, DWORD nPort = -1)	PURE;
};

_declspec(selectany) extern const KSCLSID CLSID_IpAddrProvider = { 0x7be93792, 0x447, 0x46b0, { 0xa2, 0x35, 0xab, 0x56, 0xa, 0xa8, 0xa1, 0x18 } };
_declspec(selectany) extern const KSCLSID CLSID_IpToGeoAddrImpl = { 0x7854bd27, 0x8d83, 0x47b0, { 0x9c, 0xb3, 0xfd, 0xa9, 0x84, 0xfd, 0x89, 0xc7 } };

#endif // __WEB_SITE_REP_H__
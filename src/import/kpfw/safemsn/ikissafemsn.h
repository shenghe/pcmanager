#pragma once

#include "SCOM/SCOM/SCOMDef.h"

#define E_NOFIND_MSN	-2
#define E_NOFIND_DLL	-3

#define KIS_SAFEMSN_EVENT_NAME	L"B827C2AC-0088-4615-8BBD-439AB3ED64CB"
#define SUB_REGISTER_KEY	L"SOFTWARE\\kissafemsn"
#define SAFE_MSN_DLL_PATH	TEXT("SafeMsnPath")
#define ENABLE_SAFE_MSN L"EnableMsn"
#define BOOL_FALSE		L"False"
#define BOOL_TRUE		L"True"



[
	uuid("A121AA2D-8CF5-44a9-BCF2-7AC3A9526387")
]
interface IKisSafeMsn:public IUnknown
{
	virtual STDMETHODIMP SetInstallPath() = 0;

	/**
	*@return E_NOFIND_MSN: 没有找到MSN,没有MSN的注册表项，或MSN 的 exe文件不存在
	*@return E_FAIL: 拷贝文件失败
	*@return S_OK:安装成功
	*/
	virtual STDMETHODIMP Install() = 0;

	/**
	*@return E_NOFIND_MSN: 没有找到MSN,没有MSN的注册表项，或MSN 的 exe文件不存在
	*@return E_FAIL: 删除文件失败
	*@return S_OK:卸载成功
	*/
	virtual STDMETHODIMP UnInstall() = 0;

	/**
	*@return FALSE: MSN加密模块没有安装
	*@return TRUE: MSN加密模块已经安装
	*/
	virtual STDMETHODIMP isInstalled() = 0;

	/**
	*@brief  配置MSN加密模块
	*@remark  key = ENABLE_SAFE_MSN  value = BOOL_FALSE	  关闭加密
	    	  key = ENABLE_SAFE_MSN  value = BOOL_TRUE	  开启加密
	*@return ERROR_INVALID_PARAMETER: 参数错误
	*@return E_FAIL: 设置失败
	*@return S_OK: 设置成功
	*/
	virtual STDMETHODIMP SetConfig(
		/*[in]*/TCHAR *szKey, 
		/*[in]*/TCHAR *szValue) = 0;

	/**
	*@brief  获取MSN加密模块配置
	*@return ERROR_INVALID_PARAMETER: 参数错误
	*@return E_FAIL: 获取失败
	*@return S_OK: 获取成功
	*@remark dwValueLen 做为输入参数，大小应该为 strlen(_value)*sizeof(TCHAR)
	*/
	virtual STDMETHODIMP GetConfig(
		/*[in]*/TCHAR *szKey, 
		/*[out]*/TCHAR *szValue, 
		/*[in,out]*/DWORD &dwValueLen) = 0;		
	
	virtual STDMETHODIMP_(BOOL) IsRunning() = 0;
};

_declspec(selectany) extern const KSCLSID CLSID_KisSafeMsn = { 0x2bd8d816, 0xb04a, 0x44ef, { 0x9f, 0x30, 0xf7, 0x20, 0x63, 0xb6, 0x6c, 0xba }  };
//////////////////////////////////////////////////////////////////////
///		@file		kxeserv_config.h
///		@author		luopeng
///		@date		2008-10-06 09:07:58
///
///		@brief		
//////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include "../kxeconfig/kxeconfig.h"
#include "../scom/scom/scombase.h"

/**
 * @defgroup kxeservice_manager_config_data_struct_group KXEngine Service Manager Config Data Struct Define
 * @{
 */

/// 服务提供者列表配置
typedef struct _KXE_SERV_SERVICE_PROVIDER_LIST
{
	std::wstring strName;  ///< key
	std::list<KSCLSID> listProviderList; ///< 服务提供者列表
} KXE_SERV_SERVICE_PROVIDER_LIST;

#if _MSC_VER > 1200
KXE_CFG_DDX_DEFINE(KSCLSID);
#endif

KXE_CFG_DDX_BEGIN(KXE_SERV_SERVICE_PROVIDER_LIST, strName)
	KXE_CFG_DDX_MEMBER("ProviderList", listProviderList)
KXE_CFG_DDX_END()

/// Plugin配置
typedef struct _KXE_SERV_PLUGIN_DLL
{
	std::wstring strPluginName; ///< Key & PluginName
	std::wstring strPluginDllPath; ///< Plugin的路径
} KXE_SERV_PLUGIN_DLL;

KXE_CFG_DDX_BEGIN(KXE_SERV_PLUGIN_DLL, strPluginName)
	KXE_CFG_DDX_MEMBER_DEFAULT("Path", strPluginDllPath, L"")
KXE_CFG_DDX_END()

/// Plugin列表配置
typedef struct _KXE_SERV_PLUGIN_DLL_LIST
{
	std::wstring strName;  ///< Key
	std::list<KXE_SERV_PLUGIN_DLL> listPlugin;  ///< Plugin列表
} KXE_SERV_PLUGIN_DLL_LIST;

KXE_CFG_DDX_BEGIN(KXE_SERV_PLUGIN_DLL_LIST, strName)
	KXE_CFG_DDX_MEMBER("PluginList", listPlugin)
KXE_CFG_DDX_END()


__declspec(selectany) const extern GUID NULL_GUID = 
	{ 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };


/// 当前产品的配置
typedef struct _KXE_PRODUCT_CONFIG
{
	std::wstring strProductID;                     ///< 该产品的产品标志
	std::wstring strParentProductID;               ///< 该产品的父产品标志，若存在，则非独立产品
	std::wstring strRecommendProductID;            ///< 推荐该产品的产品标志
	std::wstring strProductPath;                   ///< 产品安装的目录
	std::wstring strPluginPath;                    ///< PluginPath
	std::wstring strWebDirectoryPath;              ///< WebPath
#if _MSC_VER > 1200
	__int64 nInstallTime;                          ///< 安装时间	
#else
	int nInstallTime;                          ///< 安装时间	
#endif
	int nInstallPosition;                          ///< 安装的顺序，其实就是说第几个安装
	std::list<KSCLSID> listChildProduct;           ///< 该产品的子产品标志
	std::list<KSCLSID> listComponent;              ///< 该产品所有的组件
	std::list<KSCLSID> listServiceProvider;        ///< 该产品所有的服务提供者组件
} KXE_PRODUCT_CONFIG;

KXE_CFG_DDX_BEGIN(KXE_PRODUCT_CONFIG, strProductID)
	KXE_CFG_DDX_MEMBER_DEFAULT("ParentProduct", strParentProductID, L"")
	KXE_CFG_DDX_MEMBER_DEFAULT("RecommendProduct", strRecommendProductID, L"")
	KXE_CFG_DDX_MEMBER_DEFAULT("ProductPath", strProductPath, L"")
	KXE_CFG_DDX_MEMBER_DEFAULT("PluginPath", strPluginPath, L"")
	KXE_CFG_DDX_MEMBER_DEFAULT("WebPath", strWebDirectoryPath, L"")
	KXE_CFG_DDX_MEMBER_DEFAULT("InstallTime", nInstallTime, 0)	
	KXE_CFG_DDX_MEMBER_DEFAULT("InstallPosition", nInstallPosition, 0)
	KXE_CFG_DDX_MEMBER("ChildProductList", listChildProduct)
	KXE_CFG_DDX_MEMBER("ComponentList", listComponent)
	KXE_CFG_DDX_MEMBER("ServiceProviderList", listServiceProvider)
KXE_CFG_DDX_END()

#if _MSC_VER == 1200
inline int kxe_config_write_config(kxe_config_key_t key,
								   const wchar_t* pwszPath,
								   const std::list<KXE_PRODUCT_CONFIG>& listValue)
{
	kxe_config_key_t newKey = NULL;
	
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	
	
	std::list<KXE_PRODUCT_CONFIG>::const_iterator iter = listValue.begin();
	for (; iter != listValue.end(); ++iter)
	{
		nRet = kxe_config_write_config(
			key,
			NULL,
			*iter
			);
		if (nRet != 0)
		{
			break;
		}
	}
	
	if (newKey)
	{
		kxe_config_close_key(newKey);
	}
	
	return nRet;
}

inline int kxe_config_read_config(kxe_config_key_t key,
								  const wchar_t* pwszPath, 
								  std::list<KXE_PRODUCT_CONFIG>& listValue)
{
	kxe_config_key_t newKey = NULL;
	
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	
	
	std::list<std::wstring> listKey;
	nRet = kxe_config_read_key(key, NULL, listKey);
	if (nRet == 0)
	{
		std::list<std::wstring>::iterator iter = listKey.begin();
		
		KXE_PRODUCT_CONFIG value;
		for (; iter != listKey.end(); ++iter)
		{
			KxEConfigSetKeyValue(value, *iter);
			int nRet = kxe_config_read_config(
				key,
				NULL,
				value
				);
			if (nRet != 0)
			{
				break;
			}
			listValue.push_back(value);
		}
	}
	
	if (newKey)
	{
		kxe_config_close_key(newKey);
	}
	
	return nRet;
}

inline int kxe_config_write_config(kxe_config_key_t key,
								   const wchar_t* pwszPath,
								   const std::list<KXE_SERV_PLUGIN_DLL>& listValue)
{
	kxe_config_key_t newKey = NULL;
	
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	
	
	std::list<KXE_SERV_PLUGIN_DLL>::const_iterator iter = listValue.begin();
	for (; iter != listValue.end(); ++iter)
	{
		nRet = kxe_config_write_config(
			key,
			NULL,
			*iter
			);
		if (nRet != 0)
		{
			break;
		}
	}
	
	if (newKey)
	{
		kxe_config_close_key(newKey);
	}
	
	return nRet;
}

inline int kxe_config_read_config(kxe_config_key_t key,
								  const wchar_t* pwszPath, 
								  std::list<KXE_SERV_PLUGIN_DLL>& listValue)
{
	kxe_config_key_t newKey = NULL;
	
	int nRet = 0;
	if (pwszPath)
	{
		nRet = kxe_base_create_config_key(
			key,
			pwszPath,
			&newKey
			);
		if (nRet != 0)
		{
			return nRet;
		}
		key = newKey;
	}
	
	
	std::list<std::wstring> listKey;
	nRet = kxe_config_read_key(key, NULL, listKey);
	if (nRet == 0)
	{
		std::list<std::wstring>::iterator iter = listKey.begin();
		
		KXE_SERV_PLUGIN_DLL value;
		for (; iter != listKey.end(); ++iter)
		{
			KxEConfigSetKeyValue(value, *iter);
			int nRet = kxe_config_read_config(
				key,
				NULL,
				value
				);
			if (nRet != 0)
			{
				break;
			}
			listValue.push_back(value);
		}
	}
	
	if (newKey)
	{
		kxe_config_close_key(newKey);
	}
	
	return nRet;
}
#endif

/// 产品列表配置
typedef struct _KXE_PRODUCT_CONFIG_LIST
{
	std::wstring strName;  ///< 键名
	std::list<KXE_PRODUCT_CONFIG> listProductConfig;  ///< 产品列表
} KXE_PRODUCT_CONFIG_LIST;

KXE_CFG_DDX_BEGIN(KXE_PRODUCT_CONFIG_LIST, strName)
	KXE_CFG_DDX_MEMBER("ProductList", listProductConfig)
KXE_CFG_DDX_END()

/**
 * @}
 */
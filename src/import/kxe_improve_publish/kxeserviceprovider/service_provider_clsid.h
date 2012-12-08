//////////////////////////////////////////////////////////////////////
///		@file		service_provider_clsid.h
///		@author		luopeng
///		@date		2008-9-17 09:07:58
///	
///		@brief		Service Provider的定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../scom/scom/scombase.h"

/**
 * @defgroup kxe_service_provider_clsid_group KXEngine Service Provider CLSID
 * @{
 */

/**
* @brief Base Service的SCOM组件的CLSID
*  {63D0B2F5-1B6D-4740-8A11-391DA258D1B9}
*/
__declspec(selectany) KSCLSID CLSID_KxEBaseServiceProvider = 
	{ 0x63d0b2f5, 0x1b6d, 0x4740, { 0x8a, 0x11, 0x39, 0x1d, 0xa2, 0x58, 0xd1, 0xb9 } };


/**
* @brief 扫描子系统的服务提供者SCOM组件的CLSID
*  {9D740557-8748-413f-9B9E-381E37FB2B2D}
*/
__declspec(selectany) KSCLSID CLSID_KxEScanServiceProvider = 
	{ 0x9d740557, 0x8748, 0x413f, { 0x9b, 0x9e, 0x38, 0x1e, 0x37, 0xfb, 0x2b, 0x2d } };

/**
* @brief 文件监控子系统的服务提供者SCOM组件的CLSID
*  {08D85372-7963-4338-8962-C51330BDD472}
*/
__declspec(selectany) KSCLSID CLSID_KxEFileMonServiceProvider = 
	{ 0x8d85372, 0x7963, 0x4338, { 0x89, 0x62, 0xc5, 0x13, 0x30, 0xbd, 0xd4, 0x72 } };

/**
* @brief 泡泡子系统的服务提供者SCOM组件的CLSID
*  {E874B30B-0581-45d6-A4C9-BDFBD9C4BC0F}
*/
__declspec(selectany) KSCLSID CLSID_KxEPopoServiceProvider = 
	{ 0xe874b30b, 0x581, 0x45d6, { 0xa4, 0xc9, 0xbd, 0xfb, 0xd9, 0xc4, 0xbc, 0xf } };

/**
* @brief 白名单子系统的服务提供者SCOM组件的CLSID
*  {E71D6FC6-6FD6-444c-9E3F-CA87C73AFFD6}
*/
__declspec(selectany) KSCLSID CLSID_KxEWhiteSerivceProvider = 
	{ 0xe71d6fc6, 0x6fd6, 0x444c, { 0x9e, 0x3f, 0xca, 0x87, 0xc7, 0x3a, 0xff, 0xd6 } };

/**
* @brief 系统服务提供者SCOM组件的CLSID
*  {1938775F-5977-4a95-9D26-8EA271F14D01}
*/
__declspec(selectany) KSCLSID CLSID_KxESystemSerivceProvider = 
	{ 0x1938775f, 0x5977, 0x4a95, { 0x9d, 0x26, 0x8e, 0xa2, 0x71, 0xf1, 0x4d, 0x1 } };

/**
* @brief 升级服务提供者SCOM组件的CLSID
*  {0159AC0E-7187-4e88-9795-DAFFC32A1C33}
*/
__declspec(selectany) KSCLSID CLSID_KxEUpdateSerivceProvider = 
	{ 0x159ac0e, 0x7187, 0x4e88, { 0x97, 0x95, 0xda, 0xff, 0xc3, 0x2a, 0x1c, 0x33 } };

/**
 * @brief Passport服务提供者SCOM组件的CLSID
 *  {FBE931D7-9397-444c-B1B8-B5645C3C02EB}
 */
__declspec(selectany) KSCLSID CLSID_KxEPassportSerivceProvider = 
	{ 0xfbe931d7, 0x9397, 0x444c, { 0xb1, 0xb8, 0xb5, 0x64, 0x5c, 0x3c, 0x2, 0xeb } };

/**
 * @brief 漏洞扫描服务提供者SCOM组件的CLSID
 *  {622BA59E-8C91-40d8-B00A-BA1F8B09D235}
 */
__declspec(selectany) KSCLSID CLSID_KxELeakScan = 
	{ 0x622ba59e, 0x8c91, 0x40d8, { 0xb0, 0xa, 0xba, 0x1f, 0x8b, 0x9, 0xd2, 0x35 } };



/**
* @brief 防御服务提供者SCOM组件的CLSID
*  {BCADEB71-895B-4acb-AF49-77C46A717E25}
*/
__declspec(selectany) KSCLSID CLSID_KxEDefendEngineSp = 
{ 0xafc6301b, 0x6dc, 0x47fc, { 0xac, 0x36, 0xb6, 0xbe, 0xfa, 0xb6, 0xfe, 0xe3 } };


/**
* @brief 病毒名字翻译服务提供者SCOM组件的CLSID
*  {BCADEB71-895B-4acb-AF49-77C46A717E25}
*/
__declspec(selectany) KSCLSID CLSID_KNameInfoSP =
{0xBCADEB71 ,0x895B ,0x4acb , {0xAF , 0x49 , 0x77 , 0xC4 , 0x6A , 0x71 , 0x7E , 0x25}};


/**
* @brief 杂项服务提供者SCOM组件的CLSID
*  {A144FDE6-D5F1-4ffb-9319-B5E542A926BB}
*/
__declspec(selectany) KSCLSID CLSID_KxEMiscServiceProvider =
{ 0xa144fde6, 0xd5f1, 0x4ffb, { 0x93, 0x19, 0xb5, 0xe5, 0x42, 0xa9, 0x26, 0xbb } };
/**

/**
* @brief 安全中心服务提供者的CLSID
*  {F3252288-3F54-4525-AF0B-22E66D94AE6B}
*/
__declspec(selectany) KSCLSID CLSID_KxESecurityCenterSP = 
{ 0xf3252288, 0x3f54, 0x4525, { 0xaf, 0xb, 0x22, 0xe6, 0x6d, 0x94, 0xae, 0x6b } };


/**
* @brief 统一升级提供者的CLSID
*  {E360C17E-E0F8-40f2-B2AE-190977153FC4}
*/
__declspec(selectany) KSCLSID CLSID_KxEUniUpdateSP = 
{ 0xe360c17e, 0xe0f8, 0x40f2, { 0xb2, 0xae, 0x19, 0x9, 0x77, 0x15, 0x3f, 0xc4 } };

/**
* @brief 统一升级，升级服务提供者的CLSID
*  {7262F8CB-EE0E-40d5-8ACE-538D571677A1}
*/
__declspec(selectany) KSCLSID CLSID_KxEUniUpdateServiceSP = 
{ 0x7262f8cb, 0xee0e, 0x40d5, { 0x8a, 0xce, 0x53, 0x8d, 0x57, 0x16, 0x77, 0xa1 } };

/**
* @brief 系统修复服务提供者SCOM组件的CLSID
*  {37C13C06-0155-4642-966A-669814ABCE72}
*/
__declspec(selectany) KSCLSID CLSID_KxESysRepairServiceProvider = 
{ 0x37c13c06, 0x155, 0x4642, { 0x96, 0x6a, 0x66, 0x98, 0x14, 0xab, 0xce, 0x72 } };

/**
* @brief 启动项扫描服务提供者SCOM组件的CLSID
*  {E8EDE567-5182-4bb6-89F6-5750DAF770F4}
*/
__declspec(selectany) KSCLSID CLSID_KxEAutorunSP = 
{ 0xe8ede567, 0x5182, 0x4bb6, { 0x89, 0xf6, 0x57, 0x50, 0xda, 0xf7, 0x70, 0xf4 } };

/**
* @brief 服务管道器服务提供者SCOM组件的CLSID
*  {FC02F150-270D-4b19-A4BD-BAB72F89AE69}
*/
__declspec(selectany) KSCLSID CLSID_KxEServiceMgrSP = 
{ 0xfc02f150, 0x270d, 0x4b19, { 0xa4, 0xbd, 0xba, 0xb7, 0x2f, 0x89, 0xae, 0x69 } };

/**
 * @}
 */

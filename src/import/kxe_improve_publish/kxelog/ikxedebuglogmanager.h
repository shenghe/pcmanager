#ifndef _IKXEDEBUGMANAGER_H_
#define _IKXEDEBUGMANAGER_H_
#include <Unknwn.h>
#include "IKxEDebugLog.h"

MIDL_INTERFACE("3D95F16F-E754-49ab-8E70-549D686354A4")
IKxEDebugLogManager : public IUnknown
{
public:
	/**
	* @brief 初始化日志，分配一些所需资源
	* @remark
	* @return 零(成功) or 非零(失败)
	*/
	virtual int __stdcall Initialize() = 0;

	/**
	* @brief 释放分配到的资源
	* @remark
	*/
	virtual int __stdcall Uninitialize() = 0;

	/**
	* @brief 创建一个名为pszLogName的IKxEDebugLog对象ppLog
	* @remark
	* @param[in ] pszLogName 类型名
	* @param[out] ppLog 动态创建IKxEDebugLog对象类指针
	* @return 零(成功) or 非零(失败)
	*/
	virtual int __stdcall CreateDebugLog(
		const char* pszLogName, 
		IKxEDebugLog** ppLog
		) = 0;
};

#endif
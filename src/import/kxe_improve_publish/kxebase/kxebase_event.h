//////////////////////////////////////////////////////////////////////
///		@file		kxebase_event.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		事件管理相关函数的定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../kxeevent/ikxeeventnotify.h"
#include "../kxeevent/kxeeventid_def.h"
#include "../DOMUtil/JSONUtil/KJSONDataAccess.h"
#include "../loki/static_check.h"
#include "../loki/TypeManip.h"
#include <string>

/**
 * @defgroup kxebase_event_group KXEngine Architecture Base SDK Event Interface
 * @{
 */


/**
* @brief 向事件管理系统发布一个事件
* @param[in] subSystemId 为发布事件的子系统id,参看KXE_SUB_SYSTEM_ID
* @param[in] sEventId 为各个子系统定义的事件id
* @param[in] pszBuffer 为各个子系统发布的事件内容,其结构为Json数据
* @param[in] nBufferSize 为pszBuffer的大小
* @remark 对于同一个子系统所发送的事件,将保存其顺序,不同的子系统不予保证
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_post_event(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId,
	const char* pszBuffer, 
	int nBufferSize
	);

/**
* @brief 向事件管理系统注册需要关注的事件
* @param[in] subSystemId 为发布事件的子系统id,参看KXE_SUB_SYSTEM_ID
* @param[in] sEventId 为各个子系统定义的事件id
* @param[in] pNotify 为接受事件响应的接口
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_register_event_notify(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId, 
	IKxEEventNotify* pNotify
	);

/**
* @brief 向事件管理系统取消需要关注的事件
* @param[in] subSystemId 为发布事件的子系统id,参看KXE_SUB_SYSTEM_ID
* @param[in] sEventId 为各个子系统定义的事件id
* @param[in] pNotify 为接受事件响应的接口
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_unregister_event_notify(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId,
	IKxEEventNotify* pNotify
	);

/**
* @brief 向事件管理系统发布事件的函数实现
* @param[in] subSystemId 为发布事件的子系统id,参看KXE_SUB_SYSTEM_ID
* @param[in] sEventId 为各个子系统定义的事件id
* @param[in] type 为需要发布的事件数据结构
* @remark 此函数将会把type数据结构序列化为Json数据
* @return 0 成功, 其他值为失败的错误码，
*/
template
<
	typename EventType
>
int KxEPostEventT(KXE_SUB_SYSTEM_ID subSystemId, short sEventId, const EventType& type)
{

	KANDOM::KJSONDataAccess dataAccess;
	int nErr = dataAccess.LoadDOMName("root");
	if (nErr == 0)
	{
		nErr = dataAccess.WriteData("event", const_cast<EventType&>(type));
	}

	if (nErr == 0)
	{
		std::string strBuffer;
		nErr = dataAccess.ToCompactString(&strBuffer);
		if (nErr == 0)
		{
			nErr = kxe_base_post_event(subSystemId, sEventId, strBuffer.c_str(), (int)strBuffer.size() + 1);
		}
	}
	return nErr;
}

/**
* @brief 向事件管理系统发布事件
* @param[in] _sub_system 为发布事件的子系统id,参看KXE_SUB_SYSTEM_ID
* @param[in] _event_id 为各个子系统定义的事件id
* @param[in] _value 为需要发布的事件数据结构
* @remark 此宏将调用KxEPostEventT,将_value转换为Json的数据结构
* @return 0 成功, 其他值为失败的错误码，
*/
#if _MSC_VER > 1200
#define KXE_POST_EVENT(_sub_system, _event_id, _value) \
	STATIC_CHECK(!(Loki::IsSameType<EventIdToType<_sub_system << 16 | _event_id>::type, void>::value), NotDefinedSubSystemModule);\
	KxEPostEventT(_sub_system, _event_id, _value)
#else
#define KXE_POST_EVENT(_sub_system, _event_id, _value) \
	STATIC_CHECK(!(Loki::IsEqualType<EventIdToType<_sub_system << 16 | _event_id>::type, void>::value), NotDefinedSubSystemModule);\
	KxEPostEventT(_sub_system, _event_id, _value)
#endif

// STATIC_CHECK((Loki::IsSameType<EventIdToType<_sub_system << 16 | _event_id>::type, EventType>::value), NotMatchedSubSystemModuleEvent);

/**
 * @}
 */

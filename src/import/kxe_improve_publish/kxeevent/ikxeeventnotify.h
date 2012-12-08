//////////////////////////////////////////////////////////////////////
///		@file		ikxeeventnotify.h
///		@author		luopeng
///		@date		2008-8-27 14:07:58
///
///		@brief		jason数据与C++数据命令转换的相关工具函数及宏
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../json/json.h"
#include "../DOMUtil/JSONUtil/KJSONDataAccess.h"
#include "kxeevent.h"
#include "kxeeventid_def.h"
#include "../loki/static_check.h"
#include "../loki/TypeManip.h"

/**
 * @defgroup kxeevent_group KXEngine Architecture Event Interface
 * @{
 */

/**
 * @brief 各个需要接受事件服务的模块必须实现的接口
 */
class IKxEEventNotify
{
public:
	/**
	 * @brief 当有对应的sSubSystemId << 16 | sEventId的事件发生时,将会调用此函数
	 * @see KXE_SUB_SYSTEM_ID
	 * @param[in] subSystemId 为子系统的ID定义,参见KXE_SUB_SYSTEM_ID
	 * @param[in] sEventId 为各个子系统定义的事件ID号
	 * @param[in] pszJson 事件发生时,将事件类型转化为的Json数据
	 * @param[in] uSize pszJson的长度,包含字符串结束符
	 */
	virtual int __stdcall Notify(
		KXE_SUB_SYSTEM_ID subSystemId, 
		short sEventId, 
		const char* pszJson, 
		unsigned int uSize
		) = 0;
};

template
<
	typename Object,
	typename EventType
>
int KxECallEventProcessFunction(Object* pObject,
								int (Object::*memFunction)(int, const EventType&),
								int nId,
								const char* pszJson,
								int nSize)
{
	EventType eventValue;

	KANDOM::KJSONDataAccess dataAccess;
	int nErr = dataAccess.LoadString(pszJson);
	if (nErr == 0)
	{
		nErr = dataAccess.ReadData("event", eventValue);  // 失败?

		if (nErr == 0)
		{
			(pObject->*memFunction)(nId, eventValue);
		}
	}

	return nErr;
}

/**
 * @brief 在需要实现IKxEEventNotify的类头文件中置放该宏
 */
#define KXE_DECLARE_EVENT_NOTIFY() \
	int __stdcall Notify(KXE_SUB_SYSTEM_ID sSubSystemId, short sEventId, const char* pszJson, unsigned int nSize);

/**
 * @brief 该宏用于实现IKxEEventNotify的Notify函数,并将pszJson转换为指定的数据类型后调用
 */
#define KXE_BEGIN_EVENT_NOTIFY(_class)\
		int _class::Notify(KXE_SUB_SYSTEM_ID sSubSystemId, short sEventId, const char* pszJson, unsigned int nSize){\
				switch (sSubSystemId << 16 | sEventId) {

/**
 * @brief 将_sub_system_id << 16 | _id这个事件转化为对应的数据类型后,调用给_function
 */
#define KXE_ON_EVENT_PROCESS(_sub_system_id, _id, _function)\
						case _sub_system_id << 16 | _id: \
						{\
							STATIC_CHECK(!(Loki::IsSameType<EventIdToType<_sub_system_id << 16 | _id>::type, void>::value), NotDefinedSubSystemModule);\
							EventIdToType<_sub_system_id << 16 | _id>::type eventValue;\
							KANDOM::KJSONDataAccess dataAccess;\
							int nErr = dataAccess.LoadString(pszJson);\
							if (nErr == 0)\
							{\
								nErr = dataAccess.ReadData("event", eventValue);  \
								if (nErr == 0)\
								{\
									return _function(_sub_system_id, _id, eventValue, 0);\
								}\
							}\
							if (nErr != 0)\
							{\
								_function(_sub_system_id, _id, eventValue, nErr);\
							}\
							return nErr;\
						}\
						break;

/**
 * @brief 该宏用于标志实现IKxEEventNotify的Notify函数结束
 */
#define KXE_END_EVENT_NOTIFY() \
						default:\
								return 0;\
				}\
				return 0;\
		}

/**
 * @}
 */

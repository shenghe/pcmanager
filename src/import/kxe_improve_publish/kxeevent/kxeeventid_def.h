//////////////////////////////////////////////////////////////////////
///		@file		kxeeventid_def.h
///		@author		luopeng
///		@date		2008-9-10 09:07:58
///
///		@brief		事件服务id与类型对应的头文件定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include "../kxebase/kxesubsystemid_def.h"

/**
 * @addtogroup kxeevent_group
 * @{
 */

template
<
	int nEventId
>
struct EventIdToType
{
	typedef void type;
};

/**
 * @brief 用于将指定的数据结构于事件id(_sub_system << 16 | _id)绑定
 * @see KXE_SUB_SYSTEM_ID
 * @param[in] _sub_system 为KXE_SUB_SYSTEM_ID中的枚举量
 * @param[in] _id 为各个子系统自已定义的事件id
 * @param[in] _type 为需要绑定的事件类型
 */
#define KXE_DECLARE_EVENT_ID_TYPE(_sub_system, _id, _type) \
	template\
	<>\
	struct EventIdToType<_sub_system << 16 | _id>\
	{\
		typedef _type type;\
	};

/**
 * @}
 */

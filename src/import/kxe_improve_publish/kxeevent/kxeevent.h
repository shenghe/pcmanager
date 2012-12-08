//////////////////////////////////////////////////////////////////////
///		@file		kxeevent.h
///		@author		luopeng
///		@date		2008-9-10 09:07:58
///
///		@brief		事件服务的头文件定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include "ikxeeventnotify.h"
#include "ikxeeventmgr.h"

/**
	@page kxeevent_example KXEngine EventSystem Example

	- @ref kxeevent_example_struct
	- @ref kxeevent_example_post
	- @ref kxeevent_example_notify

	@section kxeevent_example_struct 定义数据结构

	@code
	#pragma once
	#include <string>
	#include "include/kxe_serialize.h"

	typedef struct _KXE_FILE_MON_STATUS
	{
		BOOL bIsEnable;
	} KXE_FILE_MON_STATUS;

	KXE_JSON_DDX_BEGIN(KXE_FILE_MON_STATUS)
		KXE_JSON_DDX_MEMBER("Enable", bIsEnable);
	KXE_JSON_DDX_END()

	#define KXE_FILE_MON_STATUS_EVENT 1

	KXE_DECLARE_EVENT_ID_TYPE(enum_SubSystem_FileMon, KXE_FILE_MON_STATUS_EVENT, KXE_FILE_MON_STATUS);

	@endcode

	@section kxeevent_example_post 发布事件
	@code
	KXE_FILE_MON_STATUS status;
	status.bIsEnable = m_bIsEnable;
	KXE_POST_EVENT(enum_SubSystem_FileMon, KXE_FILE_MON_STATUS_EVENT, status);		
	@endcode

	@section kxeevent_example_notify 响应事件
	@code
	// 头文件中的定义
	class KxEScanServiceProvider : public IKxEEventNotify
	{
	public:
		KXE_DECLARE_EVENT_NOTIFY();

	int OnFileMonStatusEvent(
		short sSubSystemId, 
		short sEventId,
		const KXE_FILE_MON_STATUS& status, 
		int nErr
		);
	};

	// 实现文件中的定义
	KXE_BEGIN_EVENT_NOTIFY(KxEScanServiceProvider)
		KXE_ON_EVENT_PROCESS(enum_SubSystem_FileMon, KXE_FILE_MON_STATUS_EVENT, OnFileMonStatusEvent)
	KXE_END_EVENT_NOTIFY()

	int KxEScanServiceProvider::OnFileMonStatusEvent(short sSubSystemId,
													 short sEventId, 
													 const KXE_FILE_MON_STATUS& status,
													 int nErr)
	{
		std::cout << "The filemon status is " << status.bIsEnable << std::endl;
		return 0;
	}
	int KxEScanServiceProvider::StartService()
	{
		kxe_base_register_event_notify(
			enum_SubSystem_FileMon,
			KXE_FILE_MON_STATUS_EVENT,
			this
			);
		return 0;
	}
	
	@endcode
 */

/**
 @page kxeevent_json_data_struct evnet_json_data_struct

 @section kxeevent_json 事件服务的Json数据结构
 @code
 root :
 {
	event:
	{
		// any json data
	}
 }

@endcode
*/

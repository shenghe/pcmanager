//////////////////////////////////////////////////////////////////////
///		@file		ikxetask.h
///		@author		luopeng
///		@date		2008-9-22 14:07:58
///
///		@brief		Task接口及JobControl的定义
//////////////////////////////////////////////////////////////////////

#pragma once

/**
* @addtogroup kxebase_thread_group
* @ref kxebase_thread_example
* @{
*/

/**
* @brief IKxETaskControl控制接口,当Job的类型为enum_kjp_away_keyboard_long_job和enum_kjp_control_cpu_long_job需要调用此接口
*/
class IKxETaskControl
{
public:
	/**
	 * @brief 由任务的实现者调用此接口以实现任务的中断,当到达指定条件时,再返回给调用者
	 * @return E_KXETHREAD_JOB_IN_STOP_CMD 代表外部调用了kxe_base_stop_cmd,需要停止任务了
	 */
	virtual int __stdcall Interruppt() = 0;

	/**
	 * @brief 由任务的实现者调用此接口以实现判断该任务是否需要停止,代表外部调用了kxe_base_stop_cmd
	 */
	virtual bool __stdcall IsStop() = 0;
};

/**
* @brief 指定的任务运行接口
*/
class IKxETask
{
public:
	/**
	* @brief 该任务的实际运行函数
	* @see kxe_base_get_job_exit_code
	* @see kxe_job_type
	* @param[in] pParam 为Job的定义者规定内容,由提交任务的提供该内容
	* @param[in] pControl 当pControl有效时,需要在循环中调用其子函数Interuppt及IsStop判断退出,并且仅能在Run函数中调用.
	* @remark 当IKxETaskControl指针有效时,Task实现者需要定时调用此接口的Interruppt,以实现CPU,用户离开及暂停的处理
	* @return 返回的值通过kxe_base_get_job_exit_code能够得到
	*/
	virtual int __stdcall RunTask(
		void* pParam, 
		IKxETaskControl* pControl
		) = 0;
};

/**
* @}
*/

/**
 * @page kxebase_thread_example KXEngine Thread Example
 * @section kxebase_thread_example_define_task 定义任务的实现
 * @code
class KxETask : public IKxETask
{
public:
	virtual int __stdcall StopTask()
	{
		m_bStop = true;
		return 0;
	}

	virtual int __stdcall RunTask(
		void* pParam, 
		IKxETaskControl* pControl
		)
	{
		while (!pControl->IsStop())
		{
			Sleep(100);
		
			// do something

			pControl->Interruppt();
		}
		return -1;
	}
};
 * @endcode
 @section kxebase_thread_example_startup_job 将任务创建为Job并进行操作
 @code
	kxe_job_t job;

	KxETask* pTask = new KxETask;
	kxe_base_create_job(
		enum_kjp_low_influence_long_job,
		pTask,
		NULL,
		&job
		);

	kxe_base_run_job(job);

	kxe_base_wait_job(job, 20000);

	kxe_base_stop_job(job);

	kxe_base_wait_job(job, INFINITE);

	int nExitCode = 0;
	kxe_base_get_job_exit_code(job, &nExitCode);

	kxe_base_close_job(job);

	delete pTask;

 @endcode
 */
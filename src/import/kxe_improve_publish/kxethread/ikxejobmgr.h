//////////////////////////////////////////////////////////////////////
///		@file		ikxejobmgr.h
///		@author		luopeng
///		@date		2008-8-22 10:38:34
///
///		@brief		线程管理的定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Unknwn.h>
#include "../kxebase/kxebase_job.h"

/**
 * @defgroup kxethread_component_interface KXEngine Architecture Thread Component Interface
 * @{
 */

/**
 * @brief Job管理器
 */
MIDL_INTERFACE("5D931AD3-14F6-4ec4-A238-E379EFB16EC3")
IKxEJobMgr : public IUnknown
{
	/**
	 * @brief 初始化Job管理器
	 * @param uInitializeThreadCount 仅代表初始化的线程数量,线程的实际数量会随着使用变化,最大值不能超过512
	 */
	virtual int __stdcall Initialize(
		unsigned int uInitializeThreadCount = 10
		) = 0;

	/**
	 * @brief 反初始化Job管理器
	 */
	virtual int __stdcall Uninitailze() = 0;

	/**
	 * @brief 启动Job管理器,将根据初始化的线程数量,将线程创建起来
	 */
	virtual int __stdcall Start() = 0;

	/**
	 * @brief 停止Job管理器
	 */
	virtual int __stdcall Stop() = 0;

	/**
	 * @brief 创建出新的Job
	 * @see kxe_base_create_job
	 */
	virtual int __stdcall CreateJob(
		kxe_job_type type, 
		IKxETask* pTask,
		void* param, 
		kxe_job_t* pJob
		) = 0;

	/**
	 * @brief 创建出新的Job，它将模拟当前线程的Token
	 * @see kxe_base_create_job
	 */
	virtual int __stdcall CreateJobImpersonate(
		kxe_job_type type, 
		IKxETask* pTask,
		void* param, 
		kxe_job_t* pJob
		) = 0;

	/**
	 * @brief 运行指定的job
	 * @see kxe_base_run_job
	 */
	virtual int __stdcall RunJob(
		kxe_job_t job
		) = 0;

	/**
	 * @brief 暂停指定的job
	 * @see kxe_base_pause_job
	 */
	virtual int __stdcall PauseJob(
		kxe_job_t job
		) = 0;

	/**
	 * @brief 恢复指定的job
	 * @see kxe_base_resume_job
	 */
	virtual int __stdcall ResumeJob(
		kxe_job_t job
		) = 0;

	/**
	 * @brief 运行指定的job
	 * @see kxe_base_set_and_run_control_cpu_job
	 */
	virtual int __stdcall RunJobOnControlCPU(
		kxe_job_t job,
		float fCpuPercent
		) = 0;

	/**
	 * @brief 运行指定的job
	 * @see kxe_base_set_and_run_user_away_job
	 */
	virtual int __stdcall RunJobOnUserAway(
		kxe_job_t job,
		unsigned int uSeconds
		) = 0;

	/**
	 * @brief 停止指定的job
	 * @see kxe_base_stop_job
	 */
	virtual int __stdcall StopJob(
		kxe_job_t job
		) = 0;

	/**
	 * @brief 等待指定的job完成
	 * @see kxe_base_wait_job
	 */
	virtual int __stdcall WaitJob(
		kxe_job_t* pJobArray,
		unsigned int uCount,
		bool bWaitAll,
		unsigned int uMilliseconds
		) = 0;

	/**
	 * @brief 关闭指定的job句柄
	 * @see kxe_base_close_job
	 */
	virtual int __stdcall CloseJob(
		kxe_job_t job
		) = 0;

	/**
	 * @brief 获取job的退出码
	 * @see kxe_base_get_job_exit_code
	 */
	virtual int __stdcall GetJobExitCode(
		kxe_job_t job,
		int* pnExitCode
		) = 0;

	/**
	 * @brief 获取job的状态
	 * @see kxe_base_get_job_status
	 */
	virtual int __stdcall GetJobStatus(
		kxe_job_t job,
		kxe_job_status* pStatus
		) = 0;
};

/**
 * @}
 */
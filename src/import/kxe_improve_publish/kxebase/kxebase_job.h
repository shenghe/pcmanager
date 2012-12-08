//////////////////////////////////////////////////////////////////////
///		@file		kxebase_job.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		job函数相关的定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../kxethread/ikxetask.h"

/**
 * @defgroup kxebase_thread_group KXEngine Architecture Base SDK Thread Interface
 * @{
 */

/**
 * @brief 提交的job的类型
 */
typedef enum _kxe_job_type
{
	enum_kjp_short_job,                   ///< 短时间就可运行完成的Job,使用线程池中的线程完成
	enum_kjp_normal_long_job,             ///< 长时间才能运行完成的Job,创建新的线程完成
	enum_kjp_low_influence_long_job,      ///< 长时间运行,并要求低影响用户的Job
	enum_kjp_away_keyboard_long_job,      ///< 长时间运行,并且仅在用户空闲时运行,需要调用IKxEJobControl.Interuppt完成
	enum_kjp_control_cpu_long_job         ///< 控制该线程CPU使用的Job,需要调用kxe_base_set_and_run_control_cpu_job设置CPU用量才能开始运行
} kxe_job_type;

inline bool KxEIsLongJobType(kxe_job_type type)
{
	if (type != enum_kjp_short_job)
	{
		return true;
	}

	return false;
}

/**
 * @brief 定义Job当前的状态
 */
typedef enum _kxe_job_status
{
	enum_kjs_ready,      ///< job已经准备好,即将运行
	enum_kjs_running,    ///< job处于运行状态
	enum_kjs_pause_cmd,  ///< 向job发出了pause命令,但job还未暂停
	enum_kjs_paused,     ///< job已经处于暂停状态
	enum_kjs_stop_cmd,   ///< 向job发出了stop命令,但job还未停止
	enum_kjs_finished,   ///< job已经完成
	enum_kjs_error       ///< job出现错误
} kxe_job_status;


/**
 * @brief 定义Job句柄类型
 */
typedef void* kxe_job_t;

/**
 * @brief 创建Job用于运行指定的任务
 * @see kxe_base_close_multi_job, kxe_base_close_job
 * @param[in] type 该任务需要运行的类型
 * @param[in] pTask 该任务的运行体
 * @param[in] pParam 该任务的参数
 * @param[out] pJobHandle 创建该任务获得的JobHandle, 使用kxe_base_close_job,
                          kxe_base_close_multi_job关闭
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_create_job(
	kxe_job_type type, 
	IKxETask* pTask, 
	void* pParam, 
	kxe_job_t* pJobHandle
	);

/**
 * @brief 创建Job用于运行指定的任务，此任务将模拟当前线程的Token
 * @see kxe_base_close_multi_job, kxe_base_close_job
 * @param[in] type 该任务需要运行的类型
 * @param[in] pTask 该任务的运行体
 * @param[in] pParam 该任务的参数
 * @param[out] pJobHandle 创建该任务获得的JobHandle, 使用kxe_base_close_job,
                          kxe_base_close_multi_job关闭
 * @remark 如果获取Token失败，将返回错误码，而不会继续执行
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_create_job_as_thread_token(
	kxe_job_type type, 
	IKxETask* pTask, 
	void* pParam, 
	kxe_job_t* pJobHandle
	);

/**
 * @brief 设定enum_kjp_control_cpu_long_job的job的CPU占用量,并开始运行job.
 *        仅类型为enum_kjp_control_cpu_long_job才能调用
 * @param[in] job 需要控制CPU占用量的job
 * @param[in] fCpuPercent 线程总共可以使用的CPU百分比
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_set_and_run_control_cpu_job(
	kxe_job_t job,
	float fCpuPercent
	);

/**
 * @brief 设定enum_kjp_away_keyboard_long_job的job在用户离开指定时间后运行
 * @param[in] job 需要控制的job
 * @param[in] uSeconds 用户离开的时间,单位为秒
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_set_and_run_user_away_job(
	kxe_job_t job,
	unsigned int uSeconds
	);

/**
 * @brief 启动job的运行
 * @see kxe_base_set_and_run_control_cpu_job
 * @see kxe_base_set_and_run_user_away_job
 * @param[in] job 需要运行的job
 * @remark 当job类型为enum_kjp_control_cpu_long_job时,需要使用kxe_base_set_and_run_control_cpu_job
          当job类型为enum_kjp_away_keyboard_long_job时,需要使用kxe_base_set_and_run_user_away_job
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_run_job(
	kxe_job_t job
	);

/**
 * @brief 停止指定的Job
 * @param[in] job 需要停止的Job, 指定的Job需要使用IKxETaskControl::IsStop判断
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_stop_job(
	kxe_job_t job
	);

/**
 * @brief 暂停指定的Job
 * @param[in] job 需要暂停的Job
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_pause_job(
	kxe_job_t job
	);

/**
 * @brief 恢复指定的Job
 * @param[in] job 需要恢复的Job
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_resume_job(
	kxe_job_t job
	);


/**
 * @brief 等待指定的Job退出
 * @param[in] job 需要等待的Job
 * @param[in] uMilliseconds 需要等待的时间
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_wait_job(
	kxe_job_t job,
	unsigned int uMilliseconds
	);

/**
* @brief 等待指定数量的Job退出
* @param[in] arrayJob 需要停止的Job数组
* @param[in] uJobCount 需要停止的job的数量
* @param[in] bWaitAll 是否等待所有的job退出后才返回
* @param[in] uMilliseconds 等待的时间
* @return 0 成功, 其他为失败错误码
*/
int __stdcall kxe_base_wait_multi_job(
	kxe_job_t* arrayJob,
	unsigned int uJobCount,
	bool bWaitAll,
	unsigned int uMilliseconds
	);

/**
 * @brief 获取该任务退出时的,返回的code
 * @param[in] job 需要获取code的job
 * @param[out] pnExitCode 返回job退出code
 */
int __stdcall kxe_base_get_job_exit_code(
	kxe_job_t job,
	int* pnExitCode
	);

/**
 * @brief 获取该任务当前的状态
 * @param[in] job 需要获取的job
 * @param[out] pStatus 存储当前任务状态
 */
int __stdcall kxe_base_get_job_status(
	kxe_job_t job,
	kxe_job_status* pStatus
	);

/**
 * @brief 关闭指定的Job
 * @param[in] job 需要关闭的Job
 * @remark 如果在job还在运行的时候不允许进行关闭
 * @return 0 成功, 其他为失败错误码
 */
int __stdcall kxe_base_close_job(
	kxe_job_t job
	);

/**
 * @}
 */
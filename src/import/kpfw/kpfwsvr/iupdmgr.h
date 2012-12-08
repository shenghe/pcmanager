#pragma once

#include "scom/scom/scombase.h"

//内部接口

struct IUpdMgr
{
	//LockCreate通知服务锁定组件的创建操作，被锁定时，除了UpdCtrl组件都不能被创建
	//Return:
	//	S_OK 成功
	virtual HRESULT STDMETHODCALLTYPE LockCreate(
		) = 0;

	//UnlockCreate通知服务解除创建锁定，再调用每个组件的CompleteUpdate通知各组件更新已经完成
	//Return:
	//	S_OK 成功
	virtual HRESULT STDMETHODCALLTYPE UnlockCreate(
		) = 0;


	//GetRelatedProcess根据服务内保存的调用关系返回正在引用clsids的界面进程的列表，用于提示用户或做记录
	//Return:
	//	S_OK 成功取到进程列表
	//	E_OUTOFMEMORY 分配的返回ID的内存不够大
	//  E_FAIL 失败
	virtual HRESULT STDMETHODCALLTYPE GetRelatedProcess(IN KSCLSID* pCLSIDs, IN int nCLSIDNum, IN OUT DWORD* pdwPIDs, IN OUT int* pnPIDNum
		) = 0;

	//NotifyComponentUpdate通知指定的组件退出
	//Return:
	//	S_OK 指定组件已退出所占用的文件
	//  S_FALSE 需要退出指定组件才能退出所占用的文件
	//	E_PENDING 该组件仍被未知模块所引用，该组件目前无法卸载
	//	E_FAIL 失败
	//Param:
	//	clsid是被更新文件的宿主(或管理者)
	//	pwsFileNameList是被更新文件的列表，多个文件件名以'|'分隔
	virtual HRESULT STDMETHODCALLTYPE NotifyComponentUpdate(IN KSCLSID clsid, IN DWORD dwDataType, IN const wchar_t* pwsFileNameList
		) = 0;

	//NotifyProcessQuit通知指定PID的进程退出
	//Return:
	//	S_OK 已通知指定进程ID的进程退出（通过广播方式）
	//	E_PENDING 如果通知的是服务本身退出，该操作不会执行，需要调用者用NotifyServiceQuit函数才能通知服务退出
	//	E_FAIL 失败
	//Param:
	//	pdwPIDs是被通知退出的进程ID列表，接到通知的进程可以不再询问用户而立即退出
	//	nPIDNum是列表的数量
	virtual HRESULT STDMETHODCALLTYPE NotifyProcessQuit(IN DWORD* pdwPIDs, IN int nPIDNum
		) = 0;
};
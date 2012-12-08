#pragma once

#include "SCOM/SCOM/SCOMBase.h"
#include "KisRpc/Rfcdef.h"
#include "wtypes.h"
#include <vector>
using namespace std;

//IRemote开头的接口表示是RPC接口
interface IRemoteUpdMgr
{
	//StartUpdCtrl通知升级组件重启
	//Return:
	//	S_OK 
	//	E_FAIL 
	//Param:
	//	clsid是升级组件的ID
	virtual HRESULT STDMETHODCALLTYPE StartUpdProcess(DWORD dwType
		) = 0;

	//QuitServiceProcess通知服务退出
	virtual void STDMETHODCALLTYPE QuitServiceProcess(
		) = 0;

	//GetServiceProcessId得到服务进程ID，升级程序Wait它以确认服务是否已经退出
	//Return:
	//	S_OK 成功取得服务进程的PID
	virtual HRESULT STDMETHODCALLTYPE GetServiceProcessId(IN OUT DWORD* pdwPID
		) = 0;

	//LockCreate通知服务锁定组件的创建操作，被锁定时，服务提供者的组件不能被创建（进程内的ＳＣＯＭ可以）
	//Return:
	//	S_OK 成功
	virtual HRESULT STDMETHODCALLTYPE LockCreate(
		) = 0;

	//UnlockCreate通知服务解除创建锁定，UnlockCreate会自动再调用每个组件的CompleteUpdate通知各组件更新已经完成
	//Return:
	//	S_OK 成功
	virtual HRESULT STDMETHODCALLTYPE UnlockCreate(
		) = 0;


	//GetRelatedProcess根据服务内保存的调用关系返回正在引用clsids的界面进程的列表，用于提示用户或做记录
	//Return:
	//	S_OK 成功取到进程列表
	//  E_FAIL 失败
	virtual HRESULT STDMETHODCALLTYPE GetRelatedProcess(IN vector<KSCLSID> clsids, IN OUT vector<DWORD>* pdwPIDs
		) = 0;

	//NotifyComponentUpdate通知指定的组件退出
	//Return:
	//	S_OK 指定组件已退出所占用的文件
	//  S_FALSE 需要退出指定组件才能退出所占用的文件
	//	E_PENDING 该组件仍被未知模块所引用，该组件目前无法卸载
	//	E_FAIL 失败
	//Param:
	//	clsid是被更新文件的宿主(或管理者)
	//	fileNameList是被更新文件的列表
	virtual HRESULT STDMETHODCALLTYPE NotifyComponentUpdate(IN KSCLSID clsid, IN DWORD dwDataType, IN vector<wstring> fileNameList
		) = 0;

	//NotifyProcessQuit通知指定PID的进程退出
	//Return:
	//	S_OK 已通知指定进程ID的进程退出（通过广播方式）
	//	E_PENDING 如果通知的是服务本身退出，该操作不会执行，需要调用者用NotifyServiceQuit函数才能通知服务退出
	//	E_FAIL 失败
	//Param:
	//	pdwPIDs是被通知退出的进程ID列表，接到通知的进程可以不再询问用户而立即退出
	//	nPIDNum是列表的数量
	virtual HRESULT STDMETHODCALLTYPE NotifyProcessQuit(IN vector<DWORD> pids
		) = 0;

};

//RPC接口定义
RCF_CLASS_BEGIN_WITH_COM(IRemoteUpdMgr)
RCF_METHOD_R_1(HRESULT, StartUpdProcess, DWORD)
RCF_METHOD_V_0(QuitServiceProcess)
RCF_METHOD_R_1(HRESULT, GetServiceProcessId, DWORD*)
RCF_METHOD_R_0(HRESULT, LockCreate)
RCF_METHOD_R_0(HRESULT, UnlockCreate)
RCF_METHOD_R_2(HRESULT, GetRelatedProcess, vector<KSCLSID>, vector<DWORD>*)
RCF_METHOD_R_3(HRESULT, NotifyComponentUpdate, KSCLSID, DWORD, vector<wstring>)
RCF_METHOD_R_1(HRESULT, NotifyProcessQuit, vector<DWORD>)
RCF_CLASS_END

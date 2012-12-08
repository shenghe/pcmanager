// ikislive 接口。

#pragma once
#ifndef __UPLIVE_COM_IKISLIVE_H__
#define __UPLIVE_COM_IKISLIVE_H__
#include "uplive_common.h"

#include "kisco.h"
#include "ikuplivecallback.h"

//{A2B79F67-A177-458f-9015-C2B8C0E654BC}
#if _MSC_VER <= 1200 // vc6.0
interface __declspec(uuid("A2B79F67-A177-458f-9015-C2B8C0E654BC"))
#else
[
    uuid("A2B79F67-A177-458f-9015-C2B8C0E654BC")
]
interface
#endif
IKislive 
	: public KISUP::Kis::IKisCoObjectT<IKislive>
{
	/*
	@ FuncName	: CheckKislive
	@ Param		: lpszCheck 检测数据块。
	@ Param		: pExVerInfo 返回kislivx.dll版本扩展信息。
	@ Brief		: 检测Kislive是否可用。
	*/
	virtual STDMETHODIMP CheckKislive(IN LPCWSTR lpszCheck, OUT KExVerInfo* pExVerInfo, OUT KExReserve* pReserve = NULL) = 0;
	/*
	@ FuncName	: InitKislive
	@ Param		: lpStrAppCmdlive 程序命令行参数。
	@ Brief		: 初始化Kislive。
	*/
	virtual STDMETHODIMP InitKislive(IN LPCWSTR lpStrAppCmdlive) = 0;
	/*
	@ FuncName	: UnInitKislive
	@ Brief		: 反初始化Kislive。
	*/
	virtual STDMETHODIMP UnInitKislive(void) = 0;
	/*
	@ FuncName	: CfgCallBack
	@ Param		: pIKUpliveCallBack 回调接口。
	@ Brief		: 配置IKUpliveCallBack回调。
	*/
	virtual STDMETHODIMP CfgCallBack(IN IKUpliveCallBack * pIKUpliveCallBack) = 0;
	/*
	@ FuncName	: CfgKisliveParam
	@ Param		: lpStrDataParam 数据参数。
	@ Brief		: 配置Kislive内部参数变量，Exe要改变kislive.dll里变量接口。
	*/
	virtual STDMETHODIMP CfgKisliveParam(IN LPCWSTR lpStrDataParam) = 0;
	/*
	@ FuncName	: LoadKisliveDataToUI
	@ Brief		: 从Kislive中加载数据到UI界面，数据通过数据回调接口OnKisDataEvent返回。
	*/
	virtual STDMETHODIMP LoadKisliveDataToUI(void) = 0;
	/*
	@ FuncName	: AnalyzeIndex
	@ Brief		: 分析Index文件（在线升级、局域网升级、离线升级）。
	*/
	virtual STDMETHODIMP AnalyzeIndex(void) = 0;
	/*
	@ FuncName	: DownloadFiles
	@ Brief		: 下载文件（若是在线升级，就下载文件，若是局域网升级就是同步病毒库等）。
	*/
	virtual STDMETHODIMP DownloadFiles(void) = 0;
	/*
	@ FuncName	: UpdateFiles
	@ Brief		: 更新文件（XXX）。
	*/
	virtual STDMETHODIMP UpdateFiles(void) = 0;
	/*
	@ FuncName	: ReUpdateFiles
	@ Param		: bShowWnd 表示UI是否已显示窗口
	@ Brief		: 重新更新文件。
	*/
	virtual STDMETHODIMP ReUpdateFiles(IN BOOL bShowWnd) = 0;
	/*
	@ FuncName	: ReUpdateActiveUIWnds
	@ Brief		: 重新更新时，激活占用文件的UI进程。
	*/
	virtual STDMETHODIMP ReUpdateActiveUIWnds(void) = 0;
	/*
	@ FuncName	: CheckLanUpdateDir
	@ Param		: lpStrLanUpdateDir 局域网升级数据所在的目录。
	@ Brief		: 检测局域网升级目录，可以升级返回S_OK。
	*/
	virtual STDMETHODIMP CheckLanUpdateDir(IN LPCWSTR lpStrLanUpdateDir) = 0;
	/*
	@ FuncName	: MakeOfflinePackage
	@ Param		: lpStrPackageDir 所要制作的离线升级包存放目录。
	@ Brief		: 制作离线升级包，成功返回S_OK。
	*/
	virtual STDMETHODIMP MakeOfflinePackage(IN LPCWSTR lpStrPackageDir) = 0;
	/*
	@ FuncName	: CfgOemInfo
	@ Param		: uOemInfoId Oem信息id号。
	@ Brief		: 配置oem信息。
	*/
	virtual STDMETHODIMP CfgOemInfo(IN u_int uOemInfoId, IN int nOemInfo) = 0;
	/*
	@ FuncName	: CfgOemInfo
	@ Param		: uOemInfoId Oem信息id号。
	@ Brief		: 配置oem信息。
	*/
	virtual STDMETHODIMP CfgOemInfo(IN u_int uOemInfoId, IN LPCWSTR lpszOemInfo) = 0;
};

#endif //__UPLIVE_COM_IKISLIVE_H__


//=================================================================
/**
* @file iksautoinstclient.h 
* @brief
* @author zhengyousheng <zhengyousheng@kingsoft.com>
* @data 2010-11-06 11:22
*/
//=================================================================

#pragma once
#include <Unknwn.h>
#include <softmgr/iksautoinstcallback.h>

// {F908EA8A-745E-4322-9E1F-088381A1D3C0}
// IMPLEMENT_OLECREATE(<<class>>, <<external_name>>, 
// 					0xf908ea8a, 0x745e, 0x4322, 0x9e, 0x1f, 0x8, 0x83, 0x81, 0xa1, 0xd3, 0xc0);
[
	uuid("F908EA8A-745E-4322-9E1F-088381A1D3C0")
]
interface IKSAutoInstClient
{
	/**
	* @brief 初始化
	* @param[in] lpwszXmlCfg 软件配置文件路径
	* @param[in] lpReserved 保留参数, 目前暂时未用,直接传NULL
	* @return 0 成功，其他为失败错误码
	*/
	virtual HRESULT STDMETHODCALLTYPE Init(LPCWSTR lpwszXmlCfg, LPVOID lpReserved) = 0;

	/**
	* @brief 设置回调
	* @param[in] piInstallCallBack 回调接口
	* @return 0 成功，其他为失败错误码
	*/
	virtual HRESULT STDMETHODCALLTYPE SetCallBack(IKSAutoInstCallBack* piInstallCallBack) = 0;

	/**
	* @brief 安装配置中的软件
	* @param[in] lpszGBKXmlBuffer 软件的xml配置，此buffer为gbk编码，其中包括软件ID、安装包路径、用户自定义信息
	* @parma[in] nBufferSize buffer大小,以byte为单位
	* @return 0 成功，其他为失败错误码
	*/
	/*
	lpwszXmlConfig格式：
	<soft ver="1.0">
	   <softinfo softid="1" inst_pack="d:\qq.exe" >
//	   <download url="" md5="" save_name="" file_size="" inst_size="" />
		 <inst_dir>C:\xxxxx</inst_dir>
	     <user_define count="3" >
			 <item name="$ctrl_name_1" value="0" info="运行QQ旋风2.9" />
			 <item name="$ctrl_name_2" value="1" info="显示新特性" />
	     </user_define>
	   </softinfo>
	</soft>
	*/
	virtual HRESULT STDMETHODCALLTYPE StartInstSoft(LPCSTR lpszGBKXmlBuffer, int nBufferSize) = 0;

	/**
	* @brief 通知停止
	* @return 0 成功，其他为失败错误码
	*/
	virtual HRESULT STDMETHODCALLTYPE NotifyStop() = 0;

	/**
	* @brief 反初始
	* @return 0 成功，其他为失败错误码
	*/
	virtual HRESULT STDMETHODCALLTYPE UnInit() = 0;

	/**
	* @brief 引用计数控制
	*/
	virtual HRESULT STDMETHODCALLTYPE AddRef() = 0;
	virtual HRESULT STDMETHODCALLTYPE Release() = 0;
};
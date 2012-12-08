/********************************************************************
	created:	2010/05/26
	created:	26:5:2010   10:14
	filename: 	libkschelper.h
	author:		Jiang Fengbing
	
	purpose:	清理专家驱动控制接口
*********************************************************************/

#ifndef LIBKSCHELPER_INC_
#define LIBKSCHELPER_INC_

//////////////////////////////////////////////////////////////////////////

/**
 * 安装/卸载驱动
 */
bool __stdcall InstallKscHelper(HINSTANCE hInstance);
bool __stdcall UnInstallKscHelper();

/**
 * 启动/停止驱动
 */
bool __stdcall StartKscHelper();
bool __stdcall StopKscHelper();

/**
 * 准备驱动
 * 在KscOpenVolume前调用，如果驱动没有安装或者启动，自动安装和启动
 */
bool __stdcall PrepareKscHelper(HINSTANCE hInstance);


/**
 * 打开/关闭卷设备
 * 可以使用ReadFile或者WriteFile进行读或者写
 */
HANDLE __stdcall KscOpenVolume(const wchar_t szVolume[]);
BOOL __stdcall KscCloseVolume(HANDLE hVolume);

//////////////////////////////////////////////////////////////////////////

#endif	// LIBKSCHELPER_INC_

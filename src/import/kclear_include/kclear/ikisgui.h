#pragma once

interface IMainWndNotify;
interface ISettingPage;
interface IModule;

[ uuid("124799F9-74B1-431a-B203-FB0BFC81695B") ]
interface IModuleMgr : IUnknown
{
	virtual HRESULT __stdcall GetModuleCnt(int* pnModuleCnt) = 0;
	virtual HRESULT __stdcall CreateModule(int nIndex, IModule** ppIModule) = 0;
};

[ uuid("37A8CFDE-3A7E-4183-9E25-836412E2C374") ]
interface IModule : IUnknown
{
	virtual HRESULT __stdcall Init() = 0;
	virtual HRESULT __stdcall Uninit() = 0;
	virtual HRESULT __stdcall Create(HWND hwndParent, RECT rect, HWND* phwndCreated) = 0;
	virtual HRESULT __stdcall Show(BOOL bShow) = 0;
	virtual HRESULT __stdcall Move(RECT rect) = 0;


	//应用程序退出前，询问各个模块是否有未保存的数据和未完成操作，如果用户选择取消，程序不退出。
	//pbCondition 返回 1 表示可退出，否则返回 0
	virtual HRESULT __stdcall CanQuitNow(int* pbCondition) = 0;


	//模块对主程序有请求时，通过IMainWndNotify接口发出
	virtual HRESULT __stdcall SetMainWndNotify(IMainWndNotify* pNotify) = 0;

	//有个名字方便以后扩展
	//返回显示名，lpszName为null时，len返回需要的buffer大小，包括'\0'，以字符为单位
	virtual HRESULT	__stdcall GetName(LPTSTR lpszName, UINT* len) = 0;

	// 设置模块参数
	virtual HRESULT	__stdcall SetParam(LPCTSTR lpcszParam) = 0;
};


#define STATE_SOFTWARE_UPDATE	0

interface IMainWndNotify
{
	// 通知主窗口选择指定tab页，nTabIndex 主tab索引，nSubTabIndex 二级tab索引，lpcszParam 模块参数
	virtual HRESULT __stdcall SelectPage(int nTabIndex, int nSubTabIndex, LPCTSTR lpcszParam) = 0;

	//当需要显示某个设置页面时候，用来通知主窗口显示此页面
	//pModule标识哪个模块，pPage标识哪个设置页面
	virtual HRESULT __stdcall ShowSettingPage(IModule* pModule, ISettingPage* pPage) = 0;

	//当需要后台运行时，通知主窗口
	virtual HRESULT __stdcall RunInBackground(IModule* pModule) = 0;//pModule标识哪个模块需要后台运行

	//通知主窗口某些它所关心的状态变了
	//主要是给快快软件升级的动态数字设计，顺便看看有没有其它扩展用途
	virtual HRESULT __stdcall StateChanged(IModule* pModule, UINT whichState, void* pData) = 0;
};




//电脑管家会统一各个模块的设置到一个界面（安全防护，软件管家等）
//
//ISettingPage对应于一个具体的设置界面，比如安全防护里面的查杀木马
//
//IModuleSetting对应于一个大的功能模块，由它拿到所有的设置页面
//
//
interface IDirtyCallback : IUnknown
{
	virtual HRESULT __stdcall SetDirty(BOOL bDirty) = 0;
};

interface ISettingPage : IUnknown
{
	//返回显示名，lpszName为null时，len返回需要的buffer大小，包括'\0'，以字符为单位
	virtual HRESULT	__stdcall GetName(LPTSTR lpszName, UINT* len) = 0;

	virtual HRESULT __stdcall GetPageRect(RECT* pRect) = 0;//窗口大小的下限，需要在创建前就支持
	virtual HRESULT __stdcall SetDirtyCallback(IDirtyCallback* pICallback) = 0;//主界面传入的接口，可用来通知主界面设置改变了
	virtual HRESULT __stdcall Apply() = 0;

	virtual HRESULT __stdcall Create(HWND hwndParent, RECT rect, HWND* phwndPage) = 0;
	virtual HRESULT __stdcall Show(BOOL bShow) = 0;
	virtual HRESULT __stdcall Move(RECT rect) = 0;//隐含resize（如果支持）
};

[ uuid("0909C3DC-3C87-4f1d-ABDD-D7465C19674C") ]
interface IModuleSetting : IUnknown
{
	//返回显示名，lpszName为null时，len返回需要的buffer大小，包括'\0'，以字符为单位
	virtual HRESULT	__stdcall GetName(LPTSTR lpszName, UINT* len) = 0;

	//如果ppISettingPage为NULL,pFetched返回支持多少个设置子窗口,主界面然后把含有这么多的ISettingPage*的数组再次传入
	virtual HRESULT __stdcall GetPages(ISettingPage** ppISettingPage, ULONG* pFetched) = 0;
};

#pragma once

#include <softmgr/ISoftInstall.h>
#include <softmgr/iksautoinstcallback.h>

namespace ksm
{

typedef struct PluginInfo
{
	CString		_key;
	CString		_name;
	BOOL		_state;
	BOOL		_curState;
} *PPluginInfo;

typedef CSimpleArray<PluginInfo>  PluginInfoList;

typedef struct InstInfo
{
	DWORD		_instTimeout;
	CString		_instPackPath;
	CString		_downUrl;
	CString		_downName;
	CString		_downMd5;
	CString		_instDir;
	ULONGLONG	_downSize;
	ULONGLONG	_instSize;
	BOOL		_bUseVirtualDesk;
	PluginInfoList _pluginInfoList;

	InstInfo()
	{
		_instTimeout = 0;
	}
} *PInstInfo;

typedef CAtlMap<DWORD, InstInfo> InstInfoMap;
	
class CSoftInstProxy
{
public:
	CSoftInstProxy();
	~CSoftInstProxy();

	void SetNotify(IKSAutoInstCallBack2 *pNotify) 
	{ _pNotify = pNotify; }
	BOOL Initialize(LPCWSTR pKSafePath);
	void Uninitialize();

	// 获取安装信息列表（调用初始化后有效）
	const InstInfoMap& GetInstInfoMap() const 
	{ return _instInfoMap; }

	const InstInfo*	GetInstInfo(DWORD nId) const;
	// 设置安装目录
	void SetInstDir(LPCWSTR pInstDir) 
	{ _instDir = pInstDir; }
	// 设置安装包路径
	BOOL SetInstPackPath(DWORD id, LPCWSTR pInstPackPath);
	// 设置插件信息
	BOOL SetPluginInfo(DWORD id, LPCWSTR pPluginKey, BOOL state);
	// 开始/停止安装软件
	BOOL StartInstSoft(DWORD id);
	void StopInstSoft();

private:
	BOOL LoadInstLib(LPCWSTR pLibPath);

private:
	HMODULE _hModSoftMgr;

	CString _instDir;
	InstInfoMap _instInfoMap;
	ISoftInstall *_pISoftInst;
	IKSAutoInstCallBack2 *_pNotify;
};

}
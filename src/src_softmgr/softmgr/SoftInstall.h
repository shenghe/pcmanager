#pragma once

#include <softmgr/ISoftInstall.h>
#include "ConewTaskMgr.h"
#include "iksautoinstclient.h"

namespace ksm
{

class CSoftInstall : public ISoftInstall
{
public:
	CSoftInstall();
	~CSoftInstall();

public:
	virtual void __stdcall SetNotify(IKSAutoInstCallBack2 *pNotify);
	virtual BOOL __stdcall Initialize(LPCWSTR pKSafePath, LPCWSTR pInstLibPath);
	virtual void __stdcall Uninitialize();

	virtual BOOL __stdcall StartInstSoft(LPCSTR pXml, DWORD cch);
	virtual void __stdcall StopInstSoft();

private:
	HMODULE				 _hAutoInstModule;
	conew::CTaskMgr		 _taskMgr;
	IKSAutoInstCallBack2 *_pNotify;
	IKSAutoInstClient	 *_pKSAutoIstClient;
};

}

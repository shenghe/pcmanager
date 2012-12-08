#include "stdafx.h"
#include "SoftInstall.h"
using namespace conew;
#include "ksautoinstcexportdef.h"
#include <string>
using namespace std;
#include <stlsoft/shims/access/string.hpp>
using namespace stlsoft;

namespace ksm
{

class CSoftInstTask : public IBaseTask
{
public:
	CSoftInstTask(IKSAutoInstClient	*pKSAutoIstClient,IKSAutoInstCallBack2* pNotify, LPSTR pXml, DWORD ccb) 
		: IBaseTask(1), 
		_pKSAutoIstClient(pKSAutoIstClient), 
		_pNotify(pNotify), 
		_pXml(pXml), 
		_ccb(ccb)
	{}

	~CSoftInstTask()
	{ if(_pXml != NULL) delete[] _pXml; }

	virtual BOOL TaskProcess(CTaskMgr *pMgr);

private:
	DWORD _ccb;
	LPCSTR _pXml;
	IKSAutoInstClient	*_pKSAutoIstClient;
	IKSAutoInstCallBack2 *_pNotify;
};

static const wchar_t sInstProxyModule[] = L"KSoft\\kauinsc.dll";
//////////////////////////////////////////////////////////////////////////
CSoftInstall::CSoftInstall()
{
	_hAutoInstModule = NULL;
	_pKSAutoIstClient = NULL;
}

CSoftInstall::~CSoftInstall()
{
}

void CSoftInstall::SetNotify(IKSAutoInstCallBack2 *pNotify)
{
	_pNotify = pNotify;
}

BOOL CSoftInstall::Initialize(LPCWSTR pKSafePath, LPCWSTR pInstLibPath)
{
	if(_pNotify == NULL) return FALSE;

	wstring path = c_str_data_w(pKSafePath);

	_hAutoInstModule = ::LoadLibraryW(c_str_data_w(path + sInstProxyModule));
	if(_hAutoInstModule == NULL) return FALSE;

	KSAICreateModuleFunc pfnKSAICreateModuleFunc =
		(KSAICreateModuleFunc)::GetProcAddress(_hAutoInstModule, "KSAICreateModule");
	if(pfnKSAICreateModuleFunc == NULL) return FALSE;

	HRESULT hr = pfnKSAICreateModuleFunc(__uuidof(IKSAutoInstClient), (void**)&_pKSAutoIstClient);
	if(!SUCCEEDED(hr)) return FALSE;

	return SUCCEEDED(_pKSAutoIstClient->Init(pInstLibPath, NULL));
}

void CSoftInstall::Uninitialize()
{
	if(_pKSAutoIstClient != NULL)
	{
		_pKSAutoIstClient->NotifyStop();

		if(_taskMgr.IsStarted())
		{
			_taskMgr.Shutdown(5000);
		}

		_pKSAutoIstClient->UnInit();
		_pKSAutoIstClient->Release();
		_pKSAutoIstClient = NULL;
	}

	if(_hAutoInstModule != NULL)
	{
		::FreeLibrary(_hAutoInstModule);
		_hAutoInstModule = NULL;
	}

	delete this;
}

BOOL CSoftInstall::StartInstSoft(LPCSTR pXml, DWORD cch)
{
	if(_pKSAutoIstClient == NULL ||
		pXml == NULL || cch == 0
		)
	{
		return FALSE;
	}

	LPSTR pXmlTemp = new char[cch+1];
	if(pXmlTemp == NULL) return FALSE;
	memcpy(pXmlTemp, pXml, cch);
	pXmlTemp[cch] = 0;

	IBaseTask *pTask = new CSoftInstTask(_pKSAutoIstClient, _pNotify, pXmlTemp, cch);
	if(pTask == NULL)
	{
		delete[] pXmlTemp;
		return FALSE;
	}

	if(!_taskMgr.IsStarted())
	{
		_taskMgr.Startup();
	}

	_taskMgr.AddTask(pTask);
	_taskMgr.StartTask();

	pTask->Release();
	return TRUE;
}

void CSoftInstall::StopInstSoft()
{
	_pKSAutoIstClient->NotifyStop();
}

BOOL CSoftInstTask::TaskProcess(CTaskMgr *pMgr)
{
	HRESULT	hr = S_OK;
	::SetThreadLocale(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED));

	_pKSAutoIstClient->SetCallBack( static_cast<IKSAutoInstCallBack*>(_pNotify) );

	hr = _pKSAutoIstClient->StartInstSoft(_pXml, _ccb);

	_pNotify->OnInstSoftComplete(hr);

	return FALSE;
}

}
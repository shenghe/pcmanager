#pragma once

#include "kclear/ikisgui.h"
#include "kscmaindlg.h"

class kclearimplforduba: public IModule
{
public:
	kclearimplforduba(void);
	~kclearimplforduba(void);

	STDMETHOD(Init)();
	STDMETHOD(Uninit)();
	STDMETHOD(Create)(HWND hwndParent, RECT rect, HWND* phwndCreated);
	STDMETHOD(Show)(BOOL bShow);
	STDMETHOD(Move)(RECT rect);
	STDMETHOD(CanQuitNow)(int* pbCondition);
	STDMETHOD(SetMainWndNotify)(IMainWndNotify* pNotify);
	STDMETHOD(GetName)(LPTSTR lpszName, UINT* len);
	STDMETHOD(GetPages)(ISettingPage** ppISettingPage, ULONG* pFetched);
	STDMETHOD(SetParam)(LPCTSTR lpcszParam);

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in ] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

private:
	LONG	m_cRef;

private:
	CKscMainDlg* m_pDlgMain;
	HWND m_hwnd;
	TCHAR m_szCmd[MAX_PATH];
};

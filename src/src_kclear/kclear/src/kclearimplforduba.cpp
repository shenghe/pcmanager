#include "StdAfx.h"
#include "kclearimplforduba.h"
#include "tinyxml.h"
#include "kclearnavigate.h"
#include "../../publish/bksafe/bksafenavigatestring.h"
#include "kscbase/kscsys.h"

bool g_bKisDlg = false;

kclearimplforduba::kclearimplforduba(void) : m_pDlgMain(NULL)
{
	m_cRef = 0;
	m_hwnd = NULL;
	ZeroMemory( m_szCmd, MAX_PATH);
}

kclearimplforduba::~kclearimplforduba(void)
{
}

HRESULT STDMETHODCALLTYPE kclearimplforduba::QueryInterface( 
	/* [in ] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject)
{
	if ( !ppvObject )
		return E_INVALIDARG;

	if ( ( IID_IUnknown == riid ) || ( __uuidof(IModule) == riid ) )
	{
		*ppvObject = static_cast<IModule*>(this);
	}
	else                                     
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE kclearimplforduba::AddRef(void)
{
	return InterlockedIncrement((LONG volatile *)&m_cRef);
}

ULONG STDMETHODCALLTYPE kclearimplforduba::Release(void)
{
	if (0L == InterlockedDecrement((LONG volatile *)&m_cRef))
	{
		delete this;
		return 0L;
	}
	return m_cRef;
}

HRESULT __stdcall kclearimplforduba::Init()
{
	HRESULT hr = S_OK;
	return hr;
}
HRESULT __stdcall kclearimplforduba::Uninit()
{
	HRESULT hr = S_OK;
	return hr;
}
HRESULT __stdcall kclearimplforduba::Create(HWND hwndParent, RECT rect, HWND* phwndCreated)
{
	HWND retval = NULL;
	HRESULT hr = 0;
	LONG	lStyle = 0;

	if (m_pDlgMain == NULL)
	{
		m_pDlgMain = new CKscMainDlg();
		if (!m_pDlgMain)
			goto clean0;

        m_pDlgMain->DisableTab(_T("bigfile"));
		m_hwnd = m_pDlgMain->Create(hwndParent);
	}

	if (this->m_hwnd == NULL)
	{
		goto clean0;
	}

	lStyle = ::GetWindowLong(this->m_hwnd, GWL_STYLE);
	lStyle &= ~(DS_MODALFRAME | WS_CAPTION | WS_DLGFRAME | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU );
	lStyle |= WS_CHILD;
	::SetWindowLong(this->m_hwnd, GWL_STYLE, lStyle);
	::SetParent(this->m_hwnd, hwndParent);

	if (!m_pDlgMain->SetWindowPos(NULL,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER))
		hr = E_FAIL;

clean0:
	*phwndCreated = m_hwnd;
	return hr;
}
HRESULT __stdcall kclearimplforduba::Show(BOOL bShow)
{
	HRESULT hr = S_OK;

	if (bShow)
	{
		CRect rcClient;
		m_pDlgMain->GetClientRect(&rcClient);
		if (!m_pDlgMain->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE))
			hr = E_FAIL;
		else
		{
			::SetForegroundWindow(this->m_hwnd);
			m_pDlgMain->SetFocus();
		}
	}
	else
	{
		if (!m_pDlgMain->ShowWindow(SW_HIDE))
			hr = E_FAIL;
	}

	return hr;
}
HRESULT __stdcall kclearimplforduba::Move(RECT rect)
{
	HRESULT hr = S_OK;
	if (!m_pDlgMain->SetWindowPos(NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER))
		hr = E_FAIL;
	return hr;
}


HRESULT __stdcall kclearimplforduba::CanQuitNow(int* pbCondition)
{
	if (0 == *pbCondition)
	{
		if (m_pDlgMain->CanQuitNow())
		{
			*pbCondition = 1;
			return 1;
		}
		*pbCondition = 0;
	}
	
	return 0;
}

HRESULT __stdcall kclearimplforduba::SetMainWndNotify(IMainWndNotify* pNotify)
{
	return S_OK;
}


HRESULT __stdcall kclearimplforduba::GetName(LPTSTR lpszName, UINT* len)
{
	return S_OK;
}

HRESULT __stdcall kclearimplforduba::GetPages(ISettingPage** ppISettingPage, ULONG* pFetched)
{
	return S_OK;
}

HRESULT __stdcall kclearimplforduba::SetParam(LPCTSTR lpcszParam)
{
	_tcscpy(m_szCmd, lpcszParam);

	if (_tcslen(m_szCmd) == 0)
		return S_OK;

	if (0 == _tcsicmp(m_szCmd, _T("StartScan")))
	{//隐私保护器，立即扫描
		PostMessage(m_pDlgMain->m_hWnd, WM_USER+250, 0, 0);
		return S_OK;
	}


	if (m_pDlgMain == NULL)
	{	
		HWND hwndParent = FindWindow(NULL, _T("金山毒霸2011"));//只能这样了，窗口句柄没传过来
		RECT rect = {8,99,796,541};
		this->Create(hwndParent, rect, &m_hwnd);
	}

	//设置指定tab为当前,非xml解析兼容旧方法
	int nTabIndex = 0;

	if (0 == _tcsicmp(m_szCmd, KCLEARNS_ONEKEY) ||
		0 == _tcsicmp(m_szCmd, BKSFNS_SYSOPT_CLR_ONEKEY))
	{
		nTabIndex = 0;
	}
	else if (0 == _tcsicmp(m_szCmd, KCLEARNS_TRASHCLEANER) ||
		0 == _tcsicmp(m_szCmd, BKSFNS_SYSOPT_CLR_RUBBISH))
	{
		nTabIndex = 1;
	}
	else if (0 == _tcsicmp(m_szCmd, KCLEARNS_TRACKCLEANER) ||
		0 == _tcsicmp(m_szCmd, BKSFNS_SYSOPT_CLR_HENJI))
	{
		nTabIndex = 2;
	}
	else if (0 == _tcsicmp(m_szCmd, KCLEARNS_REGCLEANER) ||
		0 == _tcsicmp(m_szCmd, BKSFNS_SYSOPT_CLR_REG))
	{
		nTabIndex = 3;
	}
	else if (0 == _tcsicmp(m_szCmd, KCLEARNS_BIGCLEANER) ||
		0 == _tcsicmp(m_szCmd, BKSFNS_SYSOPT_CLR_SHOUSHEN))
	{
		nTabIndex = 4;
	}
	else
	{//新方法
		try
		{
			TiXmlDocument xmlDoc;
			const TiXmlElement* pXmlItem = NULL;
			char pbuffer[1024] = { 0 };
			strcpy(pbuffer, KUTF16_To_ANSI(lpcszParam));

			if (!xmlDoc.LoadBuffer(pbuffer, (long)strlen(pbuffer), TIXML_ENCODING_UTF8))
				return S_OK;


			pXmlItem = xmlDoc.FirstChildElement("tab");
			if (!pXmlItem)
				return S_OK;

			char szIndex[10] = { 0 };
			strcpy(szIndex, pXmlItem->Attribute("index"));
			nTabIndex = atoi(szIndex);
		}
		catch (...)
		{
			return E_FAIL;
		}
	}

	m_pDlgMain->SetTabCurSel(IDC_TAB_MAIN, nTabIndex);
	return S_OK;
}


STDAPI DllGetClassObject(const IID & rclsid, const IID & riid, LPVOID* ppv)//special for BCM
{
	HRESULT hr = E_FAIL;

	kclearimplforduba* pCKSafePlugin = new kclearimplforduba();
	if (!pCKSafePlugin)
	{
		return E_OUTOFMEMORY;
	}
	pCKSafePlugin->AddRef();

	hr = pCKSafePlugin->QueryInterface(riid, ppv);
	pCKSafePlugin->Release();
	g_bKisDlg = true;
	return hr;
}
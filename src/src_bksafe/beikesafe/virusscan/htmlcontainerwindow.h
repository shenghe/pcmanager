//不能改变whwindow.h里面的代码，只能另外起个文件。
#pragma once
#include<wtlhelper/whwindow.h>

//#define WM_MSG_CALL_DUBA		(WM_USER+ 1678)

class CWHHtmlContainerWindowEx
	: public CWindowImpl<CWHHtmlContainerWindowEx>
	, public IDispEventImpl<IDYES, CWHHtmlContainerWindowEx>
{
public:
	CWHHtmlContainerWindowEx()
		: m_bNavigateError(FALSE)
		, m_crBg(RGB(0xFF, 0xFF, 0xFF))
		, m_hMsgWnd(NULL)
	{
	}

	HWND Create(HWND hWndParent, UINT nID, BOOL bVisible = TRUE, COLORREF crBg = RGB(0xFF, 0xFF, 0xFF))
	{
		HWND hWnd = __super::Create(hWndParent, NULL, NULL, WS_CHILD/* | WS_CLIPCHILDREN*/ | (bVisible ? WS_VISIBLE : 0), 0, nID);
		if (!hWnd)
			return NULL;

		m_crBg = crBg;

		_CreateIEControl(hWnd);

		return hWnd;
	}

	void SetMsg( HWND hWnd )
	{
		m_hMsgWnd = hWnd;
	}

	void Show(LPCTSTR lpszURL)
	{
		m_bNavigateError = FALSE;

		m_strUrl = lpszURL;

		SetTimer(1, 500);

		//         ::CloseHandle(::CreateThread(NULL, 0, _CreateCtrlProc, this, 0, NULL));
	}

	// Navi 失败后自动加载一个本地页面
	void Show2(LPCTSTR lpszUrl, LPCTSTR lpszErrorUrl)
	{
		if (lpszErrorUrl != NULL)
			m_strErrorUrl = lpszErrorUrl;

		Show(lpszUrl);
	}

	void SetBkColor(COLORREF color)
	{
		m_crBg = color;
		Invalidate();
	}

protected:

	CAxWindow m_wndIE;
	CString m_strUrl;
	BOOL m_bNavigateError;
	COLORREF m_crBg;
	CString	  m_strErrorUrl;
	HWND	 m_hMsgWnd;

	static DWORD WINAPI _CreateCtrlProc(LPVOID pvParam)
	{
		CWHHtmlContainerWindowEx *pThis = (CWHHtmlContainerWindowEx *)pvParam;
		CComPtr<IWebBrowser2> piWebBrowser;
		CComVariant v;


		::Sleep(1000);

		pThis->m_wndIE.QueryControl(&piWebBrowser);

		if (piWebBrowser)
		{
			piWebBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
			piWebBrowser->put_Silent( VARIANT_TRUE );
			piWebBrowser->Navigate(CComBSTR(pThis->m_strUrl), &v, &v, &v, &v);
		}

		return 0;
	}

	void WINAPI NavigateComplete2WebAdinfo(LPDISPATCH pDisp, VARIANT* URL)
	{
		if (!m_bNavigateError)
		{
			m_wndIE.ShowWindow(SW_SHOW);
		}
	}

	void WINAPI NavigateErrorWebAdinfo(
		IDispatch* pDisp,
		VARIANT* pvtURL,
		VARIANT* pvtFrame,
		VARIANT* pvtStatusCode,
		VARIANT_BOOL* pvbCancel
		)
	{
		BOOL bNaviErrorPage = FALSE;
		if (pvtURL!=NULL && !m_strErrorUrl.IsEmpty())
		{
			CComBSTR bstrUrl(pvtURL->bstrVal);
			if (bstrUrl == CComBSTR(m_strUrl))
			{
				CComPtr<IWebBrowser2> piWebBrowser;
				CComVariant v;

				m_wndIE.QueryControl(&piWebBrowser);

				if (piWebBrowser)
				{
					piWebBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
					piWebBrowser->put_Silent( VARIANT_TRUE );
					piWebBrowser->Navigate(CComBSTR(m_strErrorUrl), &v, &v, &v, &v);
					bNaviErrorPage = TRUE;
				}
			}
		}


		if ( !bNaviErrorPage)
		{
			m_bNavigateError = TRUE;
		}
		*pvbCancel = VARIANT_TRUE;
	}

	void WINAPI NewWindow3WebAdinfo(
		IDispatch **ppDisp,
		VARIANT_BOOL *Cancel,
		DWORD dwFlags,
		BSTR bstrUrlContext,
		BSTR bstrUrl)
	{
		CString strUrl(bstrUrl);

		if( m_hMsgWnd )
			::SendMessage( m_hMsgWnd, WM_MSG_CALL_DUBA, (WPARAM)(LPCTSTR)strUrl, NULL );

		*Cancel = TRUE;
	}

	void OnDestroy()
	{
		AtlAdviseSinkMap(this, FALSE);
		if (m_wndIE.IsWindow())
			m_wndIE.DestroyWindow();
	}

	BOOL OnEraseBkgnd(CDCHandle dc)
	{
		if (!m_wndIE.IsWindow() || !m_wndIE.IsWindowVisible())
		{
			CRect rcClient;
			GetClientRect(rcClient);
			
			dc.FillSolidRect(rcClient, m_crBg);
			dc.SetBkMode(TRANSPARENT);
			HFONT hftOld = dc.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
			
			CString strText = L"正在读取信息...";
			dc.DrawText(strText, strText.GetLength(), rcClient, DT_SINGLELINE);
			dc.SelectFont(hftOld);
		}

		return TRUE;
	}

	void OnPaint(CDCHandle /*dc*/)
	{
		CPaintDC dc(m_hWnd);

		if (!m_wndIE.IsWindow() || !m_wndIE.IsWindowVisible())
		{
			CRect rcClient;

			GetClientRect(rcClient);

			dc.FillSolidRect(rcClient, m_crBg);
		}
	}

	void OnSize(UINT nType, CSize /*size*/)
	{
		if (m_wndIE.IsWindow())
		{
			CRect rcClient;

			GetClientRect(rcClient);

			m_wndIE.MoveWindow(rcClient);
		}
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		if (1 != nIDEvent)
			return;

		KillTimer(1);

		CComPtr<IWebBrowser2> piWebBrowser;
		CComVariant v;

		//::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		m_wndIE.QueryControl(&piWebBrowser);

		if (piWebBrowser)
		{
			piWebBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
			piWebBrowser->put_Silent( VARIANT_TRUE );
			piWebBrowser->Navigate(CComBSTR(m_strUrl), &v, &v, &v, &v);
		}

		//::CoUninitialize();
	}

	void _CreateIEControl(HWND hWnd)
	{
		CRect rcClient;

		GetClientRect(rcClient);

		m_wndIE.Create(m_hWnd, rcClient, NULL, WS_CHILD | WS_CLIPCHILDREN);

		m_wndIE.CreateControl(L"Shell.Explorer");
		m_wndIE.SetDlgCtrlID(IDYES);

		AtlAdviseSinkMap(this, TRUE);
	}

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		CComPtr<IWebBrowser2> pWebBrowser;
		m_wndIE.QueryControl(&pWebBrowser);

		if (pWebBrowser)
		{
			IServiceProvider* pServiceProvider = NULL;
			if (SUCCEEDED(pWebBrowser->QueryInterface(
				IID_IServiceProvider, 
				(void**)&pServiceProvider)))
			{			
				IOleWindow* pWindow = NULL;
				if (SUCCEEDED(pServiceProvider->QueryService(
					SID_SShellBrowser, 
					IID_IOleWindow,
					(void**)&pWindow)))
				{
					HWND hwndBrowser = NULL;
					if (SUCCEEDED(pWindow->GetWindow(&hwndBrowser)))
					{
						OutputDebugString(_T("发送消息"));

						HWND hwnd = NULL;
						hwnd = ::FindWindowEx(hwndBrowser,NULL,TEXT("Shell DocObject View"),NULL);
						if (hwnd)
						{
							HWND hIE = ::FindWindowEx(hwnd,NULL,TEXT("Internet Explorer_Server"),NULL);

							if ( zDelta < 0 )
							{
								::SendMessage(hIE, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN,0),0);
							}
							else
							{
								::SendMessage(hIE, WM_VSCROLL, MAKEWPARAM(SB_LINEUP,0),0);
							}

						}

					}

					pWindow->Release();
				}
				pServiceProvider->Release();
			} 
		}		

		return FALSE;
	}

public:
	BEGIN_MSG_MAP(CWHHtmlContainerWindowEx)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_SINK_MAP(CWHHtmlContainerWindowEx)
		SINK_ENTRY(IDYES, 252, NavigateComplete2WebAdinfo)
		SINK_ENTRY(IDYES, 271, NavigateErrorWebAdinfo)
		SINK_ENTRY(IDYES, 273, NewWindow3WebAdinfo)
	END_SINK_MAP()
};
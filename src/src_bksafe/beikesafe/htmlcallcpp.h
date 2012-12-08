
#pragma once
#include "wtlhelper/whwindow.h"

static const DISPID							BASE_DIPSPID = 0x100000;
//////////////////////////////////////////////////////////////////////////
//现在支持的回调函数调用原型
typedef void		(*JSCallbackFunction)(DISPPARAMS* params, VARIANT* retval, void* pContext);
typedef const char* (*JSSimpleCallbackFunction)(int argc, const char** argv, void* pContext);

//////////////////////////////////////////////////////////////////////////
//注册回调函数需要的参数
struct JsCallHandler 
{ 
	DISPID		DispFuncID; 
	union {
		JSCallbackFunction				normal; 
		JSSimpleCallbackFunction		simple; 
	} callback;
	void*		pContext;
	BOOL		bSimple;


	JsCallHandler()
	{
		DispFuncID = 0;
		pContext = NULL;
		bSimple = 0;
	}
};


//////////////////////////////////////////////////////////////////////////
//回调函数管理器
class CBKCallBackFuncContainer
{
public:
	static CBKCallBackFuncContainer* GetFuncPtr()
	{
		static CBKCallBackFuncContainer func;
		return &func;
	}
	~CBKCallBackFuncContainer(){}

	//////////////////////////////////////////////////////////////////////////
	//注册JS调用函数
	void ConnectJSFuncHandler(BSTR name, JSCallbackFunction func, BOOL simple, void* pContext)
	{
		JsCallHandler handler;		
		handler.DispFuncID = BASE_DIPSPID + (DISPID)m_mapJSHandlers.GetCount(); // generate a unique id
		handler.callback.normal = func;
		handler.bSimple = simple;
		handler.pContext = pContext;
		m_mapJSHandlers[name] = handler;
	}


	//////////////////////////////////////////////////////////////////////////
	//检测是否注册过回调
	DISPID NameId(BSTR name)
	{
		if (NULL != m_mapJSHandlers.Lookup(name))
			return m_mapJSHandlers[name].DispFuncID;

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//根据JS的调用，来判断调用本地的具体函数
	HRESULT Invoke(DISPID id, DISPPARAMS* params, VARIANT* retval)
	{
		HRESULT hr = S_OK;
		POSITION pos = m_mapJSHandlers.GetStartPosition();
		while(NULL != pos)
		{
			JsCallHandler JScall = m_mapJSHandlers.GetValueAt(pos);
			if (JScall.DispFuncID == id)
			{
				if (FALSE == JScall.bSimple)
				{
					JScall.callback.normal(params, retval, JScall.pContext);
				}
				else
				{
					int count = params->cArgs;
					if (count > 256) 
						return E_INVALIDARG; // no more than 256 args
					// convert args
					const char* argv[256] = {0};
					count --;

					for (int i=0; count >= 0; i++, count--)
						argv[i] = CW2A(params->rgvarg[count].bstrVal);

					const char* retstr = (JScall.callback.simple)(params->cArgs, argv, JScall.pContext);
					if (retval)
					{
						CComVariant res(static_cast<LPSTR>((LPSTR)retstr));
						res.Detach(retval); 
					}

				}

				break;
			}
			m_mapJSHandlers.GetNext(pos);
		}

		return S_OK;
	}
private:
	CAtlMap<CString, JsCallHandler>				m_mapJSHandlers;

	CBKCallBackFuncContainer()
	{
		m_mapJSHandlers.RemoveAll();
	}

};


//////////////////////////////////////////////////////////////////////////
//注册回调函数宏
//根据不同需要支持注册两个函数原型，注意CONNECT_JS_CALL_SIMPLE_HANDLER中注册的函数
//需要提前和HTML调用协调好，参数必须都为字符，负责会引起未知问题
#define CONNECT_JS_CALL_HANDLER(name, func, pContext) do \
{\
	CBKCallBackFuncContainer::GetFuncPtr()->ConnectJSFuncHandler(L#name, (JSCallbackFunction)&func, FALSE, pContext);\
} while (FALSE);


#define CONNECT_JS_CALL_SIMPLE_HANDLER(name, func, pContext)do \
{\
	CBKCallBackFuncContainer::GetFuncPtr()->ConnectJSFuncHandler(L#name, (JSCallbackFunction)&func, TRUE,	pContext);\
} while (FALSE);



//////////////////////////////////////////////////////////////////////////
//DispatchInterface
class __declspec(uuid("5CE2BDBA-12D7-4c08-8924-67B53F511723"))

IHtmlCallDispatch
	: public IDispatch
{
public:
	IHtmlCallDispatch()
	{
		m_cRef = 0;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;
		if (riid == __uuidof(IDispatch))
		{
			*ppvObject = (IDispatch*)this;
			hr = S_OK;
		} 
		else if (riid == __uuidof(DWebBrowserEvents2))
		{
			*ppvObject = (DWebBrowserEvents2*)this;
			hr = S_OK;
		}
		return hr;
	}

protected:
	ULONG m_cRef;
 
 	ULONG STDMETHODCALLTYPE AddRef(void) { return ++m_cRef; }

 	ULONG STDMETHODCALLTYPE Release(void) { return --m_cRef; }

	STDMETHOD(GetTypeInfoCount)(UINT*) { return E_NOTIMPL; }
	STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) { return E_NOTIMPL; }

	STDMETHODIMP GetIDsOfNames(REFIID, LPOLESTR *rgszNames, UINT, LCID, DISPID *rgDispId)
	{
		*rgDispId = CBKCallBackFuncContainer::GetFuncPtr()->NameId(rgszNames[0]);//_NameId(rgszNames[0]);
		return S_OK;
	}


	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, 
		VARIANT* pVarResult, EXCEPINFO*, UINT*)
	{
		HRESULT hr = E_NOTIMPL;
		if (dispIdMember >= BASE_DIPSPID)
			hr = CBKCallBackFuncContainer::GetFuncPtr()->Invoke(dispIdMember, pDispParams, pVarResult);

		return S_OK;
	}
};

//////////////////////////////////////////////////////////////////////////
//UIHandlerHost
class __declspec(uuid("669FBACB-4693-4945-9CB5-5F327F0A4A50"))

	IHtmlCallDocHostUIHandlerDispatch
	: public IDocHostUIHandlerDispatch
{
public:
	IHtmlCallDocHostUIHandlerDispatch()
	{
		m_ptrHtmlDispatchPtr = NULL;
		 m_cRef = 0;
	}


	IDispatch* SetHtmlDispatchPtr(IDispatch* ptrDispatch)
	{
		IDispatch* ptr = m_ptrHtmlDispatchPtr;
		m_ptrHtmlDispatchPtr = ptrDispatch;

		return ptr;
	}

	STDMETHOD(GetTypeInfoCount)(UINT*) { return E_NOTIMPL; }
	STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) { return E_NOTIMPL; }
	STDMETHODIMP GetIDsOfNames(REFIID, LPOLESTR *rgszNames, UINT, LCID, DISPID *rgDispId)
	{
		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS* pDispParams, 
		VARIANT* pVarResult, EXCEPINFO*, UINT*)
	{
		return S_OK;

		return E_NOTIMPL;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) { return ++m_cRef; }
	// 
	ULONG STDMETHODCALLTYPE Release(void) { return --m_cRef; }

	HRESULT ( STDMETHODCALLTYPE QueryInterface )( 
		/* [in] */ REFIID riid,
		/* [annotation][iid_is][out] */ 
		__RPC__deref_out  void **ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;
		if (riid == __uuidof(IDocHostUIHandlerDispatch))
		{
			*ppvObject = (IDocHostUIHandlerDispatch*)this;
			hr = S_OK;
		}else if (riid == __uuidof(IUnknown))
		{
			*ppvObject = (IUnknown*)this;
			hr = S_OK;
		}
		return hr;
		 
	}
	virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( 
		/* [in] */ DWORD dwID,
		/* [in] */ DWORD x,
		/* [in] */ DWORD y,
		/* [in] */ IUnknown *pcmdtReserved,
		/* [in] */ IDispatch *pdispReserved,
		/* [retval][out] */ HRESULT *dwRetVal){ 

			*dwRetVal = S_OK;
			return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE GetHostInfo( 
		/* [out][in] */ DWORD *pdwFlags,
		/* [out][in] */ DWORD *pdwDoubleClick) { 
			*pdwFlags = DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DOUTERBORDER;
			return S_OK; 
	};

	virtual HRESULT STDMETHODCALLTYPE ShowUI( 
		/* [in] */ DWORD dwID,
		/* [in] */ IUnknown *pActiveObject,
		/* [in] */ IUnknown *pCommandTarget,
		/* [in] */ IUnknown *pFrame,
		/* [in] */ IUnknown *pDoc,
		/* [retval][out] */ HRESULT *dwRetVal) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE HideUI( void) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE UpdateUI( void) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
		/* [in] */ VARIANT_BOOL fEnable) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
		/* [in] */ VARIANT_BOOL fActivate) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
		/* [in] */ VARIANT_BOOL fActivate) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE ResizeBorder( 
		/* [in] */ long left,
		/* [in] */ long top,
		/* [in] */ long right,
		/* [in] */ long bottom,
		/* [in] */ IUnknown *pUIWindow,
		/* [in] */ VARIANT_BOOL fFrameWindow) { 
			//fFrameWindow = VARIANT_FALSE;
			return E_NOTIMPL; 
	};

	virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
		/* [in] */ DWORD_PTR hWnd,
		/* [in] */ DWORD nMessage,
		/* [in] */ DWORD_PTR wParam,
		/* [in] */ DWORD_PTR lParam,
		/* [in] */ BSTR bstrGuidCmdGroup,
		/* [in] */ DWORD nCmdID,
		/* [retval][out] */ HRESULT *dwRetVal) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
		/* [out] */ BSTR *pbstrKey,
		/* [in] */ DWORD dw) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
		/* [in] */ IUnknown *pDropTarget,
		/* [out] */ IUnknown **ppDropTarget) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE GetExternal( 
		/* [out] */ IDispatch **ppDispatch) { 

			if (NULL != m_ptrHtmlDispatchPtr)
				*ppDispatch = static_cast<IHtmlCallDispatch*>(m_ptrHtmlDispatchPtr); 
			return S_OK; 
	};

	virtual HRESULT STDMETHODCALLTYPE TranslateUrl( 
		/* [in] */ DWORD dwTranslate,
		/* [in] */ BSTR bstrURLIn,
		/* [out] */ BSTR *pbstrURLOut) { return E_NOTIMPL; };

	virtual HRESULT STDMETHODCALLTYPE FilterDataObject( 
		/* [in] */ IUnknown *pDO,
		/* [out] */ IUnknown **ppDORet) { return E_NOTIMPL; };

	private:
		IDispatch*			m_ptrHtmlDispatchPtr;
		ULONG				m_cRef;
};



//////////////////////////////////////////////////////////////////////////
//IE
class CWHHtmlContainerWindow2
	: public CWHHtmlContainerWindow
{
public:
	CWHHtmlContainerWindow2()
	{
		//dispatch
		m_ptrDispatch = NULL;
		m_HtmlDiapatch.QueryInterface(__uuidof(IDispatch), (void**)&m_ptrDispatch);

		//HostUIHandle
		m_ptrHostUIDispatch = NULL;
		m_HostUIHandleDispatch.QueryInterface(__uuidof(IDocHostUIHandlerDispatch), (void**)&m_ptrHostUIDispatch);

	}
	~CWHHtmlContainerWindow2()
	{
		if (NULL != m_ptrDispatch)
			m_ptrDispatch->Release();

		m_ptrDispatch = NULL;

		if (NULL != m_ptrHostUIDispatch)
			m_ptrHostUIDispatch->Release();

		m_ptrHostUIDispatch = NULL;
	}

	IDispatch* SetDispatchPtr(IDispatch* ptrDispatch)
	{
		if (NULL == ptrDispatch)
			return NULL;

		IDispatch* ptr = m_ptrDispatch;
		m_ptrDispatch = ptrDispatch;

		return ptr;
	}

	IDocHostUIHandlerDispatch* SetHostUIHandlerDispatch(IDocHostUIHandlerDispatch* ptrHostUIDispatch)
	{
		if (NULL == m_ptrHostUIDispatch)
			return NULL;

		IDocHostUIHandlerDispatch* ptr = m_ptrHostUIDispatch;
		m_ptrHostUIDispatch = ptrHostUIDispatch;

		return ptr;
	}

	HWND Create(HWND hWndParent, UINT nID, BOOL bVisible /* = TRUE */, COLORREF crBg /* = RGB */)
	{
		HWND hWnd = __super::Create(hWndParent, nID, bVisible, crBg);

		if (NULL != m_ptrDispatch)
			m_wndIE.SetExternalDispatch(m_ptrDispatch);	

/*
		// 暂时屏蔽掉这个功能，在IE6下显示有边框 [1/11/2011 zhangbaoliang]
		if (NULL != m_ptrHostUIDispatch)
		{
			if (NULL != m_ptrDispatch)
				m_HostUIHandleDispatch.SetHtmlDispatchPtr(m_ptrDispatch);

			m_wndIE.SetExternalUIHandler(m_ptrHostUIDispatch);
		}
*/

		return hWnd;
	}

	//////////////////////////////////////////////////////////////////////////
	//本地调用JS
	HRESULT CallJS(BSTR func, DISPPARAMS* params, VARIANT* retval)
	{
		HRESULT hr = S_OK;

		do 
		{
			IWebBrowser2* browser;
			hr = m_wndIE.QueryControl(__uuidof(IWebBrowser2), (void**)&browser);
			if (FAILED(hr))
				break;

			IDispatch* disp;
			hr = browser->get_Document(&disp);
			if (FAILED(hr))
				break;

			IHTMLDocument2* doc;
			hr = disp->QueryInterface(__uuidof(IHTMLDocument2), (void**)&doc);
			if (FAILED(hr))
				break;

			IDispatch* spDisp;
			hr = doc->get_Script(&spDisp);
			if (FAILED(hr))
				break;

			USES_CONVERSION;
			OLECHAR FAR* name = func;
			DISPID dispid;
			hr = spDisp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);		
			if (FAILED(hr))
				break;

			hr = spDisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
				DISPATCH_METHOD, params, retval, NULL, NULL);


		} while (FALSE);

		return hr;
	}

private:
	IHtmlCallDispatch					m_HtmlDiapatch;
	IDispatch*							m_ptrDispatch;

	IHtmlCallDocHostUIHandlerDispatch	m_HostUIHandleDispatch;
	IDocHostUIHandlerDispatch*			m_ptrHostUIDispatch;
};
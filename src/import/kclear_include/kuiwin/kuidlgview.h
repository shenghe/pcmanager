//////////////////////////////////////////////////////////////////////////
//  Class Name: CKuiDlgView
// Description: Dialog View, Real Container of KuiWindow
//     Creator: ZhangXiaoxuan
//     Version: 2010.3.12 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlconv.h>

#include "kuiwndnotify.h"
#include "kuiwndpanel.h"

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// KuiWinManager
// CKuiViewImpl
// CKuiDialogViewTraits
// CKuiDialogViewImpl
// CKuiDialogView
// CKuiDialogImpl
// CKuiSimpleDialog

//////////////////////////////////////////////////////////////////////////
// KuiWinManager

class KuiWinManager
{
public:
    KuiWinManager()
        : m_hWndActive(NULL)
    {
        if (NULL == ms_mangager)
            ms_mangager = this;

        ::InitializeCriticalSection(&m_lockRepaint);
    }

    virtual ~KuiWinManager()
    {
        if (this == ms_mangager)
            ms_mangager = NULL;

        ::DeleteCriticalSection(&m_lockRepaint);
    }

    static HWND SetActive(HWND hWnd)
    {
        HWND hWndLastActive = NULL;

        if (ms_mangager)
        {
            hWndLastActive = ms_mangager->m_hWndActive;
            ms_mangager->m_hWndActive = hWnd;
        }

        return hWndLastActive;
    }

    static HWND GetActive()
    {
        if (ms_mangager)
            return ms_mangager->m_hWndActive;

        return NULL;
    }

    static void EnterPaintLock()
    {
        if (ms_mangager)
            ::EnterCriticalSection(&(ms_mangager->m_lockRepaint));
    }

    static void LeavePaintLock()
    {
        if (ms_mangager)
            ::LeaveCriticalSection(&(ms_mangager->m_lockRepaint));
    }

protected:
    static KuiWinManager* ms_mangager;

    HWND m_hWndActive;
    CRITICAL_SECTION m_lockRepaint;
};

__declspec(selectany) KuiWinManager* KuiWinManager::ms_mangager = NULL;

//////////////////////////////////////////////////////////////////////////

template <class T>
class ATL_NO_VTABLE CKuiViewImpl
{
public:

    BOOL SetRichText(UINT uItemID, UINT uResID)
    {
        CStringA strXml;
        BOOL bRet = KuiResUtil::LoadKuiXmlResource(uResID, strXml);
        if (!bRet)
            return FALSE;

        return SetRichText(uItemID, strXml);
    }

    BOOL SetRichText(UINT uItemID, LPCWSTR lpszXml)
    {
        CStringA strXml = CW2A(lpszXml, CP_UTF8);

        return SetRichText(uItemID, strXml);
    }

    BOOL SetRichText(UINT uItemID, LPCSTR lpszXml)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (!pWnd)
            return FALSE;

        if (!pWnd->IsClass(CKuiRichText::GetClassName()))
            return FALSE;

        TiXmlDocument xmlDoc;

        { // Free stack
            CStringA strXml;
            strXml = L'<';
            strXml += CKuiRichText::GetClassName();
            strXml += L'>';
            strXml += lpszXml;
            strXml += L'<';
            strXml += L'/';
            strXml += CKuiRichText::GetClassName();
            strXml += L'>';
            xmlDoc.Parse(strXml, NULL, TIXML_ENCODING_UTF8);
        }

        if (xmlDoc.Error())
            return FALSE;

        CKuiRichText *pWndRichText = (CKuiRichText *)pWnd;
        pWndRichText->LoadChilds(xmlDoc.RootElement());

        pT->_InvalidateControl(pWnd);

        if (pWnd->IsVisible(TRUE))
            pWnd->KuiSendMessage(WM_SHOWWINDOW, TRUE);

        return TRUE;
    }

    BOOL SetItemText(UINT uItemID, LPCTSTR lpszText)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
        {
            pT->_InvalidateControl(pWnd);

            pWnd->SetInnerText(lpszText);

            pT->_InvalidateControl(pWnd);

            return TRUE;
        }

        return FALSE;
    }

    BOOL SetItemAttribute(UINT uItemID, LPCSTR lpszAttrib, LPCSTR lpszValue)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
        {
            pT->_InvalidateControl(pWnd);

            pWnd->SetAttribute(lpszAttrib, lpszValue, FALSE);

            pT->_InvalidateControl(pWnd);

            return TRUE;
        }

        return FALSE;
    }

    BOOL SetItemStringAttribute(UINT uItemID, LPCSTR lpszAttrib, LPCTSTR lpszValue)
    {
        return SetItemAttribute(uItemID, lpszAttrib, CT2A(lpszValue, CP_UTF8));
    }

    BOOL SetItemIntAttribute(UINT uItemID, LPCSTR lpszAttrib, int nValue)
    {
        CStringA strValue;
        strValue.Format("%d", nValue);

        return SetItemAttribute(uItemID, lpszAttrib, strValue);
    }

    BOOL SetItemDWordAttribute(UINT uItemID, LPCSTR lpszAttrib, DWORD dwValue)
    {
        CStringA strValue;
        strValue.Format("%u", dwValue);

        return SetItemAttribute(uItemID, lpszAttrib, strValue);
    }

    BOOL SetItemColorAttribute(UINT uItemID, LPCSTR lpszAttrib, COLORREF crValue)
    {
        CStringA strValue;
        strValue.Format("%02X%02X%02X", GetRValue(crValue), GetGValue(crValue), GetBValue(crValue));

        return SetItemAttribute(uItemID, lpszAttrib, strValue);
    }

    BOOL GetItemCheck(UINT uItemID)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
            return pWnd->IsChecked();

        return FALSE;
    }

    BOOL SetItemCheck(UINT uItemID, BOOL bCheck)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
        {
            if (bCheck)
                pWnd->ModifyState(KuiWndState_Check, 0);
            else
                pWnd->ModifyState(0, KuiWndState_Check);

            pT->_InvalidateControl(pWnd);

            return TRUE;
        }

        return FALSE;
    }

    BOOL EnableItem(UINT uItemID, BOOL bEnable)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
        {
            if (bEnable)
                pWnd->ModifyState(0, KuiWndState_Disable);
            else
                pWnd->ModifyState(KuiWndState_Disable, KuiWndState_Hover);

            pT->_InvalidateControl(pWnd);

            return TRUE;
        }

        return FALSE;
    }

    BOOL IsItemEnable(UINT uItemID, BOOL bCheckParent = FALSE)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
            return !pWnd->IsDisabled(bCheckParent);

        return FALSE;
    }

    BOOL SetItemVisible(UINT uItemID, BOOL bVisible)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
        {
            pWnd->KuiSendMessage(WM_SHOWWINDOW, (WPARAM)bVisible);

            pT->_InvalidateControl(pWnd, FALSE);

            return TRUE;
        }

        return FALSE;
    }

    BOOL IsItemVisible(UINT uItemID, BOOL bCheckParent = FALSE)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);

        if (pWnd)
            return pWnd->IsVisible(bCheckParent);

        return FALSE;
    }

    BOOL GetItemRect(UINT uItemID, RECT &rcItem)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);
        if (!pWnd)
            return FALSE;

        pWnd->GetRect(&rcItem);

        return TRUE;
    }

    BOOL SetTabCurSel(UINT uItemID, int nPage)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);
        if (!pWnd)
            return FALSE;

        if (!pWnd->IsClass(CKuiTabCtrl::GetClassName()))
            return FALSE;

        BOOL bRet = ((CKuiTabCtrl *)pWnd)->SetCurSel(nPage);

        return bRet;
    }

	BOOL DeleteTab(UINT uItemID, int nPage)
	{
		T* pT = static_cast<T*>(this);
		CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);
		if (!pWnd)
			return FALSE;

		if (!pWnd->IsClass(CKuiTabCtrl::GetClassName()))
			return FALSE;

		BOOL bRet = ((CKuiTabCtrl *)pWnd)->DeleteTabItemFormList(nPage);

		return bRet;
	}

    BOOL SetTabTitle(UINT uItemID, int nPage, LPCTSTR lpszTitle)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);
        if (!pWnd)
            return FALSE;

        if (!pWnd->IsClass(CKuiTabCtrl::GetClassName()))
            return FALSE;

        BOOL bRet = ((CKuiTabCtrl *)pWnd)->SetTabTitle(nPage, lpszTitle);

        return bRet;
    }

    int GetTabCurSel(UINT uItemID)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);
        if (!pWnd)
            return -1;

        if (!pWnd->IsClass(CKuiTabCtrl::GetClassName()))
            return -1;

        return ((CKuiTabCtrl *)pWnd)->GetCurSel();
    }

    BOOL SetItemIconHandle(UINT uItemID, HICON hIcon)
    {
        T* pT = static_cast<T*>(this);
        CKuiWindow *pWnd = pT->FindChildByCmdID(uItemID);
        if (!pWnd)
            return FALSE;

        if (!pWnd->IsClass(CKuiIconWnd::GetClassName()))
            return FALSE;

        ((CKuiIconWnd *)pWnd)->AttachIcon(hIcon);

        pT->_InvalidateControl(pWnd);

        return TRUE;
    }

    BOOL FormatRichText(UINT uItemID, LPCTSTR lpszFormat, ...)
    {
        va_list args;
        CString strText;

        va_start(args, lpszFormat);

        strText.FormatV(lpszFormat, args);

        return SetRichText(uItemID, strText);
    }

    BOOL FormatItemText(UINT uItemID, LPCTSTR lpszFormat, ...)
    {
        va_list args;
        CString strText;

        va_start(args, lpszFormat);

        strText.FormatV(lpszFormat, args);

        return SetItemText(uItemID, strText);
    }
};

// NOTICE: WS_TABSTOP needed for accept focus
//         WS_CLIPSIBLINGS may cause IE Control Redraw Error
typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP, 0> CKuiDialogViewTraits;

//////////////////////////////////////////////////////////////////////////
// CKuiDialogViewImpl

template <class T, class TKuiWin = CKuiDialog, class TBase = ATL::CWindow, class TWinTraits = CKuiDialogViewTraits>
class ATL_NO_VTABLE CKuiDialogViewImpl
    : public ATL::CWindowImpl<T, TBase, TWinTraits>
    , public CKuiViewImpl<T>
{
    friend CKuiViewImpl;

public:
    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

public:
    CKuiDialogViewImpl()
        : m_hKuiWndHover(NULL)
        , m_hKuiWndPushDown(NULL)
        , m_bTrackFlag(FALSE)
        , m_bPushDownFlag(FALSE)
        , m_dwDlgStyle(0)
        , m_dwDlgExStyle(0)
        , m_bHasHeader(FALSE)
        , m_bHasBody(FALSE)
        , m_bHasFooter(FALSE)
        , m_bXmlLoaded(FALSE)
        , m_bCaptureSetted(FALSE)
        , m_bCanMaximize(FALSE)
        , m_bNeedRepaint(FALSE)
        , m_bNeedAllRepaint(TRUE)
    {
    }

    ~CKuiDialogViewImpl()
    {
        m_kuiHeader.KuiSendMessage(WM_DESTROY);
        m_kuiBody.KuiSendMessage(WM_DESTROY);
        m_kuiFooter.KuiSendMessage(WM_DESTROY);
    }

protected:

    typedef ATL::CWindowImpl<T, TBase, TWinTraits>  __baseClass;
    typedef CKuiDialogViewImpl<T, TBase, TWinTraits> __thisClass;

    TKuiWin m_kuiHeader;
    TKuiWin m_kuiBody;
    TKuiWin m_kuiFooter;

    // Double Cached
    CKuiImage m_imgMem;

    // Only one hover control
    HKUIWND m_hKuiWndHover;
    // Only one pushdown control
    HKUIWND m_hKuiWndPushDown;

    // Tracking flag
    BOOL m_bTrackFlag;

    // Pushdown flag
    BOOL m_bPushDownFlag;

    BOOL m_bCaptureSetted;

    CString m_strWindowCaption;
    SIZE m_sizeDefault;

    DWORD m_dwDlgStyle;
    DWORD m_dwDlgExStyle;

    BOOL m_bHasHeader;
    BOOL m_bHasBody;
    BOOL m_bHasFooter;

    BOOL m_bCanMaximize;

    BOOL m_bXmlLoaded;
    BOOL m_bNeedRepaint;
    BOOL m_bNeedAllRepaint;

    CToolTipCtrl m_wndToolTip;

    CRgn m_rgnInvalidate;

public:

    HWND Create(
        HWND hWndParent, _U_RECT rect = NULL, DWORD dwStyle = 0,
        DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
    {
        if (NULL != m_hWnd)
            return m_hWnd;

        HWND hWnd = __baseClass::Create(hWndParent, rect, m_strWindowCaption, dwStyle, dwExStyle, MenuOrID, lpCreateParam);

        if (hWnd)
        {
            m_kuiHeader.SetContainer(m_hWnd);
            m_kuiBody.SetContainer(m_hWnd);
            m_kuiFooter.SetContainer(m_hWnd);

            m_wndToolTip.Create(hWnd);

            CToolInfo ti(0, hWnd);
            m_wndToolTip.AddTool(ti);
            m_wndToolTip.Activate(TRUE);

            if (!m_rgnInvalidate.IsNull())
                m_rgnInvalidate.DeleteObject();
        }

        return hWnd;
    };

    HWND Create(HWND hWndParent, _U_MENUorID MenuOrID)
    {
        return Create(hWndParent, NULL, 0, 0, MenuOrID, NULL);
    };

    BOOL Load(UINT uResID)
    {
        return FALSE;
    }

    BOOL Load(const std::string& strResId)
    {
        std::string strXml;
        KAppRes::Instance().GetXmlData(strResId, strXml);
        return SetXml(strXml.c_str());
    }

    BOOL XmlLoaded()
    {
        return m_bXmlLoaded;
    }

    BOOL SetXml(LPCSTR lpszXml)
    {
        CStringA strValue;

        TiXmlDocument xmlDoc;

        m_bXmlLoaded    = FALSE;

        m_dwDlgStyle    = WS_POPUP | WS_SYSMENU | 0;
        m_dwDlgExStyle  = 0;

        m_kuiHeader.KuiSendMessage(WM_DESTROY);
        m_kuiBody.KuiSendMessage(WM_DESTROY);
        m_kuiFooter.KuiSendMessage(WM_DESTROY);

        m_bHasHeader    = FALSE;
        m_bHasBody      = FALSE;
        m_bHasFooter    = FALSE;

        { // Free stack
            xmlDoc.Parse(lpszXml, NULL, TIXML_ENCODING_UTF8);
        }

        if (xmlDoc.Error())
        {
            _Redraw();
            return FALSE;
        }

        TiXmlElement *pXmlRootElem = xmlDoc.RootElement();

        strValue = pXmlRootElem->Value();
        if (strValue != "layer")
        {
            _Redraw();
            return FALSE;
        }

        {
            m_strWindowCaption = CA2T(pXmlRootElem->Attribute("title"), CP_UTF8);
            m_sizeDefault.cx = 0;
            m_sizeDefault.cy = 0;
            pXmlRootElem->Attribute("width", (int *)&m_sizeDefault.cx);
            pXmlRootElem->Attribute("height", (int *)&m_sizeDefault.cy);

            BOOL bValue = FALSE;

            pXmlRootElem->Attribute("appwin", &bValue);
            if (bValue)
                m_dwDlgExStyle |= WS_EX_APPWINDOW;

            bValue = FALSE;

            pXmlRootElem->Attribute("resize", &bValue);

            if (bValue)
            {
                m_bCanMaximize = TRUE;
                m_dwDlgStyle |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
            }
            else
                m_bCanMaximize = FALSE;

            bValue = FALSE;

            pXmlRootElem->Attribute("noborder", &bValue);

            if (bValue)
            {
                m_dwDlgExStyle |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
            }
            else
            {
                m_dwDlgExStyle |= WS_EX_OVERLAPPEDWINDOW;
            }

            bValue = FALSE;

            pXmlRootElem->Attribute("child", &bValue);
            
            if (bValue)
            {
                m_dwDlgStyle |= WS_CHILD;
                m_dwDlgStyle &= ~DS_MODALFRAME;
                m_dwDlgStyle &= ~WS_POPUP;
                m_dwDlgStyle &= ~WS_CAPTION;
                m_dwDlgExStyle &= ~WS_EX_CLIENTEDGE;
            }
        }

        TiXmlElement *pTiElement = NULL;

        pTiElement = pXmlRootElem->FirstChildElement("header");
        m_bHasHeader = m_kuiHeader.Load(pTiElement);
        if (m_bHasHeader)
        {
            CStringA strPos;
            int nHeight = 0;
            pTiElement->Attribute("height", &nHeight);
            strPos.Format("0,0,-0,%d", nHeight);
            m_kuiHeader.SetAttribute("pos", strPos, TRUE);
        }

        pTiElement = pXmlRootElem->FirstChildElement("footer");
        m_bHasFooter = m_kuiFooter.Load(pTiElement);
        if (m_bHasFooter)
        {
            CStringA strPos;
            int nHeight = 0;
            pTiElement->Attribute("height", &nHeight);
            strPos.Format("0,-%d,-0,-0", nHeight);
            m_kuiFooter.SetAttribute("pos", strPos, TRUE);
        }

        m_bHasBody = m_kuiBody.Load(pXmlRootElem->FirstChildElement("body"));
        if (m_bHasBody)
        {
            m_kuiBody.SetAttribute("pos", "0,0,-0,-0", TRUE);
        }

        m_bXmlLoaded = TRUE;

        if (!m_hWnd)
            return TRUE;

        _Redraw();

        m_hKuiWndHover = NULL;
        m_hKuiWndPushDown = NULL;

        return TRUE;
    }

    LPCTSTR GetWindowCaption()
    {
        return m_strWindowCaption;
    }

    void SetWindowCaption(LPCTSTR lpszCaption)
    {
        m_strWindowCaption = lpszCaption;
    }

    SIZE GetDefaultSize()
    {
        return m_sizeDefault;
    }

    DWORD GetDlgStyle()
    {
        return m_dwDlgStyle;
    }

    DWORD GetDlgExStyle()
    {
        return m_dwDlgExStyle;
    }

    void ShowAllRealWindows(BOOL bShow)
    {
        if (m_bHasHeader)
            m_kuiHeader.ShowAllRealWindowChilds(bShow);
        if (m_bHasFooter)
            m_kuiFooter.ShowAllRealWindowChilds(bShow);
        if (m_bHasBody)
            m_kuiBody.ShowAllRealWindowChilds(bShow);
    }

    void RecomposeItems(BOOL bRedraw)
    {

    }

    void Redraw()
    {

    }

    CKuiWindow* FindChildByCmdID(UINT uCmdID)
    {
        CKuiWindow *pWnd = NULL;

        pWnd = m_kuiHeader.FindChildByCmdID(uCmdID);
        if (pWnd)
            return pWnd;

        pWnd = m_kuiBody.FindChildByCmdID(uCmdID);
        if (pWnd)
            return pWnd;

        pWnd = m_kuiFooter.FindChildByCmdID(uCmdID);

        return pWnd;
    }

    BOOL SetPanelXml(UINT uItemID, UINT uResID)
    {
        CStringA strXml;
        BOOL bRet = KuiResUtil::LoadKuiXmlResource(uResID, strXml);
        if (!bRet)
            return FALSE;

        return SetPanelXml(uItemID, strXml);
    }

    BOOL SetPanelXml(UINT uItemID, LPCSTR lpszXml)
    {
        TiXmlDocument xmlDoc;

        { // Free stack
            xmlDoc.Parse(lpszXml, NULL, TIXML_ENCODING_UTF8);
        }

        if (xmlDoc.Error())
            return FALSE;

        CKuiWindow *pWnd = FindChildByCmdID(uItemID);

        if (!pWnd)
            return FALSE;

        if (!pWnd->IsClass(CKuiDialog::GetClassName()) && !pWnd->IsClass(CKuiPanel::GetClassName()) && !pWnd->IsClass(CKuiTab::GetClassName()))
            return FALSE;

        TiXmlElement *pXmlRootElem = xmlDoc.RootElement();

        CKuiPanel *pWndPanel = (CKuiPanel *)pWnd;

        pWndPanel->LoadChilds(pXmlRootElem);
        pWndPanel->RepositionChilds();

        _InvalidateControl(pWnd);

        if (pWnd->IsVisible(TRUE))
            pWnd->KuiSendMessage(WM_SHOWWINDOW, TRUE);

        return TRUE;
    }

protected:

    void _Redraw()
    {
        m_bNeedAllRepaint = TRUE;
        m_bNeedRepaint = TRUE;
        if (!m_rgnInvalidate.IsNull())
            m_rgnInvalidate.DeleteObject();

        Invalidate(FALSE);
    }

    void _RepositionItems(BOOL bRedraw = TRUE)
    {
        CRect rcClient, rcHeader, rcFooter, rcBody;

        if (!m_hWnd)
            return;

        GetClientRect(rcClient);

        if (rcClient.Width() == 0)
            return;

        WINDOWPOS WndPos = { 0, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW };

        if (m_bHasHeader)
        {
            m_kuiHeader.KuiSendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&WndPos);
            m_kuiHeader.GetRect(rcHeader);
        }

        if (m_bHasFooter)
        {
            m_kuiFooter.KuiSendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&WndPos);
            m_kuiFooter.GetRect(rcFooter);

            WndPos.y = rcClient.bottom - rcFooter.Height();
            WndPos.cy = rcFooter.Height();
            m_kuiFooter.KuiSendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&WndPos);
        }

        if (m_bHasBody)
        {
            WndPos.y = rcHeader.bottom;
            WndPos.cy = rcClient.bottom - rcFooter.Height() - rcHeader.bottom;

            m_kuiBody.KuiSendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&WndPos);
        }

        _Redraw();
    }

    void _ModifyWndState(CKuiWindow *pWnd, DWORD dwStateAdd, DWORD dwStateRemove)
    {
        pWnd->ModifyState(dwStateAdd, dwStateRemove);
        if (pWnd->NeedRedrawWhenStateChange())
            _InvalidateControl(pWnd);
    }

    void _InvalidateControl(CKuiWindow *pWnd, BOOL bCheckVisible = TRUE)
    {
        if (!bCheckVisible || pWnd->IsVisible(TRUE))
        {
            CRect rcInvalidate;

            pWnd->GetRect(rcInvalidate);
            _InvalidateRect(rcInvalidate);
        }
    }

    void _InvalidateRect(CRect& rcInvalidate)
    {
        if (m_rgnInvalidate.IsNull())
        {
            m_rgnInvalidate.CreateRectRgnIndirect(rcInvalidate);
        }
        else
        {
            CRgn rgnInvalidate;

            rgnInvalidate.CreateRectRgnIndirect(rcInvalidate);

            m_rgnInvalidate.CombineRgn(rgnInvalidate, RGN_OR);
        }

        if (IsWindow())
            InvalidateRect(rcInvalidate, FALSE);

        m_bNeedRepaint = TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
    // Message handler

    void OnPrint(CDCHandle dc, UINT uFlags)
    {
        if (m_bNeedAllRepaint)
        {
            if (!m_rgnInvalidate.IsNull())
                m_rgnInvalidate.DeleteObject();

            m_bNeedAllRepaint = FALSE;
        }

        if (m_bNeedRepaint)
        {
            KuiWinManager::EnterPaintLock();

            CDC dcMem;
            CDCHandle dcMemHandle;
            HDC hDCDesktop = ::GetDC(NULL);
            dcMem.CreateCompatibleDC(hDCDesktop);
            ::ReleaseDC(NULL, hDCDesktop);
            HBITMAP hbmpOld = dcMem.SelectBitmap(m_imgMem);

            HFONT hftOld = dcMem.SelectFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));

            dcMem.SetBkMode(TRANSPARENT);

            if (!m_rgnInvalidate.IsNull())
                dcMem.SelectClipRgn(m_rgnInvalidate);

            dcMemHandle.Attach(dcMem);

            if (m_bHasHeader)
                m_kuiHeader.RedrawRegion(dcMemHandle, m_rgnInvalidate);
            if (m_bHasBody)
                m_kuiBody.RedrawRegion(dcMemHandle, m_rgnInvalidate);
            if (m_bHasFooter)
                m_kuiFooter.RedrawRegion(dcMemHandle, m_rgnInvalidate);

            dcMemHandle.Detach();

            if (!m_rgnInvalidate.IsNull())
                m_rgnInvalidate.DeleteObject();

            dcMem.SelectFont(hftOld);
            dcMem.SelectBitmap(hbmpOld);

            KuiWinManager::LeavePaintLock();

            m_bNeedRepaint = FALSE;
        }

        m_imgMem.Draw(dc, 0, 0);
    }

    void OnPaint(CDCHandle dc)
    {
        CPaintDC dcPaint(m_hWnd);

        OnPrint((HDC)dcPaint, 0);
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
        return TRUE;
    }

    void OnDestroy()
    {
        if (m_imgMem.M_HOBJECT)
            m_imgMem.DeleteObject();
    }

    void OnSize(UINT nType, CSize size)
    {
        if (m_imgMem.M_HOBJECT)
            m_imgMem.DeleteObject();

        CRect rcClient;

        GetClientRect(rcClient);

        if (0 == rcClient.Width())
            return;

        m_imgMem.CreateBitmap(rcClient.Width(), rcClient.Height(), RGB(0, 0, 0));

        _RepositionItems();
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        if (!m_bTrackFlag)
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = m_hWnd;
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = 0;
            m_bTrackFlag = _TrackMouseEvent(&tme);

            {
                NMHDR nms;
                nms.code = KUINM_MOUSEHOVER;
                nms.hwndFrom = m_hWnd;
                nms.idFrom = GetDlgCtrlID();
                ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.idFrom, (WPARAM)&nms);
            }
        }

        HKUIWND hKuiWndHitTest = NULL;

        hKuiWndHitTest = m_kuiHeader.KuiGetHWNDFromPoint(point, TRUE);
        if (NULL == hKuiWndHitTest)
            hKuiWndHitTest = m_kuiBody.KuiGetHWNDFromPoint(point, TRUE);
        if (NULL == hKuiWndHitTest)
            hKuiWndHitTest = m_kuiFooter.KuiGetHWNDFromPoint(point, TRUE);

        CKuiWindow* pWndHover = KuiWnds::GetWindow(hKuiWndHitTest);

        if (pWndHover)
        {
            BOOL bDisabled = pWndHover->IsDisabled(TRUE);
            if (!bDisabled)
                pWndHover->KuiSendMessage(WM_MOUSEMOVE, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));

            if (hKuiWndHitTest != m_hKuiWndHover)
            {
                CRect rcInvalidate;

                if (NULL != m_hKuiWndHover)
                {
                    CKuiWindow* pWndHoverOld = KuiWnds::GetWindow(m_hKuiWndHover);

                    if (pWndHoverOld && !pWndHoverOld->IsDisabled(TRUE))
                    {
                        _ModifyWndState(pWndHoverOld, 0, KuiWndState_Hover);
                        pWndHoverOld->KuiSendMessage(WM_MOUSELEAVE);
                    }
                }

                if (!bDisabled)
                    _ModifyWndState(pWndHover, KuiWndState_Hover, 0);

                m_hKuiWndHover = hKuiWndHitTest;

                if (pWndHover)
                    m_wndToolTip.UpdateTipText(pWndHover->GetToolTipText(), m_hWnd);
            }
        }
    }

    void OnMouseLeave()
    {
        m_bTrackFlag = FALSE;

        if (m_hKuiWndHover)
        {
            CKuiWindow* pWndHover = KuiWnds::GetWindow(m_hKuiWndHover);
            if (pWndHover)
            {
                _ModifyWndState(pWndHover, 0, KuiWndState_Hover);
                pWndHover->KuiSendMessage(WM_MOUSELEAVE);
            }

            m_hKuiWndHover = NULL;
        }

        if (m_hKuiWndPushDown)
        {
            CKuiWindow* pWndPushdown = KuiWnds::GetWindow(m_hKuiWndPushDown);
            if (pWndPushdown)
                _ModifyWndState(pWndPushdown, 0, KuiWndState_PushDown);

            m_hKuiWndPushDown = NULL;
        }

        {
            NMHDR nms;
            nms.code = KUINM_MOUSELEAVE;
            nms.hwndFrom = m_hWnd;
            nms.idFrom = GetDlgCtrlID();
            ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.idFrom, (WPARAM)&nms);
        }
    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        HKUIWND hKuiWndHitTest = m_kuiHeader.KuiGetHWNDFromPoint(point, TRUE);

        if (hKuiWndHitTest)
        {
            CKuiWindow* pWndPushDown = KuiWnds::GetWindow(hKuiWndHitTest);

            if (!(pWndPushDown->IsClass("button") || pWndPushDown->IsClass("imgbtn") || pWndPushDown->IsClass("link")))
            {
                CWindow &wndParent = GetParent();

                if (0 == (wndParent.GetStyle() & (WS_MAXIMIZE | WS_MINIMIZE)))
                    wndParent.SendMessage(WM_SYSCOMMAND, SC_MOVE | HTCAPTION);

                return;
            }
        }

        if (NULL == hKuiWndHitTest)
            hKuiWndHitTest = m_kuiBody.KuiGetHWNDFromPoint(point, TRUE);
        if (NULL == hKuiWndHitTest)
            hKuiWndHitTest = m_kuiFooter.KuiGetHWNDFromPoint(point, TRUE);

        CKuiWindow* pWndPushDown = KuiWnds::GetWindow(hKuiWndHitTest);

        if (pWndPushDown)
        {
            if (pWndPushDown->IsDisabled(TRUE))
                return;

            if (!m_bCaptureSetted)
            {
                SetCapture();
                m_bCaptureSetted = TRUE;

                if (hKuiWndHitTest != m_hKuiWndHover)
                {
                    // Hover和Pushdown不同的原因是：鼠标一直没动，界面刷新，切换了鼠标所在位置的控件
                    //ATLASSERT(FALSE);
                    m_hKuiWndHover = hKuiWndHitTest;
                }

                m_hKuiWndPushDown = hKuiWndHitTest;

                _ModifyWndState(pWndPushDown, KuiWndState_PushDown, 0);
                pWndPushDown->KuiSendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
            }
        }
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        BOOL bNotifyClick = FALSE;
        UINT uCmdID = 0;
        CKuiWindow* pWndClick = NULL;

        if (m_bCaptureSetted)
        {
            ReleaseCapture();
            m_bCaptureSetted = FALSE;
        }

        if (m_hKuiWndPushDown)
        {
            pWndClick = KuiWnds::GetWindow(m_hKuiWndPushDown);

            if (pWndClick)
            {
                _ModifyWndState(pWndClick, 0, KuiWndState_PushDown);

                if (m_hKuiWndPushDown == m_hKuiWndHover)
                {
                    pWndClick->KuiSendMessage(WM_LBUTTONUP, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));

                    LPCTSTR lpszUrl = pWndClick->GetLinkUrl();
                    if (lpszUrl && lpszUrl[0])
                    {
                        HINSTANCE hRet = ::ShellExecute(NULL, L"open", lpszUrl, NULL, NULL, SW_SHOWNORMAL);
                    }
                    else if (pWndClick->GetCmdID())
                    {
                        bNotifyClick = TRUE;
                        uCmdID = pWndClick->GetCmdID();
                    }
                }
            }

            m_hKuiWndPushDown = NULL;
        }

        if (bNotifyClick)
        {
            KUINMCOMMAND nms;
            nms.hdr.code = KUINM_COMMAND;
            nms.hdr.hwndFrom = m_hWnd;
            nms.hdr.idFrom = GetDlgCtrlID();
            nms.uItemID = uCmdID;
            nms.szItemClass = pWndClick->GetObjectClass();

            LRESULT lRet = ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
        }

    }

    void OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        HKUIWND hKuiWndHitTest = m_kuiHeader.KuiGetHWNDFromPoint(point, TRUE);

        if (hKuiWndHitTest)
        {
            CKuiWindow* pWndPushDown = KuiWnds::GetWindow(hKuiWndHitTest);

            if (!(pWndPushDown->IsClass("button") || pWndPushDown->IsClass("imgbtn") || pWndPushDown->IsClass("link")))
            {
                if (m_bCanMaximize)
                {
                    CWindow &wndParent = GetParent();
                    DWORD dwStyle = wndParent.GetStyle();

                    if (WS_MAXIMIZE == (dwStyle & WS_MAXIMIZE))
                        wndParent.SendMessage(WM_SYSCOMMAND, SC_RESTORE | HTCAPTION);
                    else
                        wndParent.SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION);
                }

                return;
            }
        }
    }

    BOOL OnSetCursor(CWindow /*wnd*/, UINT nHitTest, UINT message)
    {
        if (m_hKuiWndHover)
        {
            CKuiWindow *pKuiWndHover = KuiWnds::GetWindow(m_hKuiWndHover);

            if (pKuiWndHover && !pKuiWndHover->IsDisabled(TRUE))
            {
                pKuiWndHover->SetCursor();
                return TRUE;
            }
        }

        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

        return TRUE;
    }

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        return FALSE;
    }

    LRESULT OnChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::SendMessage(::GetParent(m_hWnd), uMsg, wParam, lParam);
    }

    LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if(m_wndToolTip.IsWindow())
        {
            MSG msg = { m_hWnd, uMsg, wParam, lParam };

            m_wndToolTip.RelayEvent(&msg);
        }

        SetMsgHandled(FALSE);

        return 0;
    }

    LRESULT OnKUIINMInvalidateRect(LPNMHDR pnmh)
    {
        LPKUIINMINVALIDATERECT pnms = (LPKUIINMINVALIDATERECT)pnmh;
        CRect rcWnd = pnms->rect;

        _InvalidateRect(rcWnd);

        return 0;
    }

protected:
    BEGIN_MSG_MAP_EX(CKuiDialogViewImpl)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PRINT(OnPrint)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_SETCURSOR(OnSetCursor)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        NOTIFY_CODE_HANDLER_EX(KUIINM_INVALIDATERECT, OnKUIINMInvalidateRect)
        REFLECT_NOTIFY_CODE(NM_CUSTOMDRAW)
        MESSAGE_HANDLER_EX(WM_NOTIFY, OnChildNotify)
        MESSAGE_HANDLER_EX(WM_COMMAND, OnChildNotify)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// CKuiDialogView

class CKuiDialogView
    : public CKuiDialogViewImpl<CKuiDialogView>
{
};

//////////////////////////////////////////////////////////////////////////
// Default view control id in CKuiDialogImpl

#define IDC_RICHVIEW_WIN            1000

//////////////////////////////////////////////////////////////////////////
// CKuiDialogImpl

template <class T, class TKuiView = CKuiDialogView, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CKuiDialogImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:

    static BOOL IsWinXPAndLater()
    {
        DWORD dwVersion = GetVersion();
        DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
        DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

        if (dwMajorVersion > 5)         // Vista and later
            return TRUE;
        else if (dwMajorVersion < 5)    // 98/NT4 and before
            return FALSE;
        else if (dwMinorVersion > 0)    // XP and 2003
            return TRUE;
        else                            // 2000
            return FALSE;
    }

    static ATL::CWndClassInfo& GetWndClassInfo()
    {
        static ATL::CWndClassInfo wc = {
                { sizeof(WNDCLASSEX),
                CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | (IsWinXPAndLater() ? CS_DROPSHADOW : 0),
                  StartWindowProc, 0, 0, NULL, NULL, NULL,
                  (HBRUSH)(COLOR_WINDOW + 1), NULL, NULL, NULL },
                NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
            };
        return wc;
    }

public:
	CKuiDialogImpl(const std::string& strResID)
        : m_strResID(strResID)
        , m_bShowWindow(TRUE)
        , m_bExitModalLoop(FALSE)
    {
    }
    virtual ~CKuiDialogImpl()
    {

    }

	HWND GetRichWnd()
	{
		return m_richView;
	}

	void SetResId(const std::string& strResId)
	{
		m_strResID = strResId;
	}

protected:
    typedef CKuiDialogImpl<T, TKuiView, TBase, TWinTraits> __thisClass;

	std::string m_strResID;
    UINT m_uRetCode;

    TKuiView m_richView;

    BOOL m_bShowWindow;

    void ResizeClient(SIZE sizeClient, BOOL bRedraw)
    {
        ResizeClient(sizeClient.cx, sizeClient.cy, bRedraw);
    }

    void ResizeClient(int cx, int cy, BOOL bRedraw)
    {
        CRect rcWindow, rcClient;
        CPoint ptWindow;

        GetWindowRect(rcWindow);
        GetClientRect(rcClient);

        ptWindow = rcWindow.TopLeft();

        rcWindow.MoveToXY(0, 0);

        rcWindow -= rcClient;
        rcClient.SetRect(0, 0, cx, cy);
        rcWindow += rcClient;

        rcWindow.MoveToXY(ptWindow);

        MoveWindow(rcWindow, bRedraw);
    }

    void OnSize(UINT nType, CSize size)
    {
        CRect rcClient;

        GetClientRect(rcClient);
        if (SIZE_MINIMIZED != nType)
        {
            if (m_richView.m_hWnd)
                m_richView.MoveWindow(rcClient);
        }

        SetMsgHandled(FALSE);
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
        return TRUE;
    }

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        if (bCalcValidRects)
        {
            CRect rcWindow;

            GetWindowRect(rcWindow);

            LPNCCALCSIZE_PARAMS pParam = (LPNCCALCSIZE_PARAMS)lParam;

            if (SWP_NOSIZE == (SWP_NOSIZE & pParam->lppos->flags))
                return 0;

            if (0 == (SWP_NOMOVE & pParam->lppos->flags))
            {
                rcWindow.left = pParam->lppos->x;
                rcWindow.top = pParam->lppos->y;
            }

            rcWindow.right = rcWindow.left + pParam->lppos->cx;
            rcWindow.bottom = rcWindow.top + pParam->lppos->cy;
            pParam->rgrc[0] = rcWindow;
            pParam->rgrc[1] = pParam->rgrc[0];
        }

        return 0;
    }

    void OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
    {
        HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONULL);

        if (hMonitor)
        {
            MONITORINFO mi = {sizeof(MONITORINFO)};
            ::GetMonitorInfo(hMonitor, &mi);

            CRect rcWork = mi.rcWork, rcMonitor = mi.rcMonitor;
            lpMMI->ptMaxPosition.x = abs(rcWork.left - rcMonitor.left) - 1;
            lpMMI->ptMaxPosition.y = abs(rcWork.top - rcMonitor.top) - 1;
            lpMMI->ptMaxSize.x = abs(rcWork.Width()) + 2;
            lpMMI->ptMaxSize.y = abs(rcWork.Height()) + 2;
            lpMMI->ptMaxTrackSize.x = abs(rcWork.Width()) + 2;
            lpMMI->ptMaxTrackSize.y = abs(rcWork.Height()) + 2;
        }
    }

    BOOL OnNcActivate(BOOL bActive)
    {
        return TRUE;
    }

public:

	BOOL Load(const std::string& strResID)
    {
		KAppRes& appRes = KAppRes::Instance();
		std::string strXml;

        if (!appRes.GetXmlData(strResID, strXml))
            return FALSE;

        return SetXml(strXml);
    }

	BOOL SetXml(const std::string& strXml)
	{
		return SetXml(strXml.c_str());
	}

    BOOL SetXml(LPCSTR lpszXml)
    {
        return m_richView.SetXml(lpszXml);
    }

    HWND GetViewHWND()
    {
        return m_richView.m_hWnd;
    }

    BOOL SetPanelXml(UINT uItemID, UINT uResID)
    {
        return m_richView.SetPanelXml(uItemID, uResID);
    }

    BOOL SetPanelXml(UINT uItemID, LPCSTR lpszXml)
    {
        return m_richView.SetPanelXml(uItemID, lpszXml);
    }

    BOOL SetRichText(UINT uItemID, UINT uResID)
    {
        return m_richView.SetRichText(uItemID, uResID);
    }

    BOOL FormatRichText(UINT uItemID, LPCTSTR lpszFormat, ...)
    {
        va_list args;
        CString strText;

        va_start(args, lpszFormat);

        strText.FormatV(lpszFormat, args);

        return m_richView.SetRichText(uItemID, strText);
    }

    BOOL SetRichText(UINT uItemID, LPCWSTR lpszXml)
    {
        return m_richView.SetRichText(uItemID, lpszXml);
    }

    BOOL SetRichText(UINT uItemID, LPCSTR lpszXml)
    {
        return m_richView.SetRichText(uItemID, lpszXml);
    }

    BOOL SetItemText(UINT uItemID, LPCTSTR lpszText)
    {
        return m_richView.SetItemText(uItemID, lpszText);
    }

    BOOL FormatItemText(UINT uItemID, LPCTSTR lpszFormat, ...)
    {
        va_list args;
        CString strText;

        va_start(args, lpszFormat);

        strText.FormatV(lpszFormat, args);

        return m_richView.SetItemText(uItemID, strText);
    }

    BOOL SetItemAttribute(UINT uItemID, LPCSTR lpszAttrib, LPCSTR lpszValue)
    {
        return m_richView.SetItemAttribute(uItemID, lpszAttrib, lpszValue);
    }

    BOOL GetItemRect(UINT uItemID, RECT &rcItem)
    {
        return m_richView.GetItemRect(uItemID, rcItem);
    }

    BOOL SetItemStringAttribute(UINT uItemID, LPCSTR lpszAttrib, LPCTSTR lpszValue)
    {
        return m_richView.SetItemStringAttribute(uItemID, lpszAttrib, lpszValue);
    }

    BOOL SetItemIntAttribute(UINT uItemID, LPCSTR lpszAttrib, int nValue)
    {
        return m_richView.SetItemIntAttribute(uItemID, lpszAttrib, nValue);
    }

    BOOL SetItemDWordAttribute(UINT uItemID, LPCSTR lpszAttrib, DWORD dwValue)
    {
        return m_richView.SetItemDWordAttribute(uItemID, lpszAttrib, dwValue);
    }

    BOOL SetItemColorAttribute(UINT uItemID, LPCSTR lpszAttrib, COLORREF crValue)
    {
        return m_richView.SetItemColorAttribute(uItemID, lpszAttrib, crValue);
    }

    BOOL GetItemCheck(UINT uItemID)
    {
        return m_richView.GetItemCheck(uItemID);
    }

    BOOL SetItemCheck(UINT uItemID, BOOL bCheck)
    {
        return m_richView.SetItemCheck(uItemID, bCheck);
    }

    BOOL IsItemEnable(UINT uItemID, BOOL bCheckParent = FALSE)
    {
        return m_richView.IsItemEnable(uItemID, bCheckParent);
    }

    BOOL EnableItem(UINT uItemID, BOOL bEnable)
    {
        return m_richView.EnableItem(uItemID, bEnable);
    }

    BOOL IsItemVisible(UINT uItemID, BOOL bCheckParent = FALSE)
    {
        return m_richView.IsItemVisible(uItemID, bCheckParent);
    }

    BOOL SetItemVisible(UINT uItemID, BOOL bVisible)
    {
        return m_richView.SetItemVisible(uItemID, bVisible);
    }

    BOOL SetTabCurSel(UINT uItemID, int nPage)
    {
        return m_richView.SetTabCurSel(uItemID, nPage);
    }
	BOOL DeleteTab(UINT uItemID, int nPage)
	{
		return m_richView.DeleteTab(uItemID, nPage);
	}

    int GetTabCurSel(UINT uItemID)
    {
        return m_richView.GetTabCurSel(uItemID);
    }

    BOOL SetTabTitle(UINT uItemID, int nPage, LPCTSTR lpszTitle)
    {
        return m_richView.SetTabTitle(uItemID, nPage, lpszTitle);
    }

    BOOL SetItemIconHandle(UINT uItemID, HICON hIcon)
    {
        return m_richView.SetItemIconHandle(uItemID, hIcon);
    }

    void SetWindowCaption(LPCTSTR lpszCaption)
    {
        m_richView.SetWindowCaption(lpszCaption);
    }

    // Call at WM_INITDIALOG
    void DontShowWindow()
    {
        m_bShowWindow = FALSE;
    }

    HWND Create(HWND hWndParent = ::GetActiveWindow(), LPRECT rect = NULL)
    {
        if (!m_richView.XmlLoaded())
        {
            if (0 == m_strResID.length() || !Load(m_strResID))
            {
                return NULL;
            }
        }

        CRect rcWnd(0, 0, 0, 0);
        LPCTSTR lpszCaption = m_richView.GetWindowCaption();

        if (_T('\0') == lpszCaption[0])
            lpszCaption = NULL;

        if (rect)
            rcWnd = rect;

        HWND hWnd = __super::Create(hWndParent, rcWnd, lpszCaption, m_richView.GetDlgStyle(), m_richView.GetDlgExStyle());
        if (!hWnd)
            return NULL;

        KuiWinThemeFunc::SetWindowTheme(hWnd, L"", L"");

        if (rect)
        {
            rcWnd.MoveToXY(0, 0);
            m_richView.Create(m_hWnd, rcWnd, 0, 0, IDC_RICHVIEW_WIN);
        }
        else
            m_richView.Create(m_hWnd, NULL, 0, 0, IDC_RICHVIEW_WIN);

        m_bShowWindow = TRUE;

        SendMessage(WM_INITDIALOG, (WPARAM)hWnd);

        m_richView.ShowAllRealWindows(TRUE);

        if (m_hWnd == hWnd)
        {
            SIZE sizeDefault = m_richView.GetDefaultSize();
            if (sizeDefault.cx && sizeDefault.cy)
            {
                ResizeClient(sizeDefault, FALSE);
            }
        }
        else
            hWnd = m_hWnd;

        return hWnd;
    }

    UINT_PTR DoModal(HWND hWndParent = NULL, LPRECT rect = NULL)
    {
        BOOL bEnableParent = FALSE;

        if (NULL == hWndParent)
        {
            hWndParent = KuiWinManager::GetActive();
            if (NULL == hWndParent)
                hWndParent = ::GetActiveWindow();
        }

        if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
        {
            ::EnableWindow(hWndParent, FALSE);
            bEnableParent = TRUE;
        }

        m_bExitModalLoop = FALSE;

        HWND hWnd = Create(hWndParent, rect);
        if (!hWnd)
        {
            ::EnableWindow(hWndParent, TRUE);
            return 0;
        }

        HWND hWndLastActive = KuiWinManager::SetActive(hWnd);

        if (!rect)
            CenterWindow();

        if (m_bShowWindow)
        {

            ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
        }

        _ModalMessageLoop();

        m_bExitModalLoop = FALSE;

        // From MFC
        // hide the window before enabling the parent, etc.
        SetWindowPos(
            NULL, 0, 0, 0, 0,
            SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

        if (bEnableParent)
        {
            ::EnableWindow(hWndParent, TRUE);
        }

        if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
            ::SetActiveWindow(hWndParent);

        KuiWinManager::SetActive(hWndLastActive);

        DestroyWindow();

        return m_uRetCode;
    }

    void OnClose()
    {
        SendMessage(WM_COMMAND, MAKELONG(IDCANCEL, 0), NULL);
    }

    void EndDialog(UINT uRetCode)
    {
        m_uRetCode = uRetCode;

        m_bExitModalLoop = TRUE;

        // DestroyWindow里面直接Send了WM_DESTROY，所以不会跑到DoModal的消息循环里，所以有了下面那行代码
        // DestroyWindow();

        // 这句非常重要，可以让DoModal消息循环再跑一次，防止卡死在GetMessage，泪奔~~~~~~~
        ::PostThreadMessage(::GetCurrentThreadId(), WM_NULL, 0, 0);
    }

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (IsWindowEnabled())
        {
            if (::GetKeyState(VK_CONTROL) >= 0)
            {
                if (VK_ESCAPE == nChar)
                {
                    SendMessage(WM_COMMAND, MAKELONG(IDCANCEL, 0), NULL);
                }
                else if (VK_RETURN == nChar)
                {
                    SendMessage(WM_COMMAND, MAKELONG(IDOK, 0), NULL);
                }
            }

            SetMsgHandled(FALSE);
        }
    }

    void OnOK(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        KUINMCOMMAND nms;
        nms.hdr.code = KUINM_COMMAND;
        nms.hdr.hwndFrom = m_hWnd;
        nms.hdr.idFrom = IDC_RICHVIEW_WIN;
        nms.uItemID = IDOK;
        nms.szItemClass = "";

        LRESULT lRet = ::SendMessage(m_hWnd, WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
    }

    void OnCancel(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
    {
        KUINMCOMMAND nms;
        nms.hdr.code = KUINM_COMMAND;
        nms.hdr.hwndFrom = m_hWnd;
        nms.hdr.idFrom = IDC_RICHVIEW_WIN;
        nms.uItemID = IDCANCEL;
        nms.szItemClass = "";

        LRESULT lRet = ::SendMessage(m_hWnd, WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
    }

protected:

    BOOL m_bExitModalLoop;

    void _ModalMessageLoop()
    {
        BOOL bRet;
        MSG msg;

        for(;;)
        {
            if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
            {
                if (WM_QUIT == msg.message)
                    break;
            }

            if (m_bExitModalLoop || NULL == m_hWnd || !::IsWindow(m_hWnd))
                break;

            bRet = ::GetMessage(&msg, NULL, 0, 0);

            if (bRet == -1)
            {
                continue;   // error, don't process
            }
            else if (!bRet)
            {
                ATLTRACE(L"Why Receive WM_QUIT here?\r\n");
                break;   // WM_QUIT, exit message loop
            }

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

	BEGIN_MSG_MAP_EX(CKuiDialogImpl)
        MSG_WM_NCACTIVATE(OnNcActivate)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
        //MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
        MSG_WM_SIZE(OnSize)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CLOSE(OnClose)
        COMMAND_ID_HANDLER_EX(IDOK, OnOK)
        COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// CKuiSimpleDialog

class CKuiSimpleDialog
    : public CKuiDialogImpl<CKuiSimpleDialog>
{
public:
	CKuiSimpleDialog(const std::string& strResID)
        : CKuiDialogImpl<CKuiSimpleDialog>(strResID)
    {
    }
protected:

    void OnKuiCommand(UINT uItemID, LPCSTR szItemClass)
    {
        if (strcmp(CKuiButton::GetClassName(), szItemClass) != 0 && strcmp(CKuiImageBtnWnd::GetClassName(), szItemClass) != 0)
            return;

        EndDialog(uItemID);
    }

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_COMMAND(OnKuiCommand)
    KUI_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CKuiDialogImpl<CKuiSimpleDialog>)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CKuiDialogImpl<CKuiSimpleDialog>)
	END_MSG_MAP()
};

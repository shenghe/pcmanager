//////////////////////////////////////////////////////////////////////////
//   File Name: KuiWnd.h
// Description: KuiWindow Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kuiwndstyle.h"
#include "kscbase/kscconv.h"

//////////////////////////////////////////////////////////////////////////

// Not Used
#define KUIWIN_DECLARE_RETURN_SETTING(before, after)     \
public:                                                 \
    virtual BOOL ReturnBeforeThisControl(void)          \
    {                                                   \
        return before;                                  \
    }                                                   \
    virtual BOOL ReturnAfterThisControl(void)           \
    {                                                   \
        return after;                                   \
    }                                                   \

//////////////////////////////////////////////////////////////////////////

// KuiWindow Message Map Define
// Use WTL Message Map Ex (include atlcrack.h)
#define KUIWIN_BEGIN_MSG_MAP()                                       \
protected:                                                          \
	virtual BOOL ProcessWindowMessage(                              \
        HWND hWnd, UINT uMsg, WPARAM wParam,                        \
        LPARAM lParam, LRESULT& lResult)                            \
	{                                                               \

#define KUIWIN_END_MSG_MAP()                                         \
        if (!IsMsgHandled())                                        \
            return __super::ProcessWindowMessage(                   \
                hWnd, uMsg, wParam, lParam, lResult);               \
        return TRUE;                                                \
	}                                                               \

#define KUIWIN_END_MSG_MAP_BASE()                                    \
        return TRUE;                                                \
	}                                                               \


//////////////////////////////////////////////////////////////////////////
// CKuiWindow Handle Manager
//////////////////////////////////////////////////////////////////////////

class CKuiWindow;

// KuiWindow Handle
typedef DWORD HKUIWND;

class KuiWnds
{
public:

    KuiWnds()
        : m_hNextWnd(NULL)
    {
		::InitializeCriticalSection(&ms_lockWndMap);
    }

    ~KuiWnds()
    {
		::DeleteCriticalSection(&ms_lockWndMap);
    }

    // Get KuiWindow pointer from handle
    static CKuiWindow* GetWindow(HKUIWND hKuiWnd)
    {
		::EnterCriticalSection(&ms_lockWndMap);

        const _KuiWndHandlePool::CPair *pairRet = _Instance()->m_mapPool.Lookup(hKuiWnd);

		::LeaveCriticalSection(&ms_lockWndMap);

        if (pairRet)
        {
            return pairRet->m_value;
        }
        else
            return NULL;
    }

    // Get KuiWindow pointer from command id
    static CKuiWindow* GetWindow(UINT uCmdID, HWND hWndInContainer = NULL);

    // Specify a handle to a KuiWindow
    static HKUIWND NewWindow(CKuiWindow *pKuiWnd)
    {
        //static int s_n_new = 0;
        //ATLTRACE(_T("NewWindow %d\r\n"), ++ s_n_new);

        if (pKuiWnd)
        {
			_Instance();

			::EnterCriticalSection(&ms_lockWndMap);

            HKUIWND hKuiWndNext = ++ _Instance()->m_hNextWnd;
            _Instance()->m_mapPool[hKuiWndNext] = pKuiWnd;

			::LeaveCriticalSection(&ms_lockWndMap);

			return hKuiWndNext;
        }
        else
            return NULL;
    }

    // Destroy KuiWindow
    static BOOL DestroyWindow(HKUIWND hKuiWnd)
    {
        //static int s_n_del = 0;
        //ATLTRACE(_T("DestroyWindow %d\r\n"), ++ s_n_del);

		::EnterCriticalSection(&ms_lockWndMap);

		BOOL bRet = (BOOL)_Instance()->m_mapPool.RemoveKey(hKuiWnd);

		::LeaveCriticalSection(&ms_lockWndMap);

        return bRet;
    }

    static size_t GetCount()
    {
		::EnterCriticalSection(&ms_lockWndMap);

		size_t nRet = _Instance()->m_mapPool.GetCount();

		::LeaveCriticalSection(&ms_lockWndMap);

        return nRet;
    }

    static BOOL RegisterRadioGroup(CKuiWindow *pkuiWnd, LPCSTR lpszGroupName)
    {
		::EnterCriticalSection(&ms_lockWndMap);

        POSITION pos = _Instance()->m_mapRadioPool[lpszGroupName].AddTail(pkuiWnd);

		::LeaveCriticalSection(&ms_lockWndMap);

        return (NULL != pos);
    }

    static void UnregisterRadioGroup(CKuiWindow *pkuiWnd, LPCSTR lpszGroupName)
    {
		::EnterCriticalSection(&ms_lockWndMap);

        CAtlList<CKuiWindow *> &ListGroup = _Instance()->m_mapRadioPool[lpszGroupName];

        POSITION posCurrent = NULL, pos = ListGroup.GetHeadPosition();

        while (pos)
        {
            posCurrent = pos;

            if (ListGroup.GetNext(pos) != pkuiWnd)
                continue;

            ListGroup.RemoveAt(posCurrent);
        }

		::LeaveCriticalSection(&ms_lockWndMap);

    }

    static void SelectRadio(CKuiWindow* pkuiWnd, LPCSTR lpszGroupName, CKuiWindow **ppkuiWndLastSelect);
//     {
//         CAtlList<CKuiWindow *> &ListGroup = _Instance()->m_mapRadioPool[lpszGroupName];
//
//         pkuiWnd->ModifyState(KuiWndState_Check, 0);
//
//         POSITION posCurrent = NULL, pos = ListGroup.GetHeadPosition();
//
//         while (pos)
//         {
//             posCurrent = pos;
//             CKuiWindow *pkuiWndFind = ListGroup.GetNext(pos);
//
//             if (pkuiWndFind == pkuiWnd)
//                 continue;
//
//             if (pkuiWndFind->IsChecked())
//             {
//                 pkuiWndFind->ModifyState(0, KuiWndState_Check);
//                 if (ppkuiWndLastSelect)
//                     *ppkuiWndLastSelect = pkuiWndFind;
//             }
//         }
//     }

protected:

    static KuiWnds* ms_pInstance;
	static CRITICAL_SECTION ms_lockWndMap;

    static KuiWnds* _Instance()
    {
        if (!ms_pInstance)
            ms_pInstance = new KuiWnds;
        return ms_pInstance;
    }

//     static KuiWnds& _Instance()
//     {
//         static KuiWnds s_obj;
//         return s_obj;
//     }

    typedef CAtlMap<HKUIWND, CKuiWindow *> _KuiWndHandlePool;
    typedef CAtlMap<CStringA, CAtlList<CKuiWindow *>> _KuiWndGroupPool;

    _KuiWndHandlePool m_mapPool;
    _KuiWndGroupPool m_mapRadioPool;
    HKUIWND m_hNextWnd;
};

__declspec(selectany) KuiWnds* KuiWnds::ms_pInstance = NULL;
__declspec(selectany) CRITICAL_SECTION KuiWnds::ms_lockWndMap;

//////////////////////////////////////////////////////////////////////////
// Inner Notify (KuiWindows to Container)

#define KUIINM_FIRST             1000
#define KUIINM_LAST              1999

// Invalidate Rect Notify
#define KUIINM_INVALIDATERECT    KUIINM_FIRST

typedef struct _KUIINMINVALIDATERECT
{
    NMHDR       hdr;
    HKUIWND      hkuiWnd;
    RECT        rect;
} KUIINMINVALIDATERECT, *LPKUIINMINVALIDATERECT;

//////////////////////////////////////////////////////////////////////////
// CKuiWindow Implement
//////////////////////////////////////////////////////////////////////////

class CKuiWindow : public CKuiObject
{
public:
    CKuiWindow()
        : m_hKuiWnd(KuiWnds::NewWindow(this))
        , m_hKuiWndParent(NULL)
        , m_hWndContainer(NULL)
        , m_bMsgHandled(FALSE)
        , m_uCmdID(NULL)
        , m_uPositionType(0)
        , m_lSpecifyWidth(0)
        , m_lSpecifyHeight(0)
        , m_dwState(KuiWndState_Normal)
        , m_uVAlign(0)
        , m_uHAlign(0)
        , m_bTransparent(FALSE)
    {
    }
    virtual ~CKuiWindow()
    {
        KuiWnds::DestroyWindow(m_hKuiWnd);
    }

    enum {
        // Specify by "width" attribute
        SizeX_Mask          = 0x0007UL,
        SizeX_Specify       = 0x0001UL, // width > 0
        SizeX_FitContent    = 0x0002UL, // width <= 0
        SizeX_FitParent     = 0x0004UL, // width = "full" default

        // Specify by "height" attribute
        SizeY_Mask          = 0x0070UL,
        SizeY_Specify       = 0x0010UL, // height > 0
        SizeY_FitContent    = 0x0020UL, // height <= 0 default
        SizeY_FitParent     = 0x0040UL, // height = "full" default
        // Specify by "float" attribute

        Position_Mask       = 0x0300UL,
        Position_Relative   = 0x0100UL, // float = 0 default
        Position_Float      = 0x0200UL, // float = 1

        // Specify by "valign" and "align" attribute, only using at float = 1 or panel control (Vert-Align)
        Align_Mask          = 0xF000UL,
        VAlign_Top          = 0x0000UL, // valign = top
        VAlign_Middle       = 0x1000UL, // valign = middle
        VAlign_Bottom       = 0x2000UL, // valign = bottom
        Align_Left          = 0x0000UL, // align = left
        Align_Center        = 0x4000UL, // align = center
        Align_Right         = 0x8000UL, // align = right
    };

    struct KUIDLG_POSITION_ITEM
    {
        BOOL bCenter;
        BOOL bMinus;
        int  nPos;
    };

    struct KUIDLG_POSITION
    {
        int nCount;
        union
        {
            struct
            {
                KUIDLG_POSITION_ITEM Left;
                KUIDLG_POSITION_ITEM Top;
                KUIDLG_POSITION_ITEM Right;
                KUIDLG_POSITION_ITEM Bottom;
            };
            KUIDLG_POSITION_ITEM Item[4];
        };
    };

protected:
    HKUIWND m_hKuiWnd;
    HKUIWND m_hKuiWndParent;
    HWND m_hWndContainer;
    UINT m_uCmdID;
    CRect m_rcWindow;
    UINT m_uPositionType;
    KuiStyle m_style;
    CString m_strInnerText;
    LONG m_lSpecifyWidth;
    LONG m_lSpecifyHeight;
    DWORD m_dwState;
    UINT m_uVAlign;
    UINT m_uHAlign;
    CString m_strLinkUrl;
    BOOL m_bTransparent;
    CString m_strToolTipText;

    KUIDLG_POSITION m_dlgpos;

public:

    //////////////////////////////////////////////////////////////////////////
    // Method Define

    // Get align
    UINT GetAlign()
    {
        return m_uVAlign | m_uHAlign;
    }

    // Get position type
    DWORD GetPositionType()
    {
        return m_uPositionType;
    }

    // Set position type
    void SetPositionType(DWORD dwPosType, DWORD dwMask = 0xFFFFFFFF)
    {
        m_uPositionType = (m_uPositionType & ~dwMask) | (dwPosType & dwMask);
    }

    // Get KuiWindow rect(position in container)
    void GetRect(LPRECT prect)
    {
        if (prect)
        {
            prect->left     = m_rcWindow.left;
            prect->right    = m_rcWindow.right;
            prect->top      = m_rcWindow.top;
            prect->bottom   = m_rcWindow.bottom;
        }
    }

    void GetDlgPosition(KUIDLG_POSITION *pPos)
    {
        if (pPos)
            memcpy(pPos, &m_dlgpos, sizeof(KUIDLG_POSITION));
    }

    // Get inner text
    LPCTSTR GetInnerText()
    {
        return m_strInnerText;
    }

    // Get tooltip text
    LPCTSTR GetToolTipText()
    {
        return m_strToolTipText;
    }

    // Set inner text
    HRESULT SetInnerText(LPCTSTR lpszText)
    {
        m_strInnerText = lpszText;

        if ((m_uPositionType & SizeX_FitContent | SizeY_FitContent) || CLR_INVALID == m_style.m_crBg)
        {
            if ((m_uPositionType & (SizeX_FitContent | SizeY_FitContent)) && (4 != m_dlgpos.nCount))
            {
                SIZE sizeRet = {0, 0};

                KuiSendMessage(WM_NCCALCSIZE, FALSE, (LPARAM)&sizeRet);

                if (m_uPositionType & SizeX_FitContent)
                    m_rcWindow.right = m_rcWindow.left + sizeRet.cx;
                if (m_uPositionType & SizeY_FitContent)
                    m_rcWindow.bottom = m_rcWindow.top + sizeRet.cy;
            }

            return S_OK;
        }
        else
            return S_FALSE;
    }

    // Send a message to KuiWindow
    LRESULT KuiSendMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0)
    {
        LRESULT lResult = 0;

        SetMsgHandled(FALSE);
        ProcessWindowMessage(NULL, Msg, wParam, lParam, lResult);

        return lResult;
    }

    // Move KuiWindow to new place
    //
    // NOTICE: Dangerous
    void Move(LPRECT prect)
    {
        if (prect)
            m_rcWindow = prect;
    }

    // Set current cursor, when hover
    virtual void SetCursor()
    {
        HCURSOR hCur = ::LoadCursor(NULL, m_style.m_lpCursorName);
        ::SetCursor(hCur);
    }

    // Get KuiWindow state
    DWORD GetState(void)
    {
        return m_dwState;
    }

    // Modify KuiWindow state
    DWORD ModifyState(DWORD dwStateAdd, DWORD dwStateRemove)
    {
        DWORD dwOldState = m_dwState;

        m_dwState |= dwStateAdd;
        m_dwState &= ~dwStateRemove;

        return dwOldState;
    }

    // Get Command ID
    UINT GetCmdID()
    {
        return m_uCmdID;
    }

    HKUIWND GetKuiHWnd()
    {
        return m_hKuiWnd;
    }

    HKUIWND GetParent()
    {
        return m_hKuiWndParent;
    }

    void SetParent(HKUIWND hKuiWndParent)
    {
        m_hKuiWndParent = hKuiWndParent;
    }

    BOOL IsChecked()
    {
        return KuiWndState_Check == (m_dwState & KuiWndState_Check);
    }

    BOOL IsDisabled(BOOL bCheckParent = FALSE)
    {
        BOOL bDisable = (KuiWndState_Disable == (m_dwState & KuiWndState_Disable));

        if (bCheckParent && !bDisable)
        {
            CKuiWindow *pWndParent = this;

            while (pWndParent = KuiWnds::GetWindow(pWndParent->GetParent()))
            {
                if (pWndParent->IsDisabled())
                    return TRUE;
            }
        }

        return bDisable;
    }

    BOOL IsVisible(BOOL bCheckParent = FALSE)
    {
        BOOL bVisible = (0 == (m_dwState & KuiWndState_Invisible));

        if (bCheckParent && bVisible)
        {
            CKuiWindow *pWndParent = this;

            while (pWndParent = KuiWnds::GetWindow(pWndParent->GetParent()))
            {
                if (!pWndParent->IsVisible())
                    return FALSE;
            }
        }

        return bVisible;
    }

    void SetVisible(BOOL bVisible)
    {
        if (bVisible)
            ModifyState(0, KuiWndState_Invisible);
        else
            ModifyState(KuiWndState_Invisible, 0);
    }

    BOOL NeedRedrawParent()
    {
        return (m_style.m_strSkinName.IsEmpty() && (m_style.m_crBg == CLR_INVALID));
    }

//     void Invalidate()
//     {
//         m_bNeedRedraw = TRUE;
//     }
//
//     void Validate()
//     {
//         m_bNeedRedraw = FALSE;
//     }

    LPCTSTR GetLinkUrl()
    {
        return m_strLinkUrl;
    }

    // Get container, container is a REAL window
    HWND GetContainer()
    {
        return m_hWndContainer;
    }

    BOOL IsTransparent()
    {
        return m_bTransparent;
    }

public:

    //////////////////////////////////////////////////////////////////////////
    // Virtual functions

    // Create KuiWindow from xml element
    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!pTiXmlElem)
            return FALSE;

        {
            m_strInnerText = CA2T(pTiXmlElem->GetText(), CP_UTF8);
            if (!m_strInnerText.IsEmpty())
            {
                int nSubStringStart = 0, nSubStringEnd = 0;

                while (TRUE)
                {
                    nSubStringStart = m_strInnerText.Find(L"%IDS_", nSubStringEnd);
                    if (-1 == nSubStringStart)
                        break;

                    nSubStringEnd = m_strInnerText.Find(L'%', nSubStringStart + 4);
                    if (-1 == nSubStringEnd)
                        break;

                    CString strId = m_strInnerText.Mid(nSubStringStart + 1,
                        nSubStringEnd - nSubStringStart - 1);
                    CString strText = KAppRes::Instance().GetString(UnicodeToAnsi(
                        (const wchar_t *)strId));

                    m_strInnerText = m_strInnerText.Left(nSubStringStart)
                        + strText
                        + m_strInnerText.Mid(nSubStringEnd + 1);
                }
            }
        }

        m_uPositionType = 0;

        __super::Load(pTiXmlElem);

        int nValue = 0;
        CStringA strValue;

        strValue = pTiXmlElem->Attribute("show", &nValue);
        if (!strValue.IsEmpty())
        {
            SetVisible(nValue);
        }

        if (2 == m_dlgpos.nCount || 4 == m_dlgpos.nCount)
            return TRUE;

        strValue = pTiXmlElem->Attribute("width", &nValue);
        if (0 == nValue && "full" == strValue)
        {
            m_rcWindow.right = 0;
            m_uPositionType = (m_uPositionType & ~SizeX_Mask) | SizeX_FitParent;
        }
        else
        {
            if (nValue > 0)
            {
                m_rcWindow.right = nValue;
                m_lSpecifyWidth = nValue;
                m_uPositionType = (m_uPositionType & ~SizeX_Mask) | SizeX_Specify;
            }
            else
            {
                m_rcWindow.right = 0;
                m_uPositionType = (m_uPositionType & ~SizeX_Mask) | SizeX_FitContent;
            }
        }

        strValue = pTiXmlElem->Attribute("height", &nValue);
        if (0 == nValue && "full" == strValue)
        {
            m_rcWindow.bottom = 0;
            m_uPositionType = (m_uPositionType & ~SizeY_Mask) | SizeY_FitParent;
        }
        else
        {
            if (nValue > 0)
            {
                m_rcWindow.bottom = nValue;
                m_lSpecifyHeight = nValue;
                m_uPositionType = (m_uPositionType & ~SizeY_Mask) | SizeY_Specify;
            }
            else
            {
                m_rcWindow.bottom = 0;
                m_uPositionType = (m_uPositionType & ~SizeY_Mask) | SizeY_FitContent;
            }
        }

        if (m_uPositionType & (SizeX_FitContent | SizeY_FitContent))
        {
            SIZE sizeRet = {0, 0};

            KuiSendMessage(WM_NCCALCSIZE, FALSE, (LPARAM)&sizeRet);

            if (m_uPositionType & SizeX_FitContent)
                m_rcWindow.right = sizeRet.cx;
            if (m_uPositionType & SizeY_FitContent)
                m_rcWindow.bottom = sizeRet.cy;
        }

        nValue = 0;
        pTiXmlElem->Attribute("float", &nValue);
        m_uPositionType |= (nValue != 0 ? Position_Float : Position_Relative);

        return TRUE;
    }

    // Set container, container is a REAL window
    virtual void SetContainer(HWND hWndContainer)
    {
        m_hWndContainer = hWndContainer;
    }

    // Get handle from point
    virtual HKUIWND KuiGetHWNDFromPoint(POINT ptHitTest, BOOL bOnlyText)
    {
        if (m_rcWindow.PtInRect(ptHitTest))
            return m_hKuiWnd;
        else
            return NULL;
    }

    virtual BOOL NeedRedrawWhenStateChange()
    {
        if (!m_style.m_strSkinName.IsEmpty())
        {
            CKuiSkinBase* pSkin = KuiSkin::GetSkin(m_style.m_strSkinName);
            if (pSkin && !pSkin->IgnoreState())
                return TRUE;
        }

        return (CLR_INVALID != m_style.m_crHoverText) || (NULL != m_style.m_ftHover) || (CLR_INVALID != m_style.m_crBgHover);
    }

//     virtual BOOL NeedRedraw()
//     {
//         return m_bNeedRedraw;
//     }

    virtual BOOL IsContainer()
    {
        return FALSE;
    }

    virtual BOOL RedrawRegion(CDCHandle& dc, CRgn& rgn)
    {
        if (rgn.IsNull() || rgn.RectInRegion(m_rcWindow))
        {
            DrawKuignd(dc);

            KuiSendMessage(WM_PAINT, (WPARAM)(HDC)dc);

            return TRUE;
        }

        return FALSE;
    }

    void NotifyInvalidate()
    {
        NotifyInvalidateRect(m_rcWindow);
    }

    void NotifyInvalidateRect(LPRECT lprect)
    {
        if (lprect)
        {
            CRect rect = *lprect;
            NotifyInvalidateRect(rect);
        }
    }

    void NotifyInvalidateRect(const CRect& rect)
    {
        if (IsVisible(TRUE) && m_hWndContainer && ::IsWindow(m_hWndContainer))
        {
            KUIINMINVALIDATERECT nms;

            nms.hdr.code        = KUIINM_INVALIDATERECT;
            nms.hdr.hwndFrom    = NULL;
            nms.hdr.idFrom      = m_uCmdID;
            nms.hkuiWnd          = m_hKuiWnd;
            nms.rect            = rect;

            ::SendMessage(m_hWndContainer, WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
        }
    }

public:

    //////////////////////////////////////////////////////////////////////////
    // Message Handler

    void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
    {
        m_rcWindow.MoveToXY(lpWndPos->x, lpWndPos->y);
        SIZE sizeRet = {lpWndPos->cx, lpWndPos->cy};

        KuiSendMessage(WM_NCCALCSIZE, TRUE, (LPARAM)&sizeRet);

        m_rcWindow.SetRect(lpWndPos->x, lpWndPos->y, lpWndPos->x + sizeRet.cx, lpWndPos->y + sizeRet.cy);

        if (GetPositionType() & SizeX_FitParent)
            m_rcWindow.right = max(lpWndPos->x + lpWndPos->cx, m_rcWindow.left);
        else if (GetPositionType() & SizeX_Specify)
            m_rcWindow.right = m_rcWindow.left + m_lSpecifyWidth;

        if (GetPositionType() & SizeY_FitParent)
            m_rcWindow.bottom = max(lpWndPos->y + lpWndPos->cy, m_rcWindow.top);
        else if (GetPositionType() & SizeY_Specify)
            m_rcWindow.bottom = m_rcWindow.top + m_lSpecifyHeight;
    }

    int OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
    {
        return TRUE;
    }

    void OnDestroy()
    {
    }

    // Draw background default
    void DrawKuignd(CDCHandle& dc)
    {
        if (m_style.m_strSkinName.IsEmpty())
        {
            COLORREF crBg = m_style.m_crBg;

            if (KuiWndState_Hover == (GetState() & KuiWndState_Hover) && CLR_INVALID != m_style.m_crBgHover)
            {
                crBg = m_style.m_crBgHover;
            }

            if (CLR_INVALID != crBg)
                dc.FillSolidRect(m_rcWindow, crBg);
        }
        else
        {
            CKuiSkinBase* pSkin = KuiSkin::GetSkin(m_style.m_strSkinName);
            if (pSkin)
            {
                pSkin->Draw(dc, m_rcWindow, m_dwState);
            }
        }
    }

    struct KuiDCPaint
    {
        KuiDCPaint()
            : bKuiModeChanged(FALSE)
            , nOldKuiMode(OPAQUE)
            , bFontChanged(FALSE)
            , hftOld(NULL)
            , bTextColorChanged(FALSE)
            , crOld(CLR_INVALID)
            , bBgColorChanged(FALSE)
            , crOldBg(CLR_INVALID)
        {
        }
        BOOL     bKuiModeChanged;
        int      nOldKuiMode;
        BOOL     bFontChanged;
        HFONT    hftOld;
        BOOL     bTextColorChanged;
        COLORREF crOld;
        BOOL     bBgColorChanged;
        COLORREF crOldBg;
        CRect    rcClient;
    };

    void BeforePaint(CDCHandle &dc, KuiDCPaint &KuiDC)
    {
        HFONT /*hftOld = NULL, */hftDraw = NULL;
        COLORREF /*crOld = CLR_INVALID, crOldBg = CLR_INVALID, */crDraw = CLR_INVALID;
        /*int nOldKuiMode = OPAQUE;*/

        KuiDC.rcClient = m_rcWindow;
        KuiDC.rcClient.DeflateRect(m_style.m_nMarginX, m_style.m_nMarginY);

        if (!m_style.m_strSkinName.IsEmpty())
        {
            KuiDC.bKuiModeChanged = TRUE;
            KuiDC.nOldKuiMode = dc.SetBkMode(TRANSPARENT);
        }
        if (CLR_INVALID != m_style.m_crBg)
        {
            KuiDC.bBgColorChanged = TRUE;
            KuiDC.crOldBg = dc.SetBkColor(m_style.m_crBg);
        }

        if (m_style.m_ftText)
            hftDraw = m_style.m_ftText;

        if (m_style.m_crText != CLR_INVALID)
            crDraw = m_style.m_crText;

        if (IsDisabled(TRUE)/*KuiWndState_Disable == (GetState() & KuiWndState_Disable)*/)
        {
            if (m_style.m_crDisabledText != CLR_INVALID)
                crDraw = m_style.m_crDisabledText;
        }
        else if (KuiWndState_Hover == (GetState() & KuiWndState_Hover))
        {
            if (m_style.m_ftHover)
                hftDraw = m_style.m_ftHover;

            if (m_style.m_crHoverText != CLR_INVALID)
                crDraw = m_style.m_crHoverText;
        }

        if (hftDraw)
        {
            KuiDC.bFontChanged = TRUE;
            KuiDC.hftOld = dc.SelectFont(hftDraw);
        }

        if (crDraw != CLR_INVALID)
        {
            KuiDC.bTextColorChanged = TRUE;
            KuiDC.crOld = dc.SetTextColor(crDraw);
        }
    }

    void AfterPaint(CDCHandle &dc, KuiDCPaint &KuiDC)
    {
        if (KuiDC.bFontChanged)
            dc.SelectFont(KuiDC.hftOld);

        if (KuiDC.bTextColorChanged)
            dc.SetTextColor(KuiDC.crOld);

        if (KuiDC.bKuiModeChanged)
            dc.SetBkMode(KuiDC.nOldKuiMode);

        if (KuiDC.bBgColorChanged)
            dc.SetBkColor(KuiDC.crOldBg);
    }

    // Draw inner text default
    void OnPaint(CDCHandle dc)
    {
        KuiDCPaint KuiDC;

        BeforePaint(dc, KuiDC);

        dc.DrawText(m_strInnerText, m_strInnerText.GetLength(), KuiDC.rcClient, m_style.m_nTextAlign);

        AfterPaint(dc, KuiDC);
    }

    // Calc KuiWindow size
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        LPSIZE pSize = (LPSIZE)lParam;

        if (4 == m_dlgpos.nCount)
        {
            CRect rect;

            if (m_dlgpos.Left.bCenter)
                rect.left = pSize->cx / 2 + (m_dlgpos.Left.bMinus ? -1 : 1) *  m_dlgpos.Left.nPos;
            else if (m_dlgpos.Left.bMinus)
                rect.left = pSize->cx - m_dlgpos.Left.nPos;
            else
                rect.left = m_dlgpos.Left.nPos;

            if (m_dlgpos.Right.bCenter)
                rect.right = pSize->cx / 2 + (m_dlgpos.Right.bMinus ? -1 : 1) *  m_dlgpos.Right.nPos;
            else if (m_dlgpos.Right.bMinus)
                rect.right = pSize->cx - m_dlgpos.Right.nPos;
            else
                rect.right = m_dlgpos.Right.nPos;

            if (m_dlgpos.Top.bCenter)
                rect.top = pSize->cy / 2 + (m_dlgpos.Top.bMinus ? -1 : 1) *  m_dlgpos.Top.nPos;
            else if (m_dlgpos.Top.bMinus)
                rect.top = pSize->cy - m_dlgpos.Top.nPos;
            else
                rect.top = m_dlgpos.Top.nPos;

            if (m_dlgpos.Bottom.bCenter)
                rect.bottom = pSize->cy / 2 + (m_dlgpos.Bottom.bMinus ? -1 : 1) *  m_dlgpos.Bottom.nPos;
            else if (m_dlgpos.Bottom.bMinus)
                rect.bottom = pSize->cy - m_dlgpos.Bottom.nPos;
            else
                rect.bottom = m_dlgpos.Bottom.nPos;

            *pSize = rect.Size();
        }
        else
        {
            CDCHandle dcDesktop = ::GetDC(::GetDesktopWindow());
            CDC dcTest;
            HFONT hftOld = NULL, hftTest = NULL;
            CRect rcTest = m_rcWindow;

            dcTest.CreateCompatibleDC(dcDesktop);

            if (bCalcValidRects)
            {
                rcTest.right = rcTest.left + pSize->cx;
                rcTest.bottom = rcTest.top + pSize->cy;
            }

            if (m_style.m_ftText)
                hftTest = m_style.m_ftText;
            else
            {
                CKuiWindow* pWnd = this;
                HKUIWND hWndParent = NULL;

                while (hWndParent = pWnd->GetParent())
                {
                    pWnd = KuiWnds::GetWindow(hWndParent);
                    if (pWnd->m_style.m_ftText)
                    {
                        hftTest = pWnd->m_style.m_ftText;
                        break;
                    }
                }

            }

            if (KuiWndState_Hover == (GetState() & KuiWndState_Hover))
            {
                if (m_style.m_ftHover)
                    hftTest = m_style.m_ftHover;
                else
                {
                    CKuiWindow* pWnd = this;
                    HKUIWND hWndParent = NULL;

                    while (hWndParent = pWnd->GetParent())
                    {
                        pWnd = KuiWnds::GetWindow(hWndParent);
                        if (pWnd->m_style.m_ftHover)
                        {
                            hftTest = pWnd->m_style.m_ftHover;
                            break;
                        }
                    }
                }
            }

            if (NULL == hftTest)
                hftTest = KuiFontPool::GetFont(KUIF_DEFAULTFONT);

            hftOld = dcTest.SelectFont(hftTest);

            rcTest.DeflateRect(m_style.m_nMarginX, m_style.m_nMarginY);

            int nTestDrawMode = m_style.m_nTextAlign & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

            if (nTestDrawMode & DT_WORDBREAK)
                rcTest.bottom = 0x7FFF;

            dcTest.DrawText(m_strInnerText, m_strInnerText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);

            pSize->cx = rcTest.Width() + 2 * m_style.m_nMarginX;
            pSize->cy = rcTest.Height() + 2 * m_style.m_nMarginY;

            dcTest.SelectFont(hftOld);

            ::ReleaseDC(::GetDesktopWindow(), dcDesktop);
        }

        return 0;
    }

    void OnShowWindow(BOOL bShow, UINT nStatus)
    {
        if (bShow)
            ModifyState(0, KuiWndState_Invisible);
        else
            ModifyState(KuiWndState_Invisible, 0);
    }

    HRESULT OnAttributePosChange(CStringA& strValue, BOOL bLoading)
    {
        if (!strValue.IsEmpty())
        {
            LPCSTR lpszValue = strValue;
            int nPos = 0, nPosPrev = 0;

            for (m_dlgpos.nCount = 0; m_dlgpos.nCount < 4; m_dlgpos.nCount ++)
            {
                nPosPrev = nPos;
                nPos = strValue.Find(',', nPosPrev) + 1;

                m_dlgpos.Item[m_dlgpos.nCount].bCenter = ('|' == lpszValue[nPosPrev]);
                if (m_dlgpos.Item[m_dlgpos.nCount].bCenter)
                    nPosPrev ++;
                m_dlgpos.Item[m_dlgpos.nCount].bMinus = ('-' == lpszValue[nPosPrev]);
                if (m_dlgpos.Item[m_dlgpos.nCount].bMinus)
                    nPosPrev ++;

                if (0 == nPos)
                {
                    m_dlgpos.Item[m_dlgpos.nCount].nPos = ::StrToIntA(strValue.Mid(nPosPrev));
                    m_dlgpos.nCount ++;
                    break;
                }

                m_dlgpos.Item[m_dlgpos.nCount].nPos = ::StrToIntA(strValue.Mid(nPosPrev, nPos - nPosPrev));
            }

            if (2 == m_dlgpos.nCount || 4 == m_dlgpos.nCount)
            {
                m_uPositionType = (m_uPositionType & ~SizeX_Mask) | SizeX_FitContent;
                m_uPositionType = (m_uPositionType & ~SizeY_Mask) | SizeY_FitContent;
            }
            else
                m_dlgpos.nCount = 0;

            if (!bLoading)
            {
                _RepositionSelf();
            }
        }

        return S_FALSE;
    }

    void _RepositionSelf();

protected:
    BOOL m_bMsgHandled;

    BOOL IsMsgHandled() const
    {
        return m_bMsgHandled;
    }
    void SetMsgHandled(BOOL bHandled)
    {
        m_bMsgHandled = bHandled;
    }

    KUIWIN_DECLARE_RETURN_SETTING(FALSE, FALSE)

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    KUIWIN_END_MSG_MAP_BASE()
//
//     enum {
//         KUIWND_INNERTEXT = 1,
//     };

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_STYLE_ATTRIBUTE("class", m_style, TRUE)
        KUIWIN_INT_ATTRIBUTE("id", m_uCmdID, FALSE)//, 0)
        KUIWIN_COLOR_ATTRIBUTE("crbg", m_style.m_crBg, FALSE)
        KUIWIN_COLOR_ATTRIBUTE("crtext", m_style.m_crText, FALSE)
        KUIWIN_FONT_ATTRIBUTE("font", m_style.m_ftText, FALSE)
        KUIWIN_TSTRING_ATTRIBUTE("href", m_strLinkUrl, FALSE)
        KUIWIN_TSTRING_ATTRIBUTE("tip", m_strToolTipText, FALSE)
        KUIWIN_ENUM_ATTRIBUTE("valign", UINT, TRUE)
            KUIWIN_ENUM_VALUE("top", VAlign_Top)
            KUIWIN_ENUM_VALUE("middle", VAlign_Middle)
            KUIWIN_ENUM_VALUE("bottom", VAlign_Bottom)
        KUIWIN_ENUM_END(m_uVAlign)
        KUIWIN_ENUM_ATTRIBUTE("align", UINT, TRUE)
            KUIWIN_ENUM_VALUE("left", Align_Left)
            KUIWIN_ENUM_VALUE("center", Align_Center)
            KUIWIN_ENUM_VALUE("right", Align_Right)
        KUIWIN_ENUM_END(m_uHAlign)
        KUIWIN_CUSTOM_ATTRIBUTE("pos", OnAttributePosChange)
        KUIWIN_INT_ATTRIBUTE("transparent", m_bTransparent, FALSE)//, 0)
    KUIWIN_DECLARE_ATTRIBUTES_END()
};

// ÒÑ·ÏÆú
inline CKuiWindow* KuiWnds::GetWindow(UINT uCmdID, HWND hWndInContainer/* = NULL*/)
{
	CKuiWindow* pWndRet = NULL;

	::EnterCriticalSection(&ms_lockWndMap);

    POSITION pos = _Instance()->m_mapPool.GetStartPosition();

    while (pos)
    {
        const _KuiWndHandlePool::CPair *pairRet = _Instance()->m_mapPool.GetNext(pos);

        if (pairRet->m_value->GetCmdID() == uCmdID)
        {
            if (NULL == hWndInContainer || hWndInContainer == pairRet->m_value->GetContainer())
			{
				pWndRet = pairRet->m_value;
				break;
			}
        }
    }

	::LeaveCriticalSection(&ms_lockWndMap);

    return pWndRet;
}


inline void KuiWnds::SelectRadio(CKuiWindow* pkuiWnd, LPCSTR lpszGroupName, CKuiWindow **ppkuiWndLastSelect)
{
	::EnterCriticalSection(&ms_lockWndMap);

    CAtlList<CKuiWindow *> &ListGroup = _Instance()->m_mapRadioPool[lpszGroupName];

    pkuiWnd->ModifyState(KuiWndState_Check, 0);

    POSITION posCurrent = NULL, pos = ListGroup.GetHeadPosition();

    while (pos)
    {
        posCurrent = pos;
        CKuiWindow *pkuiWndFind = ListGroup.GetNext(pos);

        if (pkuiWndFind == pkuiWnd)
            continue;

        if (pkuiWndFind->IsChecked())
        {
            pkuiWndFind->ModifyState(0, KuiWndState_Check);
            pkuiWndFind->NotifyInvalidate();

            if (ppkuiWndLastSelect)
                *ppkuiWndLastSelect = pkuiWndFind;
        }
    }

	::LeaveCriticalSection(&ms_lockWndMap);
}

class CKuiContainerWnd : public CKuiWindow
{
public:
    virtual CKuiWindow* FindChildByCmdID(UINT uCmdID)
    {
        ATLASSERT(FALSE);

        return NULL;
    }

    virtual void RepositionChilds()
    {
    }

    virtual void RepositionChild(CKuiWindow *pKuiWndChild)
    {
    }

    BOOL IsContainer()
    {
        return TRUE;
    }
};

inline void CKuiWindow::_RepositionSelf()
{
    CKuiWindow *pWndParent = KuiWnds::GetWindow(GetParent());
    if (pWndParent && pWndParent->IsContainer())
    {
        CKuiContainerWnd *pWndParentContainer = (CKuiContainerWnd *)pWndParent;
        pWndParentContainer->RepositionChild(this);
    }
}

#pragma once

#include <atlcoll.h>
#include <wtlhelper/whwindow.h>

#define MB_BK_CUSTOM_BUTTON         0x80000000

class CKuiMsgBox
    : public CKuiDialogImpl<CKuiMsgBox>
    , public CWHRoundRectFrameHelper<CKuiMsgBox>
{
public:
    CKuiMsgBox()
        : m_bShowNoNotifyLater(FALSE)
        , m_bNoNotifyLater(FALSE)
        , m_uType(MB_OK)
		, m_isTextXml(FALSE)
		, CKuiDialogImpl<CKuiMsgBox>("IDR_KSC_MESSAGEBOX")
    {

    }

    static UINT_PTR Show(
        LPCWSTR lpszText, 
        LPCWSTR lpszCaption     = NULL, 
        UINT uType              = MB_OK, 
        BOOL *pbNoNotifyLater   = NULL, 
        HWND hWndParent         = NULL);

    void AddButton(LPCWSTR lpszText, UINT uCmdID, UINT uWidth = 0);
    void AddButtonEx(LPCWSTR lpszText, UINT uCmdID, LPCWSTR lpszPos);
    void SetNoNotifyText(LPCWSTR lpszText);

    UINT_PTR ShowMsg(
        LPCWSTR lpszText, 
        LPCWSTR lpszCaption     = NULL, 
        UINT uType              = MB_OK, 
        BOOL *pbNoNotifyLater   = NULL, 
        HWND hWndParent         = NULL);
	UINT_PTR ShowPanelMsg(
		LPCWSTR lpszXml, 
		LPCRECT lpRect,				// Required Xml msg RECT 
		LPCWSTR lpszCaption     = NULL, 
		UINT uType              = MB_OK, 
		BOOL *pbNoNotifyLater   = NULL, 
		HWND hWndParent         = NULL
		);

protected:

	BOOL m_isTextXml;	// true:use m_rcXml or use TEXT_CALC(m_strText) 
	RECT m_rcXml;
    CString m_strText;
    CString m_strCaption;
    CString m_strNoNotifyText;
    UINT m_uType;
    BOOL m_bShowNoNotifyLater;
    BOOL m_bNoNotifyLater;

    class __BKMBButton
    {
    public:
        __BKMBButton(LPCWSTR lpszText, UINT uid, UINT width)
            : uCmdID(uid)
            , strText(lpszText)
            , uWidth(width)
        {

        }
        CString strText;
        UINT    uCmdID;
        UINT    uWidth;
    };

    class __BKMBButtonEx
    {
    public:
        __BKMBButtonEx(LPCWSTR lpszText, UINT uid, const CString& pos)
            : uCmdID(uid)
            , strText(lpszText)
            , strPos(pos)
        {

        }
        CString strText;
        UINT    uCmdID;
        CString strPos;
    };

    CAtlList<__BKMBButton> m_lstButton;
    CAtlList<__BKMBButtonEx> m_lstButtonEx;

	void _InitLayout( BOOL * pbNoNotifyLater );
    void _MakeButtons();
    void OnBkCommand(UINT uItemID, LPCSTR szItemClass);
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_COMMAND(OnBkCommand)
    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CKuiMsgBox)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CKuiDialogImpl<CKuiMsgBox>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CKuiMsgBox>)
        MSG_WM_INITDIALOG(OnInitDialog)
    END_MSG_MAP()
};

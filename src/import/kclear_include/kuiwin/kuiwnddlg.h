#pragma once

#include "kuiwndpanel.h"

class CKuiDialog
    : public CKuiPanel
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiDialog, "dlg")

public:

    void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
    {
        CKuiWindow::OnWindowPosChanged(lpWndPos);

        _RepositionChilds();
    }

    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        BOOL bRet = __super::Load(pTiXmlElem);

        return bRet;
    }

    virtual void RepositionChilds()
    {
        _RepositionChilds();
    }

    virtual void RepositionChild(CKuiWindow *pKuiWndChild)
    {
        KUIDLG_POSITION dlgPos;
        WINDOWPOS WndPos = {0};
        CRect rcDlg = m_rcWindow;

        rcDlg.DeflateRect(m_style.m_nMarginX, m_style.m_nMarginY);

        pKuiWndChild->GetDlgPosition(&dlgPos);

        if (dlgPos.Left.bCenter)
            WndPos.x = rcDlg.left + rcDlg.Width() / 2 + (dlgPos.Left.bMinus ? -1 : 1) *  dlgPos.Left.nPos;
        else if (dlgPos.Left.bMinus)
            WndPos.x = rcDlg.left + rcDlg.Width() - dlgPos.Left.nPos;
        else
            WndPos.x = rcDlg.left + dlgPos.Left.nPos;

        if (dlgPos.Top.bCenter)
            WndPos.y = rcDlg.top + rcDlg.Height() / 2 + (dlgPos.Top.bMinus ? -1 : 1) *  dlgPos.Top.nPos;
        else if (dlgPos.Top.bMinus)
            WndPos.y = rcDlg.top + rcDlg.Height() - dlgPos.Top.nPos;
        else
            WndPos.y = rcDlg.top + dlgPos.Top.nPos;

        WndPos.cx = rcDlg.Width();
        WndPos.cy = rcDlg.Height();
        pKuiWndChild->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);
    }

protected:

    void _RepositionChilds()
    {
        POSITION pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            RepositionChild(pKuiWndChild);
        }
    }

protected:

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
    KUIWIN_END_MSG_MAP()
};

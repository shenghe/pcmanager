#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////

class CBigFileListWrapper : public CWindowImpl<CBigFileListWrapper>
{
public:
    CBigFileListWrapper() {}
    ~CBigFileListWrapper() {}

    BEGIN_MSG_MAP_EX(CBigFileListWrapper)
        //MSG_WM_PAINT(OnPaint)
        MSG_WM_SIZE(OnSize)
        REFLECT_NOTIFICATIONS()
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    void Init()
    {
        ModifyStyle(0, WS_CHILD|WS_CLIPCHILDREN);
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL retval = FALSE;
        retval = __super::SubclassWindow(hWnd);
        Init();
        return retval;
    }

    void OnPaint(CDCHandle dc)
    {
    }

    void AddChild(HWND hChildWnd)
    {
        bool existed = false;
        for (size_t i = 0; i < m_vChildWnds.size(); ++i)
        {
            if (m_vChildWnds[i] == hChildWnd)
            {
                existed = true;
                break;
            }
        }
        
        if (!existed)
            m_vChildWnds.push_back(hChildWnd);
    }

    void ShowChild(HWND hChildWnd)
    {
        for (size_t i = 0; i < m_vChildWnds.size(); ++i)
        {
            if (m_vChildWnds[i] != hChildWnd)
            {
                ::ShowWindow(m_vChildWnds[i], SW_HIDE);
            }
        }

        ::ShowWindow(hChildWnd, SW_SHOW);
    }

    void OnSize(UINT nType, CSize size)
    {
        for (size_t i = 0; i < m_vChildWnds.size(); ++i)
        {
            ::MoveWindow(m_vChildWnds[i], 0, 0, size.cx, size.cy, TRUE);
        }
    }

protected:
    std::vector<HWND> m_vChildWnds;
};

//////////////////////////////////////////////////////////////////////////

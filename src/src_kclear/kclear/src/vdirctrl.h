#pragma once

//////////////////////////////////////////////////////////////////////////

#include "kscbase/ksclock.h"

//////////////////////////////////////////////////////////////////////////

class CVirtualDirCtrl;
typedef CWindowImpl<CVirtualDirCtrl> CVirtualDirCtrlBase;

//////////////////////////////////////////////////////////////////////////

typedef struct tagVDir {
    CString strName;
    CString strComment;
    UINT nPercent;
    BOOL bShowPercent;
    PVOID pData;
    BOOL bVisual;
} VDir;

typedef struct tagVDirSet {
    HICON hIcon;
    CString strTitle;
    std::vector<VDir> vDirs;
    BOOL bHasScroll;
} VDirSet;

//////////////////////////////////////////////////////////////////////////

class CVirtualDirCtrl
    : public CVirtualDirCtrlBase
    , public CDoubleBufferImpl<CVirtualDirCtrl>
{
public:
    DECLARE_WND_CLASS(L"KVDirs");

    CVirtualDirCtrl();
    virtual ~CVirtualDirCtrl();

    void Init();
    void UnInit();
    size_t AddItem(HICON hIcon, const CString& strTitle);
    size_t AddSubItem(
        size_t nItem,
        const CString& strName, 
        const CString& strComment, 
        UINT nPercent = 0, 
        BOOL bShowPercent = FALSE
        );
    void UpdateSubItem(
        size_t nItem, 
        size_t nSubItem, 
        const CString& strName, 
        const CString& strComment, 
        UINT nPercent = 0, 
        BOOL bShowPercent = FALSE
        );
    void Select(size_t nItem, size_t nSubItem);
    void SetItemData(size_t nItem, size_t nSubItem, PVOID pData);
    PVOID GetItemData(size_t nItem, size_t nSubItem);

    BEGIN_MSG_MAP_EX(CVirtualDirCtrl)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        CHAIN_MSG_MAP(CDoubleBufferImpl<CVirtualDirCtrl>)
    END_MSG_MAP()

    void DoPaint(CDCHandle /*dc*/);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);

protected:
    void PaintVDirSet(CDCHandle& dc, size_t idx);
    UINT GetPaintOffsetY(size_t idx);
    UINT GetPaintHeight(size_t idx);
    BOOL NeedScrollBar();
    WORD HitTest(CPoint pt);

private:
    std::vector<VDirSet> m_vBigSets;
    size_t m_nSelect;
    size_t m_nSubSelect;
    KLock m_lock;
    CRect m_rcClient;
    CRect m_rcUp;
    CRect m_rcDown;
    UINT m_nScrollOffset;
    UINT m_nScrollLimit;
};

//////////////////////////////////////////////////////////////////////////

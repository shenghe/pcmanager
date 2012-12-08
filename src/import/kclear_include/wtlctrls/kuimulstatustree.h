/********************************************************************
* CreatedOn: 2006-7-18   16:21
* FileName: KASMulStatusTree.h
* CreatedBy: lidengwang<lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL: $
* Purpose:
*********************************************************************/
#pragma once

#undef __IStream_INTERFACE_DEFINED__

#include <string>
#include <vector>
#include <map>

#define WM_KAN_NOTIFY_TREE_CHECKED_CHANGE	(WM_USER + 103)
#define WM_KAN_NOTIFY_TREE_SELECT_CHANGE	(WM_USER + 104)


class KUIMulStatusTree : 
    public ATL::CWindowImpl<KUIMulStatusTree, CTreeViewCtrl>
{
public:
    
	enum{
        EM_TVIS_NONE          = 0,
        EM_TVIS_UNCHECK       = 1,
        EM_TVIS_INDETERMINING = 2,
        EM_TVIS_CHECK         = 3
    };

public:
	KUIMulStatusTree(void);
	~KUIMulStatusTree(void);

    BOOL SubclassWindow(HWND hWnd);

    BEGIN_MSG_MAP(KASMulStatusTree)

        MESSAGE_HANDLER(WM_CREATE  , OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,  OnDestroy)

        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK,OnLDBCLK);

        MESSAGE_HANDLER(WM_KEYDOWN    , OnKeyDown)
        REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK   , OnStateIconClick)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_KEYDOWN, OnTvnKeydown)

        REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnNMRClick)

    END_MSG_MAP()
	
	HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage,
		int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter, int nCheck = EM_TVIS_UNCHECK );
	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter,  int nCheck);
    
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnStateIconClick(int nCode, LPNMHDR pnmh, BOOL& bHandled);    
    LRESULT OnTvnKeydown    (int nCode, LPNMHDR pnmh, BOOL& bHandled);   

    LRESULT OnKeyDown    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLDBCLK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNMRClick(int, LPNMHDR pnmh, BOOL&);

public:
	void EnableCheck(BOOL bEnable)
	{
		m_bEnableCheck = bEnable;
	}

    BOOL GetCheck(HTREEITEM hItem, int& nCheck);
    BOOL SetCheck(HTREEITEM hItem, int  nCheck, BOOL bUpdate = TRUE);

    HTREEITEM GetFirstCheck(HTREEITEM hFirstCheck = NULL);
    HTREEITEM GetNextCheck(HTREEITEM hPrevCheck);

    void TravelChild(HTREEITEM hItem, int nCheck);
    void TravelSiblingAndParent(HTREEITEM hItem, int nCheck);

public:

    BOOL GetSelectPaths(std::vector<CString>& vtPaths);

private:

    void _Init();
	void _Uninit();

    bool CheckFolder(LPITEMIDLIST pidl);

private:
	LPCTSTR		m_ICO_ID;
    UINT		m_uClickFlags;
    CImageList	m_stateImageList;

	BOOL		m_bEnableCheck;		// 是否允许选中checkbox
};

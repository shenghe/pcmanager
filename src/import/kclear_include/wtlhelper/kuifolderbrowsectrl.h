#pragma once

#include <shobjidl.h>
#include <atlcoll.h>
#include "iidl.h"
#include "wh3statecheckimglst.h"

#define WH3STVN_ITEMCHECKCHANGE 1

class CWH3StateCheckTreeCtrl
    : public CWindowImpl<CWH3StateCheckTreeCtrl, CTreeViewCtrl>
{
public:

    enum {
        CHECK_STATE_NONE      = 0,
        CHECK_STATE_UNCHECKED,
        CHECK_STATE_CHECKED,
        CHECK_STATE_MIXED,
    };

    BOOL GetCheckState(HTREEITEM hItem)
    {
        return CHECK_STATE_CHECKED == (GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12);
    }

    BOOL HasCheckedItem()
    {
        HTREEITEM hItem = GetChildItem(TVI_ROOT);

        while (NULL != hItem)
        {
            switch (GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12)
            {
            case CHECK_STATE_UNCHECKED:
                break;

            case CHECK_STATE_CHECKED:
            case CHECK_STATE_MIXED:
                return TRUE;

            case CHECK_STATE_NONE:
                hItem = GetChildItem(hItem);
                continue;
            }

            HTREEITEM hNextItem = GetNextSiblingItem(hItem);
            if (NULL == hNextItem)
            {
                HTREEITEM hParentItem = hItem;

                do
                {
                    hParentItem = GetParentItem(hParentItem);
                    if (NULL == hParentItem)
                        break;

                    hNextItem = GetNextSiblingItem(hParentItem);

                } while (NULL == hNextItem);
            }

            hItem = hNextItem;
        }

        return FALSE;
    }

    void Init3State()
    {
        _Init3State();
    }

protected:

    CWH3StateCheckImageList m_wnd3StateCheckImgList;

    void _Init3State()
    {
        m_wnd3StateCheckImgList.Create();

        SetImageList(m_wnd3StateCheckImgList, TVSIL_STATE);
    }

    void _CheckSelfAndAllChilds(HTREEITEM hItem, BOOL bCheck)
    {
        SetCheckState(hItem, bCheck);

        HTREEITEM hItemChild = GetChildItem(hItem);
        while (NULL != hItemChild)
        {
            _CheckSelfAndAllChilds(hItemChild, bCheck);

            hItemChild = GetNextSiblingItem(hItemChild);
        }
    }

    void _ResetParentState(HTREEITEM hItem)
    {
        HTREEITEM hItemParent = GetParentItem(hItem);

        if (NULL == hItemParent)
            return;

        UINT uCheck = (GetItemState(hItemParent, TVIS_STATEIMAGEMASK) >> 12);

        if (CHECK_STATE_NONE == uCheck)
            return;

        HTREEITEM hItemParentChild = GetChildItem(hItemParent);

        BOOL bHasCheck = FALSE, bHasUncheck = FALSE;

        while (NULL != hItemParentChild)
        {
            uCheck = (GetItemState(hItemParentChild, TVIS_STATEIMAGEMASK) >> 12);

            bHasCheck |= CHECK_STATE_CHECKED == uCheck || CHECK_STATE_MIXED == uCheck;
            bHasUncheck |= CHECK_STATE_UNCHECKED == uCheck || CHECK_STATE_MIXED == uCheck;

            hItemParentChild = GetNextSiblingItem(hItemParentChild);
        }

        if (bHasCheck)
        {
            uCheck = CHECK_STATE_MIXED;
//             if (bHasUncheck)
//             else
//                 uCheck = CHECK_STATE_CHECKED;
        }
        else
        {
            if (bHasUncheck)
                uCheck = CHECK_STATE_UNCHECKED;
            else
                uCheck = CHECK_STATE_NONE;
        }

        SetItemState(hItemParent, INDEXTOSTATEIMAGEMASK(uCheck), TVIS_STATEIMAGEMASK);

        _ResetParentState(hItemParent);
    }

    void _ToggleItem(HTREEITEM hItem)
    {
        UINT uState = GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12;

        if (CHECK_STATE_NONE == uState)
            return;

        SetRedraw(FALSE);

        BOOL bCheck = !(CHECK_STATE_CHECKED == uState);

        _CheckSelfAndAllChilds(hItem, bCheck);
        _ResetParentState(hItem);

        SetRedraw(TRUE);

        NMHDR nms;
        nms.code = WH3STVN_ITEMCHECKCHANGE;
        nms.hwndFrom = m_hWnd;
        nms.idFrom = GetDlgCtrlID();

        LRESULT lRet = ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.idFrom, (WPARAM)&nms);
    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        TVHITTESTINFO htinfo;

        ZeroMemory(&htinfo, sizeof(htinfo));

        htinfo.pt = point;

        HitTest(&htinfo);

        if (NULL == htinfo.hItem || TVHT_ONITEMSTATEICON != htinfo.flags)
        {
            SetMsgHandled(FALSE);
            return;
        }

        _ToggleItem(htinfo.hItem);
    }

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (VK_SPACE != nChar)
        {
            SetMsgHandled(FALSE);
            return;
        }

        HTREEITEM hItem = GetSelectedItem();
        if (NULL == hItem)
        {
            SetMsgHandled(FALSE);
            return;
        }

        _ToggleItem(hItem);
    }

public:

    BEGIN_MSG_MAP_EX(CKuiFolderBrowseCtrl)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
        MSG_WM_KEYDOWN(OnKeyDown)
    END_MSG_MAP()
};

// 采用异步方式添加节点时的消息
#define MSG_KUIFBC_INSERT_ITEM   (WM_APP)

class CKuiFolderBrowseCtrl : public CWH3StateCheckTreeCtrl
{
protected:
    typedef CAtlMap<HTREEITEM, CString> __MapItemToPath;

public:
    CKuiFolderBrowseCtrl()
        : m_hItemMyComputer(NULL)
    {
    }

    HWND Create(
        HWND hWndParent, UINT uID = 0U, DWORD dwStyle = 0, DWORD dwExStyle = 0,
        _U_RECT rect = NULL)
    {
        dwStyle |= WS_CHILD | TVS_CHECKBOXES | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS;

        m_hItemMyComputer = NULL;

        HWND hWnd = __super::Create(hWndParent, rect, NULL, dwStyle, dwExStyle, uID);
        if (NULL == hWnd)
            return NULL;

        _Init();

        _Init3State();

        return hWnd;
    }

    void GetSelectedPathes(CAtlList<CString>& lstPathes)
    {
        HTREEITEM hItem = GetChildItem(TVI_ROOT);
        CString strPath;

        lstPathes.RemoveAll();

        while (NULL != hItem)
        {
            switch (GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12)
            {
            case CHECK_STATE_UNCHECKED:
                break;

            case CHECK_STATE_CHECKED:
                if (hItem != m_hItemMyComputer)
                {
                    _GetFullPath(hItem, strPath);
                    lstPathes.AddTail(strPath);
                    break;
                }

            case CHECK_STATE_NONE:
            case CHECK_STATE_MIXED:
                hItem = GetChildItem(hItem);
                continue;
            }

            HTREEITEM hNextItem = GetNextSiblingItem(hItem);
            if (NULL == hNextItem)
            {
                HTREEITEM hParentItem = hItem;

                do
                {
                    hParentItem = GetParentItem(hParentItem);
                    if (NULL == hParentItem)
                        break;

                    hNextItem = GetNextSiblingItem(hParentItem);

                } while (NULL == hNextItem);
            }

            hItem = hNextItem;
        }
    }

    BOOL Expand(HTREEITEM hItem, UINT nCode)
    {
        if ((nCode != TVE_EXPAND) && (nCode != TVE_TOGGLE))
            return TRUE;

        TVITEM tvi = {0};
        TVITEM *ptvi = &tvi;
        ZeroMemory(&tvi, sizeof(tvi));

        tvi.hItem = hItem;
        tvi.mask =  TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_PARAM;
        tvi.stateMask = 0x0FFFF;

        GetItem(&tvi);

        if (tvi.state & (TVIS_EXPANDED | TVIS_EXPANDEDONCE))
            return FALSE;

        _ExpandByNewThread(hItem, nCode);

        return TRUE;
    }

    static void SpeedUp()
    {
        DWORD dwThreadId = 0;
        HANDLE hThread = ::CreateThread(
            NULL, 0,
            _SpeedUpThread,
            NULL, 0, &dwThreadId
            );
        ::CloseHandle(hThread);
    }

protected:

    struct __EXPAND_THREAD_PARAM
    {
        CKuiFolderBrowseCtrl *pThis;
        HWND        hWnd;
        HTREEITEM   hItem;
//         UINT        nCode;
        BOOL        bCheck;
        CString     strFullPath;
    };

    CImageList m_wndIconList;
    __MapItemToPath m_mapItemToPath;
    HTREEITEM m_hItemMyComputer;

    static DWORD WINAPI _SpeedUpThread(LPVOID pvParam)
    {
        SHFILEINFO sfi;
        size_t size = sizeof(SHFILEINFO);

        CIidl iidl(CSIDL_DESKTOP), iidlDrives(CSIDL_DRIVES);

        ::SHGetFileInfo(
            (LPCTSTR)(LPITEMIDLIST)iidl, 0,
            &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_SMALLICON);

        CIidl iidlChild, iidlFull;

        IShellFolder *piFolder = iidlDrives.GetShellFolder();
        if (NULL == piFolder)
            goto Exit0;

        IEnumIDList *piEnum = NULL;
        HRESULT hRet = piFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN, &piEnum);
        if (FAILED(hRet))
            goto Exit0;

        while (TRUE)
        {
            hRet = piEnum->Next(1, iidlChild, NULL);
            if (S_OK != hRet)
                break;

            iidlFull = iidlDrives.Append(iidlChild);

            SHFILEINFO   sfi;
            DWORD_PTR dwRet = ::SHGetFileInfo(
                (LPCTSTR)(LPITEMIDLIST)iidlFull,
                0,
                &sfi,
                sizeof(SHFILEINFO),
                SHGFI_ICON | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME);

//             if (0 == dwRet)
//                 continue;
//
//             if (SFGAO_FILESYSTEM == (SFGAO_FILESYSTEM & sfi.dwAttributes))
//             {
//                 CString strPath;
//
//                 BOOL bRet = ::SHGetPathFromIDList(iidlFull, strPath.GetBuffer(MAX_PATH + 1));
//                 if (bRet)
//                 {
//                     strPath.ReleaseBuffer();
//                     strPath.TrimRight(L'\\');
//                 }
//                 else
//                     strPath.ReleaseBuffer(0);
//             }
        }

Exit0:

        if (piEnum)
        {
            piEnum->Release();
            piEnum = NULL;
        }

        if (piFolder)
        {
            piFolder->Release();
            piFolder = NULL;
        }

        return 0;
    }

    void _ExpandByNewThread(HTREEITEM hItem, UINT nCode)
    {
        __EXPAND_THREAD_PARAM *pThreadParam;
        CString strFullPath;

        BOOL bRet = _GetFullPath(hItem, strFullPath);
        if (!bRet)
            return;

        pThreadParam = new __EXPAND_THREAD_PARAM;
        pThreadParam->hWnd      = m_hWnd;
        pThreadParam->hItem     = hItem;
//         pThreadParam->nCode     = nCode;
        pThreadParam->bCheck    = GetCheckState(hItem);
        pThreadParam->strFullPath   = strFullPath;

        DWORD dwThreadId = 0;
        HANDLE hThread = ::CreateThread(
            NULL, 0,
            _ExpandThreadProc,
            pThreadParam, 0, &dwThreadId
            );
        ::CloseHandle(hThread);
    }

    HRESULT _Init()
    {
        HRESULT hRet = E_FAIL;

        DWORD dwTickCountFrom = ::GetTickCount(), dwTickCountTo = 0;

        hRet = _InitIconList();
        if (S_OK != hRet)
            return hRet;

        dwTickCountTo = ::GetTickCount();
        dwTickCountFrom = dwTickCountTo;

        SetImageList(m_wndIconList, TVSIL_NORMAL);

        HTREEITEM hRootItem = _InsertItem(CSIDL_DESKTOP, TVI_ROOT);

        SetItemState(
            hRootItem,
            INDEXTOSTATEIMAGEMASK(CHECK_STATE_NONE) | TVIS_EXPANDED | TVIS_EXPANDEDONCE,
            TVIS_STATEIMAGEMASK | TVIS_EXPANDED | TVIS_EXPANDEDONCE);

        m_hItemMyComputer = _InsertItem(CSIDL_DRIVES, hRootItem);
        HTREEITEM hPersonalItem = _InsertItem(CSIDL_PERSONAL, hRootItem);

        CString strDesktopPath, strPersonalPath;
        BOOL bRet = ::SHGetSpecialFolderPath(NULL, strDesktopPath.GetBuffer(MAX_PATH + 1), CSIDL_DESKTOPDIRECTORY, FALSE);
        if (!bRet)
        {
            strDesktopPath.ReleaseBuffer(0);

            _Uninit();

            return E_FAIL;
        }

        strDesktopPath.ReleaseBuffer();

        m_mapItemToPath[hRootItem] = strDesktopPath;

        dwTickCountTo = ::GetTickCount();
        dwTickCountFrom = dwTickCountTo;

        _ExpandByNewThread(hRootItem, TVE_EXPAND);
        //_InsertSubFolders(hRootItem, strDesktopPath);

        dwTickCountTo = ::GetTickCount();
        dwTickCountFrom = dwTickCountTo;

        bRet = ::SHGetSpecialFolderPath(NULL, strPersonalPath.GetBuffer(MAX_PATH + 1), CSIDL_PERSONAL, FALSE);
        if (!bRet)
        {
            strPersonalPath.ReleaseBuffer(0);

            return E_FAIL;
        }
        else
        {
            strPersonalPath.ReleaseBuffer();

            m_mapItemToPath[hPersonalItem] = strPersonalPath;
        }

        SetItemState(m_hItemMyComputer, TVIS_EXPANDED | TVIS_EXPANDEDONCE, TVIS_EXPANDED | TVIS_EXPANDEDONCE);

        dwTickCountTo = ::GetTickCount();
        dwTickCountFrom = dwTickCountTo;

//         CIidl iidlDrives(CSIDL_DRIVES);
//
//         _InsertDrives(m_hItemMyComputer, iidlDrives);
        DWORD dwThreadId = 0;
        HANDLE hThread = ::CreateThread(
            NULL, 0,
            _ExpandDrivesThreadProc,
            this, 0, &dwThreadId
            );
        ::CloseHandle(hThread);

        dwTickCountTo = ::GetTickCount();
        dwTickCountFrom = dwTickCountTo;

        return S_OK;
    }

    HRESULT _Uninit()
    {
        m_mapItemToPath.RemoveAll();

        if (m_wndIconList.m_hImageList)
        {
            m_wndIconList.Detach();
        }

        m_hItemMyComputer = NULL;

        return S_OK;
    }

    HRESULT _InitIconList()
    {
        SHFILEINFO sfi;
        size_t size = sizeof(SHFILEINFO);
        HRESULT hRet = E_FAIL;

        if (NULL != m_wndIconList.m_hImageList)
            return S_FALSE;

        CIidl iidl(CSIDL_DESKTOP);

        m_wndIconList.Attach((HIMAGELIST)::SHGetFileInfo(
            (LPCTSTR)(LPITEMIDLIST)iidl, 0,
            &sfi, sizeof(sfi),
            SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_SMALLICON)
            );

        m_wndIconList.SetBkColor(CLR_NONE);

        return (NULL != m_wndIconList.m_hImageList) ? S_OK : E_FAIL;
    }

    HTREEITEM _InsertItem(int nFolder, HTREEITEM hParent = TVI_ROOT)
    {
        HTREEITEM hItem = NULL;
        HRESULT hrRet = E_FAIL;

        SHFILEINFO sfi = { 0 };

        hItem = _InsertItem(CIidl(nFolder), hParent);

        return hItem;
    }

    HTREEITEM _InsertItem(LPITEMIDLIST pidl, HTREEITEM hParent = TVI_ROOT)
    {
	    SHFILEINFO sfi;
	    TVINSERTSTRUCT tvis;
	    HTREEITEM hItem = NULL;
        TCHAR szRootPath[MAX_PATH] = { 0 };

	    ATLASSERT(pidl != NULL);

	    ZeroMemory(&sfi, sizeof(sfi));

	    ::SHGetFileInfo(
		    (LPCTSTR) pidl, 0,
		    &sfi, sizeof(sfi),
		    SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME | SHGFI_PIDL | SHGFI_ATTRIBUTES
	        );
	    if (0 == sfi.dwAttributes)
		    goto Exit0;

	    ZeroMemory(&tvis, sizeof(tvis));

        tvis.hParent = hParent;
	    tvis.hInsertAfter = TVI_LAST;
	    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	    tvis.item.pszText = sfi.szDisplayName;
	    tvis.item.iImage = sfi.iIcon;
        tvis.item.iSelectedImage = sfi.iIcon;

	    if (sfi.dwAttributes & SFGAO_HASSUBFOLDER)
		    tvis.item.cChildren = TRUE;

        hItem = __super::InsertItem(&tvis);

    Exit0:

	    return hItem;
    }

    struct __InsertItemWParam
    {
        HTREEITEM hParent;
        LPTSTR    lpszText;
        int       nImage;
        BOOL      bHasSubFolder;
//         BOOL      bCheck;
        LPCTSTR   lpszPath;
    };

    static BOOL _InsertItem(HWND hWndTree, HTREEITEM hParent, LPTSTR lpszText, int nImage, BOOL bHasSubFolder/*, BOOL bCheck*/, LPCTSTR lpszPath = NULL)
    {
        BOOL bRet = FALSE;
        __InsertItemWParam wParam;

        wParam.hParent          = hParent;
        wParam.lpszText         = lpszText;
        wParam.nImage           = nImage;
        wParam.bHasSubFolder    = bHasSubFolder;
//         wParam.bCheck           = bCheck;
        wParam.lpszPath         = lpszPath;

        if (::IsWindow(hWndTree))
            bRet = ::SendMessage(hWndTree, MSG_KUIFBC_INSERT_ITEM, (WPARAM)&wParam, 0) ? TRUE : FALSE;

        return bRet;
    }

    static DWORD WINAPI _ExpandDrivesThreadProc(LPVOID lpParam)
    {
        CKuiFolderBrowseCtrl *pThis = (CKuiFolderBrowseCtrl *)(lpParam);

        CIidl iidlDrives(CSIDL_DRIVES);

        _InsertDrives(pThis->m_hWnd, pThis->m_hItemMyComputer, iidlDrives);

        return TRUE;
    }

    static BOOL _InsertDrives(HWND hWnd, HTREEITEM hParent, LPITEMIDLIST pidlParent)
    {
        CIidl iidlParent, iidlChild, iidlFull;

        iidlParent.Attach(pidlParent);

        IShellFolder *piFolder = iidlParent.GetShellFolder();
        if (NULL == piFolder)
            goto Exit0;

        IEnumIDList *piEnum = NULL;
        HRESULT hRet = piFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN, &piEnum);
        if (FAILED(hRet))
            goto Exit0;

        while (TRUE)
        {
            hRet = piEnum->Next(1, iidlChild, NULL);
            if (S_OK != hRet)
                break;

            iidlFull = iidlParent.Append(iidlChild);

            SHFILEINFO   sfi;
            DWORD_PTR dwRet = ::SHGetFileInfo(
                (LPCTSTR)(LPITEMIDLIST)iidlFull,
                0,
                &sfi,
                sizeof(SHFILEINFO),
                SHGFI_ICON | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME);

            if (0 == dwRet)
                continue;

            if (SFGAO_FILESYSTEM == (SFGAO_FILESYSTEM & sfi.dwAttributes))
            {
                CString strPath;

                BOOL bRet = ::SHGetPathFromIDList(iidlFull, strPath.GetBuffer(MAX_PATH + 1));
                if (bRet)
                {
                    strPath.ReleaseBuffer();
                    strPath.TrimRight(L'\\');
                }
                else
                    strPath.ReleaseBuffer(0);

                _InsertItem(
                    hWnd, hParent, sfi.szDisplayName, sfi.iIcon,
                    SFGAO_HASSUBFOLDER == (sfi.dwAttributes & SFGAO_HASSUBFOLDER),
                    /*FALSE, */strPath);

/*
                TVINSERTSTRUCT tvis;

                ZeroMemory(&tvis, sizeof(tvis));

                tvis.hParent = hParent;
                tvis.hInsertAfter = TVI_LAST;
                tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

                tvis.item.pszText = sfi.szDisplayName;
                tvis.item.iImage = sfi.iIcon;
                tvis.item.iSelectedImage = sfi.iIcon;

                if (sfi.dwAttributes & SFGAO_HASSUBFOLDER)
                    tvis.item.cChildren = TRUE;

                HTREEITEM hItem = __super::InsertItem(&tvis);
                m_mapItemToPath[hItem] = strPath;
*/

            }
        }

Exit0:

        if (piEnum)
        {
            piEnum->Release();
            piEnum = NULL;
        }

        if (piFolder)
        {
            piFolder->Release();
            piFolder = NULL;
        }

        iidlParent.Detach();

        return TRUE;
    }

    static BOOL _InsertSubFolders(HWND hWnd, HTREEITEM hParent, LPCTSTR lpszPath, BOOL bCheck)
    {
        SHFILEINFOW sfi = {0};
        DWORD_PTR dwRet = 0;

        WIN32_FIND_DATA findData;
        HANDLE hFind = INVALID_HANDLE_VALUE;

        CString strPath = lpszPath, strFileName;

        if (strPath[strPath.GetLength() - 1] != L'\\')
            strPath += L'\\';

        hFind = ::FindFirstFile(strPath + L"*.*", &findData);
        if (hFind)
        {
            do
            {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    continue;

                if (L'.' == findData.cFileName[0] && (L'\0' == findData.cFileName[1] || (L'.' == findData.cFileName[1] && L'\0' == findData.cFileName[2])))
                    continue;

                strFileName = strPath + findData.cFileName;

                dwRet = ::SHGetFileInfo(
                    strFileName,
                    INVALID_FILE_ATTRIBUTES,
                    &sfi,
                    sizeof(sfi),
                    SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX);

                if (0 == dwRet || (SFGAO_LINK == (sfi.dwAttributes & SFGAO_LINK)))
                    continue;

//                 TVINSERTSTRUCT tvis;
//                 ZeroMemory(&tvis, sizeof(tvis));
//
//                 tvis.hParent = hParent;
//                 tvis.hInsertAfter = TVI_LAST;
//                 tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_STATE;
//                 tvis.item.pszText = findData.cFileName;//sfi.szDisplayName;
//                 tvis.item.iImage = sfi.iIcon;
//                 tvis.item.iSelectedImage = sfi.iIcon;
//                 tvis.item.state = INDEXTOSTATEIMAGEMASK(bCheck ? CHECK_STATE_CHECKED : CHECK_STATE_UNCHECKED);
//                 tvis.item.stateMask = TVIS_STATEIMAGEMASK;
//
//                 if (sfi.dwAttributes & SFGAO_HASSUBFOLDER)
//                     tvis.item.cChildren = TRUE;
//
//                 InsertItem(&tvis);

                _InsertItem(
                    hWnd, hParent, findData.cFileName, sfi.iIcon,
                    SFGAO_HASSUBFOLDER == (sfi.dwAttributes & SFGAO_HASSUBFOLDER)
                    /*, bCheck*/);

            } while (::FindNextFile(hFind, &findData));

            ::FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }

        return TRUE;
    }

    static BOOL _Expand(HWND hWnd, HTREEITEM hItem, LPCTSTR lpszFullPath, BOOL bCheck)
    {
        //__super::Expand(hItem, nCode);
//         CString strFullPath;
//
//         BOOL bRet = _GetFullPath(hItem, strFullPath);
//         if (!bRet)
//             return FALSE;

        BOOL bRet = FALSE;

        if (::IsWindow(hWnd))
        {
            TVITEM item = { 0 };
            item.hItem          = hItem;
            item.mask           = TVIF_STATE;
            item.pszText        = NULL;
            item.iImage         = 0;
            item.iSelectedImage = 0;
            item.state          = TVIS_EXPANDED | TVIS_EXPANDEDONCE;
            item.stateMask      = TVIS_EXPANDED | TVIS_EXPANDEDONCE;
            item.lParam         = NULL;

            ::SendMessage(hWnd, TVM_SETITEM, 0, (LPARAM)&item);

            bRet = _InsertSubFolders(hWnd, hItem, lpszFullPath, bCheck);
        }

        return bRet;
    }

    BOOL _GetFullPath(HTREEITEM hItem, CString& strFullPath)
    {
        CString strPath;
        strFullPath.Empty();

        while (TVI_ROOT != hItem)
        {
            const __MapItemToPath::CPair *pPair = m_mapItemToPath.Lookup(hItem);

            if (NULL != pPair)
            {
                    strFullPath = pPair->m_value + L'\\' + strFullPath;
                    return TRUE;
            }

            GetItemText(hItem, strPath);

            if (strFullPath.IsEmpty())
                strFullPath = strPath;
            else
                strFullPath = strPath + L'\\' + strFullPath;

            hItem = GetParentItem(hItem);
        }

        return FALSE;
    }

    static DWORD WINAPI _ExpandThreadProc(LPVOID lpParam)
    {
        __EXPAND_THREAD_PARAM *pThreadParam;

        pThreadParam = (__EXPAND_THREAD_PARAM *)lpParam;

//         CKuiFolderBrowseCtrl *pThis = (CKuiFolderBrowseCtrl *)(pThreadParam->pThis);

        _Expand(pThreadParam->hWnd, pThreadParam->hItem, pThreadParam->strFullPath, pThreadParam->bCheck);

        delete pThreadParam;

        return TRUE;
    }

    void OnDestroy()
    {
        _Uninit();
    }

    LRESULT OnTVNItemExpanding(int n, LPNMHDR pnmh, BOOL &bHandled)
    {
        LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

        if (TVE_COLLAPSE == pnmtv->action)
            return FALSE;

        return Expand(pnmtv->itemNew.hItem, pnmtv->action);
    }

    LRESULT OnKuiFBCInsertItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
    {
        __InsertItemWParam *pwParam = (__InsertItemWParam *)wParam;

        if (NULL == pwParam)
            return FALSE;

        TVINSERTSTRUCT tvis;

        ZeroMemory(&tvis, sizeof(tvis));

        tvis.hParent = pwParam->hParent;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_STATE;
        tvis.item.state = INDEXTOSTATEIMAGEMASK(GetCheckState(pwParam->hParent) ? CHECK_STATE_CHECKED : CHECK_STATE_UNCHECKED);
        tvis.item.stateMask = TVIS_STATEIMAGEMASK;
        tvis.item.pszText = pwParam->lpszText;
        tvis.item.iImage = pwParam->nImage;
        tvis.item.iSelectedImage = pwParam->nImage;
        tvis.item.cChildren = pwParam->bHasSubFolder;

        HTREEITEM hItem = __super::InsertItem(&tvis);

        if (pwParam->lpszPath)
            m_mapItemToPath[hItem] = pwParam->lpszPath;

        return TRUE;
    }

public:

    BEGIN_MSG_MAP_EX(CKuiFolderBrowseCtrl)
        CHAIN_MSG_MAP(CWH3StateCheckTreeCtrl)
        MSG_WM_DESTROY(OnDestroy)
        MESSAGE_HANDLER_EX(MSG_KUIFBC_INSERT_ITEM, OnKuiFBCInsertItem)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnTVNItemExpanding)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};

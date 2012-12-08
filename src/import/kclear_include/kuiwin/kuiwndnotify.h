#pragma once

//////////////////////////////////////////////////////////////////////////
// Notify Messages For KuiWin User

#define MSG_KUI_NOTIFY(ctrlid) NOTIFY_ID_HANDLER_EX(ctrlid, OnKuiNotify_##ctrlid)

#define KUI_NOTIFY_MAP(ctrlid)                   \
    LRESULT OnKuiNotify_##ctrlid(LPNMHDR pnmh)   \
    {                                           \
        UINT_PTR  uCode = pnmh->code;           \

#define KUI_NOTIFY_MAP_END()                     \
        SetMsgHandled(FALSE);                   \
        return FALSE;                           \
    }                                           \

#define KUI_CHAIN_NOTIFY_MAP(ChainClass, ctrlid)                 \
        if(ChainClass::OnKuiNotify_##ctrlid(pnmh))               \
            return TRUE;                                        \

// Command Notify
#define KUINM_COMMAND  1

// void OnKuiItemCommand()
#define KUI_NOTIFY_ID_COMMAND(itemid, func)                                  \
    if (KUINM_COMMAND == uCode && itemid == ((LPKUINMCOMMAND)pnmh)->uItemID)  \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \

// void OnKuiCommand(UINT uItemID, LPCSTR szItemClass)
#define KUI_NOTIFY_COMMAND(func)                                                     \
    if (KUINM_COMMAND == uCode)                                                      \
    {                                                                               \
        func(((LPKUINMCOMMAND)pnmh)->uItemID, ((LPKUINMCOMMAND)pnmh)->szItemClass);   \
        return TRUE;                                                                \
    }                                                                               \

typedef struct _KUINMCOMMAND
{
    NMHDR       hdr;
    UINT        uItemID;
    LPCSTR      szItemClass;
} KUINMCOMMAND, *LPKUINMCOMMAND;

// Real Window Resized Notify
#define KUINM_REALWND_RESIZED 2

// void OnKuiRealWndResized(CRect rcWnd)
#define KUI_NOTIFY_REALWND_RESIZED(itemid, func)                                             \
    if (KUINM_REALWND_RESIZED == uCode && itemid == ((LPKUINMREALWNDRESIZED)pnmh)->uItemID)   \
    {                                                                                       \
        func(((LPKUINMREALWNDRESIZED)pnmh)->rcWnd);                                          \
        return TRUE;                                                                        \
    }                                                                                       \

typedef struct _KUINMREALWNDRESIZED
{
    NMHDR       hdr;
    UINT        uItemID;
    CRect       rcWnd;
} KUINMREALWNDRESIZED, *LPKUINMREALWNDRESIZED;

// Mouse Hover Notify
// void OnKuiMouseHover()
#define KUINM_MOUSEHOVER 3

#define KUI_NOTIFY_MOUSEHOVER(func)                                          \
    if (KUINM_MOUSEHOVER == uCode)                                           \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \

// Mouse Leave Notify
// void OnKuiMouseLeave()
#define KUINM_MOUSELEAVE 4

#define KUI_NOTIFY_MOUSELEAVE(func)                                          \
    if (KUINM_MOUSELEAVE == uCode)                                           \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \

// Tab Sel Change Notify
#define KUINM_TAB_SELCHANGE  5

typedef struct _KUINMTABSELCHANGE
{
    NMHDR       hdr;
    UINT        uTabID;
    UINT        uTabItemIDNew;
    UINT        uTabItemIDOld;
    BOOL        bCancel;
} KUINMTABSELCHANGE, *LPKUINMTABSELCHANGE;

// BOOL OnKuiTabSelChange(int nTabItemIDOld, int nTabItemIDNew)
#define KUI_NOTIFY_TAB_SELCHANGE(tabid, func)                                        \
    if (KUINM_TAB_SELCHANGE == uCode && tabid == ((LPKUINMTABSELCHANGE)pnmh)->uTabID) \
    {                                                                               \
        BOOL bRet = func(                                                           \
            ((LPKUINMTABSELCHANGE)pnmh)->uTabItemIDOld,                              \
            ((LPKUINMTABSELCHANGE)pnmh)->uTabItemIDNew);                             \
        if (!bRet)                                                                  \
            ((LPKUINMTABSELCHANGE)pnmh)->bCancel = TRUE;                             \
        return TRUE;                                                                \
    }                                                                               \


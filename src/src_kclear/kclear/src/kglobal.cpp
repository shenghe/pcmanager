#include "stdafx.h"
#include "kglobal.h"

BOOL g_bStop[TRASHCLASS] = {
    FALSE, //普通清理
    FALSE,  //一件清理
    FALSE   //体检
};

HWND g_hWndKClear = NULL;
//////////////////////////////////////////////////////////////////////////
// Create Author: zhang zexin
// Create Date: 2010-12-16
// Description: 
// Modify Record:
//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////
// 瘦身方法
typedef enum tagSlimMethod {
    SLIM_DELETE_FILE = 0,
    SLIM_COMPRESS_FILE = 1,
} SlimMethod;

//////////////////////////////////////////////////////////////////////////
// 系统盘瘦身回调(扫描,删除,压缩)
struct ISystemSlimCallBack
{
    // 扫描某个具体的文件
    virtual BOOL OnScanItem(
        const wchar_t* szFilePath,
        ULONGLONG qwFileSizeOnDisk,
        BOOL bCompressed
        ) = 0;

    // 处理某个具体的文件
    virtual BOOL OnBeginProcessItem(
        const wchar_t* szFilePath
        ) = 0;

    virtual BOOL OnEndProcessItem(
        const wchar_t* szFilePath,
        ULONGLONG qwSavedSize
        ) = 0;

    // 空闲和控制退出
    virtual BOOL OnIdle(
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

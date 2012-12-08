#ifndef KCLEAR_SLIMHELPER_H_
#define KCLEAR_SLIMHELPER_H_

//////////////////////////////////////////////////////////////////////////

#include "slimcallback.h"

//////////////////////////////////////////////////////////////////////////

namespace slimhelper
{
    // 瘦身标记文件
    extern const TCHAR g_kSlimFlag[];

    // 获取安全特权
    BOOL GainSecurityPrivilege1();
    BOOL GainSecurityPrivilege2();

    // 文件是否被压缩
    BOOL IsCompressed(const CString& strFilePath, BOOL& bCompressed);

    // 压缩单个文件
    BOOL CompressFile(const CString& strFilePath,
                      ULONGLONG qwFileSize,
                      DWORD dwFileAttributes,
                      ISystemSlimCallBack* piCallback);

    // 压缩一个目录
    BOOL CompressDirectory(const CString& strDirectory, 
                           ISystemSlimCallBack* piCallback,
                           int nLevel = 0);

    // 删除单个文件
    BOOL DeleteFile(const CString& strFilePath,
                    ULONGLONG qwFileSize,
                    DWORD dwFileAttributes,
                    ISystemSlimCallBack* piCallback);

    // 删除一个目录
    BOOL DeleteDirectory(const CString& strDirectory,
                         ISystemSlimCallBack* piCallback,
                         BOOL bKeepRootDir = TRUE,
                         int nLevel = 0);

    // 删除一个路径到回收站
    BOOL RecyclePath(const CString& strFilePath,
                     BOOL bKeepRootDir = TRUE);

    // 扫描当个文件
    BOOL ScanFile(const CString& strFilePath,
                  ULONGLONG qwFileSize,
                  DWORD dwFileAttributes,
                  ISystemSlimCallBack* piCallback);

    // 扫描一个目录
    BOOL ScanDirectory(const CString& strDirectory,
                       ISystemSlimCallBack* piCallback);

    // 数值转成字符型的大小
    void GetFileSizeTextString(__int64 fNum, CString& strFileSize);

    // 打开文件夹
    BOOL LocateFile(const CString& strFilePath);
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_SLIMHELPER_H_

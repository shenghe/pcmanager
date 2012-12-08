#pragma once

//////////////////////////////////////////////////////////////////////////

// 卷信息
typedef struct tagVolInfo {
    CHAR cVol;
    CString strVolName;
    ULONGLONG qwTotalSize;  // 总大小
    ULONGLONG qwFreeSize;   // 剩余大小
    BOOL bRemovable;
} VolInfo;

// 文件信息
typedef struct tagBigFileInfo {
    CString strFilePath;
    CString strExt;
    ULONGLONG qwFileSize;

    bool operator < (const struct tagBigFileInfo& fileInfo) const
    {
        return qwFileSize < fileInfo.qwFileSize;
    }

    bool operator==(const CString& strFilePath) const {
        if (strFilePath.CompareNoCase(this->strFilePath) == 0)
            return true;
        return false;
    }  

    bool Equal(const CString& strFilePath) const
    {
        if (strFilePath.CompareNoCase(this->strFilePath) == 0)
            return true;
        return false;
    }

} BigFileInfo;

typedef enum tagEnumState {
    enumES_Notstart = 0,    // 没有开始
    enumES_Scanning = 1,    // 正在枚举
    enumES_Canceled = 2,    // 被取消
    enumES_Finished = 3,    // 正常枚举结束
    enumES_Cache = 4,       // 旧的数据
} EnumState;

//////////////////////////////////////////////////////////////////////////

struct IEnumFileback {
    //virtual void Lock() = 0;
    //virtual void UnLock() = 0;
    virtual void BeginEnum() = 0;
    virtual void EndEnum(BOOL bCanceled) = 0;
    virtual BOOL OnFindFile(const BigFileInfo& strFilePath) = 0;
};

//////////////////////////////////////////////////////////////////////////

namespace bigfilehelper
{
    // 显示文件属性
    void ShowFileProperties(LPCTSTR lpFilePath);

    // 定位文件
    BOOL LocateFile(const CString& strFilePath);

    // 删除文件
    BOOL Recycle(LPCTSTR pszPath, BOOL bDelete = FALSE);

    BOOL BaiduFileinfo(LPCTSTR pszPath);

    // 获得系统分区的盘符(eg. C)
    CHAR GetSystemDrive();

    // 获得系统盘盘符(eg. C:\)
    BOOL GetSystemDrive(CString& strDrv);

    // 自动获得首先得迁移目标目录
    BOOL GetAutoDestDirFromSize(CString& strDir);

    // 获得文件大小的字符串
    void GetFileSizeString(ULONGLONG qwFileSize, CString& strFileSize);

    // 获得短扩展名
    void GetShortExtString(CString& strExt);

    // 递归枚举文件
    // in 路径需要\结尾
    // out 接收结果的对象
    // in 退出信号
    BOOL EnumFiles(const CString& strPath, IEnumFileback* piEnumFileback);

    // 获得扩展名
    void GetFileExt(const CString& strFilePath, CString& strExt);

    // 在指定的目录里面是否有文件
    BOOL HasFileInDir(const CString& strPath);

    // 获得桌面的路径
    BOOL GetDesktopPath(CString& strPath);

    // 获得我的文档的路径
    BOOL GetDocumentPath(CString& strPath);

    // 获得Chrome的下载目录
    BOOL GetChromeDownloadDirs(std::vector<CString>& vDirs);

    // 获得Firefox的下载目录
    BOOL GetFirefoxDownloadDirs(std::vector<CString>& vDirs);

    // 获得360浏览器的下载目录
    BOOL Get360DownloadDirs(std::vector<CString>& vDirs);

    // 获得世界之窗的下载目录
    BOOL GetTheworldDownloadDirs(std::vector<CString>& vDirs);

    // 获得马桶的下载目录
    BOOL GetMaxmonthDownloadDirs(std::vector<CString>& vDirs);

    // 获得FlashGet的下载目录
    BOOL GetFlashGetDownloadDirs(std::vector<CString>& vDirs);

    // 获得迅雷的下载目录
    BOOL GetXunLeiDownloadDirs(std::vector<CString>& vDirs);

    // 获得比特彗星的下载目录
    BOOL GetBitCometDownloadDirs(std::vector<CString>& vDirs);

    // 获得QQ旋风的下载目录
    BOOL GetQQDownLoadDownloadDirs(std::vector<CString>& vDirs);

    // 获得所有的下载目录
    BOOL GetAllDownloadDirs(std::vector<CString>& vDirs);

    // 获得所有固定和可移动磁盘的卷
    BOOL GetAllVols(std::vector<VolInfo>& vVols);

    // 压缩长路径到短路径
    void CompressedPath(CString& strFilePath);

    // 获得文件的图标
    HICON GetIconFromFilePath(const CString& strFilePath);
}

//////////////////////////////////////////////////////////////////////////

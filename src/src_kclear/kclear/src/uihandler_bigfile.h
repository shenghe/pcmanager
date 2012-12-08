#pragma once

//////////////////////////////////////////////////////////////////////////

#include "kscbase/ksclock.h"
#include "kclear/ifcache.h"
#include "bigfilehelper.h"
#include "bigfilelistctrl.h"
#include "vdirctrl.h"
#include "bigfilelistwrapper.h"
#include "kclear/libheader.h"

//////////////////////////////////////////////////////////////////////////

// 扩展名信息
typedef struct tagBigExtInfo {
    CString strExt;
    ULONGLONG qwTotalCount;
    ULONGLONG qwTotalSize;
    UINT nPercent;

    bool operator < (const struct tagBigExtInfo& extInfo) const
    {
        return qwTotalSize < extInfo.qwTotalSize;
    }

} BigExtInfo;

typedef enum tagVirtualDirType {
    enumVDT_Dir = 0,        // 桌面或者下载
    enumVDT_Volume = 1,     // 卷
} VirtualDirType;

typedef enum tagRealDirType {
    enumRDT_Desktop = 0,    // 我的桌面
    enumRDT_Document = 1,   // 我的文档
    enumRDT_Download = 2,   // 下载目录
    enumRDT_Other,
} RealDirType;

//////////////////////////////////////////////////////////////////////////

typedef enum tagBigFileTimerId {
    enumBFTI_Animation = 0, // 动画
    enumBFTI_ScanPath = 1,
} BigFileTimerId;

//////////////////////////////////////////////////////////////////////////

// 使用WM_COMMAND发生的内部ID
typedef enum tagBigFileInternalId {
    enumBFII_ScanEnd = 0,   // 扫描结束
    enumBFII_CtrlId = 888,  // ID
} BigFileInternalId;

//////////////////////////////////////////////////////////////////////////

#include "kclearmsg.h"

//////////////////////////////////////////////////////////////////////////
class CBigfileFilter
{
public:
    static CBigfileFilter& Instance();
    BOOL LoadFilter();
    BOOL IsFileInFilter(const CString& strFilepath);
   
private:
    CBigfileFilter();
    ~CBigfileFilter();
   
private:
    std::vector<CString> m_vsysExt;
    std::vector<CString> m_vsysFile;
    std::vector<CString> m_vsysDir;
};


// 虚拟目录管理器
class CVirtualDirManager 
    : public IFCacheQueryback
    , public IEnumFileback
{
public:
    CVirtualDirManager();
    virtual ~CVirtualDirManager();

    void SetDirType(VirtualDirType nType);
    VirtualDirType GetDirType();

    void SetRealDirType(RealDirType nType);
    RealDirType GetRealDirType();

    void SetName(const CString& strName);
    CString GetName();

    void SetVolumeChar(CHAR cVol);
    CHAR GetVolumeChar();
    void UpdateVolumeSizeInfo();

    void SetSysVolume();
    BOOL IsSysVolume();

    void SetRemovable(BOOL bRemovable);

    void GetStatus(CString& strStatus);

    void GetCacheList(std::vector<BigFileInfo>& vCacheList, size_t nLimit, size_t& nMore);
    void CopyTopListToCacheList();

    // 获得Top类型榜
    void GetTopExts(std::vector<BigExtInfo>& vTopExts);

    // 获得Top100的大文件
    void GetTop100Files(std::vector<BigFileInfo>& vTopFiles);
    void GetTop100Size(ULONGLONG& qwSize);

    // 按照过滤条件进行查询
    void QueryFilesWithFilter(
        std::vector<BigFileInfo>& vFiles, 
        const CString& strExt, 
        ULONG dwSizeLimit
        );

    void SetNotify(HWND hWnd);
    void OnFileListSize(CRect rcZone);
    HWND InitCtrl(HWND hParent);        // 创建控件
    CBigFileListCtrl& GetCtrl();

    void SetSizeFilter(int idx);
    int GetSizeFilter();
    void SetSizeFilterEnable(BOOL fEnable);
    void SetSysFilterEnable(BOOL bEnable);
    BOOL GetSysFilterEnable();
    BOOL GetSizeFilterEnable();
    void SetExtFilter(const CString& strExt);
    void GetExtFilter(CString& strExt);
    void SetListLimit(DWORD dwLimit);

    BOOL FillListCtrl();    // 根据过滤条件填充过滤列表
    void CleanBigFileList();

    BOOL LoadCacheASync();
    BOOL LoadCacheSync();      // 从数据库加载Top100数据
    static UINT WINAPI LoadCacheThread(LPVOID pParam);
    void GetCacheSummary(CString& strSummary);

    BOOL GetBigFileInfo(size_t idx, BigFileInfo& fileInfo);
    BOOL DeleteBigFile(const CString& strFilePath);

    BOOL StartScan();       // 开始扫描
    BOOL StopScan();        // 停止扫描
    BOOL IsScanning();      // 是否正在扫描
    void GetScanningFile(CString& strFilePath, int& nPos); // 获得正在枚举的文件

    BOOL StartFillList();   // 开始填充List
    BOOL StopFillList();    // 停止填充List
    BOOL IsFilling();       // 是否正在填充

    CVirtualDirManager& FindParentVirtualDir();

    static UINT WINAPI ScanThread(LPVOID pParam);
    static UINT WINAPI FillThread(LPVOID pParam);

    static BOOL ShowFileProperties(const CString& strFilePath);

    // 实现查询结果的callback
    virtual void OnData(
        FCacheQueryType nQueryType,
        LPCWSTR lpFilePath,
        ULONGLONG qwFileSize
        );

    virtual void OnExtData(
        LPCWSTR lpExt,
        ULONGLONG qwSize,
        ULONGLONG qwCount
        );

    virtual void BeginEnum();
    virtual BOOL OnFindFile(const BigFileInfo& fileInfo);
    virtual void EndEnum(BOOL bCanceled);
    EnumState GetEnumState();

    BOOL IsFilterFile(const CString& strFile);

//protected:
    IFCache* GetFCache();
    BOOL GenerateCacheForTopExts();             // 扫描完成时生成Top榜的缓存(比较耗时)
    BOOL GenerateCacheForTop100();              // 扫描完成时生成Top500的缓存(比较耗时)
    BOOL GetSummaryInfo();                      // 扫描完成时生成总大小和总数目的统计(比较耗时)
    void GetEnumPaths(std::vector<CString>& vEnumPaths);    // 获得需要枚举的目录

//protected:
    KLock m_lock;
    KLock m_lockStatus;                         // 访问扫描路径专用
    std::vector<BigFileInfo> m_vTopFiles;       // 存放Top500
    std::vector<BigExtInfo> m_vTopExts;         // 存放Top扩展名
    HWND m_hNotifyWnd;                          // 通知窗体
    CString m_strName;                          // 显示名称
    CHAR m_cVolume;                             // 所在卷
    VirtualDirType m_nType;                     // 类型
    RealDirType m_nDirType;                     // 目录类型(如果是dir类型的话)
    ULONGLONG m_qwTotalSize;                    // 总大小
    ULONGLONG m_qwTotalCount;                   // 总数目
    //BOOL m_bScanning;                         // 正在扫描
    BOOL m_bFilling;                            // 正在填充
    //HANDLE m_hScanThread;                     // 扫描线程句柄
    HANDLE m_hFillThread;                       // 填充线程句柄
    BOOL m_bStopScanFlag;                       // 停止扫描的标记
    BOOL m_bStopFillFlag;                       // 停止填充的标记
    EnumState m_nEnumState;                     // 枚举状态
    ULONGLONG m_qwEnumedSize;                   // 已经枚举了的大小
    CString m_strCurrentFile;                   // 正在枚举的文件
    BOOL m_bSysVolume;                          // 是否为系统盘
    BOOL m_bRemovable;                          // 是否为移动硬盘

    BOOL m_bSysFilter;                           //是否为系统文件

    std::vector<BigFileInfo> m_vCacheFiles;     // 上次查询结果的缓冲
    std::vector<CString> m_vDeleteCache;    // 删除Cache(用于没有数据库的情况)
    CString m_strCacheExt;                      // 上次查询的扩展名
    ULONGLONG m_qwSizeFilter;                   // 上次查询的大小限制
    BOOL m_bEnableSizeFilter;                   // 大小限制的开关
    CString m_strExtFilter;                     // 扩展名过滤限制
    IFCache* m_piFCache;                        // 数据库访问接口
    SYSTEMTIME m_CacheTime;
    BOOL m_bFullCache;

    ULONGLONG m_qwVolTotalSize;                 // 所在卷总大小
    ULONGLONG m_qwVolFreeSize;                  // 所在卷剩余空间

    CBigFileListCtrl m_ctrlFileList;            // FileList控件(跟随虚拟目录)
    DWORD m_dwFileListLimit;                    // FileList控件的数目限制
};

//////////////////////////////////////////////////////////////////////////

class CUIHandlerBigFile
{
public:
    CUIHandlerBigFile(CKscMainDlg* refDialog);
    virtual ~CUIHandlerBigFile();

    // for container
    void Init();
	void InitDelay();
    void UnInit();
    void Show();

protected:
    BOOL CreateDirectoryCtrls();
    BOOL CreateFileListCtrl();
    BOOL CreateFilterBox();
    BOOL StartBackgroundScan();

    HMENU InitFileContextMenu();
    HMENU InitExtFilterMenu();
    void OnExtFilterContext();

    BOOL LoadFCache();
    
    BOOL OnStartScan(); // 开始扫描
    BOOL OnStopScan();  // 停止扫描
    BOOL OnReScan();    // 重新扫描

    // win32控件的ReSize
    void OnDirectoryCtlReSize(CRect rcZone);
    void OnFileListSize(CRect rcZone);
    void OnComboboxReSize(CRect rcZone);

    void OnSizeLimitCheck();    // 大小限制CheckBox的勾选
    void OnSwitchVDir(size_t nItem, size_t nSubItem);
    void OnSysFilterLnk();

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        //MESSAGE_HANDLER(MSG_USER_LDLISTVIEWCTRL_LBP_URL, OnFileListViewClickUrl)
        KUI_NOTIFY_REALWND_RESIZED(ID_CTL_BIGFILE_LISTLEFT, OnDirectoryCtlReSize)
        KUI_NOTIFY_REALWND_RESIZED(ID_CTL_BIGFILE_SIZE_COMBOBOX, OnComboboxReSize)
        KUI_NOTIFY_REALWND_RESIZED(ID_CTL_BIGFILE_LISTRIGHT, OnFileListSize)
        KUI_NOTIFY_ID_COMMAND(ID_BTN_BIGFILE_ENUM_START, OnStartScan)
        KUI_NOTIFY_ID_COMMAND(ID_BTN_BIGFILE_ENUM_STOP, OnStopScan)
        KUI_NOTIFY_ID_COMMAND(ID_CHK_BIGFILE_SIZE_LIMIT, OnSizeLimitCheck)
        KUI_NOTIFY_ID_COMMAND(ID_BTN_BIGFILE_RESCAN, OnReScan)
        KUI_NOTIFY_ID_COMMAND(ID_BTN_BIGFILE_FILTER_MENU1, ShowTopExtMenu)
        KUI_NOTIFY_ID_COMMAND(ID_BTN_BIGFILE_FILTER_MENU2, ShowTopExtMenu)
        KUI_NOTIFY_ID_COMMAND(ID_TXT_BIGFILE_FILTER, ShowTopExtMenu)
        KUI_NOTIFY_ID_COMMAND(ID_CHK_SYSFILE_DISPLAY, OnSysFilterLnk)
    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CUIHandlerBigFile)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_COMMAND(OnCommand)
        MESSAGE_HANDLER(WM_BIGFILE_SCAN_END, OnScanEnd)
        MESSAGE_HANDLER(WM_BIGFILE_LIST_LINK, OnListLink)
        MESSAGE_HANDLER(WM_BIGFILE_LIST_MORE, OnListMore)
        MESSAGE_HANDLER(WM_BIGFILE_LIST_DBCLICK, OnListDBClick)
        MESSAGE_HANDLER(WM_BIGFILE_LIST_CONTEXT, OnListContext)
        // 过滤菜单
        MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
        MSG_WM_MENUSELECT(OnMenuSelect)
        MSG_WM_DRAWITEM(OnDrawItem)
        MSG_WM_MEASUREITEM(OnMeasureItem)

    END_MSG_MAP()

    // 消息响应函数
    void OnTimer(UINT_PTR nIDEvent);   // 动画定时器
    void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnInitMenuPopup(CMenu menuPopup, UINT nIndex, BOOL bSysMenu);  // 过滤菜单
    void OnMenuSelect(UINT nItemID, UINT nFlags, CMenu menu);
    void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    LRESULT OnFileListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); // FileList中Url点击
    LRESULT OnScanEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnListLink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnListMore(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnListDBClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnListContext(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    // 辅助函数
    HICON GetIconFromFileType(LPCTSTR lpExtName);   // 从扩展名获得图标

    void ShowFilterUI(BOOL bShow = TRUE);    // 显示/隐藏 过滤菜单&过滤CheckBox
    void ShowReScanButton(BOOL bShow = TRUE);      // 显示/隐藏 重新扫描按钮
    void ShowScanButton();                          // 显示扫描按钮(大)
    void ShowScanProgress();                        // 显示扫描过程
    void HideScanUI();                              // 隐藏扫描界面
    void ShowFileList(BOOL bShow = TRUE);           // 显示/隐藏 当前FileList
    BOOL ShowTopExtMenu();                          // 显示Top扩展名菜单
    void UpdateHeaderTip();                         // 刷新顶部提示条
    void UpdateFileListWithExt(int idx);            // 根据设置的过滤条件，重新刷新
    void UpdateFileSizeWithSize(int idx);           // 根据设置的过滤条件，重新刷新

    // 功能函数
    BOOL DelBigFile(int idx);
    BOOL DelBigFile(const CString& strFilePath);    // 删除大文件
    BOOL DoMoveJobs(const CString& strDest, const CString& strSource, ULONGLONG& llSize);
    BOOL DoDelJobs(const CString& strFilePath, BOOL bType);
    static BOOL SaveAutoDestDir(CString& strDir);

    CKscMainDlg* m_dlg; // 容器
    CBigFileListWrapper m_listWrapper;  // FileList的容器
    size_t m_nCurrentVirtualDir;   // 当前活动的虚拟目录
    CVirtualDirCtrl m_ctrlVirtualDirs;
    CVirtualDirManager* m_pCurrentDir;
    CComboBox m_boxSizeLimit;
    HMODULE m_hFCacheModule;
    CString m_strDestDir;   // 默认迁移目录
};

//////////////////////////////////////////////////////////////////////////

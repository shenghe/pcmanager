#ifndef KCLEAR_TRASHONEKEY_H_
#define KCLEAR_TRASHONEKEY_H_

//////////////////////////////////////////////////////////////////////////

#include "kclear/ionekeyclean.h"
#include "configdata.h"
#include "filelistdata.h"

//////////////////////////////////////////////////////////////////////////
// 一键清理支持
class KTrashOnekeyTask : public ICleanTask
{
public:
    KTrashOnekeyTask();
    ~KTrashOnekeyTask();

    //初始化
    HRESULT __stdcall Initialize();

    //反初始化
    HRESULT __stdcall Uninitialize();

    //设置回调
    void __stdcall SetCallback(ICleanCallback* pCallBack);

   //停止清除
    bool __stdcall StopClean();

    //开始清除
    bool __stdcall StartClean(int nTask,const char* szTask);

    void __stdcall Release()
    {
        delete this;
    }

private:
    BOOL IsCleaning() const;
    void RunCleanFile();
    static DWORD WINAPI CleanFileThread(LPVOID lpVoid);

    bool SplitStrTask( 
        /*[in]*/  std::wstring str, 
        /*[out]*/ std::vector <std::wstring>& vcResult,
        /*[in]*/  char delim = ','
        );
 
    ICleanCallback* m_pCallBack;       //回调
    CConfigData m_config;
    CFileListData  m_FileListData;
    HANDLE m_hThread;
    DWORD  m_dwThreadID;
    CConfigData::DirWorkArray m_DirWorks;
    DWORD m_ulTotalJunkFileSize;
    int m_nCurrentIndex;
    CFileDelete DelFile;
    std::map<std::wstring,bool> TaskMap;
    std::vector<std::wstring> TaskVector;
    std::vector<std::wstring> TaskDesc;
    std::wstring strDesc;  
};

//////////////////////////////////////////////////////////////////////////
// 提供给一键清理
bool __cdecl GetTrashCleanerForOnekey(const GUID& riid, void** ppv);

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_TRASHONEKEY_H_

#ifndef KCLEAR_EXAMIMP_H_
#define KCLEAR_EXAMIMP_H_

//////////////////////////////////////////////////////////////////////////

#include "configdata.h"
#include "filelistdata.h"
#include "traversefile.h"

//////////////////////////////////////////////////////////////////////////

bool __stdcall HasTrash();
bool __stdcall SetTrashNotifyHwnd(HWND hNotifyWnd);
HWND __stdcall GetTrashNotifyHwnd();

//////////////////////////////////////////////////////////////////////////
// À¬»øÌå¼ìÉ¨ÃèÆ÷
class KExamScanner : ITraverseFile
{
public:
    KExamScanner();
    ~KExamScanner();

    HRESULT __stdcall Initialize();
    HRESULT __stdcall Uninitialize();
    bool __stdcall StopScan();
    bool __stdcall StartScan();

    ULONGLONG GetFileSize()
    {
        return m_ulTotalJunkFileSize;
    }
    
    int GetLimit()
    {
        if (m_nLimitSize == 0)
        {
            m_nLimitSize = 150;
        }
        return m_nLimitSize;
    }

public:
    void TraverseFile(LPFINDFILEDATA pFileData);

    void TraverseProcess(unsigned long uCurPos);

    void TraverseProcessEnd(unsigned long uCurPos,CString videoPath);

    void TraverseFinished();

    BOOL RunVideoCacheFile(int id);

private:
    BOOL IsScaning() const;
    void RunScanFile();
    static DWORD WINAPI ScanFileThread(LPVOID lpVoid);

    CConfigData m_config;
    CFileListData  m_FileListData;
    HANDLE m_hThread;
    DWORD  m_dwThreadID;

    CConfigData::DirWorkArray m_DirWorks;
    ULONGLONG m_ulTotalJunkFileSize;
    int m_nCurrentIndex;
    std::map<std::wstring,bool> TaskMap;
    std::vector<std::wstring> TaskVector;

    std::vector<UINT> m_vCustomSelected;
    int m_nLimitSize;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_EXAMIMP_H_

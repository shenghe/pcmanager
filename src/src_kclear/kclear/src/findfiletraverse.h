// FindFileTraverse.h: interface for the CFindFileTraverse class.
//
//////////////////////////////////////////////////////////////////////


#pragma once

#include "kvideocache.h"
#include "ConfigData.h"

class ITraverseFile;
class CConfigData;

class CFindFileTraverse
{
public:
    CFindFileTraverse();
    virtual ~CFindFileTraverse();

public:

    BOOL StartFindFileTraverse( 
        const CConfigData& config,
        ITraverseFile* piTraverseFile,
        BOOL Is_Init);

    BOOL StopFileTraverse();

    BOOL IsFinding() const ;

private:

    void RunFindFile();

    static DWORD WINAPI FindFileThread(LPVOID lpVoid);

	BOOL RunVideoCacheFile(int id, CString& strPath);

    CConfigData::DirWorkArray m_DirWorks;


private:

    ITraverseFile* m_piTraverseFile;
    BOOL Is_Dlg_Init;
    HANDLE m_hThread;
    DWORD  m_dwThreadID;
};


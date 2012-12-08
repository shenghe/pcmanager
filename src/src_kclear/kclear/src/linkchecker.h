#pragma once

//////////////////////////////////////////////////////////////////////////

class CLinkChecker
{
public:
    static CLinkChecker& Instance();
    BOOL Init();
    void UnInit();

    BOOL IsDirectoryJunction(LPCTSTR pszDir);
    BOOL IsFileSymlinkd(LPCTSTR pszFile);

private:
    CLinkChecker();
    ~CLinkChecker();
    
    BOOL m_bInit;
    BOOL _GainBackupPrivilege();
    HANDLE _OpenDirectory(LPCTSTR pszPath, BOOL bReadWrite);
    HANDLE _OpenFile(LPCTSTR pszPath);
};

//////////////////////////////////////////////////////////////////////////

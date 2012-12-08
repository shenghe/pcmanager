// GetEnvPath.h: interface for the CGetEnvPath class.
//
//////////////////////////////////////////////////////////////////////


#pragma once


class CGetEnvPath  
{
public:
	CGetEnvPath();
	virtual ~CGetEnvPath();
    
public:    
    CString GetRealPath(LPCTSTR pszEnvPath);

    BOOL StandardPath(CString& strPath);

private:

    CString CombinationPath(const CString& strLeft, 
        const CString& strMiddle, 
        const CString& strRight);

    BOOL FindEnvironmentPos(LPCTSTR pszEnvPath, int& nFirstPos, int& nSecondPos);
    CString GetEnvVariable(LPCTSTR pszName);
    CString GetFolderPath(LPCTSTR pszName);
    CString GetDrive(CString suffix);

private:

    typedef DWORD (WINAPI* pfnGetLongPathNameW)(LPWSTR, LPWSTR, DWORD);

    HMODULE m_hModule;
    pfnGetLongPathNameW m_pfnGetLongPathName;
};


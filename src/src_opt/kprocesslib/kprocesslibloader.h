#pragma once


class ProcessLib;
class KSProcessInfo;

class KProcessLibLoader
{
public:
    KProcessLibLoader(void);
    ~KProcessLibLoader(void);

	// 获取默认进程库的路径
	static CString GetDefaultLibPath();

    BOOL LoadLib(LPCTSTR szLibDatPath = NULL);

	BOOL FindInfo(LPCTSTR szFilePath,  KSProcessInfo& processInfo);

protected:
    

protected:
    BOOL						m_bLoaded;
    CString						m_strDatPath;
    ProcessLib*                 m_pProcessLib;

};

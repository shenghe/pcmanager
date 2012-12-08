#ifndef	 _TTCLEAN_H_
#define  _TTCLEAN_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"

#include "fileopt.h"
#include "kscbase/kscconv.h"
#include "getinstallpath.h"
#include "sqlite3.h"
#include "ksconversion.h"
#include <UrlHist.h>

/************************************************************************/
//功能:腾讯清理
//
/************************************************************************/

extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;


class CTTClean
{
public:
	CTTClean();
	~CTTClean();
	void	Start(){m_appHistory.Start(); m_bScan=TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanTT();
    BOOL CleanTT(CString& strInPath);
	BOOL ModifyDataBase(const CString strFileName, const CString strTable);
	BOOL ModifyTxtData(const CString strFileName, const CString strSection);

private:
	CAppHistory m_appHistory;
    CFileOpt	m_fileOpt;
    BOOL		m_bScan;

    BOOL FindFileInDirectory(LPCTSTR pszFullPath,CString &filePathName,CString fileName)
    {
        BOOL bResult = TRUE;
        WIN32_FIND_DATA ff = { 0 }; 


        // 递归搜索子目录
        TCHAR szFindName[MAX_PATH] = {0};
        _tcsncpy_s( szFindName, MAX_PATH, pszFullPath,  MAX_PATH - 1 );
        _tcsncat_s( szFindName, MAX_PATH, TEXT("*.*") , MAX_PATH - _tcslen(pszFullPath) - 1 );
        HANDLE findhandle = ::FindFirstFile( szFindName, &ff ); 

        if( findhandle == INVALID_HANDLE_VALUE )
        {
            bResult = FALSE;
            goto Exit0;
        }

        BOOL res = TRUE;

        while(res)
        {
            if( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  
            {
                if (( _tcscmp( ff.cFileName, _T( "."  ) ) != 0) 
                    && ( _tcscmp( ff.cFileName, _T( ".." ) ) != 0 ))
                {
                    memset(szFindName, 0, sizeof(szFindName));
                    _tcsncpy_s( szFindName, MAX_PATH, pszFullPath,
                        MAX_PATH - 1 );
                    _tcsncat_s( szFindName, MAX_PATH, ff.cFileName, 
                        MAX_PATH - _tcslen(szFindName) - 1 );
                    _tcsncat_s( szFindName, MAX_PATH,TEXT("\\"),
                        MAX_PATH - _tcslen(szFindName) - 1 );

                    FindFileInDirectory( szFindName,filePathName,fileName);  
                }
            }
            else
            {
				if(fileName.CompareNoCase(ff.cFileName)==0)
				{
					bResult = TRUE;
					filePathName = pszFullPath;
					filePathName += ff.cFileName;
					break;
				}
                
            }

            res = ::FindNextFile( findhandle, &ff );
        }

        ::FindClose( findhandle );
Exit0:
        return bResult;       
    }
};


#endif
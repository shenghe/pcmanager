#ifndef __PUBLICFUNC_H__
#define __PUBLICFUNC_H__

#include <vector>
#include "trashdefine.h"
#include "traversefile.h"
#include "kclear/ionekeyclean.h"
#include "filestruct.h"
#include "filelistdata.h"
#include "deletefiletraverse.h"
//#include "trashcleaner.h"
#include <TlHelp32.h>

void SplitString(CString str, WCHAR split, std::vector <CString>& vcResult);

int KAEGetSystemVersion(/* [Out] */ SYSTEM_VERSION *pnSystemVersion);

CString KasLangFormatFileName( LPCTSTR pszName );

CString KasLangGetTrashConfigFilePath();

void GetFileSizeTextString(__int64 fNum, CString& strFileSize);

void GetFileCountText(UINT nCount, CString& strCount);

CString GetExtName(CString fileName);

BOOL FindFileInDirectory(LPCTSTR pszFullPath,
                         const std::vector<CString>& extArray, 
                         int nRecursive, 
                         int nStopIndex,
                         ITraverseFile *pTF,
                         CFileListData *pFilelist,
                         int nCurrentIndex,
                         std::vector<CString> &vFilt,
						 CString);

BOOL FileFilter(LPCTSTR pszFileName);

BOOL DirFilter(LPCTSTR pszDirName);

BOOL FindFileWithExtInDirectory(LPCTSTR pszFullPath, 
                                LPCTSTR pszExt, 
                                int nStopIndex, 
                                ITraverseFile *pTF,
                                CFileListData *pFilelist,
                                int nCurrentIndex,
                                std::vector<CString> &vFilt,
								CString);

BOOL FindFileByExtListInDirectory(LPCTSTR pszFullPath, 
                                  const std::vector<CString> &vExt, 
                                  int nStopIndex, 
                                  ITraverseFile *pTF,
                                  CFileListData *pFilelist,
                                  int nCurrentIndex,
                                  std::vector<CString> &vFilt,
                                  CString& strScanItem);

BOOL DeleteFileWithExtInDirectory(LPCTSTR pszFullPath, 
                                  LPCTSTR pszExt,
                                  int nStopIndex,
                                  CFileListData *pFilelist,
                                  int nCurIndex,
                                  ICleanCallback *pCcb,
                                  CFileDelete *pDelFile,
                                  std::wstring strDesc,
                                  std::vector<CString> &vFilt);

BOOL DeleteFileInDirectory(LPCTSTR pszFullPath,
                           const std::vector<CString>& extArray, 
                           int nRecursive,
                           int nStopIndex,
                           CFileListData *pFilelist,
                           int nCurIndex,
                           ICleanCallback *pCcb,
                           CFileDelete *pDelFile,
                           std::wstring strDesc,
                           std::vector<CString> &vFilt);

BOOL IsFileTimeInner(FILETIME tCurFile, int nDays = 1);

 DWORD GetProcessIdFromName(LPCTSTR szProcessName);

 DWORD GetProcessPathFromName(LPCTSTR szProcessName, __out CString& strPath);

 BOOL IsExtCompare(std::wstring strFileName, std::wstring strExt);

 BOOL IsItemProcessRunning(int nCurIndex);

 BOOL GetProcessNamesByID(UINT nCurIndex, std::vector<CString>& vecPro);

//  HRESULT FindLinkArgument(const WCHAR *pLnkFilePath, 
// 	 BOOL &bHave);
// 
//  HRESULT FindLinkUrl(const WCHAR *pLnkFilePath, 
// 	 BOOL &bHave);

#endif 
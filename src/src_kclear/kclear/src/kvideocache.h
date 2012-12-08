#ifndef _KVIDEOCACHE_H_
#define _KVIDEOCACHE_H_
#include <Shlwapi.h>
#include "misc/kregfunction.h"
#include "misc/inieditor.h"
#include "traversefile.h"
#include "filestruct.h"
#include "trashdefine.h"

#pragma comment(lib, "shlwapi.lib")

CString GetDrive(CString suffix);
DWORD GetDiffDaysEx( SYSTEMTIME local,  FILETIME ftime );
DWORD FileTimeInnerEx(FILETIME tCurFile);
void FileTimeToTime_tEx( FILETIME ft, time_t *t );
BOOL ScanFileBasedPathAndName(CString strItem,ITraverseFile *pTF, const CString strPath, const CString strName,  int nStopIndex, BOOL bTime = FALSE, DWORD dwTime = 0, int nNameLen = -1);
BOOL _GetComputerDrives(std::vector<CString>& vecDrive);
BOOL CheckFileNameLength(const CString& strFileName, const int nNameLength);

BOOL _ScanTudouCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL _ScanQvodCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL _ScanKankanCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);
BOOL ScanKankanCacheEx(ITraverseFile *pTF, const CString strPath, int nStopIndex, int nFlag);

BOOL _ScanYoukuCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL _ScanKu6Cache(ITraverseFile *pTF, CString& strPath, int nStopIndex);
BOOL ScanKu6CacheEx(ITraverseFile *pTF, const CString strPath, int nStopIndex);

BOOL _ScanPPTVCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL _ScanFengxingCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL ScanFengxingCacheEx(ITraverseFile *pTF, const CString strPath, int nStopIndex);

BOOL _ScanQQLiveCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);
BOOL ScanQQLiveCacheEx(ITraverseFile *pTF, CString& strPath, int nStopIndex, BOOL bTime = FALSE, DWORD dwTime = 0);
BOOL _EnumQQLiveDrivePath(std::vector<CString>& strScanPath);

BOOL _ScanStormCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL _ScanKuGooCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);

BOOL _ScanPIPICache(ITraverseFile *pTF, CString& strPath, int nStopIndex);
BOOL _ScanPIPICacheEx(ITraverseFile *pTF, CString& strPath, int nStopIndex, std::vector<CString>& vecFilter);
void _GetPIPIFilterPaths(LPCTSTR szPath, std::vector<CString>& vecFilter);

BOOL _ScanKuwoCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);


BOOL _ScanKSafeVulCache(ITraverseFile *pTF, CString& strPath, int nStopIndex);
BOOL _ScanKSafeVulCacheEx(ITraverseFile *pTF, CString& strPath, int nStopIndex);

#endif
#pragma once

#include <set>
#include <map>

typedef struct FIND_FILE_DATA
{
    CString     strFileName;    // 文件名
    CString     strSubPathName; // 文件相对路径
    CString     strExt;         // 文件格式
    FILETIME    ftCreateDate;  // 文件创建时间
    ULONGLONG   uFileSize;      // 文件大小
    CString     strFullPath;//全路径

//     FIND_FILE_DATA(const FIND_FILE_DATA& cpy)
//     {
// 
//         strFileName = cpy.strFileName;
//         strSubPathName = cpy.strSubPathName; 
//         strExt = cpy.strExt;         // 文件格式
//         ftCreateDate = cpy.ftCreateDate;  // 文件创建时间
//         uFileSize = cpy.uFileSize;      // 文件大小
//         strFullPath = cpy.strFullPath;//全路径
//     }

    const FIND_FILE_DATA& operator = (const FIND_FILE_DATA& cpy)
    {

        strFileName = cpy.strFileName;
        strSubPathName = cpy.strSubPathName; 
        strExt = cpy.strExt;         // 文件格式
        ftCreateDate = cpy.ftCreateDate;  // 文件创建时间
        uFileSize = cpy.uFileSize;      // 文件大小
        strFullPath = cpy.strFullPath;//全路径
        return *this;
    }

    bool operator < (const FIND_FILE_DATA & item){  
        return uFileSize > item.uFileSize;  
    } 

}FILEDATA, *LPFILEDATA;

typedef struct FIND_FILE_DATA_LIST {

    std::vector<FILEDATA> itemArray;

    ULONGLONG ulAllFileSize;

    int nDirIndex;

    FIND_FILE_DATA_LIST() : ulAllFileSize(0)
    {}


}FILEDATALIST,*LPFILEDATALIST;

class SetComp:std::less<FILEDATA>  
{  
public:  
    bool operator() (FILEDATA item1,FILEDATA item2) const{  
         return item1.uFileSize > item2.uFileSize;  
      }  
}; 
bool SortComp(FILEDATA item1,FILEDATA item2);


typedef struct  DIR_WORK
{
    std::set<CString> strDirectory; //大项包含的路径
    CString strCaptionName; // 扫描大项
    ULONGLONG ulSize; //该项总大小
    int nIndex; //索引
    CString dirDesc;
    BOOL bScanFlag;

}BIGDIRECTORY;




class CBigFileScan
{
public:
    CBigFileScan(void);
public:
    virtual ~CBigFileScan(void);
//     const CAtlList<FINDFILEDATA>& GetFileDetailList();
//     const CAtlList<CString>& GetFilePathList();
     bool StopScan();
     bool StartScan();
     BOOL IsScaning() const;

     BOOL StartSystemDriveScan();
     bool StopSystemDriveScan();
     BOOL IsSystemDriveScaning() const;

     std::vector<FILEDATALIST> m_FileList;
     std::vector<FILEDATA> m_BigFileVec;
     std::vector<BIGDIRECTORY> m_DirWork;
     HWND ParentHWND;
     std::map<CString,CString> ExtMap;

     CString GetExtName(const CString fileName)
     {
         int pos=fileName.Find(L"."); //获取 . 的位置
         if(pos==-1){ //如果没有找到，直接返回该字符串
             return fileName;
         }else{
             return GetExtName(fileName.Mid(pos+1)); //找到了的话，往深层遍历，直到最底层
         }
     }

     int GetPosLength(CString strPath,TCHAR wPos)
     {
         int nOut = 0;

         for(int i=0;i<strPath.GetLength();i++)
         {
             if(strPath.GetAt(i)==wPos)
                 nOut++;
         }

         return nOut;
     }

protected:
     BOOL FindFileInDirectory(LPCTSTR pszFullPath);
     BOOL FindSystemFileInDirectory(LPCTSTR pszFullPath);
     void RunScanFile();
     static DWORD WINAPI ScanFileThread(LPVOID lpVoid);
     static DWORD WINAPI ScanSystemFileThread(LPVOID lpVoid);
     BOOL FileFilter(LPCTSTR pszFileName);
     BOOL DirFilter(CString strDirName);
     BOOL FileExtFilter(CString pszFileName);
	 BOOL _LoadBigFileRule();
     BOOL UserDirFilter(CString strDirName);
    

private:
    BOOL Initialize();

    
    CAtlList<CString> m_CurrentFileList;
    HANDLE m_hThread;
    DWORD  m_dwThreadID;
    HANDLE m_hSystemThread;
    DWORD  m_dwSystemThreadID;
    BOOL m_bstopscan;
    BOOL m_bstopsystemscan;
    ULONGLONG m_ulTotalFileSize;
    int m_nCurrentIndex;
    CString m_strRecursive;
    CString m_strSystemRecursive;

	CAtlList<CString> m_pszFilterExt;
	CAtlList<CString> m_pszFilterFile;
	CAtlList<CString> m_pszFilterDir;

    CString strDocPath;
    CString strDownLoadPath;
    CString strDesktopPath;
  
};

//const LPCTSTR g_pszFilterExt[] = {
//    TEXT("torrent"),
//    TEXT("lnk"),
//    TEXT("download"),
//    TEXT("part"),
//    TEXT("se!"),
//    TEXT("tw!"),
//    TEXT("cfg"),
//    TEXT("td"),
//    TEXT("jccfg3"),
//    TEXT("jc"),
//    TEXT("tdl"),
//    TEXT("bc!"),
//    TEXT("met"),
//    TEXT("part"),
//    TEXT("!mv"),
//    TEXT("kg!"),
//    TEXT("ini"),
//    TEXT("sys"),
//    TEXT("dll"),
//    TEXT("db"),
//    TEXT("jar"),
//    TEXT("hxw")
//};
//
//const LPCTSTR g_pszFilterFile[] = {
//    TEXT("desktop.ini"),
//    TEXT("INFO2"),
//    TEXT("FP_AX_CAB_INSTALLER.exe"),
//    TEXT("swflash.inf"),
//    TEXT("dwusplay.dll"),
//    TEXT("dwusplay.exe"),
//    TEXT("isusweb.dll"),
//    TEXT("IDrop.ocx"),
//    TEXT("IDropCHS.dll"),
//    TEXT("IDropENU.dll"),
//    TEXT("index.dat")
//};
//
//static LPCTSTR g_pszFilterDir[] = {
//    TEXT("\\recovery"),
//    TEXT("\\windows"),
//    TEXT("\\boot"),
//    TEXT("\\program files\\microsoft"),
//    TEXT("\\program files\\windows!"),
//    TEXT("\\program files\\google"),
//    TEXT("\\program files\\adobe"),
//    TEXT("\\program files\\vmware"),
//    TEXT("\\program files\\msdn"),
//    TEXT("\\program files\\java"),
//    TEXT("\\program files\\auto cad"),
//    TEXT("\\program files\\vitakey"),
//    TEXT("\\program files\\vp suite"),
//    TEXT("\\program files\\common files"),
//    TEXT("\\program files\\foxmail"),
//    TEXT("\\microsoft"),
//    TEXT("\\identities"),
//    TEXT("\\virtualstore")
//};


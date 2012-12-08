
#ifndef __PUBLICSTRUCT_H__
#define __PUBLICSTRUCT_H__


typedef struct tagFIND_FILE_DATA
{
    LPCTSTR     pszFileNmae;    // 文件名
    LPCTSTR     pszPathName;    // 文件路径
    
    LPCTSTR     pszExtName;     // 扩展名
    ULONGLONG   uFileSize;      // 文件大小
	int         nCount;
    
}FINDFILEDATA, *LPFINDFILEDATA;

#endif //__PUBLICSTRUCT_H__
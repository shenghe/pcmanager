#include "StdAfx.h"
#include "bigfilescan.h"
#include "bigfilemgr.h"
#include "msgdefine.h"
#include <algorithm>
#include "kscbase/kscfilepath.h"
#include "linkchecker.h"
#include "kclear/libheader.h"

CBigFileScan::CBigFileScan(void)
{
    m_bstopscan     = false;
    m_hThread    = NULL;
    m_dwThreadID = 0;
    m_nCurrentIndex = -1;
    m_strRecursive = L"";
    m_strSystemRecursive = L"";

    m_hSystemThread = NULL;
    m_dwSystemThreadID = 0;
    m_bstopsystemscan = false;

    ExtMap[L"avi"] = L"视频文件";
    ExtMap[L"mkv"] = L"视频文件";
    ExtMap[L"ogm"] = L"视频文件";
    ExtMap[L"mp4"] = L"视频文件";
    ExtMap[L"m4p"] = L"视频文件";
    ExtMap[L"m4b"] = L"视频文件";
    ExtMap[L"mpg"] = L"视频文件";
    ExtMap[L"mpeg"] = L"视频文件";
    ExtMap[L"pss"] = L"视频文件";
    ExtMap[L"pav"] = L"视频文件";
    ExtMap[L"vob"] = L"视频文件";
    ExtMap[L"mpe"] = L"视频文件";
    ExtMap[L"wv"] = L"视频文件";
    ExtMap[L"m2ts"] = L"视频文件";
    ExtMap[L"ra"] = L"视频文件";
    ExtMap[L"ram"] = L"视频文件";
    ExtMap[L"rm"] = L"视频文件";
    ExtMap[L"rmvp"] = L"视频文件";
    ExtMap[L"rp"] = L"视频文件";
    ExtMap[L"rpm"] = L"视频文件";
    ExtMap[L"m4v"] = L"视频文件";
    ExtMap[L"f4v"] = L"视频文件";
    ExtMap[L"flv"] = L"视频文件";
    ExtMap[L"swf"] = L"视频文件";
    ExtMap[L"rmvb"] = L"视频文件";

    ExtMap[L"rsc"] = L"音频文件";
    ExtMap[L"rt"] = L"音频文件";
    ExtMap[L"wmv"] = L"音频文件";
    ExtMap[L"wmp"] = L"音频文件";
    ExtMap[L"wm"] = L"音频文件";
    ExtMap[L"wma"] = L"音频文件";
    ExtMap[L"asf"] = L"音频文件";
    ExtMap[L"mp3"] = L"音频文件";
    ExtMap[L"ogg"] = L"音频文件";
    ExtMap[L"cda"] = L"音频文件";
    ExtMap[L"wav"] = L"音频文件";
    ExtMap[L"ape"] = L"音频文件";

    ExtMap[L"doc"] = L"文档";
    ExtMap[L"xls"] = L"文档";
    ExtMap[L"docx"] = L"文档";
    ExtMap[L"ppt"] = L"文档";
    ExtMap[L"xlsx"] = L"文档";
    ExtMap[L"pptx"] = L"文档";
    ExtMap[L"pdf"] = L"文档";
    ExtMap[L"caj"] = L"文档";
    ExtMap[L"vss"] = L"文档";
    ExtMap[L"et"] = L"文档";
    ExtMap[L"dps"] = L"文档";
    ExtMap[L"wps"] = L"文档";
    
   

    ExtMap[L"msi"] = L"安装文件";
    ExtMap[L"exe"] = L"应用程序";
    ExtMap[L"iso"] = L"镜像文件";
    ExtMap[L"img"] = L"镜像文件";
    ExtMap[L"gho"] = L"镜像文件";
   

    ExtMap[L"jpg"] = L"图像文件";
    ExtMap[L"png"] = L"图像文件";
    ExtMap[L"ico"] = L"图像文件";
    ExtMap[L"gif"] = L"图像文件";
    ExtMap[L"jpeg"] = L"图像文件";
    ExtMap[L"bmp"] = L"图像文件";

    ExtMap[L"zip"] = L"压缩文件";
    ExtMap[L"rar"] = L"压缩文件";
    ExtMap[L"7z"] = L"压缩文件";
    ExtMap[L"cab"] = L"压缩文件";

	_LoadBigFileRule();

    _GetMyDeskTop(strDesktopPath);
    _GetDownLoadsDir(strDownLoadPath);
    _GetMyDocument(strDocPath);
	strDesktopPath = strDesktopPath.Mid(strDesktopPath.Find(L'\\'));
	strDownLoadPath = strDownLoadPath.Mid(strDownLoadPath.Find(L'\\'));
	strDocPath = strDocPath.Mid(strDocPath.Find(L'\\'));
    strDesktopPath.MakeLower();
    strDownLoadPath.MakeLower();
    strDocPath.MakeLower();

}

CBigFileScan::~CBigFileScan(void)
{
}

BOOL CBigFileScan::Initialize()
{

    BOOL bRet = FALSE;
    CAtlList<CString> dirs;
    POSITION pos;
    

    if(m_DirWork.size() == 0)
    {
        BIGDIRECTORY dirData;

        dirData.nIndex = 0;
        dirData.strCaptionName = L"大文件";
        dirData.dirDesc = L"为您扫描出系统盘上最大的50个文件，请";
        _GetDrive(dirs);
        pos = dirs.GetHeadPosition();
        dirData.strDirectory.clear();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        dirData.ulSize = 0;
        dirData.bScanFlag = TRUE;
        m_DirWork.push_back(dirData);

        dirData.nIndex = 1;
        dirData.strCaptionName = L"我的文档";
        dirData.dirDesc = L"您在系统盘上存放有大量文档内容。建议您可以尝试将其中的文件迁移到其他盘符，以节省系统盘空间。";
        _GetMyDocumentDir(dirs);
        pos = dirs.GetHeadPosition();
        dirData.strDirectory.clear();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        dirData.ulSize = 0;
        dirData.bScanFlag = TRUE;
        m_DirWork.push_back(dirData);

        dirData.nIndex = 2;
        dirData.strCaptionName = L"桌面";
        dirData.dirDesc = L"您的桌面存放了较多大文件，您可以选择删除或迁移这些文件至其他分区，以节省系统盘空间。";
        _GetMyDeskTopDir(dirs);
        pos = dirs.GetHeadPosition();
        dirData.strDirectory.clear();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        dirData.ulSize = 0;
        dirData.bScanFlag = TRUE;
        m_DirWork.push_back(dirData);

        dirData.nIndex = 3;
        dirData.strCaptionName = L"下载目录";
        dirData.dirDesc = L"您下载的文件占用了较多空间，您可以根据需要删除或迁移这些文件，以节省系统盘空间。";
        _GetChormeBigDir(dirs);
        pos = dirs.GetHeadPosition();
        dirData.strDirectory.clear();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetFirefoxBigDir(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _Get360BigDir(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetTheworldBigDir(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetMaxmonthBigDir(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetFlashGetBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetXunLeiBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetBitCometBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetQQDownLoadBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        dirData.ulSize = 0;
        dirData.bScanFlag = TRUE;
        m_DirWork.push_back(dirData);

        
        

        dirData.nIndex = 4;
        dirData.strCaptionName = L"视频目录";
		dirData.dirDesc = L"您的视频播放软件下载了较多视频文件，您可以根据需要删除或迁移这些视频文件，以节省系统盘空间。";
        _GetYouKuBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        dirData.strDirectory.clear();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetKu6BigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetXunleiKankanBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        dirData.ulSize = 0;
        dirData.bScanFlag = TRUE;
        m_DirWork.push_back(dirData);


        dirData.nIndex = 5;
        dirData.strCaptionName = L"音乐目录";
        dirData.dirDesc = L"您的音频播放软件下载了较多音频文件，您可以根据需要删除或迁移这些音频文件，以节省系统盘空间。";
        _GetKuwoBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        dirData.strDirectory.clear();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        _GetKugouBigDirs(dirs);
        pos = dirs.GetHeadPosition();
        while(pos!=NULL)
        {
            CString dirTmp(dirs.GetNext(pos));
            if(dirTmp.GetAt(dirTmp.GetLength()-1)!=L'\\')
                dirTmp+=L"\\";
            dirData.strDirectory.insert(dirTmp.MakeLower());
        }
        dirData.ulSize = 0;
        dirData.bScanFlag = TRUE;
        m_DirWork.push_back(dirData);
        
        m_FileList.resize(m_DirWork.size());
    }

    bRet = TRUE;
    return bRet;
}

bool CBigFileScan::StartScan()
{
    std::vector<BIGDIRECTORY>::iterator iter ;
    bool bResult = TRUE;
    Initialize();

    if (IsScaning())
    {
        bResult = FALSE;
        goto Exit0;
    }

    m_ulTotalFileSize = 0; 
    iter = m_DirWork.begin();

    for (int nIndex = 0; nIndex < (int)m_FileList.size(); nIndex++)
    {
        if(m_DirWork.at(nIndex).bScanFlag)
        {
            m_FileList.at(nIndex).ulAllFileSize = 0;
            m_FileList.at(nIndex).itemArray.clear();
        }
    }

    m_bstopscan = FALSE;

    m_hThread = CreateThread(NULL, 
        0, 
        ScanFileThread, 
        (LPVOID)this,
        NULL,
        &m_dwThreadID);

    if (m_hThread == NULL)   
        bResult = FALSE;

Exit0:
    return bResult;
}



bool CBigFileScan::StopScan()
{
    
    if (IsScaning())
    {
        m_bstopscan = TRUE;
        WaitForSingleObject(m_hSystemThread,1000);
        
    }
    return true;
}

BOOL CBigFileScan::IsScaning() const
{
    return m_hThread != NULL;
}


BOOL CBigFileScan::StartSystemDriveScan()
{
    BOOL bResult = TRUE;

    if (IsSystemDriveScaning())
    {
        bResult = FALSE;
        goto Exit0;
    }

  

    m_bstopsystemscan = FALSE;

    m_hSystemThread = CreateThread(NULL, 
        0, 
        ScanSystemFileThread, 
        (LPVOID)this,
        NULL,
        &m_dwSystemThreadID);

    if (m_hSystemThread == NULL)   
        bResult = FALSE;

Exit0:
    return bResult;
}



bool CBigFileScan::StopSystemDriveScan()
{
    if (IsSystemDriveScaning())
    {
        m_bstopsystemscan = TRUE;
        WaitForSingleObject(m_hSystemThread,1000);
    }
    return true;
}

BOOL CBigFileScan::IsSystemDriveScaning() const
{
    return m_hSystemThread != NULL;
}


BOOL CBigFileScan::FindFileInDirectory(LPCTSTR pszFullPath)
{
    BOOL bResult = FALSE;
    WIN32_FIND_DATA ff = { 0 }; 

    if ( m_bstopscan )
    {
        bResult = TRUE;
        goto Exit0;
    }

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

    while( res && !m_bstopscan )
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

               
                    FindFileInDirectory(szFindName);  
            }
        }
        else if (!FileFilter(ff.cFileName) )
        {
            CString fileext =  GetExtName(ff.cFileName);
            FILEDATA TraverseFileData;

            TraverseFileData.strExt  = fileext;
            TraverseFileData.strFileName = ff.cFileName;
            TraverseFileData.uFileSize   = ff.nFileSizeHigh;
            TraverseFileData.uFileSize   = TraverseFileData.uFileSize << 32;
            TraverseFileData.uFileSize  |= ff.nFileSizeLow;

            CString subPath(pszFullPath);
            int nPos = m_strRecursive.GetLength()-1;
            subPath = subPath.Mid(nPos);

            TraverseFileData.strSubPathName = subPath;
            TraverseFileData.strFullPath = pszFullPath;
            TraverseFileData.ftCreateDate = ff.ftCreationTime;

            if(ExtMap.find(fileext)!=ExtMap.end()&&m_nCurrentIndex==1 || m_nCurrentIndex > 1)
            {
                
                if (TraverseFileData.uFileSize > 1024*1024*2)
                {
                    
                    BOOL bFind = FALSE;
                    for(int curPos = 0;curPos<(int)m_FileList.at(m_nCurrentIndex).itemArray.size();curPos++)
                    {
                        FILEDATA tempData = m_FileList.at(m_nCurrentIndex).itemArray.at(curPos);
                        CString strFullname = tempData.strFullPath+tempData.strFileName;
                        if(strFullname.CompareNoCase(TraverseFileData.strFullPath+TraverseFileData.strFileName)==0)
                        {
                            bFind = TRUE;
                            break;
                        }
                        
                    }
                    if(!bFind)
                    {
                        m_FileList.at(m_nCurrentIndex).ulAllFileSize += TraverseFileData.uFileSize;
                        m_FileList.at(m_nCurrentIndex).itemArray.push_back(TraverseFileData);
                        m_ulTotalFileSize += TraverseFileData.uFileSize;
                     //   InterlockedExchangeAdd((LONG*)&m_ulTotalFileSize,TraverseFileData.uFileSize);

                    }
                   
                }
                

            }
            
        }

        res = ::FindNextFile( findhandle, &ff );
    }

    ::FindClose( findhandle );
Exit0:
    return bResult;       
}


BOOL CBigFileScan::FileExtFilter(CString pszFileName)
{
    BOOL nRet = FALSE;
    CString fileext = GetExtName(pszFileName);
	fileext.MakeLower();
	if (m_pszFilterExt.Find(fileext) != NULL)
	{
		nRet = TRUE;
	}
  
    return nRet;
}

BOOL CBigFileScan::FileFilter( LPCTSTR pszFileName)
{

    BOOL nRet = FALSE;
	CString strFile(pszFileName);
	strFile.MakeLower();
	if (m_pszFilterFile.Find(strFile) != NULL)
	{
		nRet = TRUE;
	}
    return nRet;
}

BOOL CBigFileScan::DirFilter(CString strDirName)
{
    BOOL nRet = FALSE;
	POSITION pos;
	pos = m_pszFilterDir.GetHeadPosition();
	while (pos)
	{
		CString str = m_pszFilterDir.GetNext(pos);
		if (strDirName.Find(str) != -1)
		{
			nRet = TRUE;
			break;
		}
	}
    
    return nRet;
}

BOOL CBigFileScan::UserDirFilter(CString strDirName)
{
    BOOL nRet = FALSE;
	POSITION pos;
	pos = m_pszFilterDir.GetHeadPosition();
	while (pos)
	{
		CString str = m_pszFilterDir.GetNext(pos);
		if (strDirName.Find(str) != -1&&str!=L"\\users"&&str!=L"\\documents and settings")
		{
			nRet = TRUE;
			break;
		}
	}

    return nRet;
}


DWORD WINAPI CBigFileScan::ScanFileThread(LPVOID lpVoid)
{

    CBigFileScan* pThis = (CBigFileScan*)lpVoid;
    pThis->RunScanFile();
    InterlockedExchange((LONG*)&pThis->m_hThread, NULL);
    InterlockedExchange((LONG*)&pThis->m_dwThreadID, 0);
    return 0;
}

DWORD WINAPI CBigFileScan::ScanSystemFileThread(LPVOID lpVoid)
{

    CBigFileScan* pThis = (CBigFileScan*)lpVoid;
    
    pThis->m_BigFileVec.clear();

    std::set<CString>::iterator iter= pThis->m_DirWork.at(0).strDirectory.begin();


    for (;iter!=pThis->m_DirWork.at(0).strDirectory.end();iter++)
    {
        if((*iter).GetLength()<=0)
            continue;
        pThis->m_strSystemRecursive = *iter;
        pThis->FindSystemFileInDirectory(*iter);  

    }

    

    int nCount = 0;
    pThis->m_FileList.at(0).ulAllFileSize = 0;
    pThis->m_FileList.at(0).itemArray.clear();

    
    std::sort(pThis->m_BigFileVec.begin(),pThis->m_BigFileVec.end(),SortComp);


    
    for (std::vector<FILEDATA>::iterator iter_bigfile = pThis->m_BigFileVec.begin();iter_bigfile!=pThis->m_BigFileVec.end();iter_bigfile++)
    {
        pThis->m_FileList.at(0).ulAllFileSize += (*iter_bigfile).uFileSize;
        pThis->m_FileList.at(0).itemArray.push_back(*iter_bigfile);
        nCount++;
        if(nCount>49)
            break;
       // InterlockedExchangeAdd((LONG*)&pThis->m_ulTotalFileSize,(*iter_bigfile).uFileSize);
    }

    pThis->m_DirWork.at(0).ulSize = pThis->m_FileList.at(0).ulAllFileSize;

    ::SendMessage(pThis->ParentHWND, MSG_USER_DIRLIST_SCANEND,0, (LPARAM)&pThis->m_DirWork.at(0));
    
    InterlockedExchange((LONG*)&pThis->m_hSystemThread, NULL);
    InterlockedExchange((LONG*)&pThis->m_dwSystemThreadID, 0);
    return 0;
}

void CBigFileScan::RunScanFile()
{
    CString strDirectory = _T("");


    for (int i = 0; i < (int)m_DirWork.size() && !m_bstopscan; i++)
    {

        m_nCurrentIndex = i;

        if(m_nCurrentIndex == 0)
        { 
            ::SendMessage(ParentHWND, MSG_USER_DIRLIST_ADD,0, (LPARAM)&m_DirWork.at(i));
            continue;
        }
       
        if(!m_DirWork.at(i).bScanFlag)
           continue;

        std::set<CString>::iterator iter= m_DirWork.at(i).strDirectory.begin();

       // ::SendMessage(ParentHWND, MSG_USER_DIRLIST_SCANING,(WPARAM)m_ulTotalFileSize, (LPARAM)&m_DirWork.at(i));
        
        for(;iter!=m_DirWork.at(i).strDirectory.end();iter++)
        {
            if((*iter).GetLength()<=0)
                continue;
             m_strRecursive = *iter;
             if(m_strRecursive.GetLength()>3&&!UserDirFilter(m_strRecursive))
                FindFileInDirectory(*iter);  

        }
        m_DirWork.at(i).ulSize = m_FileList.at(m_nCurrentIndex).ulAllFileSize;

        std::sort(m_FileList.at(m_nCurrentIndex).itemArray.begin(),m_FileList.at(m_nCurrentIndex).itemArray.end(),SortComp);

        ::SendMessage(ParentHWND, MSG_USER_DIRLIST_ADD,0, (LPARAM)&m_DirWork.at(i));

        m_DirWork.at(i).bScanFlag = FALSE;

        
    }
       // ::SendMessage(ParentHWND, MSG_USER_DIRLIST_SCANEND,(WPARAM)m_ulTotalFileSize, 0);
        
}


BOOL CBigFileScan::FindSystemFileInDirectory(LPCTSTR pszFullPath)
{
    BOOL bResult = FALSE;
    WIN32_FIND_DATA ff = { 0 }; 
   

    if ( m_bstopsystemscan )
    {
        bResult = TRUE;
        goto Exit0;
    }

    if(CLinkChecker::Instance().IsDirectoryJunction(pszFullPath))
    {
        bResult = TRUE;
        goto Exit0;
    }

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

    while( res && !m_bstopsystemscan )
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

                CString subPath(pszFullPath);
                int nPos = m_strSystemRecursive.GetLength()-1;
                subPath = subPath.Mid(nPos);
                subPath = subPath.MakeLower();
                CString strCmpName = subPath;
                strCmpName += ff.cFileName;
                strCmpName = strCmpName.MakeLower();
               
                CString FileFiltWindows(strCmpName.Left(8));


                if(!DirFilter(strCmpName))
                {
                    ::SendMessage(ParentHWND, MSG_USER_DIRLIST_SCANING,(WPARAM)szFindName, 0);
                    ::Sleep(1);
                    FindSystemFileInDirectory(szFindName);  
                }
                else if(FileFiltWindows.CompareNoCase(L"\\windows")==0)
                {
                    if(strCmpName.CompareNoCase(L"\\windows")==0)
                    {
                        ::SendMessage(ParentHWND, MSG_USER_DIRLIST_SCANING,(WPARAM)szFindName, 0);
                        ::Sleep(1);
                        FindSystemFileInDirectory(szFindName);  

                    }
                }
                else if (strCmpName.Find(L"users")!=-1||strCmpName.Find(L"documents and settings")!=-1)
                {
                    ::Sleep(1);
				//	if(strCmpName.Find(strDocPath.GetBuffer())!=-1
				//		||strCmpName.Find(strDownLoadPath.GetBuffer())!=-1
				//		||strCmpName.Find(strDesktopPath.GetBuffer())!=-1)
					::SendMessage(ParentHWND, MSG_USER_DIRLIST_SCANING,(WPARAM)szFindName, 0);

                    FindSystemFileInDirectory(szFindName);  

                }
                
            }
        }
        else if (!FileFilter(ff.cFileName)&&!FileExtFilter(ff.cFileName))
        {
            CString fileext =  GetExtName(ff.cFileName);
            FILEDATA TraverseFileData;

            TraverseFileData.strExt  = fileext;
            TraverseFileData.strFileName = ff.cFileName;
            TraverseFileData.uFileSize   = ff.nFileSizeHigh;
            TraverseFileData.uFileSize   = TraverseFileData.uFileSize << 32;
            TraverseFileData.uFileSize  |= ff.nFileSizeLow;

            CString subPath(pszFullPath);
            int nPos = m_strSystemRecursive.GetLength()-1;
            subPath = subPath.Mid(nPos);
            subPath = subPath.MakeLower();

            TraverseFileData.strSubPathName = subPath;
            TraverseFileData.strFullPath = pszFullPath;
            TraverseFileData.ftCreateDate = ff.ftCreationTime;
            
           
            CString FileFiltWindows(subPath.Left(8));
            
            if (!DirFilter(subPath))
            {
				if(TraverseFileData.uFileSize>=1024*1024)
					m_BigFileVec.push_back(TraverseFileData);

            }
            else if (FileFiltWindows.CompareNoCase(L"\\windows") == 0)
            {
                CString strTempFile  = subPath.Left(13);
                if(strTempFile.CompareNoCase(L"\\windows\\temp")==0 
                    || TraverseFileData.strFileName.CompareNoCase(L"memory.dmp")==0)
                {
					if(TraverseFileData.uFileSize>=1024*1024)
						m_BigFileVec.push_back(TraverseFileData);

                }

            }
            else if (subPath.Find(L"users")!=-1||subPath.Find(L"documents and settings")!=-1)
            {
                if(subPath.Find(strDocPath.GetBuffer())!=-1
                    ||subPath.Find(strDownLoadPath.GetBuffer())!=-1
                    ||subPath.Find(strDesktopPath.GetBuffer())!=-1)
					if(TraverseFileData.uFileSize>=1024*1024)
						m_BigFileVec.push_back(TraverseFileData);

            }

            
           


        }

        res = ::FindNextFile( findhandle, &ff );
    }

    ::FindClose( findhandle );
Exit0:
    return bResult;       
}

bool SortComp(FILEDATA item1,FILEDATA item2){  
    return item1.uFileSize > item2.uFileSize;  
} 

// BOOL CBigFileScan::_LoadBigFileRule()
// {
// 	BOOL retval = FALSE;
// 	TiXmlDocument xmlDoc;
// 	const TiXmlElement *pXmlHistory = NULL;
// 	const TiXmlElement *pXmlMove = NULL;
// 	KFilePath xmlPath = KFilePath::GetFilePath(NULL);
// 	CString strFilter;
// 
// 	xmlPath.RemoveFileSpec();
// 	xmlPath.Append(L"cfg\\bigfilerule.xml");
// 	if (!xmlDoc.LoadFile(UnicodeToAnsi(xmlPath.value()).c_str(), TIXML_ENCODING_UTF8))
// 		goto clean0;
// 
// 	pXmlHistory = xmlDoc.FirstChildElement("setting");
// 	if (!pXmlHistory)
// 		goto clean0;
// 
// 	pXmlMove = pXmlHistory->FirstChildElement("filter_ext");
// 	while (pXmlMove)
// 	{
// 		strFilter = KUTF8_To_UTF16(pXmlMove->Attribute("src"));
//         strFilter.MakeLower();
// 		m_pszFilterExt.AddTail(strFilter);
// 		pXmlMove = pXmlMove->NextSiblingElement("filter_ext");
// 	}
// 
// 	pXmlMove = pXmlHistory->FirstChildElement("filter_file");
// 	while (pXmlMove)
// 	{
// 		strFilter = KUTF8_To_UTF16(pXmlMove->Attribute("src"));
//         strFilter.MakeLower();
// 		m_pszFilterFile.AddTail(strFilter);
// 		pXmlMove = pXmlMove->NextSiblingElement("filter_file");
// 	}
// 
// 	pXmlMove = pXmlHistory->FirstChildElement("filter_dir");
// 	while (pXmlMove)
// 	{
// 		strFilter = KUTF8_To_UTF16(pXmlMove->Attribute("src"));
//         strFilter.MakeLower();
// 		m_pszFilterDir.AddTail(strFilter);
// 		pXmlMove = pXmlMove->NextSiblingElement("filter_dir");
// 	}
// 
// 	retval = TRUE;
// 
// clean0:
// 	return retval;
// }


BOOL CBigFileScan::_LoadBigFileRule()
{
    BOOL retval = FALSE;
    CDataFileLoader dataLoader;
    TiXmlDocument xmlDoc;
    const TiXmlElement *pXmlHistory = NULL;
    const TiXmlElement *pXmlMove = NULL;
    TCHAR szConfPath[MAX_PATH] = { 0 };
    CString strXmlUtf16;
    CStringA strXmlAnsi;
    CString strFilter;

    

    GetModuleFileName(NULL, szConfPath, MAX_PATH);
    PathRemoveFileSpec(szConfPath);
    PathAppend(szConfPath, _T("cfg\\bigfilerule.dat"));


    if (!dataLoader.LoadFile(szConfPath, strXmlUtf16))
        goto clean0;

    strXmlAnsi = KUTF16_To_ANSI(strXmlUtf16);

    if (!xmlDoc.LoadBuffer((LPSTR)(LPCSTR)strXmlAnsi, strXmlAnsi.GetLength(), TIXML_ENCODING_UTF8))
        goto clean0;

    pXmlHistory = xmlDoc.FirstChildElement("setting");
    if (!pXmlHistory)
        goto clean0;

    pXmlMove = pXmlHistory->FirstChildElement("filter_ext");
    while (pXmlMove)
    {
        strFilter = KUTF8_To_UTF16(pXmlMove->Attribute("src"));
        strFilter.MakeLower();
        m_pszFilterExt.AddTail(strFilter);
        pXmlMove = pXmlMove->NextSiblingElement("filter_ext");
    }

    pXmlMove = pXmlHistory->FirstChildElement("filter_file");
    while (pXmlMove)
    {
        strFilter = KUTF8_To_UTF16(pXmlMove->Attribute("src"));
        strFilter.MakeLower();
        m_pszFilterFile.AddTail(strFilter);
        pXmlMove = pXmlMove->NextSiblingElement("filter_file");
    }

    pXmlMove = pXmlHistory->FirstChildElement("filter_dir");
    while (pXmlMove)
    {
        strFilter = KUTF8_To_UTF16(pXmlMove->Attribute("src"));
        strFilter.MakeLower();
        m_pszFilterDir.AddTail(strFilter);
        pXmlMove = pXmlMove->NextSiblingElement("filter_dir");
    }

    retval = TRUE;

clean0:
    return retval;
}
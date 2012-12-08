//////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "filtertool.h"
#include "misc/wildcmp.h"
#include <TlHelp32.h>
//////////////////////////////////////////////////////////////////////////

CFilterTool::CFilterTool(void)
{
    m_bInitFlag = FALSE;
    m_FilterFuncMap.clear();    
    Init();

    m_bTimeFilter = FALSE;
}

CFilterTool::~CFilterTool(void)
{
    Uninit();
    m_bInitFlag = TRUE;
}

CFilterTool& CFilterTool::Instance()
{
    static CFilterTool ToolInst;

    return ToolInst;

}

BOOL CFilterTool::Init()
{
    BOOL bretval = FALSE;
    CStringA  strXml;
    TCHAR szModule[MAX_PATH] = { 0 };
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlSetting = NULL;
    TiXmlElement* pXmlFilter = NULL;
    TiXmlElement* pXmlItem = NULL; 

    GetModuleFileName(NULL, szModule, MAX_PATH);
    PathRemoveFileSpec(szModule);
    PathAppend(szModule, _T("cfg\\trashfilerule.dat"));
    CDataFileLoader datloader;
    if(!datloader.LoadFile(szModule, strXml))
        goto Exit0;

    xmlDoc.Parse(strXml);

    pXmlSetting = xmlDoc.FirstChildElement("setting");
    if(!pXmlSetting)
        goto Exit0;
    
    pXmlFilter = pXmlSetting->FirstChildElement("filter");
    while (pXmlFilter)
    {
        CString strName;
        CString strTemp;
        std::vector<CString> vFilter;

        strTemp = KUTF8_To_UTF16(pXmlFilter->Attribute("name"));
        _wcslwr_s(strTemp.GetBuffer(), strTemp.GetLength() + 1);
        strName += strTemp;
        
        pXmlItem = pXmlFilter->FirstChildElement("item");
        while (pXmlItem)
        {
            CString strTem;
            CString strAtt;

            strTem = KUTF8_To_UTF16(pXmlItem->Attribute("src"));
            _wcslwr_s(strTem.GetBuffer(), strTem.GetLength() + 1);
            strAtt += strTem;

            vFilter.push_back(strAtt);
        
            pXmlItem = pXmlItem->NextSiblingElement("item");
        }
        
        m_FilterMap[strName] = vFilter;

        pXmlFilter = pXmlFilter->NextSiblingElement("filter");
    }

    bretval = TRUE;

Exit0:     
    return bretval;
}

BOOL CFilterTool::Uninit()
{
    _DoClearAllFuncMap();
    return TRUE;
}


BOOL CFilterTool::_DoMakeFuncMap(CString strFuncName,FilterOper pHandle)
{
    BOOL bRet = FALSE;

    if (pHandle)
    {
        assert(pHandle);

        m_FilterFuncMap[strFuncName] = pHandle;

        bRet = TRUE;
    }

    return bRet;

}


BOOL CFilterTool::_DoFindFuncMap(CString& strFuncName, CString strFileName)
{
    BOOL bRet = FALSE;

    std::map<CString, std::vector<CString>>::iterator it;
    
    std::vector<CString> vFilter;
    std::vector<CString>::iterator iterFilter;

    if (strFileName.IsEmpty())
        goto Clear0;

    it = m_FilterMap.find(strFuncName);

    if (it == m_FilterMap.end())
        goto Clear0;

    vFilter = it->second;

    for (iterFilter = vFilter.begin(); iterFilter != vFilter.end(); ++iterFilter)
    {
        CString strFilter = (*iterFilter);
        strFilter.MakeLower();
        strFileName.MakeLower();
        if (wildcmp(strFilter, strFileName))
        {
            bRet = TRUE;
            goto Clear0;
        }
    }



Clear0:
    return bRet;

}


BOOL CFilterTool::_DoClearAllFuncMap()
{
    m_FilterFuncMap.clear();
    return TRUE;
}

BOOL CFilterTool::CheckWindowsPath(CString strFilePath)
{
    BOOL bRet = FALSE;
    CString strWindir = L"";

    GetWindowsDir(strWindir);
    strWindir.MakeLower();
    strFilePath.MakeLower();
   

    int nPos = strFilePath.Find(strWindir);

    if (nPos!=-1)
    {
       bRet = TRUE;   
    }

    return bRet;
}

BOOL CFilterTool::CheckDrivePath(CString strFilePath)
{
    BOOL bRet = FALSE;
    strFilePath.MakeLower();

    ::PathRemoveFileSpec(strFilePath.GetBuffer());
    
    if (strFilePath.GetLength()<=3)
    {
       bRet = TRUE;
    }

    return bRet;
}

BOOL CFilterTool::CheckProgramPath(CString strFilePath)
{
    BOOL bRet = FALSE;
    CString strSysdrive = L"";

    strFilePath.MakeLower();

    GetSystemDrive(strSysdrive);
    strSysdrive.MakeLower();
    strSysdrive += L"\\program files";

    if (strFilePath.Find(strSysdrive)!=-1)
    {
        bRet = TRUE;
    }

    return bRet;
}

BOOL CFilterTool::CheckDownloadProPath(CString strFilePath)
{
    BOOL bRet = FALSE;

    strFilePath.MakeLower();
    CString strFileExt = GetFileExtName(strFilePath);
    CString strFileName = strFilePath.Mid(strFilePath.ReverseFind(L'\\')+1);


    if(Filter(strFileName,g_pszDownloadFiles,8))
    {
        bRet = TRUE;
    }

    return bRet;
}

BOOL CFilterTool::CheckSpecFile(CString strFilePath)
{
    BOOL bRet = FALSE;

    strFilePath.MakeLower();

    CString strFileExt = GetFileExtName(strFilePath);
    CString strFileName = strFilePath.Mid(strFilePath.ReverseFind(L'\\')+1);

    if (Filter(strFileName, g_pszSpecFiles, 9))
    {
        bRet = TRUE;
    }

    return bRet;
}

BOOL CFilterTool::SetTimeFilter(BOOL bFilter)
{
    m_bTimeFilter = bFilter;

    return m_bTimeFilter;
}

BOOL CFilterTool::CheckTimeFilterByHours(FILETIME tCurFile, DWORD nHours)
{
    BOOL bRet = FALSE;

    if (!m_bTimeFilter)
        goto Clear0;

    if (FileTimeInnerEx(tCurFile) > nHours)
        bRet = TRUE;

Clear0:
    return bRet;
}

BOOL CFilterTool::CheckTimeFilterByDay(FILETIME tCurFile, DWORD nDays)
{
    BOOL bRet = FALSE;
    SYSTEMTIME tLocal;
    SYSTEMTIME tSysCurFile;

    if (!m_bTimeFilter)
    {
        bRet = TRUE;
        goto Clear0;
    }

    if (nDays < 0)
    {
        goto Clear0;
    }

    GetLocalTime(&tLocal);

    FileTimeToSystemTime(&tCurFile, &tSysCurFile);   


    if (tLocal.wYear < 2000 || tLocal.wYear > 2030 
        ||tSysCurFile.wYear < 2000 || tSysCurFile.wYear > 2030)
    {
        bRet = TRUE;
        goto Clear0;
    }
    else
    {
        CTime standardTime(tSysCurFile.wYear, tSysCurFile.wMonth, tSysCurFile.wDay, 0,0,0);
        CTime currentTime(tLocal.wYear, tLocal.wMonth, tLocal.wDay, 0, 0, 0);
        CTimeSpan ctSpan = currentTime - standardTime;
        DWORD spanDays = (DWORD)ctSpan.GetDays();
        
        if (spanDays >= nDays)
        {
            bRet = TRUE;
        }
    }
Clear0:
    return bRet;
}
BOOL CFilterTool::CheckProcessIsRunning(const CString& strProcessName)
{
    BOOL bRet = FALSE;
    DWORD processID;

    if (strProcessName.IsEmpty())
        goto Clear0;

    processID = GetProcessIdFromName(strProcessName);

    if (processID >= 4)
        bRet = TRUE;

Clear0:
    return bRet;
}
//////////////////////////////////////////////////////////////////////////
DWORD CFilterTool::GetDiffDaysEx( SYSTEMTIME local,  FILETIME ftime )
{
    FILETIME flocal;
    time_t ttlocal;
    time_t ttfile;

    SystemTimeToFileTime( &local, &flocal );

    FileTimeToTime_tEx( flocal, &ttlocal );
    FileTimeToTime_tEx( ftime, &ttfile );

    time_t difftime = ttlocal - ttfile; 

    return DWORD(difftime / 3600L);// ≥˝“‘3600√Î
}

void CFilterTool::FileTimeToTime_tEx( FILETIME ft, time_t *t )
{

    ULARGE_INTEGER ui;

    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;

    *t = ((ULONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
}

DWORD CFilterTool::FileTimeInnerEx(FILETIME tCurFile)
{
    SYSTEMTIME stLocal;  
    GetSystemTime(&stLocal);  
    DWORD dwDiff = GetDiffDaysEx(stLocal,tCurFile);

    return dwDiff;

}

DWORD CFilterTool::GetProcessIdFromName(LPCTSTR szProcessName)
{
    PROCESSENTRY32 pe;
    DWORD id = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    pe.dwSize = sizeof(PROCESSENTRY32);
    if( !Process32First(hSnapshot,&pe) )
        return 0;
    while(1)
    {
        pe.dwSize = sizeof(PROCESSENTRY32);
        if( Process32Next(hSnapshot,&pe)==FALSE )
            break;
        if(wcsicmp(pe.szExeFile,szProcessName) == 0)
        {
            id = pe.th32ProcessID;
            break;
        }

    }
    CloseHandle(hSnapshot);
    return id;
}

//////////////////////////////////////////////////////////////////////////

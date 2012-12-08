#include "stdafx.h"
#include "stubbornfiles.h"
#include "kclear/libheader.h"
#include "kscbase/kscconv.h"

//////////////////////////////////////////////////////////////////////////

CStubbornFiles::CStubbornFiles()
{
}

CStubbornFiles::~CStubbornFiles()
{
}

//////////////////////////////////////////////////////////////////////////

BOOL CStubbornFiles::Init()
{
    m_fileMap.RemoveAll();
    m_fileList.RemoveAll();
    return _LoadFromFile();
}

void CStubbornFiles::UnInit()
{
    //_SaveToFile();
    m_fileMap.RemoveAll();
    m_fileList.RemoveAll();
}

void CStubbornFiles::AddFile(const CStringW& strFilePath)
{
    CStringW strTempPath = strFilePath;
    CAtlMap<CStringW, BOOL>::CPair* pPair = NULL;
    KAutoLock lock(klock);

    if (strFilePath.IsEmpty())
        goto clean0;

    strTempPath.MakeLower();

    
    pPair = m_fileMap.Lookup(strTempPath);
    if (pPair)
        goto clean0;

    m_fileMap[strTempPath] = TRUE;
    m_fileList.AddTail(strTempPath);

clean0:
    return;
}

BOOL CStubbornFiles::Sync()
{
    return _SaveToFile();
}

BOOL CStubbornFiles::IsStubbornFile(const CStringW& strFilePath)
{
    BOOL retval = FALSE;

    CStringW strTempPath = strFilePath;
    CAtlMap<CStringW, BOOL>::CPair* pPair = NULL;
    KAutoLock lock(klock);


    if (strFilePath.IsEmpty())
        goto clean0;


    strTempPath.MakeLower();

//     static bool btest = FALSE;
// 
//     if(!btest)
//     {
//         POSITION pos = m_fileList.GetHeadPosition();
//         while (pos)
//         {
//             
// 
//              ::OutputDebugString(m_fileList.GetNext(pos));
//             
//         }
// 
//         btest = true;
// 
//     }

    

   
    pPair = m_fileMap.Lookup(strTempPath);
    if (!pPair)
        goto clean0;

   
    retval = TRUE;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

BOOL CStubbornFiles::_LoadFromFile()
{
    BOOL retval = FALSE;
    CDataFileLoader dataLoader;
    TiXmlDocument xmlDoc;
    const TiXmlElement* pXmlItems = NULL;
    const TiXmlElement* pXmlItem = NULL;
    const TiXmlElement* pXmlTime = NULL;
    TCHAR szConfPath[MAX_PATH] = { 0 };
    CStringA strXmlAnsi;
    CStringW strPathUtf16;
    CStringA strPathAnsi;
    SYSTEMTIME sysTime = { 0 };
    CStringA strTime;

    GetLocalTime(&sysTime);
    strTime.Format("%d.%d.%d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
    
    GetModuleFileName(NULL, szConfPath, MAX_PATH);
    PathRemoveFileSpec(szConfPath);
    PathAppend(szConfPath, _T("data\\strash.dat"));


    if (!dataLoader.LoadFile(szConfPath, strXmlAnsi))
        goto clean0;

    if (!xmlDoc.LoadBuffer((LPSTR)(LPCSTR)strXmlAnsi, strXmlAnsi.GetLength(), TIXML_ENCODING_UTF8))
        goto clean0;

    pXmlTime = xmlDoc.FirstChildElement("time");
    if (!pXmlTime)
        goto clean0;

    if (strTime.Compare(pXmlTime->GetText()))
        goto clean0;

    pXmlItems = xmlDoc.FirstChildElement("items");
    if (!pXmlItems)
        goto clean0;

    pXmlItem = pXmlItems->FirstChildElement("item");
    while (pXmlItem) 
    {
        strPathAnsi = pXmlItem->GetText();
        if (strPathAnsi.GetLength())
        {
            strPathUtf16 = KUTF8_To_UTF16(strPathAnsi);

            if (GetFileAttributes(strPathUtf16) != INVALID_FILE_ATTRIBUTES)
            {
                m_fileList.AddTail(strPathUtf16);
                m_fileMap[strPathUtf16] = TRUE;
            }
        }

        pXmlItem = pXmlItem->NextSiblingElement("item");
    }

    retval = TRUE;

clean0:
    return retval;
}

BOOL CStubbornFiles::_SaveToFile()
{
    BOOL retval = FALSE;
    CDataFileLoader dataLoader;
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlItem = NULL;
    TiXmlElement* pXmlTime = NULL;
    TiXmlElement* pXmlItems = NULL;
    TiXmlText* pXmlText = NULL;
    TCHAR szConfPathTemp[MAX_PATH] = { 0 };
    TCHAR szConfPath[MAX_PATH] = { 0 };
    TiXmlDeclaration *pXmlDecl = new TiXmlDeclaration("1.0", "utf-8", "yes"); 
    POSITION pos = NULL;
    CStringW strPathUtf16;
    CStringA strPathUtf8;
    CString strXmlUtf16;
    CStringA strXmlAnsi;
    LARGE_INTEGER ver;
    BOOL fRetCode;
    FILE* pFile = NULL;
    SYSTEMTIME sysTime = { 0 };
    CStringA strTime;

    GetModuleFileName(NULL, szConfPath, MAX_PATH);
    PathRemoveFileSpec(szConfPath);
    _tcscpy_s(szConfPathTemp, MAX_PATH, szConfPath);
    PathAppend(szConfPathTemp, _T("data\\~strash.dat"));
    PathAppend(szConfPath, _T("data\\strash.dat"));

    xmlDoc.LinkEndChild(pXmlDecl);

    pXmlItems = new TiXmlElement("items");
    if (!pXmlItems)
        goto clean0;

    pos = m_fileList.GetHeadPosition();
    while (pos)
    {
        strPathUtf16 = m_fileList.GetNext(pos);
        strPathUtf8 = KUTF16_To_UTF8(strPathUtf16);

        pXmlText = new TiXmlText(strPathUtf8);
        if (!pXmlText)
            goto clean0;

        pXmlItem = new TiXmlElement("item");
        if (!pXmlItem)
            goto clean0;

        pXmlItem->LinkEndChild(pXmlText);
        pXmlItems->LinkEndChild(pXmlItem);
    }
    xmlDoc.LinkEndChild(pXmlItems);

    GetLocalTime(&sysTime);
    strTime.Format("%d.%d.%d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
    pXmlTime = new TiXmlElement("time");
    if (!pXmlTime)
        goto clean0;

    pXmlText = new TiXmlText(strTime);
    if (!pXmlText)
        goto clean0;

    pXmlTime->LinkEndChild(pXmlText);

    xmlDoc.LinkEndChild(pXmlTime);

    if (!xmlDoc.SaveFile(KUTF16_To_ANSI(szConfPathTemp)))
        goto clean0;

    pFile = _wfopen(szConfPathTemp, L"r");
    if (!pFile)
        goto clean0;

    {
        fseek(pFile, 0, SEEK_END);
        int nSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
        char* pXml = strXmlAnsi.GetBuffer(nSize + 1);
        memset(pXml, 0, nSize + 1);
        fread(pXml, 1, nSize, pFile);
        fclose(pFile);
        pFile = NULL;
        strXmlAnsi.ReleaseBuffer();
        strXmlUtf16 = KANSI_TO_UTF16(strXmlAnsi);
    }

    {
        ver.QuadPart = 1;
        fRetCode = dataLoader.Save( 
            szConfPath, BkDatLibEncodeParam2(enumLibTypePlugine, ver, strXmlUtf16, 1).GetEncodeParam() 
            );
    }
    
    if (!fRetCode)
        goto clean0;

    retval = TRUE;

clean0:
    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if (GetFileAttributes(szConfPathTemp) != INVALID_FILE_ATTRIBUTES)
    {
        ::DeleteFile(szConfPathTemp);
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

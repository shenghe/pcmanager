// GetEnvPath.cpp: implementation of the CGetEnvPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GetEnvPath.h"
#include "trashpublicfunc.h"
#include "trashdefine.h"
#include "misc/kregfunction.h"
#include "misc/inieditor.h"
#include "kscbase/kscconv.h"
#include "kscbase/kscsys.h"

typedef struct tagCSIDL_NAME
{
    LPCTSTR pszCsidlName;
    int     nFolder;
}CSIDL_NAME, *LPCSIDL_NAME;

#ifdef _UNICODE
#define CSIDL_NAME_ENTRY(ID) \
{ L#ID , ID }
#else
#define CSIDL_NAME_ENTRY(ID) \
{ #ID , ID }
#endif

CSIDL_NAME g_csidlnamelist[] = 
{
    CSIDL_NAME_ENTRY(CSIDL_DESKTOP),
    CSIDL_NAME_ENTRY(CSIDL_INTERNET),
    CSIDL_NAME_ENTRY(CSIDL_PROGRAMS),
    CSIDL_NAME_ENTRY(CSIDL_CONTROLS),
    CSIDL_NAME_ENTRY(CSIDL_PRINTERS),
    CSIDL_NAME_ENTRY(CSIDL_PERSONAL),
    CSIDL_NAME_ENTRY(CSIDL_FAVORITES),
    CSIDL_NAME_ENTRY(CSIDL_STARTUP),
    CSIDL_NAME_ENTRY(CSIDL_RECENT),
    CSIDL_NAME_ENTRY(CSIDL_SENDTO),
    CSIDL_NAME_ENTRY(CSIDL_BITBUCKET),
    CSIDL_NAME_ENTRY(CSIDL_STARTMENU),
    CSIDL_NAME_ENTRY(CSIDL_DESKTOPDIRECTORY),
    CSIDL_NAME_ENTRY(CSIDL_DRIVES),
    CSIDL_NAME_ENTRY(CSIDL_NETWORK),
    CSIDL_NAME_ENTRY(CSIDL_NETHOOD),
    CSIDL_NAME_ENTRY(CSIDL_FONTS),
    CSIDL_NAME_ENTRY(CSIDL_TEMPLATES),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_STARTMENU),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_PROGRAMS),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_STARTUP),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_DESKTOPDIRECTORY),
    CSIDL_NAME_ENTRY(CSIDL_APPDATA),
    CSIDL_NAME_ENTRY(CSIDL_PRINTHOOD),
    CSIDL_NAME_ENTRY(CSIDL_ALTSTARTUP),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_ALTSTARTUP),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_FAVORITES),
    CSIDL_NAME_ENTRY(CSIDL_INTERNET_CACHE),
    CSIDL_NAME_ENTRY(CSIDL_COOKIES),
    CSIDL_NAME_ENTRY(CSIDL_HISTORY),
    CSIDL_NAME_ENTRY(CSIDL_LOCAL_APPDATA),
    CSIDL_NAME_ENTRY(CSIDL_COMMON_APPDATA),
    CSIDL_NAME_ENTRY(CSIDL_PROGRAM_FILES_COMMON)
};


CGetEnvPath::CGetEnvPath()
{
    m_hModule = NULL;
    m_pfnGetLongPathName = NULL;

    while (true)
    {
        m_hModule = LoadLibrary(TEXT("Kernel32.dll"));

        if (m_hModule == NULL)
            break;

        m_pfnGetLongPathName = 
            (pfnGetLongPathNameW)GetProcAddress(m_hModule, "GetLongPathNameW");

        break;
    }
}

CGetEnvPath::~CGetEnvPath()
{
    m_pfnGetLongPathName = NULL;

    if (m_hModule != NULL)
        FreeLibrary( m_hModule );
}

CString CGetEnvPath::GetRealPath(LPCTSTR pszEnvPath)
{

    CString strResult;

    if (pszEnvPath == NULL)
    {
        strResult = _T("");
        goto Exit0;
    }
    
    int nFirstPos  = 0; 
    int nSecondPos = 0;
    BOOL bFind = FALSE;

    strResult = pszEnvPath;

    bFind = FindEnvironmentPos(strResult, nFirstPos, nSecondPos);

    if (bFind == TRUE)
    {
        CString strLeft       ;
        CString strRight      ;
        CString strEnvName    ;
        CString strEnvVariable;


        strLeft    = strResult.Left(nFirstPos);
        strRight   = strResult.Mid (nSecondPos + 1);
        strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);

        strEnvVariable = GetEnvVariable(strEnvName);
        if (strEnvVariable.GetLength() <= 0)
            strEnvVariable = GetFolderPath( strEnvName );

        if(strEnvVariable.Find(_T(';')) == -1)
            strResult = CombinationPath(strLeft, strEnvVariable, strRight);
        else
        {
            //SplitString()
            std::vector<CString> strvec;
            SplitString(strEnvVariable,_T(';'), strvec);
            strResult = _T("");
            for(std::vector<CString>::iterator it = strvec.begin();it != strvec.end(); it++)
            {
                strResult += CombinationPath(strLeft, (*it), strRight);
                strResult += _T(";");
            }
            
        }

        
        bFind = FindEnvironmentPos(strResult, nFirstPos, nSecondPos);
    }
Exit0:
    return strResult;
}

CString CGetEnvPath::CombinationPath(const CString& strLeft, 
    const CString& strMiddle, 
    const CString& strRight)
{
    CString strResult;
    const int nCount = 3;
    const CString* pStr[nCount] = 
        { &strLeft, &strMiddle, &strRight};

    for (int i = 0; i < nCount; i++)
    {
        const CString& str = *pStr[i];

        if (strResult.GetLength() <= 0)
            strResult += str;
        else
        {
            if (str.GetLength() <= 0)
                continue;

            if (strResult[strResult.GetLength() - 1] == TEXT('\\'))
                strResult.Delete(strResult.GetLength() - 1);

            if (str[0] != TEXT('\\'))
            {
                strResult += TEXT('\\');
            }

            strResult += str;
        }
    }

    return strResult;
}

BOOL CGetEnvPath::FindEnvironmentPos(LPCTSTR pszEnvPath, int& nFirstPos, int& nSecondPos)
{
    BOOL bRet = TRUE;
    if (pszEnvPath == NULL)
    {
        bRet = FALSE;
        goto Exit0;
    }
    
    const TCHAR cSpil = TEXT('%');

    const TCHAR* pFirstPos  = NULL;
    const TCHAR* pSecondPos = NULL;

    pFirstPos = _tcschr(pszEnvPath, cSpil);
    if (pFirstPos == NULL)
    {
        bRet = FALSE;
        goto Exit0;
    }

    pSecondPos = _tcschr(pFirstPos + 1, cSpil);
    if (pSecondPos == NULL)
    {
        bRet = FALSE;
        goto Exit0;
    }

    nFirstPos  = int ( pFirstPos  - pszEnvPath );
    nSecondPos = int ( pSecondPos - pszEnvPath );
Exit0:
    return bRet;
}

CString CGetEnvPath::GetEnvVariable(LPCTSTR pszName)
{
    CString strResult;
	WCHAR wcsTemp[MAX_PATH] = {0}; 
    
    TCHAR szBuffer[MAX_PATH] = { 0 };
    TCHAR szLongPathBuffer[MAX_PATH] = { 0 };

    if ( CString("systemdrive").CompareNoCase(pszName) == 0 )
    {
        //UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);
        //strResult = GetDrive(_T(""));
        UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

        if (uResult > 3 && szBuffer[1] == TEXT(':') && szBuffer[2] == TEXT('\\'))
        {
            szBuffer[2] = TEXT('\0');
            strResult = szBuffer;
            goto Exit0;         
        }
    }
    else if ( CString("program").CompareNoCase(pszName) == 0 )
    {
        //UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);
        //strResult = GetDrive(_T(""));
        UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

        if (uResult > 3 && szBuffer[1] == TEXT(':') && szBuffer[2] == TEXT('\\'))
        {
            szBuffer[3] = TEXT('\0');
            strResult = szBuffer;
            strResult += L"program files";
            goto Exit0;         
        }
    }
    else if ( CString("boot").CompareNoCase(pszName) == 0 )
    {
        //UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);
        //strResult = GetDrive(_T(""));
        UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

        if (uResult > 3 && szBuffer[1] == TEXT(':') && szBuffer[2] == TEXT('\\'))
        {
            szBuffer[3] = TEXT('\0');
            strResult = szBuffer;
            strResult += L"boot";
            goto Exit0;         
        }
    }
    else if ( CString("recovery").CompareNoCase(pszName) == 0 )
    {
        //UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);
        //strResult = GetDrive(_T(""));
        UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

        if (uResult > 3 && szBuffer[1] == TEXT(':') && szBuffer[2] == TEXT('\\'))
        {
            szBuffer[3] = TEXT('\0');
            strResult = szBuffer;
            strResult += L"recovery";
            goto Exit0;         
        }
    }
    else if ( CString("systemroot").CompareNoCase(pszName) == 0 )
    {
       
        UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

        if (uResult > 0)
        {
            strResult = szBuffer;
            goto Exit0;         
        }
    }
    else if(CString("windir").CompareNoCase(pszName) == 0)
    {
        UINT uResult = GetWindowsDirectory(szBuffer, MAX_PATH);

        if(szBuffer[wcslen(szBuffer)-1] == _T('\\'))
            szBuffer[wcslen(szBuffer)-1] = _T('\0');

        strResult = szBuffer;
        goto Exit0;
    }
    else if ( CString("systemrecycled").CompareNoCase(pszName) == 0 )
    {
        SYSTEM_VERSION     m_eSysVer;
        KAEGetSystemVersion(&m_eSysVer);
        CString StrSuffix;
        if(m_eSysVer == enumSystem_Win_7)
        {
            StrSuffix = _T("\\$RECYCLE.BIN");
        }
        else
            StrSuffix = _T("\\Recycled");
        strResult = GetDrive(StrSuffix);

        goto Exit0;
    }
	else if ( CString("minidump").CompareNoCase(pszName) == 0 )
	{
		DWORD len = sizeof(szLongPathBuffer);
		GetRegistryValue(HKEY_LOCAL_MACHINE,
			L"SYSTEM\\CurrentControlSet\\Control\\CrashControl",
			L"MinidumpDir",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if(wcslen(szLongPathBuffer) == 0)
			strResult = L"%minidump%";
		else
		{
			int nFirstPos  = 0; 
			int nSecondPos = 0;
			BOOL bFind = FALSE;
			bFind = FindEnvironmentPos(szLongPathBuffer, nFirstPos, nSecondPos);
			if(bFind)
			{
				CString strLeft       ;
				CString strRight      ;
				CString strEnvName    ;
				CString strEnvVariable;
				
				strResult = szLongPathBuffer;
				strLeft    = strResult.Left(nFirstPos);
				strRight   = strResult.Mid (nSecondPos + 1);
				strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);
				TCHAR szTempBuf[MAX_PATH];
				DWORD dwResult = GetEnvironmentVariable(strEnvName.GetBuffer(), szTempBuf, MAX_PATH);
				if (dwResult > 0)
					strEnvVariable = szTempBuf;

				strResult = CombinationPath(strLeft, strEnvVariable, strRight);

			}
			else
				strResult = szLongPathBuffer;

			goto Exit0;

		}
	}
	else if ( CString("memdump").CompareNoCase(pszName) == 0 )
	{
		DWORD len = sizeof(szLongPathBuffer);
		GetRegistryValue(HKEY_LOCAL_MACHINE,
			L"SYSTEM\\CurrentControlSet\\Control\\CrashControl",
			L"DumpFile",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if(wcslen(szLongPathBuffer) == 0)
			strResult = L"%memdump%";
		else
		{
			int nFirstPos  = 0; 
			int nSecondPos = 0;
			BOOL bFind = FALSE;
			bFind = FindEnvironmentPos(szLongPathBuffer, nFirstPos, nSecondPos);
			if(bFind)
			{
				CString strLeft       ;
				CString strRight      ;
				CString strEnvName    ;
				CString strEnvVariable;

				strResult = szLongPathBuffer;
				strLeft    = strResult.Left(nFirstPos);
				strRight   = strResult.Mid (nSecondPos + 1);
				strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);
				TCHAR szTempBuf[MAX_PATH];
				DWORD dwResult = GetEnvironmentVariable(strEnvName.GetBuffer(), szTempBuf, MAX_PATH);
				if (dwResult > 0)
					strEnvVariable = szTempBuf;

				strResult = CombinationPath(strLeft, strEnvVariable, L"");
				strResult += strRight;
			}
			else
				strResult = szLongPathBuffer;
			if (GetFileAttributes(strResult.GetBuffer()) != FILE_ATTRIBUTE_DIRECTORY)
			{
				WCHAR szTempBuffer[MAX_PATH] = {0};
				wcscpy_s(szTempBuffer, MAX_PATH - 1, strResult.GetBuffer());
				::PathRemoveFileSpec(szTempBuffer);
				strResult = szTempBuffer;
			}

			goto Exit0;

		}
	}
	else if (CString("archivefiles").CompareNoCase(pszName) == 0)
	{
		DWORD len = sizeof(szLongPathBuffer);
		GetRegistryValue(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Windows Error Reporting Archive Files",
			L"Folder",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if (wcslen(szLongPathBuffer) == 0)
		{
			strResult = L"%archivefiles%";
		}
		else
		{
			int nFirstPos  = 0; 
			int nSecondPos = 0;
			BOOL bFind = FALSE;
			bFind = FindEnvironmentPos(szLongPathBuffer, nFirstPos, nSecondPos);
			if(bFind)
			{
				CString strLeft       ;
				CString strRight      ;
				CString strEnvName    ;
				CString strEnvVariable;

				strResult = szLongPathBuffer;
				strLeft    = strResult.Left(nFirstPos);
				strRight   = strResult.Mid (nSecondPos + 1);
				strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);
				TCHAR szTempBuf[MAX_PATH];
				DWORD dwResult = GetEnvironmentVariable(strEnvName.GetBuffer(), szTempBuf, MAX_PATH);
				if (dwResult > 0)
					strEnvVariable = szTempBuf;

				strResult = CombinationPath(strLeft, strEnvVariable, L"");
				strResult += strRight;
			}
			else
			{
				strResult = szLongPathBuffer;
			}
			goto Exit0;

		}
	}
	else if (CString("queuefiles").CompareNoCase(pszName) == 0)
	{
		DWORD len = sizeof(szLongPathBuffer);
		GetRegistryValue(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Windows Error Reporting Queue Files",
			L"Folder",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if (wcslen(szLongPathBuffer) == 0)
		{
			strResult = L"%queuefiles%";
		}
		else
		{
			int nFirstPos  = 0; 
			int nSecondPos = 0;
			BOOL bFind = FALSE;
			bFind = FindEnvironmentPos(szLongPathBuffer, nFirstPos, nSecondPos);
			if(bFind)
			{
				CString strLeft       ;
				CString strRight      ;
				CString strEnvName    ;
				CString strEnvVariable;

				strResult = szLongPathBuffer;
				strLeft    = strResult.Left(nFirstPos);
				strRight   = strResult.Mid (nSecondPos + 1);
				strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);
				TCHAR szTempBuf[MAX_PATH];
				DWORD dwResult = GetEnvironmentVariable(strEnvName.GetBuffer(), szTempBuf, MAX_PATH);
				if (dwResult > 0)
					strEnvVariable = szTempBuf;

				strResult = CombinationPath(strLeft, strEnvVariable, L"");
				strResult += strRight;
			}
			else
			{
				strResult = szLongPathBuffer;
			}
			goto Exit0;

		}
	}
	else if (CString("systemarchivefiles").CompareNoCase(pszName) == 0)
	{
		DWORD len = sizeof(szLongPathBuffer);
		GetRegistryValue(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Windows Error Reporting System Archive Files",
			L"Folder",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if (wcslen(szLongPathBuffer) == 0)
		{
			strResult = L"%systemarchivefiles%";
		}
		else
		{
			int nFirstPos  = 0; 
			int nSecondPos = 0;
			BOOL bFind = FALSE;
			bFind = FindEnvironmentPos(szLongPathBuffer, nFirstPos, nSecondPos);
			if(bFind)
			{
				CString strLeft       ;
				CString strRight      ;
				CString strEnvName    ;
				CString strEnvVariable;

				strResult = szLongPathBuffer;
				strLeft    = strResult.Left(nFirstPos);
				strRight   = strResult.Mid (nSecondPos + 1);
				strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);
				TCHAR szTempBuf[MAX_PATH];
				DWORD dwResult = GetEnvironmentVariable(strEnvName.GetBuffer(), szTempBuf, MAX_PATH);
				if (dwResult > 0)
					strEnvVariable = szTempBuf;

				strResult = CombinationPath(strLeft, strEnvVariable, L"");
				strResult += strRight;
			}
			else
			{
				strResult = szLongPathBuffer;
			}
			goto Exit0;

		}
	}
	else if (CString("systemqueuefiles").CompareNoCase(pszName) == 0)
	{
		DWORD len = sizeof(szLongPathBuffer);
		GetRegistryValue(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Windows Error Reporting System Queue Files",
			L"Folder",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if (wcslen(szLongPathBuffer) == 0)
		{
			strResult = L"%systemqueuefiles%";
		}
		else
		{
			int nFirstPos  = 0; 
			int nSecondPos = 0;
			BOOL bFind = FALSE;
			bFind = FindEnvironmentPos(szLongPathBuffer, nFirstPos, nSecondPos);
			if(bFind)
			{
				CString strLeft       ;
				CString strRight      ;
				CString strEnvName    ;
				CString strEnvVariable;

				strResult = szLongPathBuffer;
				strLeft    = strResult.Left(nFirstPos);
				strRight   = strResult.Mid (nSecondPos + 1);
				strEnvName = strResult.Mid(nFirstPos + 1, nSecondPos - nFirstPos - 1);
				TCHAR szTempBuf[MAX_PATH];
				DWORD dwResult = GetEnvironmentVariable(strEnvName.GetBuffer(), szTempBuf, MAX_PATH);
				if (dwResult > 0)
					strEnvVariable = szTempBuf;

				strResult = CombinationPath(strLeft, strEnvVariable, L"");
				strResult += strRight;
			}
			else
			{
				strResult = szLongPathBuffer;
			}
			goto Exit0;

		}
	}
    else if ( CString("tudou").CompareNoCase(pszName) == 0 )
    {
         DWORD len = sizeof(szLongPathBuffer);
         GetRegistryValue(HKEY_LOCAL_MACHINE,
             L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\飞速土豆",
             L"UninstallString",
             NULL,
             (LPBYTE)szLongPathBuffer,
             &len
             );
         if(wcslen(szLongPathBuffer) == 0)
            strResult = L"%tudou%";
         else
         {
             ::PathRemoveFileSpec(szLongPathBuffer);
             ::PathAppend(szLongPathBuffer,L"tudouva.ini");
             IniEditor IniEdit;
             IniEdit.SetFile(szLongPathBuffer);
             std::wstring tmpPath = IniEdit.ReadString(L"public",L"savedirectory");
             if(tmpPath.length() <= 3)
                 strResult = L"%tudou%";
             else 
                 strResult = tmpPath.c_str();

         }

        goto Exit0;
    }else if ( CString("qvod").CompareNoCase(pszName) == 0 )
    {
        DWORD len = sizeof(szBuffer);
        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\QvodPlayer\\Insert",
            L"Insertpath",
            NULL,
            (LPBYTE)szLongPathBuffer,
            &len
            );
        if(wcslen(szLongPathBuffer) == 0)
            strResult = L"%qvod%";
        else
        {
            //::PathRemoveFileSpec(szLongPathBuffer);
            ::PathAppend(szLongPathBuffer,L"QvodPlayer.xml");
            TiXmlDocument xmlDoc;
            const TiXmlElement *pXmlQvodPlayer = NULL;
            const TiXmlElement *pXmlGeneral = NULL;
            strResult = L"%qvod%";
            if (!xmlDoc.LoadFile(UnicodeToAnsi(szLongPathBuffer).c_str(), TIXML_ENCODING_UTF8))
                goto Exit0;
            pXmlQvodPlayer = xmlDoc.FirstChildElement("QvodPlayer");
            if (!pXmlQvodPlayer)
                goto Exit0;
            pXmlGeneral = pXmlQvodPlayer->FirstChildElement("General");
            if (pXmlGeneral)
            {
                const char* szTmp = pXmlGeneral->Attribute("Defaultsavepath");
                if(!szTmp)
                    strResult = L"%qvod%";
                else 
                    strResult = Utf8ToUnicode(szTmp).c_str();
            }

        }

        goto Exit0;
    }
    else if ( CString("xunleikankan").CompareNoCase(pszName) == 0 )
    {
        DWORD len = sizeof(szBuffer);
        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Thunder network\\Xmp",
            L"storepath",
            NULL,
            (LPBYTE)szBuffer,
            &len
            );
        if(wcslen(szBuffer) == 0)
            strResult = L"%xunleikankan%";
        else
        {
//            wcscat(szBuffer,L"\\VODCache");
            strResult =szBuffer;
        }

        goto Exit0;
    }
    else if ( CString("youku").CompareNoCase(pszName) == 0 )
    {
        DWORD len = sizeof(szBuffer);
        GetRegistryValue(HKEY_CURRENT_USER,
            L"SOFTWARE\\youku\\iKuAcc",
            L"CachePath",
            NULL,
            (LPBYTE)szBuffer,
            &len
            );
        if(wcslen(szBuffer) == 0)
            strResult = L"%youku%";
        else
        {
            strResult =szBuffer;
            if(strResult.Find(L"\\Desktop")!=-1)
                strResult = L"%youku%";
        }

        goto Exit0;
    }
    else if ( CString("ku6").CompareNoCase(pszName) == 0 )
    {
        DWORD len = sizeof(szLongPathBuffer);
        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ku6SpeedUpper",
            L"UninstallString",
            NULL,
            (LPBYTE)szLongPathBuffer,
            &len
            );
        if(wcslen(szLongPathBuffer) == 0)
        {
            CString StrSuffix = _T("\\kvideo_cache");;
            strResult = GetDrive(StrSuffix);
        }
        else
        {
            ::PathRemoveFileSpec(szLongPathBuffer);
            ::PathAppend(szLongPathBuffer,L"Ku6SpeedUpper.ini");
            IniEditor IniEdit;
            IniEdit.SetFile(szLongPathBuffer);
            std::wstring tmpPath = IniEdit.ReadString(L"Settings",L"BufPath");
            if(tmpPath.length() > 0)
                strResult = tmpPath.c_str();
            else
            {
                CString StrSuffix = _T("\\kvideo_cache");;
                strResult = GetDrive(StrSuffix);
                
            }
        }
        goto Exit0;
    }
    else if ( CString("ppstream").CompareNoCase(pszName) == 0 )
    {
        CString strTmp = GetFolderPath(L"CSIDL_APPDATA");
        strTmp += L"\\ppstream\\psnetwork.ini";
        if(!::PathFileExists(strTmp.GetBuffer()))
        {
            strTmp = L"";
            GetWindowsDirectory(szLongPathBuffer,MAX_PATH);
            strTmp = szLongPathBuffer;
            strTmp +=  L"\\psnetwork.ini";
        }
        IniEditor IniEdit;
        IniEdit.SetFile(strTmp.GetBuffer());
        std::wstring tmpPath = IniEdit.ReadString(L"vodnet",L"pgfpath");
        if(tmpPath.length()>0)
          ::PathRemoveFileSpec((LPWSTR)tmpPath.c_str());
        else
        {
            tmpPath = IniEdit.ReadString(L"vodnet",L"pgfpath1");
            if(tmpPath.length()>0)
                tmpPath.erase(tmpPath.length()-1);
        }
        strResult = tmpPath.c_str();
        goto Exit0;
    }
    else if ( CString("pptv").CompareNoCase(pszName) == 0 )
    {
        CString strTmp = GetFolderPath(L"CSIDL_COMMON_APPDATA");
        strTmp += L"\\PPLive\\Core\\config.ini";
        IniEditor IniEdit;
        IniEdit.SetFile(strTmp.GetBuffer());
        std::wstring tmpPath = IniEdit.ReadString(L"peer",L"cachepath");
        std::string utf8str = KUTF16_To_ANSI(tmpPath.c_str());
        tmpPath = Utf8ToUnicode(utf8str.c_str());
        if(tmpPath.length() == 0)
            strResult = L"%pptv%";
        else
            strResult = tmpPath.c_str();
        goto Exit0;
    }
    else if ( CString("fengxing").CompareNoCase(pszName) == 0 )
    {
        CString strTmp = GetFolderPath(L"CSIDL_PERSONAL");
        int a = strTmp.ReverseFind(L'\\');
        strTmp = strTmp.Mid(0,strTmp.ReverseFind(L'\\'));
        strTmp += L"\\funshion.ini";
        IniEditor IniEdit;
        IniEdit.SetFile(strTmp.GetBuffer());
        std::wstring tmpPath = IniEdit.ReadString(L"FILE_PATH",L"MEDIA_PATH");
        if(tmpPath.length() == 0)
            strResult = L"%fengxing%";
        else
            strResult = tmpPath.c_str();
        goto Exit0;
    }
    else if ( CString("qqlive").CompareNoCase(pszName) == 0 )
    {
       CString strTmp = GetFolderPath(L"CSIDL_APPDATA");
       strTmp += L"\\Tencent\\QQLive\\user.ini";
       IniEditor IniEdit;
       IniEdit.SetFile(strTmp.GetBuffer());
       std::wstring tmpPath = IniEdit.ReadString(L"Cache",L"Directory");
       if(tmpPath.length()>0)
       {
           tmpPath.erase(tmpPath.length()-1);
           strResult = tmpPath.c_str();
       }
       else strResult = L"%qqlive%";
       
       goto Exit0;
    }
	else if ( CString("firefox").CompareNoCase(pszName) == 0 )
	{
		WCHAR szPath[MAX_PATH] = {0};
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
		std::wstring strPath;
		std::wstring strTemp;
		strPath = szPath;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		if (strPath.rfind(L'\\') != strPath.size())
		{
			strPath += L"\\";
		}
		strPath += L"Mozilla\\Firefox\\profiles.ini";
		IniEditor inif;
		inif.SetFile(strPath.c_str());
		strTemp = inif.ReadString(L"Profile0", L"Path");
		if(strTemp.find(L"//") != -1)
		{
			strTemp.replace(strTemp.find(L"//"), 1, L"\\");
		}

		ZeroMemory(szPath, MAX_PATH);
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);
		strPath = szPath;
		if (strPath.rfind(L'\\') != strPath.size())
		{
			strPath += L"\\";
		}
		strPath += L"Mozilla\\Firefox\\";
		strPath += strTemp;
		strPath += L"\\cache";

		strResult = strPath.c_str();
		goto Exit0;
	}
    else if ( CString("sogou").CompareNoCase(pszName) == 0 )
    {
        std::wstring strPath;
        std::wstring strTemp;
        WCHAR szPath[MAX_PATH] = {0};
        SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
        strTemp = szPath;
        if ( strTemp.rfind(L'\\') != strTemp.size())
        {
            strTemp += L"\\";
        }
        strTemp += L"SogouExplorer\\CommCfg.xml";

        strResult = L"%sogou%";
        TiXmlDocument xmlDoc;
        const TiXmlElement *pXmlSogou = NULL;
        const TiXmlElement *pXmlGeneral = NULL;
        if (!xmlDoc.LoadFile(UnicodeToAnsi(strTemp.c_str()).c_str(), TIXML_ENCODING_UTF8))
            goto Exit0;
        pXmlSogou = xmlDoc.FirstChildElement("main");
        if (!pXmlSogou)
            goto Exit0;
        pXmlGeneral = pXmlSogou->FirstChildElement("Item");
        if (pXmlGeneral && pXmlGeneral->Attribute("videoacccachepath"))
        {
            const char* szTmp = pXmlGeneral->Attribute("videoacccachepath");
            if(!szTmp)
                strResult = L"%sogou%";
            else 
                strResult = Utf8ToUnicode(szTmp).c_str();
        }
        // 过滤根目录
        if (strResult.GetLength() <= 3)
            strResult = L"%sogou%";
        goto Exit0;
    }
    else if (CString("usertemp").CompareNoCase(pszName) == 0)
    {
        TCHAR szBuffer[MAX_PATH] = { 0 };
        CString strPath;
        CString strEnvName = L"%TEMP%";
        CString strEnvPath;

        ExpandEnvironmentStrings(strEnvName, strEnvPath.GetBuffer(MAX_PATH), MAX_PATH);
        strEnvPath.ReleaseBuffer();
        
        StandardPath(strEnvPath);
    
        UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

        if (uResult < 3 || szBuffer[1] != TEXT(':') || szBuffer[2] != TEXT('\\'))
        {
            goto Exit0;     
        }
        szBuffer[3] = TEXT('\0');
        strPath += szBuffer;
        memset(szBuffer, 0, sizeof(szBuffer));
        DWORD MaxSize = sizeof(szBuffer);
        GetUserName(szBuffer, &MaxSize);

        if (KGetWinVersion() >= 4) 
        {
            strPath += L"Users\\"; 
            strPath += szBuffer; 
            strPath += L"\\AppData\\Local\\Temp\\";
        }
        else
        {
            strPath += L"Documents and Settings\\"; 
            strPath += szBuffer; 
            strPath += L"\\Local Settings\\Temp\\";
        }
        //
        if (strPath.CompareNoCase(strEnvPath) == 0 
            || strPath.Find(strEnvPath) != -1
            || strEnvPath.Find(strPath) != -1)
        {
            strResult = L"";
        }
        else
        {
            strResult = strPath;
        }
        goto Exit0;
    }
    else if (CString("wintemp").CompareNoCase(pszName) == 0)
    {
        DWORD len = sizeof(szLongPathBuffer);
        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
            L"TEMP",
            NULL,
            (LPBYTE)szLongPathBuffer,
            &len
            );
        if (wcslen(szLongPathBuffer) == 0)
        {
            strResult = L"";
        }
        else
        {
            if (CString("%SystemRoot%\\TEMP").CompareNoCase(szLongPathBuffer) == 0)
            {
                strResult = L"";
            }
            else
            {
                //解析环境变量
                CString strWinPath = szLongPathBuffer;

                int nFirstPos  = 0; 
                int nSecondPos = 0;
                BOOL bFind = FALSE;

                bFind = FindEnvironmentPos(strWinPath, nFirstPos, nSecondPos);
                
                if (bFind)
                {
                    CString strEnvName;
                    CString strEnvPath;
                    CString strLeft;
                    CString strRight;
                    strLeft = strWinPath.Left(nFirstPos);
                    strEnvName = strWinPath.Mid(nFirstPos, nSecondPos - nFirstPos + 1);
                    strRight = strWinPath.Mid(nSecondPos + 1);

                    ExpandEnvironmentStrings(strEnvName, strEnvPath.GetBuffer(MAX_PATH), MAX_PATH);
                    strEnvPath.ReleaseBuffer();
                    strResult += strLeft;
                    strResult += strEnvPath;
                    strResult += strRight;

                    UINT uResult = GetWindowsDirectory(szBuffer, MAX_PATH);

                    if(szBuffer[wcslen(szBuffer)-1] == _T('\\'))
                        szBuffer[wcslen(szBuffer)-1] = _T('\0');

                    CString strNormal = szBuffer;

                    strNormal += L"\\Temp\\";

                    StandardPath(strResult);

                    // 根目录跳过 和 子目录过滤，同时过滤 
                    if (strResult.GetLength() <= 3 
                        || strResult.Find(strNormal) != -1
                        || strNormal.Find(strResult) != -1)
                    {
                        strResult = L"";
                    }

                }
                else
                {
                    strResult = szLongPathBuffer;
                }
            }
        }
        goto Exit0;
    }
    else if (CString("office").CompareNoCase(pszName) == 0)
    {
        CString strDriverName;
        std::vector<CString> vecDrive;
        std::vector<CString>::iterator ite;

        TCHAR   szDriverName[500]; 

        DWORD nLength = GetLogicalDriveStrings(sizeof(szDriverName), szDriverName); 

        for (int i = 0; i < (int)nLength; i++) 
        { 
            if (szDriverName[i] != L'\0')
            {
                strDriverName += szDriverName[i]; 
            }
            else 
            { 
                strDriverName = strDriverName.Left(strDriverName.GetLength() - 1); 
                vecDrive.push_back(strDriverName); 
                strDriverName = ""; 
            } 
        }

        if (vecDrive.size() <= 0)
            goto Exit0;

        // 枚举盘符
        for (ite = vecDrive.begin(); ite != vecDrive.end(); ++ite)
        {
            CString strPath = (*ite) + _T("\\MSOCache");
            BOOL bRet = PathFileExists(strPath);

            if (bRet) 
            {
                strResult = strPath;
                break;
            }
        }
        goto Exit0;
    }
    else if (CString("qqmusic").CompareNoCase(pszName) == 0)
    {
        WCHAR szPath[MAX_PATH] = {0};
        std::wstring strPath;
        std::wstring strTemp;

        SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
        strPath = szPath;

        if (strPath.rfind(L'\\') != strPath.size())
        {
            strPath += L"\\";
        }
        strPath += L"Tencent\\QQMusic\\CachePath.ini";
        IniEditor inif;
        inif.SetFile(strPath.c_str());
        strTemp = inif.ReadString(L"Cache", L"Path");
        if(strTemp.find(L"//") != -1)
        {
            strTemp.replace(strTemp.find(L"//"), 1, L"\\");
        }

        strResult += strTemp.c_str();

        if (strResult.IsEmpty())
            goto Exit0;

        if (strResult[strResult.GetLength() - 1] != _T('\\'))
            strResult += _T('\\');

        strResult += L"musiccache";

        goto Exit0;
    }
    else if (CString("kuwo").CompareNoCase(pszName) == 0)
    {
        WCHAR szPath[MAX_PATH] = {0};
        std::wstring strPath;
        std::wstring strTemp;

        SHGetSpecialFolderPath(NULL, szPath, CSIDL_COMMON_APPDATA, FALSE);
        strPath = szPath;

        if (strPath.rfind(L'\\') != strPath.size())
        {
            strPath += L"\\";
        }
        strPath += L"kuwo\\conf\\user\\config.ini";
        IniEditor inif;

        inif.SetFile(strPath.c_str());
        strTemp = inif.ReadString(L"Setting", L"temppath");
        if(strTemp.find(L"//") != -1)
        {
            strTemp.replace(strTemp.find(L"//"), 1, L"\\");
        }

        strResult = strTemp.c_str();

        goto Exit0;
    }
    else if (CString("ttplayer").CompareNoCase(pszName) == 0)
    {
        DWORD len = sizeof(szLongPathBuffer);
        TCHAR szDefault[MAX_PATH * 2] = { 0 };
        SHGetSpecialFolderPath(NULL, szDefault, CSIDL_APPDATA, FALSE);
        PathAppend(szDefault, L"TTPlayer\\cache");

        GetRegistryValue(HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TTPlayer",
            L"AppPath",
            NULL,
            (LPBYTE)szLongPathBuffer,
            &len
            );
        if(wcslen(szLongPathBuffer) == 0)
        {
            strResult = L"%ttplayer%";
        }
        else
        {
            ::PathRemoveFileSpec(szLongPathBuffer);
            ::PathAppend(szLongPathBuffer,L"TTPlayer.xml");
            TiXmlDocument xmlDoc;
            const TiXmlElement *pXmlTTPlayer = NULL;
            const TiXmlElement *pXmlGeneral = NULL;
            if (!xmlDoc.LoadFile(UnicodeToAnsi(szLongPathBuffer).c_str(), TIXML_ENCODING_UTF8))
                goto Exit0;

            pXmlTTPlayer = xmlDoc.FirstChildElement("ttplayer");
            if (!pXmlTTPlayer)
                goto Exit0;

            pXmlGeneral = pXmlTTPlayer->FirstChildElement("Network");
            if (pXmlGeneral)
            {
                const char* szTmp = pXmlGeneral->Attribute("CacheFolder");
                if(!szTmp)
                    strResult = L"%ttplayer%";
                else 
                    strResult = Utf8ToUnicode(szTmp).c_str();
            }
            
        }

        if (strResult.Find(szDefault) != -1 || strResult.GetLength() <= 3)
            strResult = L"%ttplayer%";

        goto Exit0;
    }
    else if (CString("ksafe").CompareNoCase(pszName) == 0)
    {
//         WCHAR szPath[MAX_PATH] = { 0 };
//         WCHAR szCmdline[MAX_PATH * 2] = { 0 };
//         ::GetModuleFileName(NULL, szPath, MAX_PATH);
// 
//         ::PathRemoveFileSpec(szPath);
//         std::wstring strIniPath = szPath;
//         std::wstring strTemp;
// 
//         strIniPath += L"\\cfg\\vulfix.ini";
// 
// //         IniEditor inif;
// // 
// //         inif.SetFile(strIniPath.c_str());
// //         strTemp = inif.ReadString(L"Main", L"downpath");
// //         if(strTemp.find(L"//") != -1)
// //         {
// //             strTemp.replace(strTemp.find(L"//"), 1, L"\\");
// //         }
// 
//         if (strTemp.length() == 0)
//         {
//             strResult = szPath;
//             strResult += L"\\hotfix";
//         }
//         else
//         {
//             strResult = strTemp.c_str();
//         }
        goto Exit0;
    }
    DWORD dwResult = GetEnvironmentVariable(pszName, szBuffer, MAX_PATH);
    if (dwResult > 0)
    {
        dwResult = 0;

        LPSTR pFileName = NULL;

        if (m_pfnGetLongPathName != NULL) 
            dwResult = m_pfnGetLongPathName(szBuffer, szLongPathBuffer, MAX_PATH);

        strResult = dwResult ? szLongPathBuffer : szBuffer;
    }
Exit0:
   
    return strResult;
}

CString CGetEnvPath::GetFolderPath(LPCTSTR pszName)
{
    CString strResult;
    TCHAR szBuffer[MAX_PATH] = { 0 };

    int nFolder = -1;

    int nCount = sizeof(g_csidlnamelist) / sizeof(g_csidlnamelist[0]);

    for (int i = 0; i < nCount; i++)
    {
        if (_tcscmp( pszName, g_csidlnamelist[i].pszCsidlName) == 0)
        {
            nFolder = g_csidlnamelist[i].nFolder;
            break;
        }
    }

    if (nFolder == -1)
        goto Exit0;
    
    if ( ::SHGetSpecialFolderPath( NULL, szBuffer, nFolder, FALSE) )
        strResult = szBuffer;
Exit0:
    return strResult;
}

CString CGetEnvPath::GetDrive(CString suffix)  //根据路径．获取只读区域卷标名称
{
    int nPos = 0;
    CString strDrive = "?:";
    DWORD dwDriveList = ::GetLogicalDrives();
    CString strTmp;

    while (dwDriveList)
    {
        if (dwDriveList & 1)
        {	
            strDrive.SetAt (0, 'A' + nPos);
            ///判断是移动磁盘
            {
                int nDiskType = GetDriveType(strDrive);   
                switch(nDiskType)
                {   
                case  DRIVE_NO_ROOT_DIR:
                    break;
                case  DRIVE_REMOVABLE: 
                    break;
                case  DRIVE_FIXED: 
                    {
                        strTmp += strDrive;
                        strTmp += suffix;
                        if(::PathIsDirectory(strTmp.GetBuffer()))
                            return strTmp;
                        strTmp = _T("");

                    }
                    break;
                case   DRIVE_REMOTE:
                    break;   
                case   DRIVE_CDROM:
                    break;
                }
            }
        }
        dwDriveList >>= 1;
        nPos++;
    }
    return strTmp;
}

BOOL CGetEnvPath::StandardPath(CString& strPath)
{
    BOOL bRet = FALSE;
    int retval = -1;
    
    if (strPath.IsEmpty())
        goto Clear0;

    strPath.Replace(_T("/"), _T("\\"));

    retval = GetLongPathName(strPath, strPath.GetBuffer(), MAX_PATH);

    if (retval == 0)
        goto Clear0;

    strPath.ReleaseBuffer();

    if (strPath[strPath.GetLength() - 1] != _T('\\'))
    {
        strPath += _T('\\');
    }

    bRet = TRUE;
Clear0:
    return bRet;
}
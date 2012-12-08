#include "stdafx.h"
#include "kvideocache.h"
#include "stubbornfiles.h"
#include "filtertool.h"
#include "misc/KppUserLog.h"

BOOL _ScanTudouCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	TCHAR szLongPathBuffer[MAX_PATH] = { 0 };
	CString strResult;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd = {0};
	FINDFILEDATA filedata = {0};
	HANDLE hSubFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfdSub = {0};
	DWORD len = sizeof(szLongPathBuffer);
	BOOL bRet =FALSE;
	CString strTemp;
	CString strTempEx;
	int nPos = -1;
	if ( g_bStop[nStopIndex] )
	{
		bRet = TRUE;
		goto clean0;
	}
	GetRegistryValue(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\飞速土豆",
		L"UninstallString",
		NULL,
		(LPBYTE)szLongPathBuffer,
		&len
		);
	if(wcslen(szLongPathBuffer) == 0)
	{
		GetRegistryValue(HKEY_CURRENT_USER,
			L"Software\\Tudou\\TudouVA",
			L"",
			NULL,
			(LPBYTE)szLongPathBuffer,
			&len
			);
		if (wcslen(szLongPathBuffer) == 0)
		{
			bRet = FALSE;
			goto clean0;
		}
		else
		{
			::PathAppend(szLongPathBuffer,L"tudouva.ini");
			IniEditor IniEdit;
			IniEdit.SetFile(szLongPathBuffer);
			std::wstring tmpPath = IniEdit.ReadString(L"public",L"savedirectory");
			if(tmpPath.length() == 0)
			{
				bRet = FALSE;
				goto clean0;
			}
			else
			{
				strResult = tmpPath.c_str();
			}
		}
	}
	else
	{
		::PathRemoveFileSpec(szLongPathBuffer);
		::PathAppend(szLongPathBuffer,L"tudouva.ini");
		IniEditor IniEdit;
		IniEdit.SetFile(szLongPathBuffer);
		std::wstring tmpPath = IniEdit.ReadString(L"public",L"savedirectory");
		if(tmpPath.length() == 0)
		{
			bRet = FALSE;
			goto clean0;
		}
		else
		{
			strResult = tmpPath.c_str();
		}
	}

//	strResult = L"D:\\work\\kclear_bug\\";
	strPath = strResult;

	nPos = strResult.ReverseFind('\\');
	if (nPos != strResult.GetLength() - 1)
	{
		strResult += L"\\";
	}
	strTemp = strResult;
	strTempEx = strResult;
	strTemp += L"*.*";
	hFile = FindFirstFile(strTemp, &wfd);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		bRet = FALSE;
		goto clean0;
	}

	do
	{
		if ( g_bStop[nStopIndex] )
		{
			bRet = TRUE;
			goto clean0;
		}
		CString strFileName;
		CString strTempResult;
		strFileName = wfd.cFileName;
		if (_wcsicmp(strFileName.GetBuffer(), L".") == 0 
			|| _wcsicmp(strFileName.GetBuffer(), L"..") == 0 
			|| !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			continue;
		}
		strTempResult = strResult;
		strTempResult += strFileName;

		strResult = strTempResult;

		strFileName += L"_*.*.tdd!";
		strTempResult += L"\\";
		strTempResult += strFileName;

		hSubFile = FindFirstFile(strTempResult, &wfdSub);
		if (hSubFile == INVALID_HANDLE_VALUE)
		{
			strResult = strTempEx;
			continue;
		}

		do
		{
			if ( g_bStop[nStopIndex] )
			{
				bRet = TRUE;
				goto clean0;
			}
			strFileName = wfdSub.cFileName;
			if (_wcsicmp(strFileName.GetBuffer(), L".") == 0 
				|| _wcsicmp(strFileName.GetBuffer(), L"..") == 0
				/*|| wfd.nFileSizeLow > 2 * 1024 * 1024*/)
			{
				continue;
			}
			strTempResult = strResult;
			filedata.pszPathName = strResult;
			filedata.pszFileNmae = strFileName;
			filedata.pszExtName = L"_*.*.tdd!";
			filedata.uFileSize   = wfdSub.nFileSizeHigh;
			filedata.uFileSize   = filedata.uFileSize << 32;
			filedata.uFileSize  |= wfdSub.nFileSizeLow;

			strTempResult +=L"\\";
			strTempResult += strFileName;

            if(CStubbornFiles::Instance().IsStubbornFile(strTempResult))
                continue;
            // 过滤
            CString strFilter = _T("%specfiles%");
            if (CFilterTool::Instance()._DoFindFuncMap(strFilter, strTempResult))
            {
                continue;
            }
            if (!CFilterTool::Instance().CheckTimeFilterByDay(wfdSub.ftLastWriteTime))
            {
                continue;
            }

			CString strLog = L"[Scan][TuDou]:";
			strLog += strTempResult;
			KppUserLog::UserLogInstance().WriteSingleLog(strLog.GetBuffer());
			//strResult = strTempResult;
			if (pTF)
			{
				pTF->TraverseFile(&filedata);
			}

		}while(FindNextFile(hSubFile, &wfdSub));
		strResult = strTempEx;

	}while(FindNextFile(hFile, &wfd));

	bRet = TRUE;
clean0:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (hSubFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hSubFile);
		hSubFile = INVALID_HANDLE_VALUE;
	}

	return TRUE;
}

BOOL _ScanQvodCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	WCHAR szLongPathBuffer[MAX_PATH] = {0};
	DWORD len = sizeof(szLongPathBuffer);
	CString strResult;
	GetRegistryValue(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\QvodPlayer\\Insert",
		L"Insertpath",
		NULL,
		(LPBYTE)szLongPathBuffer,
		&len
		);
	if(wcslen(szLongPathBuffer) == 0)
	{
		bRet = FALSE;
		goto clean0;
	}
	else
	{
		//::PathRemoveFileSpec(szLongPathBuffer);
		::PathAppend(szLongPathBuffer,L"QvodPlayer.xml");
		TiXmlDocument xmlDoc;
		const TiXmlElement *pXmlQvodPlayer = NULL;
		const TiXmlElement *pXmlGeneral = NULL;
		strResult = L"%qvod%";
		if (!xmlDoc.LoadFile(UnicodeToAnsi(szLongPathBuffer).c_str(), TIXML_ENCODING_UTF8))
			goto clean0;
		pXmlQvodPlayer = xmlDoc.FirstChildElement("QvodPlayer");
		if (!pXmlQvodPlayer)
			goto clean0;
		pXmlGeneral = pXmlQvodPlayer->FirstChildElement("General");
		if (pXmlGeneral)
		{
			const char* szTmp = pXmlGeneral->Attribute("Defaultsavepath");
            if(!szTmp)
			{
				bRet = FALSE;
				goto clean0;
			}
			else 
            {
                strResult = Utf8ToUnicode(szTmp).c_str();
            }
        }
	}

//    if (strResult.GetLength() > 3)
    {
	    bRet = ScanFileBasedPathAndName(L"Qvod",pTF, strResult, L"*.!mv", 0);
    }
clean0:
	return bRet;
}

BOOL _ScanKankanCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	DWORD len = sizeof(szBuffer);
	CString strTempVOD;
	CString strTempXMP;
	CString strTemp;
	int nPos = -1;

	GetRegistryValue(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Thunder network\\Xmp",
		L"storepath",
		NULL,
		(LPBYTE)szBuffer,
		&len
		);
	if(wcslen(szBuffer) == 0)
	{
		bRet = FALSE;
		goto clean0;
	}

	strTemp = szBuffer;
	nPos = strTemp.ReverseFind('\\');
	strPath = strTemp;
	if (nPos != strTemp.GetLength() - 1)
	{
		strTemp += L"\\";
	}
	strTempVOD = strTemp + L"VODCache";
	strTempXMP = strTemp + L"XMPCache";

	ScanKankanCacheEx(pTF, strTempVOD, nStopIndex, 1);
	ScanKankanCacheEx(pTF, strTempXMP, nStopIndex, 1);
	if (strTemp.GetLength() > 3)
	{
		ScanKankanCacheEx(pTF, strTemp, nStopIndex, 2);
	}
	//hFile = FindFirstFile()

clean0:
	return bRet;
}

BOOL ScanKankanCacheEx(ITraverseFile *pTF, const CString strPath, int nStopIndex, int nFlag)
{
	BOOL bRet = FALSE;

	if(nFlag == 2)
	{
		ScanFileBasedPathAndName(L"xlkankan",pTF, strPath, L"*.xv", nStopIndex, TRUE, 48);
		ScanFileBasedPathAndName(L"xlkankan",pTF, strPath, L"*.!xm",nStopIndex, TRUE, 48);
	}
	else if (nFlag == 1)
	{
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATA wfd = {0};

		CString strFullPath;
		CString strTemp;
		strFullPath = strPath;
		strFullPath += L"\\*.*";


		hFile = FindFirstFile(strFullPath, &wfd);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			bRet = FALSE;
			goto clean0;
		}
		do 
		{
			if ( g_bStop[nStopIndex] )
			{
				bRet = TRUE;
				goto clean0;
			}
			if (_wcsicmp(wfd.cFileName, L".") == 0 
				|| _wcsicmp(wfd.cFileName, L"..") == 0 
				|| !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				continue;
			}

			CString strFileName;
			strFileName = wfd.cFileName;
			if (strFileName.GetLength() != 40 && nFlag == 1)
			{
				continue;
			}
			CString strTempPath;
			strTempPath = strPath;
			strTempPath += L"\\";
			strTempPath += strFileName;

			if (nFlag == 1)
			{
				ScanFileBasedPathAndName(L"xlkankan",pTF, strTempPath, L"*.dat", nStopIndex, TRUE, 48);
				ScanFileBasedPathAndName(L"xlkankan",pTF, strTempPath, L"*.cfg", nStopIndex, TRUE, 48);
			}

		} while(FindNextFile(hFile, &wfd));

		bRet = TRUE;
clean0:
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FindClose(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	return bRet;
}

BOOL ScanFileBasedPathAndName(CString strItem, ITraverseFile *pTF, 
                              const CString strPath, const CString strName, int nStopIndex, 
                              BOOL bTime /* = FALSE */, DWORD dwTime /* = 0 */, int nNameLen /* = -1 */)
{
	BOOL bRet = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd = {0};
	FINDFILEDATA filedata = {0};
	CString strFilePath;
    CString strBase = strPath;

    if (strBase[strBase.GetLength() - 1] != _T('\\'))
        strBase += L"\\";

    strFilePath = strBase;

	strFilePath += strName;

	hFile = FindFirstFile(strFilePath, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		bRet = FALSE;
		goto clean0;
	}
	do 
	{
		if ( g_bStop[nStopIndex] )
		{
			bRet = TRUE;
			goto clean0;
		}

        if (_wcsicmp(wfd.cFileName, _T(".")) == 0 || _wcsicmp(wfd.cFileName, _T("..")) == 0)
            continue;
        

        CString strFileName;
        strFileName = wfd.cFileName;

        if (nNameLen != -1)
        {
            BOOL bLimitLen = CheckFileNameLength(strFileName, nNameLen);
            
            if (!bLimitLen) 
                continue;
        }

		filedata.pszPathName = strBase;
		filedata.pszFileNmae = strFileName;
		filedata.pszExtName = strName;
		filedata.uFileSize   = wfd.nFileSizeHigh;
		filedata.uFileSize   = filedata.uFileSize << 32;
		filedata.uFileSize  |= wfd.nFileSizeLow;

        if(CStubbornFiles::Instance().IsStubbornFile(strBase + strFileName))
            continue;

        // 过滤
        CString strFilter = _T("%specfiles%");
        CString FileName = strBase + strFileName;
        if (CFilterTool::Instance()._DoFindFuncMap(strFilter, FileName))
        {
            continue;
        }
        if (!CFilterTool::Instance().CheckTimeFilterByDay(wfd.ftLastWriteTime))
        {
            continue;
        }

// 		CString strLog = L"[Scan][";
// 		strLog += strItem;
// 		strLog += L"]:";
// 		strLog += strBase;
// 		strLog += strFileName;
// 		KppUserLog::UserLogInstance().WriteSingleLog(strLog.GetBuffer());

		if (pTF)
		{
			pTF->TraverseFile(&filedata);
		}
	} while(FindNextFile(hFile, &wfd));
	bRet = TRUE;
clean0:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bRet;
}

BOOL _ScanYoukuCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	TCHAR szBuffer[MAX_PATH] = {0};
	DWORD len = sizeof(szBuffer);
	CString strResult;
	CString strName;
	GetRegistryValue(HKEY_CURRENT_USER,
		L"SOFTWARE\\youku\\iKuAcc",
		L"CachePath",
		NULL,
		(LPBYTE)szBuffer,
		&len
		);
	if(wcslen(szBuffer) == 0)
	{
		bRet = FALSE;
		goto clean0;
	}
	else
	{
		strResult = szBuffer;
	}
    // + ikudisk
	strName = L"cache_*.dat";

    bRet = ScanFileBasedPathAndName(L"YouKu",pTF, strResult, strName, nStopIndex);

    strResult += L"\\ikudisk";

	bRet = ScanFileBasedPathAndName(L"YouKu",pTF, strResult, strName, nStopIndex);

    strPath = strResult;

    bRet = TRUE;
	 
clean0:
	return bRet;
}

BOOL _ScanKu6Cache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	WCHAR szLongPathBuffer[MAX_PATH] = {0};
	CString strResult;
	CString strTemp;

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
		{
			strResult = tmpPath.c_str();
		}
		else
		{
			CString StrSuffix = _T("\\kvideo_cache");;
			strResult = GetDrive(StrSuffix);

		}
	}
    strPath = strResult;

	bRet = ScanKu6CacheEx(pTF, strResult, nStopIndex);
	strTemp = strResult + L"\\cachetemp";
	bRet = ScanKu6CacheEx(pTF, strTemp, nStopIndex );
	strTemp = strResult + L"\\cachestatic";
	bRet = ScanKu6CacheEx(pTF, strTemp, nStopIndex );

	return bRet;
}

BOOL ScanKu6CacheEx(ITraverseFile *pTF, const CString strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd = {0};

	CString strFullPath;
	CString strTemp;
	strFullPath = strPath;
	strFullPath += L"\\*.*";

	hFile = FindFirstFile(strFullPath, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		bRet = FALSE;
		goto clean0;
	}
	do 
	{
		if ( g_bStop[nStopIndex] )
		{
			bRet = TRUE;
			goto clean0;
		}
		if (_wcsicmp(wfd.cFileName, L".") == 0 
			|| _wcsicmp(wfd.cFileName, L"..") == 0 
			|| !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			continue;
		}

		CString strFileName;
		strFileName = wfd.cFileName;
		if (strFileName.GetLength() != 32)
		{
			continue;
		}
		CString strTempPath;
		strTempPath = strPath;
		strTempPath += L"\\";
		strTempPath += strFileName;

		ScanFileBasedPathAndName(L"Ku6",pTF, strTempPath, L"Block*.k6p", nStopIndex);


	} while(FindNextFile(hFile, &wfd));

	bRet = TRUE;
clean0:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bRet;
}


BOOL _ScanPPTVCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	CString strResult;
	WCHAR szBuffer[MAX_PATH] = {0};
	CString strTmp;
	CString strName;
	CString strFullPath;

	::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_COMMON_APPDATA, FALSE);
	strTmp = szBuffer;
	strTmp += L"\\PPLive\\Core\\config.ini";
	IniEditor IniEdit;
	IniEdit.SetFile(strTmp.GetBuffer());
	std::wstring tmpPath = IniEdit.ReadString(L"peer",L"cachepath");
	std::string utf8str = KUTF16_To_ANSI(tmpPath.c_str());
	tmpPath = Utf8ToUnicode(utf8str.c_str());
	if(tmpPath.length() == 0)
	{
		bRet = FALSE;
		goto clean0;
	} 
	else
	{
		strResult = tmpPath.c_str();
	}
    strPath = strResult;
	strName = L"??????????????_*.mp4";
    ScanFileBasedPathAndName(L"PPLive",pTF, strResult, strName, nStopIndex);
    strName = L"??????????????_*.flv";
	ScanFileBasedPathAndName(L"PPLive",pTF, strResult, strName, nStopIndex);
	strName = L"*[*].mp4.tpp";
	ScanFileBasedPathAndName(L"PPLive",pTF, strResult, strName, nStopIndex);
	strName = L"*[*](*).mp4.tpp";
	ScanFileBasedPathAndName(L"PPLive",pTF, strResult, strName, nStopIndex);
	strName = L"*[*].mp4";
	ScanFileBasedPathAndName(L"PPLive",pTF, strResult, strName, nStopIndex);
	strName = L"*[*](*).mp4";
	ScanFileBasedPathAndName(L"PPLive",pTF, strResult, strName, nStopIndex);
	
	strFullPath = strResult;
	strFullPath += L"\\InvisibleFolder";

	strName = L"??????????????_*.swf";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"??????????????_*.jpg";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"??????????????_*.wmv";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"??????????????_*.gif";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"_db_allinonetoday*.zip";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"_db_big*.zip";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"_db_small*.zip";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"_db_small*.zip.ttp";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"vip_db_allinonetoday*.zip";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"vip_db_big*.zip";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

	strName = L"vip_db_small*.zip";
	ScanFileBasedPathAndName(L"PPLive",pTF, strFullPath, strName, nStopIndex);

    bRet = TRUE;
clean0:
	return bRet;
}

BOOL _ScanFengxingCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	WCHAR* szBuffer;
	CString strTmp;
	CString strResult;
	//::SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_APPDATA, FALSE);
	szBuffer = _wgetenv(_T("USERPROFILE"));
	strTmp = szBuffer;
	/*int a = strTmp.ReverseFind(L'\\');
	strTmp = strTmp.Mid(0,strTmp.ReverseFind(L'\\'));*/
	strTmp += L"\\funshion.ini";
	IniEditor IniEdit;
	IniEdit.SetFile(strTmp.GetBuffer());
	std::wstring tmpPath = IniEdit.ReadString(L"FILE_PATH",L"MEDIA_PATH");
	if(tmpPath.length() == 0)
	{
		CString StrSuffix = _T("\\FunshionMedia");
		strResult = GetDrive(StrSuffix);
	}
	else
	{
		strResult = tmpPath.c_str();
	}

    strPath = strResult;
	bRet = ScanFengxingCacheEx(pTF, strResult, nStopIndex);


	return bRet;
}

BOOL ScanFengxingCacheEx(ITraverseFile *pTF, const CString strPath, int nStopIndex)
{
	BOOL bRet = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd = {0};

	CString strFullPath;
	CString strTemp;
	strFullPath = strPath;
	strFullPath += L"\\*.*";

	hFile = FindFirstFile(strFullPath, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		bRet = FALSE;
		goto clean0;
	}
	do 
	{
		if ( g_bStop[nStopIndex] )
		{
			bRet = TRUE;
			goto clean0;
		}
		if (_wcsicmp(wfd.cFileName, L".") == 0 
			|| _wcsicmp(wfd.cFileName, L"..") == 0 
			|| !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			continue;
		}

		CString strFileName;
		strFileName = wfd.cFileName;
		//if (strFileName.GetLength() != 32)
		//{
		//	continue;
		//}
		CString strTempPath;
		strTempPath = strPath;
		strTempPath += L"\\";
		strTempPath += strFileName;

		ScanFileBasedPathAndName(L"FengXing",pTF, strTempPath, L"*.fc!", nStopIndex, TRUE, 48);


	} while(FindNextFile(hFile, &wfd));

	bRet = TRUE;
clean0:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bRet;
}

BOOL _ScanQQLiveCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
    BOOL bRet = FALSE;
	WCHAR szBuffer[MAX_PATH] = {0};
	CString strTmp;
	CString strResult;

	int nPos = -1;
	CString strTemp;
	CString strTempEx;
    std::vector<CString> vecScanPath;
    std::vector<CString>::iterator ite;

	::SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_APPDATA, FALSE);
	strTmp = szBuffer;
	strTmp += L"\\Tencent\\QQLive\\user.ini";
	IniEditor IniEdit;
	IniEdit.SetFile(strTmp.GetBuffer());
	std::wstring tmpPath = IniEdit.ReadString(L"Cache",L"Directory");       

	if(tmpPath.length() > 0 && PathFileExists(tmpPath.c_str()))
	{
		tmpPath.erase(tmpPath.length()-1);
		strResult = tmpPath.c_str();
		//strResult += L"\\vodcache";
	}
	else
	{
        // 枚举盘符
        BOOL bRes = _EnumQQLiveDrivePath(vecScanPath);
        
        if (!bRes) goto clean0;
	}

    for (ite = vecScanPath.begin(); ite != vecScanPath.end(); ++ite)
    {
        strResult = (*ite);
        strPath = strResult;

        nPos = strResult.ReverseFind('\\');
        if (nPos != strResult.GetLength() - 1)
        {
            strResult += L"\\";
        }
        strTemp = strResult + L"vodcache\\";
        strTempEx = strTemp + L"advcache\\";

        bRet = ScanQQLiveCacheEx(pTF, strTemp, nStopIndex, TRUE, 3);
        bRet = ScanQQLiveCacheEx(pTF, strTempEx, nStopIndex);

        strTemp += L"filecache\\";

        bRet = ScanQQLiveCacheEx(pTF, strTemp, nStopIndex);
    }

	bRet = TRUE;
clean0:

	return TRUE;
}

BOOL _EnumQQLiveDrivePath(std::vector<CString>& vecScanPath)
{
    BOOL bRet = FALSE;
    CString strScanPath;
    std::vector<CString> vecDrive;
    std::vector<CString>::iterator ite;

    if ( !_GetComputerDrives(vecDrive) )
        goto Clear0;

    for (ite = vecDrive.begin(); ite != vecDrive.end(); ++ite)
    {
        CString strPath = (*ite);
        CString strFindPath = strPath + L"\\*.*";
        HANDLE hFind = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA wfd = { 0 };

        hFind = ::FindFirstFile(strFindPath, &wfd);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            continue;
        }
        do 
        {
            if (_wcsicmp(wfd.cFileName, _T("."))== 0 || _wcsicmp(wfd.cFileName, _T("..")) == 0)
                continue;
            
            CString strTemp = wfd.cFileName;
            
            if (strTemp.GetLength() != 32)
                continue;
            
            strTemp = strPath + L"\\";
            strTemp += wfd.cFileName;
            strTemp += L"\\vodcache";

            if (::PathFileExists(strTemp))
            {
                bRet = TRUE;
                strScanPath = strPath + L"\\" + wfd.cFileName;
                vecScanPath.push_back(strScanPath);
                break;
            }

        } while (::FindNextFile(hFind, &wfd));

        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }  
    
Clear0:
    return bRet;
}

BOOL ScanQQLiveCacheEx(ITraverseFile *pTF, CString& strPath, int nStopIndex, BOOL bTime, DWORD dwTime)
{
	BOOL bRet = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CString strTemp;
	WIN32_FIND_DATA wfd = {0};
	strTemp = strPath;
	strTemp += L"*.*";
	hFile = FindFirstFile(strTemp, &wfd);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		bRet = FALSE;
		goto clean0;
	}
	do
	{
		if ( g_bStop[nStopIndex] )
		{
			bRet = TRUE;
			goto clean0;
		}

		CString strFileName;
		CString strTempResult;
		strFileName = wfd.cFileName;
		if (_wcsicmp(strFileName.GetBuffer(), L".") == 0 
			|| _wcsicmp(strFileName.GetBuffer(), L"..") == 0 
			|| !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			|| strFileName.GetLength() != 32)
		{
			continue;
		}
		strTempResult = strPath;
		strTempResult += strFileName;

		CString strFileTemp;
		strFileTemp = strFileName;
		strFileTemp += L".*.tdl";
		ScanFileBasedPathAndName(L"QQLive",pTF, strTempResult, strFileTemp, nStopIndex, bTime, dwTime);

		strFileTemp = strFileName;
		strFileTemp += L".tii";
		ScanFileBasedPathAndName(L"QQLive",pTF, strTempResult, strFileTemp, nStopIndex, bTime, dwTime);

		strFileTemp = strFileName;
		strFileTemp += L".tdi";
		ScanFileBasedPathAndName(L"QQLive",pTF, strTempResult, strFileTemp, nStopIndex, bTime, dwTime);

		ScanFileBasedPathAndName(L"QQLive",pTF, strTempResult,  L"*.tpt", nStopIndex, bTime, dwTime);
		//strPath = strTempEx;

	}while(FindNextFile(hFile, &wfd));

clean0:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bRet;
}
BOOL _ScanStormCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
    BOOL bRet = FALSE;
    WCHAR szBuffer[MAX_PATH] = {0};
    CString strTmp;
    CString strResult;

    int nPos = -1;
    CString strTemp;
    CString strTempEx;

    ::SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_COMMON_APPDATA, FALSE);
    strTmp = szBuffer;
    strTmp += L"\\Storm\\StormNC\\uninst_cache.ini";
    IniEditor IniEdit;
    IniEdit.SetFile(strTmp.GetBuffer());

    int numPath = 0;
    IniEdit.ReadDWORD(L"cache", L"cache_num", numPath);

    for (int i = 0; i < numPath; ++i)
    {
        CString strName;
        strName.Format(L"path%d", i);
        CString tmpPath = IniEdit.ReadString(L"cache", strName.GetBuffer()).c_str(); 
        
        if (g_bStop[nStopIndex])
            goto Clear0;

        if(tmpPath.GetLength() > 3) // 过滤根目录
        {   
            HANDLE hFind = INVALID_HANDLE_VALUE;
            WIN32_FIND_DATA wfd = {0};
            CString strFileName;
            CString strFind = tmpPath + L"*.*";

            hFind = FindFirstFile(strFind, &wfd);
            if (hFind == INVALID_HANDLE_VALUE)
            {
                continue;
            }
            do 
            {
                strFileName = wfd.cFileName;
                if (_wcsicmp(strFileName.GetBuffer(), L".") == 0 
                    || _wcsicmp(strFileName.GetBuffer(), L"..") == 0 
                    || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    continue;
                }
                
                CString strScan = tmpPath + strFileName;

                bRet = ScanFileBasedPathAndName(L"Storm",pTF, strScan,  L"*.*", nStopIndex);

            } while (::FindNextFile(hFind, &wfd));
            
            if (hFind != INVALID_HANDLE_VALUE)
            {
                FindClose(hFind);
                hFind = INVALID_HANDLE_VALUE;
            }
        }
    }

Clear0:
    return bRet;
}
BOOL _ScanKuGooCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
    BOOL bRet = FALSE;
    WCHAR szLongPathBuffer[MAX_PATH] = {0};
    CString strResult;
    CString strTemp;
    CString strScanPath;
    std::string strTmp;
    TiXmlDocument xmlDoc;
    const TiXmlElement *pXmlKugoo = NULL;
    const TiXmlElement *pXmlGeneral = NULL;

    DWORD len = sizeof(szLongPathBuffer);
    GetRegistryValue(HKEY_CURRENT_USER,
        L"Software\\KuGoo6",
        L"AppFileName",
        NULL,
        (LPBYTE)szLongPathBuffer,
        &len
        );

    ::PathRemoveFileSpec(szLongPathBuffer);

    if(wcslen(szLongPathBuffer) == 0)
    {
        goto Clear0;
    }

//     strTemp = szLongPathBuffer;
//     strTemp += L"\\KuGoo.xml";

    ::PathAppend(szLongPathBuffer, L"KuGoo.xml");

    strResult = L"%qvod%";
    if (!xmlDoc.LoadFile(UnicodeToAnsi(szLongPathBuffer).c_str(), TIXML_ENCODING_UTF8))
        goto Clear0;

    pXmlKugoo = xmlDoc.FirstChildElement("KuGoo");
    if (!pXmlKugoo)
        goto Clear0;

    pXmlGeneral = pXmlKugoo->FirstChildElement("Misc");
    if (pXmlGeneral)
    {
        const char* szTmp = pXmlGeneral->Attribute("DefaultDownPath");
        if(szTmp == NULL)
        {
            bRet = FALSE;
            goto Clear0;
        }
        else strResult = Utf8ToUnicode(szTmp).c_str();
    }
    else
    {
        goto Clear0;
    }

    // 分析字符串
    
    for (int i = 0; i < strResult.GetLength(); ++i)
    {
        if (strResult[i] == _T(';'))
        {
            strScanPath += L"Temp";
            ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.kg!", nStopIndex);
            ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.cfg", nStopIndex);
            strScanPath = _T("");
        }
        else
        {
            strScanPath += strResult[i];
        }
    }

    if (!strScanPath.IsEmpty())
    {
        strScanPath += L"Temp";
        ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.kg!", nStopIndex);
        ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.cfg", nStopIndex);
    }

    const char* szTmp = pXmlGeneral->Attribute("DefaultMVDownPath");
    
    if (!szTmp)
        goto Clear0;

    strTmp = szTmp;

    if (strTmp.length() > 0)
    {
        strScanPath = Utf8ToUnicode(strTmp).c_str();
        if (strScanPath.GetLength() > 3)
        {
            ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.mkv", nStopIndex);
        }
        strScanPath += L"Temp";
        ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.kg!", nStopIndex);
        ScanFileBasedPathAndName(L"KuGoo", pTF, strScanPath,  L"*.cfg", nStopIndex);
    }

    
    bRet = TRUE;
    
Clear0:
    return bRet;
}

BOOL _ScanPIPICache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
    BOOL bRet = FALSE;
    WCHAR szLongPathBuffer[MAX_PATH] = {0};
    CString strIniPath;
    std::vector<CString>  vecFilter;

    DWORD len = sizeof(szLongPathBuffer);

    GetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\CCKm\\Kamun Player",
        L"ExePath",
        NULL,
        (LPBYTE)szLongPathBuffer,
        &len
        );

    ::PathRemoveFileSpec(szLongPathBuffer);

    if(wcslen(szLongPathBuffer) == 0)
    {
        goto Clear0;
    }

    ::PathAppend(szLongPathBuffer, _T("config"));

    strIniPath = szLongPathBuffer;
    strIniPath += L"\\config.ini";

//    _GetPIPIFilterPaths(szLongPathBuffer, vecFilter);

    bRet = _ScanPIPICacheEx(pTF, strIniPath, nStopIndex, vecFilter);


Clear0:
    return bRet;
}

BOOL _ScanPIPICacheEx(ITraverseFile *pTF, CString& strIniPath, int nStopIndex, std::vector<CString>& vecFilter)
{
    BOOL bRet = FALSE;
    CString strScanPath;
    IniEditor IniEdit(strIniPath.GetBuffer());
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA wfd = { 0 };
    CString strScanTemp;

    std::wstring tmpPath = IniEdit.ReadString(L"SYSTEM",L"SAVEDPATH");
    if(tmpPath.length() == 0)
    {
        bRet = FALSE;
        goto Clear0;
    }
    else
    {
        strScanPath = tmpPath.c_str();
    }

    if (strScanPath.GetLength() <= 3)
        goto Clear0;

    if (strScanPath[ strScanPath.GetLength() - 1 ] != _T('\\'))
        strScanPath += _T('\\');

    ScanFileBasedPathAndName(L"PIPIPlayer", pTF, strScanPath,  L"*.jfenc", nStopIndex);

    ScanFileBasedPathAndName(L"PIPIPlayer", pTF, strScanPath,  L"*.inpart", nStopIndex);

    ScanFileBasedPathAndName(L"PIPIPlayer", pTF, strScanPath,  L"*.jfrec", nStopIndex);

    bRet = TRUE;
Clear0:
    return bRet;
}

void _GetPIPIFilterPaths(LPCTSTR szPath, std::vector<CString>& vecFilter)
{
    CString strPath = szPath;

    vecFilter.clear();
}

BOOL _ScanKuwoCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
    BOOL bRet = FALSE;
    TCHAR szPath[MAX_PATH] = { 0 };
    std::wstring strTemp;
    CString strScanPath;
    std::wstring strAppdata;


    SHGetSpecialFolderPath(NULL, szPath, CSIDL_COMMON_APPDATA, FALSE);
    strAppdata = szPath;

    if (strAppdata.rfind(L'\\') != strAppdata.size())
    {
        strAppdata += L"\\";
    }
    strAppdata += L"kuwo\\conf\\user\\config.ini";
    IniEditor inif;

    inif.SetFile(strAppdata.c_str());
    strTemp = inif.ReadString(L"Setting", L"temppath");
    if(strTemp.find(L"//") != -1)
    {
        strTemp.replace(strTemp.find(L"//"), 1, L"\\");
    }
    
    if (strTemp.length() == 0)
        goto Clear0;

    strScanPath = strTemp.c_str();

    bRet = ScanFileBasedPathAndName(L"Kuwo", pTF, strScanPath,  L"*.wma", nStopIndex, FALSE, 0, 16);

    bRet = ScanFileBasedPathAndName(L"Kuwo", pTF, strScanPath,  L"*.pack", nStopIndex, FALSE, 0, 16);

    bRet = ScanFileBasedPathAndName(L"Kuwo", pTF, strScanPath,  L"*.mkv", nStopIndex, FALSE, 0, 16);

    bRet = TRUE;

Clear0:
    return bRet;
}

BOOL _ScanKSafeVulCache(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
    BOOL bRet = FALSE;
    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szCmdline[MAX_PATH * 2] = { 0 };
    CString strResult;

    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    ::PathRemoveFileSpec(szPath);

    std::wstring strIniPath = szPath;
    std::wstring strTemp;

    IniEditor inif;

    strIniPath += L"\\cfg\\vulfix.ini";

    if (!PathFileExists(strIniPath.c_str()))
        goto Clear0;

    inif.SetFile(strIniPath.c_str());
    strTemp = inif.ReadString(L"Main", L"downpath");
    if(strTemp.find(L"//") != -1)
    {
        strTemp.replace(strTemp.find(L"//"), 1, L"\\");
    }

    if (strTemp.length() == 0)
    {
        strResult = szPath;
        strResult += L"\\hotfix";
    }
    else
    {
        strResult = strTemp.c_str();
    }

    bRet =_ScanKSafeVulCacheEx(pTF, strResult, nStopIndex);

Clear0:
    return bRet;
}

BOOL _ScanKSafeVulCacheEx(ITraverseFile *pTF, CString& strPath, int nStopIndex)
{
#define MAX_BUFFER_SIZE 32767
 
    BOOL bRet = FALSE;
    
    IniEditor iniEditor;
    CString strIni = strPath + L"\\vulfix.ini";

    iniEditor.SetFile(strIni.GetBuffer());

    long  m_lRetValue;   
    int   i,index;
    CString  m_strFileName;   
    std::vector<CString>  vecSectionData;
    CString strSectionTemp;
    TCHAR ac_Result[MAX_BUFFER_SIZE] = {0};

    m_lRetValue = GetPrivateProfileSection(TEXT("downloadedfiles"),   ac_Result,  MAX_BUFFER_SIZE,   strIni);   

    if (m_lRetValue == 0)
        goto Clear0;

    for (i = 0; i < m_lRetValue; i++)   
    {   
        if (ac_Result[i] != L'\0')   
        {   
            //strSectionData[m]   =   strSectionData[m]   +   ac_Result[i];   
            strSectionTemp += ac_Result[i];
        }   
        else
        {   
            vecSectionData.push_back(strSectionTemp);
            strSectionTemp = L"";
        }   
    }  

    for (size_t iCount = 0; iCount < vecSectionData.size(); ++iCount)
    {
        index = vecSectionData[iCount].Find('=');
        if (index >= 0)
        {
            int len = vecSectionData[iCount].GetLength();
            //CString strName = vecSectionData[iCount].Left(index);
            //iniEditor.Del(L"downloadedfiles", strName.GetBuffer());
            vecSectionData[iCount] = vecSectionData[iCount].Right(len - index - 1);
        }
    }

    for (size_t iCount = 0; iCount < vecSectionData.size(); ++iCount)
    {
        ScanFileBasedPathAndName(L"KSafe", pTF, strPath, vecSectionData[iCount], nStopIndex);
    }

    ScanFileBasedPathAndName(L"KSafe", pTF, strPath, L"vulfix.ini", nStopIndex);
    
    bRet = TRUE;
Clear0:
    return bRet;
}
//////////////////////////////////////////////////////////////////////////
DWORD GetDiffDaysEx( SYSTEMTIME local,  FILETIME ftime )
{
    FILETIME flocal;
    time_t ttlocal;
    time_t ttfile;

    SystemTimeToFileTime( &local, &flocal );

    FileTimeToTime_tEx( flocal, &ttlocal );
    FileTimeToTime_tEx( ftime, &ttfile );

    time_t difftime = ttlocal - ttfile; 

    return DWORD(difftime / 3600L);// 除以每天24小时3600秒
}

void FileTimeToTime_tEx( FILETIME ft, time_t *t )
{

    ULARGE_INTEGER ui;

    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;

    *t = ((ULONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
}

DWORD FileTimeInnerEx(FILETIME tCurFile)
{
    SYSTEMTIME stLocal;  
    GetSystemTime(&stLocal);  
    DWORD dwDiff = GetDiffDaysEx(stLocal,tCurFile);

    return dwDiff;

}
CString GetDrive(CString suffix)  //根据路径．获取只读区域卷标名称
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

BOOL _GetComputerDrives(std::vector<CString>& vecDrive)
{
    BOOL bRet = FALSE;
    CString strDriverName;
    vecDrive.clear();

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

    if (vecDrive.size() > 0)
        bRet = TRUE;

    return bRet;
}

BOOL CheckFileNameLength(const CString& strFileName, const int nNameLength)
{
    BOOL bRet = FALSE;
    CString strName = strFileName;
    CString strExtName;
    int nPoint;

    if (strFileName.IsEmpty() || nNameLength < 0  || nNameLength > MAX_PATH)
        goto Clear0;

    nPoint = strName.ReverseFind('.');
    if (nPoint == -1)
    {
        if (strFileName.GetLength() == nNameLength)
        {
            bRet = TRUE;
        }
        goto Clear0;
    }

    strExtName = strName.Mid(nPoint + 1);

    if (nPoint > 0)
    {
        CString strCount = strName.Mid(0, nPoint);
        if (strCount.GetLength() == nNameLength)
        {
            bRet = TRUE;
        }
    }
    
Clear0:
    return bRet;
}
//////////////////////////////////////////////////////////////////////////


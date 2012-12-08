//////////////////////////////////////////////////////////////////////////
//取文件版本号,返回字符串格式"2008.00.00.02"
//qinzhiming
#pragma once

#pragma comment(lib, "Version.lib")
//////////////////////////////////////////////////////////////////////////

namespace kis
{

class KFileVersion
{
private:
	ATL::CString			m_strFile;
	DWORD			m_nFileInfoSize;
	BYTE*			m_pFileInfo;

	typedef struct tagLANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	}LANGANDCODEPAGE, *PLANGANDCODEPAGE ;

	PLANGANDCODEPAGE m_lpTransition;

	UINT			m_nTranSize;
	VS_FIXEDFILEINFO* m_pVerInfo;

public:
	KFileVersion(LPCWSTR strFile): m_lpTransition(NULL), 
									m_pVerInfo(NULL),
									m_nFileInfoSize(NULL),
									m_pFileInfo(NULL),
									m_nTranSize(NULL)
	{
		m_strFile = strFile;
		DWORD dwHandle = 0;
		if (m_nFileInfoSize = GetFileVersionInfoSize(strFile, &dwHandle))
		{
			m_pFileInfo = new BYTE[m_nFileInfoSize];
			if (m_pFileInfo)
			{
				memset(m_pFileInfo, 0, m_nFileInfoSize);
				if (!GetFileVersionInfo(strFile, NULL, m_nFileInfoSize, m_pFileInfo))
				{
					delete m_pFileInfo;
					m_pFileInfo = NULL;
				}
				else
				{
					UINT dwSize = 0;
					VerQueryValue(m_pFileInfo, L"\\", (void**)&m_pVerInfo, &dwSize);
					VerQueryValue(m_pFileInfo, L"\\VarFileInfo\\Translation", (void**)&m_lpTransition, &m_nTranSize);
				}
			}
		}
	}

	~KFileVersion()
	{
		if (m_pFileInfo)
		{
			delete m_pFileInfo;
			m_pFileInfo = NULL;
		}
	}

	ATL::CString GetVersion()
	{
		ATL::CString sResult(TEXT(""));
		sResult = TEXT("N/A");
		if (!m_pVerInfo)
			return sResult;

		sResult.Format(
			L"%02d.%02d.%02d.%02d",
			HIWORD(m_pVerInfo->dwFileVersionMS),
			LOWORD(m_pVerInfo->dwFileVersionMS),
			HIWORD(m_pVerInfo->dwFileVersionLS),
			LOWORD(m_pVerInfo->dwFileVersionLS));

		return sResult;
	}

	ATL::CString	GetCompany()
	{
		if (!m_lpTransition)
			return ATL::CString(TEXT(""));
		return GetString(m_lpTransition, TEXT("CompanyName"));
	}

	ATL::CString	GetProductName()
	{
		if (!m_lpTransition)
			return ATL::CString(TEXT(""));
		return GetString(m_lpTransition, TEXT("ProductName"));
	}

	ATL::CString	GetFileDescription()
	{
		if (!m_lpTransition)
			return ATL::CString(TEXT(""));
		return GetString(m_lpTransition, TEXT("FileDescription"));
	}

	ATL::CString	GetString(PLANGANDCODEPAGE pLang, LPCWSTR strName)
	{
		ATL::CString strCode;
		strCode.Format(TEXT("\\StringFileInfo\\%04x%04x\\%s"), pLang->wLanguage, pLang->wCodePage, strName);
		LPCTSTR pBuf = NULL;
		UINT nSize = 0;
		if (VerQueryValue(m_pFileInfo, strCode.GetBuffer(), (void**)&pBuf, &nSize))
			return ATL::CString (pBuf);
		return ATL::CString(TEXT(""));
	}

	void GetTime(SYSTEMTIME *pFileTime)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE Handle;
		int nRetCode    = 0;		
		Handle = ::FindFirstFile(m_strFile, &FindFileData);
		if (Handle != INVALID_HANDLE_VALUE)
		{
			nRetCode = ::FileTimeToSystemTime(&FindFileData.ftLastWriteTime, pFileTime);

			::FindClose(Handle);
		}
	}
};


inline void GetPEFileVersion(
	const WCHAR *psFilePath,
	ATL::CString &sResult)
{
	int nRetCode;
	DWORD dwRetCode = 0;	
	DWORD dwHandle = 0;
	unsigned int ulQuerySize = 0;
	void *pQueryBuffer = NULL;

	VS_FIXEDFILEINFO *pVerInfo = NULL;

	sResult = TEXT("N/A");

	dwRetCode = GetFileVersionInfoSize(psFilePath, &dwHandle);
	if (dwRetCode == 0)
		goto Exit0;

	pQueryBuffer = new char[dwRetCode];
	if (pQueryBuffer == NULL)
		goto Exit0;

	memset(pQueryBuffer, 0, dwRetCode);

	nRetCode = GetFileVersionInfo(psFilePath, NULL, dwRetCode, pQueryBuffer);
	if (nRetCode == 0)
		goto Exit0;

	nRetCode = VerQueryValue(pQueryBuffer, L"\\", (void**)&pVerInfo, &ulQuerySize);
	if (nRetCode == 0)
		goto Exit0;

	sResult.Format(
		L"%02d.%02d.%02d.%02d",
		HIWORD(pVerInfo->dwFileVersionMS),
		LOWORD(pVerInfo->dwFileVersionMS),
		HIWORD(pVerInfo->dwFileVersionLS),
		LOWORD(pVerInfo->dwFileVersionLS));	

Exit0:
	if (pQueryBuffer != NULL)
	{
		delete [] pQueryBuffer;
		pQueryBuffer = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

}
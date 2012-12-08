#pragma once
#include <FileDownloader\IFileDownloader.h>
#include <FileDownloader\KFileDownloaderCreate.h>


#ifdef _FILEDOWNLOADER_STATIC_LIB_

class CFileDownloadObjectFactory
{
public:
	CFileDownloadObjectFactory() 
	{
	}

	~CFileDownloadObjectFactory()
	{
	}

	BOOL CreateObject(IFileDownloader **pObj)
	{
		ATLASSERT(pObj);
		return CreateFileDownloaderFromStaticLib(pObj);
	}
	
	VOID ReleaseObject(IFileDownloader **pObj)
	{
		if(pObj && *pObj)
		{
			(*pObj)->Release();
			(*pObj) = NULL;
		}
	}
};


#else
	

class CFileDownloadObjectFactory
{
public:
	CFileDownloadObjectFactory() : m_hModule(NULL)
	{
		TCHAR szExePath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, szExePath, MAX_PATH);
		_tcscat(szExePath, _T("/../kfiled.dll"));
		m_hModule = ::LoadLibrary(szExePath);
		//m_hModuleKFileDownload = ::LoadLibrary(_T("kfiled.dll"));
		ATLASSERT(m_hModule);
	}

	~CFileDownloadObjectFactory()
	{
		// FreeLibrary(m_hModule);
		// m_hModule = NULL;
	}

	BOOL CreateObject(IFileDownloader **pObj)
	{
		ATLASSERT(pObj);
		*pObj = NULL;
		if(m_hModule)
		{
			return CreateFileDownloader(m_hModule, pObj);
		}
		return FALSE;
	}
	
	VOID ReleaseObject(IFileDownloader **pObj)
	{
		if(pObj && *pObj)
		{
			(*pObj)->Release();
			(*pObj) = NULL;
		}
	}

protected:
	HMODULE m_hModule; 
};

#endif
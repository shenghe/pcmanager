#ifndef _PATH_UTIL_H__
#define _PATH_UTIL_H__
#include <atlstr.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>

class CPathUtil
{
public:
	static CPathUtil* Instance()
	{
		if ( !m_pInstance )
			m_pInstance = new CPathUtil;

		return m_pInstance;
	}

	static void	Destroy()
	{
		if ( m_pInstance )
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

	LPCWSTR	GetShellFolder(DWORD dwFolder)
	{
		if ( dwFolder == CSIDL_PROGRAMS )
		{
			return m_strCurrentStartMenu;
		}
		else if ( dwFolder == CSIDL_COMMON_PROGRAMS )
		{
			return m_strAllUserStartMenu;
		}
		else if ( dwFolder == CSIDL_DESKTOP )
		{
			return m_strDeskTop;
		}

		return L"";
	}

	DWORD	GetLnkFullPath(IN const wchar_t* pszLnkFile, OUT const wchar_t* pszPath, IN DWORD dwLength)
	{
		WIN32_FIND_DATA _wfd = { 0 };
		HRESULT _hrRet = E_FAIL;

		if ( !m_piPersistFile || !m_piShellLnk )
			return -1;

		DWORD ret = -1;

		__try
		{
			_hrRet = m_piPersistFile->Load(pszLnkFile, STGM_READ);
			if ( FAILED(_hrRet) )
				__leave;

			_hrRet = m_piShellLnk->GetPath((LPWSTR)pszPath, dwLength, &_wfd, SLGP_UNCPRIORITY);
			if ( FAILED(_hrRet) )
				__leave;

			ret = 0;
		}
		__except(1) {}

		return ret;
	}

protected:
	//-----------------------------------------------------------------------
	//lnk
	BOOL	_InitShell()
	{
		::CoInitialize(NULL);
		HRESULT _hrRet = E_FAIL;

		_hrRet = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<void**>(&m_piShellLnk));
		if ( FAILED(_hrRet) )
			return FALSE;

		_hrRet = m_piShellLnk->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&m_piPersistFile));
		if ( FAILED(_hrRet) )
			return FALSE;

		return TRUE;
	}

	void	_UnInitShell()
	{
		if ( m_piPersistFile )
		{
			m_piPersistFile->Release();
			m_piPersistFile = NULL;
		}

		if ( m_piShellLnk)
		{
			m_piShellLnk->Release();
			m_piShellLnk = NULL;
		}

		::CoUninitialize();
	}

	//-----------------------------------------------------------------------
	//shell folder
	void	_InitFolder()
	{
		wchar_t _szBuf[MAX_PATH] = { 0 };
		SHGetSpecialFolderPath(NULL,_szBuf,CSIDL_PROGRAMS,FALSE);
		::PathAddBackslash(_szBuf);
		m_strCurrentStartMenu = _szBuf;

		SHGetSpecialFolderPath(NULL,_szBuf,CSIDL_COMMON_PROGRAMS,FALSE);
		::PathAddBackslash(_szBuf);
		m_strAllUserStartMenu = _szBuf;

		SHGetSpecialFolderPath(NULL,_szBuf,CSIDL_DESKTOP,FALSE);
		::PathAddBackslash(_szBuf);
		m_strDeskTop = _szBuf;
	}

	CPathUtil()
		: m_piShellLnk(NULL)
		, m_piPersistFile(NULL)
	{
		_InitShell();
		_InitFolder();
	}

	~CPathUtil()
	{
		_UnInitShell();
	}

private:
	static CPathUtil* m_pInstance;

private:
	CString	m_strCurrentStartMenu;	// Start Menu\Programs
	CString	m_strAllUserStartMenu;	// All Users\Start Menu\Programs
	CString	m_strDeskTop;			// <desktop>

	IShellLink*		m_piShellLnk;
	IPersistFile*	m_piPersistFile;

};

CPathUtil* CPathUtil::m_pInstance = NULL;

#endif // _PATH_UTIL_H__
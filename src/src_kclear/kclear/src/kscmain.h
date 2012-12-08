#pragma once


//////////////////////////////////////////////////////////////////////////

//#include <miniutil/bkcmdline.h>

//////////////////////////////////////////////////////////////////////////

#define KSC_DDE_WND_CLASS L"{A866E23E-2789-44a7-AA60-8E728E108B93}"

//////////////////////////////////////////////////////////////////////////

class CKscModule : public CAppModule
{
public:
	HRESULT Init(HINSTANCE hInstance);
	VOID UnInit();
	UINT_PTR Main();

	LPCTSTR	GetAppFilePath();
	LPCTSTR GetAppDirPath();
    LPCTSTR GetAppVersion();	//get product version
	LPCTSTR GetAppFileVersion();//get file version	

protected:
	CString m_strAppFileName;
	CString m_strAppPath;
	CString m_strAppVersion;
	CString m_strFileVersion;
	LONG    m_lAppVersionNumber;

	void _GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet);
	BOOL _GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion);
	BOOL _GetPEFileVersion(LPCWSTR lpszFileName, CString &strFileVersion);
	void _InitUIResource();
    BOOL GetProductVersion();
	BOOL GetFileVersion();
};

extern CKscModule _Module;

//////////////////////////////////////////////////////////////////////////

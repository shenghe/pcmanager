#pragma once

#include "resource.h"
#include <bkres/bkres.h>
#include <atlfile.h>
#include <bkwin/bkatldefine.h>
#include "_idl_gen/bksafesvc.h"
#include <miniutil/bkcmdline.h>
#include <bksafe/bksafenavigatestring.h>
#include "bksafeexamcall.h"


#define BKSAFE_INSTANCE_OBJECT_NAME                L"{E855FBFA-BE48-4487-8354-14E99D8754C7}"

// 直接调用某功能的命令行参数
#define CMDPARAM_NAME_NAVIGATE                     L"do"
#define CMDPARAM_NAME_SETUCACHE					   L"setupcache"
#define CMDPARAM_NAME_INSTALL_LOCAL_ENGINE         L"installlocalengine"
#define CMDPARAM_NAME_UNINSTALL					   L"uninstall"	
#define CMDPARAM_NAME_SAFEMONITOR				   L"safemonitor"
#define CMDPARAM_NAME_RESTART					   L"restart"
#define CMDPARAM_NAME_SETTING					   L"bksafesetting"

#ifndef REGKEY_OEM
#define REGKEY_OEM			_T("OEM")
#endif

#ifndef REGKEY_PREOEM
#define REGKEY_PREOEM		_T("PreOEM")
#endif

#ifndef REGKEY_ORGOEM
#define REGKEY_ORGOEM		_T("OrgOEM")
#endif

#ifndef REGKEY_OEMNAME
#define REGKEY_OEMNAME		_T("OEMName")
#endif

#ifndef REGPATH_COOP
#define REGPATH_COOP		_T("Software\\KSafe\\Coop")
#endif

#ifndef REPORT_INST_MUTEX
#define REPORT_INST_MUTEX	_T("{E2359503-877A-4637-AF53-920A857CEB7F}")
#endif

class CBkNavigator
{
public:
	CBkNavigator()
	{
		m_strNavigate.Empty();
	}
	CString		GetNavigateStr(){return m_strNavigate;}
	

    BOOL Navigate(LPCWSTR lpszNavigateString)
    {
		m_strNavigate = lpszNavigateString;
        CString strNavigate = lpszNavigateString;

        return Navigate(strNavigate);
    }

    BOOL Navigate(CString &strNavigateString)
    {
        CString strChild;
        int nSpliter = strNavigateString.Find(L'_');
		m_strNavigate = strNavigateString;

        if (-1 == nSpliter)
        { 
            strChild = strNavigateString;
            strNavigateString.Empty();
        }
        else
        {
            strChild = strNavigateString.Left(nSpliter);
            strNavigateString = strNavigateString.Mid(nSpliter + 1);
        }

        CBkNavigator *pChild = OnNavigate(strChild);

        if (pChild && !strNavigateString.IsEmpty())
        {
            return pChild->Navigate(strNavigateString);
        }

        return FALSE;
    }

    virtual CBkNavigator* OnNavigate(CString &strChildName)
    {
        return NULL;
    }
private:
	CString		m_strNavigate;
};

class CBeikeSafeUrlNavigator
    : public CBkNavigator
{
public:
    CBkNavigator* OnNavigate(CString &strChildName)
    {
        ::ShellExecute(NULL, L"open", strChildName, NULL, NULL, SW_SHOWNORMAL);

        return NULL;
    }
protected:
private:
};

class CBeikeSafeModule
    : public CAppModule
    , public CBkNavigator
{
public:
    CBeikeSafeModule()
        : m_pNavigator(NULL)
        , m_bShowMainUI(TRUE)
		, m_hModRichEdit2(NULL)
		, m_nCmdShow(-1)
    {
    }

    HRESULT Init(HINSTANCE hInstance);
    HRESULT ParseCommandLine(LPCWSTR lpszCmdLine);
    UINT_PTR Main();
    BOOL CheckInstance();
    void SetActiveWindow(HWND hWndActive);
	VOID InitRichEdit2();

    BOOL CheckIntegrityLevel();

    CBkNavigator* OnNavigate(CString &strChildName);

	LPCTSTR	GetAppFilePath();
	LPCTSTR GetAppDirPath();
    LPCTSTR GetProductVersion();
	LPCTSTR GetPID();
	LPCTSTR GetOrgOem();

	int		ReSetUnKnownFileLink();

    CBkSafeExamCaller Exam;
	int				  m_nCmdShow;

protected:

    CBkNavigator *m_pNavigator;

    CString m_strAppFileName;
    CString m_strAppPath;
    CString m_strAppVersion;
	CString		m_strInstallPID;
	CString     m_strOrgOem;
    LONG    m_lAppVersionNumber;

    CBkCmdLine _CmdLine;
	HMODULE	m_hModRichEdit2;

    BOOL m_bShowMainUI;

    CBeikeSafeUrlNavigator m_UrlNavigator;

    BkWinManager m_BkWndMagnager;

    struct BkSafe_Instance_Data
    {
        HWND hActiveWnd;
    };

    CAtlFileMapping<BkSafe_Instance_Data> m_mapping;

    void _GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet);
    BOOL _GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion);
    void _InitUIResource();
	void _RestartSelf();
	HANDLE _GetWindowProcess( HWND hWnd );
	void CheckUnknowFile();
	BOOL GetOrgPID(CString & strPid);
	BOOL GetInstallPID(CString & strPid);
};

extern CBeikeSafeModule _Module;


#pragma once

#include <miniutil/bkcmdline.h>

class KAppModule : public CAppModule
{
public:
	KAppModule(): m_hModRichEdit2(NULL),m_dwRunParamType(0)
	{
	}


	HRESULT		Init(HINSTANCE hInstance);
	void		Uninit();
	HRESULT		Run();
	BOOL		CheckInstance();
	void		SetActiveWindow(HWND hWndActive);
	BOOL		CheckIntegrityLevel();
	LPCTSTR		GetAppFilePath();
	HRESULT		ParseCommandLine(LPCWSTR lpszCmdLine);

private:
	void _InitUIResource(void);

private:
	HMODULE		m_hModRichEdit2;
	CBkCmdLine	m_CmdLine;
	CString		m_strAppFileName;
	CString		m_strAppPath;
	DWORD		m_dwRunParamType;
};

extern KAppModule _Module;

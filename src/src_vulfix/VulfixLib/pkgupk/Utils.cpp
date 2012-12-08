#include "StdAfx.h"
#include "Utils.h"

BOOL ExecuteFile(LPCTSTR lpszFilename, LPCTSTR lpszInstallParam, DWORD &dwExitCode)
{
	if (!lpszFilename || !lpszInstallParam || !PathFileExists(lpszFilename))
		return FALSE;

	TCHAR szCmdline[MAX_PATH] = {0};
	if(lpszInstallParam && _tcslen(lpszInstallParam)>0)
		_stprintf(szCmdline, _T("\"%s\" %s"), lpszFilename, lpszInstallParam);
	else
		_tcscpy(szCmdline, lpszFilename);

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi={0};
	BOOL processCreated = CreateProcess(NULL, szCmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if(!processCreated)
		return FALSE;
	WaitForSingleObject(pi.hProcess, INFINITE);

	dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	DBG_TRACE(_T("%s -> %d\n"), szCmdline, dwExitCode );
	return TRUE;
}

INT DeletePath(LPCTSTR lpszPath)
{
	TCHAR szPath[MAX_PATH] = {0};
	_tcscpy(szPath, lpszPath);
	SHFILEOPSTRUCT shfileopstruct = {0};
	shfileopstruct.wFunc = FO_DELETE;
	shfileopstruct.pFrom = szPath;   
	shfileopstruct.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;   
	return SHFileOperation(&shfileopstruct);
}

void DumpFiles( const Files& files, BOOL bOnlyFileName )
{
	if(files.empty())
	{
		_tprintf(_T("Empty Files\r\n"));
	}
	else
	{
		int i=0; 
		for(Files::const_iterator it=files.begin(); it!=files.end(); ++it)
		{
			++i;
			LPCTSTR szFile = *it;
			if(bOnlyFileName)
			{
				LPCTSTR szName = _tcsrchr(szFile, _T('\\'));
				if(szName)
					szFile = ++szName;
			}
			_tprintf(_T("%d/%d %s\r\n"), i, files.size(), szFile);
		}
	}
}
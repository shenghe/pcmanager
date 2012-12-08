#pragma once
#include <windows.h>
#include <string>
#define		defSENDBYGET		1
#define		defSENDBYPOST		2

class KCallInfocSend
{
public:
	KCallInfocSend(void);
	~KCallInfocSend(void);

public:
	static void SendData(IN int nSendMode = defSENDBYGET, IN const wchar_t * lpStrFilePath = NULL);

private:
	static BOOL _GetKInfocPath(OUT std::wstring& strInfocPath);
	static BOOL _CheckFileFromReg(OUT std::wstring& strFilePath);
	static BOOL _ReadOneRegKey(OUT wchar_t * lpszResult, IN long dwOutMaxSize);
	static DWORD _LaunchAppEx(LPCWSTR lpczApp, LPWSTR lpszCmdLine, BOOL bWait, PDWORD pdwExitCode = NULL, BOOL bShowWnd = TRUE);
};

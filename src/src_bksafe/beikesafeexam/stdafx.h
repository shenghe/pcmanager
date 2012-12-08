// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <atlbase.h>
#include <atlstr.h>

#define IESCANENG_EXPORTS

#ifdef _DEBUG

#   include <miniutil/bkconsole.h>

#   define DEBUG_TRACE wprintf

#else

#   define DEBUG_TRACE

#endif

#include <tinyxml/tinyxml.h>
#include <string>
#include "common.h"
using namespace BEIKESAFEEXAM;

#include "communits/ThreadLangLocale.h"
#include "strsafe.h"
#include "kispublic/kcalloem.h"
#include "kispublic/kopksafereg.h"

#include "common/utility.h"
#include "iefix/RegisterOperate.h"




#define REG_KSAFE	L"SOFTWARE\\KSafe"
class CKSafeExamLog
{
public:
	static CKSafeExamLog* GetLogPtr()
	{
		static CKSafeExamLog log;
		return &log;
	}
	~CKSafeExamLog(){}
public:
	void WriteLog(LPCTSTR pszFormat, ...)
	{
		USE_CHINESE_LANG_LOCAL;
		USES_CONVERSION;

		if (FALSE == IsCanWriteLog())
			return;

		CString strTrace = L"";
		va_list args;
		va_start(args, pszFormat);
		strTrace.FormatV(pszFormat, args);
		va_end(args);

		if (TRUE == strTrace.IsEmpty())
			return;

		char szBuf[1000] = {'\0'};
		StringCbPrintfA(szBuf, sizeof(szBuf), "%s", W2A(strTrace.GetBuffer(-1)));
		strTrace.ReleaseBuffer(-1);

		FILE *pFile = fopen(W2A(m_strUpliveLogPath.GetBuffer(MAX_PATH)), ("a+b"));
		m_strUpliveLogPath.ReleaseBuffer(MAX_PATH);

		if(pFile == NULL)
			return;

		SYSTEMTIME timeCur;
		GetLocalTime(&timeCur);
		char szTime[100] = {0};
		StringCbPrintfA(szTime, sizeof(szTime), ("%d-%d-%d %d:%d:%d:%d  "), timeCur.wYear, timeCur.wMonth, timeCur.wDay,timeCur.wHour, timeCur.wMinute, timeCur.wSecond, timeCur.wMilliseconds);

		fwrite((szTime), strlen(szTime), 1, pFile);
		fwrite((szBuf),strlen(szBuf), 1, pFile);
		fwrite(("\r\n"), 2, 1,pFile);
		fclose(pFile);
	}

	CString SetUpLiveLog(CString strLogPath)
	{
		if (TRUE == strLogPath.IsEmpty())
			return L"";
		CString strLog;
		strLog = m_strUpliveLogPath;
		m_strUpliveLogPath = strLogPath;

		return strLog;
	}

	BOOL IsCanWriteLog()
	{
		return m_bCanWriteLog;
	}
private:
	CKSafeExamLog()
	{
		m_bCanWriteLog = FALSE;
		CString strLog;
		CAppPath::Instance().GetLeidianLogPath(strLog);
		m_strUpliveLogPath = strLog;
		m_strUpliveLogPath.Append(L"\\ksafeexam.log");

		DWORD dwValue = 0;
		RegisterOperate::CRegisterOperate::GetRegOperatePtr()->GetRegValue(HKEY_LOCAL_MACHINE, REG_KSAFE, L"examdebug", dwValue);
		m_bCanWriteLog = dwValue;
	}
	CString		m_strUpliveLogPath;
	BOOL		m_bCanWriteLog;
};
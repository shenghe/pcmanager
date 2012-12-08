

#ifndef __KPPUSERLOG_H__
#define __KPPUSERLOG_H__

#include <string>
#include <vector>
#include <map>
#include <atlstr.h>

#define TIME_MAX_LEN 30
#define TIME_LEN 19

class KppUserLog
{

public:

	KppUserLog();

public:

	~KppUserLog();

	STDMETHODIMP Init(const WCHAR* wszPathName);

	STDMETHODIMP Uninit(void);

	STDMETHODIMP WriteSingleLog(const std::wstring& wstLogString);

    STDMETHODIMP ReadOrgLog(std::vector<std::wstring>& vLogCache);

    STDMETHODIMP ReadLog(std::multimap<std::wstring, std::wstring>& mapLine);

    STDMETHODIMP ClearLog();
	
public:

	static KppUserLog& KppUserLog::UserLogInstance();
	BOOL						m_bWrite;

protected:

	BOOL ParseTime(std::wstring& sTime, SYSTEMTIME* pTime);

	HRESULT AddPathName(WCHAR* wszMainPathName, const WCHAR* wszPathName);
    
    DWORD _DoGetLineByBuf(
        WCHAR* const pszBuf, 
        DWORD dwlen, 
        std::multimap<std::wstring, std::wstring>& mapStrline);

    DWORD _DoGetFileSizeByFileName(const WCHAR* pFileName);

    DWORD _DoGetLineByBuf(
        WCHAR* const pszBuf, 
        DWORD dwlen, 
        std::vector<std::wstring>& vcStrline);

    DWORD _DoGetLineDescByBuf(
         WCHAR*  pszBuf, 
        DWORD dwlen, 
        std::multimap<std::wstring, std::wstring>& mapStrline);

protected:

	void _LockWork();

	void _UnlockWork();

protected:

	CRITICAL_SECTION			m_LogLock;
	FILE*						m_pFile;
	std::wstring				m_strPathName;
	std::locale					m_Loc;
	BOOL						m_bInitFlag;
	
	
};

#endif /* __KPPUSERLOG_H__ */

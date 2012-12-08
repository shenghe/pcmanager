#include "StdAfx.h"
#include "Utilfunction.h"
#include "FileStream.h"
#include <iostream>
#include <fstream>
#include <vulfix/BeikeUtils.h>

BOOL CrackUrl( LPCTSTR lpszUrl, UrlInfo& sUrlInfo )
{
	DWORD dwFlags = ICU_DECODE;

	URL_COMPONENTS us = {0};
	memset( &us, 0, sizeof( us ) );
	memset( &sUrlInfo, 0, sizeof( sUrlInfo ) );

	us.dwStructSize = sizeof( us );
	us.lpszHostName = sUrlInfo.szHostName;
	us.dwHostNameLength = SIZE_OF_ARRAY( sUrlInfo.szHostName );
	us.lpszUrlPath = sUrlInfo.szUrlPath;
	us.dwUrlPathLength = SIZE_OF_ARRAY( sUrlInfo.szUrlPath );
	us.lpszUserName = sUrlInfo.szUserName;
	us.dwUserNameLength = SIZE_OF_ARRAY( sUrlInfo.szUserName );
	us.lpszPassword = sUrlInfo.szPassword;
	us.dwPasswordLength = SIZE_OF_ARRAY( sUrlInfo.szPassword );
	us.lpszExtraInfo = sUrlInfo.szExtraInfo;
	us.dwExtraInfoLength = SIZE_OF_ARRAY( sUrlInfo.szExtraInfo );

	if ( InternetCrackUrl( lpszUrl, _tcslen( lpszUrl ), dwFlags, &us ) )
	{
		if ( INTERNET_SCHEME_HTTP == us.nScheme || INTERNET_SCHEME_HTTPS == us.nScheme )
		{
			_tcscpy_s(sUrlInfo.szUrl, lpszUrl);
			sUrlInfo.nPort = us.nPort;
			sUrlInfo.nScheme = us.nScheme;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GetFileNameFromUrl( LPCTSTR szUrl, CString &strFilename )
{
	LPCTSTR pslash = _tcsrchr(szUrl, _T('/'));
	if(pslash && _tcslen(pslash)>0)
	{
		++pslash;
		LPCTSTR pq = _tcschr(pslash, _T('?'));
		if(pq)
			strFilename.SetString(pslash, pq-pslash);
		else
			strFilename = pslash;
		return TRUE;
	}
	return FALSE;
}

void GetUrlsFromFile( LPCTSTR szFilename, std::vector<std::string> &urls )
{
	CFileInStream fin(szFilename);
	if(fin.Create())
	{
		int64 dwSize = fin.GetFileSize();
		if(dwSize>0)
		{
			DWORD dwReaded = 0;
			CHAR *pbuffer = new CHAR[dwSize + 1];
			ATLASSERT(pbuffer);
			ZeroMemory(pbuffer, dwSize+1);
			fin.Read(pbuffer, dwSize, &dwReaded);

			const char* pb = pbuffer, *pn=NULL, *pnn=NULL;
			while(pb)
			{
				pnn = pn = strchr(pb, '\n');
				while(*pb=='\t'||*pb==' '||*pb=='\r'||*pb=='\n')
					++pb;
				while(pn && pn>pb && (*pn=='\t'||*pn==' '||*pn=='\r'||*pn=='\n') )
					--pn;

				const char* lpszHttp = "http://";
				if( _strnicmp(pb, lpszHttp, strlen(lpszHttp))==0 )
				{
					std::string surl;
					if(pn)							
						surl.assign(pb, pn-pb+1);
					else
						surl = pb;
					urls.push_back( surl );
				}
				pb = pnn ? pnn+1 : pnn;
			}
		}
	}
}

void FormatSizeString(INT64 nFileSize, CString &str)
{
	const INT64 SIZE_KILO = 1024;
	const INT64 SIZE_MEGA = SIZE_KILO * SIZE_KILO;
	const INT64 SIZE_GIGA = SIZE_MEGA * SIZE_KILO;

	if(nFileSize==0)
		str = _T("0");
	else if(nFileSize<SIZE_KILO)
		str.Format(_T("%dB"), nFileSize);
	else if(nFileSize<SIZE_MEGA)
		str.Format(_T("%.2fKB"), nFileSize*1.0/SIZE_KILO );
	else if(nFileSize<SIZE_GIGA)
		str.Format(_T("%.2fMB"), nFileSize*1.0/SIZE_MEGA );
	else
		str.Format(_T("%.2fGB"), nFileSize*1.0/SIZE_GIGA );
}


#ifndef _DEBUG

BOOL __cdecl MYTRACE( LPCTSTR xxx, ... ) 
{
	return TRUE;
}

#else

#define LOG_FILE_NAME _T("DOWNLOAD.log")
template<int LogLevel>
class Log
{
public:
	CString &CompletePathWithModulePath(CString &strPath, LPCTSTR lpszFilename)
	{
		TCHAR szPath[MAX_PATH] = {0};
		::GetModuleFileName( (HMODULE)&__ImageBase, szPath, MAX_PATH);
		PathAppend(szPath, _T("..\\"));
		PathAppend(szPath, lpszFilename);
		strPath = szPath;
		return strPath;
	}

	Log()
	{
		CString strLog;
		CompletePathWithModulePath(strLog, LOG_FILE_NAME);
		m_fs.open(strLog, std::ifstream::out | std::ifstream::binary | std::ifstream::app | std::ifstream::ate);
	}
	
	template<typename T>
	Log& operator<<(T _Val)
	{
#ifdef _DEBUG
		std::cout << _Val;
#endif
		m_fs << _Val; 
		return *this;
	}
	~Log()
	{
		m_fs.close();
	}
	std::ofstream m_fs;
};

typedef CComAutoCriticalSection                     CObjLock;
typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;


BOOL __cdecl MYTRACE( LPCTSTR xxx, ... )
{
	static CObjLock lock;
	CObjGuard _guard_(lock);

	CString	str;
	va_list arg_ptr; 
	va_start(arg_ptr, xxx); 
	str.FormatV( xxx, arg_ptr );
	va_end( arg_ptr );
	
	CString strDate;
	SYSTEMTIME st;
	GetLocalTime(&st);
	strDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	USES_CONVERSION;
	CStringA szDate = CT2CA(strDate);
	CStringA szMsg = CT2CA(str);
	
	static Log<1> log;
	log<< szDate << " : " << szMsg;
	log.m_fs.flush();

	return TRUE;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for netmon
//      
//      File      : logtools.cpp
//      Version   : 1.0
//      Comment   : 日志记录protocol buffer 序列化工具,和一些共用Tools
//      
//      Create at : 2008-11-25
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "logtools.h"
#include <atltime.h>

void AssignProtocalString(std::string& strTarget, std::wstring& strSource)
{
	if (strSource.length() > 0)
		strTarget.assign((char*)strSource.c_str(), sizeof(wchar_t)*(strSource.length() + 1));
}

std::string ToProtocolString(std::wstring& strSource)
{
	std::string strRes;
	AssignProtocalString(strRes, strSource);
	return strRes;
}

inline std::string ToProtocolString(wchar_t* str)
{
	std::wstring strSource(str);
	return ToProtocolString(strSource);
}

template<class T>
std::string ToProtocolString(T* pData)
{
	std::string strRes;
	if (pData)
		strRes.assign((char*)pData, sizeof(T));
	return strRes;
}

template<class T>
std::string ToProtocolString(T* pData, int nSize)
{
	std::string strRes;
	if (pData)
		strRes.assign((char*)pData, sizeof(T)*nSize);
	return strRes;
}

std::string ToProtocolString( char* pData, int nSize)
{
	std::string strRes;
	if (pData)
		strRes.assign((char*)pData, sizeof(char)*nSize);
	return strRes;
}

std::string ToProtocolString(const wchar_t* str)
{
	std::wstring strSource(str);
	return ToProtocolString(strSource);
}


void ExtractProtocolString(std::wstring& strTarget, std::string& strSource)
{
	strTarget = (WCHAR*)strSource.c_str();
}

HRESULT TimeToString( __time64_t nTime, ATL::CString& strTime )
{
	ATL::CTime time(nTime);
	strTime.Format( TEXT("%d %d %d %d:%d:%d"), 
		time.GetYear(), time.GetMonth(), 
		time.GetDay(), time.GetHour(), 
		time.GetMinute(),time.GetSecond());

	return S_OK;
}

// __time64_t FileTimeToTime( FILETIME time )
// {
// 	__time64_t time64;
// 	BYTE * pBuf = (BYTE*)&time64;
// 	memcpy( pBuf, &time.dwHighDateTime, sizeof(DWORD) );
// 	memcpy( pBuf + sizeof(DWORD), &time.dwLowDateTime, sizeof(DWORD) );
// 
// 	return time64;
// }

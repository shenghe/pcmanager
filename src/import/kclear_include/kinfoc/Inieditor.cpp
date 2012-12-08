#include "StdAfx.h"
#include "IniEditor.h"
#include <string>
#include <stdio.h>
#include <windows.h>
using std::string;

wstring IniEditor::ReadString(const wstring& strSec, const wstring& strKey ) const
{
	WCHAR strBuf[ 65536 ] = {0};

	DWORD dwLen = GetPrivateProfileString( 
		strSec.c_str(), 
		strKey.c_str(), 
		L"", 
		strBuf, 
		sizeof(strBuf),
		m_wstrFile.c_str() 
	);

	/// 对于超出长度的串一律返空
	if ( sizeof( strBuf ) - 1 == dwLen )
		return L"";

	return strBuf;
}

BOOL IniEditor::WriteString(
                            const wstring& strSec, 
                            const wstring& strKey, 
                            const wstring& strVal )
{
    return ( 
        0 != WritePrivateProfileStringW( 
        strSec.c_str(), 
        strKey.c_str(),
        strVal.c_str(),
        m_wstrFile.c_str() 
        ) 
        );

	DWORD dwError = ::GetLastError();
}

BOOL IniEditor::ReadDWORD(const wstring& strSec, const wstring& strKey, int& dwVal ) const
{
	wstring strVal = ReadString( strSec, strKey );
	int nRet = swscanf_s( strVal.c_str(), L"%u", &dwVal );
	return ( 0 != nRet && EOF != nRet );
}

BOOL IniEditor::WriteDWORD(const wstring& strSec, const wstring& strKey, DWORD dwVal )
{
    wchar_t szBuf[16] = { 0 };
    (void)swprintf_s( szBuf, sizeof( szBuf ) / sizeof( szBuf[0] ), L"%u", dwVal );
    return WriteString( strSec, strKey, szBuf );
}

BOOL IniEditor::IncrDWORD(const wstring& strSec, const wstring& strKey )
{
	int dwVal = 0;
	if ( !ReadDWORD( strSec, strKey, dwVal ) )
		dwVal = 0;

	return WriteDWORD( strSec, strKey, dwVal + 1 );
}

BOOL IniEditor::DelFile()
{
	::SetFileAttributes( m_wstrFile.c_str(), FILE_ATTRIBUTE_NORMAL );
	return (::DeleteFile( m_wstrFile.c_str() ) || ERROR_FILE_NOT_FOUND == GetLastError() );
}

BOOL IniEditor::Del(const wstring& strSec, const wstring& strKey )
{
	return ( 
		0 != WritePrivateProfileString( 
			strSec.c_str(), 
			strKey.c_str(),
			NULL,
			m_wstrFile.c_str() 
		) 
	);
}

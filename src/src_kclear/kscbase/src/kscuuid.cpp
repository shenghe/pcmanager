#include "kscuuid.h"
#include <windows.h>
#include "kscconv.h"

//////////////////////////////////////////////////////////////////////////

class GUIDGener
{
public :
	GUIDGener();
	~GUIDGener();

	const std::string& GetGUIDA() const;
private :
	std::string m_strGUID;
};

GUIDGener::GUIDGener()
{
	GUID guid;
	unsigned char szTmp[ 17 ] = { 0 };
	char szGuidElm[3] = { 0 };
	int i = 0;
	CoInitialize( NULL );
	CoCreateGuid( &guid );
	//CoUninitialize();
	szTmp[0] = (char)( ( guid.Data1 >> 24 ) & 0XFF );
	szTmp[1] = (char)( ( guid.Data1 >> 16 ) & 0XFF );
	szTmp[2] = (char)( ( guid.Data1 >> 8 ) & 0XFF );
	szTmp[3] = (char)( guid.Data1 & 0XFF );
	szTmp[4] = (char)( ( guid.Data2 >> 8 ) & 0XFF );
	szTmp[5] = (char)( guid.Data2 & 0XFF );
	szTmp[6] = (char)( ( guid.Data3 >> 8 ) & 0XFF );
	szTmp[7] = (char)( guid.Data3 &0XFF );
	for ( i = 0; i < 8; ++i )
	{
		szTmp[ i + 8 ] = guid.Data4[ i ];
	}
	for ( i = 0; i < 16; ++i )
	{
		sprintf_s( szGuidElm, sizeof( szGuidElm ) / sizeof( szGuidElm[0] ),  "%02X", szTmp[i] );
		m_strGUID += szGuidElm;
	}
}
GUIDGener::~GUIDGener()
{
}

const std::string& GUIDGener::GetGUIDA() const
{
	return m_strGUID;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	__declspec(selectany) char* g_szKscRegPaths[] = 
	{
		"software\\KingSoft\\KClear",		// 前面的是曾经使用过的注册表路径
		"software\\KingSoft\\KSSC"			// 最后一个为当前产品使用的注册表路径
	};

	__declspec(selectany) char g_szKscRegPath[] = "software\\KingSoft\\KSSC";
}

//////////////////////////////////////////////////////////////////////////

KscUuid::KscUuid()
{
	if (!ReadFromReg(m_strUUID))
	{
		GUIDGener guidGener;
		m_strUUID = guidGener.GetGUIDA();
		WriteToReg(m_strUUID);
	}
}

KscUuid::~KscUuid()
{
}

//////////////////////////////////////////////////////////////////////////

bool KscUuid::WriteToReg(const std::string& strGuid)
{
	bool retval = false;
	LONG lRetCode;
	HKEY hReg = NULL;

	lRetCode = ::RegCreateKeyExA(HKEY_LOCAL_MACHINE, g_szKscRegPath, 0, NULL,
			0, KEY_ALL_ACCESS, NULL, &hReg, NULL);
	if (lRetCode)
		goto clean0;

	lRetCode = ::RegSetValueExA(hReg, "GUID", 0, REG_SZ, 
			(const BYTE*)strGuid.c_str(), strGuid.length() + 1);
	if (lRetCode)
		goto clean0;

	retval = true;

clean0:
	if (hReg)
	{
		::RegCloseKey(hReg);
		hReg = NULL;
	}
	return retval;
}

bool KscUuid::ReadFromReg(const char szPath[], std::string& strGuid)
{
	bool retval = false;
	LONG lRetCode;
	HKEY hReg = NULL;
	char szGUID[MAX_PATH] = { 0 };
	ULONG dwInput = MAX_PATH;
	DWORD dwType;

	lRetCode = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, szPath, 0, KEY_READ, &hReg);
	if (lRetCode)
		goto clean0;

	lRetCode = ::RegQueryValueExA(hReg, "GUID", NULL, &dwType, (LPBYTE)szGUID,
			&dwInput);
	if (lRetCode)
		goto clean0;

	if (dwType != REG_SZ)
		goto clean0;

	strGuid = szGUID;
	if (strGuid.length() == 0)
		goto clean0;

	retval = true;

clean0:
	if (hReg)
	{
		::RegCloseKey(hReg);
		hReg = NULL;
	}
	return retval;
}

bool KscUuid::ReadFromReg(std::string& strGuid)
{
	bool retval = false;
	bool fRetCode;
	size_t i = sizeof(g_szKscRegPaths) / sizeof(TCHAR*);

	for (; i > 0; --i)
	{
		fRetCode = ReadFromReg(g_szKscRegPaths[i-1], strGuid);
		if (fRetCode)
		{
			retval = true;
			goto clean0;
		}
	}

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

bool KscUuid::GetUUID(std::string& strUUID)
{
	bool retval = false;
	
	if (m_strUUID.length() == 0)
		goto clean0;

	strUUID = m_strUUID;

	retval = true;

clean0:
	return retval;
}

bool KscUuid::GetUUID(std::wstring& strUUID)
{
	bool retval = false;
	bool fRetCode;
	std::string strUUIDa;

	fRetCode = GetUUID(strUUIDa);
	if (!fRetCode)
		goto clean0;

	strUUID = AnsiToUnicode(strUUIDa);

	retval = true;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

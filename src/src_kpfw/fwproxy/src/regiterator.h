///////////////////////////////////////////////////////////////	
//	
// FileName  :  regiterator.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-08-20  14:27
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once

class CRegIterator
{
public:
	CRegIterator():m_hKey(NULL),m_dwIndex(0)
	{
	}

	~CRegIterator()
	{
		if( m_hKey != NULL )
		{
			RegCloseKey( m_hKey );
			m_hKey = NULL;
		}
	}

	void begin( HKEY hKey, TCHAR *szSubKeyName , TCHAR* lpValueName, LPDWORD lpcValueName, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData )
	{
		m_dwIndex = 0;
		m_lpValueName = lpValueName;
		m_lpcValueName = lpcValueName;
		m_lpType = lpType;
		m_lpData = lpData;
		m_lpcbData = lpcbData;
		RegOpenKeyEx( hKey, szSubKeyName, 0, KEY_READ, &m_hKey );
	}

	
	bool next()
	{
		LONG nRetCode = 0;
		if( m_hKey == NULL )
			return false;

		nRetCode = RegEnumValue( m_hKey, m_dwIndex++, m_lpValueName, m_lpcValueName, NULL, m_lpType, m_lpData, m_lpcbData );
		return nRetCode == ERROR_SUCCESS;
	}

private:

	HKEY m_hKey;
	DWORD m_dwIndex;
	LPTSTR m_lpValueName;
	LPDWORD m_lpcValueName;
	LPDWORD m_lpType;
	LPBYTE m_lpData;
	LPDWORD m_lpcbData;
};

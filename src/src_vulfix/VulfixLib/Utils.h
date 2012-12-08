#pragma once
#include <vulfix/Defines.h>
#include "vulfix/BeikeUtils.h"

template <class T> class singleton : private T
{
public:
	static T &Instance()
	{
		static singleton<T> _instance;
		return _instance;
	}
private:
	singleton(){}
	~singleton(){}
};

template <typename T>
T& G()
{
	return singleton<T>::Instance();
}

template<typename T>
void CopySimpleArray(const CSimpleArray<T> &arrFrom, CSimpleArray<T> &arrTo)
{
	for(int i=0; i<arrFrom.GetSize(); ++i)
	{
		arrTo.Add( arrFrom[i] );
	}
}

template<typename T>
void ClearResult(CSimpleArray<T>& arrResult )
{
	for ( int i = 0; i < arrResult.GetSize(); i ++ )
	{
		if( arrResult[i] )
			delete arrResult[i];
	}
	arrResult.RemoveAll();
}

struct TPairIdString
{
	TPairIdString(int nid, LPCTSTR szValue) : nID(nid), str(szValue)
	{ }

	TPairIdString(LPCTSTR szKey, LPCTSTR szValue)
	{
		nID = _ttoi(szKey);
		str = szValue;
	}
	BOOL GetKeyValue(CString &strKey, CString &strValue)
	{
		strKey.Format(_T("%d"), nID);
		strValue = str;
		return TRUE;
	}
	int		nID;
	CString str;
};

template<typename T>
BOOL LoadIniSectionKeyValues(LPCTSTR iniFilename, LPCTSTR lpszSectionName, CSimpleArray<T> &keyValus)
{
	TCHAR* szBuf = new TCHAR[32768];
	if(!szBuf) return FALSE;

	BOOL bRet = FALSE;
	do 
	{
		if( GetPrivateProfileSection(lpszSectionName, szBuf, 32768 * sizeof(TCHAR), iniFilename)<=0 )
			break;

		LPTSTR szTemp = szBuf;
		while (szTemp[0])
		{
			size_t len = _tcslen(szTemp);
			LPTSTR pequal = _tcschr(szTemp, _T('='));
			if(!pequal) break;			
			*pequal = 0;
			
			++pequal;
			while( *pequal==_T(' ') || *pequal==_T('\t') )
				++ pequal;
			
			if(_tcslen(szTemp)>0 && _tcslen(pequal)>0)
			{
				keyValus.Add( T(szTemp, pequal) );
			}
			szTemp += len + 1;
		}
		bRet = TRUE;
	} while (FALSE);

	delete[] szBuf;
	return bRet;
}

template<typename T>
BOOL EnumRegKey(HKEY hKey, LPCTSTR lpszPath, T &fn, LPVOID lpData=NULL)
{
	HKEY hk = NULL;
	if( ERROR_SUCCESS!=RegOpenKeyEx(hKey, lpszPath, 0, KEY_READ, &hk) )
		return FALSE;

	for(DWORD nIndex=0;;++nIndex)
	{
		DWORD dwLen = MAX_PATH;
		TCHAR szKey[MAX_PATH] = {0};

		if( ERROR_SUCCESS!=SHEnumKeyEx(hk, nIndex, szKey, &dwLen) )
			break;

		if( !fn(hKey, lpszPath, nIndex, szKey, lpData) )
			break;
	}
	RegCloseKey( hk );
	return TRUE;
}

template<typename T>
BOOL EnumRegValue(HKEY hKey, LPCTSTR lpszPath, T &fn, LPVOID lpData=NULL)
{
	HKEY hk;
	if( ERROR_SUCCESS!=RegOpenKeyEx(hKey, lpszPath, 0, KEY_READ, &hk) )
		return FALSE;

	DWORD dwcbKey = MAX_PATH;
	DWORD dwcbValue = MAX_PATH;
	TCHAR szKey[MAX_PATH];
	TCHAR szValue[MAX_PATH];
	DWORD nValueType;
	for( DWORD dwIndex = 0; ; ++dwIndex )
	{
		dwcbKey = dwcbValue = MAX_PATH;
		if( ERROR_SUCCESS!=SHEnumValue(hk, dwIndex, szKey, &dwcbKey, &nValueType, szValue, &dwcbValue) )
			break; 

		if( !fn(hKey, lpszPath, dwIndex, nValueType, szKey, szValue, lpData) )
			break;
	}
	RegCloseKey( hk );
	return TRUE;
}

template<typename T>
BOOL SaveIniSectionKeyValues(LPCTSTR iniFilename, LPCTSTR lpszSectionName, CSimpleArray<T> &keyValus)
{
	WritePrivateProfileSection(lpszSectionName, NULL, iniFilename);
	for (int i = 0; i < keyValus.GetSize(); i++)
	{
		CString strKey, strValue;
		keyValus[i].GetKeyValue(strKey, strValue);
		WritePrivateProfileString(lpszSectionName, strKey, strValue, iniFilename);
	}
	return TRUE;
}

template<typename T>
int FindArrayIndex( const CSimpleArray<T> &arr, int nID )
{
	for ( int i = 0; i < arr.GetSize(); i++)
	{
		if ( arr[i].nID == nID )
		{
			return i;
		}
	}
	return -1;
}

template<typename T>
int FindArrayIndex( const CSimpleArray<T*> &arr, int nID )
{
	for ( int i = 0; i < arr.GetSize(); i++)
	{
		if ( arr[i]->nID == nID )
		{
			return i;
		}
	}
	return -1;
}

template<typename T>
T FindArrayItem( const CSimpleArray<T> &arr, int nID )
{
	int nIndex = FindArrayIndex( arr, nID );
	if(nIndex==-1)
		return NULL;
	return arr[nIndex];
}

struct T_ComInit {
	T_ComInit() 
	{
		CoInitialize(NULL);
	}
	~T_ComInit()
	{
		CoUninitialize();
	}
};

class CThreadGuard {
public:
	CThreadGuard() { InitializeCriticalSection(&m_cs); }
	~CThreadGuard() { DeleteCriticalSection(&m_cs); }
	void lock() { EnterCriticalSection(&m_cs); }
	void unlock() { LeaveCriticalSection(&m_cs); }
private:
	CRITICAL_SECTION m_cs;
};

template <typename T>
class autolock {
public:
	autolock(T& lockable) : m_lockable(lockable) { m_lockable.lock(); }
	~autolock() { m_lockable.unlock(); }
private:
	T& m_lockable;
};

#ifdef _DEBUG
#define TIME_CHECK(x) T_TimeEvaluate __time_check(x);

struct T_TimeEvaluate{
	T_TimeEvaluate(LPCTSTR szTitle)
		: strTitle(szTitle), dwTickBegin(GetTickCount())
	{
	}

	~T_TimeEvaluate()
	{
		_tcprintf( _T("%s : %.3f \n"), strTitle, (GetTickCount()-dwTickBegin)/1000.0f );
	}
protected:
	CString strTitle;
	DWORD dwTickBegin;
};

#else

#define TIME_CHECK(x) 

#endif

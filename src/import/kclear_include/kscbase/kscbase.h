/********************************************************************
	created:	2010/03/08
	created:	8:3:2010   9:38
	filename: 	kppbase.h
	author:		Jiang Fengbing
	
	purpose:	
*********************************************************************/

#ifndef KPPBASE_INC_
#define KPPBASE_INC_

//////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <tchar.h>
#include <windows.h>

//////////////////////////////////////////////////////////////////////////

template <typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

//////////////////////////////////////////////////////////////////////////

inline bool GetModuleFileName(std::string& strFileName, HMODULE hModule) 
{
	bool retval = false;
	CHAR szFileName[MAX_PATH] = { 0 };
	DWORD dwRetCode;

	dwRetCode = ::GetModuleFileNameA(hModule, szFileName, MAX_PATH);
	if (dwRetCode)
		goto clean0;

	strFileName = szFileName;
	retval = true;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

inline bool GetModuleFileName(std::wstring& strFileName, HMODULE hModule) 
{
	bool retval = false;
	WCHAR szFileName[MAX_PATH] = { 0 };
	DWORD dwRetCode;

	dwRetCode = ::GetModuleFileNameW(hModule, szFileName, MAX_PATH);
	if (dwRetCode)
		goto clean0;

	strFileName = szFileName;
	retval = true;

clean0:
	return retval;
}

inline bool trims( const std::wstring& str, std::vector <std::wstring>& vcResult, TCHAR c)
{
	size_t fst = str.find_first_not_of( c );
	size_t lst = str.find_last_not_of( c );

	if( fst != std::wstring::npos )
		vcResult.push_back(str.substr(fst, lst - fst + 1));

	return true;
}

inline int SplitCString(std::wstring strIn, std::vector<std::wstring>& vec_String, TCHAR division) 
{ 
	vec_String.clear(); 
	if (!strIn.empty()) 
	{ 
		size_t nIter = 0;
		size_t nLast = 0;
		std::wstring v;

		while (true) 
		{ 
			nIter = strIn.find(division, nIter); 
			trims(strIn.substr(nLast, nIter - nLast), vec_String, division);
			if( nIter == std::wstring::npos )
				break;

			nLast = ++nIter;
		} 
	} 

	return (int)vec_String.size(); 
}

//////////////////////////////////////////////////////////////////////////

#endif // KPPBASE_INC_

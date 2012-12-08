//////////////////////////////////////////////////////////////////////
///		@file		kxe_crt_v6.h
///		@author		luopeng
///		@date		2009-6-25 9:04:42
///
///		@brief		crt针对v6的一些转换
//////////////////////////////////////////////////////////////////////

#pragma once

#if _MSC_VER == 1200

#include <tchar.h>
#include <errno.h>

inline int _taccess_s(const TCHAR* pszFileName, int nAccessMode)
{
	int nRet = _taccess(pszFileName, nAccessMode);
	if (nRet != 0)
	{
		return errno;
	}
	return nRet;
}



#endif
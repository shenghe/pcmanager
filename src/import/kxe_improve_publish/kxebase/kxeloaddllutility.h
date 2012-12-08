/********************************************************************
* CreatedOn: 2006-9-6   16:12
* FileName: KLoadDllUtility.h
* CreatedBy: qiuruifeng <qiuruifeng@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/
#ifndef KXELOADDLLUTILITY_H
#define KXELOADDLLUTILITY_H

#include "kxesearchfile.h"

namespace KxELoadDllUtility
{
	//根据查找策略(先查注册表或先查当前目录)寻找scom.dll
	inline HMODULE __stdcall AutoSearchLoadLibrary(const TCHAR* pszFileName, BOOL bRegFirst = TRUE)
	{
		if (NULL == pszFileName)
			return NULL;

		HMODULE handle = NULL;

		KxESearchFile search(pszFileName);
		TCHAR szFilePath[MAX_PATH] = { 0 };
		int nResult = E_FAIL;
		
		if (bRegFirst)
		{	//在注册表中搜索
			nResult = search.SearchInReg(szFilePath, MAX_PATH);
			if (0 == nResult)
			{
				handle = LoadLibrary(szFilePath);
				if (handle)
					return handle;	
			}
		}

		//在当前dll目录下找
		nResult = search.SearchInCurModuleDir(szFilePath, MAX_PATH);
		if (0 == nResult)
		{
			handle = LoadLibrary(szFilePath);
			if (handle)
			{
				if (bRegFirst)
					search.WritePathToReg(szFilePath);

				return handle;
			}
		}

		//在当前exe目录下找
		nResult = search.SearchInCurProcessDir(szFilePath, MAX_PATH);
		if (0 == nResult)
		{
			handle = LoadLibrary(szFilePath);
			if (handle)
			{
				if (bRegFirst)
					search.WritePathToReg(szFilePath);

				return handle;
			}
		}

		if (!bRegFirst)
		{	//在注册表中搜索
			nResult = search.SearchInReg(szFilePath, MAX_PATH);
			if (0 == nResult)
			{
				handle = LoadLibrary(szFilePath);
				if (handle)
					return handle;	
			}
		}

		//让系统自动找
		handle = LoadLibrary(pszFileName);
		return handle;	
	}
}


#endif //KXELOADDLLUTILITY_H
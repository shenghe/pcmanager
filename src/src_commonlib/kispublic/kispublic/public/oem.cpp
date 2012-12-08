// ----------------------------------------------------------------------------- 
//	FileName	: oem.cpp
//	Author		: Zqy
//	Create On	: 2007-10-30 15:42:04
//	Description	: 
//
// ----------------------------------------------------------------------------- 
#include "stdafx.h"
#include "oeminforw.h"
#include "public.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
// ----------------------------------------------------------------------------- 

struct KOEMInfos : KFeatureReader
{
	KOEMInfos(LPCWSTR lpOemFilePath)
	{
		_LoadOEM(lpOemFilePath);
	}

	HRESULT Reload(LPCWSTR lpOemFilePath)
	{
		Clear();
		return _LoadOEM(lpOemFilePath);
	}

private:
	HRESULT _LoadOEM(LPCWSTR lpOemFilePath)
	{
		// 先读取OEM目录
		wchar_t szFilePath[MAX_PATH] = {0};
		int nSize = MAX_PATH;
		KIS::IO::KPath::GetKafeOemPathW(szFilePath, nSize);
		KWString strPath =  szFilePath;
		strPath += L"oem.dat";

		HRESULT hr = Load(strPath.c_str());
// 		if (FAILED(hr)) // 再读取kis.dat
// 		{
// 			strPath = kisGetPath(cfgpCurLng) + L"kis.dat";
// 			hr = Load(strPath);
// 		}
		return hr;
	}
};

static KOEMInfos& _getinfos()
{
	static KOEMInfos _oeminfos;
	return _oeminfos;
}

int kisQueryOEMInfo(__in oem_category oi)
{
	return _getinfos().QueryInt(oi);
}

LPCWSTR kisQueryOEMContent(__in oem_category oi)
{
	static const WCHAR s_empty[] = L"";

	LPCWSTR p = _getinfos().QueryStr(oi);
	if (!p)
		p = s_empty;

	return p;
}

STDMETHODIMP kisOEMInfoReload()
{
	return _getinfos().Reload();
}

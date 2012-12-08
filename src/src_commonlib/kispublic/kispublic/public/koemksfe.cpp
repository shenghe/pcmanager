#include "stdafx.h"
#include "KOemKsfe.h"
#include "kstringex.h"
#include "shellapi.h"
#include "oeminforw.h"

#pragma comment(lib, "aplib")

NS_BEGIN(KIS)
NS_BEGIN(KOemKsfe)

static LPCWSTR s_lpFullFilePath = NULL;

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
		KWString strPath;
		if (lpOemFilePath == NULL)
		{
			std::wstring strOemPath;
			int nRet = CallFunc<int, LPWSTR, size_t&>(KIS::IO::KPath::GetKafeOemFilePathW, strOemPath);
			if (!nRet)	
				return S_FALSE;

			strPath = strOemPath;

		}
		else
			strPath = lpOemFilePath;
		
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
	static KOEMInfos _oeminfos(s_lpFullFilePath);
	return _oeminfos;
}

int KOemKsfe::SetOEMFullPathA(IN LPCSTR lpFilePath)
{
	KWString strFilePath = lpFilePath;
	return SetOEMFullPathW(strFilePath.c_str());
}

int KOemKsfe::SetOEMFullPathW(IN LPCWSTR lpFilePath)
{
	s_lpFullFilePath = lpFilePath;
	_getinfos();
	return TRUE;
}

int KOemKsfe::QueryOEMStrA(IN KOemKsfe::oem_category nKey, OUT LPSTR lpValue, IN OUT size_t& nSize)
{
	KWString strValue;
	strValue.GetBuffer((int)nSize);
	BOOL bRet = QueryOEMStrW(nKey, (LPWSTR)strValue.c_str(), nSize);
	if (bRet && lpValue != NULL)
	{
		KAString strValueA = strValue;
		strcpy_s(lpValue, nSize, strValueA.c_str());
	}
	strValue.ReleaseBuffer();

	return bRet;
}

int KOemKsfe::QueryOEMStrW(IN KOemKsfe::oem_category nKey, OUT LPWSTR lpValue, IN OUT size_t& nSize)
{
	LPCWSTR pValue = _getinfos().QueryStr(nKey);
	if (pValue != NULL)
	{
		size_t nLen = wcslen(pValue);
		if (lpValue == NULL)
		{
			nSize = nLen;
			return TRUE;
		}

		if (nLen > nSize)
		{
			nSize = nLen;
			return FALSE;
		}

		wcscpy_s(lpValue, nSize, pValue);

		return TRUE;
	}

	return FALSE;
}

int KOemKsfe::QueryOEMInt(IN KOemKsfe::oem_category nKey, IN int& nValue)
{
	int nValueCur = _getinfos().QueryInt(nKey);
	if (nValueCur != -1)
		nValue = nValueCur;

	return TRUE;
}

int _MarkOemFileForBaidu()
{
	std::wstring strPath ;
	int nRet = CallFunc<int, LPWSTR, size_t&>(KIS::IO::KPath::GetCurDirectoryW, strPath);
	if (!nRet)	return FALSE;

	strPath.append(_T("oem\\0x100001\\oem.dat"));

	KIS::IO::KDirectory::CreateDirCleanFileNameW(strPath.c_str());

	KFeatureWriter w;

	w.Start()
		.StartGroup(0)
		.AddFeature(oemc_VirusFreeShow, 1)
		.AddFeature(oemc_MoreOptimizeShow, 0)
		.AddFeature(oemc_SearchEngineProtectShow, 0)
		.AddFeature(oemc_SoftwareMangerShow, 0)
		.AddFeature(oemc_trayShow, 0)
		.AddFeature(oemc_netmonterShow, 0)
		.AddFeature(oemc_OtherCheckName, L"baidu")
		.AddFeature(oemc_DownProtectedColor, 3355443)
		.AddFeature(oemc_SafeLinkURL, L"http://guanjia.baidu.com/iframe/safe.html")
		.AddFeature(oemc_IconBig, 106)
		.AddFeature(oemc_IconSmall, 107)
		.AddFeature(oemc_SubKeyStr, L"\\baidusafe\\")
		.AddFeature(oemc_IsOem, 1)
		.AddFeature(oemc_Appdata, L"BaiduSafe")
		.AddFeature(oemc_Reclye, L"BaiduSafeRecycle")
		.AddFeature(oemc_KwspluginName, L"kwsplugin2.dll")
		.AddFeature(oemc_ServiceName, L"BaiduSafeSvc")
		.AddFeature(oemc_KWSCShowName, L"百度安全卫士")
		.AddFeature(oemc_IsOpenNetM, 0)
		.AddFeature(oemc_SetCloudAddr, L"http://guanjia.baidu.com/help/safeguide.html#n4")
		.AddFeature(oemc_BBSUrl, L"http://tieba.baidu.com/f?kw=%B0%D9%B6%C8%B5%E7%C4%D4%B9%DC%BC%D2")
		.AddFeature(oemc_RunTrayName, L"BaiduKSafeTray")
		.AddFeature(oemc_RunKSafeSvcName, L"BaiduKSafeSve")
		.AddFeature(oemc_FishBlockUrl, L"http://api.pc120.com/bdfishblock/replace.htm")
		.AddFeature(oemc_UpdataUrlMsg, L"http://up.ijinshan.com/safe/msg.pack")
		.AddFeature(oemc_UpdataUrlShow, L"http://up.ijinshan.com/safe/slow.pack")
		.AddFeature(oemc_UpdataUrlQuick, L"http://up.ijinshan.com/safe/quick.pack")
		.AddFeature(oemc_NetmonPriLevel, 2)
		.EndGroup(0);
	w.End();
	w.Write(strPath.c_str());

	return TRUE;
}

int KOemKsfe::WriteAllOemData( void )
{
	int nRet = _MarkOemFileForBaidu();

	return nRet;
}
NS_END(KOemKsfe)	
NS_END(KIS)





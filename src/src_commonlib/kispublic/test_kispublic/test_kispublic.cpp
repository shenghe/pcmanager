// test_kispublic.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "d:\\Kingsoft\\2010new\\kisengine\\src\\commonapp\\uplive\\test\\kispublic\\kispublic\\public\\public.h"
#include "D:\\Kingsoft\\ksafe_baidu2.1.1_fb\\src\\publish\\kispublic\\public.h"
//#pragma comment(lib, "kispublic.lib");


void TestKFile()
{
//	KIS::IO::KFile::TestData(1);
//	KIS::IO::KDirectory::DeleteDirW(L"c:\\a");
//	KIS::IO::KDirectory::CopyDirW(L"D:\\a", L"D:\\b\\B\\C\\", TRUE);
	int nValue = -1;

	std::wstring strValue;
	BOOL bRet = CallFunc<int, LPWSTR, size_t&>(KIS::IO::KPath::GetKafeOemFilePathW, strValue);

	KIS::KOemKsfe::WriteAllOemData();
//	KIS::KOemKsfe::SetOEMFullPathA("D:\\Kingsoft\\2010new\\kisengine\\src\\commonapp\\uplive\\test\\kispublic\\debug\\baidu\\oem.dat");
//	KIS::KOem::QueryOEMIntA(KIS::KOem::oemc_a, nValue);

	int nValue3 = 1000;
	KIS::KOemKsfe::QueryOEMInt(KIS::KOemKsfe::oemc_VirusFreeShow, nValue3);

	std::wstring strValueEx;
	bRet = CallFunc<int, KIS::KOemKsfe::oem_category, LPWSTR, size_t&>(KIS::KOemKsfe::QueryOEMStrW, KIS::KOemKsfe::oemc_KWSCShowName, strValueEx);
	

//	std::wstring strValue;
//	CallFunc<int, KIS::KOemKsfe::oem_category, LPWSTR, size_t&>(KIS::KOemKsfe::QueryOEMStrW, KIS::KOemKsfe::oemc_SearchEngineProtectShow2, strValue);
}

int _tmain(int argc, _TCHAR* argv[])
{
	TestKFile();
	return 0;
}


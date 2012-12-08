// testKprocesslibDll.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "kprocesslib/interface.h"
#include "libheader/libupdhelper.h"
#include <string>
using namespace std;
#include "kprocesslib/ccf/KPath.h"
using namespace Kingsoft::Test::CCF::IO;

void TestQuery()
{
	DWORD beginCount = ::GetTickCount();

	IProcessLibQuery* pQuery = CProcessLibLoader::GetGlobalPtr(KPath::GetCurrentModuleFileDir().c_str())->GetQuery();

	DWORD endCount = GetTickCount();

	pQuery->Init(NULL);

	DWORD endCount2 = GetTickCount();

	CString strPath = L"C:\\Program Files\\ESET\\ESET NOD32 Antivirus\\ekrn.exe";
	strPath = L"d:\\Program Files\\KSafe\\KSafeSvc.exe";
	strPath = L"C:\\Program Files\\Internet Explorer\\iexplore.exe";
	KSProcessInfo* pInfo = NULL;
	if(!pQuery->GetInfoByPath(strPath, &pInfo))
	{
		wprintf(L"GetInfoByPath Failed!\n");
	}
	else
	{
		wprintf(L"Desc: %s\n", pInfo->strDesc.c_str());
	}

	DWORD endCount3 = GetTickCount();
	for (int i = 0;i < 100;i++)
	{
		if (!pQuery->GetInfoByPath(L"c:\\Windows\\System32\\alg.exe", &pInfo))
		{
			wprintf(L"GetInfoByPath Failed!\n");
		}
	}

	DWORD endCount4 = GetTickCount();


	//wprintf(L"Desc: %s\nLoadDll:%d\nInit:%d\nQuery1:%d\nquery100:%d", pInfo->strDesc.c_str(), 
	wprintf(L"Desc: \nLoadDll:%d\nInit:%d\nQuery1:%d\nquery100:%d", 
		endCount - beginCount,
		endCount2 - endCount, 
		endCount3 - endCount2,
		endCount4 - endCount3);
	pQuery->DeleteInfo(pInfo);
}
void TestUpdate()
{
	HMODULE				hModDLL;
#if _DEBUG
	hModDLL	= ::LoadLibrary(L"d:\\ksafe\\trunk\\product\\win32d\\kproclib.dll");
#else
	hModDLL	= ::LoadLibrary(L"d:\\ksafe\\trunk\\product\\win32\\kproclib.dll");
#endif

	PFN_CreateUpdateHelper			pfnCreateUpdater;
	pfnCreateUpdater = (PFN_CreateUpdateHelper)::GetProcAddress( hModDLL, FN_CreateUpdateHelper);

	IUpdateHelper* pUpdateHelper=NULL;

	pfnCreateUpdater(__uuidof(IUpdateHelper), (void**)&pUpdateHelper);

	if (pUpdateHelper)
	{
		pUpdateHelper->Combine(L"d:\\temp\\vcdiff\\kproclib\\kproclib_0.0.0.1_0.0.0.2_inc.dat");
	}

	if (hModDLL)
	{
		::FreeLibrary(hModDLL);
		hModDLL = NULL;
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(0,"chs");
	TestQuery();
	//TestUpdate();
	return 0;
}


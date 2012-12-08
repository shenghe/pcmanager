// ksafevul_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define LEAKSCAN_EXPORTS
#include <libheader/libupdhelper.h>
#include <vulfix/Vulfix.h>
#include <vulfix/BeikeUtils.h>

void DumpIVulCollection( IVulCollection * pCollection )
{
	LONG nStatus = 0;
	if(SUCCEEDED(pCollection->get_Status(&nStatus)))
	{
		_tcprintf(_T("USE Status(FOR QuickScan AND Not Use IUpdate) %d\r\n"), nStatus);
	}
	else
	{
		INT nHigh, nOption, nSP;
		ATLVERIFY(SUCCEEDED(pCollection->get_VulCount(&nHigh, &nOption, &nSP)));
		_tcprintf(_T("COUNT High:%d  Option:%d  SP:%d\r\n"), nHigh, nOption, nSP);

		ULONG count = 0;
		pCollection->get_Count(&count);
		for(size_t i=0; i<count; ++i)
		{
			// et_Item2(ULONG n, int *pKBID, ULONG *pLevel, int *pNType, DWORD *pVFlags, BSTR* pTitle, BSTR* pDescription) = 0
			int kbID=0, nLevel=0, nType=0;
			DWORD vFlags=0;
			BSTR title, desc, pubdate;

			pCollection->get_Item3(i, &kbID, &nLevel, &nType, &vFlags, &title, &desc, &pubdate);
			_tcprintf(_T("%d/%d[Lvl:%d Type:%d VFlag:%04x] [%s]%s\t%s\r\n"), i, count, nLevel, nType, vFlags, pubdate, title, desc);
			SysFreeString(title);
			SysFreeString(desc);
		}
	}
}

void TestFullScan(DWORD dwFlags=0)
{
	IVulScan *pscan = NULL;
	if(SUCCEEDED(CreateObject(__uuidof(IVulScan), (VOID**)&pscan)))
	{
		IVulCollection *pCollection = NULL;
		if(SUCCEEDED(pscan->ScanLeakEx(&pCollection, dwFlags)))
		{
			DumpIVulCollection(pCollection);
			pCollection->Release();
		}
		pscan->Release();
	}
}

void TestQuickScan()
{
	IVulCollection *pCollection = NULL;
	if(SUCCEEDED(ExpressScanLeak(&pCollection, 0, TRUE)))
	{
		DumpIVulCollection(pCollection);
		pCollection->Release();
	}
}


void TestCombine()
{
	IUpdateHelper *pidb = NULL;
	if( SUCCEEDED( CreateUpdateHelper(__uuidof(IUpdateHelper), (void**)&pidb) ) )
	{
		LPCTSTR szSysIncPack = _T("D:\\system.dat\\system_2010.4.27.1_2010.4.27.2_inc.dat");
		LPCTSTR szIncPack = _T("D:\\office.dat\\office_2010.4.20.1_2010.4.27.1_inc.dat");
		pidb->Combine(szSysIncPack);
		//pidb->Combine(szIncPack);
		int i = 0;
		++i;
	}
}

void TestExpression()
{
	//LPCTSTR szCondtion = _T("fv(\"@system\\tzchange.exe\",\"<\",\"5.1.2600.5930\")");
	//LPCTSTR szCondtion = argv[1];
	LPCTSTR szCondtion = _T("fv(\"@system\\tzres.dll\",\"<\",\"6.0.6002.18192\")");
	_tprintf(_T("%s -> %d\n"), szCondtion, EvaluateCondition(szCondtion));
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*
	if(argc>1)
	{
		TCHAR t = argv[1][0];
		if(t==_T('1'))
		{
			TestQuickScan();
		}
		else if(t==_T('2'))
		{
			TestFullScan(0);
		}
		else if(t==_T('3'))
		{
			TestFullScan(VULSCAN_EXPRESS_SCAN);
		}
	}
	*/
	
	TestCombine();
	//TestExpression();

	return 0;
}


// perfdemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "../../../publish/perfmon/kperfmon.h"
#include "../../../publish/perfmon/ksimpleperf.h"
#include "Psapi.h"
#include "Tlhelp32.h"
#include <map>
/*
#define _WIN32_DCOM
#include <afxwin.h>
*/
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

using namespace std;
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "wbemuuid.lib")

//////////////////////////////////////////////////////////////////////////
BOOL TestProcessData()
{
	KProcessPerfMgr mgr;
	mgr.Init();
	do 
	{
		mgr.Update(Update_Perf_Data_And_Calc);
		::Sleep(1000);
	} while (1);
}

BOOL TestProcessMem()
{
	KProcessPerfMgr mgr;
	mgr.Init();
	mgr.Update(Update_Perf_Data_And_Calc);
	mgr.PrintProcessInfos();
	return TRUE;
}

BOOL TestProcessIO()
{
	KProcessPerfMgr mgr;
	mgr.Init();
	do 
	{
		mgr.Update(Update_Only_Process_Perf_Data);
		mgr.PrintTotalInfo();
		::Sleep(1000);
	} while (1);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// Use to change the divisor from Kb to Mb.

#define DIV 1024
// #define DIV 1

char *divisor = "K";
// char *divisor = "";

// Handle the width of the field in which to print numbers this way to
// make changes easier. The asterisk in the print format specifier
// "%*I64d" takes an int from the argument list, and uses it to pad 
// and right-justify the number being formatted.
#define WIDTH 20

void TestMem()
{
	MEMORYSTATUSEX statex = {0};
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);

	printf ("%ld percent of memory is in use.\n",
		statex.dwMemoryLoad);
	printf ("There are %*I64d total %sbytes of physical memory.\n",
		WIDTH, statex.ullTotalPhys/DIV, divisor);
	printf ("There are %*I64d free %sbytes of physical memory.\n",
		WIDTH, statex.ullAvailPhys/DIV, divisor);
	printf ("There are %*I64d total %sbytes of paging file.\n",
		WIDTH, statex.ullTotalPageFile/DIV, divisor);
	printf ("There are %*I64d free %sbytes of paging file.\n",
		WIDTH, statex.ullAvailPageFile/DIV, divisor);
	printf ("There are %*I64x total %sbytes of virtual memory.\n",
		WIDTH, statex.ullTotalVirtual/DIV, divisor);
	printf ("There are %*I64x free %sbytes of virtual memory.\n",
		WIDTH, statex.ullAvailVirtual/DIV, divisor);

	// Show the amount of extended memory available.

	printf ("There are %*I64x free %sbytes of extended memory.\n",
		WIDTH, statex.ullAvailExtendedVirtual/DIV, divisor);
}

// 简单暴力的释放内存
BOOL FreeMemTest()
{
	MEMORYSTATUSEX statex = {0};
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	ULONGLONG nMem = statex.ullAvailPhys;

	DWORD nProcessID[1000] = {0};
	DWORD nCount = sizeof(nProcessID);
	DWORD nReturn = 0;
	if (EnumProcesses(nProcessID, nCount, &nReturn))
	{
		HANDLE hToken = NULL; 
		TOKEN_PRIVILEGES tkp = {0}; 

		if (!OpenProcessToken(GetCurrentProcess(), 
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
			return( TRUE ); 

		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, 
			&tkp.Privileges[0].Luid); 
		tkp.PrivilegeCount = 1;  
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
			(PTOKEN_PRIVILEGES)NULL, 0); 

		if (GetLastError() != ERROR_SUCCESS) 
			return TRUE; 

		for (DWORD i = 0; i < (nReturn /sizeof(DWORD)); i++)
		{
			DWORD nID = nProcessID[i];
			HANDLE hHandle = ::OpenProcess(PROCESS_SET_QUOTA, FALSE, nID);
			if (hHandle)
			{
				::SetProcessWorkingSetSize(hHandle, -1, -1);
				::CloseHandle(hHandle);
			}
		}
		if (hToken)
		{
			::CloseHandle(hToken);
			hToken = NULL;
		}
	}

	GlobalMemoryStatusEx (&statex);
	if (statex.ullAvailPhys > nMem)
	{
		ULONGLONG nMemFree = statex.ullAvailPhys - nMem;
		printf ("There are %*I64d %sbytes was freed.\n",
			WIDTH, nMemFree/DIV, divisor);
	}
	return FALSE;
}

void TestSimpleMgr()
{
	KSimplePerfMgr mgr;
	mgr.Init();
	do 
	{
		::Sleep(1000);
		mgr.Update();
		printf("simple mgr cpu: %f, mem: %f\n", mgr.GetSysCpuUsage(), mgr.GetSysMemUsage());
	} while (1);
}

void TestShatShot()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 process = {0};
		process.dwSize = sizeof(process);
		if (Process32First(hSnapShot, &process))
			printf("find process: %S", process.szExeFile);
		do 
		{
			PROCESSENTRY32 process = {0};
			process.dwSize = sizeof(process);
			if (!Process32Next(hSnapShot, &process))
				break;
			printf("find process: %S", process.szExeFile);
		} while (1);
		::CloseHandle(hSnapShot);
	}
}
//////////////////////////////////////////////////////////////////////////

int TestFullPath()
{
	HRESULT hres;

	// Step 1: Initialize COM--------------------------------------------------
	hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x" 
			<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------
	// Note: If you are using Windows 2000, you need to specify -
	// the default authentication credentials for a user by using
	// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
	// parameter of CoInitializeSecurity ----------------------
	hres =  CoInitializeSecurity(
		NULL, 
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);

	if (FAILED(hres))
	{
		cout << "Failed to initialize security. Error code = 0x" 
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                    // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------
	IWbemLocator *pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,             
		0, 
		CLSCTX_INPROC_SERVER, 
		IID_IWbemLocator, (LPVOID *) &pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (e.g. Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
		);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x" 
			<< hex << hres << endl;
		pLoc->Release();     
		CoUninitialize();
		return 1;                // Program has failed.
	}

	cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;

	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------
	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x" 
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();     
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----
	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		//bstr_t("SELECT * FROM Win32_OperatingSystem"),
		bstr_t("Select * from Win32_Process"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x" 
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
			&pclsObj, &uReturn);

		if(0 == uReturn) {
			break;
		}

		VARIANT vtProp;
		CString proc_info=L"";

		USES_CONVERSION;
		vtProp.ulVal = 0;
		hr = pclsObj->Get(L"ProcessId", 0, &vtProp, 0, 0);
		if (SUCCEEDED(hr))
		{
			DWORD nPID = vtProp.ulVal;
			printf("process: Id : %d\t", nPID);
		}

		vtProp.bstrVal=NULL;
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);

		if(vtProp.bstrVal != NULL)
			printf(" name: %S ", vtProp.bstrVal);

		vtProp.bstrVal=NULL;
		hr = pclsObj->Get(L"ExecutablePath", 0, &vtProp, 0, 0);
		if(vtProp.bstrVal != NULL)
			printf("path: %S\n", vtProp.bstrVal);
		else
			printf("\n", vtProp.bstrVal);

		VariantClear(&vtProp);
		pclsObj->Release();
		pclsObj=NULL;
	}

	// Cleanup
	
	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();

	if(pclsObj != NULL)
		pclsObj->Release();

	CoUninitialize();
	return 0;   // Program successfully completed.
}


int _tmain(int argc, _TCHAR* argv[])
{
	// TestCounterNames();
	// TestProcessData();
	// TestMem();
	// TestProcessMem();
	FreeMemTest();
	// TestProcessIO();
	// TestSimpleMgr();
	// TestShatShot();
	// TestFullPath();
	return TRUE;
}

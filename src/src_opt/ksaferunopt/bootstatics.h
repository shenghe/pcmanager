
#pragma once

#include <TlHelp32.h>
#include <Psapi.h>
#include <globalstate/GetGlobalState.h>
#include <common/utility.h>
#include <communits/FileVersionInfo.h>
#include "RunManEngine.h"
#include <runoptimize/interface.h>
#include <common/procprivilege.h>
#include <ntdll/ntdllhelper.h>

#pragma comment(lib,"psapi.lib")

#define PATH_CONFIG_REPORT_INI		_T("ksfmon.ini")
#define SECTION_NAME_REPORT			_T("runreport")
#define KEY_REPORT_ALREADY			_T("reportnum")

#define MAX_REPORT_NUM				3


#define RUNSTC_ERROR_CODE_SUCCESS			0
#define RUNSTC_ERROR_CODE_CREATE_SNAP		1
#define RUNSTC_ERROR_CODE_NO_EXPLORER		2
#define RUNSTC_ERROR_CODE_NO_WINLOGON		3
#define RUNSTC_ERROR_CODE_OPEN_EXPLORER		4
#define RUNSTC_ERROR_CODE_OPEN_WINLOGON		5
#define RUNSTC_ERROR_CODE_GETT_EXPLORER		6
#define RUNSTC_ERROR_CODE_GETT_WINLOGON		7


inline LPCTSTR GetProductVersion()
{
	static TCHAR szVersion[MAX_PATH] = {0};
	static BOOL bInited = FALSE;

	if ( !bInited )
	{
		bInited = TRUE;

		CString strVersion;
		CString strFilePath;

		CAppPath::Instance().GetLeidianAppPath(strFilePath);
		strFilePath += _T("\\ksafever.dll");

		CFileVersionInfo fileVer;
		if ( fileVer.Create(strFilePath) )
		{
			_tcsncpy(szVersion, fileVer.GetProductVersion(), MAX_PATH);
			szVersion[MAX_PATH - 1] = 0;
		}
	}

	return szVersion;
}


class CBootStatics
{
public:
	CBootStatics()
	{
		CAppPath::Instance().GetLeidianCfgPath(m_strIniPath,TRUE);
		m_strIniPath.Append( _T("\\") );
		m_strIniPath.Append( PATH_CONFIG_REPORT_INI );

	}
	virtual ~CBootStatics(){}

	struct PROC_RUN_TIME 
	{
	public:
		PROC_RUN_TIME()
		{
			nTime		= 0;
			nIoRead		= 0;
			nIoWrite	= 0;
			nMemSize	= 0;
		}

		CString		strPath;
		DWORD		nTime;
		DWORD		nIoWrite;
		DWORD		nIoRead;
		DWORD		nMemSize;
	};

protected:
	DWORD GetReportCount()
	{
		return ::GetPrivateProfileInt(SECTION_NAME_REPORT, KEY_REPORT_ALREADY,0, m_strIniPath);
	}


public:

	// 统计开机时间
	VOID StaticsSTProcTime()
	{
		DWORD nCountRpt = GetReportCount();
		if ( nCountRpt < MAX_REPORT_NUM )
		{
			CString							strRpt;
			CSimpleArray<PROC_RUN_TIME>		arrProcTimeList;
			DWORD							nErrorCode = 0;
			CProcPrivilege					procPriv;
			DWORD							nEnterpasswordTime;

			procPriv.SetPri(TRUE,SE_DEBUG_NAME);

			GetProcStartTime(arrProcTimeList,nErrorCode,nEnterpasswordTime);


			DWORD	nBeginCount = GetTickCount();
			DWORD	nRes = WaitForStartComplete();
			DWORD	nEndCount = GetTickCount();

		}
	}

	DWORD WaitForStartComplete()
	{
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

		DWORD nUseLowTimes   = 0;
		NT::CNTSystem	sysInfo;
		NT::SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo; 
		NT::SYSTEM_TIME_INFORMATION SysTimeInfo; 
		NT::SYSTEM_BASIC_INFORMATION SysBaseInfo; 
		double dbIdleTime; 
		double dbSystemTime; 
		double dbDiskRate;
/*		LONG status; */
		LARGE_INTEGER liOldIdleTime = {0,0}; 
		LARGE_INTEGER liOldSystemTime = {0,0};
		LARGE_INTEGER liOldReadDisk = {0,0};
		LARGE_INTEGER liOldWriteDisk = {0,0};


		try
		{
			// 获取失败的话返回1
			if ( !sysInfo.GetBasicInfo(SysBaseInfo) )
				return 1;

			// 最多循环3分钟
			for (int i=0; i<30; i++ )
			{
				memset(&SysTimeInfo,0,sizeof(NT::SYSTEM_TIME_INFORMATION));
				if ( !sysInfo.GetTimeInfo(SysTimeInfo) )
					return 1;

				memset(&SysPerfInfo,0,sizeof(NT::SYSTEM_PERFORMANCE_INFORMATION));
				if ( !sysInfo.GetPerformanceInfo(SysPerfInfo) )
					return 1;

				// if it's a first call - skip it 
				if (liOldIdleTime.QuadPart != 0) 
				{ 
					dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
					if (dbSystemTime != 0.0)
					{
						dbIdleTime   = Li2Double(SysPerfInfo.IdleTime) - Li2Double(liOldIdleTime); 
						dbIdleTime = dbIdleTime / dbSystemTime;
						dbIdleTime = 100.0 - dbIdleTime * 100.0 /(double)SysBaseInfo.bKeNumberProcessors + 0.5;
						dbDiskRate = Li2Double(SysPerfInfo.ReadTransferCount) - Li2Double(liOldReadDisk) ;
						dbDiskRate += Li2Double(SysPerfInfo.WriteTransferCount) - Li2Double(liOldWriteDisk);
						dbDiskRate = 100.0*dbDiskRate/dbSystemTime;

						// 磁盘速度比较低，CPU使用在10% 以下
						if (/*dbDiskRate < 10.0 &&*/ dbIdleTime < 10.0)
							nUseLowTimes++;
						else
							nUseLowTimes = 0;

						// 成功返回0
						if (nUseLowTimes > 9)
							return 0;
					}
				}

				liOldIdleTime = SysPerfInfo.IdleTime; 
				liOldSystemTime = SysTimeInfo.liKeSystemTime;

				liOldReadDisk = SysPerfInfo.ReadTransferCount; 
				liOldWriteDisk = SysPerfInfo.WriteTransferCount;

				// wait one second 
				Sleep(1000); 
			}
			return 2;
		}
		catch (...)
		{
			
		}
		return 3;
	}

	BOOL IsFilterProcessName(LPCTSTR lpName)
	{
		static LPCTSTR lpProcName[] = 
		{
			_T("svchost.exe"), 
			_T("csrss.exe"), 
			_T("services.exe"), 
			_T("smss.exe"), 
			_T("lsass.exe"), 
			_T("system"), 
		};

		for ( int i=0; i < sizeof(lpProcName)/sizeof(lpProcName[0]); i++)
		{
			if ( _tcsicmp( lpProcName[i], lpName) == 0 )
				return TRUE;
		}
		return FALSE;
	}

protected:
	VOID GetProcStartTime(CSimpleArray<PROC_RUN_TIME>& timeList, DWORD & dwErrorCode, DWORD& nLoginTime )
	{
		HANDLE			hProcList = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		PROCESSENTRY32	pl32 = {0};
		NT::CNTProcess	ntProc;
		FILETIME		ftExpCreate;
		FILETIME		ftWinlgnCreate;

		nLoginTime  = 0;
		dwErrorCode = RUNSTC_ERROR_CODE_SUCCESS;
		if ( hProcList != NULL )
		{
			BOOL			bFindExplore	= FALSE;
			BOOL			bFindWinlogon	= FALSE;
			BOOL			bQueryWinlogon	= FALSE;
			BOOL			bQueryExplorer	= FALSE;
			BOOL			bOpenWinlogon	= FALSE;
			BOOL			bOpenExplorer	= FALSE;

			memset(&pl32,0,sizeof(PROCESSENTRY32));
			pl32.dwSize	= sizeof(PROCESSENTRY32);
			if ( Process32First(hProcList, &pl32) )
			{
				do 
				{
					BOOL	bIsExplorer	= FALSE;
					BOOL	bIsWinlogon = FALSE;

					if ( _tcsicmp( pl32.szExeFile, _T("explorer.exe")) == 0 )
					{
						bFindExplore	= TRUE;
						bIsExplorer		= TRUE;
					}
					else if ( _tcsicmp( pl32.szExeFile, _T("winlogon.exe")) == 0 )
					{
						bFindWinlogon	= TRUE;
						bIsWinlogon		= TRUE;
					}

					if ( !IsFilterProcessName(pl32.szExeFile) )
					{
						HANDLE	hProc = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pl32.th32ProcessID );
						if ( hProc != NULL )
						{	
							PROC_RUN_TIME		timex;
							TCHAR				szKAVEMod[MAX_PATH] = {0};
							FILETIME			timeKernel;
							FILETIME			timeUser;
							FILETIME			ftCreate;
							FILETIME			ftExit;

							GetModuleFileNameEx(hProc,NULL,szKAVEMod,MAX_PATH);

							timex.nTime			= 0;
							if ( szKAVEMod[0] != 0 )
								timex.strPath = szKAVEMod;
							else
								timex.strPath	= pl32.szExeFile;

							timex.strPath.Replace( _T("?"), _T("") );
							if ( ::GetProcessTimes(hProc,&ftCreate,&ftExit,&timeKernel,&timeUser) )
							{
								timex.nTime = timeKernel.dwLowDateTime/100 + timeUser.dwLowDateTime/100;
							}
							else
							{
								if ( bIsExplorer )
									bQueryExplorer = TRUE;
								else if ( bIsWinlogon )
									bQueryWinlogon = TRUE;
							}

							if (TRUE)
							{
								NT::IO_COUNTERS	ioCounter;
								
								if ( ntProc.QueryIOCounter(hProc,ioCounter) )
								{
									timex.nIoRead  = (DWORD)ioCounter.ReadOperationCount;
									timex.nIoWrite = (DWORD)ioCounter.WriteOperationCount;
								}

								NT::VM_COUNTERS	vmCounters;
								if (ntProc.QueryVMMemory(hProc,vmCounters) )
								{
									timex.nMemSize = (DWORD)vmCounters.WorkingSetSize;
								}
							}

							if (bIsExplorer)
								ftExpCreate = ftCreate;
							else if (bIsWinlogon)
								ftWinlgnCreate = ftCreate;

							timeList.Add(timex);
							
							::CloseHandle(hProc);
						}
						else
						{
							if ( bIsExplorer )
								bOpenExplorer = TRUE;
							else if ( bIsWinlogon )
								bOpenWinlogon = TRUE;
						}
					}

					memset(&pl32,0,sizeof(PROCESSENTRY32));
					pl32.dwSize	= sizeof(PROCESSENTRY32);					
				} while ( Process32Next(hProcList, &pl32) );
			}
			::CloseHandle(hProcList);

			if ( !bFindWinlogon )
				dwErrorCode = RUNSTC_ERROR_CODE_NO_WINLOGON;
			else if ( !bFindExplore )
				dwErrorCode = RUNSTC_ERROR_CODE_NO_EXPLORER;
			else if ( bOpenExplorer )
				dwErrorCode = RUNSTC_ERROR_CODE_OPEN_EXPLORER;
			else if ( bOpenWinlogon )
				dwErrorCode = RUNSTC_ERROR_CODE_OPEN_WINLOGON;
			else if ( bQueryWinlogon )
				dwErrorCode = RUNSTC_ERROR_CODE_GETT_WINLOGON;
			else if ( bQueryExplorer )
				dwErrorCode = RUNSTC_ERROR_CODE_GETT_EXPLORER;
		}
		else
		{
			dwErrorCode = RUNSTC_ERROR_CODE_CREATE_SNAP;
		}

		if (dwErrorCode==RUNSTC_ERROR_CODE_SUCCESS)
		{
			CTime	timxExp( ftExpCreate );
			CTime	timxWlg( ftWinlgnCreate);

			nLoginTime = (DWORD)timxExp.GetTime() - (DWORD)timxWlg.GetTime();
		}

		return;
	}

protected:

	CString							m_strIniPath;
	HANDLE							m_hEventEnum;
};

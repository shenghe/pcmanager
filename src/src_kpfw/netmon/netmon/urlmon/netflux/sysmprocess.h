////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : sysprocess.h
//      Version   : 1.0
//      Comment   : 定义自动放行的系统进程路径
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <map>
#include <string>
#include "kpfw/msg_logger.h"
using namespace std;

struct KDFT_APP_INFO
{
	BOOL bValid;
};

#define _ADDDEFTAPP(root, fn, desc, valid)\
{\
	AddProcess(root, fn, desc, valid);	\
}

class KSysApps
{
private:
	typedef map<ATL::CString, KDFT_APP_INFO> theMap;

	map<ATL::CString, KDFT_APP_INFO>		m_SysProcess;

	KSysApps()
	{
		AddSystemPath();

		AddKpfwPath();

		AddWeishiPath();
		// AddDubaPath();
	}

	void	AddDubaPath()
	{
		// 添加毒霸2010程序路径
		CRegKey key;
		if (key.Open(HKEY_LOCAL_MACHINE, TEXT("software\\kingsoft\\antivirus"), KEY_READ) == ERROR_SUCCESS)
		{
			WCHAR path[MAX_PATH];
			ULONG nPathCnt = 0;
			if (key.QueryStringValue(TEXT("ProgramPath"), path, &nPathCnt) == ERROR_SUCCESS)
			{
				/*
				_ADDDEFTAPP(path, 
					TEXT("kavlog2.exe"),
					TEXT("kingsoft duba module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kavstart.exe"),
					TEXT("kingsoft duba module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kismain.exe"),
					TEXT("kingsoft duba module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("ksamain.exe"),
					TEXT("kingsoft duba module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("krecycle.exe"),
					TEXT("kingsoft duba module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("setupwiz.exe"),
					TEXT("kingsoft duba module"),
					TRUE);*/
			}
		}

		// 添加大引擎路径
		if (key.Open(HKEY_LOCAL_MACHINE, TEXT("software\\kingsoft\\kiscommon\\kxengine"), KEY_READ) == ERROR_SUCCESS)
		{
			WCHAR path[MAX_PATH];
			ULONG nPathCnt = 0;
			if (key.QueryStringValue(TEXT("path"), path, &nPathCnt) == ERROR_SUCCESS)
			{
				INT nLen = wcslen(path);
				if (path[nLen-1] != L'\\' || path[nLen-1] != L'/')
					wcscat(path, TEXT("\\"));

				_ADDDEFTAPP(path, 
					TEXT("deinstall.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kiscall.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kpopserver.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kpopui.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("ksaserver.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kxecolct.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kxeserv.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kxeupsrv.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				_ADDDEFTAPP(path, 
					TEXT("kxfwsserv.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);

				/*
				_ADDDEFTAPP(path, 
					TEXT("scomregsvr.exe"),
					TEXT("kingsoft common engine module"),
					TRUE);*/
			}
		}
	}

	void	AddWeishiPath()
	{
		TCHAR kpfwdir[MAX_PATH] = {0};
		GetModuleFileName(NULL, kpfwdir, MAX_PATH);
		TCHAR* pEnd = _tcsrchr(kpfwdir, TEXT('\\'));
		if (pEnd)
		{
			*(pEnd+1) = 0;

			_ADDDEFTAPP(kpfwdir, 
				TEXT("KSafeSvc.exe"),
				TEXT("weishi service"),
				TRUE);

			_ADDDEFTAPP(kpfwdir, 
				TEXT("KSafeTray.exe"),
				TEXT("weishi tray"),
				TRUE);
		}
	}

	void	AddKpfwPath()
	{
		TCHAR kpfwdir[MAX_PATH] = {0};
		GetModuleFileName(NULL, kpfwdir, MAX_PATH);
		TCHAR* pEnd = _tcsrchr(kpfwdir, TEXT('\\'));
		if (pEnd)
		{
			*(pEnd+1) = 0;

			_ADDDEFTAPP(kpfwdir, 
				TEXT("kpfwsvr.exe"),
				TEXT("kingsoft personal firewall service"),
				TRUE);

			_ADDDEFTAPP(kpfwdir, 
				TEXT("kpfwtray.exe"),
				TEXT("kingsoft personal firewall tray"),
				TRUE);

			/*
			_ADDDEFTAPP(kpfwdir, 
			TEXT("kpfwtray.exe"),
			TEXT("kingsoft personal firewall tray"),
			TRUE);

			_ADDDEFTAPP(kpfwdir, 
			TEXT("kpfwfrm.exe"),
			TEXT("kingsoft personal firewall ui"),
			TRUE);

			_ADDDEFTAPP(kpfwdir, 
			TEXT("kuplive.exe"),
			TEXT("kingsoft personal firewall service"),
			TRUE);*/
		}
	}
	void	AddSystemPath()
	{
		TCHAR SYSTEM32[MAX_PATH] = {0};
		GetSystemDirectory(SYSTEM32, MAX_PATH);
		_tcscat(SYSTEM32, _T("\\"));

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("Kernel32.dll"),
			TEXT("Win32 Kernel core component"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("csrss.exe"),
			TEXT("Client Server Runtime Process"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("MSTask.exe"),
			TEXT("Task Scheduler Engine"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("services.exe"),
			TEXT("Services and Controller app"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("lsass.exe"),
			TEXT("LSA Executable and Server DLL (Export Version)"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("spoolsv.exe"),
			TEXT("Spooler SubSystem App"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("MPREXE.EXE"),
			TEXT("Win32 Network interface Server"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("tcpsvcs.exe"),
			TEXT("TCP/IP services Application"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("svchost.exe"),
			TEXT("Generic Host Process for Win32"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("RNAAPP.EXE"),
			TEXT("Dial-up Networking Application"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("winlogon.exe"),
			TEXT("Winlogon.exe"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("RpcSs.exe"),
			TEXT("Distribute COM Servicesn"),
			TRUE);

		_ADDDEFTAPP(SYSTEM32, 
			TEXT("wininit.exe"),
			TEXT("wininit.exe"),
			TRUE);
	}
	BOOL	AddProcess(LPCWSTR root, LPCWSTR file, LPCWSTR desc, BOOL b)
	{
		ATL::CString tmp = root;
		tmp += file;
		tmp.MakeLower();

		kws_log(TEXT("add proces to pass: %s"), (LPCWSTR)tmp);
		KDFT_APP_INFO info = {b};
		m_SysProcess.insert(make_pair(tmp, info));
		return TRUE;
	}

public:
	static KSysApps* Instance()
	{
		static KSysApps _instance;
		return &_instance;
	}

	BOOL Search(LPCWSTR key)
	{
		ATL::CString tmp = key;
		tmp.MakeLower();

		theMap::iterator it = m_SysProcess.find(tmp);
		
		if (it != m_SysProcess.end())
			return it->second.bValid;
		return FALSE;
	}
};
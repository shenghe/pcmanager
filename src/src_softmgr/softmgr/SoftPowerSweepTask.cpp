#include "stdafx.h"
#include "SoftPowerSweepTask.h"
#include "SoftUninstall.h"
#include "SoftUninstallApi.h"
#include "SoftUninstallSql.h"
#include <skylark2/bkdb.h>
using namespace Skylark;

#include <stlsoft/stlsoft.h>
#include <stlsoft/string/trim_functions.hpp>
#include <winstl/registry/reg_value.hpp>
#include <winstl/registry/reg_key_sequence.hpp>
#include <winstl/registry/reg_value_sequence.hpp>
#include <winstl/filesystem/filesystem_traits.hpp>
#include <comstl/util/creation_functions.hpp>
using namespace stlsoft;
using namespace winstl;
using namespace comstl;

#include <set>
#include <string>
#include <algorithm>
using namespace std;
#include <hash_map>
using namespace stdext;

#include <shlobj.h>
#include <ShellAPI.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Rpcrt4.lib")

namespace ksm
{

inline ULARGE_INTEGER FileTime2ULargeInteger(const FILETIME &fileTime);
//////////////////////////////////////////////////////////////////////////
BOOL CSoftPowerSweepTask::TaskProcess(class conew::CTaskMgr *) 
{
	ISoftUnincallNotify *pNotify = _pSoftUninst->GetNotify();
	pNotify->SoftDataSweepEvent( UE_Sweep_Begin, this);

	do
	{
		if(!LoadSoftUninstDataByKey(_pSoftUninst->GetDBPtr(), _key.c_str(), _curSoftData))
		{
			pNotify->SoftDataSweepEvent(UE_Sweep_Failed, this);
			break;
		}
		if(_bCancel) break;

		//
		// 64位关闭文件重定向功能
		//
		auto_ptr<CFsRedDisScoped> pFsRedirectDisabledScoped;
		if(_x64)
		{
			pFsRedirectDisabledScoped = auto_ptr<CFsRedDisScoped>(new CFsRedDisScoped);
		}

		// 根据主目录文件个数来判断用户是否取消调用卸载程序
		FileCounter fileCnt;
		if(!_curSoftData._mainPath.empty())
		{
			//
			// 在目录后端增加Slash
			//
			//@Remark
			//
			// C:\Program Files\Tencent\QQ
			// C:\Program Files\Tencent\QQSoftMgr
			//
			trim_right(_curSoftData._mainPath, L"\\");
			_curSoftData._mainPath += L'\\';

			RecurseEnumFile(fileCnt, _curSoftData._mainPath, L"*.*", 1);
		}

		// 通知标准卸载开始
		pNotify->SoftDataSweepEvent(UE_Invoke_Uninst, this);
		if(_InvokeUninstProgram(const_cast<LPWSTR>(_curSoftData._uninstString.c_str())))
		{
			UninstEvent event = UE_Invoke_Succeeded;
			if (!_curSoftData._mainPath.empty())
			{
				FileCounter curFileCnt;
				RecurseEnumFile(curFileCnt, _curSoftData._mainPath, L"*.*", 1);

				if(fileCnt._count <= curFileCnt._count)
				{
					event = UE_Invoke_Canceled;
				}
			}

			pNotify->SoftDataSweepEvent(event, this);
		}
		else
		{
			pNotify->SoftDataSweepEvent(UE_Invoke_Failed,	this);
		}
		if(_bCancel) break;

		// 修正主目录
		if(!_x64 && !_curSoftData._guid.empty())
		{
			FixMainPath();
			if(_bCancel) break;
		}

		// 是否主目录里装了其他软件
		if (!_curSoftData._mainPath.empty() && _isMultiDirInstallOneDir())
		{
			pNotify->SoftDataSweepEvent(UE_Enum_Multi, this);
			break;
		}

		if(_bCancel) break;

		if(!_PowerCleanScanFile()) break;
		if(_bCancel) break;

		_PowerCleanScanReg();
		if(_bCancel) break;

		pNotify->SoftDataSweepEvent( UE_Enum_End, this);

		::WaitForSingleObject( _eventWait, -1 );
		if(_bCancel) break;

		_beginRubbishSweep();

		// 清理完成的时候确定软件是否还能被扫到, 如果已经完全卸载，则从数据库中删除
		if(!DataSoftExists())
		{
			_deleteSoftFromDB();
		}
	}
	while(FALSE);

	pNotify->SoftDataSweepEvent( UE_Sweep_End, this);
	return FALSE; 
}

void CSoftPowerSweepTask::FixMainPath()
{
	static const wstring userDataKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";

	WStrList dirList;
	wstring hexGuid;
	Guid2Hex(_curSoftData._guid, hexGuid);

	KeyList keyList;
	RegListKey(HKEY_LOCAL_MACHINE, userDataKey, keyList);

	KeyCIter end = keyList.end();
	for(KeyCIter it = keyList.begin(); !_bCancel && it != end; ++it)
	{
		KeyList keyList2;
		wstring subkey = userDataKey + L'\\' + *it + L"\\Components";
		RegListKey(HKEY_LOCAL_MACHINE, subkey, keyList2);

		KeyCIter end2 = keyList2.end();
		for(KeyCIter it2 = keyList2.begin(); !_bCancel && it2 != end2; ++it2)
		{
			try
			{
				wstring path = subkey + L'\\' + *it2;
				reg_key_w key(HKEY_LOCAL_MACHINE, path);
				if(!key.has_value(hexGuid)) continue;

				// 记录当前注册表项
				_regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_LOCAL_MACHINE\\") + path + L"\\\\" + hexGuid));

				// 处理目录：C:
				if(_curSoftData._mainPath.empty())
				{
					reg_key_w::key_value_type val = key.get_value(hexGuid);
					wstring str = val.value_expand_sz();

					if(str.size() < 2 || str[1] != L':') continue;

					make_lower(str);
					if(filesystem_traits<wchar_t>::is_directory(str.c_str()))
					{
						trim_right(str, L"\\");
					}
					else
					{
						RemoveFileSpec(str);
					}

					bool add = true;

					WStrListIter end = dirList.end();
					for(WStrListIter it = dirList.begin(); it != end;)
					{
						if(str.size() < it->size())
						{
							if(wcsncmp(it->c_str(), str.c_str(), str.size()) == 0)
							{
								dirList.erase(it++);
								continue;
							}
						}
						else
						{
							if(wcsncmp(str.c_str(), it->c_str(), it->size()) == 0)
							{
								add = false;
								break;
							}
						}

						++it;
					}

					if(add) dirList.push_back(str);
				}
			}
			catch(...) {}
		}
	}

	if(_curSoftData._mainPath.empty())
	{
		WStrListIter end3 = dirList.end();
		for(WStrListIter it3 = dirList.begin(); it3 != end3;)
		{
			if(_sensPaths.IsSensitive(*it3) || it3->find(L"\\common files\\") != wstring::npos)
				dirList.erase(it3++);
			else
				++it3;
		}

		if(!dirList.empty())
		{
			_curSoftData._mainPath = dirList.front();

			//
			// 更新程序主目录
			//
			CComPtr<ISQLiteComStatement3> pStateUpdate;
			HRESULT hr = _pSoftUninst->GetDBPtr()->PrepareStatement(L"update local_soft_list set main_path=? where soft_key=?;", &pStateUpdate);
			if(SUCCEEDED(hr))
			{
				pStateUpdate->Bind(1, _curSoftData._mainPath.c_str());
				pStateUpdate->Bind(2, _curSoftData._key.c_str());

				pStateUpdate->ExecuteUpdate();
			}
		}
	}
}

BOOL CSoftPowerSweepTask::_InvokeUninstProgram(LPWSTR pCmd)
{
	typedef set<DWORD> PidSet;

	PidSet pidSet;
	STARTUPINFO	si = { sizeof(si) };
	PROCESS_INFORMATION	pi = {0};

	if(!::CreateProcessW(NULL, pCmd, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
		return FALSE;

	pidSet.insert(pi.dwProcessId);
	ULARGE_INTEGER startTime;
	{
		FILETIME createTime, exitTime, kernelTime, userTime;
		::GetProcessTimes(pi.hProcess, &createTime, &exitTime, &kernelTime, &userTime);

		startTime = FileTime2ULargeInteger(createTime);
	}


	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);

	while(TRUE)
	{
		DWORD ret = ::WaitForSingleObject(pi.hProcess, 200);
		if(_bCancel || ret == WAIT_OBJECT_0)
		{
			::CloseHandle(pi.hProcess);
			break;
		}
	}

	while(!_bCancel)
	{
		PidSet ps;
		CProcessSnap snap;
		snap.Snap();

		for(int i = 0; i < snap.GetSize(); ++i)
		{
			if(pidSet.find(snap[i].th32ProcessID) != pidSet.end())
			{
				ps.insert(snap[i].th32ProcessID);
				continue;
			}

			if(_IsBrowserProgram(snap[i].szExeFile)) continue;

			if(pidSet.find(snap[i].th32ParentProcessID) != pidSet.end())
			{
				ps.insert(snap[i].th32ProcessID);
				continue;
			}

			//
			//@Remark
			//	异常情况处理：
			//　对于Firefox，会在极短的时间内产生进程序列：helper.exe => uninstaller.exe => Au_.exe
			//
			if(_wcsicmp(snap[i].szExeFile, L"au_.exe") == 0)
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, snap[i].th32ProcessID);
				if(hProcess != NULL)
				{
					FILETIME createTime, exitTime, kernelTime, userTime;
					::GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime);

					ULARGE_INTEGER ulCreate = FileTime2ULargeInteger(createTime);
					if(ulCreate.QuadPart > startTime.QuadPart && ulCreate.QuadPart < startTime.QuadPart + 5*10000000)
					{
						ps.insert(snap[i].th32ProcessID);
					}

					::CloseHandle(hProcess);
				}
			}
		}

		pidSet.swap(ps);
		if(_bCancel || pidSet.empty()) break;

		DWORD ret = ::WaitForSingleObject(_eventWait, 200);
		if(_bCancel || ret != WAIT_TIMEOUT) break;
	}

	return TRUE;
}

BOOL CSoftPowerSweepTask::_IsBrowserProgram(LPCWSTR pName)
{
	static const LPCWSTR lpIEArray[] = 
	{
		L"explor",
		L"opera",
		L"firefox",
		L"tt",
		L"chrome",
		L"360se",
		L"theworld",
		L"maxth",
		L"kinstool",
	};

	for(int i = 0; i < STLSOFT_NUM_ELEMENTS(lpIEArray); ++i)
	{
		if(StrStrNIW(pName, lpIEArray[i], MAX_PATH) != NULL) return TRUE;
	}

	return FALSE;
}

BOOL CSoftPowerSweepTask::_PowerCleanScanFile()
{
	if(
		_curSoftData._mainPath.empty() || 
		!filesystem_traits<wchar_t>::file_exists( _curSoftData._mainPath.c_str())
		) 
		return TRUE;

	SoftRubbish2List mainRubbish;
	if(!_PowerCleanScanMain(mainRubbish))
	{
		_pSoftUninst->GetNotify()->SoftDataSweepEvent(UE_Enum_Too_Much, this);
		return FALSE;
	}

	//
	// 2.快速启动栏、启动文件夹、桌面、开始菜单
	//
	SoftRubbish2List lnkRubbish;
	_PowerCleanScanOther(lnkRubbish);
	if(_bCancel) return TRUE;

	//
	// 合并目录
	//
	_rubbishList.splice(_rubbishList.end(), lnkRubbish);
	_rubbishList.splice(_rubbishList.end(), mainRubbish);
	return TRUE;
}

BOOL CSoftPowerSweepTask2::_PowerCleanScanMain(SoftRubbish2List &rubbish2List)
{
	SoftRubbish2List rubbish2List2;
	MainPathEnum mainPathEnum(rubbish2List2);
	rubbish2List.push_back(SoftRubbish2(SIA_Directory, _curSoftData._mainPath));

	wstring mainPath = _curSoftData._mainPath;
	RecurseEnumFile(mainPathEnum, trim_right(mainPath, L"\\"));

	BOOL ret = FALSE;
	_rubbishSize = mainPathEnum.GetSize();

	do
	{
		// 1.判断是否大于100MB
		if(_rubbishSize > 100*1024*1024) break;

		// 2.计算同类型文件数量
		typedef hash_map<wstring, ULONG>	ExtHash;
		typedef ExtHash::iterator			ExtIter;
		typedef ExtHash::const_iterator		ExtCIter;

		ExtHash extHash;
		SoftRubbish2CIter endRubbish = rubbish2List2.end();
		for(SoftRubbish2CIter it = rubbish2List2.begin(); it != endRubbish; ++it)
		{
			if((it->_type & SIA_File) != SIA_File) continue;

			LPWSTR pExt = ::PathFindExtensionW(&it->_data[0]);
			if(pExt == NULL) continue;

			++extHash[pExt];
		}

		ULONG sum = 0;
		ExtCIter endExt = extHash.end();
		for(ExtCIter it = extHash.begin(); it != endExt; ++it)
		{
			if(it->second > sum) sum = it->second;
		}

		// 3.判断同类型文件数量是否大于20
		if(sum >= 20) break;

		// 4.判断是否存在常用软件类型
		if(
			extHash[L".doc"] > 5 ||
			extHash[L".docx"] > 5 ||
			extHash[L".xls"] > 5 ||
			extHash[L".xlsx"] > 5 ||
			extHash[L".ppt"] > 5 ||
			extHash[L".pptx"] > 5 ||
			extHash[L".zip"] > 5 ||
			extHash[L".rar"] > 5 ||
			extHash[L".7z"] > 5 ||
			extHash[L".avi"] > 5 ||
			extHash[L".mp4"] > 5 ||
			extHash[L".mp3"] > 5 ||
			extHash[L".jpg"] > 5 ||
			extHash[L".bak"] > 5
			)
			break;

		ret = TRUE;
	}
	while(FALSE);

	rubbish2List.splice(rubbish2List.end(), rubbish2List2);
	return ret;
}

BOOL CSoftPowerSweepTask::_PowerCleanScanMain(SoftRubbish2List &rubbish2List)
{
	MainPathEnum mainPathEnum(rubbish2List);
	rubbish2List.push_back(SoftRubbish2(SIA_Directory, _curSoftData._mainPath));

	wstring mainPath = _curSoftData._mainPath;
	RecurseEnumFile(mainPathEnum, trim_right(mainPath, L"\\"));
	return TRUE;
}

BOOL CSoftPowerSweepTask::_PowerCleanScanOther(SoftRubbish2List &rubbish2List)
{
	WStrList linkList;
	LinkFileEnum linkEnum(linkList);
	wchar_t path[MAX_PATH];

	::SHGetSpecialFolderPathW(NULL, path, CSIDL_APPDATA, FALSE);
	::PathRemoveBackslashW(path);
	wcscat_s(path, MAX_PATH, L"\\Microsoft\\Internet Explorer\\Quick Launch");
	RecurseEnumFile(linkEnum, path, L"*.*", 5);
	if(_bCancel) return TRUE;

	::SHGetSpecialFolderPathW(NULL, path, CSIDL_STARTUP, FALSE);
	::PathRemoveBackslashW(path);
	RecurseEnumFile(linkEnum, path, L"*.*", 5);
	if(_bCancel) return TRUE;

	::SHGetSpecialFolderPathW(NULL, path, CSIDL_DESKTOPDIRECTORY, FALSE);
	::PathRemoveBackslashW(path);
	RecurseEnumFile(linkEnum, path, L"*.*", 5);
	if(_bCancel) return TRUE;

	::SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
	::PathRemoveBackslashW(path);
	RecurseEnumFile(linkEnum, path, L"*.*", 5);
	if(_bCancel) return TRUE;

	::SHGetSpecialFolderPathW(NULL, path, CSIDL_STARTMENU, FALSE);
	::PathRemoveBackslashW(path);
	RecurseEnumFile(linkEnum, path, L"*.*", 5);
	if(_bCancel) return TRUE;

	::SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_STARTMENU, FALSE);
	::PathRemoveBackslashW(path);
	RecurseEnumFile(linkEnum, path, L"*.*", 5);
	if(_bCancel) return TRUE;

	ParseLinkList(linkList, rubbish2List);
	return TRUE;
}

void CSoftPowerSweepTask::ParseLinkList(const WStrList &linkList, SoftRubbish2List &rubbish2List)
{
	CLinkOpr linkOpr;
	if(!linkOpr.Initialize()) return;

	wchar_t path[MAX_PATH];
	WStrListCIter end = linkList.end();

	for(WStrListCIter it = linkList.begin(); !_bCancel && it != end; ++it)
	{
		if(!linkOpr.GetPath(it->c_str(), path, MAX_PATH)) continue;

		if(::PathFileExistsW(path) && _IsIncludedPath(path))
		{
			rubbish2List.push_back(SoftRubbish2(SIA_Link, *it));
		}
	}
}

BOOL CSoftPowerSweepTask::_IsIncludedPath2(LPCWSTR lpPath)
{
	if(!filesystem_traits<wchar_t>::file_exists(lpPath))
	{
		int argc;
		LPWSTR *pArgv = ::CommandLineToArgvW(lpPath, &argc);
		if(pArgv != NULL)
		{
			BOOL ret = _IsIncludedPath(pArgv[0]);
			::LocalFree(pArgv);
			return ret;
		}
	}

	return _IsIncludedPath(lpPath);
}

VOID CSoftPowerSweepTask::_PowerCleanScanReg()
{
	SoftRubbish2List regList;
	REGSAM samDesired = KEY_READ;
	if(_x64) samDesired |= KEY_WOW64_64KEY;

	if(!_curSoftData._mainPath.empty())
	{
		// 注册表Run项
		try
		{
			static const wstring subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

			{
				reg_value_sequence_w valList(HKEY_LOCAL_MACHINE, subkey.c_str(), samDesired);
				reg_value_sequence_w::const_iterator end = valList.end();
				for(reg_value_sequence_w::const_iterator it = valList.begin(); it != end; ++it)
				{
					reg_value_w val = *it;
					wstring path = val.value_expand_sz();

					if(_IsIncludedPath2(path.c_str()))
					{
						regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_LOCAL_MACHINE\\") + subkey + L"\\\\" + val.name()));
					}
				}
			}

			{
				reg_value_sequence_w valList(HKEY_CURRENT_USER, subkey.c_str(), samDesired);
				reg_value_sequence_w::const_iterator end = valList.end();
				for(reg_value_sequence_w::const_iterator it = valList.begin(); it != end; ++it)
				{
					reg_value_w val = *it;
					wstring path = val.value_expand_sz();

					if(_IsIncludedPath2(path.c_str()))
					{
						regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_CURRENT_USER\\") + subkey + L"\\\\" + val.name()));
					}
				}
			}
		}
		catch(...) {}

		if(_bCancel) return;

		// MUICache
		try
		{
			static const wstring subkey = L"Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache";

			KeyList keyList;
			RegListKey(HKEY_USERS, wstring(), keyList, samDesired);

			KeyCIter end = keyList.end();
			for(KeyCIter it = keyList.begin(); it != end; ++it)
			{
				KeyList keyList2;
				RegListKey(HKEY_USERS, subkey, keyList2, samDesired);

				KeyCIter end2 = keyList2.end();
				for(KeyCIter it2 = keyList2.begin(); it2 != end2; ++it2)
				{
					if(_IsIncludedPath2(it2->c_str()))
					{
						regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_CURRENT_USER\\") + subkey + L"\\\\" + *it2));
					}
				}
			}
		}
		catch(...) {}

		if(_bCancel) return;

		// 右键
		try
		{
			static LPCWSTR pSubkeys[] = 
			{
				L"*\\shellex\\ContextMenuHandlers",
				L"Folder\\ShellEx\\ContextMenuHandlers",
			};

			for(int i = 0; i < STLSOFT_NUM_ELEMENTS(pSubkeys); ++i)
			{
				KeyList keyList;
				RegListKey(HKEY_CLASSES_ROOT, pSubkeys[i], keyList, samDesired);

				KeyCIter end = keyList.end();
				for(KeyCIter it = keyList.begin(); it != end; ++it)
				{
					reg_key_w key2(HKEY_CLASSES_ROOT, (wstring(pSubkeys[i]) + L'\\' + *it), samDesired);
					wstring guid = key2.get_value(wstring()).value_sz();

					if(IsGuid(guid))
					{
						try
						{
							reg_key_w key3(HKEY_CLASSES_ROOT, (wstring(L"CLSID\\") + guid + L"\\InProcServer32"), samDesired);
							wstring path = key3.get_value(wstring()).value_expand_sz();

							if(_IsIncludedPath2(path.c_str()))
							{
								regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_CLASSES_ROOT\\") + pSubkeys[i] + L'\\' + *it));
								regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_CLASSES_ROOT\\CLSID\\") + guid));
							}
						}
						catch(...) {}
					}
				}
			}
		}
		catch(...) {}

		if(_bCancel) return;
	}

	// installer与uninstall
	try
	{
		// uninstall
		if(IsUninstallKey(_key))
		{
			wstring name = _curSoftData._key.substr(10);
			static const wstring subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

			reg_key_w key(HKEY_LOCAL_MACHINE, subkey, samDesired);
			if(key.has_sub_key(name))
			{
				regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_LOCAL_MACHINE\\") + subkey + L'\\' + name));
			}

			reg_key_w key2(HKEY_CURRENT_USER, subkey, samDesired);
			if(key2.has_sub_key(name))
			{
				regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_CURRENT_USER\\") + subkey + L'\\' + name));
			}
		}

		// installer
		if(!_curSoftData._guid.empty())
		{
			wstring hexGuid;
			Guid2Hex(_curSoftData._guid, hexGuid);
			static const wstring subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";

			KeyList userList;
			RegListKey(HKEY_LOCAL_MACHINE, subkey, userList, samDesired);

			KeyCIter end = userList.end();
			for(KeyCIter it = userList.begin(); it != end; ++it)
			{
				wstring path = wstring(subkey) + L'\\' + *it + L"\\Products";
				reg_key_w key(HKEY_LOCAL_MACHINE, path, samDesired);
				if(key.has_sub_key(hexGuid))
				{
					regList.push_back(SoftRubbish2(SIA_Registry, wstring(L"HKEY_LOCAL_MACHINE\\") + path + L'\\' + hexGuid));
				}
			}
		}
	}
	catch(...) {}

	//
	// 合并结果
	//
	_rubbishList.splice(_rubbishList.end(), regList);
	_rubbishList.splice(_rubbishList.end(), _regList);
}

BOOL CSoftPowerSweepTask::_isMultiDirInstallOneDir()
{
	wstring strMainPath = _curSoftData._mainPath;
	trim_right(strMainPath, L"\\");

	// 从数据库中读取是否有同一目录下安装了多个软件，根据路径匹配来判断
	CComPtr<Skylark::ISQLiteComStatement3> pState;
	HRESULT hr = _pSoftUninst->GetDBPtr()->PrepareStatement(L"select count(*) from local_soft_list where main_path like ? or main_path = ?", &pState);
	if(!SUCCEEDED(hr)) return FALSE;

	wstring strMainPathWithSlash = strMainPath + L"\\%";
	pState->Bind(1, strMainPathWithSlash.c_str());
	pState->Bind(2, strMainPath.c_str());

	return (pState->ExecuteScalar() > 1);
}

VOID CSoftPowerSweepTask::_beginRubbishSweep()
{
	REGSAM samDesired = KEY_WRITE;
	if(_x64) samDesired |= KEY_WOW64_64KEY;
	CSimpleArray<CString>	dirlistRebootMove;

	for ( SoftRubbish2Iter iter = _sweepList.begin(); iter != _sweepList.end(); iter++)
	{
		SoftRubbish2&	rubbish = *iter;
		CString			strPath = rubbish._data.c_str();

		_notifyList.clear();
		_notifyList.push_back(*iter);

		if( rubbish._type != SIA_Registry )
		{
			SHFILEOPSTRUCT fp={0};

			strPath.AppendChar(0);
			fp.pFrom	= strPath.GetBuffer();
			fp.wFunc	= FO_DELETE;
			fp.hwnd		= GetDesktopWindow();

			if ( _brecycle )
			{
				// 删除到回收站(整个目录删除到回收站中方便用户还原)
				fp.fFlags=FOF_NO_UI|FOF_NOERRORUI|FOF_SILENT|FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
			}
			else
			{
				// 彻底删除FOF_NO_UI|FOF_NOERRORUI|FOF_SILENT|
				fp.fFlags=FOF_NO_UI|FOF_NOERRORUI|FOF_SILENT|FOF_NOCONFIRMATION;
			}

			SHFileOperation(&fp);

			if( PathFileExists(strPath) )
			{
				if( PathIsDirectory(strPath) )
				{
					// 如果用SHFileOperation没有成功删除， 则目录放到最后一起删除
					RemoveDirectory(strPath);

					if ( PathIsDirectory(strPath) )
						dirlistRebootMove.Add(strPath);
				}
				else
				{
					if ( !_brecycle )		// 需要删除到回收站中
					{
						// 重启删除之前复制一份放到回收站中， 使重启删除的文件也能恢复
						CString	strNewName;
						strNewName = strPath + _T( ".old" ); 
						MoveFile( strPath, strNewName );
						CopyFile( strNewName, strPath, FALSE );

						// 原名的文件放到回收站
						SHFileOperation(&fp);		
						MoveFileEx(strNewName,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);	
					}
					else
					{
						// 不能删除的文件重启删除
						MoveFileEx(strPath,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
					}
				}
			}
		}
		else
		{
			CString strParent=strPath.Left(strPath.Find(L"\\"));
			HKEY hParent;
			if(strParent==L"HKEY_LOCAL_MACHINE")
				hParent=HKEY_LOCAL_MACHINE;
			if(strParent==L"HKEY_CURRENT_USER")
				hParent=HKEY_CURRENT_USER;
			if(strParent=="HKEY_CLASSES_ROOT")
				hParent=HKEY_CLASSES_ROOT;
			if(strParent=="HKEY_USERS")
				hParent=HKEY_USERS;

			strPath.Replace(strParent+L"\\",L"");

			CString strKey=strPath;
			if(strPath.Find(L"\\\\")>0)
			{
				strKey = strKey.Left(strPath.Find(L"\\\\"));
				strPath.Replace(strKey+L"\\\\",L"");
				CString strValue = strPath;

				_RegDeleteValue(hParent, strKey, strValue, samDesired);
				//::SHDeleteValue(hParent,strKey,strValue);
			}
			else
			{
				RegRecurseDeleteKey(hParent, strKey, samDesired);
				//::SHDeleteKey(hParent,strKey);
			}
		}

		if ( _bCancel )
			break;

		_pSoftUninst->GetNotify()->SoftDataSweepEvent(UE_Sweeping, this);
	}

	if ( !_bCancel && dirlistRebootMove.GetSize() > 0 )
	{
		for ( int i= dirlistRebootMove.GetSize()-1; i >=0 ; --i)
		{
			::MoveFileEx( dirlistRebootMove[i], NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
	}
}

BOOL CSoftPowerSweepTask::DataSoftExists()
{
	try
	{
		REGSAM samDesired = KEY_READ;
		if(_x64) samDesired |= KEY_WOW64_64KEY;

		// uninstall
		if(IsUninstallKey(_key))
		{
			wstring name;
			GetNameKey(_key, name);
			static const wstring subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

			{
				reg_key_w key(HKEY_LOCAL_MACHINE, subkey.c_str(), samDesired);
				if(key.has_sub_key(name)) 
				{
					return TRUE;
				}
			}

			if(!_x64)
			{
				reg_key_w key(HKEY_CURRENT_USER, subkey.c_str(), samDesired);
				if(key.has_sub_key(name)) 
				{
					return TRUE;
				}
			}
		}

		// installer
		if(!_curSoftData._guid.empty())
		{
			wstring hexGuid;
			Guid2Hex(_curSoftData._guid, hexGuid);
			static const wstring subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";

			KeyList userList;
			RegListKey(HKEY_LOCAL_MACHINE, subkey.c_str(), userList, samDesired);

			KeyCIter end = userList.end();
			for(KeyCIter it = userList.begin(); it != end; ++it)
			{
				wstring path = wstring(subkey) + L'\\' + *it + L"\\Products";

				reg_key_w key(HKEY_LOCAL_MACHINE, path, samDesired);
				if(key.has_sub_key(hexGuid)) 
				{
					return TRUE;
				}
			}
		}
	}
	catch(...) {}

	return FALSE;
}

VOID CSoftPowerSweepTask::_deleteSoftFromDB()
{
	if (TRUE)
	{
		CCacheRubbish(_pSoftUninst->GetDBPtr()).Delete(_curSoftData._key);
	}

	if (TRUE)
	{
		CComPtr<Skylark::ISQLiteComStatement3> pState;
		HRESULT hr = _pSoftUninst->GetDBPtr()->PrepareStatement(
			L"delete from local_soft_list where soft_key = ?", &pState);

		if(!SUCCEEDED(hr)) return;

		pState->Bind(1, _curSoftData._key.c_str() );
		pState->ExecuteUpdate();
	}
}

LPCWSTR __stdcall CSoftPowerSweepTask::GetMainDir()
{
	return _curSoftData._mainPath.c_str();
}

inline ULARGE_INTEGER FileTime2ULargeInteger(const FILETIME &fileTime)
{
	ULARGE_INTEGER ul;
	ul.HighPart = fileTime.dwHighDateTime;
	ul.LowPart = fileTime.dwLowDateTime;
	return ul;
}

}
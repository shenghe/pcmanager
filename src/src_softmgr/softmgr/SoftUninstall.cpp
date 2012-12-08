#include "stdafx.h"
#include "SoftUninstall.h"
#include "SoftUninstallApi.h"
#include "SoftUninstallEnum.h"
#include "SoftUninstallTask.h"
#include "SoftPowerSweepTask.h"
#include <assert.h>
#include <stlsoft/string/trim_functions.hpp>
#include <stlsoft/string/case_functions.hpp>
#include <stlsoft/string/string_tokeniser.hpp>
#include <winstl/filesystem/filesystem_traits.hpp>
using namespace winstl;
using namespace stlsoft;
using namespace Skylark;
using namespace conew;
#include <sstream>
#include <iterator>
#include <algorithm>
using namespace std;
#include <process.h>
#include <tinyxml/tinyxml.h>
#include <libheader/libheader.h>
using namespace conew;

namespace ksm
{

//////////////////////////////////////////////////////////////////////////
CSoftUninstall::CSoftUninstall()
{
	_exit = FALSE;
	_firstRefresh = 0UL;
	_pNotify = NULL;
	_regMonitorEvent = NULL;
	_regMonitorThread = HANDLE(-1);

	Cx64Api::Startup();
	CFsRedDisScoped::Startup();
}

CSoftUninstall::~CSoftUninstall()
{
	Cx64Api::Shutdown();
	CFsRedDisScoped::Shutdown();
}

void CSoftUninstall::SetNotify(ISoftUnincallNotify *pNotify)
{
	_pNotify = pNotify;
}

BOOL CSoftUninstall::Initialize(LPCWSTR pKSafePath)
{
	assert(pKSafePath != NULL && _pNotify != NULL);

	_ksafePath = pKSafePath; 
	trim_right(_ksafePath, L"\\"); 
	make_lower(_ksafePath);

	if(
		!filesystem_traits<wchar_t>::file_exists((_ksafePath + SOFT_UNINST_DAT).c_str()) ||
		!filesystem_traits<wchar_t>::file_exists((_ksafePath + SOFT_PINYIN_DAT).c_str()) ||
		!InitCacheDB() ||
		!_commTaskMgr.Startup()
		) 
		return FALSE;

	// 启动初始化任务
	return StartTask(new CSoftInitTask(this));
}

void CSoftUninstall::Uninitialize()
{
	_exit = TRUE;

	if(_regMonitorThread != HANDLE(-1))
	{
		::SetEvent(_regMonitorEvent);
		DWORD ret = ::WaitForSingleObject(_regMonitorThread, 5000);
		if(ret == WAIT_TIMEOUT)
		{
			::TerminateThread(_regMonitorThread, 0);
			::WaitForSingleObject(_regMonitorThread, INFINITE);
		}

		::CloseHandle(_regMonitorEvent);
		::CloseHandle(_regMonitorThread);
	}

	if(_commTaskMgr.IsStarted())
		_commTaskMgr.Shutdown(5000);

	if(_pDB != NULL)
		_pDB->Close();

	delete this;
}

BOOL CSoftUninstall::DataRefresh(BOOL force)
{
	if(force)
	{
		return StartTask(new CSoftRefreshTask(this));
	}
	else
	{
		_pNotify->SoftDataEvent(UE_Refresh_Begin, NULL);

		SoftData2List softData2List;
		LoadSoftUninstData(_pDB, softData2List);

		CSoftDataEnum softData2Enum(softData2List);
		_pNotify->SoftDataEvent(UE_Refresh_End, &softData2Enum);
	}
	return TRUE;
}

ISoftDataSweep* CSoftUninstall::DataSweep(LPCWSTR pKey)
{
	CSoftPowerSweepTask *pSweep = new CSoftPowerSweepTask(this, pKey);
	{
		_commTaskMgr.AddTask(pSweep);
		_commTaskMgr.StartTask();
	}
	return pSweep;
}

ISoftDataSweep2* CSoftUninstall::DataSweep2(LPCWSTR pKey)
{
	CSoftPowerSweepTask2 *pSweep = new CSoftPowerSweepTask2(this, pKey);
	{
		_commTaskMgr.AddTask(pSweep);
		_commTaskMgr.StartTask();
	}
	return pSweep;
}

BOOL CSoftUninstall::LinkRefresh(SoftItemAttri type, BOOL force)
{
	if(!force)
	{
		if(type == SIA_Desktop || type == SIA_Start || type == SIA_Quick)
		{
			if((type & _firstRefresh) == 0)
			{
				// 第一次运行执行一次强制刷新以更新缓存
				_firstRefresh |= type;
				LinkRefresh(type, TRUE);
			}
			else
			{
				wostringstream sql;
				sql<<L"select count(*) from link_list where type="<<static_cast<int>(type)<<L';';

				if(0 >= _pDB->ExecuteScalar(sql.str().c_str()))
				{
					// 若缓存无效，则进行强制刷新
					force = TRUE;
				}
			}
		}
		else
		{	
			// 进程与托盘仅有强制刷新
			force = TRUE;
		}
	}

	if(force)
	{
		LONG taskType;
		switch(type)
		{
		case SIA_Desktop:
			taskType = TASK_TYPE_LINK_DESKTOP_REFRESH;
			break;
			
		case SIA_Start:
			taskType = TASK_TYPE_LINK_START_REFRESSH;
			break;

		case SIA_Quick:
			taskType = TASK_TYPE_LINK_QUICK_REFRESH;
			break;

		case SIA_Tray:
			taskType = TASK_TYPE_LINK_TRAY_REFRESH;
			break;

		case SIA_Process:
			taskType = TASK_TYPE_LINK_PROCESS_REFRESH;
			break;

		default :
			assert(false);
			break;
		}

		return StartTask(new CSoftLinkRefreshTask(this, taskType, type));
	}
	else
	{
		_pNotify->SoftLinkEvent(UE_Refresh_Begin, type, NULL);

		WStrList linkList;
		LoadLinkData(_pDB, type, linkList);

		CSoftLinkEnum softLinkEnum(linkList);
		_pNotify->SoftLinkEvent(UE_Refresh_End, type, &softLinkEnum);
		return TRUE;
	}
}

BOOL CSoftUninstall::RubbishRefresh()
{
	if(Is64BitWindows()) return FALSE;

	if(0 >= _pDB->ExecuteScalar(L"select count(*) from rubbish_list"))
	{
		return StartTask(new CSoftRubbishRefreshTask(this));
	}
	else
	{
		_pNotify->SoftRubbishEvent(UE_Refresh_Begin, NULL);

		// 从缓存中获取数据
		SoftRubbish2List softRubbish2List;

		CComPtr<ISQLiteComResultSet3> pRs;
		HRESULT hr = _pDB->ExecuteQuery(L"select path,type from rubbish", &pRs);
		if(SUCCEEDED(hr))
		{
			while(!pRs->IsEof())
			{
				softRubbish2List.push_back(SoftRubbish2());
				softRubbish2List.back()._data = pRs->GetAsString(L"path");
				softRubbish2List.back()._type = (SoftItemAttri)pRs->GetInt(L"type");

				pRs->NextRow();
			}
		}

		CSoftRubbishEnum softRubbishEnum(softRubbish2List);
		_pNotify->SoftRubbishEvent(UE_Refresh_End, &softRubbishEnum);

		return TRUE;
	}
}

ISoftRubbishSweep* CSoftUninstall::RubbishSweep()
{
	CSoftRubbishSweep* pSweep = new CSoftRubbishSweep(this);
	{
		_commTaskMgr.AddTask(pSweep);
		_commTaskMgr.StartTask();
	}
	return pSweep;
}

BOOL CSoftUninstall::InitCacheDB()
{
	BOOL ret = FALSE;

	do
	{
		HMODULE hMod = LoadLibraryW(L"ksafedb.dll");
		if(hMod == NULL) break;

		PFN_BKDbCreateObject pBKDbCreateObject = (PFN_BKDbCreateObject)GetProcAddress(hMod, "BKDbCreateObject");
		if(pBKDbCreateObject == NULL) break;

		HRESULT hr = pBKDbCreateObject(__uuidof(ISQLiteComDatabase3), (void**)&_pDB, BKENG_CURRENT_VERSION);
		if(!SUCCEEDED(hr)) break;

		hr = _pDB->Open((_ksafePath + SOFT_UNINST_DB).c_str());
		if(!SUCCEEDED(hr)) break;

		_pDB->SetBusyTimeout(3000);
		_pDB->ExecuteUpdate(L"pragma page_size=4096;");
		_pDB->ExecuteUpdate(L"pragma encoding = \"UTF-16\";");

		ret = TRUE;
	}
	while(FALSE);

	return ret;
}

void CSoftUninstall::Startup(BOOL cacheIsValid)
{
	_regMonitorEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	_regMonitorThread = (HANDLE)_beginthreadex(NULL, 0, RegMonitorThread, this, 0, NULL);

	if(cacheIsValid) DataRefresh(TRUE);

//#ifdef _DEBUG
//	LinkRefresh(SIA_Process, TRUE);
//#endif
//	LinkRefresh(SIA_Desktop, TRUE);
//	LinkRefresh(SIA_Start, TRUE);
//	LinkRefresh(SIA_Quick, TRUE);
//	LinkRefresh(SIA_Tray, TRUE);

	StartTask(new CSoftRubbishRefreshTask(this));
	StartTask(new CSoftCalcSpaceTask(this));
	StartTask(new CSoftLastUseTask(this));
}

unsigned CSoftUninstall::RegMonitorThread(LPVOID pVoid)
{
	CSoftUninstall *pSoftUninst = (CSoftUninstall*)pVoid;
	assert(pSoftUninst != NULL);

	return pSoftUninst->RegMonitor();
}

BOOL CSoftUninstall::StartTask(conew::IBaseTask *pTask)
{
	BOOL ret = TRUE;
	LONG type = pTask->GetType();

	_commTaskMgr.LockTaskQueue();

	// 检查该类型任务是否已存在
	CTaskMgr::TaskCIter it = _commTaskMgr.NaiveGetTaskQueue().begin();
	CTaskMgr::TaskCIter end = _commTaskMgr.NaiveGetTaskQueue().end();
	for(; it != end; ++it)
	{
		if(type == (*it)->GetType())
		{
			ret = FALSE;
			break;
		}
	}

	if(ret)
	{
		_commTaskMgr.NaiveAddTask(pTask);
		_commTaskMgr.StartTask();
	}

	_commTaskMgr.UnlockTaskQueue();

	pTask->Release();
	return ret;
}

BOOL CSoftUninstall::LoadPinYin()
{
	CDataFileLoader	loader;
	BkDatLibContent content;

	if(!loader.GetLibDatContent((_ksafePath + SOFT_PINYIN_DAT).c_str(), content))
		return FALSE;

	TiXmlDocument plugins;
	if(plugins.Parse((char*)content.pBuffer) == NULL)
		return FALSE;

	TiXmlHandle hRoot(plugins.FirstChildElement("fonts"));
	for(TiXmlElement *pElem = hRoot.FirstChildElement("font").Element(); pElem != NULL; pElem = pElem->NextSiblingElement())
	{
		LPCSTR pHan = pElem->Attribute("c");
		if(pHan == NULL) continue;

		PinYin pinyin;
		pinyin._whole = pElem->Attribute("p");
		pinyin._acronym = pElem->Attribute("s");

		_hashPinYin.insert(make_pair(pHan, pinyin));
	}

	return TRUE;
}

void CSoftUninstall::GetPinYin(const std::wstring &name, std::wstring &whole, std::wstring &acronym)
{
	typedef list<string>					str_list;
	typedef str_list::iterator				str_iter;
	typedef string_tokeniser<string, char>	string_tokeniser_a;

	typedef void (string::*PFPushback)(char);
	typedef string& (string::*PFAppend)(const char*);

	PFAppend pfAppend = &string::append;
	PFPushback pfPushback = &string::push_back;

	char mbcs[5];
	str_list wholeList, acronymList;
	wholeList.push_back(string());
	acronymList.push_back(string());

	for(wstring::size_type i = 0; i < name.size(); ++i)
	{
		if(iswascii(name[i]) == 0)
		{	// 非ASCII码
			int ret;
			if(wctomb_s(&ret, mbcs, sizeof(mbcs), name[i]) != 0) continue;
			mbcs[ret] = '\0';

			PinYinCIter itPinYin = _hashPinYin.find(mbcs);
			if(itPinYin != _hashPinYin.end())
			{
				const PinYin &pinyin = itPinYin->second;

				// M X N组合
				{
					string_tokeniser_a tokeniser(pinyin._whole, ' ');

					str_list resultList;
					string_tokeniser_a::const_iterator end = tokeniser.end();
					for(string_tokeniser_a::const_iterator it = tokeniser.begin(); it != end; ++it)
					{
						str_iter endList = wholeList.end();
						for(str_iter itList = wholeList.begin(); itList != endList; ++itList)
							resultList.push_back(*itList + *it);
					}
					wholeList.swap(resultList);
				}
				{
					string_tokeniser_a tokeniser(pinyin._acronym, ' ');

					str_list resultList;
					string_tokeniser_a::const_iterator end = tokeniser.end();
					for(string_tokeniser_a::const_iterator it = tokeniser.begin(); it != end; ++it)
					{
						str_iter endList = acronymList.end();
						for(str_iter itList = acronymList.begin(); itList != endList; ++itList)
							resultList.push_back(*itList + *it);
					}
					acronymList.swap(resultList);
				}
			}
			else
			{
				for_each(wholeList.begin(), wholeList.end(), bind2nd(mem_fun1_ref(pfAppend), mbcs));
				for_each(acronymList.begin(), acronymList.end(), bind2nd(mem_fun1_ref(pfAppend), mbcs));
			}
		}
		else
		{
			// ASCII码直接组合
			for_each(wholeList.begin(), wholeList.end(), bind2nd(mem_fun1_ref(pfPushback), (char)towlower(name[i])));
			for_each(acronymList.begin(), acronymList.end(), bind2nd(mem_fun1_ref(pfPushback), (char)tolower(name[i])));
		}
	}

	// 合并结果
	{
		ostringstream os;
		copy(wholeList.begin(), wholeList.end(), ostream_iterator<string>(os, " "));

		whole = CA2W(os.str().c_str());
		trim_right(whole);
	}

	{
		ostringstream os;
		copy(acronymList.begin(), acronymList.end(), ostream_iterator<string>(os, " "));

		acronym = CA2W(os.str().c_str());
		trim_right(acronym);
	}
}

unsigned CSoftUninstall::RegMonitor()
{
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_NOTIFY);
	if(ret != ERROR_SUCCESS) return unsigned(-1);

	CRegKey reg2;
	ret = reg2.Open(HKEY_CURRENT_USER,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_NOTIFY);
	if(ret != ERROR_SUCCESS) return unsigned(-1);

	CRegKey reg3;
	if(Is64BitWindows())
	{
		ret = reg3.Open(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_NOTIFY | KEY_WOW64_64KEY);
		if(ret != ERROR_SUCCESS) return unsigned(-1);
	}

	HANDLE hEvents[2] = {0};
	hEvents[0] = _regMonitorEvent;
	hEvents[1] = ::CreateEventW(NULL, FALSE, FALSE, NULL);

	do
	{
		ret = reg.NotifyChangeKeyValue(TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, hEvents[1]);
		if(ret != ERROR_SUCCESS) break;

		ret = reg2.NotifyChangeKeyValue(TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, hEvents[1]);
		if(ret != ERROR_SUCCESS) break;

		if(Is64BitWindows())
		{
			ret = reg3.NotifyChangeKeyValue(TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, hEvents[1]);
			if(ret != ERROR_SUCCESS) break;
		}

		DWORD res = ::WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if(res == WAIT_OBJECT_0 + 1)
		{
			// 等注册表稳定
			::Sleep(500);

			// 强制刷新
			DataRefresh(TRUE);
		}
		else
		{
			// 退出
			break;
		}
	}
	while(TRUE);

	::CloseHandle(hEvents[1]);
	return 0;
}

}
#pragma once

#include "SoftUninstallTask.h"
#include <TlHelp32.h>
#include <atlcoll.h>
#include <stlsoft/string/case_functions.hpp>
#include <winstl/shims/conversion/to_uint64.hpp>
#include "SoftUninstallDef.h"
#include "SoftUninstallApi.h"

namespace ksm
{

class CProcessSnap
{
public:
	CProcessSnap(){}
	virtual ~CProcessSnap(){}

	VOID Snap()
	{
		PROCESSENTRY32 pe32 = {0};

		pe32.dwSize = sizeof( PROCESSENTRY32 );

		HANDLE snap	= ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

		if ( snap != INVALID_HANDLE_VALUE )
		{
			if ( ::Process32First( snap, &pe32 ) )
			{
				do
				{
					m_procList.Add(pe32);

					memset(&pe32, 0, sizeof(PROCESSENTRY32) );

					pe32.dwSize = sizeof( PROCESSENTRY32 );

				} while( Process32Next( snap, &pe32 ) );
			}

			::CloseHandle(snap);
		}
	}

	BOOL StrStrProcessExist(LPCWSTR lpwstrName)
	{
		CString		strFind = lpwstrName;

		strFind.MakeLower();
		for ( int i=0; i < m_procList.GetSize(); i++)
		{
			CString		strName = m_procList[i].szExeFile;

			strName.MakeLower();

			if ( strName.Find( strFind ) != -1 )
				return TRUE;
		}
		return FALSE;
	}

	int GetSize()
	{
		return m_procList.GetSize();
	}

	PROCESSENTRY32& operator[](int nIndex)
	{
		ATLASSERT( nIndex < m_procList.GetSize() );

		return m_procList[nIndex];
	}

protected:
	CSimpleArray<PROCESSENTRY32>	m_procList;
};

//
// 卸载任务
//
class CSoftPowerSweepTask : public CSoftTask
	, public ISoftDataSweep2
{
public:
	CSoftPowerSweepTask(CSoftUninstall *pSoftUninst, LPCWSTR pKey)
		: CSoftTask(pSoftUninst, TASK_TYPE_DATA_UNINST)
		, _key(pKey) 
		, _rubbishEnum(_rubbishList)
		, _notifyEnum(_notifyList)
	{ 
		_x64		= Is64BitKey(_key);
		_rubbishSize= 0;
		_brecycle	= FALSE;
		_bCancel	= FALSE;
		_eventWait	= ::CreateEventW(NULL, FALSE, FALSE, NULL);
		SetPriority(TASK_PRI_DATA_UNINST); 
	}

	virtual BOOL TaskProcess(conew::CTaskMgr *pMgr);
	virtual void __stdcall Uninitialize() { IBaseTask::Release(); }
	virtual LPCWSTR __stdcall GetKey(){ return _key.c_str(); }

	virtual ISoftRubbishEnum* __stdcall DataUninstEnum(){return &_rubbishEnum;}
	virtual ISoftRubbishEnum* __stdcall DataSweepingEnum(){return &_notifyEnum;}
	virtual BOOL __stdcall DataSetSweep(PCSoftRubbish pcData);
	virtual BOOL __stdcall DataSweep(BOOL recycle);
	virtual BOOL __stdcall DataSoftExists();
	virtual BOOL __stdcall DataCancelUninst(LPCWSTR pKey);
	virtual LPCWSTR __stdcall GetMainDir();

	virtual ULONGLONG __stdcall GetRubbishSize() { return _rubbishSize; }

protected:
	PSoftData2	GetSoftDataByKey(LPCWSTR pKey);

	// 标准卸载后的强力卸载
	BOOL		_IsIncludedPath(LPCWSTR lpPath);
	BOOL		_IsIncludedPath2(LPCWSTR lpPath);
	BOOL		_PowerCleanScanFile();
	VOID		_PowerCleanScanReg();
	// 是否多个软件安装同一目录
	BOOL		_isMultiDirInstallOneDir();

	VOID		_beginRubbishSweep();
	VOID		_deleteSoftFromDB();

	BOOL		_InvokeUninstProgram(LPWSTR pCmd);
	BOOL		_IsBrowserProgram(LPCWSTR pName);
	void		FixMainPath();
	void		ParseLinkList(const WStrList &linkList, SoftRubbish2List &rubbish2List);

protected:
	virtual BOOL _PowerCleanScanMain(SoftRubbish2List &rubbish2List);
	BOOL		_PowerCleanScanOther(SoftRubbish2List &rubbish2List);

protected:
	// 计算文件数量
	class FileCounter
	{
	public:
		FileCounter() { _count = 0; }
		bool operator()(const std::wstring &, const WIN32_FIND_DATAW &)
		{ ++_count; return true; }

		int _count;
	};

	// 枚举主目录
	class MainPathEnum
	{
	public:
		MainPathEnum(SoftRubbish2List &rubbish2List)
			: _rubbish2List(rubbish2List), _size(0) {}

		ULONGLONG GetSize() const 
		{ return _size; }

		bool operator()(const std::wstring &root, const WIN32_FIND_DATAW &wfd)
		{
			std::wstring path = root + L'\\' + wfd.cFileName;
			stlsoft::make_lower(path);

			if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				_rubbish2List.push_back(SoftRubbish2(SIA_Directory, path));
			}
			else
			{
				_size += stlsoft::to_uint64(wfd);
				_rubbish2List.push_back(SoftRubbish2(SIA_File, path));
			}

			return true;
		}

	private:
		ULONGLONG _size;
		SoftRubbish2List &_rubbish2List;
	};

protected:
	BOOL				_x64;
	std::wstring		_key;
	SoftData2			_curSoftData;
	BOOL				_bCancel;
	BOOL				_brecycle;

	LONGLONG			_rubbishSize;
	CSensitivePaths		_sensPaths;
	SoftRubbish2List	_regList;
	SoftRubbish2List	_rubbishList;
	CSoftRubbishEnum	_rubbishEnum;
	SoftRubbish2List	_notifyList;
	CSoftRubbishEnum	_notifyEnum;
	SoftRubbish2List	_sweepList;
	HANDLE				_eventWait;
};

class CSoftPowerSweepTask2 : public CSoftPowerSweepTask
{
public:
	CSoftPowerSweepTask2(CSoftUninstall *pSoftUninst, LPCWSTR pKey)
		: CSoftPowerSweepTask(pSoftUninst, pKey) {}

protected:
	virtual BOOL _PowerCleanScanMain(SoftRubbish2List &rubbish2List);
};
//////////////////////////////////////////////////////////////////////////
inline BOOL CSoftPowerSweepTask::_IsIncludedPath(LPCWSTR lpPath)
{
	std::wstring absPath = MakeAbsolutePath(lpPath);
	return (_wcsnicmp(_curSoftData._mainPath.c_str(), absPath.c_str(), _curSoftData._mainPath.size()) == 0);
}

inline BOOL CSoftPowerSweepTask::DataSetSweep(PCSoftRubbish pcData)
{
	_sweepList.push_back(SoftRubbish2(pcData->_type, pcData->_pData));
	return TRUE;
}

inline BOOL CSoftPowerSweepTask::DataCancelUninst(LPCWSTR pKey)
{ 
	_bCancel = TRUE;
	::SetEvent(_eventWait);
	return TRUE;
}

inline BOOL CSoftPowerSweepTask::DataSweep( BOOL recycle )
{
	_brecycle = recycle;
	::SetEvent(_eventWait);
	return	TRUE;
}

}
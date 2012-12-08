#pragma once

#include <string>
#include <skylark2/bkdb.h>
#include <softmgr/ISoftUninstall.h>
#include "ConewTaskMgr.h"
#include "SoftUninstallDef.h"

namespace ksm
{

//
// 软件卸载接口实现
//
class conew::CTaskMgr;
class CSoftUninstall : public ISoftUninstall2
{
public:
	CSoftUninstall();
	~CSoftUninstall();

public:
	virtual void __stdcall SetNotify(ISoftUnincallNotify *pNotify);
	virtual BOOL __stdcall Initialize(LPCWSTR pKSafePath);
	virtual void __stdcall Uninitialize();

	virtual BOOL __stdcall DataRefresh(BOOL force);
	virtual ISoftDataSweep* __stdcall DataSweep(LPCWSTR pKey);

	virtual BOOL __stdcall LinkRefresh(SoftItemAttri type, BOOL force);

	virtual BOOL __stdcall RubbishRefresh();
	virtual ISoftRubbishSweep* __stdcall RubbishSweep();
	virtual ISoftDataSweep2* __stdcall DataSweep2(LPCWSTR pKey);

public:
	BOOL							IsExited() const;
	const std::wstring&				GetKSafePath() const;
	Skylark::ISQLiteComDatabase3* 	GetDBPtr() const;
	ISoftUnincallNotify*			GetNotify() const;
	void							Startup(BOOL cacheIsValid);
	BOOL							LoadPinYin();
	void							GetPinYin(const std::wstring &name, std::wstring &whole, std::wstring &acronym);

private:
	BOOL InitCacheDB();
	BOOL StartTask(conew::IBaseTask *pTask);

	unsigned RegMonitor();
	static unsigned __stdcall RegMonitorThread(LPVOID pVoid);

private:
	BOOL _exit;
	ULONG _firstRefresh;

	std::wstring _ksafePath;
	ISoftUnincallNotify *_pNotify;

	PinYinHash _hashPinYin;
	conew::CTaskMgr	_commTaskMgr;

	HANDLE _regMonitorEvent;
	HANDLE _regMonitorThread;

	CComPtr<Skylark::ISQLiteComDatabase3> _pDB;
};
//////////////////////////////////////////////////////////////////////////
// inline implement
inline BOOL CSoftUninstall::IsExited() const 
{ return _exit; }
inline Skylark::ISQLiteComDatabase3* CSoftUninstall::GetDBPtr() const
{ return (Skylark::ISQLiteComDatabase3*)_pDB; }
inline const std::wstring& CSoftUninstall::GetKSafePath() const 
{ return _ksafePath; }
inline ISoftUnincallNotify* CSoftUninstall::GetNotify() const 
{ return _pNotify; }

}
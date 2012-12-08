#pragma once

#include <softmgr/ISoftDesktop.h>
#include <skylark2/bkdb.h>
#include <string>
#include <list>
#include "ConewTaskMgr.h"
#include <winhttp/WinHttpApi.h>

namespace ksm
{

//
// 导出SetOption函数
//
class WinHttpApi2 : public WinHttpApi
{
public:
	BOOL SetOption(HINTERNET h, DWORD o, LPVOID p, DWORD l)
	{
		BOOL bSuccess = FALSE;

		if (bUseWinHttp_)
			bSuccess = (WinHttpApi_.pWinHttpSetOption)(h, o, p, l);
		else
			bSuccess = (WinINetApi_.pInternetSetOption)(h, o, p, l);

		return bSuccess;
	}

	HINTERNET OpenSession2(LPCWSTR userAgent, LPCWSTR pProxyServer)
	{
		HINTERNET hSession = NULL;

		if (bUseWinHttp_)
			hSession = (WinHttpApi_.pWinHttpOpen)(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, pProxyServer, WINHTTP_NO_PROXY_BYPASS, NULL/*WINHTTP_FLAG_ASYNC*/);
		else
			hSession = (WinINetApi_.pInternetOpen)(userAgent, INTERNET_OPEN_TYPE_PROXY, pProxyServer, NULL, 0);

		return hSession;
	}

	BOOL IsWinHttp() const
	{return bUseWinHttp_;}
};

//
// 查询信息
//
typedef struct SoftQueryInfo
{
	SoftInfoMask _mask;

	LONG _id;

	SoftQueryInfo(LONG id, SoftInfoMask mask)
		: _id(id), _mask(mask) {}
} *PSoftQueryInfo;
typedef const SoftQueryInfo* PCSoftQueryInfo;

typedef std::list<SoftQueryInfo>		SoftQueryInfoList;
typedef SoftQueryInfoList::iterator		SoftQueryInfoIter;
typedef SoftQueryInfoList::const_iterator
										SoftQueryInfoCIter;
//
// 软件信息
//
typedef struct SoftInfo2
{
	SoftInfoMask _mask;

	LONG _id;
	std::wstring _grade;

	SoftInfo2() : _id(0), _mask(SIM_None) {}
	SoftInfo2(LONG id, const std::wstring &grade) 
		:_id(id), _grade(grade), _mask(SIM_Grade) {}
} *PSoftInfo2;
typedef const SoftInfo2* PCSoftInfo2;

typedef std::list<SoftInfo2>			SoftInfo2List;
typedef SoftInfo2List::iterator			SoftInfo2Iter;
typedef SoftInfo2List::const_iterator	SoftInfo2CIter;


//
// 软件信息查询
//
class CSoftInfoQueryTask;
class CSoftInfoQuery : public ISoftInfoQuery
{
public:
	CSoftInfoQuery();
	~CSoftInfoQuery();

public:
	virtual void __stdcall SetNotify(ISoftInfoQueryNotify *pNotify);
	virtual BOOL __stdcall Initialize(LPCWSTR pKSafePath);
	virtual void __stdcall Uninitialize();

	virtual void __stdcall ClsSoft();
	virtual void __stdcall AddSoft(LONG id, SoftInfoMask mask);
	virtual BOOL __stdcall Query(void *pParams);

public:
	Skylark::ISQLiteComDatabase3* 	GetDBPtr() const;
	ISoftInfoQueryNotify*			GetNotify() const;
	WinHttpApi2*					GetWinHttpApi();


private:
	BOOL InitCacheDB();
	void CancelCurTask();
	BOOL CommitTask(SoftQueryInfoList &queryList, void *pParams);

private:
	std::wstring _ksafePath;
	ISoftInfoQueryNotify *_pNotify;

	SoftQueryInfoList _softQueryInfoList;

	conew::CTaskMgr _taskMgr;
	CComPtr<Skylark::ISQLiteComDatabase3> _pDB;

	BOOL _initHttp;
	WinHttpApi2 _httpApi;
};
//////////////////////////////////////////////////////////////////////////
// inline implemenation
inline Skylark::ISQLiteComDatabase3* CSoftInfoQuery::GetDBPtr() const 
{ return (Skylark::ISQLiteComDatabase3*)_pDB; }
inline ISoftInfoQueryNotify* CSoftInfoQuery::GetNotify() const { return _pNotify; }
inline WinHttpApi2* CSoftInfoQuery::GetWinHttpApi() { return _initHttp ? &_httpApi : NULL; }

}
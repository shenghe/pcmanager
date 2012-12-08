#pragma once

#include <string>
#include <skylark2/bkdb.h>
#include "SoftUninstallDef.h"
#include "SoftUninstallApi.h"

namespace ksm
{

// 绑定助手函数
inline void BindHelper(Skylark::ISQLiteComStatement3 *pState, int idx, LPCWSTR pStr);
inline void BindHelper(Skylark::ISQLiteComStatement3 *pState, int idx, const std::wstring &str);

// 初始化Cache数据库
void InitCacheDB(Skylark::ISQLiteComDatabase3 *pDB);

//
// cache事务
//
class CCacheTransaction
{
public:
	CCacheTransaction(Skylark::ISQLiteComDatabase3 *pDB) 
		: _pDB(pDB) { _pDB->BeginTransaction(); }

	~CCacheTransaction() { _pDB->CommitTransaction(); }

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
};

//
// cache_flag_list操作
//
class CCacheFlagOpr
{
public:
	CCacheFlagOpr(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Query(const std::wstring &path, ULONGLONG &time);
	BOOL Insert(const std::wstring &path, ULONGLONG time);

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateQuery;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateInsert;
};

//
// local_soft_list软件查找
//
class CCacheSoftFind
{
public:
	CCacheSoftFind(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Query(const std::wstring &path, std::wstring &key);

private:
	BOOL InternalQuery(const std::wstring &path, std::wstring &key);

private:
	CSensitivePaths _sensiPaths;
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateQuery;
};

//
// 残留项操作
//
class CCacheRubbish
{
public:
	CCacheRubbish(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Delete();
	BOOL Delete(SoftItemAttri type);
	BOOL Delete(const std::wstring &key);
	BOOL Delete(const SoftRubbish2 &softRubbish2);
	BOOL Insert(const SoftRubbish2 &softRubbish2);

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateInsert;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateDelete;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateDelete2;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateDelete3;
};

//
// Link操作
//
class CCacheLink
{
public:
	CCacheLink(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Delete(SoftItemAttri type);

	BOOL Insert(SoftItemAttri type, const std::wstring &key);

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateInsert;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateDelete;
};

//
// soft_data_list
//
class CCacheSoftData
{
public:
	CCacheSoftData(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Insert(int softId, int typeId, int matchType, LPCWSTR pLogoUrl, LPCWSTR pBrief, LPCWSTR pInfoUrl, LPCWSTR pPattern);

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateInsert;
};

//
// calc space
//
class CCacheCalcSpace
{
public:
	CCacheCalcSpace(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Update(const std::wstring &key, ULONGLONG size);

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateUpdate;
};

//
// last use
//
class CCacheLastUse
{
public:
	CCacheLastUse(Skylark::ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Update(const std::wstring &key, LONG time);
	BOOL UpdateCount(const std::wstring &key, int nCnt);

private:
	Skylark::ISQLiteComDatabase3 *_pDB;
	CComPtr<Skylark::ISQLiteComStatement3> _pStateUpdate;
};
//////////////////////////////////////////////////////////////////////////
inline void BindHelper(Skylark::ISQLiteComStatement3 *pState, int idx, LPCWSTR pStr)
{
	if(pStr == NULL || pStr[0] == 0)
		pState->BindNull(idx);
	else
		pState->Bind(idx, pStr);
}

inline void BindHelper(Skylark::ISQLiteComStatement3 *pState, int idx, const std::wstring &str)
{
	if(str.empty())
		pState->BindNull(idx);
	else
		pState->Bind(idx, str.c_str());
}

inline BOOL CCacheFlagOpr::Query(const std::wstring &path, ULONGLONG &time)
{
	if(_pStateQuery == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"select last_write_time from cache_flag_list where path=?;", &_pStateQuery);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateQuery->Reset();
	}

	_pStateQuery->Bind(1, path.c_str());

	CComPtr<Skylark::ISQLiteComResultSet3> pRs;
	HRESULT hr = _pStateQuery->ExecuteQuery(&pRs);
	if(!SUCCEEDED(hr) || pRs->IsEof()) return FALSE;

	time = pRs->GetInt64(L"last_write_time");
	return TRUE;
}

inline BOOL CCacheFlagOpr::Insert(const std::wstring &path, ULONGLONG time)
{
	if(_pStateInsert == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"insert or replace into cache_flag_list(path, last_write_time) values(?,?);", &_pStateInsert);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateInsert->Reset();
	}

	_pStateInsert->Bind(1, path.c_str());
	_pStateInsert->Bind(2, time);

	return SUCCEEDED(_pStateInsert->ExecuteUpdate());
}

inline BOOL CCacheSoftFind::InternalQuery(const std::wstring &path, std::wstring &key)
{
	if(_pStateQuery == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"select soft_key from local_soft_list where main_path=?;", &_pStateQuery);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateQuery->Reset();
	}

	_pStateQuery->Bind(1, path.c_str());

	CComPtr<Skylark::ISQLiteComResultSet3> pRs;
	HRESULT hr = _pStateQuery->ExecuteQuery(&pRs);
	if(!SUCCEEDED(hr) || pRs->IsEof()) return FALSE;

	key = pRs->GetAsString(L"soft_key");
	return TRUE;
}

inline BOOL CCacheRubbish::Delete()
{
	return (SUCCEEDED(_pDB->ExecuteUpdate(L"delete from rubbish_list")));
}

inline BOOL CCacheRubbish::Delete(SoftItemAttri type)
{
	if(_pStateDelete == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"delete from rubbish_list where type=?", &_pStateDelete);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateDelete->Reset();
	}

	_pStateDelete->Bind(1, static_cast<int>(type));
	return (SUCCEEDED(_pStateDelete->ExecuteUpdate()));
}

inline BOOL CCacheRubbish::Delete(const std::wstring &key)
{
	if(_pStateDelete3 == NULL)
	{
		HREFTYPE hr = _pDB->PrepareStatement(L"delete from rubbish_list where soft_key=?", &_pStateDelete2);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateDelete3->Reset();
	}

	_pStateDelete3->Bind(1, key.c_str());
	return (SUCCEEDED(_pStateDelete3->ExecuteUpdate()));
}

inline BOOL CCacheRubbish::Delete(const SoftRubbish2 &softRubbish2)
{
	if(_pStateDelete2 == NULL)
	{
#if 1
		HRESULT hr = _pDB->PrepareStatement(L"delete from rubbish_list where path = ?", &_pStateDelete2);
#else
		HRESULT hr = _pDB->PrepareStatement(L"delete from rubbish_list where path = ? and type=?", &_pStateDelete2);
#endif
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateDelete2->Reset();
	}

#if 1
	_pStateDelete2->Bind(1, static_cast<int>(softRubbish2._type));
#else
	_pStateDelete2->Bind(1, softRubbish2._data.c_str());
	_pStateDelete2->Bind(2, static_cast<int>(softRubbish2._type));
#endif

	return (SUCCEEDED(_pStateDelete2->ExecuteUpdate()));
}

inline BOOL CCacheRubbish::Insert(const SoftRubbish2 &softRubbish2)
{
	if(_pStateInsert == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"insert or replace into rubbish_list(path,type)values(?,?)", &_pStateInsert);
		if(!SUCCEEDED(hr)) return FALSE;
	}

	_pStateInsert->Bind(1, softRubbish2._data.c_str());
	_pStateInsert->Bind(2, static_cast<int>(softRubbish2._type));
	return (SUCCEEDED(_pStateInsert->ExecuteUpdate()));
}

inline BOOL CCacheLink::Delete(SoftItemAttri type)
{
	if(_pStateDelete == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"delete from link_list where type=?", &_pStateDelete);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateDelete->Reset();
	}

	_pStateDelete->Bind(1, static_cast<int>(type));
	return (SUCCEEDED(_pStateDelete->ExecuteUpdate()));
}

inline BOOL CCacheLink::Insert(SoftItemAttri type, const std::wstring &key)
{
	if(_pStateInsert == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"insert into link_list(soft_key,type)values(?,?)", &_pStateInsert);
		if(!SUCCEEDED(hr)) return FALSE;
	}

	_pStateInsert->Bind(1, key.c_str());
	_pStateInsert->Bind(2, static_cast<int>(type));
	return (SUCCEEDED(_pStateInsert->ExecuteUpdate()));
}

inline BOOL CCacheSoftData::Insert(int softId, int typeId, int matchType, LPCWSTR pLogoUrl, LPCWSTR pBrief, LPCWSTR pInfoUrl, LPCWSTR pPattern)
{
	if(_pStateInsert == NULL)
	{
		static const LPCWSTR pSqlInsert = 
			L"insert into soft_info_list"
			L"(soft_id, type_id, match_type, logo_url, brief, info_url, pattern)"
			L"values(?, ?, ?, ?, ?, ?, ?);";

		HRESULT hr = _pDB->PrepareStatement(pSqlInsert, &_pStateInsert);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateInsert->Reset();
	}

	_pStateInsert->Bind(1, softId);
	_pStateInsert->Bind(2, typeId);
	_pStateInsert->Bind(3, matchType);	
	BindHelper(_pStateInsert, 4, pLogoUrl);
	BindHelper(_pStateInsert, 5, pBrief);
	BindHelper(_pStateInsert, 6,pInfoUrl);
	BindHelper(_pStateInsert, 7, pPattern);

	return (SUCCEEDED(_pStateInsert->ExecuteUpdate()));
}

inline BOOL CCacheCalcSpace::Update(const std::wstring &key, ULONGLONG size)
{
	if(_pStateUpdate == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"update local_soft_list set size=? where soft_key=?", &_pStateUpdate);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateUpdate->Reset();
	}

	_pStateUpdate->Bind(1, size);
	_pStateUpdate->Bind(2, key.c_str());

	return (SUCCEEDED(_pStateUpdate->ExecuteUpdate()));
}

inline BOOL CCacheLastUse::Update(const std::wstring &key, LONG time)
{
	if(_pStateUpdate == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"update local_soft_list set last_use=? where soft_key=?", &_pStateUpdate);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateUpdate->Reset();
	}

	_pStateUpdate->Bind(1, static_cast<int>(time));
	_pStateUpdate->Bind(2, key.c_str());

	return (SUCCEEDED(_pStateUpdate->ExecuteUpdate()));
}

inline BOOL CCacheLastUse::UpdateCount(const std::wstring &key, int nCnt)
{
	if(_pStateUpdate == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"update local_soft_list set daycnt=? where soft_key=?", &_pStateUpdate);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pStateUpdate->Reset();
	}

	_pStateUpdate->Bind(1, nCnt);
	_pStateUpdate->Bind(2, key.c_str());

	return (SUCCEEDED(_pStateUpdate->ExecuteUpdate()));
}

}
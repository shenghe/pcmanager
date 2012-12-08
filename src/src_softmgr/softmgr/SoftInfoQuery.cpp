#include "stdafx.h"
#include "SoftInfoQuery.h"
#include <stlsoft/memory/auto_buffer.hpp>
#include <stlsoft/string/trim_functions.hpp>
#include <stlsoft/string/case_functions.hpp>
using namespace stlsoft;
#include <time.h>
#include <locale>
#include <clocale>
#include <sstream>
#include <algorithm>
using namespace std;
using namespace Skylark;
using namespace conew;
#include <WinInet.h>
#pragma comment(lib, "Wininet.lib")
#include <tinyxml/tinyxml.h>
#include <kws/crc32/crc32.h>
#include <kws/base64/base64_safe.h>
#include <common/utility.h>

namespace ksm
{

//
// 初始化语言任务
//
//class CSoftInitLanguage : public IBaseTask
//{
//public:
//	CSoftInitLanguage() : IBaseTask(0) { SetPriority(TP_Highest); }
//
//	virtual BOOL TaskProcess(CTaskMgr *pMgr)
//	{
//		::SetThreadLocale(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED));
//
//		setlocale(LC_ALL, "chs");
//		try{ locale::global(locale("chs")); } catch(...) {}
//
//		return FALSE;
//	}
//};

//
// 软件信息查询任务
//
class CSoftInfoQueryTask : public IBaseTask
{
public:
	CSoftInfoQueryTask(CSoftInfoQuery *pSoftQuery, SoftQueryInfoList &softQueryInfoList, void *pParams) 
		: IBaseTask(1), _pSoftQuery(pSoftQuery), _pParams(pParams)
	{ _softQueryInfoList.swap(softQueryInfoList); }

	virtual BOOL TaskProcess(CTaskMgr *pMgr);

private:
	BOOL CombineBodyContent(const SoftQueryInfoList &queryInfoList, string &bodyContent);
	BOOL QueryByNetwork(const SoftQueryInfoList &queryInfoList, SoftInfo2List &resultList);

private:
	void *_pParams;
	SoftQueryInfoList _softQueryInfoList;
	CSoftInfoQuery *_pSoftQuery;
};

//
// 软件信息枚举类
//
class CSoftInfoEnum : public ISoftInfoEnum
{
public:
	CSoftInfoEnum(const SoftInfo2List &softInfo2List)
		: _softInfo2List(softInfo2List) {}

	virtual BOOL __stdcall EnumFirst(PSoftInfo pSoftInfo)
	{
		_it = _softInfo2List.begin();
		if(_it != _softInfo2List.end())
		{
			CopyData(pSoftInfo);
			return TRUE;
		}
		return FALSE;
	}

	virtual BOOL __stdcall EnumNext(PSoftInfo pSoftInfo)
	{
		if(++_it != _softInfo2List.end())
		{
			CopyData(pSoftInfo);
			return TRUE;
		}
		return FALSE;
	}

private:
	void CopyData(PSoftInfo pSoftInfo)
	{
		pSoftInfo->_mask	= _it->_mask;
		pSoftInfo->_id		= _it->_id;
		pSoftInfo->_pcGrade	= _it->_grade.c_str();
	}

private:
	SoftInfo2CIter _it;
	const SoftInfo2List &_softInfo2List;
};

//
// 软件信息操作
//
class CCacheSoftInfoOpr
{
public:
	CCacheSoftInfoOpr(ISQLiteComDatabase3 *pDB) : _pDB(pDB) {}

	BOOL Query(LONG id, wstring &grade);
	BOOL Insert(LONG id, const wstring &grade, __time32_t time);

private:
	ISQLiteComDatabase3 *_pDB;
	CComPtr<ISQLiteComStatement3> _pQuery;
	CComPtr<ISQLiteComStatement3> _pInsert;
};

//
// 路径定义
//
static const LPCWSTR SOFT_INFO_DB			= L"\\appdata\\ksoft_infocache";

//
// 软件信息更新时间间隔（单位：秒）
//
static const __time32_t SOFT_INFO_INTERVAL	= 1*60*60;

//
// 单次最大查询数量
//
static const size_t MAX_PER_QUERY			= 100;

//
// 查询地址
//
// 测试地址：http://10.20.223.113/soft.do?v=1&sid=7440&c=319326C3
static const int QUERY_TIMEOUT = 30*1000;
static const LPCWSTR QUERY_SERVER_NAME = L"baike.ijinshan.com";
static const LPCWSTR QUERY_OBJECT_NAME = L"/soft.do";
//////////////////////////////////////////////////////////////////////////
CSoftInfoQuery::CSoftInfoQuery()
{
	_pNotify = NULL;
	_initHttp = _httpApi.Initialize();
}

CSoftInfoQuery::~CSoftInfoQuery()
{
	_httpApi.Uninitialize();
}

void CSoftInfoQuery::SetNotify(ISoftInfoQueryNotify *pNotify)
{
	_pNotify = pNotify;
}

BOOL CSoftInfoQuery::Initialize(LPCWSTR pKSafePath)
{
	assert(pKSafePath != NULL && _pNotify != NULL);

	_ksafePath = pKSafePath; 
	trim_right(_ksafePath, L"\\"); 
	make_lower(_ksafePath);

	return InitCacheDB();
}

void CSoftInfoQuery::Uninitialize()
{
	if(_taskMgr.IsStarted())
	{
		_taskMgr.Shutdown(5000);
	}

	if(_pDB != NULL)
		_pDB->Close();

	delete this;
}

void CSoftInfoQuery::AddSoft(LONG id, SoftInfoMask mask) 
{ 
	_softQueryInfoList.push_back(SoftQueryInfo(id, mask)); 
}

void CSoftInfoQuery::ClsSoft() 
{
	_softQueryInfoList.clear();
}

BOOL CSoftInfoQuery::InitCacheDB()
{
	if(!_initHttp) return FALSE;

	BOOL ret = FALSE;

	do
	{
		HMODULE hMod = LoadLibraryW(L"ksafedb.dll");
		if(hMod == NULL) break;

		PFN_BKDbCreateObject pBKDbCreateObject = (PFN_BKDbCreateObject)GetProcAddress(hMod, "BKDbCreateObject");
		if(pBKDbCreateObject == NULL) break;

		HRESULT hr = pBKDbCreateObject(__uuidof(ISQLiteComDatabase3), (void**)&_pDB, BKENG_CURRENT_VERSION);
		if(!SUCCEEDED(hr)) break;

		hr = _pDB->Open((_ksafePath + SOFT_INFO_DB).c_str());
		if(!SUCCEEDED(hr)) break;

		_pDB->SetBusyTimeout(3000);
		_pDB->ExecuteUpdate(L"pragma page_size=4096;");
		_pDB->ExecuteUpdate(L"pragma encoding = \"UTF-16\";");

		if(_pDB->TableExists(L"soft_info_list"))
		{
			wostringstream os;
			os<<"delete from soft_info_list where update_time < "<<_time32(NULL) - SOFT_INFO_INTERVAL<<L';';

			_pDB->ExecuteUpdate(os.str().c_str());
			ret = TRUE;
		}
		else
		{
			static const LPCWSTR pSql = 
				L"create table soft_info_list("
				L"soft_id int primary key,"
				L"grade nvarchar(32),"
				L"update_time int);";

			ret = SUCCEEDED(_pDB->ExecuteUpdate(pSql));
		}		
	}
	while(FALSE);

	return ret;
}

BOOL CSoftInfoQuery::CommitTask(SoftQueryInfoList &queryList, void *pParams)
{
	CSoftInfoQueryTask *pTask = new CSoftInfoQueryTask(this, queryList, pParams);
	if(pTask != NULL)
	{
		_taskMgr.AddTask(pTask);
		_taskMgr.StartTask();

		pTask->Release();
	}
	return (pTask != NULL);
}

BOOL CSoftInfoQuery::Query(void *pParams)
{
	if(!_initHttp) return FALSE;

	if(_softQueryInfoList.empty()) return FALSE;

	if(!_taskMgr.IsStarted())
	{
		_taskMgr.Startup();
	}

	// 查询分组
	while(_softQueryInfoList.size() > MAX_PER_QUERY)
	{
		SoftQueryInfoIter end = _softQueryInfoList.begin();
		advance(end, MAX_PER_QUERY);

		SoftQueryInfoList queryList;
		queryList.splice(queryList.end(), _softQueryInfoList, _softQueryInfoList.begin(), end);

		CommitTask(queryList, pParams);
	}

	if(!_softQueryInfoList.empty())
	{
		CommitTask(_softQueryInfoList, pParams);
	}

	return TRUE;
}

BOOL CCacheSoftInfoOpr::Query(LONG id, wstring &grade)
{
	if(_pQuery == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"select grade from soft_info_list where soft_id=?;", &_pQuery);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pQuery->Reset();
	}

	_pQuery->Bind(1, static_cast<int>(id));

	CComPtr<Skylark::ISQLiteComResultSet3> pRs;
	HRESULT hr = _pQuery->ExecuteQuery(&pRs);
	if(!SUCCEEDED(hr) || pRs->IsEof()) return FALSE;

	grade = pRs->GetAsString(L"grade");
	return TRUE;
}

BOOL CCacheSoftInfoOpr::Insert(LONG id, const wstring &grade, __time32_t time)
{
	if(_pInsert == NULL)
	{
		HRESULT hr = _pDB->PrepareStatement(L"insert into soft_info_list(soft_id,grade, update_time)values(?,?,?)", &_pInsert);
		if(!SUCCEEDED(hr)) return FALSE;
	}
	else
	{
		_pInsert->Reset();
	}

	_pInsert->Bind(1, static_cast<int>(id));
	_pInsert->Bind(2, grade.c_str());
	_pInsert->Bind(3, static_cast<int>(time));
	return (SUCCEEDED(_pInsert->ExecuteUpdate()));
}

BOOL CSoftInfoQueryTask::TaskProcess(CTaskMgr *pMgr)
{
	do
	{
		ISQLiteComDatabase3 *pDB = _pSoftQuery->GetDBPtr();
		if(pMgr->IsExited()) break;

		// 1.查询本地缓存
		{
			SoftInfo2List resultList;
			CCacheSoftInfoOpr softInfoOpr(pDB);
			SoftQueryInfoIter endQuery = _softQueryInfoList.end();
			for(SoftQueryInfoIter it = _softQueryInfoList.begin(); it != endQuery; )
			{
				wstring grade;
				if(softInfoOpr.Query(it->_id, grade) && !grade.empty())
				{
					resultList.push_back(SoftInfo2(it->_id, grade));
					_softQueryInfoList.erase(it++);
				}
				else
				{
					++it;
				}
			}

			// 3.通知用户
			if(pMgr->IsExited()) break;
			if(!resultList.empty())
			{
				CSoftInfoEnum softInfoEnum(resultList);
				_pSoftQuery->GetNotify()->SoftQueryNotify(&softInfoEnum, _pParams);
			}
		}

		// 2.查询网络
		if(!_softQueryInfoList.empty())
		{
			if(pMgr->IsExited()) break;

			SoftInfo2List resultList;
			try{ QueryByNetwork(_softQueryInfoList, resultList); } catch(...) {}

			// 处理返回结果
			if(!resultList.empty())
			{
				if(pMgr->IsExited()) break;
				{
					CSoftInfoEnum softInfoEnum(resultList);
					_pSoftQuery->GetNotify()->SoftQueryNotify(&softInfoEnum, _pParams);
				}
				if(pMgr->IsExited()) break;

				// 保存到本地缓存
				pDB->BeginTransaction();
				{
					__time32_t now = _time32(NULL);
					CCacheSoftInfoOpr softInfoOpr(pDB);

					SoftInfo2CIter end = resultList.end();
					for(SoftInfo2CIter it = resultList.begin(); it != end; ++it)
					{
						softInfoOpr.Insert(it->_id, it->_grade, now);
					}
				}
				pDB->CommitTransaction();
			}
		}
	}
	while(FALSE);

	return FALSE;
}

BOOL CSoftInfoQueryTask::QueryByNetwork(const SoftQueryInfoList &queryInfoList, SoftInfo2List &resultList)
{
	static const LPCWSTR pContentType = L"Content-Type: application/x-www-form-urlencoded\r\n";

	WinHttpApi2 *pHttpApi = _pSoftQuery->GetWinHttpApi();
	if(pHttpApi == NULL) return FALSE;

	BOOL succeeded = FALSE;
	HINTERNET hSess = NULL;
	HINTERNET hConn = NULL;
	HINTERNET hHttp = NULL;

	do 
	{
		//
		// 加载代理数据
		//
		CString strIniPath;
		CAppPath::Instance().GetLeidianCfgPath(strIniPath);
		strIniPath += _T("\\bksafe.ini");

		static const int BUF_SIZE = 64;

		UINT validate = 0;
		wchar_t szUser[BUF_SIZE] = {0};
		wchar_t szPwd[BUF_SIZE] = {0};
		wchar_t szProxy[BUF_SIZE * 2] = {0};

		UINT type = ::GetPrivateProfileIntW(L"proxy", L"type", 0, strIniPath);
		if(type == 1)
		{
			// 改成无代理
			type = 0;
			
			do
			{
				UINT port = ::GetPrivateProfileIntW(L"proxy", L"port", 65536, strIniPath);
				if(port == 65536) break;

				wchar_t szHost[BUF_SIZE] = {0};
				::GetPrivateProfileStringW(L"proxy", L"host", NULL, szHost, BUF_SIZE, strIniPath);
				if(szHost[0] == L'\0') break;

				validate = ::GetPrivateProfileIntW(L"proxy", L"validate", -1, strIniPath);
				if(validate == static_cast<UINT>(-1)) break;

				INTERNET_PROXY_INFO proxy = {0};
				proxy.dwAccessType = INTERNET_OPEN_TYPE_PROXY;

				swprintf_s(szProxy, BUF_SIZE * 2, L"%s:%d", szHost, port);
				proxy.lpszProxy = szProxy;

				if(validate != 0)
				{
					::GetPrivateProfileStringW(L"proxy", L"user", NULL, szUser, BUF_SIZE, strIniPath);
					if(szUser[0] == L'\0') break;

					::GetPrivateProfileStringW(L"proxy", L"password", NULL, szPwd, BUF_SIZE, strIniPath);
					if(szPwd[0] == L'\0') break;
				}

				// 加载数据成功
				type = 1;
			}
			while(FALSE);
		}

		//
		// 设置代理
		//
		if(type == 1)
		{
			hSess = pHttpApi->OpenSession2(L"king_guard_softmgr", szProxy);
		}
		else
		{
			hSess = pHttpApi->OpenSession(L"king_guard_softmgr");
		}		
		if(hSess == NULL) break;

		pHttpApi->SetTimeouts(hSess, QUERY_TIMEOUT, QUERY_TIMEOUT, QUERY_TIMEOUT, QUERY_TIMEOUT);

		hConn = pHttpApi->Connect(hSess, QUERY_SERVER_NAME, INTERNET_DEFAULT_PORT);
		if(hConn == NULL) break;

		hHttp = pHttpApi->OpenRequest(hConn, L"POST", QUERY_OBJECT_NAME, INTERNET_SCHEME_HTTP);
		if(hHttp == NULL) break;

		string bodyContent;
		BOOL result = CombineBodyContent(queryInfoList, bodyContent);
		if(!result) break;

		result = pHttpApi->AddRequestHeaders(hHttp, pContentType);
		if(!result) break;

		//
		// 设置代理用户名与密码
		//
		if(type == 1  && validate == 1)
		{
			if(pHttpApi->IsWinHttp())
			{
				pHttpApi->SetOption(hHttp, WINHTTP_OPTION_PROXY_USERNAME, szUser, wcslen(szUser));
				pHttpApi->SetOption(hHttp, WINHTTP_OPTION_PROXY_PASSWORD, szPwd, wcslen(szPwd));
			}
			else
			{
				pHttpApi->SetOption(hHttp, INTERNET_OPTION_PROXY_USERNAME, szUser, wcslen(szUser));
				pHttpApi->SetOption(hHttp, INTERNET_OPTION_PROXY_PASSWORD, szPwd, wcslen(szPwd));
			}
		}

		result = pHttpApi->SendRequest(hHttp, &bodyContent[0], static_cast<DWORD>(bodyContent.size()));
		if(!result) break;

		result = pHttpApi->EndRequest(hHttp);
		if(!result) break;

		DWORD cbReturn = 0;
		DWORD cbSize = sizeof(cbReturn);
		result = pHttpApi->QueryInfo(hHttp, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, reinterpret_cast<char*>(&cbReturn), &cbSize);
		if(!result || cbReturn == 0) break;

		auto_buffer<char> buffer(cbReturn + 2 + 1);
		if(buffer.empty()) break;

		DWORD cbRecved = 0;
		result = pHttpApi->ReadData(hHttp, &buffer[0], cbReturn, &cbRecved);
		if(!result || cbRecved != cbReturn) break;

		// 在结尾加\r\n\0空字符
		buffer[cbReturn] = '\r';
		buffer[cbReturn + 1] = '\n';
		buffer[cbReturn + 2] = '\0';

		// 开始解析
		TiXmlDocument xmlDoc;
		if(xmlDoc.Parse(&buffer[0]) == NULL) break;

		TiXmlHandle hRoot(xmlDoc.FirstChildElement("r"));
		for(TiXmlElement *pElem = hRoot.FirstChildElement("sid").Element(); pElem != NULL; pElem = pElem->NextSiblingElement())
		{
			LPCSTR pId = pElem->Attribute("id");
			if(pId == NULL) continue;

			LONG id = atol(pId);
			if(id == 0) continue;

			TiXmlElement *ppElem = pElem->FirstChildElement("grade");
			if(ppElem == NULL) continue;

			LPCSTR pGrade = ppElem->GetText();
			if(pGrade == NULL) continue;

			resultList.push_back(SoftInfo2(id, wstring(CA2W(pGrade))));
		}

		succeeded = TRUE;
	}
	while(FALSE);

	if(hHttp != NULL)
		pHttpApi->CloseInternetHandle(hHttp);

	if(hConn != NULL)
		pHttpApi->CloseInternetHandle(hConn);

	if(hSess != NULL)
		pHttpApi->CloseInternetHandle(hSess);

	return succeeded;
}

BOOL CSoftInfoQueryTask::CombineBodyContent(const SoftQueryInfoList &queryInfoList, string &bodyContent)
{
	try
	{
		// SID
		string sids;
		{
			ostringstream os;
			os.imbue(locale::classic());

			SoftQueryInfoCIter end = queryInfoList.end();
			for(SoftQueryInfoCIter it = queryInfoList.begin(); it != end; ++it)
			{
				os<<it->_id<<',';
			}

			sids = os.str();
			trim_right(sids, ",");
		}

		// 验证码
		string code = "&c=";
		{
			unsigned int crc32 = ::CRC32(0, sids.c_str(), static_cast<unsigned int>(sids.length()));

			static const char KEY[] = "kingsoftgrade2_%^}[793dkle";
			crc32 = ::CRC32(crc32, KEY, static_cast<unsigned int>(sizeof(KEY) - 1));

			ostringstream os;
			os.imbue(locale::classic());

			os<<hex<<crc32;
			code += os.str();
		}

		// 版本 & SID & 验证码
		string plainText = string("v=1_0&sid=") + sids + code;

		// base64
		DWORD sizeBase64 = Base64EncodeGetLength(static_cast<DWORD>(plainText.length()));

		auto_buffer<char> buffer(sizeBase64 + 1);
		if(buffer.empty()) return FALSE;

		sizeBase64 = Base64Encode(&buffer[0], plainText.c_str(), static_cast<DWORD>(plainText.length()));
		buffer[sizeBase64] = '\0';

		bodyContent = string("c=") + &buffer[0] + "\r\n";
		return TRUE;
	}
	catch(...) {}
	return FALSE;
}

}
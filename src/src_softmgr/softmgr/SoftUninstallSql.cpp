#include "stdafx.h"
#include "SoftUninstallSql.h"

namespace ksm
{

void InitCacheDB(Skylark::ISQLiteComDatabase3 *pDB)
{
	CCacheTransaction transaction(pDB);

	// soft_info_list
	static const LPCWSTR pSqlCreateSoftInfoList =
		L"create table soft_info_list("
		L"soft_id int primary key,"
		L"type_id int,"
		L"match_type int,"
		L"logo_url,"
		L"brief,"
		L"info_url,"
		L"pattern);";

	pDB->ExecuteUpdate(L"drop table soft_info_list");
	pDB->ExecuteUpdate(pSqlCreateSoftInfoList);

	// local_soft_list
	static const LPCWSTR pSqlCreateLocalSoftList =
		L"create table local_soft_list("
		L"soft_key nvarchar(260) primary key,"
		L"guid,"
		L"display_name,"
		L"main_path,"
		L"descript,"
		L"descript_reg,"
		L"info_url,"
		L"spell_whole,"
		L"spell_acronym,"
		L"icon_location,"
		L"uninstall_string,"
		L"logo_url,"
		L"size bigint,"
		L"last_use int,"
		L"daycnt int,"
		L"type_id int,"
		L"soft_id int,"
		L"match_type int,"
		L"pattern);";

	pDB->ExecuteUpdate(L"drop table local_soft_list");
	pDB->ExecuteUpdate(pSqlCreateLocalSoftList);
	pDB->ExecuteUpdate(L"create index index_main_path on local_soft_list(main_path);");

	// link_list
	static const LPCWSTR pSqlCreateLinkList = 
		L"create table link_list("
		L"soft_key,"
		L"type int);";

	pDB->ExecuteUpdate(L"drop table link_list");
	pDB->ExecuteUpdate(pSqlCreateLinkList);

	// rubbish_list
	static const LPCWSTR pSqlCreateRubbishList =
		L"create table rubbish_list("
		L"path primary key,"
		L"type int);";

	pDB->ExecuteUpdate(L"drop table rubbish_list");
	pDB->ExecuteUpdate(pSqlCreateRubbishList);

	// cache_flag_list
	static const LPCWSTR pSqlCreateCacheFlagList =
		L"create table cache_flag_list("
		L"path primary key,"
		L"last_write_time bigint);";

	pDB->ExecuteUpdate(L"drop table cache_flag_list");
	pDB->ExecuteUpdate(pSqlCreateCacheFlagList);
}

BOOL CCacheSoftFind::Query(const std::wstring &path, std::wstring &key)
{
	std::wstring temp = path;
	// 仅针对三层目录进行查找
	for(int i = 0; i < 3; ++i)
	{
		// 过滤敏感路径（主要针对系统进程等）
		if(_sensiPaths.IsSensitive(temp)) break;

		if(InternalQuery(temp, key))
		{
			return TRUE;
		}

		// 上一级目录
		if(!RemoveFileSpec(temp)) break;
	}

	return FALSE;
}

}
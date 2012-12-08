/**
* @file    SoftManager
* @brief   软件管理
* @author  liveck
* @date    2010/04/07 16:10
*/
#pragma once

#include <atlstr.h>
#include <atlrx.h>
#include <atlcoll.h>
#include <sqlite3/sqlite3.h>
#include <softmgr/ISoftManager.h>
#include <tinyxml/tinyxml.h>

class CSoftManager :public ISoftManager
{
public:
	CSoftManager(void);
	virtual ~CSoftManager(void);

	/**
	* 加载软件库
	* @return   int
	* @param CString lib_file
	* @param bool reload
	*/
	virtual int LoadSoftDB(CString lib_file);

	/**
	* 清空软件库
	* @return   int
	*/
	virtual int FreeSoftDB();

	virtual int GetSoftCount(CString cat);

	virtual int GetSoftById(CString id,CAtlMap<CString,CString>& soft);
	virtual int GetSoftById(CString id, GetInfoCallback func ,void* pSoft)
	{
		CAtlMap<CString,CString> soft;
		int ret=GetSoftById(id,soft);
		for (POSITION p=soft.GetStartPosition();p;)
		{
			CString key=soft.GetKeyAt(p);
			CString val=soft.GetValueAt(p);
			func(pSoft,key.GetBuffer(),val.GetBuffer());
			soft.GetNext(p);
		}
		return ret;
	}
	virtual void* GetLast50Soft();

	virtual void* GetEssentialSoft(CString cat,CString rank);

	virtual void* GetAllNecessSoft();
	virtual void* GetAllSoft(CString cat,CString rank=L"");

	virtual void* SearchSoft(CString cat,CString rank="");

	virtual int GetNextSoft(void* pos,CAtlMap<CString,CString>& soft);
	virtual int GetNextSoft(void* pos, GetInfoCallback func ,void* pSoft)
	{
		if( pos == NULL )
			return 1;

		if( pSoft == NULL )
			return 1;

		CAtlMap<CString,CString> soft;
		soft.RemoveAll();
		int ret=GetNextSoft(pos,soft);

		if( soft.GetCount() > 0 )
		{
			for (POSITION p=soft.GetStartPosition();p;)
			{
				CString key=soft.GetKeyAt(p);
				CString val=soft.GetValueAt(p);
				func(pSoft,key.GetBuffer(),val.GetBuffer());
				soft.GetNext(p);
			}
		}
		
		return ret;
	}

	virtual int FinalizeGet(void* pos);

	/**
	* 加载热度文件
	* @return   CString rank_name
	* @param CString lib_file 热度文件路径
	*/
	virtual CString LoadRank(CString lib_file);

	virtual int FreeRank(CString rank_name);

	// 加载装机必备的库
	virtual BOOL LoadNecessDat(LPCTSTR strLibFile);

	BOOL	LoadOnekeyDat(LPCTSTR lpstrLib);
	// 刷入数据库头部信息
	virtual void UpdateCache();

	//判断库文件跟缓存文件是否版本不一样
	virtual BOOL IsLibUpdate();

	BOOL	_CheckCacheOutTime();	// 检测是否cache过期

	virtual BOOL GetNecessSoftInfo( CSimpleArray<NECESS_GROUP>** ppInfo );
	virtual BOOL GetOneKeySoftInfo( CSimpleArray<ONE_KEY_GROUP>** ppInfo);

	virtual VOID FreeNecessInfo();
	/**
	* 获取软件分类
	* @return   int 分类数量
	* @param CAtlArray<CString> & result
	*/
	virtual int GetCatalog(CAtlArray<CString>& result);
	virtual int GetCatalog(GetCatlogCallback func,void* para)
	{
		CAtlArray<CString> a;
		int ret=GetCatalog(a);
		for (size_t i=0;i<a.GetCount();i++)
		{
			func(para,a[i].GetBuffer());
		}
		return ret;
	}

	/**
	* 猜测下载一句话描述
	* @return   CString
	* @param CString name
	*/
	virtual CString GetUnistallInfo(CString name,CAtlMap<CString,CString>& soft);

	virtual CString GetUnistallInfo(CString name, GetInfoCallback func ,void* pSoft)
	{
		CAtlMap<CString,CString> soft;
		CString ret=GetUnistallInfo(name,soft);
		for (POSITION p=soft.GetStartPosition();p;)
		{
			CString key=soft.GetKeyAt(p);
			CString val=soft.GetValueAt(p);
			func(pSoft,key.GetBuffer(),val.GetBuffer());
			soft.GetNext(p);
		}
		return ret;
	}


	virtual int Combine(CString diff_file);

	virtual int GetCategroy(CAtlMap<CString, CString>& type);
	virtual int GetCategroy(GetInfoCallback func ,void* pParam);


	void __merge( TiXmlElement* el, CString att, CAtlMap<CString,TiXmlElement*>* filter, TiXmlElement* re );
private:
	sqlite3* _db;
	CString _libfile,_rankfile,_necessLibFile,_autoinstLibFile;

	CAtlMap<CString,CString> _sln;

	int sql_run(CString& sql);
	int sql_run(const wchar_t* sql)
	{
		CString ssql=sql;
		return sql_run(ssql);
	}

	CString GuessDescirption(CString word,CAtlMap<CString,CString>& soft);

	int InitSqlite3();

	int AppendNV(CString& name,CString& value,const char* name_key,const char* name_att,CString sep=",");

	void __GetDel(CString act,CString type, TiXmlElement* pRt, CAtlMap<CString,TiXmlElement*> &ids );

	bool has_cache();
	void update_cache();

	int __loadSoftdb(CString lib_file, BOOL bCheckCache);
	CString __loadRank(CString lib_file, BOOL bCheckCache);
	
	CString _cache_name;


protected:
	CSimpleArray<NECESS_GROUP>	m_necessGroup;
	CSimpleArray<ONE_KEY_GROUP> m_oneKeyGroup;
	CAtlMap<CString,CString>	m_ncessIdMap;
};


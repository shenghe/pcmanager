/**
* @file    SoftManager.cpp
* @brief   软件管理
* @author  liveck
* @date    2010/04/07 23:33
*/

#pragma once
#include "stdafx.h"
#include <libheader/libheader.h>
#include "SoftManager.h"

#pragma comment(lib,"sqlite3.lib")


int CSoftManager::LoadSoftDB( CString lib_file )
{
	return __loadSoftdb(lib_file,TRUE);
}

CSoftManager::~CSoftManager( void )
{
	sqlite3_close(_db);
}

CSoftManager::CSoftManager( void )//_cache_name(L":memory:")
{
	TCHAR tempdir[MAX_PATH]={0};
	GetTempPath(MAX_PATH,tempdir);
	//_cache_name=tempdir;
	//_cache_name+=L"\\ksoft_cache_2";

	
	GetModuleFileName(NULL,tempdir,MAX_PATH);
	PathRemoveFileSpec(tempdir);

	_cache_name=tempdir;
	_cache_name+=L"\\AppData\\ksoft_cache_3";

	CString	strOldCache;
	strOldCache = tempdir;
	strOldCache += L"\\AppData\\ksoft_cache_2";
	if (::PathFileExists(strOldCache))
	{
		::DeleteFile(strOldCache);
	}

	InitSqlite3();

	TCHAR szPath[MAX_PATH]={0};
	GetModuleFileName(NULL, szPath, MAX_PATH);
	CString strPath;
	strPath.Format(_T("%s"), szPath);
	strPath = strPath.Right( strPath.GetLength() - strPath.ReverseFind(_T('\\')) - 1 );
	strPath.MakeLower();
	if (strPath == L"ksafetray.exe")
	{
		_cache_name=L":memory:";
	}

	CString temp=tempdir;
	if(PathFileExists(temp+L"\\ksoft\\data\\softmgr.dat"))
	{
		_libfile=temp+L"\\ksoft\\data\\softmgr.dat";
	}
	if(PathFileExists(temp+L"\\ksoft\\data\\rank.dat"))
		_rankfile=temp+L"\\ksoft\\data\\rank.dat";

	if(PathFileExists(temp+L"\\ksoft\\data\\softnecess.dat"))
		_necessLibFile=temp+L"\\ksoft\\data\\softnecess.dat";
	if (PathFileExists(temp+L"\\ksoft\\data\\autoinst.dat"))
		_autoinstLibFile=temp+L"\\ksoft\\data\\autoinst.dat";
}

int CSoftManager::FreeSoftDB()
{
	int ret=sqlite3_close(_db);
	if(PathFileExists(_cache_name))
		DeleteFile(_cache_name);
	InitSqlite3();
	return 0;
}

int CSoftManager::GetSoftCount( CString cat )
{
	CString sql=L"select count(softid) from soft";
	sql+=L" join catalog on typeid=id ";
	if(cat!=L"")
		sql+=L"where typename == \""+cat+L"\"";
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	ret=sqlite3_step(st);
	int ct=sqlite3_column_count(st);
	if(ct==0)
		sql="";
	else
		sql=(wchar_t*)sqlite3_column_text16(st,0);
	ret=sqlite3_finalize(st);
	return _wtoi(sql);
}

int CSoftManager::GetSoftById( CString id,CAtlMap<CString,CString>& soft )
{
	CString sql="select * from soft join catalog on typeid=id where softid = \""+id+L"\"";
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	ret=sqlite3_step(st);
	if(ret==SQLITE_DONE)
		return 1;

	int ct=sqlite3_column_count(st);
	if(ct==0)
		return 1;
	for (int i=0;i<ct;i++)
	{
		soft[(wchar_t*)sqlite3_column_name16(st,i)]=(wchar_t*)sqlite3_column_text16(st,i);
	}
	ret=sqlite3_finalize(st);

	return 0;
}

void* CSoftManager::GetEssentialSoft( CString cat,CString rank )
{
	CString sql;
	if(_sln[L"装机必备"]!="")
	{
		sql_run(L"drop view ess_view");
		sql=L"create view ess_view as select * from soft join catalog on typeid=id ";
		if(rank!=L"")
		{
			sql+=L" join rank on softid=sfid ";
		}
		sql+=L"where softid in ("+_sln[L"装机必备"]+L")";

		if(rank!=L"")
		{
			sql+=L" and "+rank+L" is not null and softid is not null";
		}

		if(cat!=L"")
			sql+=L" and typename == \""+cat+L"\"";

		if(rank!=L"")
		{
			sql+=L" group by softid, "+rank+L" ";
		}

		if(rank!=L"")
			sql+=L" order by "+rank+L" desc ,updatetime desc";
		else
			sql+=L" order by updatetime desc";
		sql_run(sql);
	}
	if(has_cache()==false)
		update_cache();

	sql=L"select * from ess_view ";
	if(rank!=L"")
		sql+=L" order by "+rank+L" desc ,updatetime desc";
	else
		sql+=L" order by updatetime desc";

	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	return st;
}

void* CSoftManager::GetAllNecessSoft()
{
	CString sql=L"";
	{
		sql=L"create view necess_view as select * from necess";
		sql+=L" join catalog on typeid=id ";

		sql_run(sql);
	}

	sql=L"select * from necess_view";

	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	return st;
}

void* CSoftManager::GetAllSoft( CString cat,CString rank )
{
	CString sql=L"";
	{
		sql=L"create view allsoft_"+cat+rank+L"_view as select * from soft";
		sql+=L" join catalog on typeid=id ";

		if(rank!=L"")
		{
			sql+=L" join rank on softid=sfid";
		}

		if(cat!=L"")
			sql+=L" where typename == \""+cat+L"\"";

		if(rank!=L"")
		{
			if(cat==L"")
				sql+=L" where ";
			else
				sql+=L" and ";
			sql+=rank+L" is not null and softid is not null";
		}
		if(rank!=L"")
		{
			sql+=L" group by softid, "+rank+L" ";
		}
		if(rank!=L"")
		{
			sql+=L" order by "+rank +L" desc ,updatetime desc";
		}
		else
			sql+=L" order by updatetime desc";

		sql_run(sql);
	}

	sql=L"select * from allsoft_"+cat+rank+L"_view";

	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	return st;
}	

int CSoftManager::GetNextSoft( void* pos,CAtlMap<CString,CString>& soft )
{
	//OutputDebugString(L"BEFORE:!");
	sqlite3_stmt* st=(sqlite3_stmt*)pos;
	
	int ret=sqlite3_step(st);
	if(ret!=SQLITE_ROW)
		return 1;

	int ct=sqlite3_column_count(st);
	if(ct==0)
		return 1;
	
	for (int i=0; i < ct; i++)
	{
		CString strKey; 
		CString strValue;

		wchar_t* pKey = (wchar_t*)sqlite3_column_name16(st,i);
		if( pKey )
			strKey = pKey;

		wchar_t* pValue = (wchar_t*)sqlite3_column_text16(st,i);
		if( pValue )
			strValue = pValue;
		
		soft[strKey] = strValue;
	}

	//OutputDebugString(L"AFTER:!");
	return 0;
}


CString CSoftManager::LoadRank( CString lib_file )
{
	return __loadRank(lib_file,TRUE);
}

int CSoftManager::FreeRank( CString rank_name )
{
	sql_run(L"drop table rank");
	sql_run(L"drop index ranki");
	sql_run(L"drop table header_t");
	return 0;
}

int CSoftManager::GetCatalog( CAtlArray<CString>& result )
{
	CString sql="select typename from catalog order by id/1";
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	do 
	{
		ret=sqlite3_step(st);
		if(ret==100)
			result.Add((wchar_t*)sqlite3_column_text16(st,0));
	} while (ret==SQLITE_ROW);
	ret=sqlite3_finalize(st);

	return (int)result.GetCount();
}

CString CSoftManager::GetUnistallInfo( CString name ,CAtlMap<CString,CString>& soft)
{
	CString ret;
	if(CAtlRECharTraits::Isdigit(name[0])&&name.GetLength()<4)
		;
	else
		ret=GuessDescirption(name,soft);

	if(ret=="")
	{
		name.MakeLower();

		CAtlRegExp<> re;
		CAtlREMatchContext<> mc;
		re.Parse(L"{(V|v)?\\d+\\.\\d+\\.?\\d*\\.?\\d*}");
		while(TRUE==re.Match(name,&mc))
		{
			//mc.GetMatch)
			for (UINT nGroupIndex = 0; nGroupIndex < mc.m_uNumGroups;
				++nGroupIndex)
			{
				const CAtlREMatchContext<>::RECHAR* szStart = 0;
				const CAtlREMatchContext<>::RECHAR* szEnd = 0;
				mc.GetMatch(nGroupIndex, &szStart, &szEnd);
				CString a;
				a.Format(L"%.*s",szEnd-szStart,szStart);
				name.Replace(a,L"");
			}
		}
		re.Parse(L"{\\d+}");
		while(TRUE==re.Match(name,&mc))
		{
			//mc.GetMatch)
			for (UINT nGroupIndex = 0; nGroupIndex < mc.m_uNumGroups;
				++nGroupIndex)
			{
				const CAtlREMatchContext<>::RECHAR* szStart = 0;
				const CAtlREMatchContext<>::RECHAR* szEnd = 0;
				mc.GetMatch(nGroupIndex, &szStart, &szEnd);
				CString a;
				a.Format(L"%.*s",szEnd-szStart,szStart);
				name.Replace(a,L"");
			}
		}
		ret=GuessDescirption(name,soft);
		if(ret!=L"")
			return ret;

		const wchar_t* torep[]={
			L"卸载",L"测试",L"公测",L"内测",L"正式",L"优化",L"美化",L"合作",
			L"修改版",L"英文版",L"专业版",L"体验版",
			L"(R)",L"(C)",L"兼容包",L"uninstall",L"install",L"beta",L" - ",
			L"简体中文",L"版",L"x86",L"x64",L"(TM)",L"service pack",
			L"语言包",L"播放器",L"(",L"（",L"）",L")",L"仅移除",L"汉化",
			L"trial",L"共享版",L"免费版",L"bit"

		};

		for (int i=0;i<sizeof torep/sizeof torep[0];i++)
		{
			name.Replace(torep[i],L"%");
		}
		ret=GuessDescirption(name,soft);
		if(ret!=L"")
			return ret;

		const wchar_t* sprep[][2]={
			{L"qq",L"qq%"}
		};
		for (int i=0;i<sizeof sprep/sizeof sprep[0];i++)
		{
			name.Replace(sprep[i][0],sprep[i][1]);
		}
		ret=GuessDescirption(name,soft);
		if(ret!=L"")
			return ret;


		if(name.Replace(L" ",L" ")>4)
		{
			name=name.Left(20);
		};
		ret=GuessDescirption(name,soft);
		if(ret!=L"")
			return ret;
		name.Replace(L" ",L"%");
		ret=GuessDescirption(name,soft);
		//if(ret==L"")
		//{
		//	name=name.Left(6);
		//	ret=GuessDescirption(name,soft);
		//}
	}
	return ret;
}

int CSoftManager::InitSqlite3()
{
	//sqlite3_open16(L"test.db",&_db);
	sqlite3_open16(_cache_name,&_db);
	sql_run(L"PRAGMA page_size=8192;");
	sql_run(L"PRAGMA synchronous=OFF;");
	sql_run(L"PRAGMA encoding = \"UTF-16\";");
	
	return 0;
}

int CSoftManager::AppendNV( CString& name,CString& val,const char* name_key,const char* name_att ,CString sep)
{
	if(name_att)
	{
		name+=sep;
		name+=name_key;
		val+=sep;
		val+="\"";
		val+=name_att;
		val+="\"";
	}
	return 0;
}

int CSoftManager::sql_run( CString& sql )
{
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	ret=sqlite3_step(st);
	ret=sqlite3_finalize(st);
	return ret;
}

int CSoftManager::FinalizeGet( void* pos )
{
	sqlite3_stmt* st=(sqlite3_stmt*)pos;
	sqlite3_finalize(st);
	return 0;
}

CString CSoftManager::GuessDescirption( CString word,CAtlMap<CString,CString>& soft )
{
	if(word==L"")
		return word;
	word=L"\"%"+word+L"%\"";
	while(word.Replace(L"%%",L"%"));
	CString sql=L"select * from soft join catalog on typeid=id join rank on softid=sfid where uninstallname like "+word +L" or softname like "+ word;
	sql+=L" order by ar desc ";
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	ret=sqlite3_step(st);
	int ct=sqlite3_column_count(st);
	if(ct==0)
		return L"";
	sql=L"";
	for (int i=0;i<ct;i++)
	{
		//sql=(wchar_t*)sqlite3_column_name16(st,i);
		if((wchar_t*)sqlite3_column_name16(st,i)==CString(L"brief"))
			sql=(wchar_t*)sqlite3_column_text16(st,i);
		CString key=(const wchar_t*)sqlite3_column_name16(st,i);
		CString val=(const wchar_t*)sqlite3_column_text16(st,i);
		soft[key]=val;
	}
	ret=sqlite3_finalize(st);
	return sql;
}

void* CSoftManager::SearchSoft( CString cat,CString rank/*=""*/ )
{
	//
	// 对搜索关键字进行预处理
	//
	// 1）两端加%
	// 2）替换空格（包括中英文）
	// 3）合并重复%
	//

	CString keyword = L"%";
	int state = 1; // 初始化为：%与空格状态，2为其它字符状态

	LPCWSTR p = cat;
	while(TRUE)
	{
		if(state == 1)
		{
			if(*p == L'\0')
			{
				break;
			}
			else if(*p != L'%' && *p != L' ' && *p != L'　')
			{
				keyword += *p;
				state = 2;
			}
		}
		else
		{
			if(*p == L'\0')
			{
				keyword += L'%';
				break;
			}
			if(*p == L'%' || *p == L' ' || *p == L'　')
			{
				keyword += L'%';
				state = 1;
			}
			else
			{
				keyword += *p;
			}
		}

		++p;
	}

	cat = keyword;

	CString sql=L"select *,softname like '"+cat+L"' as nm ";
	sql+=L", py like '"+cat+L"' as pm ";
	sql+=L", qp like '"+cat+L"' as qm ";
	sql+=L", substr(filename,1,length(filename)-4) like '"+cat+L"' as fm ";
	sql+=L", brief like '"+cat+L"' as bm ";
	sql+=L" from soft ";

	if(rank!=L"")
	{
		sql+=L" join (select "+rank+L",sfid from rank where "+rank+ L">=0 ) on softid=sfid";
	}

	sql+=L" where softname like '"+cat+L"' or brief like '" +cat +
		L"' or substr(filename,1,length(filename)-4) like '"+ cat + L"' or qp like '"+ cat + L"' or py like '"+ cat + L"'";

	if(rank!=L"")
	{
		sql+=L" and "+rank+L" is not null and sfid is not null and "+rank+L"!=\"\" ";
	}

	if(rank!=L"")
	{
		sql+=L" group by sfid,"+rank;//nm desc,qm desc,pm desc,fm desc,bm desc, 
		sql+=L" order by nm desc, ";
		sql+=L"pm desc, ";
		sql+=L"qm desc, ";
		sql+=L"fm desc, ";
		sql+=L"bm desc, ";
		sql+=rank +L" desc ,updatetime desc";
	}

	sql += L" limit 0,200";

	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);

	return st;


}

int CSoftManager::Combine( CString diff_file )
{
	::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 

	CDataFileLoader	loader;
	BkDatLibHeader old_h;
	BkDatLibHeader new_h;
	if(_libfile==L"")
	{
		CString lib;
		PathCombine(lib.GetBuffer(1024),diff_file,L"..\\softmgr.dat");
		_libfile=lib;
	}
	if(loader.GetLibDatHeader(diff_file,new_h)==FALSE
		||loader.GetLibDatHeader(_libfile,old_h)==FALSE
		||old_h.llVersion.QuadPart!=new_h.llUpdateForVer.QuadPart)
		return 1;

	BkDatLibContent old_c;
	BkDatLibContent new_c;
	if(loader.GetLibDatContent(diff_file,new_c)==FALSE
		||loader.GetLibDatContent(_libfile,old_c)==FALSE)
		return 1;


	TiXmlDocument plugins;
	/*if(false==plugins.LoadFile(CW2A(_libfile)))
		return 1;*/
	TiXmlDocument diffs;
	//if(false==diffs.LoadFile(CW2A(diff_file)))
	//	return 1;
	if(plugins.Parse((char*)old_c.pBuffer)==false||diffs.Parse((char*)new_c.pBuffer)==false)
		return 2;

	TiXmlDocument newlib;
	TiXmlDeclaration* dec=new TiXmlDeclaration("1.0","gbk","");
	newlib.LinkEndChild(dec);
	TiXmlElement* rt=new TiXmlElement("allsoft");
	newlib.LinkEndChild(rt);

	TiXmlElement* pRt_old=plugins.RootElement();
	TiXmlElement* pRt_new=diffs.RootElement();

	TiXmlElement* pE=pRt_new->FirstChildElement("need");

	rt->LinkEndChild(pE?pE->Clone():pRt_old->FirstChildElement("need")->Clone());
	pE=pRt_new->FirstChildElement("sample");
	rt->LinkEndChild(pE?pE->Clone():pRt_old->FirstChildElement("sample")->Clone());
	pE=pRt_new->FirstChildElement("softwares");
	rt->LinkEndChild(pE?pE->Clone():pRt_old->FirstChildElement("softwares")->Clone());

	CAtlMap<CString,TiXmlElement*> types;
	CAtlMap<CString,TiXmlElement*> softs;

	 __GetDel("del","softwares", pRt_new, softs);
	 __GetDel("del","softtype", pRt_new, types);
	 __GetDel("modify","softwares", pRt_new, softs);
	 __GetDel("modify","softtype", pRt_new, types);

	 CString type=L"softwares";
	 CString att=L"softid";
	 TiXmlElement* el=pRt_old->FirstChildElement(CW2A(type));
	 TiXmlElement* re=rt->FirstChildElement(CW2A(type));
	 if(re==NULL)
	 {
		 re=new TiXmlElement(CW2A(type));
		 rt->LinkEndChild(re);
	 }
	 CAtlMap<CString,TiXmlElement*>* filter=&softs;
	 __merge(el, att, filter, re);
	 filter=&types;
	 type=L"softtype";
	 att=L"id";
	 el=pRt_old->FirstChildElement(CW2A(type));
	 re=rt->FirstChildElement(CW2A(type));
	 if(re==NULL)
	 {
		 re=new TiXmlElement(CW2A(type));
		 rt->LinkEndChild(re);
	 }
	 __merge(el, att, filter, re);

	 TiXmlPrinter printer;
	 printer.SetIndent( "\t" );
	 newlib.Accept( &printer );

	 BkDatLibEncodeParam	paramx(UnMarkdatUpdate(new_h.dwType),new_h.llVersion,(BYTE*)printer.CStr(),(DWORD)printer.Size(),1);
	 loader.Save(_libfile,paramx);

	//newlib.SaveFile(CW2A(_libfile));

	//FreeSoftDB();
	//LoadSoftDB(_libfile);
	//LoadRank(_rankfile);


	return 0;
}

int CSoftManager::GetCategroy(CAtlMap<CString, CString>& type)
{
	CString sql="select id, typename from catalog order by id/1";
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);

	do
	{
		ret=sqlite3_step(st);
		if( ret==100 )
		{
			CString _strKey = (wchar_t*)sqlite3_column_text16(st,0);
			CString _strValue = (wchar_t*)sqlite3_column_text16(st,1);

			type[_strKey] = _strValue;
		}

	} while (ret==SQLITE_ROW);

	ret=sqlite3_finalize(st);

	return 0;
}

int CSoftManager::GetCategroy(GetInfoCallback func ,void* pParam)
{
	CAtlMap<CString, CString> _mapType;
	GetCategroy(_mapType);

	for ( POSITION _pPos = _mapType.GetStartPosition(); _pPos; )
	{
		CString key=_mapType.GetKeyAt(_pPos);
		CString val=_mapType.GetValueAt(_pPos);
		func(pParam, key.GetBuffer(), val.GetBuffer() );
		_mapType.GetNext(_pPos);
	}

	return 0;
}

void* CSoftManager::GetLast50Soft()
{
	CString sql=L"select * from soft join catalog on typeid=id order by lastedittime desc";
	int ret=0;
	const void *zLeftover; 
	sqlite3_stmt* st=NULL;
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	return st;
}

void CSoftManager::__GetDel(CString act,CString type, TiXmlElement* pRt, CAtlMap<CString,TiXmlElement*> &ids )
{
	for (TiXmlElement* pE=pRt->FirstChildElement(CW2A(act));pE;pE=pE->NextSiblingElement())
	{
		for (TiXmlElement* pDel_soft=pE->FirstChildElement(CW2A(type));pDel_soft;pDel_soft=pDel_soft->NextSiblingElement())
		{
			const char* id=pDel_soft->GetText();
			CString a=id;
			if(id)
			{
				wchar_t* del;
				wchar_t* tok=wcstok_s(a.GetBuffer(),L",",&del);
				while(tok)
				{
					ids.SetAt(tok,NULL);
					tok=wcstok_s(NULL,L",",&del);
				}
			}
		}
	}

}

void CSoftManager::__merge( TiXmlElement* el, CString att, CAtlMap<CString,TiXmlElement*>* filter, TiXmlElement* re )
{
	for (TiXmlElement* pNewItem=el->FirstChildElement();pNewItem;pNewItem=pNewItem->NextSiblingElement())
	{
		
		const char* patt=pNewItem->Attribute(CW2A(att));
		if(patt&&filter->Lookup(CA2W(patt))==false)
		{
			re->LinkEndChild(pNewItem->Clone());
		}
	}
}

bool CSoftManager::has_cache()
{
	if(PathFileExists(_cache_name))
	{
		CDataFileLoader	loader;
		BkDatLibHeader lib_h;
		BkDatLibHeader rank_h;
		BkDatLibHeader necess_h;
		BkDatLibHeader autoinst_h;
		if(!loader.GetLibDatHeader(_libfile,lib_h) ||
		   !loader.GetLibDatHeader(_rankfile,rank_h) ||
		   !loader.GetLibDatHeader(_necessLibFile,necess_h) ||
		   !loader.GetLibDatHeader(_autoinstLibFile,autoinst_h) )
			return false;

		int ret=0;
		const void *zLeftover; 
		BkDatLibHeader* ph=NULL;
		BkDatLibHeader* ph2=NULL;
		BkDatLibHeader* phNecess = NULL;
		BkDatLibHeader* phAutoInst = NULL;
		sqlite3_stmt* st=NULL;
		CString sql=L"select lib,rank,necess,autoinst from header_t;";
		ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
		ret=sqlite3_step(st);
		ph=(BkDatLibHeader *)sqlite3_column_blob(st,0);
		ph2=(BkDatLibHeader *)sqlite3_column_blob(st,1);
		phNecess=(BkDatLibHeader*)sqlite3_column_blob(st,2);
		phAutoInst=(BkDatLibHeader*)sqlite3_column_blob(st,3);
		if(ph==NULL||ph2==NULL||phNecess==NULL||phAutoInst==NULL)
			return false;
		if(ph->llVersion.QuadPart==lib_h.llVersion.QuadPart&&
		   ph2->llVersion.QuadPart==rank_h.llVersion.QuadPart&&
		   phNecess->llVersion.QuadPart==necess_h.llVersion.QuadPart&&
		   phAutoInst->llVersion.QuadPart==autoinst_h.llVersion.QuadPart)
		{
			ret=sqlite3_finalize(st);
			return true;
		}
		else if(ph->llVersion.QuadPart!=lib_h.llVersion.QuadPart ||
			    phNecess->llVersion.QuadPart != necess_h.llVersion.QuadPart ||
				phAutoInst->llVersion.QuadPart != autoinst_h.llVersion.QuadPart )
		{
			ret=sqlite3_finalize(st);
			FreeSoftDB();
			__loadSoftdb(_libfile,FALSE);
			__loadRank(_rankfile,FALSE);
			update_cache();
			return true;
		}
		else if(ph2->llVersion.QuadPart!=rank_h.llVersion.QuadPart)
		{
			ret=sqlite3_finalize(st);
			FreeRank(L"");
			__loadRank(_rankfile,FALSE);
			update_cache();
			return true;
		}
		
	}
	return false;
}

void CSoftManager::update_cache()
{
	CDataFileLoader	loader;
	BkDatLibHeader lib_h;
	BkDatLibHeader rank_h;
	BkDatLibHeader necess_h;
	BkDatLibHeader autoinst_h;
	if(!loader.GetLibDatHeader(_libfile,lib_h) ||
	   !loader.GetLibDatHeader(_rankfile,rank_h) ||
	   !loader.GetLibDatHeader(_necessLibFile,necess_h) ||
	   !loader.GetLibDatHeader(_autoinstLibFile,autoinst_h) )
		return;

	int ret=0;
	sql_run(L"drop table header_t");
	sql_run(L"create table header_t(lib,rank,necess,autoinst)");
	const void *zLeftover; 
	BkDatLibHeader* ph=NULL;
	sqlite3_stmt* st=NULL;
	CString sql=L"insert into header_t values (?,?,?,?)";
	ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
	ret=sqlite3_bind_blob(st,1,&lib_h,sizeof lib_h,SQLITE_STATIC);
	ret=sqlite3_bind_blob(st,2,&rank_h,sizeof rank_h,SQLITE_STATIC);
	ret=sqlite3_bind_blob(st,3,&necess_h,sizeof necess_h,SQLITE_STATIC);
	ret=sqlite3_bind_blob(st,4,&autoinst_h,sizeof autoinst_h,SQLITE_STATIC);
	ret=sqlite3_step(st);
	ret=sqlite3_finalize(st);
}

int CSoftManager::__loadSoftdb( CString lib_file, BOOL bCheckCache )
{
	::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 

	OutputDebugString(L"LoadSoftDB");
	CDataFileLoader	loader;
	BkDatLibHeader new_h;

	if(loader.GetLibDatHeader(lib_file,new_h)==FALSE)
		return 1;
	_libfile=lib_file;
	if(bCheckCache && has_cache())
		return 0;

	TiXmlDocument plugins;
	BkDatLibContent cont;

	if(loader.GetLibDatContent(lib_file,cont)==FALSE)
		return 2;
	if(false==plugins.Parse((char*)cont.pBuffer))
		return 3;

	LoadNecessDat(_necessLibFile);

	TiXmlHandle hDoc(&plugins);
	TiXmlElement* pElem;
	TiXmlHandle hroot(NULL);
	pElem=hDoc.FirstChildElement().Element();
	hroot=TiXmlHandle(pElem);
	pElem=hroot.FirstChildElement("need").Element();
	for(pElem;pElem;pElem=pElem->NextSiblingElement("need"))
	{
		for(TiXmlElement* psoft=pElem->FirstChildElement("softid");psoft;psoft=psoft->NextSiblingElement("softid"))
		{
			const char* att=psoft->Attribute("name");
			if(att==NULL)
				continue;
			CString et=psoft->GetText();
			et.Replace(L",",L"\",\"");
			_sln[CA2W(att)]=L"\""+et+L"\"";
		}
	}


	pElem=hroot.FirstChildElement("sample").Element();
	for(pElem;pElem;pElem=pElem->NextSiblingElement("sample"))
	{
		for(TiXmlElement* pE_ps=pElem->FirstChildElement();pE_ps;pE_ps=pE_ps->NextSiblingElement())
		{
			const char* vvv=pE_ps->Value();
			const char* att=pE_ps->Attribute("tname");
			const char* cld=pE_ps->Attribute("child");

			if(att==NULL||cld==NULL)
				continue;

			CAtlArray<CString> atts;
			for(TiXmlElement* pE_att=pE_ps->FirstChildElement("att");pE_att;pE_att=pE_att->NextSiblingElement("att"))
			{
				const char* txt=pE_att->GetText();
				if(txt)
					atts.Add(CA2W(txt));
			}
			CAtlArray<CString> pros;
			for(TiXmlElement* pE_att=pE_ps->FirstChildElement("pro");pE_att;pE_att=pE_att->NextSiblingElement("pro"))
			{
				const char* txt=pE_att->GetText();
				if(txt)
					pros.Add(CA2W(txt));
			}

			//建表啦！！！
			sql_run(L"BEGIN;");

			CString drop=L"DROP TABLE ";
			drop+=att;
			sql_run(drop);

			CString sql=L"CREATE TABLE ";
			sql+=att;
			CString	strCreateStr;
			strCreateStr+=L"(";
			for (size_t i=0;i<atts.GetCount();i++)
			{
				if(i!=0)
					strCreateStr+=L",";
				strCreateStr+=atts[i];
			}
			for (size_t i=atts.GetCount();i-atts.GetCount()<pros.GetCount();i++)
			{
				if(i!=0)
					strCreateStr+=L",";
				strCreateStr+=pros[i-atts.GetCount()];
			}
			strCreateStr+=L")";

			sql += strCreateStr;
			sql_run(sql);

			//处理这个表！ 

			// 增加了装机必备的表，提高必备的加载速度
			BOOL		bIsSoftTable = FALSE;
			CString		strTableNecess = L"necess";
			CString		strSqlNeecess;
			if ( vvv != NULL && stricmp(vvv, "softwares") == 0 )
			{
				bIsSoftTable = TRUE;

				CString drop=L"DROP TABLE ";
				drop += strTableNecess;
				sql_run(drop);

				strSqlNeecess = L"CREATE TABLE ";
				strSqlNeecess += strTableNecess;
				strSqlNeecess += strCreateStr;

				sql_run(strSqlNeecess);
			}

			for(TiXmlElement* pE_t=hroot.FirstChildElement(vvv).Element();pE_t;pE_t=pE_t->NextSiblingElement(vvv))
			{
				CString name;
				CString value;
				CString sql_ins;
				for(TiXmlElement* pE_t_i=pE_t->FirstChildElement(cld);pE_t_i;pE_t_i=pE_t_i->NextSiblingElement(cld))
				{
					CString		strId;

					name=att;
					name+=L"(";
					value=L"values (";
					for (size_t i=0;i<atts.GetCount();i++)
					{
						const char* item=pE_t_i->Attribute(CW2A(atts[i]));
						if(item)
						{
							AppendNV(name,value,CW2A(atts[i]),item,i==0?"":",");

							if ( bIsSoftTable && wcsicmp(atts[i], L"softid") == 0)
							{
								strId = item;
							}
						}
					}
					for (size_t i=atts.GetCount();i-atts.GetCount()<pros.GetCount();i++)
					{
						TiXmlElement* pf=pE_t_i->FirstChildElement(CW2A(pros[i-atts.GetCount()]));
						if(pf==NULL)
							continue;
						const char* item=pf->GetText();
						if(item)
						{
							AppendNV(name,value,CW2A(pros[i-atts.GetCount()]),item,i==0?"":",");
						}
					}

					name+=L")";
					value+=L")";
					sql_ins=L"insert into "+name +L" " +value;
					sql_run(sql_ins);

					if (bIsSoftTable && m_ncessIdMap.Lookup(strId) != NULL)
					{
						strSqlNeecess = L"insert into "+strTableNecess +L" " +value;
						sql_run(strSqlNeecess);
					}
				}
			}
			sql_run(L"COMMIT;");

		}
	}

	sql_run(L"create index neindex on necess(softid);");
	sql_run(L"create index ciindex on soft(softid,uninstallname);");
	sql_run(L"create index scindex on soft(typeid);");
	sql_run(L"create index ciindex on catalog(id);");

	return 0;
}

CString CSoftManager::__loadRank( CString lib_file, BOOL bCheckCache )
{
	::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 

	CString ret;
	CDataFileLoader	loader;
	BkDatLibHeader new_h;
	if(loader.GetLibDatHeader(lib_file,new_h)==FALSE)
		return L"";
	_rankfile=lib_file;
	if(bCheckCache && has_cache())
		return L"";
	TiXmlDocument plugins;
	BkDatLibContent cont;

	if(loader.GetLibDatContent(lib_file,cont)==FALSE)
		return L"";
	if(false==plugins.Parse((char*)cont.pBuffer))
		return L"";



	TiXmlHandle hDoc(&plugins);
	TiXmlElement* pElem;
	TiXmlHandle hroot(NULL);
	pElem=hDoc.FirstChildElement().Element();
	hroot=TiXmlHandle(pElem);
	pElem=hroot.FirstChildElement("sample").Element();

	CAtlArray<CString> rk_a;

	for(pElem=pElem->FirstChildElement("rank");pElem;pElem=pElem->NextSiblingElement("rank"))
	{
		const char* rk_name=pElem->GetText();
		const char* def=pElem->Attribute("default");
		if(rk_name)
		{
			rk_a.Add(CA2W(rk_name));
		}
		if(def)
			ret=rk_name;
	}

	CString drop=L"DROP TABLE ";
	drop+=L"rank";
	sql_run(drop);

	CString sql=L"CREATE TABLE ";
	sql+=L"rank";
	sql+=L"(sfid";
	for (size_t i=0;i<rk_a.GetCount();i++)
	{
		sql+=L",";
		sql+=rk_a[i];
		sql+=L" default null";
	}
	sql+=L")";

	sql_run(sql);

	sql=L"CREATE INDEX ";
	sql+=L"ranki on rank";
	sql+=L"(sfid";
	for (size_t i=0;i<rk_a.GetCount();i++)
	{
		sql+=L",";
		sql+=rk_a[i];
		sql+=L" DESC";
	}
	sql+=L")";

	sql_run(sql);

	sql_run(L"BEGIN;");
	for (size_t i=0;i<rk_a.GetCount();i++)
	{
		pElem=hroot.FirstChildElement(CW2A(rk_a[i])).Element();
		for(pElem;pElem;pElem=pElem->NextSiblingElement(CW2A(rk_a[i])))
		{
			const char* id=pElem->Attribute("i");
			const char* val=pElem->Attribute("v");
			if(id==NULL||val==NULL)
				continue;

			//sql=L"update or replace rank  set "+rk_a[i]+L"="+CA2W(val)+L" where sfid =\""+CA2W(id)+L"\"";
			sql=L"insert into rank(sfid,"+rk_a[i]+L") values (\""+CA2W(id)+L"\","+CA2W(val)+L")";
			sql_run(sql);
		}
	}
	sql_run(L"COMMIT;");
	return ret;
}

template<class T>
VOID AddInArray( CSimpleArray<T>& arr, T& ele)
{
	BOOL			bAddIn = FALSE;
	CSimpleArray<T> newArr;
	for ( int i=0; i < arr.GetSize(); )
	{
		if ( !bAddIn && ele < arr[i] )
		{
			bAddIn = TRUE;
			newArr.Add(ele);
		}
		else
		{
			newArr.Add(arr[i]);
			i++;
		}
	}
	if (!bAddIn)
	{
		newArr.Add(ele);
	}
	arr.RemoveAll();
	arr = newArr;
}

BOOL CSoftManager::LoadNecessDat( LPCTSTR lpstrLibFile )
{
	::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 

	CDataFileLoader	loader;
	BkDatLibContent cont;
	TiXmlDocument xmlDoc;

	// 已经加载过，不再加载
	if (m_necessGroup.GetSize() > 0 )
		return TRUE;

	if( loader.GetLibDatContent(lpstrLibFile,cont) )
	{
		if ( cont.header.nFormatVer != 1 )
			return FALSE;

		xmlDoc.Parse((char*)cont.pBuffer);

		if ( xmlDoc.Error() )
			return FALSE;
	}
#if 1
	else
	{
		if ( !xmlDoc.LoadFile( CW2A(lpstrLibFile) ) )
			return FALSE;
	}

#endif

	_necessLibFile = lpstrLibFile;

	TiXmlHandle hDoc(&xmlDoc);
	TiXmlElement* pElem;
	TiXmlHandle hroot(NULL);
	const char*		pAttrib = NULL;
	pElem=hDoc.FirstChildElement().Element();
	hroot=TiXmlHandle(pElem);

	for ( TiXmlElement* pGroup = hroot.FirstChildElement("group").Element() ; pGroup; pGroup = pGroup->NextSiblingElement("group") )
	{
		NECESS_GROUP	negroup;
		
		negroup.strGroupName = pGroup->Attribute("name");
		pAttrib				 = pGroup->Attribute("order");
		if (pAttrib)
			negroup.nSortIndex = atoi(pAttrib);

		for ( TiXmlElement* pType = pGroup->FirstChildElement("type"); pType; pType = pType->NextSiblingElement("type") )
		{
			NECESS_TYPE	neType;

			neType.strTypeName = pType->Attribute("name");
			pAttrib			   = pType->Attribute("order");
			if (pAttrib)
				neType.nSortIndex = atoi(pAttrib);

			for ( TiXmlElement* pSoft = pType->FirstChildElement("soft"); pSoft; pSoft = pSoft->NextSiblingElement("soft") )
			{
				NECESS_SOFT	neSoft;

				neSoft.strSoftID   = pSoft->Attribute("id");
				pAttrib			   = pSoft->Attribute("order");
				if (pAttrib)
					neSoft.nSortIndex = atoi(pAttrib);


				AddInArray<NECESS_SOFT>(neType.softList,neSoft);

				m_ncessIdMap[neSoft.strSoftID] = neSoft.strSoftID;
			//	neType.softList.Add(neSoft);
			}

			//negroup.typeList.Add(neType);
			AddInArray<NECESS_TYPE>(negroup.typeList,neType);
		}

		//m_necessGroup.Add(negroup);
		AddInArray<NECESS_GROUP>(m_necessGroup,negroup);
	}

	if (TRUE)
	{
		TiXmlHandle hOneKeyElem = hroot.FirstChildElement("onekey");
		for ( TiXmlElement* pGroup = hOneKeyElem.FirstChildElement("group").Element() ; pGroup; pGroup = pGroup->NextSiblingElement("group") )
		{
			ONE_KEY_GROUP	negroup;
			negroup.strGroupName = pGroup->Attribute("name");
			pAttrib				 = pGroup->Attribute("order");
			if (pAttrib)
				negroup.nSortIndex = atoi(pAttrib);
			for ( TiXmlElement* pType = pGroup->FirstChildElement("type"); pType; pType = pType->NextSiblingElement("type") )
			{
				ONE_KEY_TYPE	neType;
				neType.strTypeName = pType->Attribute("name");
				pAttrib			   = pType->Attribute("order");
				if (pAttrib)
					neType.nSortIndex = atoi(pAttrib);
				for ( TiXmlElement* pSoft = pType->FirstChildElement("soft"); pSoft; pSoft = pSoft->NextSiblingElement("soft") )
				{
					ONE_KEY_SOFT	neSoft;

					neSoft.strShowName = pSoft->Attribute("name");
					neSoft.strSoftID   = pSoft->Attribute("id");
					pAttrib			   = pSoft->Attribute("order");
					if (pAttrib)
						neSoft.nSortIndex = atoi(pAttrib);
					pAttrib				= pSoft->Attribute("check");
					if (pAttrib)
						neSoft.bCheck	= atoi(pAttrib);
					AddInArray<ONE_KEY_SOFT>(neType.softList,neSoft);
					m_ncessIdMap[neSoft.strSoftID] = neSoft.strSoftID;
				}
				AddInArray<ONE_KEY_TYPE>(negroup.typeList,neType);
			}
			AddInArray<ONE_KEY_GROUP>(m_oneKeyGroup,negroup);
		}
	}
	LoadOnekeyDat(_autoinstLibFile);
	return m_necessGroup.GetSize()>0;
}
BOOL CSoftManager::LoadOnekeyDat( LPCTSTR lpstrLibFile )
{
	::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 
	CDataFileLoader	loader;
	BkDatLibContent cont;
	TiXmlDocument xmlDoc;
	CString			strLib = lpstrLibFile;
	if (strLib.IsEmpty())
		strLib = _autoinstLibFile;
	if( loader.GetLibDatContent(strLib,cont) )
	{
		if ( cont.header.nFormatVer != 1 )
			return FALSE;
		xmlDoc.Parse((char*)cont.pBuffer);
		if ( xmlDoc.Error() )
			return FALSE;
	}
#if 1
	else
	{
		if ( !xmlDoc.LoadFile( CW2A(strLib) ) )
			return FALSE;
	}
#endif
	_autoinstLibFile = strLib;
	CSimpleArray<ONE_KEY_GROUP>	onekeyGroup;
	TiXmlHandle hDoc(&xmlDoc);
	TiXmlElement* pElem;
	TiXmlHandle hroot(NULL);
	const char*		pAttrib = NULL;
	pElem=hDoc.FirstChildElement().Element();
	hroot=TiXmlHandle(pElem);
	if (TRUE)
	{
		TiXmlHandle hOneKeyElem = hroot.FirstChildElement("onekey");
		for ( TiXmlElement* pGroup = hOneKeyElem.FirstChildElement("group").Element() ; pGroup; pGroup = pGroup->NextSiblingElement("group") )
		{
			ONE_KEY_GROUP	negroup;
			negroup.strGroupName = pGroup->Attribute("name");
			pAttrib				 = pGroup->Attribute("order");
			if (pAttrib)
				negroup.nSortIndex = atoi(pAttrib);
			for ( TiXmlElement* pType = pGroup->FirstChildElement("type"); pType; pType = pType->NextSiblingElement("type") )
			{
				ONE_KEY_TYPE	neType;
				neType.strTypeName = pType->Attribute("name");
				pAttrib			   = pType->Attribute("order");
				if (pAttrib)
					neType.nSortIndex = atoi(pAttrib);
				for ( TiXmlElement* pSoft = pType->FirstChildElement("soft"); pSoft; pSoft = pSoft->NextSiblingElement("soft") )
				{
					ONE_KEY_SOFT	neSoft;
					neSoft.strShowName = pSoft->Attribute("name");
					neSoft.strSoftID   = pSoft->Attribute("id");
					pAttrib			   = pSoft->Attribute("order");
					if (pAttrib)
						neSoft.nSortIndex = atoi(pAttrib);
					pAttrib				= pSoft->Attribute("check");
					if (pAttrib)
						neSoft.bCheck	= atoi(pAttrib);
					AddInArray<ONE_KEY_SOFT>(neType.softList,neSoft);
					m_ncessIdMap[neSoft.strSoftID] = neSoft.strSoftID;
				}
				AddInArray<ONE_KEY_TYPE>(negroup.typeList,neType);
			}
			AddInArray<ONE_KEY_GROUP>(onekeyGroup,negroup);
		}
		if (onekeyGroup.GetSize() > 0)
		{
			m_oneKeyGroup.RemoveAll();
			m_oneKeyGroup = onekeyGroup;
		}
	}
	return (onekeyGroup.GetSize()>0);
}

BOOL CSoftManager::GetNecessSoftInfo( CSimpleArray<NECESS_GROUP>** ppInfo )
{
	if ( ppInfo == NULL )
		return FALSE;

	*ppInfo = &m_necessGroup;
//	*ppInfo = NULL;
	return TRUE;
}

VOID CSoftManager::FreeNecessInfo()
{
	m_necessGroup.RemoveAll();
	m_ncessIdMap.RemoveAll();
	m_oneKeyGroup.RemoveAll();
}

void CSoftManager::UpdateCache()
{
	if ( _CheckCacheOutTime() )
		update_cache();
}

BOOL CSoftManager::IsLibUpdate()
{
	return _CheckCacheOutTime();
}

BOOL CSoftManager::_CheckCacheOutTime()
{
	if(PathFileExists(_cache_name))
	{
		CDataFileLoader	loader;
		BkDatLibHeader lib_h;
		BkDatLibHeader rank_h;
		BkDatLibHeader necess_h;
		BkDatLibHeader autoinst_h;
		if( !loader.GetLibDatHeader(_libfile,lib_h) ||
			!loader.GetLibDatHeader(_rankfile,rank_h) ||
			!loader.GetLibDatHeader(_necessLibFile,necess_h) ||
			!loader.GetLibDatHeader(_autoinstLibFile,autoinst_h) )
			return false;

		int ret=0;
		const void *zLeftover; 
		BkDatLibHeader* ph=NULL;
		BkDatLibHeader* ph2=NULL;
		BkDatLibHeader* phNecess = NULL;
		BkDatLibHeader* phAutoInst = NULL;
		sqlite3_stmt* st=NULL;
		CString sql=L"select lib,rank,necess,autoinst from header_t;";
		ret=sqlite3_prepare16_v2(_db,sql.GetBuffer(),-1,&st,&zLeftover);
		ret=sqlite3_step(st);
		ph=(BkDatLibHeader *)sqlite3_column_blob(st,0);
		ph2=(BkDatLibHeader *)sqlite3_column_blob(st,1);
		phNecess=(BkDatLibHeader*)sqlite3_column_blob(st,2);
		phAutoInst=(BkDatLibHeader*)sqlite3_column_blob(st,3);
		if(ph==NULL||ph2==NULL||phNecess==NULL||phAutoInst==NULL)
			return TRUE;

		ret=sqlite3_finalize(st);

		if(ph->llVersion.QuadPart==lib_h.llVersion.QuadPart&&
			ph2->llVersion.QuadPart==rank_h.llVersion.QuadPart&&
			phNecess->llVersion.QuadPart==necess_h.llVersion.QuadPart &&
			phAutoInst->llVersion.QuadPart==autoinst_h.llVersion.QuadPart )
		{
			return FALSE;
		}
		else
			return TRUE;
	}
	else
		return TRUE;
}

BOOL CSoftManager::GetOneKeySoftInfo( CSimpleArray<ONE_KEY_GROUP>** ppInfo )
{
	if ( ppInfo )
		*ppInfo = NULL;
	if (m_oneKeyGroup.GetSize() == 0)
		return FALSE;
	if ( ppInfo==NULL )
		return FALSE;
	*ppInfo = &m_oneKeyGroup;
	return TRUE;
}

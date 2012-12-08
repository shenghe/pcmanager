/**
* @file    SoftChecker
* @brief   用于检测软件安装卸载，并提供强力清除
* @author  liveck
* @date    2010/04/07 16:02
*/
#pragma once

#include <atlstr.h>
#include <atlcoll.h>
#include <Tlhelp32.h>
#include <softmgr/ISoftChecker.h>
#include <sqlite3/sqlite3.h>
#include <tinyxml/tinyxml.h>
#include <map>



struct Soft;

struct Sign
{
	Soft* soft;
	CString func;
	CAtlArray<CString> arg;
};

struct Soft
{
	CString id;
	CString ver;
	CString name;
	CString last_update;
	CString m_strCurVer;
	CString m_strSize;
	CString m_strMainExe;

	CAtlArray<Sign*> signs;
	
	virtual ~Soft()
	{		
		for (size_t i=0;i<signs.GetCount();i++)
		{
			delete signs[i];
		}
	}
};


typedef std::map<CString, Soft*>	t_mapUpdateSoft;
typedef t_mapUpdateSoft::iterator	t_iterUpdateSoft;

//管理升级项
struct UpdMgr
{
	void AddSoft(Soft* pUpdSoft)
	{
		if ( !pUpdSoft ) return;
		m_mapUpdSoft.insert(std::pair<CString, Soft*>(pUpdSoft->id, pUpdSoft));
	}

	void RemoveSoft(CString strID)
	{
		t_iterUpdateSoft _iterFind = m_mapUpdSoft.find(strID);
		if ( _iterFind != m_mapUpdSoft.end() )
			m_mapUpdSoft.erase(_iterFind);
	}

	const t_mapUpdateSoft& GetUpdMgr() const
	{
		return m_mapUpdSoft;
	}

	UpdMgr()
	{
		m_mapUpdSoft.clear();
	}

	virtual ~UpdMgr()
	{
		m_mapUpdSoft.clear();
	}

private:
	t_mapUpdateSoft	m_mapUpdSoft;
};



//__declspec(uuid("C431B499-65C1-4bc3-A91C-1EA2927DB7E2")) 

class CSoftChecker:public ISoftCheckerEx
{
public:
	CSoftChecker(void);
	virtual ~CSoftChecker(void);

	///检测机器上已安装所有的软件
	virtual int CheckAllInstalled(InsCheckCallBack func,void* para);

	///检测特定的软件是否安装
	virtual int CheckOneInstalled(CString id,InsCheckCallBack func,void* para);

	virtual int CheckOneInstalled(CString& loc,CString id,InsCheckCallBack func,void* para,bool once=false);

	///检测卸载项
	virtual int CheckAll2Uninstall(UniCheckCallBack func,void* para);
	virtual int CheckAll2UninstallEx(UniCheckCallBack func,void* para);
	
	///检测指定的卸载项是否存在
	virtual int CheckOne2Uninstall(CString key,UniCheckCallBack func,void* para);

	virtual const wchar_t* FindUninstallId(CString key);
	virtual const wchar_t* FindUninstallIdByName( CString name );

	///正常卸载
	virtual int Uninstall(CString cmd);
	///安装
	virtual int Install(CString cmd);

	///强力清扫
	virtual int PowerSweep(CString key,CString loc,PowerSweepCallBack func, void* para, PowerSweepFilterCallBack fnFilter);


	///用于差量更新
	virtual int Combine(CString diff_file);

	///加载库，检测最后修改时间，如果不同，说明有变化，则重新加载，否则直接返回

	virtual int Load(CString lib_file="",bool reload=true);

	virtual size_t GetSoftCount();

	//Ex的内容~~

	virtual int CheckAll2Uninstall(UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;

	virtual int CheckOne2Uninstall(CString key ,UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;

	virtual int CheckAll2UninstallByType(SoftType tp,UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;

	virtual void* SearchUninstallItem(CString word) ;
	virtual wchar_t* GetNextItem(void* pos) ;
	virtual void FinalizeGet(void* pos) ;
	virtual int LoadUnInfo(CString lib_file) ;
	virtual int LoadFonts(CString strFile);
	virtual int FreeUnInfo() ;
	virtual int Combine_UniInfo( CString diff_file );


	//清理快捷方式
	virtual int RemoveLnk(SoftType tp,CString loc,PowerSweepCallBack func,void* para) ;

	void __RemoveLnk( CString &pa, CString &loc, PowerSweepCallBack func, void* para );
	//清理残留项
	virtual int RemoveRemainds(CString name,CString ins_loc,CString type,CString loc,PowerSweepCallBack func,void* para) ;

	//计算软件大小
	virtual __int64 CountSize(CString loc) ;

	//检测所有的残留项……
	virtual int CheckAll2Remains(UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;

	virtual int CheckAll2RemainsByGroup(UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;

	virtual int CheckAll2RemainsByNameAndLoc(CString name,CString loc,UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;



	//从缓存中取得信息
	virtual int CheckAll2UninstallCache(UniCheckCallBackEx func,GetInfoCallback cbfun,void* mp,void* para) ;

	//建立缓存
	virtual int MakeAll2UninstallCache();

	//获取升级缓存
	virtual int GetUpdateFromCache(pfnUpdateCheckCallBack pfnCallBack, GetInfoCallback cbfun, void* mp, void* para);

	//建立升级缓存, 界面做了异步检测处理, 这里就不再做异步保存了, 但结构不好
	virtual int MakeUpdateCache();

private:
	virtual void __update_ps(CString table_n);
	virtual void __update_startmenu(CString table_n);
	virtual void __update_desktop(CString table_n);
	virtual void __update_ql(CString table_n);
	virtual void __update_lnk(CString table_n,CString dir);
	virtual void __update_notifyicon(CString table_n);

public:
	virtual CString __get_lnk_path(CString lnk);
	virtual void __update_lastuse();
private:
	virtual bool __is_lnk_changed(CString lnk,struct _stat* sta);
	virtual bool __is_lnk_remains(CString lnk);
	virtual bool __update_remains(CString type,CString path,CString loc,CString name);
	virtual bool __remove_remains(CString type,CString path);
	virtual void __update_size(CString loc,__int64 size);

	CString _get_realname( Soft* pSoft );
	


private:

	int CheckAll2UninstallByHK(HKEY parent,UniCheckCallBack func,void* para);
	int CheckAll2UninstallByHKEx(HKEY parent,UniCheckCallBack func,void* para);
	int CheckOne2UninstallByHKEx( HKEY parent,CString key2c,UniCheckCallBack func,void* para );
	int CheckOne2UninstallByHK( HKEY parent,CString key ,UniCheckCallBack func,void* para );

	enum FIND_REG_FLAG
	{
		FIND_VALUE		= 0x0000001,
		FIND_VALUE_NAME	= 0x0000002,
		FIND_KEY		= 0x0000004,	
		FIND_SUB_KEY	= 0x0000008,
		FIND_EQU		= 0x0000010,
	};

	int FindReg(HKEY parent,CString key,CString value,DWORD flag,CAtlList<CString>& result);

	CString uuid2hex(CString uuid)
	{
		uuid.MakeUpper();
		uuid=uuid.Mid(1,uuid.GetLength()-2);
		UUID uid;
		UuidFromString((RPC_WSTR)uuid.GetBuffer(),&uid);
		BYTE* da=(BYTE*)&uid;
		wchar_t* st=uuid.GetBuffer(sizeof uid*2);
		for (int i=0;i<sizeof uid;i++)
		{
			int hi=da[i]&0x0f;
			int lo=(da[i]&0xf0)>>4;
			st[i*2]= hi<=9?'0'+hi:'A'+hi-10;
			st[i*2+1]= lo<=9?'0'+lo:'A'+lo-10;
		}
		CString ret=st;
		return ret.Left(sizeof uid*2);
	}
	CString hex2uuid(CString hex)
	{
		hex.MakeUpper();
		UUID uid;
		BYTE* da=(BYTE*)&uid;
		for (int i=0;i<hex.GetLength()-1;i+=2)
		{
			da[i/2]=hex[i+1]<=L'9'?hex[i+1]-L'0':hex[i+1]-L'A'+10;
			da[i/2]<<=4;
			da[i/2]+=hex[i]<=L'9'?hex[i]-L'0':(hex[i]-L'A'+10);
		}
		CString ret;
		RPC_WSTR s;
		UuidToString(&uid,&s);
		ret=(wchar_t*)s;
		RpcStringFree(&s);
		return L"{"+ret+L"}";
	}

	bool IsSysDir(CString dir);

	void build_snap()
	{
		__snap.RemoveAll();
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof( PROCESSENTRY32 );

		HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

		while( !Process32First( snap, &pe32 ) )
		{
			CloseHandle( snap ); 
			snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
			Sleep(550);
		}
		do
		{
			__snap.SetAt(pe32.th32ProcessID,0);
		} while( Process32Next( snap, &pe32 ) );

		CloseHandle(snap);
	}

	int __PowerSweep(CString type,CString path,PowerSweepCallBack func,void* para,int& fund,CString ins_loc=L"");
	size_t __Find(CAtlArray<Sign*>&ar,Sign* sgn);
	CString __FindInstLoc(CString key);
	bool __hasAVP();
	
	time_t _last_modify;

	CAtlMap<DWORD,DWORD> __snap;

	CAtlMap<CString,Soft*> _softs;
	CAtlArray<Sign*> _signs;

	CAtlMap<CString,CString> __temp;
	CAtlMap<CString,CString> __temp_keys;
	CAtlMap<CString,CString> __temp_names;
	CAtlMap<CString,CString> __key2id;
	CAtlMap<CString,CString> __dname2id;
	CAtlList<CString> __dir;

	UpdMgr	m_stUpdateMgr;

	CString _libfile;
	LARGE_INTEGER _ver;

	//cache相关的内容
	CRITICAL_SECTION m_csUninCache;

	int sql_run(CString& sql);
	int sql_run(const wchar_t* sql)
	{
		CString ssql=sql;
		return sql_run(ssql);
	}

	int sql_run_upd(CString& sql);
	int sql_run_upd(const wchar_t* sql)
	{
		CString ssql=sql;
		return sql_run_upd(ssql);
	}

	struct Transaction 
	{
		Transaction(CSoftChecker* p):_p(p)
		{
			_p->sql_run(L"BEGIN;");
		}
		~Transaction()
		{
			_p->sql_run(L"COMMIT;");
		}
		CSoftChecker* _p;
	};

	int init_cache();
	int _InitUpdateCache();

	bool IsKeyCached(HKEY parent,CString sub,FILETIME& t);
	void UpdateKeyCache( HKEY parent,CString sub,FILETIME& t );
	
	sqlite3* _cache;		// 卸载缓存ksoft_ucache_4 
	CString _cache_name;	// 卸载缓存名
	CString _font;

	sqlite3*	m_pUpdateDB;	//升级缓存数据库
	CString m_strUpdateCache;	//升级缓存名

	struct UniHook
	{
		CSoftChecker* theOne;
		CString cache_name;
		UniCheckCallBack func;
		void* para;
	};
	static int UninstallHook(CString key,CString name,CString disp_icon,CString loc,CString uni_cmd,CString parent,void* param);
	int CheckAll2UninstallExByCache( UniCheckCallBack func,void* para );

	int CheckAll2UninstallByHKByCache(HKEY parent,UniCheckCallBack func,void* para);

	int CheckAll2UnistallByCache(UniHook* para );
	void SetItemStat(CString sta,CString name);
	void RemoveCache(CString& key);

	bool has_cache();
	int AppendNV( CString& name,CString& val,const char* name_key,const char* name_att ,CString sep);
	void update_cache();
	static CString RemoveVersionEtc(CString& disp_name);
	void __GetDel(CString act,CString type, TiXmlElement* pRt, CAtlMap<CString,TiXmlElement*> &ids );
	void __merge( TiXmlElement* el, CString att, CAtlMap<CString,TiXmlElement*>* filter, TiXmlElement* re );
	
	CString _uni_lib;		// SoftUninst.dat
};





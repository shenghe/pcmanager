#pragma once
#include <vector>
#include <string>

class SevenZipModule;
class WcxModule;
struct T_UserPatchInfo;
typedef std::vector<std::string> strings;

enum EPatcherCode
{
	KPATCHER_OK = 0,
	KPATCHER_ERR_GENERAL,		//
	KPATCHER_ERR_DOWNLOADFILE,	// 从服务器拷贝数据出错
	KPATCHER_ERR_LOADXML,	// 从服务器拷贝数据出错

	KPATCHER_ERR_XMLFILES = 0x10,
	KPATCHER_ERR_EXDLL,			// 加载7z, msi.dll 出错 
	KPATCHER_ERR_EXTRACT,		// 解包出错 
	KPATCHER_ERR_FILEBUSY,		// 目的文件忙, 无法拷贝 
	KPATCHER_ERR_FILENOTMATCH,	// 文件不匹配 
	KPATCHER_ERR_REPLACE,		// 文件拷贝的时候出错 
};

class IExeUnpack
{
public:
	virtual bool Initialize( LPCTSTR sz7z, LPCTSTR szwcx ) = 0;
	virtual bool Extract(LPCTSTR szfile, LPCTSTR szpath, strings &files) = 0;
	virtual bool IsValid() const = 0;
};

class CExeUnpackPkg : public IExeUnpack
{
public:
	CExeUnpackPkg(){}
	~CExeUnpackPkg(){}
	bool Initialize(LPCTSTR sz7z, LPCTSTR szwcx )
	{
		return true;
	}
	bool IsValid() const;
	bool Extract(LPCTSTR szfile, LPCTSTR szpath, strings &files);
};

class CUserPatcher
{
public:
	CUserPatcher()
	{
		m_objUpk = new CExeUnpackPkg;
	}
	virtual ~CUserPatcher()
	{
		if (m_objUpk)
		{
			delete m_objUpk;
			m_objUpk = NULL;
		}
	}

	bool Initialize( LPCTSTR sz7z, LPCTSTR szwcx );
	int InstallPatch(LPCTSTR szfile, const T_UserPatchInfo &unpkinfo);
	
protected:
	int _Patch(LPCTSTR szfile, const T_UserPatchInfo &unpkinfo, const char *szdirBackup, CString &strLogfile);
	void _FillRegInfo(INT nKBID, LPCTSTR szPatchName, LPCTSTR szProductKey, LPCTSTR szPatchKey, LPCTSTR szPatchValue, LPCTSTR szLogfile);
	void _GetDateString( CString &strDate );

protected:
	IExeUnpack* m_objUpk;
	strings m_files;
};

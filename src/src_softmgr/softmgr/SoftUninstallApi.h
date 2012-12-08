#pragma once

#include <set>
#include <list>
#include <string>
#include <skylark2/bkdb.h>
#include "SoftUninstallDef.h"
#include <winstl/filesystem/findfile_sequence.hpp>
#include <ShlObj.h>

#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY         (0x0200)
#endif
#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY         (0x0100)
#endif

namespace ksm
{

//
// 前置声明
//
class CSoftUninstall;
class Skylark::ISQLiteComDatabase3;

// 获取Key的名称
BOOL GetNameKey(const std::wstring &key, std::wstring &name);
// 判断Key是否为Uninstall类型，否则为Installer类型
BOOL IsUninstallKey(const std::wstring &key);
// 是否为64位软件Key
BOOL Is64BitKey(const std::wstring &key);
// 是否为64系统
BOOL Is64BitWindows();
// 从注册表中搜索软件数据
void SearchSoftData(SoftData2List &softData2List);
// 从缓存中加载卸载数据
BOOL LoadSoftUninstData(Skylark::ISQLiteComDatabase3 *pDB, SoftData2List &sfotData2List);
// 从缓存中加载单个卸载数据
BOOL LoadSoftUninstDataByKey(Skylark::ISQLiteComDatabase3 *pDB, LPCWSTR lpKey, SoftData2 &sfotData2);
// 从缓存中加载链接数据
BOOL LoadLinkData(Skylark::ISQLiteComDatabase3 *pDB, SoftItemAttri type, WStrList &linkList);
// 修正数据中的所有项并保存到Cache中
BOOL FixSoftData2AndSave(CSoftUninstall *pSoftUninst, SoftData2List &softData2List);
// 列出注册表指定键下的所有键名
typedef std::list<std::wstring> KeyList;
typedef KeyList::iterator		KeyIter;
typedef KeyList::const_iterator	KeyCIter;
BOOL RegListKey(HKEY hKey, const std::wstring &sub, KeyList &keyList, REGSAM samDesired = KEY_READ);
// 递归的删除子键
BOOL RegRecurseDeleteKey(HKEY hKeyRoot, LPCTSTR lpSubKey, REGSAM samDesired = KEY_WRITE);
// 删除键值
BOOL _RegDeleteValue(HKEY hKeyRoot, LPCTSTR lpSubKey, LPCTSTR lpValueName, REGSAM samDesired = KEY_WRITE);
// 得到绝对路径
std::wstring MakeAbsolutePath(LPCWSTR pPath);
inline std::wstring MakeAbsolutePath(const std::wstring &path);
// 扩展环境变量
std::wstring ExpandEnvironString(LPCWSTR pPath);
inline std::wstring ExpandEnvironString(const std::wstring &path);
// 判断是否为GUID
inline BOOL IsGuid(const std::wstring &guid);
// 十六进制转GUID
BOOL Hex2Guid(const std::wstring &strHex,std:: wstring &strGuid);
BOOL Guid2Hex(const std::wstring &strGuid, std::wstring &strHex);
// 获取注册表Key写入时间
BOOL GetKeyWriteTime(HKEY hKey, LPCWSTR pSub, ULONGLONG &writeTime);
// 更新注册表缓存标志
BOOL UpdateRegCacheFlag(Skylark::ISQLiteComDatabase3 *pDB);
// 删除文件
BOOL DeleteFile2(const std::wstring &path, BOOL recycle);
// 删除文件及斜线
inline BOOL RemoveFileSpec(std::wstring &path);
// 删除最后的斜线
inline BOOL RemoveBackSlash(std::wstring &path);
// 判断文件是否存在
BOOL PathFileExistsX64(std::wstring &path);
// 递归枚举文件
template <class Pred> inline
	Pred& RecurseEnumFile(Pred &pred, const std::wstring &root, const std::wstring &filter = std::wstring(L"*.*"), size_t deep = size_t(-1));

//
// 查找Link文件
//
class LinkFileEnum
{
public:
	LinkFileEnum(WStrList &fileList) : _fileList(fileList) {}
	bool operator()(const std::wstring &root, const WIN32_FIND_DATAW &wfd);

private:
	WStrList &_fileList;
};


//
// SoftData2查找谓词
//
class SoftData2KeyFind
{
public:
	SoftData2KeyFind(const std::wstring &key) : _key(key) {}

	bool operator()(const SoftData2 &softData2) const 
	{ return _key == softData2._key; }

private:
	const std::wstring &_key;
};

class SoftData2NameFind
{
public:
	SoftData2NameFind(const std::wstring &name) : _name(name) {}

	bool operator()(const SoftData2 &softData2) const
	{ return _wcsicmp(_name.c_str(), softData2._displayName.c_str()) == 0; }

private:
	const std::wstring &_name;
};

//
// 关键目录类
//
class CSensitivePaths
{
public:
	CSensitivePaths();

	// 检测目录必须小写，非斜线结尾
	BOOL IsSensitive(const std::wstring &path) const;

private:
	WStrHash _pathHash;
};

//
// 搜索软件数据
//
class CSearchSoftData
{
public:
	CSearchSoftData(SoftData2List &softData2List);

private:
	void SearchInstaller(SoftData2List &softData2List, REGSAM samDesired = KEY_READ);
	void SearchUninstall(SoftData2List &softData2List, REGSAM samDesired = KEY_READ);
	BOOL LoadInstallerData(const std::wstring &root, SoftData2 &softData2, REGSAM samDesired = KEY_READ);
	BOOL LoadUninstallData(HKEY hKey, const std::wstring &root, SoftData2 &softData2, REGSAM samDesired = KEY_READ);
	void MergeSoftData2(SoftData2 &uninst, const SoftData2 &inst);
	void FilterSoftData2ByName(const SoftData2List &filter, SoftData2List &src);

private:
	SoftData2List &_softData2List;
	std::set<std::wstring> _excluded;
	std::set<std::wstring> _excluded64;
};

class CLinkOpr
{
public:
	BOOL Initialize();
	BOOL GetPath(LPCWSTR pLinkPath, LPWSTR pBuf, size_t, PWIN32_FIND_DATAW pWfd = NULL);

private:
	CComPtr<IShellLinkW> _pShellLink;
	CComPtr<IPersistFile> _pPersistFile;
};

class CFsRedDisScoped
{
public:
	static void Startup();
	static void Shutdown();

public:
	CFsRedDisScoped();
	~CFsRedDisScoped();

private:
	PVOID pOldValue;

private:
	typedef BOOL (WINAPI *PFN_Wow64DisableWow64FsRedirection)(PVOID*);
	typedef BOOL (WINAPI *PFN_Wow64RevertWow64FsRedirection)(PVOID OlValue);

private:
	static PFN_Wow64RevertWow64FsRedirection spfnWow64RevertWow64FsRedirection;
	static PFN_Wow64DisableWow64FsRedirection spfnWow64DisableWow64FsRedirection;
};

class Cx64Api
{
public:
	static void Startup();
	static void Shutdown();
	static LONG RegDeleteKeyExW(HKEY, LPCWSTR, REGSAM, DWORD);

private:
	typedef LONG (WINAPI *PFN_RegDeleteKeyExW)(HKEY, LPCWSTR, REGSAM, DWORD);
	static PFN_RegDeleteKeyExW sRegDeleteKeyExW;
};
//////////////////////////////////////////////////////////////////////////
// inline implementation
inline BOOL IsGuid(const std::wstring &guid)
{
	if(
		guid.size() != 38 ||
		guid[0] != L'{' || guid[9] != L'-' || guid[14] != L'-' || guid[19] != L'-' || guid[24] != L'-' || guid[37] != L'}'
		)
		return FALSE;
	return TRUE;
}

inline std::wstring MakeAbsolutePath(const std::wstring &path)
{
	return MakeAbsolutePath(path.c_str());
}

inline std::wstring ExpandEnvironString(const std::wstring &path)
{
	return ExpandEnvironString(path.c_str());
}

inline BOOL RemoveFileSpec(std::wstring &path)
{
	using namespace std;
	wstring::size_type pos = path.find_last_of(L'\\');
	if(pos == wstring::npos) return FALSE;

	path.resize(pos);
	return TRUE;
}

inline BOOL RemoveBackSlash(std::wstring &path)
{
	using namespace std;
	if(!path.empty() && (path[path.size() - 1] == L'\\' || path[path.size() - 1] == L'/'))
	{
		path.resize(path.size() - 1);
		return TRUE;
	}
	return FALSE;
}

template <class Pred> inline
	Pred& RecurseEnumFile(Pred &pred, const std::wstring &root, const std::wstring &filter, size_t deep)
{
	using namespace winstl;

	try
	{
		findfile_sequence_w finder(root.c_str(), filter.c_str());
		findfile_sequence_w::const_iterator end = finder.end();
		for(findfile_sequence_w::const_iterator it = finder.begin(); it != end; ++it)
		{
			findfile_sequence_w::const_iterator::value_type value = *(it);
			if(!pred(root, value.get_find_data())) break;

			if(value.is_directory() && deep != 0)
			{
				RecurseEnumFile(pred, root + L'\\' + value.get_filename(), filter, deep - 1);
			}
		}

	}
	catch(...) {}

	return pred;
}

inline BOOL CSensitivePaths::IsSensitive(const std::wstring &path) const
{
	// 根目录
	if(path.size() <= 3) return TRUE;
	return (_pathHash.find(path) != _pathHash.end());
}

}
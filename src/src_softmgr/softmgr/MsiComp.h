#pragma once

#include <atlstr.h>
#include <atlcoll.h>

//
// 查找Msi打包程序的相关组件
//
//@Remark
// 一.注册表项
// HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\...\\Components\\...
// HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Installer\\Products\\GUID
//
// 二.根据GUID从Installer中分析主目录
// 可能包含多个目录
//
class CMsiComp
{
public:
//	typedef struct RegInfo
//	{
//		HKEY		_key;
//		CStringW	_subkey;
//
//		RegInfo(HKEY key, const CStringW &subkey) 
//			: _key(key), _subkey(subkey) {}
//	} *PRegInfo;
//	typedef const RegInfo *PCRegInfo;
	// 注册表列表
	typedef CAtlList<CStringW>	RegList;
	// 文件列表
	typedef RegList				DirList;

public:
	CMsiComp(const CStringW &guid);

public:
	// 返回注册表项
	const RegList& GetRegList() const 
	{ return _regList; }

	// 返回主目录
	const DirList& GetDirList() const 
	{ return _dirList; }

private:
	void EnumComponent(const CStringW &subkey);
	void ParseComponent(const CStringW &subkey);
	void MergeDirectory(LPCWSTR pBuff, size_t cchBuff);

private:
	RegList _regList;
	DirList _dirList;
	const CStringW &_guid;
};
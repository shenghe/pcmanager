#pragma once

#include <map>
#include <list>
#include <string>
#include <utility>
#include <hash_map>
#include <hash_set>
#include <softmgr/ISoftUninstall.h>

namespace ksm
{

//
// 数据结构定义
//
// 软件数据类型定义
typedef struct SoftData2
{
	ULONG _mask;
	std::wstring _key;
	std::wstring _guid;
	std::wstring _displayName;
	std::wstring _mainPath;
	std::wstring _descript;
	std::wstring _descriptReg;
	std::wstring _infoUrl;
	std::wstring _spellWhole;
	std::wstring _spellAcronym;
	std::wstring _iconLocation;
	std::wstring _uninstString;
	std::wstring _logoUrl;
	ULONGLONG _size;
	ULONG _lastUse;
	LONG _type;
	LONG _id;
	LONG _count;

	SoftData2()
	{
		_mask = SDM_None;
		_size = 0;
		_lastUse = 0;
		_type = 0;
		_id = 0;
		_count = -1;
	}
} *PSoftData2;
typedef const SoftData2* PCSoftData2;
typedef std::list<SoftData2>			SoftData2List;
typedef SoftData2List::iterator			SoftData2Iter;
typedef SoftData2List::const_iterator	SoftData2CIter;

// 残留项类型定义
typedef struct SoftRubbish2
{
	SoftItemAttri	_type;	// 开始、快速、桌面、注册表
	std::wstring	_data;

	SoftRubbish2() {}
	SoftRubbish2(SoftItemAttri type, const std::wstring &data)
		: _type(type), _data(data) {}
} *PSoftRubbish2;
typedef const SoftRubbish2* PCSoftRubbish2;

typedef std::list<SoftRubbish2>			SoftRubbish2List;
typedef SoftRubbish2List::iterator		SoftRubbish2Iter;
typedef SoftRubbish2List::const_iterator
	SoftRubbish2CIter;

// 字符串链表定义
typedef std::list<std::wstring>			WStrList;
typedef WStrList::iterator				WStrListIter;
typedef WStrList::const_iterator		WStrListCIter;

class WStrListFind
{
public:
	WStrListFind(const std::wstring &str) : _str(str) {}
	bool operator()(const std::wstring &str) const 
	{ return _str == str; }

private:
	const std::wstring &_str;
};

// 字符串Hash表定义
typedef stdext::hash_set<std::wstring>	WStrHash;
typedef WStrHash::iterator				WStrHashIter;
typedef WStrHash::const_iterator		WStrHashCIter;

// 拼音结构定义
typedef struct PinYin
{
	std::string _whole;
	std::string _acronym;
} *PPinYin;
typedef const PinYin* PCPinYin;
typedef stdext::hash_map<std::string, PinYin> 
										PinYinHash;
typedef PinYinHash::iterator			PinYinIter;
typedef PinYinHash::const_iterator		PinYinCIter;

//
// 路径定义
//
const LPCWSTR SOFT_UNINST_DB	= L"\\appdata\\ksoft_ucache_5";
const LPCWSTR SOFT_UNINST_DAT	= L"\\ksoft\\data\\softuninst2.dat";
const LPCWSTR SOFT_PINYIN_DAT	= L"\\ksoft\\data\\fonts.dat";
const LPCWSTR SOFT_STARTUP_LOG	= L"\\appdata\\startup.log";

}
#include "stdafx.h"
#include "MsiComp.h"
#pragma warning(push)
#pragma warning(disable : 6011)
#pragma warning(disable : 6328)
#pragma warning(disable : 6387)
#include <winstl/registry/reg_value.hpp>
#include <winstl/registry/reg_key_sequence.hpp>
#include <winstl/registry/reg_value_sequence.hpp>
#include <winstl/filesystem/filesystem_traits.hpp>
#include <stlsoft/string/case_functions.hpp>
#pragma warning(pop)
using namespace stlsoft;
using namespace winstl_project;

//////////////////////////////////////////////////////////////////////////
CMsiComp::CMsiComp(const CStringW &guid) : _guid(guid)
{
	try
	{
		static const CStringW userDataKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData";

		reg_key_sequence_w regUserData(HKEY_LOCAL_MACHINE, (LPCWSTR)userDataKey);
		reg_key_sequence_w::const_iterator it = regUserData.begin();
		reg_key_sequence_w::const_iterator end = regUserData.end();
		for(; it != end; ++it)
		{
			EnumComponent(userDataKey + L'\\' + (*it).name().c_str());
		}
#if 0
		CStringW classesKey = L"SOFTWARE\\Classes\\Installer\\Products\\" + guid;

		reg_key_sequence_w regClasses(HKEY_LOCAL_MACHINE, (LPCWSTR)classesKey);
		_regList.AddHead(L"HKEY_LOCAL_MACHINE\\" + classesKey);		
#endif
	}
	catch(...) {}
}

void CMsiComp::EnumComponent(const CStringW &subkey)
{
	try
	{
		reg_key_sequence_w keyComp(HKEY_LOCAL_MACHINE, subkey + L"\\Components");
		reg_key_sequence_w::const_iterator it = keyComp.begin();
		reg_key_sequence_w::const_iterator end = keyComp.end();
		for(; it != end; ++it)
		{
			ParseComponent(subkey + L"\\Components\\" + (*it).name().c_str());
		}
	}
	catch(...) {}
}

void CMsiComp::ParseComponent(const CStringW &subkey)
{
	try
	{
		reg_key_w key(HKEY_LOCAL_MACHINE, (LPCWSTR)subkey, KEY_READ);
		
		reg_key_w::key_value_type val = key.get_value((LPCWSTR)_guid);

		reg_key_w::string_type str = val.value_sz();

#if 0
		// 记录当前注册表项
		_regList.AddTail(L"HKEY_LOCAL_MACHINE\\" + subkey);
#endif

		// 处理目录：C:
		if(
			filesystem_traits<wchar_t>::file_exists(str.c_str())
			)
		{
			make_lower(str);
			size_t cch = str.size();
			if(filesystem_traits<wchar_t>::is_directory(str.c_str()))
			{
				if(str[str.size()- 1] != L'\\')
				{
					str += L'\\';
					++cch;
				}
			}
			else
			{
				LPCWSTR p = wcsrchr(str.c_str(), L'\\');
				if(p != NULL) cch = p-str.c_str()+1;
			}

			if(_dirList.IsEmpty())
			{
				_dirList.AddTail(CStringW(str.c_str(), cch));
			}
			else
			{
				MergeDirectory(str.c_str(), cch);
			}
		}
#if 0
		// 处理注册表项：00:
		else if(str.size() >= 4 && str[2] == L':')
		{
			CStringW strReg;
			if(str[0] == L'0' && str[1] == L'0')
			{
				strReg = L"HKEY_CLASSES_ROOT";
			}
			else if(str[0] == L'0' && str[1] == L'1')
			{
				strReg = L"HKEY_CURRENT_USER";
			}
			else if(str[0] == L'0' && str[1] == L'2')
			{
				strReg = L"HKEY_LOCAL_MACHINE";
			}
			else if(str[0] == L'0' && str[1] == L'3')
			{
				strReg = L"HKEY_USERS";
			}
			else if(str[0] == L'0' && str[1] == L'4')
			{
				strReg = L"HKEY_PERFORMANCE_DATA";
			}
			else if(str[0] == L'0' && str[1] == L'5')
			{
				strReg = L"HKEY_CURRENT_CONFIG";
			}
			else if(str[0] == L'0' && str[1] == L'6')
			{
				strReg = L"HKEY_DYN_DATA";
			}
			else if(str[0] == L'5' && str[1] == L'0')
			{
				strReg = L"HKEY_PERFORMANCE_TEXT";
			}
			else if(str[0] == L'6' && str[1] == L'0')
			{
				strReg = L"HKEY_PERFORMANCE_NLSTEXT";
			}

			if(!strReg.IsEmpty()) 
			{
				_regList.AddTail(strReg + L'\\' + str.c_str());
			}
		}
#endif
	}
	catch(...) {}
}

void CMsiComp::MergeDirectory(LPCWSTR pBuff, size_t cchBuff)
{
	bool add = false;
	POSITION pos = _dirList.GetHeadPosition();
	while(pos != NULL)
	{
		POSITION cur = pos;
		const CStringW &str = _dirList.GetNext(pos);

		if(str.GetLength() < static_cast<int>(cchBuff))
		{
			if(wcsncmp(str, pBuff, str.GetLength()) == 0) 
				return;
		}
		else
		{
			if(wcsncmp(pBuff, str, cchBuff) == 0)
				_dirList.RemoveAt(cur), add = true;
		}
	}

	if(add) _dirList.AddTail(CStringW(pBuff, cchBuff));
}
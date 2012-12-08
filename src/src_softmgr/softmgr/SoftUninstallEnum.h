#pragma once

#include "SoftUninstallDef.h"
#include "SoftUninstallApi.h"

namespace ksm
{

#define SoftData2Member(m) (m.empty() ? NULL : m.c_str())
//
// 软件数据枚举接口实现
//
class CSoftDataEnum : public ISoftDataEnum
{
public:
	CSoftDataEnum(const SoftData2List &softData2List)
		: _softData2List(softData2List) {}

	virtual BOOL __stdcall EnumFirst(PSoftData pData)
	{
		_it = _softData2List.begin();
		if(_it != _softData2List.end())
		{
			CopyData(pData, _it);
			return TRUE;
		}
		return FALSE;
	}

	virtual BOOL __stdcall EnumNext(PSoftData pData)
	{
		if(++_it != _softData2List.end())
		{
			CopyData(pData, _it);
			return TRUE;
		}
		return FALSE;
	}

private:
	void CopyData(PSoftData pData, SoftData2CIter it)
	{
		pData->_mask			= it->_mask;
		pData->_pcKey			= it->_key.c_str();
		pData->_pcDisplayName	= SoftData2Member(it->_displayName);
		pData->_pcMainPath		= SoftData2Member(it->_mainPath);
		pData->_pcIconLocation	= SoftData2Member(it->_iconLocation);
		pData->_pcDescript		= SoftData2Member(it->_descript);
		pData->_pcInfoUrl		= SoftData2Member(it->_infoUrl);
		pData->_pcSpellWhole	= SoftData2Member(it->_spellWhole);
		pData->_pcSpellAcronym	= SoftData2Member(it->_spellAcronym);
		pData->_pcLogoUrl		= SoftData2Member(it->_logoUrl);
		pData->_size			= it->_size;
		pData->_lastUse			= it->_lastUse;
		pData->_id				= it->_id;
		pData->_type			= it->_type;
		pData->_count			= it->_count;

		// 当描述字符串为空时，使用注册表中的“Comments”字段
		if(pData->_pcDescript == NULL && !it->_descriptReg.empty())
		{
			pData->_mask |= SDM_Description;
			pData->_pcDescript = it->_descriptReg.c_str();
		}

		//if(Is64BitKey(it->_key))
		//{
		//	// 不允许打开64位软件主目录
		//	pData->_mask &=~(SDM_Main_Path);
		//	pData->_pcMainPath = NULL;
		//}
	}

private:
	SoftData2CIter _it;
	const SoftData2List &_softData2List;
};

//
// 软件链接枚举接口实现
//
class CSoftLinkEnum : public ISoftLinkEnum
{
public:
	CSoftLinkEnum(const WStrList &linkList)
		: _linkList(linkList)	{}

	virtual BOOL __stdcall EnumFirst(LPCWSTR *pKey)
	{
		_it = _linkList.begin();
		if(_it != _linkList.end())
		{
			*pKey = _it->c_str();
			return TRUE;
		}
		return FALSE;
	}

	virtual BOOL __stdcall EnumNext(LPCWSTR *pKey)
	{
		if(++_it != _linkList.end())
		{
			*pKey = _it->c_str();
			return TRUE;
		}
		return FALSE;
	}

private:
	WStrListCIter _it;
	const WStrList &_linkList;
};


//
// 残留项枚举接口实现
//
class CSoftRubbishEnum : public ISoftRubbishEnum
{
public:
	CSoftRubbishEnum(const SoftRubbish2List &softRubbish2List)
		: _softRubbish2List(softRubbish2List) {}

	virtual BOOL __stdcall EnumFirst(PSoftRubbish pData)
	{
		_it = _softRubbish2List.begin();
		if(_it != _softRubbish2List.end())
		{
			pData->_type	= _it->_type;
			pData->_pData	= SoftData2Member(_it->_data);
			return TRUE;
		}
		return FALSE;
	}

	virtual BOOL __stdcall EnumNext(PSoftRubbish pData)
	{
		if(++_it != _softRubbish2List.end())
		{
			pData->_type	= _it->_type;
			pData->_pData	= SoftData2Member(_it->_data);
			return TRUE;
		}
		return FALSE;
	}

private:
	SoftRubbish2List::const_iterator _it;
	const SoftRubbish2List &_softRubbish2List;
};

}
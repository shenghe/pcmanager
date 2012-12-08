#include "stdafx.h"
#include "SoftInstProxy.h"
#include <string>
#include <sstream>
using namespace std;
#include <stlsoft/memory/auto_buffer.hpp>
#include <stlsoft/shims/access/string.hpp>
#include <stlsoft/string/trim_functions.hpp>
using namespace stlsoft;
#include <tinyxml/tinyxml.h>
#include <libheader/libheader.h>

namespace ksm
{

static inline int safe_atoi(LPCSTR pStr, int def = 0);
static inline __int64 safe_atoi64(LPCSTR pstr, __int64 def = 0);
static const wchar_t sSoftMgrModule[]	= L"KSoft\\softmgr.dll";
static const wchar_t sInstLibPath[]		= L"KSoft\\data\\autoinst.dat";
//////////////////////////////////////////////////////////////////////////
CSoftInstProxy::CSoftInstProxy()
{
	_pNotify = NULL;
	_pISoftInst = NULL;
	_hModSoftMgr = NULL;
}

CSoftInstProxy::~CSoftInstProxy()
{
	Uninitialize();
}

BOOL CSoftInstProxy::Initialize(LPCWSTR pKSafePath)
{
	ATLASSERT(_pNotify != NULL);
	if(_pNotify == NULL)
	{
		return FALSE;
	}

	wstring path = c_str_data_w(pKSafePath);
	trim_right(path, L"\\");
	path += L'\\';
	
	_hModSoftMgr = ::LoadLibraryW(c_str_data_w(path + sSoftMgrModule));
	if(_hModSoftMgr == NULL)
	{
		return FALSE;
	}

	typedef HRESULT (*PFN_CreateObject)(REFIID riid, void** ppvObj);
	PFN_CreateObject pfnCreateObject = (PFN_CreateObject)::GetProcAddress(_hModSoftMgr, "CreateSoftMgrObject");
	if(pfnCreateObject == NULL)
	{
		return FALSE;
	}

	pfnCreateObject(__uuidof(ISoftInstall), (void**)&_pISoftInst);
	if(_pISoftInst == NULL)
	{
		return FALSE;
	}

	_pISoftInst->SetNotify(_pNotify);

	wstring instLibPath = path + sInstLibPath;
	if(!_pISoftInst->Initialize(c_str_data_w(path), c_str_data_w(instLibPath)))
	{
		return FALSE;
	}

	return LoadInstLib(c_str_data_w(instLibPath));
}

void CSoftInstProxy::Uninitialize()
{
	if(_pISoftInst != NULL)
	{
		_pISoftInst->Uninitialize();
		_pISoftInst = NULL;
	}

	if(_hModSoftMgr != NULL)
	{
		::FreeLibrary(_hModSoftMgr);
		_hModSoftMgr = NULL;
	}
}

BOOL CSoftInstProxy::SetInstPackPath(DWORD id, LPCWSTR pInstPackPath)
{
	InstInfoMap::CPair *it = _instInfoMap.Lookup(id);
	if(it == NULL) return FALSE;

	it->m_value._instPackPath = pInstPackPath;
	return TRUE;
}

const InstInfo* CSoftInstProxy::GetInstInfo( DWORD id ) const
{
	const InstInfoMap::CPair *itInst = _instInfoMap.Lookup(id);
	if(itInst == NULL) return NULL;

	return &itInst->m_value;
}

BOOL CSoftInstProxy::SetPluginInfo(DWORD id, LPCWSTR pPluginKey, BOOL state)
{
	InstInfoMap::CPair *itInst = _instInfoMap.Lookup(id);
	if(itInst == NULL) return FALSE;

	PluginInfoList &pluginList = itInst->m_value._pluginInfoList;
	for(int i = 0; i < pluginList.GetSize(); ++i)
	{
		if(pluginList[i]._key == pPluginKey)
		{
			pluginList[i]._curState = state;
			return TRUE;
		}
	}

	return TRUE;
}

BOOL CSoftInstProxy::StartInstSoft(DWORD id)
{
	InstInfoMap::CPair *itInst = _instInfoMap.Lookup(id);
	if(
		itInst == NULL || 
		itInst->m_value._instPackPath.IsEmpty() || 
		_instDir.IsEmpty()
		)
	{
		return FALSE;
	}

	// 序列化参数
	string params;
	{
		ostringstream os;
		const InstInfo &instInfo = itInst->m_value;

		os<<"<soft ver=\"1.0\">\n";
		{
			os<<"<softinfo softid=\""<<id<<"\" inst_pack=\""<<(LPCSTR)CW2A(instInfo._instPackPath)<<"\">\n";
			{
				CString		strOrgPath = instInfo._instDir;

				if (strOrgPath.Find( L"$Target" ) != -1)
					strOrgPath.Replace( L"$Target", _instDir);
				else
					strOrgPath = _instDir;

				os<<"<inst_size>"<<instInfo._instSize<<"</inst_size>\n";
				os<<"<inst_dir>"<<(LPCSTR)CW2A(strOrgPath)<<"</inst_dir>\n";

				os<<"<inst_timeout>"<<instInfo._instTimeout<<"</inst_timeout>\n";
				os<<"<setting use_virtual_desktop=\""<<instInfo._bUseVirtualDesk<<"\"/>\n";

				if(instInfo._pluginInfoList.GetSize() != 0)
				{
					os<<"<user_define count=\""<<instInfo._pluginInfoList.GetSize()<<"\">\n";

					const PluginInfoList &pluginList = instInfo._pluginInfoList;
					for(int i = 0; i < pluginList.GetSize(); ++i)
					{
						os<<"<item name=\""
							<<(LPCSTR)CW2A(pluginList[i]._key)<<"\" value=\""
							<<(pluginList[i]._curState ? 1 : 0)<<"\" info=\""
							<<(LPCSTR)CW2A(pluginList[i]._name)<<"\"/>\n";
					}

					os<<"</user_define>\n";
				}
			}
			os<<"</softinfo>\n";
		}
		os<<"</soft>\n";

		params = os.str();
	}

	// 安装软件
	return _pISoftInst->StartInstSoft(c_str_data_a(params), static_cast<DWORD>(params.size()));
}

void CSoftInstProxy::StopInstSoft()
{
	_pISoftInst->StopInstSoft();
}

BOOL CSoftInstProxy::LoadInstLib(LPCWSTR pLibPath)
{
	_instInfoMap.RemoveAll();

	TiXmlDocument xmlDoc;
	BkDatLibHeader header;
	CDataFileLoader	loader;
	if(!loader.GetLibDatHeader(pLibPath, header))
		return FALSE;

	BkDatLibContent cont;
	if(!loader.GetLibDatContent(pLibPath, cont))
		return FALSE;

	if(xmlDoc.Parse((char*)cont.pBuffer) == NULL)
		return FALSE;
	


	TiXmlHandle hRoot(xmlDoc.FirstChildElement("onekeyinstall"));
	hRoot = hRoot.FirstChildElement("alluserdefine").Element();

	for(TiXmlElement *pElem = hRoot.FirstChildElement("softinfo").Element(); pElem != NULL; pElem = pElem->NextSiblingElement())
	{
		DWORD id = static_cast<DWORD>(safe_atoi(pElem->Attribute("softid")));
		if(id == 0) return FALSE;

		TiXmlElement *pSubElem = pElem->FirstChildElement("download");
		if(pSubElem == NULL) return FALSE;

		InstInfo instInfo;
		instInfo._downUrl = pSubElem->Attribute("url");
		instInfo._downMd5 = pSubElem->Attribute("md5");
		instInfo._downName = pSubElem->Attribute("save_name");
		instInfo._downSize = static_cast<ULONGLONG>(safe_atoi64(pSubElem->Attribute("file_size")));
		if(instInfo._downSize == 0) return FALSE;
		instInfo._instSize = static_cast<ULONGLONG>(safe_atoi64(pSubElem->Attribute("inst_size")));
		if(instInfo._instSize == 0) return FALSE;

		pSubElem = pElem->FirstChildElement("inst_timeout");
		if(pSubElem == NULL) return FALSE;

		instInfo._instTimeout = static_cast<DWORD>(safe_atoi(pSubElem->GetText()));
		if(instInfo._instTimeout == 0) return FALSE;

		pSubElem = pElem->FirstChildElement("inst_dir");
		if(pSubElem == NULL) return FALSE;

		instInfo._instDir = pSubElem->GetText();
//		if (instInfo._instDir.IsEmpty())return FALSE;

		// 插件信息
		pSubElem = pElem->FirstChildElement("user_define");
		if(pSubElem != NULL)
		{
			for(TiXmlElement *pPlugin = pSubElem->FirstChildElement("item"); pPlugin != NULL; pPlugin = pPlugin->NextSiblingElement())
			{
				PluginInfo pluginInfo;
				
				pluginInfo._key = pPlugin->Attribute("name");
				if(pluginInfo._key.IsEmpty()) return FALSE;

				pluginInfo._name = pPlugin->Attribute("info");

				LPCSTR pState = pPlugin->Attribute("value");
				if(pState == NULL) return FALSE;

				if(pState[0] == '0') pluginInfo._state = FALSE;
				else pluginInfo._state = TRUE;
				pluginInfo._curState = pluginInfo._state;

				instInfo._pluginInfoList.Add(pluginInfo);
			}
		}

		TiXmlElement *pSettingElem = pElem->FirstChildElement("setting");
		if (pSettingElem)
		{
			instInfo._bUseVirtualDesk = static_cast<BOOL>(safe_atoi(pSettingElem->Attribute("use_virtual_desktop")));
		}
		else
			instInfo._bUseVirtualDesk = FALSE;

		_instInfoMap[id] = instInfo;
	}

	return TRUE;
}

inline int safe_atoi(LPCSTR pStr, int def)
{
	if(pStr == NULL) return def;
	return atoi(pStr);
}

inline __int64 safe_atoi64(LPCSTR pStr, __int64 def)
{
	if(pStr == NULL) return def;
	return _atoi64(pStr);
}

}
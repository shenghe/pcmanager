#pragma once
#include "kclear/libheader.h"
#include "CleanManager.h"




CCleanManager::~CCleanManager( void )
{
	m_pInfo = NULL;
	m_pTree = NULL;
	m_pInfoThis = NULL;
	m_pTreeThis = NULL;
}

CCleanManager::CCleanManager( void )
{
}

int CCleanManager::LoadTreetruct(CString lib_file)
{
	if(lib_file.GetLength()==0)lib_file = _libfile;
	_libfile=lib_file;

	::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 

	CDataFileLoader	loader;
	BkDatLibHeader new_h;
	if(loader.GetLibDatHeader(lib_file,new_h)==FALSE)
	{
		return 0;
	}
	
	TiXmlDocument plugins;
	BkDatLibContent cont;

	if(loader.GetLibDatContent(lib_file,cont)==FALSE)
	{
		return 2;
	}		
	if(false==plugins.Parse((char*)cont.pBuffer))
	{
		return 3;
	}		

	TiXmlHandle hDoc(&plugins);
	TiXmlElement* pElem;
	TiXmlHandle hroot(NULL);
	pElem=hDoc.FirstChildElement().Element();
	hroot=TiXmlHandle(pElem);

	pElem=hroot.Element();
	for(TiXmlElement* psoft=pElem->FirstChildElement("clear");psoft;psoft=psoft->NextSiblingElement("clear"))
	{
		const char* atttypeid=psoft->Attribute("typeid");
		const char* attclearid=psoft->Attribute("clearid");
		const char* attchecked=psoft->Attribute("checked");
		const char* attsn=psoft->Attribute("sn");

		ClearTreeInfo treetype;
		treetype.nSn = atoi(attsn);
		treetype.nCheck = atoi(attchecked);
		treetype.strTypeid = atttypeid;
		treetype.strClearid = attclearid;
		treetype.strTypeid.AllocSysString();
		treetype.strClearid.AllocSysString();
		treetype.strExtName = L"";
		treetype.nFileext = 0;
		treetype.nApp = 0;
		TiXmlElement* pName = psoft->FirstChildElement("clearname");
		treetype.strClearName = CA2W(pName->GetText());

		TiXmlElement* pbmpurl = psoft->FirstChildElement("iconurl");
		treetype.strBmpurl = CA2W(pbmpurl->GetText());
		TiXmlElement* pbmpcrc = psoft->FirstChildElement("iconcrc");
		treetype.strBmpcrc= CA2W(pbmpcrc->GetText());

		if(m_pTree!=NULL)
		{
			m_pTree(m_pTreeThis, treetype);
		}
		if(m_pInfo!=NULL)
		{
			for(TiXmlElement* pscan=psoft->FirstChildElement("scan");pscan;pscan=pscan->NextSiblingElement("scan"))
			{
				SoftInfo softinfo;
				const char* attmethod = pscan->Attribute("method");
				const char* attpath = pscan->Attribute("path");
				const char* attpara = pscan->Attribute("para");
				const char* attkeyname = pscan->Attribute("keyname");

				softinfo.strmethod = attmethod;
				softinfo.strmethod.AllocSysString();
				softinfo.strpath = attpath;
				softinfo.strpath.AllocSysString();
				softinfo.strpara = attpara;
				softinfo.strpara.AllocSysString();
				softinfo.strkeyname = attkeyname;
				softinfo.strkeyname.AllocSysString();
				softinfo.strClearid = treetype.strClearid;
				softinfo.strTypeid = treetype.strTypeid;
				m_pInfo(m_pInfoThis, softinfo);
			}
		}
	}
	for(TiXmlElement* psoft=pElem->FirstChildElement("cleardisk");psoft;psoft=psoft->NextSiblingElement("cleardisk"))
	{
		const char* atttypeid=psoft->Attribute("typeid");
		const char* attclearid=psoft->Attribute("clearid");
		const char* attchecked=psoft->Attribute("checked");
		const char* attsn=psoft->Attribute("sn");

		ClearTreeInfo treetype;
		treetype.nSn = atoi(attsn);
		treetype.nCheck = atoi(attchecked);
		treetype.strTypeid = atttypeid;
		treetype.strClearid = attclearid;
		treetype.strTypeid.AllocSysString();
		treetype.strClearid.AllocSysString();
		treetype.nFileext = 1;
		TiXmlElement* pExtName = psoft->FirstChildElement("clearrule");
		CString str = CA2W(pExtName->GetText());
		TiXmlElement* pName = psoft->FirstChildElement("clearname");
		CString strClearName = CA2W(pName->GetText());
		treetype.strClearName.Format(L"%s£¨%s£©", strClearName,str);
		str.Replace('|', ';');
		str+=L";";
		treetype.strExtName = str;		
		treetype.nApp = 0;

		TiXmlElement* pbmpurl = psoft->FirstChildElement("iconurl");
		treetype.strBmpurl = CA2W(pbmpurl->GetText());
		TiXmlElement* pbmpcrc = psoft->FirstChildElement("iconcrc");
		treetype.strBmpcrc= CA2W(pbmpcrc->GetText());

		if(m_pTree!=NULL)
		{
			m_pTree(m_pTreeThis, treetype);
		}
	}
	return 1;
}

void CCleanManager::SetSoftInfoCallBack(SoftInfoCallBack pSoftInfoCallBack,void* pThis)
{
	m_pInfo = pSoftInfoCallBack;
	m_pInfoThis = pThis;
}
void CCleanManager::SetTreeCallBack(SoftTreeCallBack pSoftTreeCallBack,void* pThis)
{
	m_pTree = pSoftTreeCallBack;
	m_pTreeThis = pThis;
}

#include "StdAfx.h"
#include "kprocesslibloader.h"
#include "kprocesslib/processlib.h"
#include <common/utility.h>
#include "kprocesslib/interface.h"
#include <libheader/libheader.h>
#include "kprocesslib/kprocessitem.h"
#include "kprocesslib/ccf/KFile.h"
using namespace Kingsoft::Test::CCF::IO;

KProcessLibLoader::KProcessLibLoader(void)
{
	m_bLoaded = FALSE;
	m_pProcessLib = new ProcessLib;
}

KProcessLibLoader::~KProcessLibLoader(void)
{
	if (m_pProcessLib)
	{
		delete m_pProcessLib;
	}
}

CString KProcessLibLoader::GetDefaultLibPath()
{
	CString	strpath;
	CAppPath().Instance().GetLeidianDataPath(strpath,TRUE);
	strpath.Append(L"\\");
	strpath.Append(STR_KSAFE_PROCESS_LIB_DAT_NAME);
	return strpath;
}

BOOL KProcessLibLoader::LoadLib( LPCTSTR szLibDatPath /*= NULL*/ )
{
	BOOL bRes = FALSE;

	if (szLibDatPath != NULL)
		m_strDatPath = szLibDatPath;
	else
		m_strDatPath = GetDefaultLibPath();

	if (!KFile::Exists(wstring(m_strDatPath.AllocSysString())))
	{
		return FALSE;
	}

	CDataFileLoader	loader;
	CString			strXML;
	BkDatLibHeader	header;
	BkDatLibContent libContent;

	if (loader.GetLibDatHeader(m_strDatPath, header) && header.nFormatVer > 1)
		return FALSE;

	if (loader.GetLibDatContent(m_strDatPath, libContent))
	{
		bRes = m_pProcessLib->Load((char*)libContent.pBuffer, libContent.nLen);
	}

	return bRes;
}

BOOL KProcessLibLoader::FindInfo( LPCTSTR szFilePath, KSProcessInfo& processInfo)
{
	KProcessItem processItem;
	if (m_pProcessLib->FindInfo(szFilePath, processItem))
	{
		processInfo.strProcessName = processItem.ProcessName();
		processInfo.strDesc = processItem.Desc();
		processInfo.bCanClose = processItem.CanClose();
		processInfo.strDisplayName = processItem.DisplayName();
		processInfo.strCompanyName = m_pProcessLib->GetCompanyName(processItem.CompanyIndex());
		processInfo.strPath = m_pProcessLib->GetPath(processItem.PathIndex());
		processInfo.strSignName = m_pProcessLib->GetSignName(processItem.SignNameIndex());
		processInfo.nProcessType = processItem.ProcessType();
		processInfo.nId = processItem.ProcessItemId();
		processInfo.strSoftId = processItem.SoftId();

		return TRUE;
	}

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : areafile.cpp
//      Version   : 1.0
//      Comment   : 网镖区域规则文件
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

/*
#include "stdafx.h"
#include "areafile.h"
#include "areamgr.h"

#include "kis/kpfw/filecomm.h"
#include "framework/kis_file.h"
#include "kis/kpfw/errorcode.h"*/

inline HRESULT KAreaFile::Load(LPCWSTR strFile)
{
	kis::KFileRead mapFile( strFile );

	if (!mapFile.Ok())
		return E_AREA_FILE_OPEN_FAILED;

	INT		nSize			= mapFile.GetSize();
	BYTE*	pFileData		= mapFile;
	AreaFileHeader* pHeader	= (AreaFileHeader*)pFileData;

	if (pHeader->nMagic != AREA_FILE_MAGIC)
		return E_AREA_FILE_UNKNOWN_FMT;

	KRecord	appRule(pFileData + pHeader->DataOffset);
	if (!appRule.Ok())
		return E_AREA_FILE_UNKNOWN_FMT;

	try
	{
		return ReadAreaRoot(&appRule);
	}
	catch (...)
	{
	}
	return E_FAIL;
}

inline HRESULT KAreaFile::Save(LPCWSTR strFile)
{
	KMemWriteProxy proxy;

	// 保存数据到内存中
	KRecordWriter root(&proxy, TRUE, 0, AreaInfo_Root);
	SaveAreaList(&root);
	root.EndRecord();

	INT nSize = proxy.GetCurPos();
	INT nFileSize = nSize + sizeof(AreaFileHeader);

	kis::KFileWrite mapFile(strFile, nFileSize);
	if (!mapFile.Ok())
		return E_AREA_FILE_SAVE_FAILED;

	BYTE* pBuf = mapFile;
	AreaFileHeader* pHeader = (AreaFileHeader*)pBuf;
	pHeader->cbSize = sizeof(AreaFileHeader);
	pHeader->nMagic = AREA_FILE_MAGIC;
	pHeader->nVer = 0;
	pHeader->DataOffset = sizeof(AreaFileHeader);

	memcpy(pBuf + pHeader->DataOffset, proxy.GetBuf(0), nSize);
	return S_OK;
}

inline HRESULT KAreaFile::ReadAreaRoot(KRecord* pRecord)
{
	ASSERT(pRecord->GetType() == AreaInfo_Root);

	KRecord ruleList = pRecord->FirstSubRecord();
	if (!ruleList.Ok())
		return E_AREA_FILE_UNKNOWN_FMT;
	return ReadAreaList(&ruleList);
}

inline HRESULT	KAreaFile::ReadAreaList(KRecord* pRecord)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == AreaInfo_AreaList);

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
			return E_AREA_FILE_UNKNOWN_FMT;

		hr = ReadArea(&rule); 
		if (!SUCCEEDED(hr))
			return hr;

		rule = rule.NextRecord();
	} 
	return hr;
}

inline HRESULT	KAreaFile::ReadArea(KRecord* pRecord)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == AreaInfo_Area);
	KArea* pArea = m_pAreaMgr->CreateArea();

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
		{
			hr = E_AREA_FILE_UNKNOWN_FMT;
			break;
		}
		switch (rule.GetType())
		{
		case AreaInfo_Name:
			pArea->SetName( (LPWSTR)rule.GetData() );
			break;
		case AreaInfo_CreateDate:
			ASSERT(rule.GetDataSize() == sizeof(FILETIME));
			pArea->SetCreateTime((FILETIME*)rule.GetData());
			break;
		case AreaInfo_Type:
			ASSERT(rule.GetDataSize() == sizeof(AreaType));
			pArea->SetType( *(AreaType*)rule.GetData());
			break;
		case AreaInfo_Gate:
			ASSERT(rule.GetDataSize() == sizeof(BYTE[6]));
			pArea->SetGateMac( (BYTE*)rule.GetData());
			break;
		case AreaInfo_GateIp:
			pArea->SetGateIp( *((UINT*)rule.GetData()));
			break;
		}
		rule = rule.NextRecord();
	}

	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(m_pAreaMgr->AddArea(pArea)))
			return hr;
	}
	delete pArea;
	return hr;
}

inline HRESULT KAreaFile::SaveAreaList(KRecordWriter* pWriter)
{
	KRecordWriter* pAreaList = pWriter->StartCompRecord(AreaInfo_AreaList);
	for (int i = 0; i < m_pAreaMgr->GetAreaCnt(); i++)
	{
		KArea* pRule = m_pAreaMgr->GetArea(i);
		KRecordWriter* pAreaWriter = pAreaList->StartCompRecord(AreaInfo_Area);
		SaveArea(pAreaWriter, pRule);

		pAreaWriter->EndRecord();
		delete pAreaWriter;
	}

	pAreaList->EndRecord();
	delete pAreaList;
	return S_OK;
}

inline HRESULT KAreaFile::SaveArea(KRecordWriter* pWriter, KArea* pArea)
{
	if (pArea->GetName())
		SaveString(pArea->GetName(), pWriter, AreaInfo_Name);

	if (pArea->GetCreateTime())
		SaveStruct(pArea->GetCreateTime(), pWriter, AreaInfo_CreateDate);

	{
		int t = pArea->GetType();
		KRecordWriter* pType = pWriter->StartRecord(AreaInfo_Type);
		pType->Write(t);
		pType->EndRecord();
		delete pType;
	}

	{
		BYTE addr[6];
		pArea->GetGateMac(addr);

		KRecordWriter* pAddr = pWriter->StartRecord(AreaInfo_Gate);
		pAddr->Write(addr, 6);
		pAddr->EndRecord();
		delete pAddr;
	}

	{
		UINT uIp = pArea->GetGateIp();
		KRecordWriter* pGateIp = pWriter->StartRecord(AreaInfo_GateIp);
		pGateIp->Write(uIp);
		pGateIp->EndRecord();
		delete pGateIp;
	}

	return S_OK;
}
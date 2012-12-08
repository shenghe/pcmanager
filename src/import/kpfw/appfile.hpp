////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : appfile.cpp
//      Version   : 1.0
//      Comment   : 网镖应用程序规则文件读写
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////
/*
#include "stdafx.h"
#include "appfile.h"
#include "appmgr.h"
#include "apprule.h"

#include "kis/kpfw/filecomm.h"
#include "framework/kis_file.h"
#include "kis/kpfw/errorcode.h"*/
#include "framework/kis_pathstr.h"

//////////////////////////////////////////////////////////////////////////
// 读取操作
inline HRESULT	KAppFile::Load(LPCWSTR strFile)
{
	kis::KFileRead mapFile( strFile );

	if (!mapFile.Ok())
		return E_APP_FILE_OPEN_FAILED;

	INT		nSize			= mapFile.GetSize();
	BYTE*	pFileData		= mapFile;
	AppFileHeader* pHeader	= (AppFileHeader*)pFileData;

	if (pHeader->nMagic != APP_FILE_MAGIC)
		return E_APP_FILE_UNKNOWN_FMT;

	KRecord	appRule(pFileData + pHeader->nAppRuleOffset);
	if (!appRule.Ok())
		return E_APP_FILE_UNKNOWN_FMT;

	try
	{
		return ReadAppRules(&appRule);
	}
	catch (...)
	{
	}
	return E_FAIL;
}

inline HRESULT	KAppFile::ReadAppRules(KRecord* pRecord)
{
	ASSERT(pRecord->GetType() == AppRule_Root);

	KRecord ruleList = pRecord->FirstSubRecord();
	if (!ruleList.Ok())
		return E_APP_FILE_UNKNOWN_FMT;
	return ReadRuleList(&ruleList);
}

inline HRESULT	KAppFile::ReadRuleList(KRecord* pRecord)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == AppRule_RuleList);

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
			return E_APP_FILE_UNKNOWN_FMT;

		hr = ReadRule(&rule); 

		rule = rule.NextRecord();
	} 
	return S_OK;
}

inline HRESULT	KAppFile::ReadRule(KRecord* pRecord)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == AppRule_Rule);
	KAppRule* pRule = m_pAppMgr->CreateRule();

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
		{
			hr = E_APP_FILE_UNKNOWN_FMT;
			break;
		}

		switch (rule.GetType())
		{
		case AppRule_RuleID:
			{
				ASSERT(rule.GetDataSize() == sizeof(GUID));
				pRule->SetID(*(GUID*)rule.GetData());
			}
			break;
		case AppRule_ExePath:
			{
				pRule->SetExePath( (LPWSTR)rule.GetData() );
				kis::KPath path(pRule->GetExePath());
				if (!path.Exists())
				{
					hr = E_FAIL;
					goto EXIT0;
					//break;
				}
				if (m_pAppMgr->FindRule(pRule->GetExePath()) != NULL)
				{
					hr = E_FAIL;
					goto EXIT0;
				}
			}
			break;
		case AppRule_UserMode:
			{
				ASSERT(rule.GetDataSize() == sizeof(INT));
				pRule->SetUserMode(*(INT*)rule.GetData());
			}
			break;
		case AppRule_TrustMode:
			{
				ASSERT(rule.GetDataSize() == sizeof(INT));
				pRule->SetTrustMode(*(INT*)rule.GetData());
			}
			break;
		case AppRule_CheckSum:
			{
				ASSERT(rule.GetDataSize() == sizeof(pRule->m_checkSum));
				memcpy(pRule->m_checkSum, rule.GetData(), sizeof(pRule->m_checkSum));
			}
			break;
		case AppRule_LastFileTime:
			{
				ASSERT(rule.GetDataSize() == sizeof(FILETIME));
				pRule->SetLastFileTime( *(FILETIME*)rule.GetData() );
			}
			break;
		case AppRule_Creator:
			{
				pRule->SetCreator( (LPWSTR)rule.GetData() );
			}
			break;
		case AppRule_CreateDate:
			{
				ASSERT(rule.GetDataSize() == sizeof(FILETIME));
				pRule->SetCreateDate( *(FILETIME*)rule.GetData() );
			}
			break;
		case AppRule_CreateReason:
			{
				pRule->SetCreateReason( (LPWSTR)rule.GetData() );
			}
			break;
		case AppRule_LastModifyDate:
			{
				ASSERT(rule.GetDataSize() == sizeof(FILETIME));
				pRule->SetLastModifyDate( *(FILETIME*)rule.GetData() );
			}
			break;
		case AppRule_LastModifyer:
			{
				pRule->SetLastModifyer( (LPWSTR)rule.GetData() );
			}
			break;
		case AppRule_DenyPortList:
			{
				hr = ReadDenyPortList( &rule , pRule);
				if (FAILED(hr))
					break;
			}
			break;
		}
		rule = rule.NextRecord();
	}


EXIT0:
	
	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(m_pAppMgr->AddRule(pRule)))
			return hr;
	}
	delete pRule;
	return hr;
}

inline HRESULT KAppFile::ReadDenyPortList(KRecord* pRecord, KAppRule* pRule)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == AppRule_DenyPortList);

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
		{
			hr = E_APP_FILE_UNKNOWN_FMT;
			break;
		}

		switch (rule.GetType())
		{
		case AppRule_TcpRemotePortList:
			{
				ASSERT(!(rule.GetDataSize() % sizeof(KPortRange)));
				KPortRange* pRangeStart = (KPortRange*)rule.GetData();
				INT nSize = rule.GetDataSize() / sizeof(KPortRange);
				KPortRange* pRangeEnd = pRangeStart + nSize;
				pRule->m_tcpRemoteDenyRange.clear();
				pRule->m_tcpRemoteDenyRange.resize(nSize);
				std::copy(pRangeStart, pRangeEnd, pRule	->m_tcpRemoteDenyRange.begin());
			}
			break;
		case AppRule_TcpLocalPortList:
			{
				ASSERT(!(rule.GetDataSize() % sizeof(KPortRange)));
				KPortRange* pRangeStart = (KPortRange*)rule.GetData();
				INT nSize = rule.GetDataSize() / sizeof(KPortRange);
				KPortRange* pRangeEnd = pRangeStart + nSize;
				pRule->m_tcpLocalDenyRange.clear();
				pRule->m_tcpLocalDenyRange.resize(nSize);
				std::copy(pRangeStart, pRangeEnd, pRule	->m_tcpLocalDenyRange.begin());
			}
			break;
		case AppRule_UdpRemotePortList:
			{
				ASSERT(!(rule.GetDataSize() % sizeof(KPortRange)));
				KPortRange* pRangeStart = (KPortRange*)rule.GetData();
				INT nSize = rule.GetDataSize() / sizeof(KPortRange);
				KPortRange* pRangeEnd = pRangeStart + nSize;
				pRule->m_udpRemoteDenyRange.clear();
				pRule->m_udpRemoteDenyRange.resize(nSize);
				std::copy(pRangeStart, pRangeEnd, pRule	->m_udpRemoteDenyRange.begin());
			}
			break;
		case AppRule_UdpLocalPortList:
			{
				ASSERT(!(rule.GetDataSize() % sizeof(KPortRange)));
				KPortRange* pRangeStart = (KPortRange*)rule.GetData();
				INT nSize = rule.GetDataSize() / sizeof(KPortRange);
				KPortRange* pRangeEnd = pRangeStart + nSize;
				pRule->m_udpLocalDenyRange.clear();
				pRule->m_udpLocalDenyRange.resize(nSize);
				std::copy(pRangeStart, pRangeEnd, pRule	->m_udpLocalDenyRange.begin());
			}
			break;
		}
		rule = rule.NextRecord();
	}
	return hr;
}

//////////////////////////////////////////////////////////////////////////
// 保存操作
inline HRESULT KAppFile::Save(LPCWSTR strFile)
{
	KMemWriteProxy proxy;
	KRecordWriter root(&proxy, TRUE, 0, AppRule_Root);
	SaveRuleList(&root);
	root.EndRecord();

	INT nSize = proxy.GetCurPos();
	INT nFileSize = nSize + sizeof(AppFileHeader);

	kis::KFileWrite mapFile(strFile, nFileSize);
	if (!mapFile.Ok())
		return E_APP_FILE_SAVE_FAILED;

	BYTE* pBuf = mapFile;
	AppFileHeader* pHeader = (AppFileHeader*)pBuf;
	pHeader->cbSize = sizeof(AppFileHeader);
	pHeader->nMagic = APP_FILE_MAGIC;
	pHeader->nVer = 0;
	pHeader->nAppRuleOffset = sizeof(AppFileHeader);

	memcpy(pBuf + pHeader->nAppRuleOffset, proxy.GetBuf(0), nSize);
	return S_OK;
}

inline HRESULT	KAppFile::SaveRuleList(KRecordWriter* pWriter)
{
	KRecordWriter* pRuleList = pWriter->StartCompRecord(AppRule_RuleList);
	for (int i = 0; i < m_pAppMgr->GetRuleCnt(); i++)
	{
		KAppRule* pRule = m_pAppMgr->GetRule(i);
		KRecordWriter* pRuleWriter = pRuleList->StartCompRecord(AppRule_Rule);
		SaveRule(pRuleWriter, pRule);

		pRuleWriter->EndRecord();
		delete pRuleWriter;
	}

	pRuleList->EndRecord();
	delete pRuleList;
	return S_OK;
}

inline HRESULT KAppFile::SaveRule(KRecordWriter* pWriter, KAppRule* pRule)
{
	// 保存id
	if (pRule->GetID())
		SaveStruct(pRule->GetID(), pWriter, AppRule_RuleID);

	// 保存exepath
	if (pRule->GetExePath())
		SaveString(pRule->GetExePath(), pWriter, AppRule_ExePath);

	INT nUserMode = pRule->GetUserMode();
	INT nTrustMode = pRule->GetTrustMode();
	SaveStruct(&nUserMode, pWriter, AppRule_UserMode);
	SaveStruct(&nTrustMode, pWriter, AppRule_TrustMode);
	SaveArray(pRule->m_checkSum, countof(pRule->m_checkSum), pWriter, AppRule_CheckSum);
	SaveStruct(pRule->GetLastFileTime(), pWriter, AppRule_LastFileTime);
	
	// 保存creator
	if (pRule->GetCreator())
		SaveString(pRule->GetCreator(), pWriter, AppRule_Creator);

	if (pRule->GetCreateDate())
		SaveStruct(pRule->GetCreateDate(), pWriter, AppRule_CreateDate);

	if (pRule->GetCreateReason())
		SaveString(pRule->GetCreateReason(), pWriter, AppRule_CreateReason);

	if (pRule->GetLastModifyDate())
		SaveStruct(pRule->GetLastModifyDate(), pWriter, AppRule_LastModifyDate);

	if (pRule->GetLastModifyer())
		SaveString(pRule->GetLastModifyer(), pWriter, AppRule_LastModifyer);

	KRecordWriter* pDenyList = pWriter->StartCompRecord(AppRule_DenyPortList);
	if (pRule->m_tcpRemoteDenyRange.size() > 0)
	{
		KRecordWriter* pRangeList = pDenyList->StartRecord(AppRule_TcpRemotePortList);
		KPortRange* pRange = &pRule->m_tcpRemoteDenyRange[0];
		pRangeList->Write(pRange, (INT)pRule->m_tcpRemoteDenyRange.size());
		pRangeList->EndRecord();
		delete pRangeList;
	}
	if (pRule->m_tcpLocalDenyRange.size() > 0)
	{
		KRecordWriter* pRangeList = pDenyList->StartRecord(AppRule_TcpLocalPortList);
		KPortRange* pRange = &pRule->m_tcpLocalDenyRange[0];
		pRangeList->Write(pRange, (INT)pRule->m_tcpLocalDenyRange.size());
		pRangeList->EndRecord();
		delete pRangeList;
	}
	if (pRule->m_udpRemoteDenyRange.size() > 0)
	{
		KRecordWriter* pRangeList = pDenyList->StartRecord(AppRule_UdpRemotePortList);
		KPortRange* pRange = &pRule->m_udpRemoteDenyRange[0];
		pRangeList->Write(pRange, (INT)pRule->m_udpRemoteDenyRange.size());
		pRangeList->EndRecord();
		delete pRangeList;
	}
	if (pRule->m_udpLocalDenyRange.size() > 0)
	{
		KRecordWriter* pRangeList = pDenyList->StartRecord(AppRule_UdpLocalPortList);
		KPortRange* pRange = &pRule->m_udpLocalDenyRange[0];
		pRangeList->Write(pRange, (INT)pRule->m_udpLocalDenyRange.size());
		pRangeList->EndRecord();
		delete pRangeList;
	}
	pDenyList->EndRecord();
	delete pDenyList;

	return S_OK;
}

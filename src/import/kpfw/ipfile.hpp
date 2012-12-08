////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : ipfileimpl.hpp
//      Version   : 1.0
//      Comment   : 网镖ip规则文件读写
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////


inline HRESULT KIpFile::Load(LPWSTR strFile)
{
	kis::KFileRead mapFile( strFile );

	if (!mapFile.Ok())
		return E_IP_FILE_OPEN_FAILED;

	INT		nSize			= mapFile.GetSize();
	BYTE*	pFileData		= mapFile;
	IPFileHeader* pHeader	= (IPFileHeader*)pFileData;

	if (pHeader->nMagic != IP_FILE_MAGIC)
		return E_IP_FILE_UNKNOWN_FMT;

	KRecord	appRule(pFileData + pHeader->nIpRuleOffset);
	if (!appRule.Ok())
		return E_IP_FILE_UNKNOWN_FMT;

	try
	{
		return ReadIpRules(&appRule);
	}
	catch(...)
	{
	}
	return E_FAIL;
}

inline HRESULT KIpFile::Save(LPWSTR strFile)
{
	KMemWriteProxy proxy;

	// 保存数据到内存中
	KRecordWriter root(&proxy, TRUE, 0, IpRule_Root);
	SaveIpRules(&root);
	root.EndRecord();

	INT nSize = proxy.GetCurPos();
	INT nFileSize = nSize + sizeof(IPFileHeader);

	kis::KFileWrite mapFile(strFile, nFileSize);
	if (!mapFile.Ok())
		return E_IP_FILE_SAVE_FAILED;

	BYTE* pBuf = mapFile;
	IPFileHeader* pHeader = (IPFileHeader*)pBuf;
	pHeader->cbSize = sizeof(IPFileHeader);
	pHeader->nMagic = IP_FILE_MAGIC;
	pHeader->nVer = 0;
	pHeader->nIpRuleOffset = sizeof(IPFileHeader);

	memcpy(pBuf + pHeader->nIpRuleOffset, proxy.GetBuf(0), nSize);
	return S_OK;

}

inline HRESULT KIpFile::ReadIpRules(KRecord* pRecord)
{
	ASSERT(pRecord->GetType() == IpRule_Root);

	KRecord ruleList = pRecord->FirstSubRecord();
	if (!ruleList.Ok())
		return E_IP_FILE_UNKNOWN_FMT;
	return ReadRuleList(&ruleList);
}



inline HRESULT KIpFile::ReadRuleList(KRecord* pRecord)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == IpRule_RuleList);

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
			return E_IP_FILE_UNKNOWN_FMT;

		hr = ReadRule(&rule); 
		if (!SUCCEEDED(hr))
			return hr;

		rule = rule.NextRecord();
	} 
	return hr;
}

inline HRESULT KIpFile::ReadRule(KRecord* pRecord)
{
	HRESULT hr = S_OK;
	ASSERT(pRecord->GetType() == IpRule_Rule);
	KIpRule* pRule = m_pIPMgr->CreateRule();
	GUID id = *pRule->GetID();

	KRecord rule = pRecord->FirstSubRecord();
	while (pRecord->IsSubRecord(rule)) 
	{
		if (!rule.Ok())
		{
			hr = E_IP_FILE_UNKNOWN_FMT;
			break;
		}

		switch (rule.GetType())
		{
		case IpRule_RuleName:
			{
				pRule->SetName( (LPWSTR)rule.GetData() );
			}
			break;
		case IpRule_RuleDesc:
			{
				pRule->SetDesc( (LPWSTR)rule.GetData() );
			}
			break;
		case IpRule_Valid:
			{
				pRule->SetValid(*(BOOL*)rule.GetData());
			}
			break;
		case IpRule_Creator:
			{
				pRule->SetCreator( (LPWSTR)rule.GetData() );
			}
			break;
		case IpRule_CreateDate:
			{
				ASSERT(rule.GetDataSize() == sizeof(FILETIME));
				pRule->SetCreateDate( *(FILETIME*)rule.GetData() );
			}
			break;
		case IpRule_CreateReason:
			{
				pRule->SetCreateReason( (LPWSTR)rule.GetData() );
			}
			break;
		case IpRule_LastModifyDate:
			{
				ASSERT(rule.GetDataSize() == sizeof(FILETIME));
				pRule->SetLastModifyDate( *(FILETIME*)rule.GetData() );
			}
			break;
		case IpRule_LastModifyer:
			{
				pRule->SetLastModifyer( (LPWSTR)rule.GetData() );
			}
			break;
		case IpRule_FilterInfo:
			{
				ASSERT(rule.GetDataSize() == sizeof(KIP_FILTER_INFO));
				pRule->SetFilterInfo( (KIP_FILTER_INFO*)rule.GetData() );
				pRule->SetID(id);
			}
			break;
		}
		rule = rule.NextRecord();
	}

	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(m_pIPMgr->AddRule(pRule)))
			return hr;
	}
	delete pRule;
	return hr;
}

inline HRESULT KIpFile::SaveIpRules(KRecordWriter* pWriter)
{
	KRecordWriter* pRuleList = pWriter->StartCompRecord(IpRule_RuleList);
	for (int i = 0; i < m_pIPMgr->GetRuleCnt(); i++)
	{
		KIpRule* pRule = m_pIPMgr->GetRule(i);
		KRecordWriter* pRuleWriter = pRuleList->StartCompRecord(IpRule_Rule);
		SaveIpRule(pRuleWriter, pRule);

		pRuleWriter->EndRecord();
		delete pRuleWriter;
	}

	pRuleList->EndRecord();
	delete pRuleList;
	return S_OK;
}
inline HRESULT KIpFile::SaveIpRule(KRecordWriter* pWriter, KIpRule* pRule)
{
	if (pRule->GetName())
		SaveString(pRule->GetName(), pWriter, IpRule_RuleName);

	if (pRule->GetDesc())
		SaveString(pRule->GetDesc(), pWriter, IpRule_RuleDesc);

	KRecordWriter* pStructWriter = pWriter->StartRecord(IpRule_Valid);
	BOOL b = pRule->GetValid();
	pStructWriter->Write(b);
	pStructWriter->EndRecord();
	delete pStructWriter;

	// 保存creator
	if (pRule->GetCreator())
		SaveString(pRule->GetCreator(), pWriter, IpRule_Creator);

	if (pRule->GetCreateDate())
		SaveStruct(pRule->GetCreateDate(), pWriter, IpRule_CreateDate);

	if (pRule->GetCreateReason())
		SaveString(pRule->GetCreateReason(), pWriter, IpRule_CreateReason);

	if (pRule->GetLastModifyDate())
		SaveStruct(pRule->GetLastModifyDate(), pWriter, IpRule_LastModifyDate);

	if (pRule->GetLastModifyer())
		SaveString(pRule->GetLastModifyer(), pWriter, IpRule_LastModifyer);

	if (pRule->GetFilterInfo())
		SaveStruct(pRule->GetFilterInfo(), pWriter, IpRule_FilterInfo);

	return S_OK;
}
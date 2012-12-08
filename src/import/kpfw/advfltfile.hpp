////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : advflt.hpp
//      Version   : 1.0
//      Comment   : 网镖高级包过滤配置文件
//      s
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

inline HRESULT KAdvFltFile::Load(LPCWSTR strFile)
{
	if (!m_pFile)
		m_pFile = new kis::KFileRead(strFile);

	if (!m_pFile->Ok())
		return E_IP_FILE_OPEN_FAILED;

	INT		nSize			= m_pFile->GetSize();
	BYTE*	pFileData		= m_pFile->operator BYTE*();

	ADVFLTFileHeader* pHeader	= (ADVFLTFileHeader*)pFileData;

	if (pHeader->nMagic != ADVIP_FILE_MAGIC)
	{
		delete m_pFile;
		m_pFile = NULL;
		return E_IP_FILE_UNKNOWN_FMT;	
	}

	// 计算校验和
	BYTE*	pData = pFileData + sizeof(ADVFLTFileHeader);
	INT		nDataSize = nSize - sizeof(ADVFLTFileHeader);

	BYTE checksum[16] = {0};
	kis::KHashAlg().Md5(pData, nDataSize, checksum);

	if (memcmp(checksum, pHeader->CheckSum, sizeof(checksum)) != 0)
	{
		delete m_pFile;
		m_pFile = NULL;
		return E_IP_FILE_UNKNOWN_FMT;
	}
	return S_OK;
}

inline INT	KAdvFltFile::GetTemplateCnt()
{
	if (!m_pFile)
		return 0;
	BYTE*	pFileData		= m_pFile->operator BYTE*();
	ADVFLTFileHeader* pHeader	= (ADVFLTFileHeader*)pFileData;
	return pHeader->nAdvFltCnt;
}

inline KPackTemplate* KAdvFltFile::GetFirstTemplate()
{
	if (!m_pFile)
		return 0;
	BYTE*	pFileData		= m_pFile->operator BYTE*();
	
	return (KPackTemplate*)(pFileData + sizeof(ADVFLTFileHeader));
}

inline HRESULT KAdvFltFile::Save(LPCWSTR strFile, KPackTemplate* pTemplate, INT nBufSize, INT nCnt)
{
	ADVFLTFileHeader header = {0};
	header.cbSize = sizeof(header) + nBufSize;
	header.nAdvFltCnt = nCnt;
	header.nIpRuleOffset = sizeof(header);
	header.nMagic = ADVIP_FILE_MAGIC;
	header.nVer = 1;
	kis::KHashAlg().Md5((BYTE*)pTemplate, nBufSize, header.CheckSum);

	kis::KFileWrite mapFile(strFile, header.cbSize);
	BYTE* pBuf = mapFile;
	memcpy(pBuf, &header, sizeof(header));
	pBuf += sizeof(header);
	memcpy(pBuf, (BYTE*)pTemplate, nBufSize);

	return S_OK;
}

inline HRESULT KAdvFltFile::Save(LPCWSTR strFile, IKAdvTemplateList* pTempList)
{
	ADVFLTFileHeader header = {0};
	INT nBufSize = pTempList->GetBufferSize();
	header.cbSize = sizeof(header) + nBufSize;
	header.nAdvFltCnt = pTempList->GetTemplateCnt();
	header.nIpRuleOffset = sizeof(header);
	header.nMagic = ADVIP_FILE_MAGIC;
	header.nVer = 1;
	
	kis::KFileWrite mapFile(strFile, header.cbSize);
	BYTE* pBuf = mapFile;
	// memcpy(pBuf, &header, sizeof(header));
	pBuf += sizeof(header);

	pTempList->WriteBuffer(pBuf);
	// memcpy(pBuf, (BYTE*)pTemplate, nBufSize);

	kis::KHashAlg().Md5(pBuf, nBufSize, header.CheckSum);
	memcpy((BYTE*)mapFile, &header, sizeof(header));
	return S_OK;
}
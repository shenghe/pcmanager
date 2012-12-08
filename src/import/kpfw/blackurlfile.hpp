////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : blackurlfile.cpp
//      Version   : 1.0
//      Comment   : 恶意网址认证文件管理
//      
//      Create at : 2009-6-30
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

inline HRESULT KBlackUrlFile::Load(LPWSTR strFile)
{
	m_pFile = new kis::KFileRead( strFile );
	if (!m_pFile)
		return E_FAIL;

	if (!m_pFile->Ok())
	{
		delete m_pFile;
		m_pFile = NULL;
		return E_IP_FILE_OPEN_FAILED;
	}

	INT		nSize			= m_pFile->GetSize();
	BYTE*	pFileData		= m_pFile->operator BYTE*();

	BlackUrlFileHeader* pHeader	= (BlackUrlFileHeader*)pFileData;

	if (pHeader->nMagic != BLACKURL_FILE_MAGIC)
	{
		delete m_pFile;
		m_pFile = NULL;
		return E_BADURL_FILE_UNKNOWN_FMT;
	}

	m_pFileHeader = pHeader;
	m_pUrlItem = (BlackUrlItem*)(pFileData + pHeader->BlackUrlData);
	m_strFileName = strFile;
	return S_OK;
}

inline HRESULT	KBlackUrlFile::Reload()
{
	if (m_strFileName.GetLength() <= 0)
		return E_FAIL;

	kis::KFileRead* pFile = new kis::KFileRead( m_strFileName );
	if (!pFile)
		return E_FAIL;

	if (!pFile->Ok())
	{
		delete pFile;
		pFile = NULL;
		return E_IP_FILE_OPEN_FAILED;
	}

	INT		nSize			= pFile->GetSize();
	BYTE*	pFileData		= pFile->operator BYTE*();

	BlackUrlFileHeader* pHeader	= (BlackUrlFileHeader*)pFileData;

	if (pHeader->nMagic != BLACKURL_FILE_MAGIC)
	{
		delete pFile;
		pFile = NULL;
		return E_BADURL_FILE_UNKNOWN_FMT;
	}

	m_pFile = pFile;
	m_pFileHeader = pHeader;
	m_pUrlItem = (BlackUrlItem*)(pFileData + pHeader->BlackUrlData);
	return S_OK;
}

inline	HRESULT	KBlackUrlFile::SaveToFile(BlackUrlItem* pItems, INT nCnt, string& version, LPCWSTR strFile)
{
	BlackUrlFileHeader header = {0};
	header.cbSize = sizeof(header);
	header.nMagic = BLACKURL_FILE_MAGIC;
	header.BlackUrlCnt = nCnt;
	header.BlackUrlData = sizeof(header);
	if (version.length() > sizeof(header.strLastVersion)-1)
		return E_FAIL;
	strcpy(header.strLastVersion, version.c_str());
	header.nVer = 1;

	INT nTotalSize = sizeof(header) + nCnt * sizeof(BlackUrlItem);
	kis::KFileWrite file(strFile, nTotalSize);
	if (!file.Ok())
		return E_FAIL;

	BYTE* pByte = file;
	memcpy(pByte, &header, sizeof(header));
	memcpy(pByte+sizeof(header), pItems, nCnt*sizeof(BlackUrlItem));
	return S_OK;
}

inline BOOL	 KBlackUrlFile::Find(BYTE md5[16], INT& nType)
{
	if (!m_pUrlItem || !m_pFileHeader)
		return FALSE;
	
	// 二分查找
	INT total = m_pFileHeader->BlackUrlCnt;
	INT i = 0, j = total-1;
	while (i <= j)
	{
		INT nCmp = (i+j)/2;
		INT nRes = memcmp(md5, m_pUrlItem[nCmp].md5, sizeof(md5));
		if (nRes == 0)
		{
			nType = m_pUrlItem[nCmp].trustLevel;
			return TRUE;
		}
		else if (nRes < 0)
		{
			j = nCmp-1;
		}
		else
		{
			i = nCmp+1;
		}
	}

	return FALSE;
}

inline const char* KBlackUrlFile::GetVersion()
{
	return m_pFileHeader ? m_pFileHeader->strLastVersion : NULL;
}

inline	HRESULT	KBlackUrlFile::BuildMergeItems(LPCWSTR strFile, KBlackUrlMergeItemVec& items,string& version)
{
	ifstream fin(strFile);
	if (fin.bad())
		return E_FAIL; 

	fin>>version;
	if (version.length() <= 0)
		return E_FAIL; 

	while (!fin.eof())
	{
		string sign;
		string md5;
		string type;

		fin >> sign;
		if (sign.length() <= 0)
			break;

		fin >> md5;
		if (md5.length() <= 0)
			break;

		fin >> type;
		if (type.length() <= 0)
			break;

		BlackUrlMergeItem item = {0};
		if (sign == "-")
			item.itemToDel = TRUE;
		else if (sign == "+")
			item.itemToDel = FALSE;
		else
			continue;

		if (!ParseFromMD5String(md5, item.item.md5))
			continue;
		
		if (type == "3")
			item.item.trustLevel = BlackUrl_Warning;
		else if (type == "4")
			item.item.trustLevel = BlackUrl_Danger;

		*items.NewItem() = item;
	}
	return S_OK; 
}

inline HRESULT KBlackUrlFile::MergePackageFile(LPCWSTR strFile, LPCWSTR orgFile)
{
	string	version;
	KBlackUrlMergeItemVec items;
	if (FAILED(BuildMergeItems(strFile, items, version)))
		return E_FAIL;

	if (items.GetCnt() <= 0)
		return E_FAIL;

	items.Sort();

	// 两堆数据合并
	INT nOrgCnt = m_pFileHeader ? m_pFileHeader->BlackUrlCnt : 0;
	INT nMergeCnt = items.GetCnt();

	INT nTotalCnt = nOrgCnt + nMergeCnt;
	INT nResCnt = 0;
	BlackUrlItem* pResItems = new BlackUrlItem[nTotalCnt];
	if (!pResItems)	return E_FAIL;

	INT nOrgCnti = 0, nMergeCnti = 0;
	BlackUrlItem* pOrgItems = m_pUrlItem ? m_pUrlItem : NULL;
	BlackUrlMergeItem* pMergeItems = items.GetItem(0);

	while ((nOrgCnti < nOrgCnt) && (nMergeCnti < nMergeCnt))
	{
		INT nCmp = memcmp(pOrgItems[nOrgCnti].md5, pMergeItems[nMergeCnti].item.md5, sizeof(pOrgItems[nOrgCnti].md5));
		if (nCmp < 0)
		{
			pResItems[nResCnt++] = pOrgItems[nOrgCnti++];
		}
		else if (nCmp == 0)
		{
			if (!pMergeItems[nMergeCnti].itemToDel)
				pResItems[nResCnt++] = pOrgItems[nOrgCnti];
			nOrgCnti++;
			nMergeCnti++;
		}
		else
		{
			if (!pMergeItems[nMergeCnti].itemToDel)
				pResItems[nResCnt++] = pMergeItems[nMergeCnti].item;
			nMergeCnti++;
		}
	}

	while (nOrgCnti < nOrgCnt)
	{
		pResItems[nResCnt++] = pOrgItems[nOrgCnti++];
	}

	while (nMergeCnti < nMergeCnt)
	{
		if (!pMergeItems[nMergeCnti].itemToDel)
			pResItems[nResCnt++] = pMergeItems[nMergeCnti].item;
		nMergeCnti++;
	}

	// 保存到文件
	WCHAR tmpPath[MAX_PATH+1] = {0};
	::GetTempPath(MAX_PATH, tmpPath);
	WCHAR tmpFile[MAX_PATH+1] = {0};
	::GetTempFileName(tmpPath, TEXT("blkurl"), 0, tmpFile);

	SaveToFile(pResItems, nResCnt, version, tmpFile);
	delete [] pResItems;

	if (m_pFile)
	{
		delete m_pFile;
		m_pFile = NULL;
	}

	LPCWSTR strNewFile = (m_strFileName.GetLength() > 0) ? ((LPCWSTR)m_strFileName) : orgFile;

	::CopyFile(tmpFile, strNewFile, FALSE);
	::DeleteFile(tmpFile);
	
	Load((LPWSTR)strNewFile);
	return S_OK;
}

inline BOOL KBlackUrlFile::ParseFromMD5String( 
	const string&  strIn,
	BYTE            md5bin[16])
{
	if (strIn.length() < 32)
	{
		return FALSE;
	}

	for (int i = 0; i < 16; ++i)
	{
		BYTE byteValue = 0;
		BYTE hexDigit  = 0;

		if (!IsHexNum(strIn[i * 2], &hexDigit))
		{   
			return FALSE;
		}
		byteValue = hexDigit << 4;

		if (!IsHexNum(strIn[i * 2 + 1], &hexDigit))
		{
			return FALSE;
		}
		byteValue |= hexDigit;

		md5bin[i] = byteValue;
	}

	return TRUE;
}

inline BOOL KBlackUrlFile::IsHexNum(char ch, BYTE* pHexDigit)
{
	int hexDigit = 0;

	if (ch >= ('a') && ch <= ('f'))
	{
		hexDigit = ch - _T('a') + 10;
	}
	else if (ch >= ('A') && ch <= ('F'))
	{
		hexDigit = ch - ('A') + 10;
	}
	else if (ch >= ('0') && ch <= ('9'))
	{
		hexDigit = ch - ('0');
	}
	else
	{
		return FALSE;
	}

	assert(hexDigit <= UCHAR_MAX && hexDigit >= 0);
	if (pHexDigit)
	{
		*pHexDigit = (BYTE)hexDigit;
	}

	return TRUE;
}

#include "StdAfx.h"
#include "LDEditTxtFile.h"
#include "strsafe.h"
#include "miniutil/bkprocprivilege.h"
#define SAFE_DELETE_ARRAY_PTR(p)	if((p)){delete [](p);(p)=NULL;}

CLDEditTxtFile::CLDEditTxtFile(void)
{
	m_pStop = NULL;
	m_arrFileItem.RemoveAll();
}

CLDEditTxtFile::CLDEditTxtFile(CString strFilePath)
{
	m_pStop = NULL;
	m_arrFileItem.RemoveAll();
	m_strTxtFile = strFilePath;
}

CLDEditTxtFile::~CLDEditTxtFile(void)
{
	m_arrFileItem.RemoveAll();
}

int CLDEditTxtFile::_LoadTxtFileWithItems(CSimpleArray<CString>& arrFileItems)
{
	int nRet = -1;
	USES_CONVERSION;
	char pszFilePath[MAX_PATH] = {0};
	StringCbPrintfA(pszFilePath, sizeof(pszFilePath), "%s", W2A(m_strTxtFile));
	FILE* pFile = NULL;
	//open file
	fopen_s(&pFile, const_cast<char*>(pszFilePath), "r");
	if (NULL == pFile)
		return nRet;

	char pszFileValue[1024] = {0};
	ZeroMemory(pszFileValue, sizeof(pszFileValue));
	arrFileItems.RemoveAll();

	while(NULL != fgets(pszFileValue, sizeof(pszFileValue), pFile))
	{
		if (NULL != m_pStop && TRUE == *m_pStop)
			return nRet;

		arrFileItems.Add(CString(pszFileValue));
		ZeroMemory(pszFileValue, sizeof(pszFileValue));
	}

	if (NULL != pFile)
		fclose(pFile);
	pFile = NULL;
	nRet = arrFileItems.GetSize();

	return nRet;
}

CString CLDEditTxtFile::EnumItem(int nItem/* =0 */)
{
	int nCount = m_arrFileItem.GetSize();
	if (NULL < nItem || nItem >= nCount )
		return CString("");
	
	return m_arrFileItem[nItem];
}

int CLDEditTxtFile::GetAllItems(CSimpleArray<CString>& arrItems)
{
	arrItems.RemoveAll();
	arrItems = m_arrFileItem;

	return arrItems.GetSize();

}

int CLDEditTxtFile::WriteItemsToFile(CString strFile, CSimpleArray<CString> arrItems)
{
	int nRet = -1;
	USES_CONVERSION;

	CBkProcPrivilege privilege;

	if (!privilege.EnableShutdown())
		return -1;

	char pszFilePath[MAX_PATH] = {0};
	CString strDesFile = strFile;
	StringCbPrintfA(pszFilePath, sizeof(pszFilePath), "%s_tmp", W2A(strFile));
	
	DWORD dwFileAttr = ::GetFileAttributes(strFile);
//	dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;
	::SetFileAttributes(strFile, FILE_ATTRIBUTE_NORMAL);
	FILE* pFile = NULL;
	//open file
	fopen_s(&pFile, const_cast<char*>(pszFilePath), "w+b");
	if (NULL == pFile)
	{
		::SetFileAttributes(strFile, dwFileAttr);
		return nRet;
	}
	//write file
	int nCount = arrItems.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		if (NULL != m_pStop && TRUE == *m_pStop)
			break;

		CString strValue = arrItems[i];
		if (TRUE == strValue.IsEmpty())
			continue;
		fputs(CW2A(strValue.GetBuffer(-1)), pFile);
		if (strValue.Right(1) != "\n")
			fputs("\r\n", pFile);
		strValue.ReleaseBuffer(-1);
	}
	if (NULL != pFile)
		fclose(pFile);
	pFile = NULL;

	MoveFileEx(CString(pszFilePath), strDesFile, MOVEFILE_REPLACE_EXISTING);
	::SetFileAttributes(strDesFile, dwFileAttr);
	nRet = 0;

	return nRet;
}

int CLDEditTxtFile::_fgets(CSimpleArray<CString>& arrFileItems)
{
	int nRet = -1;
	USES_CONVERSION;
	arrFileItems.RemoveAll();
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(m_strTxtFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return nRet;
	char *pszFileBuffer = NULL;

	do 
	{
		DWORD dwFileSize = 0;
		dwFileSize = ::GetFileSize(hFile, NULL);
		if (0 == dwFileSize || dwFileSize > 1*1024*1024)//大于1M的文件默认任务失败
			break;

		dwFileSize += 1;
		pszFileBuffer = new char[dwFileSize];
		ZeroMemory(pszFileBuffer, dwFileSize);
		DWORD dwRetSize = 0;
		if (FALSE == ReadFile(hFile, pszFileBuffer, dwFileSize, &dwRetSize, NULL) || 0 == dwRetSize)
			break;
		int nFind = -1;
		CString strValue;//(CA2W(pszFileBuffer));
		strValue = CA2W(pszFileBuffer);
		if (FALSE == strValue.IsEmpty())
			strValue += TEXT("\r\n");

		nFind = strValue.Find('\n');
		while(nFind >= 0)
		{
			if (NULL != m_pStop && TRUE == *m_pStop)
				break;

			CString strTmp;
			strTmp = strValue.Left(nFind+1);
			arrFileItems.Add(strTmp);
			strValue = strValue.Mid(nFind+1);
			nFind = -1;
			nFind = strValue.Find('\n');
		}
		nRet = 0;

	} while (FALSE);

	if (INVALID_HANDLE_VALUE != hFile)
		CloseHandle(hFile);

	hFile = NULL;
	SAFE_DELETE_ARRAY_PTR(pszFileBuffer);

	return nRet;
}
BOOL* CLDEditTxtFile::SetStopFlag(BOOL* bFlag)
{
	BOOL* pFlag = m_pStop;

	m_pStop = bFlag;

	return pFlag;
}

int CLDEditTxtFile::LoadData()
{
	if (TRUE == PathFileExists(m_strTxtFile))
	{
		//_LoadTxtFileWithItems(m_arrFileItem);
		return _fgets(m_arrFileItem);
	}

	return 0;
}
#pragma once


struct TPairIdString
{
	TPairIdString(int nid, LPCTSTR szValue) : nID(nid), str(szValue)
	{ }

	TPairIdString(LPCTSTR szKey, LPCTSTR szValue)
	{
		nID = _ttoi(szKey);
		str = szValue;
	}
	BOOL GetKeyValue(CString &strKey, CString &strValue)
	{
		strKey.Format(_T("%d"), nID);
		strValue = str;
		return TRUE;
	}
	int		nID;
	CString str;
};


#define VULFIX_HOTFIX_ERROR				_T("hotfix_error") 
#define VULFIX_HOTFIX_ERROR_KEY_FMT		_T("KB_%d")

class CBeikeVulfixFileLog
{
public:
	CBeikeVulfixFileLog() 
	{
		m_bDirty = FALSE;
	}
	~CBeikeVulfixFileLog()
	{
		if(m_bDirty)
			Save();
	}
	
	BOOL Load();
	BOOL Load(CString sFileName);
	BOOL Save();
	BOOL CleanFiles(BOOL bCleanAll, LPCTSTR szFolder);	
	INT MoveFiles( LPCTSTR szPathFrom, LPCTSTR szPathTo, HWND hWnd=NULL );
	BOOL UpdateDownload(INT nKBID, LPCTSTR szFilename);
	BOOL UpdateInstalled(INT nKBID, BOOL bSuccess);
			
	INT IncHotfixError(INT nKBID);
	VOID ClearHotfixError(INT nKBID);
	
	INT64 GetDownloadFilesSize();
	INT64 GetDownloadFilesSize(CString strFileName);
	
protected:
	BOOL _DeleteAll(LPCTSTR szFolder);
	BOOL _DeleteAllDownloadSaveInIni( LPCTSTR szFolder );
	BOOL _DeleteAllDownloadWithPrefix( LPCTSTR szFolder );
	BOOL _DeleteInstalled(LPCTSTR szFolder);

protected:
	CSimpleArray<TPairIdString> m_DownloadedFiles;
	CSimpleArray<TPairIdString> m_InstallResults;
	CSimpleArray<TPairIdString> m_DownloadedFiles2;
	CSimpleArray<TPairIdString> m_InstallResults2;
	BOOL m_bDirty;
};


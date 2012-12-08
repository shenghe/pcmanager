
#pragma once

#include "runhelper.h"
#include <common/utility.h>
#include <atldef.h>
#include <globalstate/GetGlobalState.h>
#include <runoptimize/x64helper.h>

// 共存1，此处要修改否则与标准版的启动项优化冲突
#define KSAFE_DISABLE_HKLM_RUN_REG		_T("Software\\KSafe\\run\\Disable\\hklm")
#define KSAFE_DISABLE_HKCU_RUN_REG		_T("Software\\KSafe\\run\\Disable\\hkcu")
#define KSAFE_DISABLE_USER_RUN_DIR		_T("userrun")
#define KSAFE_DISABLE_COMM_RUN_DIR		_T("commrun")

#define S3XX_DISABLE_RUN_REG			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\360Disabled")

class CAutoBuffer
{
public:
	CAutoBuffer(){
		m_buffer = NULL;
		m_nSize	 = 0;
	}
	virtual ~CAutoBuffer(){
		_free();
	}

	PVOID AllocBuffer(DWORD nSize)
	{
	//	ASSERT(nSize!=0);

		_free();

		if (nSize!=0)
		{
			m_buffer = new char[nSize];
			m_nSize	 = nSize;
		}
		return m_buffer;
	}

	PVOID GetPtr()
	{
		return m_buffer;
	}

	DWORD GetSize()
	{
		return m_nSize;
	}

protected:
	void _free()
	{
		if (m_buffer!=NULL)
		{
			delete m_buffer;
			m_buffer = NULL;
		}
		m_nSize = 0;
	}
protected:
	PVOID	m_buffer;	
	DWORD	m_nSize;
};

class IPreOpcallBack
{
public:
	virtual BOOL WriteRunKey(HKEY hRoot, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData) = 0;
	virtual BOOL WriteRunFile(LPCTSTR lpFile) = 0;
};

class CRegRunOper
{
public:
	CRegRunOper(){}

	virtual ~CRegRunOper(){}

public:
	BOOL ControlRun(BOOL bEnable, DWORD& nRegType, LPCTSTR lpStrValue,IPreOpcallBack* pcallBack=NULL)
	{
		if (bEnable == KSRUN_START_ENABLE ||
			bEnable == KSRUN_START_DELAY_FAIL)
			return EnableRun(nRegType,lpStrValue,pcallBack);
		else
			return DisableRun(nRegType,lpStrValue,pcallBack);
	}

	BOOL ControlRun2(BOOL bEnable, LPCTSTR lpStrSid, DWORD& nRegType, LPCTSTR lpStrValue,IPreOpcallBack* pcallBack=NULL)
	{
		if (bEnable == KSRUN_START_ENABLE ||
			bEnable == KSRUN_START_DELAY_FAIL)
			return EnableRun2(nRegType, lpStrSid, lpStrValue, pcallBack);
		else
			return DisableRun(nRegType,lpStrValue,pcallBack);
	}

	BOOL DeleteRun(DWORD nRegType, LPCTSTR lpstrValue)
	{
		HKEY		hRoot;
		CString		strKey;
		DWORD samDesired;

		if (nRegType==REG_RUN_TYPE_KSAFE_HKLM)
		{
			hRoot  = HKEY_LOCAL_MACHINE;
			strKey = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKLM_RUN_REG).c_str();
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKCU)
		{
			hRoot  = HKEY_CURRENT_USER;
			strKey = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKCU_RUN_REG).c_str();
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKLM)
		{
			hRoot  = HKEY_LOCAL_MACHINE;
			strKey = STR_RUN_REG_KEY;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKCU)
		{
			hRoot  = HKEY_CURRENT_USER;
			strKey = STR_RUN_REG_KEY;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKLM64)
		{
			hRoot  = HKEY_LOCAL_MACHINE;
			strKey = KSAFE_DISABLE_HKLM_RUN_REG;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKCU64)
		{
			hRoot  = HKEY_CURRENT_USER;
			strKey = KSAFE_DISABLE_HKCU_RUN_REG;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKLM64)
		{
			hRoot  = HKEY_LOCAL_MACHINE;
			strKey = STR_RUN_REG_KEY;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKCU64)
		{
			hRoot  = HKEY_CURRENT_USER;
			strKey = STR_RUN_REG_KEY;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE;
		}

		BOOL bRes = _DeleteRegValue(hRoot, strKey, lpstrValue, samDesired);

		// 把自己目录下的干掉
		if (nRegType==REG_RUN_TYPE_WINDOW_HKLM)
		{
			hRoot  = HKEY_LOCAL_MACHINE;
			strKey = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKLM_RUN_REG).c_str();
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKCU)
		{
			hRoot  = HKEY_CURRENT_USER;
			strKey = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKCU_RUN_REG).c_str();
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKLM64)
		{
			hRoot  = HKEY_LOCAL_MACHINE;
			strKey = KSAFE_DISABLE_HKLM_RUN_REG;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKCU64)
		{
			hRoot  = HKEY_CURRENT_USER;
			strKey = KSAFE_DISABLE_HKCU_RUN_REG;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE;
		}

		_DeleteRegValue(hRoot, strKey, lpstrValue, samDesired);
		return bRes;
	}

protected:
	BOOL _DeleteRegValue(HKEY hRoot, LPCTSTR lpKey, LPCTSTR lpValue, DWORD samDesired)
	{
		HKEY hKey = NULL;
		DWORD dwRet = FALSE;

		if (ERROR_SUCCESS == (dwRet = ::RegOpenKeyEx(hRoot, lpKey, NULL, samDesired, &hKey)))
		{
			dwRet = ::RegDeleteValue(hKey, lpValue);
			::RegCloseKey(hKey);
		}

		return dwRet==ERROR_SUCCESS;
	}

	BOOL EnableRun(DWORD& nRegType, LPCTSTR lpStrValue,IPreOpcallBack* pcallBack)
	{
		DWORD	nNewRegtype = 0;
		HKEY	hRoot1;
		CString	strKey1;

		HKEY	hRoot2;
		CString	strKey2;
		DWORD samDesired;

		if (nRegType==REG_RUN_TYPE_KSAFE_HKLM)
		{
			hRoot1  = HKEY_LOCAL_MACHINE;
			strKey1 = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKLM_RUN_REG).c_str();

			hRoot2 = HKEY_LOCAL_MACHINE;

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKLM;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKCU)
		{
			hRoot1  = HKEY_CURRENT_USER;
			strKey1 = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKCU_RUN_REG).c_str();

			hRoot2 = HKEY_CURRENT_USER;

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKCU;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKLM64)
		{
			hRoot1  = HKEY_LOCAL_MACHINE;
			strKey1 = KSAFE_DISABLE_HKLM_RUN_REG;

			hRoot2 = HKEY_LOCAL_MACHINE;

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKLM64;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKCU64)
		{
			hRoot1  = HKEY_CURRENT_USER;
			strKey1 = KSAFE_DISABLE_HKCU_RUN_REG;

			hRoot2 = HKEY_CURRENT_USER;

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKCU64;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else
			return FALSE;

		strKey2 = STR_RUN_REG_KEY;

		if ( _MoveRegKey(hRoot1,strKey1,hRoot2,strKey2,lpStrValue,pcallBack, samDesired) )
		{
			nRegType = nNewRegtype;
			return TRUE;
		}
		else
			return FALSE;
	}

	BOOL EnableRun2(DWORD& nRegType, CString strUserSid, LPCTSTR lpStrValue,IPreOpcallBack* pcallBack)
	{
		DWORD	nNewRegtype = 0;
		HKEY	hRoot1;
		CString	strKey1;

		HKEY	hRoot2;
		CString	strKey2;
		DWORD samDesired;

		if (nRegType==REG_RUN_TYPE_KSAFE_HKLM)
		{
			hRoot1  = HKEY_LOCAL_MACHINE;
			strKey1 = KSAFE_DISABLE_HKLM_RUN_REG;

			hRoot2 = HKEY_LOCAL_MACHINE;

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKLM;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKCU)
		{
			if (strUserSid.IsEmpty())
			{
				hRoot1  = HKEY_USERS;
				strKey1 = strUserSid;
				strKey1.Append(KSAFE_DISABLE_HKCU_RUN_REG);

				hRoot2 = HKEY_USERS;
			} 
			else
			{
				hRoot1  = HKEY_CURRENT_USER;
				strKey1 = KSAFE_DISABLE_HKCU_RUN_REG;
				hRoot2  = HKEY_CURRENT_USER;
			}

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKCU;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKLM64)
		{
			hRoot1  = HKEY_LOCAL_MACHINE;
			strKey1 = KSAFE_DISABLE_HKLM_RUN_REG;

			hRoot2 = HKEY_LOCAL_MACHINE;

			nNewRegtype = REG_RUN_TYPE_WINDOW_HKLM64;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_KSAFE_HKCU64)
		{
			if (strUserSid.IsEmpty())
			{
				hRoot1  = HKEY_USERS;
				strKey1 = strUserSid;
				strKey1.Append(KSAFE_DISABLE_HKCU_RUN_REG);
				hRoot2 = HKEY_USERS;
			} 
			else
			{
				hRoot1  = HKEY_CURRENT_USER;
				strKey1	= KSAFE_DISABLE_HKCU_RUN_REG;
				hRoot2  = HKEY_CURRENT_USER;
			}
			
			nNewRegtype = REG_RUN_TYPE_WINDOW_HKCU64;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else
			return FALSE;

		strKey2 = STR_RUN_REG_KEY;

		if ( _MoveRegKey(hRoot1,strKey1,hRoot2,strKey2,lpStrValue,pcallBack, samDesired) )
		{
			nRegType = nNewRegtype;
			return TRUE;
		}
		else
			return FALSE;
	}

	BOOL DisableRun(DWORD& nRegType, LPCTSTR lpStrValue,IPreOpcallBack* pcallBack)
	{
		DWORD	nNewRegtype = 0;
		HKEY	hRoot1;
		CString	strKey1;

		HKEY	hRoot2;
		CString	strKey2;
		DWORD samDesired;

		if (nRegType==REG_RUN_TYPE_WINDOW_HKLM)
		{
			hRoot1  = HKEY_LOCAL_MACHINE;
			strKey1 = STR_RUN_REG_KEY;

			hRoot2 = HKEY_LOCAL_MACHINE;
			strKey2 = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKLM_RUN_REG).c_str();

			nNewRegtype = REG_RUN_TYPE_KSAFE_HKLM;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKCU)
		{
			hRoot1  = HKEY_CURRENT_USER;
			strKey1 = STR_RUN_REG_KEY;

			hRoot2  = HKEY_CURRENT_USER;
			strKey2 = KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKCU_RUN_REG).c_str();

			nNewRegtype = REG_RUN_TYPE_KSAFE_HKCU;
			samDesired = KEY_WOW64_32KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKLM64)
		{
			hRoot1  = HKEY_LOCAL_MACHINE;
			strKey1 = STR_RUN_REG_KEY;

			hRoot2 = HKEY_LOCAL_MACHINE;
			strKey2 = KSAFE_DISABLE_HKLM_RUN_REG;

			nNewRegtype = REG_RUN_TYPE_KSAFE_HKLM64;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else if (nRegType==REG_RUN_TYPE_WINDOW_HKCU64)
		{
			hRoot1  = HKEY_CURRENT_USER;
			strKey1 = STR_RUN_REG_KEY;

			hRoot2  = HKEY_CURRENT_USER;
			strKey2 = KSAFE_DISABLE_HKCU_RUN_REG;

			nNewRegtype = REG_RUN_TYPE_KSAFE_HKCU64;
			samDesired = KEY_WOW64_64KEY | KEY_SET_VALUE | KEY_QUERY_VALUE;
		}
		else
			return FALSE;

		if( _MoveRegKey(hRoot1,strKey1,hRoot2,strKey2,lpStrValue,pcallBack, samDesired) )
		{
			nRegType = nNewRegtype;
			return TRUE;
		}
		return FALSE;
	}

protected:
	BOOL _MoveRegKey(HKEY hRoot1, LPCTSTR strKey1, HKEY hRoot2, LPCTSTR strKey2, LPCTSTR lpStrValue, IPreOpcallBack* pCB=NULL, DWORD samDesired = KEY_SET_VALUE | KEY_QUERY_VALUE)
	{
 		DWORD		nType = 0;
		CAutoBuffer	atBuffer;
		DWORD		nSize = 0;
		DWORD		nRet = ERROR_SUCCESS;
		DWORD		nAllocSize = MAX_PATH;

		do 
		{
			atBuffer.AllocBuffer(nAllocSize);

			nAllocSize+= MAX_PATH;
			nType = 0;
			nSize = atBuffer.GetSize();

			HKEY hKey = NULL;

			if (ERROR_SUCCESS == (nRet = ::RegOpenKeyEx(hRoot1, strKey1, NULL, samDesired, &hKey)))
			{
				nRet = ::RegQueryValueEx(hKey, lpStrValue, NULL, &nType, (LPBYTE)atBuffer.GetPtr(), &nSize);
				::RegCloseKey(hKey);
			}

			if (nRet==ERROR_SUCCESS)
				break;
			else if (nRet!=ERROR_MORE_DATA)
				break;
		} while (TRUE);

		if (nRet==ERROR_SUCCESS)
		{
			if (pCB!=NULL && (nType==REG_SZ || nType==REG_EXPAND_SZ) )
			{
				if (!pCB->WriteRunKey(hRoot2,strKey2,lpStrValue,(LPCTSTR)atBuffer.GetPtr()) )
					return FALSE;
			}


			HKEY hKey = NULL;

			if ((nRet = ::RegCreateKeyEx(hRoot2, strKey2, NULL, NULL, REG_OPTION_NON_VOLATILE, samDesired | KEY_ALL_ACCESS , NULL, &hKey, NULL)) == ERROR_SUCCESS)
			{
				nRet = ::RegSetValueEx(hKey, lpStrValue, NULL, nType, (LPBYTE)atBuffer.GetPtr(), nSize);
				::RegCloseKey(hKey);
			}

			if (nRet==ERROR_SUCCESS)
			{
				if(_DeleteRegValue(hRoot1,strKey1,lpStrValue, samDesired))
				{
					nRet = ERROR_SUCCESS;
				}
				else
				{
					nRet = GetLastError();
				}

				if (nRet != ERROR_SUCCESS )
				{
					_DeleteRegValue(hRoot2,strKey2,lpStrValue, samDesired);
				}
			}
		}
		return (nRet==ERROR_SUCCESS);
	}
};

class IEnumRunFilter
{
public:
	virtual BOOL PreCopyFile(DWORD nType, LPCTSTR lpFileName) = 0;
	virtual BOOL PreCopyValue(DWORD nType, LPCTSTR lpValue) = 0;
};

class CKSRunDirEnumer : public CRunDirEnumer,public CCheck3XXInst
{
public:
	CKSRunDirEnumer(){
		m_pCopyFilter = NULL;
	}
	virtual ~CKSRunDirEnumer(){}

	void SetCopyFilter(IEnumRunFilter* pFilter)
	{
		m_pCopyFilter = pFilter;
	}
public:

	virtual BOOL DoEnum()
	{
		if (Is3XXInstalled())
			_PreEnum();

		CString	strDir;

		if (GetEnumDir(FALSE,strDir))
			_EnumRunDir(strDir, DIR_RUN_TYPE_KSAFE_COMM);
		if (GetEnumDir(TRUE,strDir))
			_EnumRunDir(strDir, DIR_RUN_TYPE_KSAFE_USER);

		return TRUE;
	}

	static BOOL GetEnumDir(BOOL bCurrentUser, CString& strpath)
	{

		if ( bCurrentUser )
		{
			TCHAR	szBuffer[MAX_PATH] = {0};
			if ( SHGetSpecialFolderPath(NULL,szBuffer,CSIDL_APPDATA,TRUE) )
			{
				CString	strDir = szBuffer;
				if (strDir.IsEmpty())
				{
					return FALSE;
				}
				strDir+=_T("\\KSafe\\");
				if (!PathFileExists(strDir))
				{
					::CreateDirectory(strDir,NULL);
				}
				if (PathIsDirectory(strDir))
				{
					strDir += KSAFE_DISABLE_USER_RUN_DIR;
				}
				if (!PathFileExists(strDir))
				{
					::CreateDirectory(strDir,NULL);
				}
				if (PathIsDirectory(strDir))
				{
					strpath = strDir;
					return TRUE;
				}
			}
			return FALSE;
		}
		else
		{
			CString	strCache;
			CAppPath::Instance().GetLeidianCachePath(strCache,TRUE);

			strCache.Append(_T("\\"));
			strCache.Append(KSAFE_DISABLE_COMM_RUN_DIR);
			strpath = strCache;
			if (!::PathIsDirectory(strCache))
			{
				return ::CreateDirectory(strCache,NULL);
			}
			else
				return TRUE;
		}
	}

protected:
	BOOL Get3XXDisDir(BOOL bCurrent, CString& strDir)
	{
		CString	strBuf;
		DWORD	nFlags = bCurrent?CSIDL_APPDATA:CSIDL_COMMON_APPDATA;

		if ( ::SHGetSpecialFolderPath(NULL,strBuf.GetBuffer(MAX_PATH),nFlags,FALSE) )
		{
			strBuf.ReleaseBuffer();
			strBuf += _T("\\360safe\\360Disabled");
			
			if (PathIsDirectory(strBuf))
			{
				strDir = strBuf;
				return TRUE;
			}
		}
		else
			strBuf.ReleaseBuffer();

		return FALSE;
	}

	virtual void _PreEnum()
	{
		if (TRUE)
		{
			CString	str3XDir;
			CString	strKSdir;
			
			if ( Get3XXDisDir(TRUE,str3XDir) && GetEnumDir(TRUE,strKSdir) )
			{
				_CopyDir(DIR_RUN_TYPE_KSAFE_USER,strKSdir,str3XDir);
			}
		}
		if (TRUE)
		{
			CString	str3XDir;
			CString	strKSdir;

			if ( Get3XXDisDir(FALSE,str3XDir) && GetEnumDir(FALSE,strKSdir) )
			{
				_CopyDir(DIR_RUN_TYPE_KSAFE_COMM,strKSdir,str3XDir);
			}
		}
	}

	void _CopyDir(DWORD nType, LPCTSTR lpDstDir, LPCTSTR lpSrcDir)
	{
		CEnumFile	enumer(lpSrcDir,_T("*.*"));

		for ( int i=0; i<enumer.GetFileCount(); i++)
		{
			CString strDstF = lpDstDir;
			strDstF.Append(_T("\\"));
			strDstF.Append(enumer.GetFileName(i));

			if (m_pCopyFilter && !m_pCopyFilter->PreCopyFile(nType,enumer.GetFileFullPath(i)))
				continue;

			if (!PathFileExists(strDstF))
			{
				CopyFile(enumer.GetFileFullPath(i),strDstF,TRUE);
			}
		}
	}

protected:
	IEnumRunFilter* m_pCopyFilter;
};


class CDirRunOper
{
public:
	CDirRunOper(){}
	virtual ~CDirRunOper(){}

public:

	BOOL ControlRun2(BOOL bEnable, DWORD& nRegType, LPCTSTR lpFileName,IPreOpcallBack* pcallBack=NULL)
	{
		CString		strName = lpFileName;
		if (nRegType>DIR_RUN_TYPE_BEGIN)
		{
			CString	strDir;
			if (nRegType==DIR_RUN_TYPE_WINDOW_COMM)
				strDir = GetGlobalState()->GetCommonStartRunDir();
			else if (nRegType==DIR_RUN_TYPE_WINDOW_USER)
				strDir = GetGlobalState()->GetUserStartRunDir();
			else if (nRegType==DIR_RUN_TYPE_KSAFE_COMM)
				CKSRunDirEnumer::GetEnumDir(FALSE,strDir);
			else if (nRegType==DIR_RUN_TYPE_KSAFE_USER)
				CKSRunDirEnumer::GetEnumDir(TRUE,strDir);
			
			if (strDir.IsEmpty())
				return FALSE;

			strDir.Append(_T("\\"));
			strDir.Append(strName);
			strName = strDir;
		}

		if (bEnable)
			return EnableRun(nRegType,strName,pcallBack);
		else
			return DisableRun(nRegType,strName);
	}

	BOOL ControlRun(BOOL bEnable, DWORD& nRegType, CString& strFilePath,IPreOpcallBack* pcallBack=NULL)
	{
		if (bEnable == KSRUN_START_ENABLE ||
			bEnable == KSRUN_START_DELAY_FAIL)
			return EnableRun(nRegType,strFilePath,pcallBack);
		else
			return DisableRun(nRegType,strFilePath);
	}

	BOOL DeleteRun(DWORD nRegType, LPCTSTR lpFileName)
	{
		if (nRegType==DIR_RUN_TYPE_WINDOW_USER || nRegType==DIR_RUN_TYPE_WINDOW_COMM )
		{
			CString	strFileName = GetFileName(lpFileName);
			CString	strBakFileName;

			if (nRegType==DIR_RUN_TYPE_WINDOW_COMM)
				CKSRunDirEnumer::GetEnumDir(FALSE,strBakFileName);
			else if (nRegType==DIR_RUN_TYPE_WINDOW_USER)
				CKSRunDirEnumer::GetEnumDir(TRUE,strBakFileName);

			strBakFileName.Append(_T("\\"));
			strBakFileName.Append(strFileName);

			::DeleteFile(strBakFileName);
		}
		return ::DeleteFile(lpFileName);
	}

protected:
	BOOL DisableRun(DWORD & nType,CString& strFileName)
	{
		CString	strNewFilePath;
		CString strOldFilePath;

		ATLASSERT(nType==DIR_RUN_TYPE_WINDOW_COMM||nType==DIR_RUN_TYPE_WINDOW_USER);

		if (nType==DIR_RUN_TYPE_WINDOW_COMM)
		{
			CKSRunDirEnumer::GetEnumDir(FALSE,strNewFilePath);
			strOldFilePath = GetGlobalState()->GetCommonStartRunDir();
		}
		else if(nType==DIR_RUN_TYPE_WINDOW_USER)
		{
			CKSRunDirEnumer::GetEnumDir(TRUE,strNewFilePath);
			strOldFilePath = GetGlobalState()->GetUserStartRunDir();
		}
		else
			return FALSE;

		strNewFilePath.Append(_T("\\"));
		strNewFilePath.Append(strFileName);
		strOldFilePath.Append(_T("\\"));
		strOldFilePath.Append(strFileName);

		if (::MoveFileEx(strOldFilePath,strNewFilePath,MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING))
		{
			nType = (nType==DIR_RUN_TYPE_WINDOW_COMM)?DIR_RUN_TYPE_KSAFE_COMM:DIR_RUN_TYPE_KSAFE_USER;
			strOldFilePath = strNewFilePath;
			strFileName = strNewFilePath;
			return TRUE;
		}
		return FALSE;
	}

	BOOL EnableRun(DWORD& nType,CString& strFileName,IPreOpcallBack* pcallBack=NULL)
	{
		CString	strNewFilePath;
		CString strOldFilePath;

		ATLASSERT(nType==DIR_RUN_TYPE_KSAFE_COMM||nType==DIR_RUN_TYPE_KSAFE_USER);

		if (nType==DIR_RUN_TYPE_KSAFE_COMM)
		{
			strNewFilePath = GetGlobalState()->GetCommonStartRunDir();
			CKSRunDirEnumer::GetEnumDir(FALSE,strOldFilePath);
		}
		else if(nType == DIR_RUN_TYPE_KSAFE_USER)
		{
			strNewFilePath = GetGlobalState()->GetUserStartRunDir();
			CKSRunDirEnumer::GetEnumDir(TRUE,strOldFilePath);
		}
		else
			return FALSE;

		strNewFilePath.Append(_T("\\"));
		strNewFilePath.Append(strFileName);
		strOldFilePath.Append(_T("\\"));
		strOldFilePath.Append(strFileName);

		if (pcallBack)
		{
			if (!pcallBack->WriteRunFile(strNewFilePath))
				return FALSE;
		}
		if (::MoveFileEx(strOldFilePath,strNewFilePath,MOVEFILE_COPY_ALLOWED))
		{
			nType = (nType==DIR_RUN_TYPE_KSAFE_COMM)?DIR_RUN_TYPE_WINDOW_COMM:DIR_RUN_TYPE_WINDOW_USER;
			strOldFilePath = strNewFilePath;
			strFileName = strNewFilePath;
			return TRUE;
		}
		return FALSE;
	}
};

class CKSRunRegEnumer : public CRunRegEnumer,public CCheck3XXInst
{
public:
	CKSRunRegEnumer(){
		m_pCopyFilter = NULL;
	}
	virtual ~CKSRunRegEnumer(){}

	void SetCopyFilter(IEnumRunFilter* pFilter)
	{
		m_pCopyFilter = pFilter;
	}
public:
	virtual void _PreEnum()
	{
		_Copy3XXToKSafe(REG_RUN_TYPE_KSAFE_HKLM, HKEY_LOCAL_MACHINE,KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKLM_RUN_REG).c_str(),HKEY_LOCAL_MACHINE,S3XX_DISABLE_RUN_REG, KEY_READ | KEY_WOW64_32KEY);
		_Copy3XXToKSafe(REG_RUN_TYPE_KSAFE_HKCU, HKEY_CURRENT_USER,KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKCU_RUN_REG).c_str(),HKEY_CURRENT_USER,S3XX_DISABLE_RUN_REG, KEY_READ | KEY_WOW64_32KEY);
		if (KWow64.IsWin64_2())
		{
			_Copy3XXToKSafe(REG_RUN_TYPE_KSAFE_HKLM64, HKEY_LOCAL_MACHINE,KSAFE_DISABLE_HKLM_RUN_REG,HKEY_LOCAL_MACHINE,S3XX_DISABLE_RUN_REG, KEY_READ | KEY_WOW64_64KEY);
			_Copy3XXToKSafe(REG_RUN_TYPE_KSAFE_HKCU64, HKEY_CURRENT_USER,KSAFE_DISABLE_HKCU_RUN_REG,HKEY_CURRENT_USER,S3XX_DISABLE_RUN_REG, KEY_READ | KEY_WOW64_64KEY);

		}
	}

	virtual BOOL DoEnum()
	{
		if (Is3XXInstalled())
			_PreEnum();

		_EnumRegRun(HKEY_LOCAL_MACHINE,KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKLM_RUN_REG).c_str(), REG_RUN_TYPE_KSAFE_HKLM, KEY_READ | KEY_WOW64_32KEY);
		_EnumRegRun(HKEY_CURRENT_USER,KOpKsafeReg::ReplaceRegStr(KSAFE_DISABLE_HKCU_RUN_REG).c_str(), REG_RUN_TYPE_KSAFE_HKCU, KEY_READ | KEY_WOW64_32KEY);
	
		if (KWow64.IsWin64_2())
		{
			_EnumRegRun(HKEY_LOCAL_MACHINE,KSAFE_DISABLE_HKLM_RUN_REG, REG_RUN_TYPE_KSAFE_HKLM64, KEY_READ | KEY_WOW64_64KEY);
			//HKEY_CURRENT_USER下不存在RUN项的重定向 _EnumRegRun(HKEY_CURRENT_USER,KSAFE_DISABLE_HKCU_RUN_REG, REG_RUN_TYPE_KSAFE_HKCU64, KEY_READ | KEY_WOW64_64KEY);
		}

		return TRUE;
	}

protected:

	virtual void _EnumRegRun(HKEY hRoot, LPCTSTR lpRunKey, DWORD nType, DWORD samDesired = KEY_READ)
	{
		HKEY	hKey = NULL;

		if (ERROR_SUCCESS==::RegOpenKeyEx(hRoot,lpRunKey,0,samDesired,&hKey))
		{
			DWORD	iIndex = 0;
			CString	strData;
			CString	strValue;
			while (SHEnumStringValue(hKey,iIndex++,strValue,strData)==ERROR_SUCCESS)
			{
				REG_RUN_INFO	runinfo;
				runinfo.strName	= strValue;
				runinfo.strPath	= strData;
				runinfo.nRegType= nType;
				runinfo.bDisabledBy3XX = IsDisabledBy3XX(strValue, samDesired);

				m_runArray.Add(runinfo);
				strData.Empty();
				strValue.Empty();
			}

			::RegCloseKey(hKey);
		}
	}

	BOOL IsDisabledBy3XX(CString strKeyValueName, DWORD samDesired = KEY_READ)
	{
		HKEY hKey = NULL;
		HKEY hRootKey = HKEY_LOCAL_MACHINE;
		HKEY hRootKey2 = HKEY_CURRENT_USER;
		BOOL bRet = FALSE;
		if (ERROR_SUCCESS==::RegOpenKeyEx( hRootKey,
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\360Disabled"),
			0,
			samDesired,
			&hKey))
		{
			DWORD	iIndex = 0;
			CString	strData;
			CString	strValue;
			while (SHEnumStringValue(hKey,iIndex++,strValue,strData)==ERROR_SUCCESS)
			{
				if (!strData.IsEmpty() && _tcslen(strData) > _tcslen(_T("rem ")) )
				{
					CString		strLeft = strData.Left(4);

					if (strValue.CompareNoCase(strKeyValueName) == 0 )
					{
						bRet = TRUE;
						goto Exit0;
					}
				}
				strData.Empty();
				strValue.Empty();
			}

			::RegCloseKey(hKey);
		}
		if (ERROR_SUCCESS==::RegOpenKeyEx( hRootKey2,
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\360Disabled"),
			0,
			samDesired,
			&hKey))
		{
			DWORD	iIndex = 0;
			CString	strData;
			CString	strValue;
			while (SHEnumStringValue(hKey,iIndex++,strValue,strData)==ERROR_SUCCESS)
			{
				if (!strData.IsEmpty() && _tcslen(strData) > _tcslen(_T("rem ")) )
				{
					CString		strLeft = strData.Left(4);

					if (strValue.CompareNoCase(strKeyValueName) == 0 )
					{
						bRet = TRUE;
						goto Exit0;
					}
				}
				strData.Empty();
				strValue.Empty();
			}

			::RegCloseKey(hKey);
		}
	Exit0:
		return bRet;
	}

	void _Copy3XXToKSafe(DWORD nRegType, HKEY hRootDst, LPCTSTR strKeyDst, HKEY hRootSrc, LPCTSTR strKeySrc, DWORD samDesired = KEY_READ)
	{
		HKEY	hKey = NULL;

		if (ERROR_SUCCESS==::RegOpenKeyEx(hRootSrc,strKeySrc,0,samDesired,&hKey))
		{
			DWORD	iIndex = 0;
			CString	strData;
			CString	strValue;
			while (SHEnumStringValue(hKey,iIndex++,strValue,strData)==ERROR_SUCCESS)
			{
				if (!strData.IsEmpty() && _tcslen(strData) > _tcslen(_T("rem ")) )
				{
					CString		strLeft = strData.Left(4);

					if (strLeft.CompareNoCase(_T("rem ")) == 0 )
					{
						CString	strNewData = strData.Mid(4);
						DWORD nType = REG_SZ;
						HKEY hSubKey = NULL;
						DWORD nRet = 0;

						if (ERROR_SUCCESS == ::RegOpenKeyEx(hRootDst, strKeyDst, NULL, samDesired | KEY_WRITE, &hSubKey))
						{
							nRet = ::RegQueryValueEx(hSubKey, strValue, NULL, &nType, NULL, NULL);
							if (nRet ==  ERROR_FILE_NOT_FOUND)
							{
								if ( !m_pCopyFilter || m_pCopyFilter->PreCopyValue(nRegType,strValue) )
								{						
									::RegSetValueEx(hSubKey, strValue, NULL, REG_SZ,(LPBYTE)strNewData.GetString(), (DWORD)(_tcslen(strNewData)*sizeof(TCHAR))); 
								}
							}
							::RegCloseKey(hSubKey);
						}

						//DWORD nRet	= ::SHGetValue(hRootDst,strKeyDst,strValue,&nType,NULL,0);

						//if (nRet!=ERROR_MORE_DATA)
						//{
						//	if ( !m_pCopyFilter || m_pCopyFilter->PreCopyValue(nRegType,strValue) )
						//		::SHSetValue(hRootDst,strKeyDst,strValue,REG_SZ,strNewData, (DWORD)(_tcslen(strNewData)*sizeof(TCHAR)));
						//}
					}
				}

				strData.Empty();
				strValue.Empty();
			}

			::RegCloseKey(hKey);
		}
	}
protected:
	IEnumRunFilter* m_pCopyFilter;
	KWow64Switcher KWow64;
};



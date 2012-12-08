/********************************************************************
* CreatedOn: 2006-9-1   11:52
* FileName: KSearchFile.h
* CreatedBy: qiuruifeng <qiuruifeng@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/
#ifndef KXESEARCHFILE_H
#define KXESEARCHFILE_H
#include <vector>
#include <string>
#include <io.h>
#include <cassert>
#include <Windows.h>
#include <tchar.h>
#include "../kxecommon/kxecommon.h"
#include "../kxeerror/kxeerror.h"


#define REGISTER_ENV_SUB_KEY			TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define ENV_VARIABLE_KEY				TEXT("Path")

#define REGISTER_CURRENT_VESION_SUB_KEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define PROGRAM_FILES_DIR_KEY			TEXT("ProgramFilesDir")

#if !defined(_UNICODE) && !defined(UNICODE)
typedef std::string std_tstring;
#else
typedef std::wstring std_tstring;
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

class KxESearchFile
{
public:
// 	KxESearchFile():m_nNextEnvDirId(0)
// 	{
// 	}

	KxESearchFile(const TCHAR* pszFileName):m_nNextEnvDirId(0)
	{
		if (pszFileName)
			m_strFileName = pszFileName;
		/*
		* 从配置文件中读取到 scom.dll 所保存到的注册表路径
		*/
		/*
		TCHAR pszPath[MAX_PATH] = {0};
		DWORD dwChRead = 0;
		int nRet = kxecommon_get_dll_reg_path_from_config_file(
			pszFileName,
			pszPath,
			MAX_PATH,
			&dwChRead
			);
		if (0 == nRet)
		{
			m_strDllRegPath.assign(T2W(pszPath), dwChRead);
		}
		*/
		std::wstring strDllRegPath;
		kxe_get_kxebase_reg_path(strDllRegPath);

#if !defined(_UNICODE) && !defined(UNICODE)
		m_strDllRegPath = (W2T(strDllRegPath.c_str()));	
#else
		m_strDllRegPath = strDllRegPath;
#endif
	}

// 	void SetFileName(const TCHAR* pszFileName)
// 	{
// 		m_strFileName = pszFileName;
// 		m_nNextEnvDirId = 0;
// 		m_strEnvDirs.clear();
// 	}

	int SearchInCurModuleDir(TCHAR* szFileFullName, size_t size)
	{
		int nRetCode = -1;
		TCHAR szDir[MAX_PATH] = { 0 };
		if (0 == GetCurModuleDir(szDir, MAX_PATH))
		{
			nRetCode = CheckFile(szDir, m_strFileName.c_str(), szFileFullName, size);
		}
		return nRetCode;
	}

	int SearchInCurProcessDir(TCHAR* szFileFullName, size_t size)
	{
		int nRetCode = -1;
		TCHAR szDir[MAX_PATH] = { 0 };
		if (0 == GetCurProcessDir(szDir, MAX_PATH))
		{
			nRetCode = CheckFile(szDir, m_strFileName.c_str(), szFileFullName, size);
		}
		return nRetCode;
	}

	int SearchInCurrentDir(TCHAR* szFileFullName, size_t size)
	{
		return CheckFile(TEXT(""), m_strFileName.c_str(), szFileFullName, size);
	}

	int SearchInProgramFilesDir(const TCHAR* pszSubDirName, TCHAR* szFileFullName, size_t size)
	{
		std_tstring strDir;
		int nRetCode = GetProgramFileDir(strDir);
		if (0 == nRetCode)
		{
			if (pszSubDirName)
			{
				strDir += TEXT("\\");
				strDir += pszSubDirName;
			}
			nRetCode = CheckFile(strDir.c_str(), m_strFileName.c_str(), szFileFullName, size);
		}
		return nRetCode;
	}

	int SearchInSystemDir(const TCHAR* pszSubDirName, TCHAR* szFileFullName, size_t size)
	{
		TCHAR szSystemDir[MAX_PATH] = { 0 };
		GetSystemDirectory(szSystemDir, MAX_PATH);
		if (pszSubDirName)
		{
#if _MSC_VER >= 1400
			_tcscat_s(szSystemDir, MAX_PATH, TEXT("\\"));
			_tcscat_s(szSystemDir, MAX_PATH, pszSubDirName);
#else
			_tcscat(szSystemDir, _T("\\"));
			_tcscat(szSystemDir, pszSubDirName);
#endif
		}
		int nRetCode = CheckFile(szSystemDir, m_strFileName.c_str(), szFileFullName, size);
		return nRetCode;
	}

	int SearchInWindowsDir(const TCHAR* pszSubDirName, TCHAR* szFileFullName, size_t size)
	{
		TCHAR szWindowsDir[MAX_PATH] = { 0 };
		GetWindowsDirectory(szWindowsDir, MAX_PATH);
		if (pszSubDirName)
		{
#if _MSC_VER >= 1400
			_tcscat_s(szWindowsDir, MAX_PATH, TEXT("\\"));
			_tcscat_s(szWindowsDir, MAX_PATH, pszSubDirName);
#else
			_tcscat(szWindowsDir, TEXT("\\"));
			_tcscat(szWindowsDir, pszSubDirName);
#endif
		}
		int nRetCode = CheckFile(szWindowsDir, m_strFileName.c_str(), szFileFullName, size);
		return nRetCode;
	}

	int SearchInEnvDir(TCHAR* szFileFullName, size_t size)
	{
		m_strEnvDirs.clear();
		m_nNextEnvDirId = 0;
		std_tstring strEnvPath;
		GetEnvPath(strEnvPath);
		SplitPath(strEnvPath);
		size_t sizeDirs = m_strEnvDirs.size();
		if (0 == sizeDirs)
			return -1;

		int nRetCode = -1;
		for(int i = static_cast<int>(sizeDirs - 1); i >= 0; --i)//逆向查找
		{
			m_nNextEnvDirId = i -1;
			if (TEXT('%') != (m_strEnvDirs[i])[0])//跳过"带%号的环境变量,主要是%SystemRoot%"
			{
				if (0 == CheckFile(m_strEnvDirs[i].c_str(), m_strFileName.c_str(), szFileFullName, size))
				{
					nRetCode = 0;
					break;
				}
			}
		}		
		return nRetCode;
	}

	int SearchInNextEnvDir(TCHAR* szFileFullName, size_t size)
	{
		std_tstring strFirstFileFullName;
		int nRetCode = -1;
		if (m_strEnvDirs.size())
		{
			for(int i = m_nNextEnvDirId; i >= 0; --i)//继续逆向查找
			{
				m_nNextEnvDirId = i -1;
				if ('%' != (m_strEnvDirs[i])[0])//跳过"带%号的环境变量,主要是%SystemRoot%"
				{
					if (0 == CheckFile(m_strEnvDirs[i].c_str(), m_strFileName.c_str(), szFileFullName, size))
					{
						nRetCode = 0;
						break;
					}
				}
			}
		}
		return nRetCode;
	}

	int SearchInReg(TCHAR* szFileFullName, size_t size)
	{
		assert(!m_strDllRegPath.empty());
		if (m_strDllRegPath.empty())
		{
			return E_KXEBASE_DLL_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE;
		}

		int nRetCode = -1;

		DWORD dwVType = REG_SZ;
		DWORD dwVLen = (DWORD)(size * sizeof(TCHAR));
		
		HKEY hKey = NULL;
		nRetCode = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			m_strDllRegPath.c_str(),
			0L,
			KEY_READ,
			&hKey
			);
		if( nRetCode != ERROR_SUCCESS )
			return nRetCode;
		
		nRetCode = RegQueryValueEx(
			hKey,
			0,
			0, 
			&dwVType, 
			(BYTE*)szFileFullName,
			&dwVLen
			);
		if( nRetCode != ERROR_SUCCESS )
		{
			RegCloseKey( hKey );
			hKey = NULL;
			return nRetCode;
		}
		
		RegCloseKey( hKey );

		nRetCode = _taccess( szFileFullName, 4 );
		return nRetCode;
	}

	//WritePathToReg先比较szFileFullName跟记录的是否相同，相同就直接返回成功了
	int WritePathToReg(const TCHAR* szFileFullName )
	{
		assert(!m_strDllRegPath.empty());
		if (m_strDllRegPath.empty())
			return E_KXEBASE_DLL_PATH_NOT_ACCQUIRED_FROM_CONFIGURATION_FILE;

		if (NULL == szFileFullName)
			return -1;

		HKEY hKey = NULL;
		int nRetCode = -1;
		TCHAR szRegFullPath[MAX_PATH] = {0};
		DWORD dwVType = REG_SZ;
		DWORD dwVLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwDisposition = 0;

		nRetCode = RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			m_strDllRegPath.c_str(),
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&hKey,
			&dwDisposition
			);
		if( nRetCode != ERROR_SUCCESS )
			return nRetCode;
	
		if (REG_OPENED_EXISTING_KEY == dwDisposition)
		{
			nRetCode = RegQueryValueEx(
				hKey,
				0,
				0,
				&dwVType,
				(BYTE*)szRegFullPath,
				&dwVLen
				);
			if( nRetCode == ERROR_SUCCESS )
			{
				if (0 == _tcsicmp(szFileFullName, szRegFullPath))
				{//相等就不用再写了
					nRetCode = ERROR_SUCCESS;
					goto Exit0;
				}
			}
		}
		nRetCode = RegSetValueEx(hKey, 0, 0, REG_SZ, (BYTE*)szFileFullName, (DWORD)(_tcslen(szFileFullName) * sizeof(TCHAR)) );

Exit0:
		if (hKey)
		{
			RegCloseKey( hKey );
			hKey = NULL;

		}

		return nRetCode;
	}

	int GetModuleDir(HMODULE hHandle, TCHAR* szDirPath, DWORD dwSize)
	{
		if (NULL == hHandle || NULL == szDirPath || 0 == dwSize)
			return -1;

		int nRetCode = -1;
		szDirPath[0] = 0;
		szDirPath[dwSize - 1] = 0;
		if (::GetModuleFileName(hHandle, szDirPath, (dwSize - 1)) != 0)
		{
			TCHAR* pszPos = _tcsrchr(szDirPath, TEXT('\\'));
			if (pszPos != NULL)
			{
				++pszPos;
				*pszPos = 0;
				nRetCode = 0;
			}		
		}
		return nRetCode;
	}

	int GetCurModuleDir(TCHAR* szDirPath, DWORD dwSize)
	{
		return GetModuleDir((HINSTANCE)&__ImageBase, szDirPath, dwSize);
	}

	int GetCurProcessDir(TCHAR* szDirPath, DWORD dwSize)
	{
		return GetModuleDir(NULL, szDirPath, dwSize);
	}

private:
	int GetEnvPath(std_tstring& strEnvPath)
	{
		HKEY hKey;
		int nRetCode = OpenReadReg(HKEY_LOCAL_MACHINE, REGISTER_ENV_SUB_KEY, hKey);
		if (nRetCode != ERROR_SUCCESS)
			return nRetCode;
		DWORD dwDataType;
		nRetCode = GetRegValue(hKey, ENV_VARIABLE_KEY, strEnvPath, dwDataType);
		CloseReadReg(hKey);
		return 0;
	}

	int GetProgramFileDir(std_tstring& strProgFileDir)
	{
		HKEY hKey;
		int nRetCode = OpenReadReg(HKEY_LOCAL_MACHINE, REGISTER_CURRENT_VESION_SUB_KEY, hKey);
		if (nRetCode != ERROR_SUCCESS)
			return nRetCode;
		DWORD dwDataType;
		nRetCode = GetRegValue(hKey, PROGRAM_FILES_DIR_KEY, strProgFileDir, dwDataType);
		CloseReadReg(hKey);
		return nRetCode;
	}

	int SplitPath(const std_tstring& strEnvPath)
	{
		m_strEnvDirs.clear();
		std_tstring::size_type idx = 0;
		std_tstring::size_type prev_idx = 0;
		std_tstring::size_type lastIdx = strEnvPath.length();

		while (idx != std_tstring::npos && prev_idx != lastIdx)
		{
			idx = strEnvPath.find(TEXT(';'), prev_idx);//
			if (idx != prev_idx)
			{
				const std_tstring& rstrDir = (idx != (std_tstring::npos)) ? 
					strEnvPath.substr(prev_idx, idx - prev_idx) :  strEnvPath.substr(prev_idx);
				m_strEnvDirs.push_back(rstrDir);
			}
			prev_idx = idx;
			++prev_idx;
		}
		return 0;
	}

	int CheckFile(const TCHAR* pszDir, const TCHAR* pszFileName, TCHAR* szFileFullName, size_t size)
	{
		std_tstring strFile = pszDir;
		//strFile += TEXT("\\");
		strFile += pszFileName;
		//TCHAR szTmpFile[MAX_PATH] = { 0 };
		//_tfullpath(szTmpFile, strFile.c_str(), MAX_PATH);
		int nRetCode = _taccess(strFile.c_str(), 4);//检查是否可读
		if (0 == nRetCode)
		{
#if _MSC_VER >= 1400
			_tcsncpy_s(szFileFullName, size, strFile.c_str(), strFile.length() + 1);
#else
			_tcsncpy(szFileFullName, strFile.c_str(), size);
#endif
		}
		//nRetCode ?= EACCES ENOENT 
		return nRetCode;
	}

	int OpenReadReg(HKEY hKey, const TCHAR* pszSubKey, HKEY& hkResult)
	{
		DWORD dwDisposition = 0;
		DWORD dwRetCode = RegCreateKeyEx(hKey, pszSubKey, 
			0L, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hkResult, &dwDisposition);
		return dwRetCode;
	}

	int CloseReadReg(HKEY hKey)
	{
		return RegCloseKey(hKey);
	}

	int GetRegValue(HKEY hKey, const TCHAR* pszValueName, std_tstring& strValue,  DWORD& dwDataType)
	{
		DWORD dwBufSize = 0;
		DWORD dwRetCode = RegQueryValueEx(hKey, pszValueName, 0, &dwDataType, NULL, &dwBufSize);
		if (dwRetCode != ERROR_SUCCESS)
			return dwRetCode;
		std::vector<unsigned char> vecVelue;
		vecVelue.resize(dwBufSize);
		dwRetCode = RegQueryValueEx(hKey, pszValueName, 0, &dwDataType, &(vecVelue[0]), &dwBufSize);
		assert(dwRetCode == ERROR_SUCCESS);
		strValue =(TCHAR*)&(vecVelue[0]);
		return dwRetCode;
	}

private:
	int m_nNextEnvDirId;
	std_tstring m_strFileName;	/*	DLL 名称	*/
	std_tstring m_strDllRegPath;	/*	DLL 所在路径	*/
	std::vector<std_tstring> m_strEnvDirs;
};
#endif //KXESEARCHFILE_H


#include "stdafx.h"
#include <cassert>
#include <shlwapi.h>
#include "enumregistry.h"
#include "getregistry.h"
#include "analyzecmdline.h"
#include "registrydata.h"
#include "regbackupdlg.h"
#include "kppuserlog.h"
#include "kbackupreg.h"
#include "stubbornregs.h"
#include "kscbase/kscsys.h"
#include "kscbase/kcslogging.h"

#pragma comment(lib, "shlwapi.lib")
#pragma warning(disable: 4996)

#define	ELEMOF(x)	(sizeof(x)/sizeof((x)[0]))


CRegistryData::CRegistryData(void) : 
	m_fnCallback(NULL), m_lpCallbackParam(NULL), m_bStopScanning(false), m_bCleaned(false)
{
	m_bHaveUndelReg = FALSE;

	CRegistryData::SCAN_DATA	data[] = 
	{
		{RST_UNINSTALL, CRegistryData::GetTitle(RST_UNINSTALL), 
		{HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\uninstall\\"), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanUninstall},
		
		{RST_SHAREDDLL, CRegistryData::GetTitle(RST_SHAREDDLL), 
		{HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\SharedDlls\\"), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanSharedDlls},
		
		{RST_OPENSAVEMRU, CRegistryData::GetTitle(RST_OPENSAVEMRU), 
		{HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSaveMRU\\"), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanOpenSaveMRU},

		{RST_FILEASSOC, CRegistryData::GetTitle(RST_FILEASSOC), 
		{HKEY_CLASSES_ROOT, TEXT(""), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanFileAssociate},

		{RST_COM, CRegistryData::GetTitle(RST_COM), 
		{HKEY_CLASSES_ROOT, TEXT("TypeLib\\"), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanTypeLib},

		{RST_COM, CRegistryData::GetTitle(RST_COM), 
		{HKEY_CLASSES_ROOT, TEXT("CLSID\\"), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanClsid},

		{RST_EXTHISTORY, CRegistryData::GetTitle(RST_EXTHISTORY), 
		{HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"), TEXT(""), WOW64_DEFAULT}, 
		&CRegistryData::ScanFileExts},

		/*{RST_WINRAR, CRegistryData::GetTitle(RST_WINRAR),
		{HKEY_CURRENT_USER, TEXT("Software\\WinRAR\\ArcHistory\\"), TEXT("")},
		&CRegistryData::ScanWinRarOpenHistory},

		{RST_WINZIP, CRegistryData::GetTitle(RST_WINZIP),
		{HKEY_CURRENT_USER, TEXT(""), TEXT("")},
		&CRegistryData::ScanWinZipOpenHistory},

		{RST_OFFICE, CRegistryData::GetTitle(RST_OFFICE),
		{HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Office\\"), TEXT("")},
		&CRegistryData::ScanOfficeOpenHistory},*/

		{RST_STARTMENU, CRegistryData::GetTitle(RST_STARTMENU),
		{HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Start Menu2\\Programs\\"), 
		TEXT(""), WOW64_DEFAULT},
		&CRegistryData::ScanMenuOrder},

		{RST_MUICACHE, CRegistryData::GetTitle(RST_MUICACHE),
		{HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache\\"), TEXT(""), WOW64_DEFAULT},
		&CRegistryData::ScanMUICache},

		{RST_HELP, CRegistryData::GetTitle(RST_HELP),
		{HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\Help\\"), TEXT(""), WOW64_DEFAULT},
		&CRegistryData::ScanHelp},

		{RST_HELP, CRegistryData::GetTitle(RST_HELP),
		{HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\Html Help\\"), TEXT(""), WOW64_DEFAULT},
		&CRegistryData::ScanHelp},

		{RST_APPPATH, CRegistryData::GetTitle(RST_APPPATH),
		{HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\"), TEXT(""), WOW64_DEFAULT},
		&CRegistryData::ScanAppPath},
	};
	
	for (int i = 0; i < ELEMOF(data); ++i)
	{
		m_scanPoints.insert(data[i]);
	}

}

CRegistryData::~CRegistryData(void)
{
}


HRESULT	CRegistryData::Scan(TYPE_SCAN_CALLBACK fnTypeCallback, LPVOID lpTypeParam, SCAN_CALLBACK fnCallback, LPVOID lpParam)
{
	HRESULT			hr = E_FAIL;

	// 清除扫描记录
	m_bStopScanning = false;
	m_scanResult.clear();
	m_bCleaned = false;
	m_bHaveUndelReg = FALSE;

	// 保存回调函数及参数
	m_fnCallback = fnCallback;
	m_lpCallbackParam = lpParam;

	// 开始扫描
	std::set<CRegistryData::SCAN_DATA>::iterator	iter;

	hr = S_OK;
	for ( iter = m_scanPoints.begin(); iter != m_scanPoints.end(); ++iter )
	{
		assert(iter->scanner != NULL);

		// 若通知退出
		if ( m_bStopScanning )
			break;

		// 若要跳过此类型的扫描
		if ( fnTypeCallback != NULL )
		{
			if ( !(*fnTypeCallback)(iter->rstType, lpTypeParam) )
				continue;
		}

		// 扫描
		hr = (this->*iter->scanner)(*iter);
		if ( hr == S_FALSE )
			break;
	}
	
	m_bStopScanning = false;
	return hr;
}


HRESULT	CRegistryData::Stop()
{
	m_bStopScanning = true;
	return S_OK;
}


HRESULT	CRegistryData::Clean(TYPE_CLEAN_CALLBACK fnTypeCallback, LPVOID lpTypeParam)
{
	HRESULT				hr = S_OK;
//	CRegBackupDlg		backup;
	CKBackupReg			backup;
	bool				bBackup = false;
	tstring				strKeyPath;
	CRegistryData::REG_SCAN_TYPE	rstLastType = RST_INVALID;
	std::set<CRegistryData::SCAN_RESULT>::iterator	iter;

	m_bStopScanning = false;

	// 清除上次的记录
	/*if ( m_bCleaned )
	{
		m_scanResult.clear();
		m_bCleaned = false;
	}*/

	// 若无记录，直接返回
	if ( m_scanResult.empty() )
	{
		hr = S_OK;
		goto Label_Ret;
	}

	m_bCleaned = true;
	rstLastType = CRegistryData::RST_INVALID;
	for ( iter = m_scanResult.begin(); iter != m_scanResult.end(); ++iter)
	{
		// 检查是否需要停止
		if ( m_bStopScanning )
			break;

		// 检查是否标记为清除
		if ( !iter->bToClean )
			continue;

		// 调用回调函数
		if ( fnTypeCallback != NULL && iter->rstType != rstLastType )
		{
			rstLastType = iter->rstType;
			(*fnTypeCallback)(iter->rstType, lpTypeParam);
		}

		// 初始化备份
		if ( !bBackup && backup.Init() )
			bBackup = true;
	
		iter->scanData.GetKeyFullPath(strKeyPath, false);
		strKeyPath += iter->scanData.strValueName;

		backup.KscBackupReg(iter->scanData.hRootKey, iter->scanData.strSubKey, iter->scanData.strValueName);
		// 调用清理函数进行清除
		assert( iter->cleaner != NULL );
		if ( iter->cleaner != NULL )
		{
			hr = (this->*iter->cleaner)(iter->scanData);
		}

	
		if (hr!=ERROR_SUCCESS)
		{
			CStubbornRegs::Instance().AddReg(strKeyPath.c_str());
			CKBackupReg DelData;
			DelData.DeleteData(iter->scanData.hRootKey, iter->scanData.strSubKey, iter->scanData.strValueName);
			m_bHaveUndelReg = TRUE;
		}
	
		// 删除节点
		iter->bToClean = false;
		//m_scanResult.erase(iter++);
	}

	if ( bBackup )
		backup.Unit();

	CStubbornRegs::Instance().Sync();

Label_Ret:
	m_bStopScanning = false;
	return hr;
}


//////////////////////////////////////////////////////////////////////////
//
// 子键下为空，指定的卸载文件不存在(64bit 扫描2个位置)
//
HRESULT	CRegistryData::ScanUninstall(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;
	
	data = scanData;
	data.lpThis = this;

    if (IsX64System())
    {      
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckUninstall, 
                                &data,
                                FORCE_WOW64_64
                                );
      
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckUninstall, 
                                &data,
                                FORCE_WOW64_32
                                );
    }
    else
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckUninstall, 
                                &data,
                                WOW64_DEFAULT
                                );
    }
	
    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckUninstall(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;
	
	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	
    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("");
    data.scanPoint.wow64type = wow64type;

	if ( !data.lpThis->CheckUninstallString(data) ) 
		return false;

	return true;
    
}


bool CRegistryData::CheckUninstallValue(const SCAN_DATA& scanData)
{
	HRESULT							hr = E_FAIL;
	CRegistryData::SCAN_DATA		data = scanData;
	SCAN_RESULT						scanResult;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;
	struct 
	{
		const TCHAR*			lpValue;
		CRegistryData::CHECKER	checker;	
		CRegistryData::CLEANER	cleaner;
	}lpszValue[] = 
	{
		{TEXT("InstallLocation"), &CRegistryData::CheckFilePath, &CRegistryData::CleanValue},
		{TEXT("Inno Setup: App Path"), &CRegistryData::CheckFilePath, &CRegistryData::CleanValue},
		//{TEXT("InstallSource"), &CRegistryData::CheckFilePath, &CRegistryData::CleanValue},
	};

	scanResult.scanData = data.scanPoint;
	for ( int i = 0; i < ELEMOF(lpszValue); ++i )
	{
		if ( data.lpThis->m_bStopScanning )
			return false;

		// 追加键值
		data.scanPoint.strValueName = lpszValue[i].lpValue;
		scanResult.scanData.strValueName = lpszValue[i].lpValue;

		// 检查键值数据中指定的路径是否效
		scanResult.ppPos = PP_VALUE_DATA;
		hr = (data.lpThis->*lpszValue[i].checker)(scanResult);
		if ( hr != S_FALSE )
			continue;

		scanResult.rstType = data.rstType;
		scanResult.strTitle = data.strTitle;
		scanResult.bToClean = true;
		scanResult.cleaner = lpszValue[i].cleaner;

		// 保存到清理列表中
		tstring strKeyPath;
		scanResult.scanData.GetKeyFullPath(strKeyPath, true);
		if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
		{          
			pairResult = data.lpThis->m_scanResult.insert(scanResult);
			if ( !pairResult.second )
			continue;

			// 调用回调函数
			std::set<SCAN_RESULT>::iterator	iterItem = pairResult.first;
			if ( data.lpThis->m_fnCallback != NULL && !(data.lpThis->m_fnCallback)(&*iterItem, data.lpThis->m_lpCallbackParam) )
			{
				data.lpThis->m_bStopScanning = true;
				return false;
			}
		}
	}

	return true;
}


bool CRegistryData::CheckUninstallString(const SCAN_DATA& scanData)
{
	HRESULT							hr = E_FAIL;
	CRegistryData::SCAN_DATA		data = scanData;
	SCAN_RESULT						scanResult;
	tstring							strSoftwareName;
	tstring::size_type				idx = tstring::npos;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

	// 追加键值
	data.scanPoint.strValueName = TEXT("UninstallString");
	scanResult.scanData = data.scanPoint;

	// 检查键值数据中指定的路径是否效
	scanResult.ppPos = PP_VALUE_DATA;
	hr = CheckCmdline(scanResult);
	if ( hr == S_OK )
		return true;

	// 检查DisplayName对应的子键是否存在: 临时关闭
	if ( hr == E_FAIL )
	{
        return true;
		//hr = CheckDisplayName(data);
		//if ( hr == S_OK )
		//	return true;
	}

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.scanData.strValueName = TEXT("");
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;
	
	strSoftwareName = scanResult.scanData.strSubKey;
	strSoftwareName.erase(strSoftwareName.size() - 1);
	idx = strSoftwareName.rfind(TEXT("\\"));
	if ( idx != tstring::npos ){
		strSoftwareName.erase(0, idx + 1);
	}

	scanResult.strProblem = TEXT("“");
	scanResult.strProblem += strSoftwareName;
	scanResult.strProblem += TEXT("”对应的安装信息不存在");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);
	
    if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);
		if ( !pairResult.second )
			return true;

		// 调用回调函数
		std::set<SCAN_RESULT>::iterator	iterItem = pairResult.first;
		if ( data.lpThis->m_fnCallback != NULL && !(data.lpThis->m_fnCallback)(&*iterItem, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


HRESULT CRegistryData::CheckDisplayName(const SCAN_DATA& scanData)
{
	HRESULT							hr = E_FAIL;
	CRegistryData::SCAN_DATA		data = scanData;
	CRegistryData::SCAN_RESULT		scanResult;
	bool							bResult = false;
	tstring							strDisplayName;
	DWORD							dwType = REG_SZ;
	DWORD							cbData = 0;
	std::vector<TCHAR>				bufData(MAX_PATH, 0);
	tstring::size_type				idx = tstring::npos;

	// 取默认值的值
	cbData = (DWORD)bufData.size() * sizeof(TCHAR);
	bResult = GetRegValueAll(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), TEXT("DisplayName"), 
		&dwType, (LPBYTE)&bufData[0], &cbData, data.scanPoint.wow64type);
	if ( !bResult )
		return E_FAIL;

	if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
		return E_FAIL;

	if ( ::_tcslen(&bufData[0]) == 0 )
		return E_FAIL;

	strDisplayName = &bufData[0];
	scanResult.scanData = data.scanPoint;
	
	idx = scanResult.scanData.strSubKey.rfind(TEXT("\\"));
	scanResult.scanData.strSubKey.erase(idx);
	idx = scanResult.scanData.strSubKey.rfind(TEXT("\\"));
	scanResult.scanData.strSubKey.erase(idx);
	scanResult.scanData.strSubKey += TEXT("\\");
	scanResult.scanData.strSubKey += strDisplayName;
	scanResult.scanData.strSubKey += TEXT("\\");
	
	scanResult.scanData.strValueName = TEXT("UninstallString");
	scanResult.ppPos = PP_VALUE_DATA;
	hr = CheckCmdline(scanResult);
	return hr;
}


HRESULT	CRegistryData::ScanSharedDlls(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
    if (IsX64System())
    {
        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckValueNamePath, 
                                &data,
                                FORCE_WOW64_64
                                );

        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckValueNamePath, 
                                &data,
                                FORCE_WOW64_32
                                );
                    	
    }
    else
    {
        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckValueNamePath, 
                                &data,
                                WOW64_DEFAULT
                                );
    }

	
    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckValueNamePath(HKEY hKey, 
                                   const TCHAR* lpszValueName, 
                                   const void* lpValueData, 
									   DWORD dwType, 
                                   DWORD dwDataLen, 
                                   LPVOID param,
                                   Wow64Type wow64type
                                   )
{
    HRESULT				hr = E_FAIL;
    bool				bResult = false;

    if ( param == NULL )
        return true;

    CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
    SCAN_RESULT						scanResult;
    std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

    if ( data.lpThis->m_bStopScanning )
        return false;

    if ( dwType != REG_DWORD )
        return true;

    if ( lpszValueName != NULL )
        data.scanPoint.strValueName = lpszValueName;

    data.scanPoint.wow64type = wow64type;
    scanResult.scanData = data.scanPoint;
    scanResult.ppPos = PP_VALUE_NAME;
    hr = (data.lpThis->CheckFilePath)(scanResult);
    if ( hr != S_FALSE )
        return true;

    scanResult.rstType = data.rstType;
    scanResult.strTitle = data.strTitle;
    scanResult.bToClean = true;
    scanResult.cleaner = &CRegistryData::CleanValue;

    scanResult.strProblem = TEXT("对应的DLL文件\"");
    scanResult.strProblem += scanResult.strInvalidPath.c_str();
    scanResult.strProblem += TEXT("\"已不存在");

    // 保存到清理列表中
    tstring strKeyPath;
    scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

    if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
    {
        pairResult = data.lpThis->m_scanResult.insert(scanResult);

        // 调用回调函数
        if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
        {
            data.lpThis->m_bStopScanning = true;
            return false;
        }
    }

    return true;
}


HRESULT	CRegistryData::ScanOpenSaveMRU(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
    if (IsX64System())
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckOpenSaveMRUSubKey, 
                                &data, 
                                FORCE_WOW64_64
                                );
    }
    else
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckOpenSaveMRUSubKey, 
                                &data, 
                                WOW64_DEFAULT
                                );
    }
    
   return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckOpenSaveMRUSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;

	data.scanPoint.strSubKey += lpszSubKeyName;
	data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.wow64type = wow64type;
    

	EnumRegValue_WithCallBack(hKey, lpszSubKeyName, &CRegistryData::CheckOpenSaveMRUDataPath, &data, WOW64_DEFAULT);

	if ( data.lpThis->m_bStopScanning )
		return false;
	else
		return true;
}


bool CRegistryData::CheckOpenSaveMRUDataPath(HKEY hKey, 
                                        const TCHAR* lpszValueName, 
                                        const void* lpValueData, 
                                        DWORD dwType, 
                                        DWORD dwDataLen, 
                                        LPVOID param,
                                        Wow64Type wow64type
                                        )
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	tstring							strLastPart;
	tstring							strTemp;
	tstring::size_type				idx = tstring::npos;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

	if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
		return true;

	if ( lpszValueName == NULL )
		return true;

	// 只处理形如a,b,c之类的键值名
	data.scanPoint.strValueName = lpszValueName;
	if ( data.scanPoint.strValueName.size() > 1 || data.scanPoint.strValueName.empty() )
		return true;

    data.scanPoint.wow64type = wow64type;
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	hr = (data.lpThis->CheckFilePath)(scanResult);
	if ( hr != S_FALSE )
		return true;

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanValue;

	strLastPart = scanResult.scanData.strSubKey;
	strLastPart += scanResult.scanData.strValueName;
	idx = strLastPart.rfind(TEXT("\\"));
	if ( idx != tstring::npos )
	{
		strTemp = strLastPart.substr(idx);
		strLastPart.erase(idx);
		idx = strLastPart.rfind(TEXT("\\"), idx);
	}

	if ( idx != tstring::npos )
	{
		strLastPart.erase(0, idx);
		strLastPart += strTemp;
	}

	if ( strLastPart.size() > 1 )
		strLastPart.erase(0, 1);

	scanResult.strProblem = TEXT("\"");
	scanResult.strProblem += strLastPart;
	scanResult.strProblem += TEXT("\"记录的历史信息已无效");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);
		if ( !pairResult.second )
			return true;

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&(*(pairResult.first)), data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


HRESULT	CRegistryData::ScanFileAssociate(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
	EnumRegSubKey_WithCallBack(HKEY_CLASSES_ROOT, 
                            NULL, 
                            &CRegistryData::CheckFileAssociateSubKey, 
                            &data,
                            WOW64_DEFAULT);

    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckFileAssociateSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	tstring							strExtKey;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("");
    data.scanPoint.wow64type = wow64type;

	// 以.开始的子键，只检查关联子键
	if ( ::_tcslen(lpszSubKeyName) >= 1 && lpszSubKeyName[0] == TEXT('.') )
	{
		return data.lpThis->CheckExtAssociateKey(data);
	}
	
	// 检查打开方式关联
	if ( !data.lpThis->CheckShellOpenCommand(data) )
		return false;

	// 检查图标关联
	if ( !data.lpThis->CheckDefaultIcon(data) )
		return false;
	
	return true;
}

// 子键下无其他子键或键值
// 1.若@未设置或为空，，删除子键
// 2.若@指向的子键不存在，删除子键 
bool CRegistryData::CheckExtAssociateKey(const SCAN_DATA& scanData)
{
	HRESULT							hr = E_FAIL;
	CRegistryData::SCAN_DATA		data = scanData;
	SCAN_RESULT						scanResult;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;
	bool							bDelete = false;
	bool							bResult = false;
	tstring							strPath;
	std::vector<TCHAR>				bufFile(MAX_PATH, 0);
	DWORD							dwType = REG_SZ;
	DWORD							cbData = 0;
	std::vector<TCHAR>				bufData(MAX_PATH, 0);

	if ( data.lpThis->m_bStopScanning )
		return false;

	// 检查子键下是否有下层子键和键值。若有，返回
	if ( !CheckIfKeyEmpty(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), data.scanPoint.wow64type))
		return true;

	// 取默认值的值
	cbData = (DWORD)bufData.size() * sizeof(TCHAR);
	bResult = GetRegValueAll(data.scanPoint.hRootKey, 
                          data.scanPoint.strSubKey.c_str(), 
                          data.scanPoint.strValueName.c_str(), 
		                   &dwType, 
                          (LPBYTE)&bufData[0], 
                          &cbData,
                          data.scanPoint.wow64type);
                          
	if ( !bResult )
	{
		// 若键值不存在, 且无下层子键
		if ( !CheckIfHasSubKey(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), data.scanPoint.wow64type) )
		{
			bDelete = true;
			goto Label_Ret;
		}
		else
		{
			bDelete = false;
			goto Label_Ret;
		}
	}
	else if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
	{
		// 类型不正确
		bDelete = true;
		goto Label_Ret;
	}
	
	if ( ::_tcslen(&bufData[0]) == 0  )
	{
		// 键值为空, 且无下层子键
		if ( !CheckIfHasSubKey(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), data.scanPoint.wow64type ))
		{
			bDelete = true;
			goto Label_Ret;
		}
		else
		{
			bDelete = false;
			goto Label_Ret;
		}
	}
	else if ( !CheckIfKeyExist(HKEY_CLASSES_ROOT, &bufData[0], data.scanPoint.wow64type))
	{
		// 指向的子键hkcr\xx不存在
		bDelete = true;
		goto Label_Ret;
	} 
	else
	{
		bDelete = false;
		goto Label_Ret;
	}

Label_Ret:
	// 若无需删除，返回
	if ( !bDelete )
		return true;

	// 删除子键
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;

	scanResult.strProblem = TEXT("\"");
	scanResult.strProblem += data.scanPoint.strSubKey;
	scanResult.strProblem.erase(scanResult.strProblem.size() - 1);
	scanResult.strProblem += TEXT("\"对应的文件关联已无效");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);
	
    if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


bool CRegistryData::CheckShellOpenCommand(const SCAN_DATA& scanData)
{
	HRESULT							hr = E_FAIL;
	CRegistryData::SCAN_DATA		data = scanData;
	SCAN_RESULT						scanResult;
	tstring							strExt;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

	strExt = data.scanPoint.strSubKey;
	strExt.erase(strExt.size() - 1);

	data.scanPoint.strSubKey += TEXT("shell\\open\\command\\");
	data.scanPoint.strValueName = TEXT("");

	// 检查子键shell\open\command\@对应的文件是否存在
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	hr = (data.lpThis->CheckCmdline)(scanResult);
	if ( hr != S_FALSE )
		return true;

	// 过滤含有"%1" %*的串
	if ( scanResult.strInvalidPath.find(TEXT("\"%1\" %*")) != tstring::npos )
		return true;
	if ( scanResult.strInvalidPath.size() > 6)
	{
		tstring strTemp(scanResult.strInvalidPath, 0, 6);
		if (strTemp.find(TEXT("\"%1\" ")) != tstring::npos )
		{
			return true;
		}
	}

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;
	
	scanResult.strProblem = TEXT("\"");
	scanResult.strProblem += strExt;
	scanResult.strProblem += TEXT("\"对应的文件关联已无效");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


bool CRegistryData::CheckDefaultIcon(const SCAN_DATA& scanData)
{
	HRESULT							hr = E_FAIL;
	CRegistryData::SCAN_DATA		data = scanData;
	SCAN_RESULT						scanResult;
	tstring							strExt;
	tstring	::size_type				idx = tstring::npos;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

	strExt = data.scanPoint.strSubKey;
	strExt.erase(strExt.size() - 1);

	data.scanPoint.strSubKey += TEXT("DefaultIcon\\");
	data.scanPoint.strValueName = TEXT("");

	// 检查子键DefaultIcon\@对应的文件是否存在
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	hr = (data.lpThis->CheckFilePath)(scanResult);
	if ( hr != S_FALSE )
		return true;

	// 去除"%1"
	idx = scanResult.strInvalidPath.find(TEXT("%1"));
	if ( idx != tstring::npos )
		return true;

	// 检查路径是否为a.dll,-nn的形式
	idx = scanResult.strInvalidPath.rfind(TEXT(","));
	if ( idx != tstring::npos )
	{
		scanResult.strInvalidPath.erase(idx);
		hr = ::CheckFilePath(scanResult.strInvalidPath, scanResult.scanData.wow64type);
		if ( hr != S_FALSE )
			return true;
	}

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;
	
	scanResult.strProblem = TEXT("\"");
	scanResult.strProblem += strExt;
	scanResult.strProblem += TEXT("\"对应的文件关联已无效");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


HRESULT	CRegistryData::ScanTypeLib(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
	EnumRegSubKey_WithCallBack(HKEY_CLASSES_ROOT, 
                            scanData.scanPoint.strSubKey.c_str(), 
		                     &CRegistryData::CheckTypeLibSubKey, 
                            &data,
                            WOW64_MISC);

    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckTypeLibSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;

    data.scanPoint.strSubKey += lpszSubKeyName;	// {clsid}
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.wow64type = wow64type;

	// 枚举版本子键
	EnumRegSubKey_WithCallBack(hKey, lpszSubKeyName, &CRegistryData::CheckTypeLibWin32Key, &data, wow64type);

	if ( data.lpThis->m_bStopScanning )
		return false;
	else
		return true;
}


bool CRegistryData::CheckTypeLibWin32Key(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

    data.scanPoint.wow64type = wow64type;
    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\0\\win32\\");
    data.scanPoint.strValueName = TEXT("");

	// 检查0\win32\@对应的文件是否存在
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	hr = (data.lpThis->CheckFilePath)(scanResult);
	if ( hr != S_FALSE )
		return true;

	// 去除"file.dll\1"形式的路径
	UINT	nLen = (UINT)scanResult.strInvalidPath.size();

	// 跳过结尾的"\数字"
	for ( int i = nLen - 1; i >= 0; --i )
	{
		if ( _istdigit(scanResult.strInvalidPath[i]) )
			continue;

		if ( scanResult.strInvalidPath[i] == TEXT('\\') )
				return true;
		
		break;
	}
	
	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanValue;

	scanResult.strProblem = TEXT("对应的文件“");
	scanResult.strProblem += scanResult.strInvalidPath.c_str();
	scanResult.strProblem += TEXT("”已不存在");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


HRESULT	CRegistryData::ScanClsid(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
	EnumRegSubKey_WithCallBack(HKEY_CLASSES_ROOT, 
                            scanData.scanPoint.strSubKey.c_str(), 
		                     &CRegistryData::CheckClsidSubKey, 
                            &data, 
                            WOW64_MISC
                            );

    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckClsidSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	tstring							strClsidKey;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

    data.scanPoint.wow64type = wow64type;
    data.scanPoint.strSubKey += lpszSubKeyName;	// {clsid}
    strClsidKey = data.scanPoint.strSubKey;
    strClsidKey += TEXT("\\");
    data.scanPoint.strSubKey += TEXT("\\InprocServer32\\");
    data.scanPoint.strValueName = TEXT("");

	// 检查InProcServer32\@指向的文件
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	hr = (data.lpThis->CheckFilePath)(scanResult);
    if ( hr == S_FALSE && scanResult.strInvalidPath.find(TEXT(":\\")) != tstring::npos )
	{
		// 删除hkcr\clsid\{guid}子键
		scanResult.scanData.strSubKey = strClsidKey;
		scanResult.rstType = data.rstType;
		scanResult.strTitle = data.strTitle;
		scanResult.bToClean = true;
		scanResult.cleaner = &CRegistryData::CleanKey;

		scanResult.strProblem = TEXT("对应的文件“");
		scanResult.strProblem += scanResult.strInvalidPath.c_str();
		scanResult.strProblem += TEXT("”已不存在");

		// 保存到清理列表中
		tstring strKeyPath;
		scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

		if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
		{
			pairResult = data.lpThis->m_scanResult.insert(scanResult);

			// 调用回调函数
			if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
			{
				data.lpThis->m_bStopScanning = true;
				return false;
			}
		}
	}

	// 枚举版本子键
	EnumRegSubKey_WithCallBack(data.scanPoint.hRootKey, 
                            data.scanPoint.strSubKey.c_str(),
		                     &CRegistryData::CheckClsidCodeBase, 
                            &data,
                            WOW64_DEFAULT);

    return data.lpThis->m_bStopScanning ? false : true;
}


bool CRegistryData::CheckClsidCodeBase(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

    data.scanPoint.wow64type = wow64type;
    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("CodeBase");

	// 检查子键InprocServer32\*\CodeBase对应的文件是否存在
	scanResult.scanData = data.scanPoint;
	scanResult.ppPos = PP_VALUE_DATA;
	hr = (data.lpThis->CheckFilePath)(scanResult);
	if ( hr != S_FALSE )
		return true;

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanValue;

	scanResult.strProblem = TEXT("对应的文件“");
	scanResult.strProblem += scanResult.strInvalidPath.c_str();
	scanResult.strProblem += TEXT("”已不存在");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


HRESULT	CRegistryData::ScanFileExts(const CRegistryData::SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
    if (IsX64System())
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckFileExts,
                                &data,
                                FORCE_WOW64_64
                                );
    }
    else
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckFileExts,
                                &data,
                                WOW64_DEFAULT
                                );
    }

    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckFileExts(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	tstring							strKey;
	bool							bHKCRKeyExist = true;
	bool							bListEmpty = false, bProgidEmpty = false, bUserChoiceEmpty = false;
	DWORD							dwType = REG_SZ;
	DWORD							cbData = 0;
	std::vector<TCHAR>				bufData(MAX_PATH, 0);
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

	// 子键名必须以.开始
	if ( lpszSubKeyName[0] != TEXT('.') )
		return true;

    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("");
    data.scanPoint.wow64type = wow64type;

	// 检查在HKCR下是否存在同名子键。若不存在，子键无效
	if ( !CheckIfKeyExist(HKEY_CLASSES_ROOT, lpszSubKeyName, data.scanPoint.wow64type) )
	{
		bHKCRKeyExist = false;
	}
	else
	{
		// OpenWithList
		strKey = data.scanPoint.strSubKey;
		strKey += TEXT("OpenWithList");
		bListEmpty = CheckIfKeyEmptyOrNonExist(data.scanPoint.hRootKey, strKey.c_str(), data.scanPoint.wow64type);

		// OpenWithProgids
		strKey = data.scanPoint.strSubKey;
		strKey += TEXT("OpenWithProgids");
		bProgidEmpty = CheckIfKeyEmptyOrNonExist(data.scanPoint.hRootKey, strKey.c_str(), data.scanPoint.wow64type);

		// UserChoice::ProgId
		strKey = data.scanPoint.strSubKey;
		strKey += TEXT("UserChoice");
		cbData = (DWORD)bufData.size() * sizeof(TCHAR);
		bResult = GetRegValueAll(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), TEXT("Progid"), 
			&dwType, (LPBYTE)&bufData[0], &cbData, data.scanPoint.wow64type);
		if ( !bResult )
		{
			// 键值不存在
			bUserChoiceEmpty = true;
		}
		else if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
		{
			// 键值类型不正确
			bUserChoiceEmpty = true;
		}
		else if ( ::_tcslen(&bufData[0]) == 0  )
		{
			// 键值为空
			bUserChoiceEmpty = true;
		}
		else if ( !CheckIfKeyExist(HKEY_CLASSES_ROOT, &bufData[0], data.scanPoint.wow64type) )
		{
			// 键值的值指向的子键HKCR\xx不存在
			bUserChoiceEmpty = true;
		} 
		else
		{
			bUserChoiceEmpty = false;
		}
	}
	
	if ( !bHKCRKeyExist || (bListEmpty && bProgidEmpty && bUserChoiceEmpty) )
	{
		scanResult.scanData = data.scanPoint;
		scanResult.rstType = data.rstType;
		scanResult.strTitle = data.strTitle;
		scanResult.strInvalidPath = TEXT("");
		scanResult.strProblem = TEXT("\"");
		scanResult.strProblem += lpszSubKeyName;
		scanResult.strProblem += TEXT("\"记录的历史信息已无效");
		scanResult.bToClean = true;
		scanResult.cleaner = &CRegistryData::CleanKey;

		// 保存到清理列表中
		tstring strKeyPath;
		scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

		if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
		{
			pairResult = data.lpThis->m_scanResult.insert(scanResult);

			// 调用回调函数
			if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
			{
				data.lpThis->m_bStopScanning = true;
				return false;
			}
		}
	}

	return true;
}


HRESULT CRegistryData::ScanWinRarOpenHistory(const SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data = scanData;
	CRegistryData::SCAN_RESULT	scanResult;
	HKEY						hSubKey = NULL;
	LONG						lResult = ERROR_SUCCESS;
	DWORD						cSubKey = 0;
	DWORD						cValue = 0;
	int							i = 0;
	const TCHAR*				lpKey[] = 
	{
		TEXT("Software\\WinRAR\\ArcHistory\\"), 
		TEXT("Software\\WinRAR\\DialogEditHistory\\ExtrPath\\"),
		TEXT("Software\\WinRAR\\DialogEditHistory\\ArcName\\"),
	};
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;
	
	data.lpThis = this;
	data.scanPoint.hRootKey = HKEY_CURRENT_USER;

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.ppPos = CRegistryData::PP_VALUE_DATA;
	scanResult.strProblem = TEXT("冗余的WinRar历史记录");
	scanResult.strInvalidPath = TEXT("");
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;
	
	for ( i = 0; i < sizeof(lpKey)/sizeof(lpKey[0]); ++i )
	{
		data.scanPoint.strSubKey = lpKey[i];
		data.scanPoint.strValueName = TEXT("");

		lResult = ::RegOpenKeyEx(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), 0, KEY_READ, &hSubKey);
		if ( lResult != ERROR_SUCCESS )
			continue;

		lResult = ::RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &cSubKey, NULL, NULL, &cValue, NULL, NULL, NULL, NULL);
		::RegCloseKey(hSubKey);
		if ( lResult != ERROR_SUCCESS )
			continue;

		if ( cValue == 0 )
			continue;

		// 保存到清理列表中
		scanResult.scanData = data.scanPoint;
		tstring strKeyPath;
		scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

		if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
		{
			pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
			if ( m_fnCallback != NULL && !(*m_fnCallback)(&*pairResult.first, m_lpCallbackParam) )
			{
				m_bStopScanning = true;
				break;
			}
		}
	}

	if ( m_bStopScanning )
		hr = S_FALSE;
	else
		hr = S_OK;

	return hr;
}


HRESULT CRegistryData::ScanWinZipOpenHistory(const SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data = scanData;
	CRegistryData::SCAN_RESULT	scanResult;
	HKEY						hSubKey = NULL;
	LONG						lResult = ERROR_SUCCESS;
	DWORD						cSubKey = 0;
	DWORD						cValue = 0;
	int							i = 0;
	const TCHAR*				lpKey[] = 
	{
		TEXT("Software\\Nico Mak Computing\\WinZip\\extract\\"), 
		TEXT("Software\\Nico Mak Computing\\WinZip\\mru\\archives\\"),
		TEXT("Software\\Nico Mak Computing\\WinZip\\mru\\jobs\\"),
	};
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	data.lpThis = this;
	data.scanPoint.hRootKey = HKEY_CURRENT_USER;

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.ppPos = CRegistryData::PP_VALUE_DATA;
	scanResult.strProblem = TEXT("冗余的WinZip历史记录");
	scanResult.strInvalidPath = TEXT("");
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;

	for ( i = 0; i < sizeof(lpKey)/sizeof(lpKey[0]); ++i )
	{
		data.scanPoint.strSubKey = lpKey[i];
		data.scanPoint.strValueName = TEXT("");

		lResult = ::RegOpenKeyEx(data.scanPoint.hRootKey, data.scanPoint.strSubKey.c_str(), 0, KEY_READ, &hSubKey);
		if ( lResult != ERROR_SUCCESS )
			continue;

		lResult = ::RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &cSubKey, NULL, NULL, &cValue, NULL, NULL, NULL, NULL);
		::RegCloseKey(hSubKey);
		if ( lResult != ERROR_SUCCESS )
			continue;

		// 若键值数>0
		if ( cValue == 0 )
			continue;

		// 保存到清理列表中
		scanResult.scanData = data.scanPoint;
		tstring strKeyPath;
		scanResult.scanData.GetKeyFullPath(strKeyPath, true);
		if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
		{
			pairResult = data.lpThis->m_scanResult.insert(scanResult);

			// 调用回调函数
			if ( m_fnCallback != NULL && !(*m_fnCallback)(&*pairResult.first, m_lpCallbackParam) )
			{
				m_bStopScanning = true;
				break;
			}
		}
	}

	if ( m_bStopScanning )
		hr = S_FALSE;
	else
		hr = S_OK;

	return hr;
}


HRESULT CRegistryData::ScanOfficeOpenHistory(const SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
	EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, scanData.scanPoint.strSubKey.c_str(), 
		&CRegistryData::CheckOfficeOpenHistory, &data, WOW64_DEFAULT );

	if ( m_bStopScanning )
		hr = S_FALSE;
	else
		hr = S_OK;

	return hr;
}


bool CRegistryData::CheckOfficeOpenHistory(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	// 参数不可为空
	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;
	const TCHAR*					lpListKey[] = 
	{
		TEXT("Word\\File MRU\\"), TEXT("Word\\Recent File List\\"),
		TEXT("PowerPoint\\File MRU\\"), TEXT("PowerPoint\\Recent File List\\"),
		TEXT("Excel\\File MRU\\"), TEXT("Excel\\Recent File List\\"),
		TEXT("Access\\File MRU\\"), TEXT("Access\\Recent File List\\"),
	};

	// 检查是否需要退出
	if ( data.lpThis->m_bStopScanning )
		return false;

    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("");
    data.scanPoint.wow64type = wow64type;

	// 逐一检查各子键
	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.strProblem = TEXT("冗余的office历史记录");
	scanResult.strInvalidPath = TEXT("");
	scanResult.ppPos = CRegistryData::PP_VALUE_DATA;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanKey;

	for ( int i = 0; i < sizeof(lpListKey)/sizeof(lpListKey[i]); ++i )
	{
		scanResult.scanData = data.scanPoint;
		scanResult.scanData.strSubKey += lpListKey[i];

		// 检查子键是否存在
		if ( !CheckIfKeyExist(scanResult.scanData.hRootKey, scanResult.scanData.strSubKey.c_str(), wow64type))
			continue;

		// 保存到清理列表中
		tstring strKeyPath;
		scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

		if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
		{
			pairResult = data.lpThis->m_scanResult.insert(scanResult);

			// 调用回调函数
			if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
			{
				data.lpThis->m_bStopScanning = true;
				return false;
			}
		}
	}

	return true;
}

HRESULT CRegistryData::ScanMUICache(const SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
    if (IsX64System())
    {
        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckMUICache, 
                                &data,
                                FORCE_WOW64_64);
    }

    else
    {
        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckMUICache, 
                                &data,
                                WOW64_DEFAULT);      
    }

    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckMUICache(HKEY hKey, 
                              const TCHAR* lpszValueName, 
                              const void* lpValueData, 
                              DWORD dwType, 
                              DWORD dwDataLen, 
                              LPVOID param,
                              Wow64Type wow64type
                              )
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;

	if ( param == NULL )
		return true;

	CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
	SCAN_RESULT						scanResult;
	std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

	if ( data.lpThis->m_bStopScanning )
		return false;

	if ( dwType != REG_SZ )
		return true;

	if ( lpszValueName != NULL )
		data.scanPoint.strValueName = lpszValueName;

	// 忽略以'@'开始的串，为系统的文件, @file,-id
	if ( data.scanPoint.strValueName.size() < 1 || data.scanPoint.strValueName[0] == TEXT('@') )
		return true;

    data.scanPoint.wow64type = wow64type;
    scanResult.scanData = data.scanPoint;
    scanResult.ppPos = PP_VALUE_NAME;
    hr = (data.lpThis->CheckFilePath)(scanResult);
	if ( hr != S_FALSE )
		return true;

	scanResult.rstType = data.rstType;
	scanResult.strTitle = data.strTitle;
	scanResult.bToClean = true;
	scanResult.cleaner = &CRegistryData::CleanValue;

	scanResult.strProblem = TEXT("对应的文件\"");
	scanResult.strProblem += scanResult.strInvalidPath.c_str();
	scanResult.strProblem += TEXT("\"已不存在");

	// 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

	// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

	return true;
}


HRESULT CRegistryData::ScanHelp(const SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
    if (IsX64System())
    {
        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckHelp, 
                                &data, 
                                FORCE_WOW64_64);

        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckHelp, 
                                &data, 
                                FORCE_WOW64_32);
    }
    else
    {
        EnumRegValue_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckHelp, 
                                &data, 
                                WOW64_DEFAULT);
    }
	

	return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckHelp(HKEY hKey, 
                           const TCHAR* lpszValueName, 
                           const void* lpValueData, 
                           DWORD dwType, 
                           DWORD dwDataLen, 
                           LPVOID param,
                           Wow64Type wow64type
                           )
{
    HRESULT							hr = E_FAIL;
    bool							bResult = false;

    if ( param == NULL )
        return true;

    CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
    SCAN_RESULT						scanResult;
    tstring							strAppend;

    std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

    if ( data.lpThis->m_bStopScanning )
        return false;

    if ( dwType != REG_SZ )
        return true;

    if ( lpszValueName != NULL )
        data.scanPoint.strValueName = lpszValueName;

    data.scanPoint.wow64type = wow64type;
    scanResult.scanData = data.scanPoint;
    scanResult.ppPos = PP_VALUE_DATA;
    scanResult.ptPath = PT_FILE;
    hr = (data.lpThis->CheckFilePath)(scanResult);
    if ( hr == S_OK )
        return true;
    else if ( hr == E_FAIL && ::_tcslen((const TCHAR*)lpValueData) != 0 )
        return true;

    // 检查键值值为目录时，是否有效
    strAppend = (LPCTSTR)lpValueData;
    if ( strAppend.size() > 1 && strAppend[strAppend.size() - 1] != TEXT('\\') )
        strAppend += TEXT("\\");
    strAppend += lpszValueName;
    hr = ::CheckFilePath(strAppend, wow64type, CRegistryData::PT_FILE);
    if ( hr == S_OK )
        return true;

    scanResult.rstType = data.rstType;
    scanResult.strTitle = data.strTitle;
    scanResult.bToClean = true;
    scanResult.cleaner = &CRegistryData::CleanValue;

    scanResult.strProblem = TEXT("\"");
    scanResult.strProblem += scanResult.strInvalidPath.c_str();
    scanResult.strProblem += TEXT("\"已不存在");

    // 保存到清理列表中
    tstring strKeyPath;
    scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

    if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
    {
        pairResult = data.lpThis->m_scanResult.insert(scanResult);

        // 调用回调函数
        if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
        {
            data.lpThis->m_bStopScanning = true;
            return false;
        }
    }

    return true;
}


HRESULT CRegistryData::ScanAppPath(const SCAN_DATA& scanData)
{
	HRESULT						hr = E_FAIL;
	CRegistryData::SCAN_DATA	data;

	data = scanData;
	data.lpThis = this;
	EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, scanData.scanPoint.strSubKey.c_str(), 
		&CRegistryData::CheckAppPath, &data, WOW64_MISC);

	return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckAppPath(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
    HRESULT				hr = E_FAIL;
    bool				bResult = false;

    // 参数不可为空
    if ( param == NULL )
        return true;

    CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
    SCAN_RESULT						scanResult;
    std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

    // 检查是否需要退出
    if ( data.lpThis->m_bStopScanning )
        return false;

    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("");
    data.scanPoint.wow64type = wow64type;

    scanResult.scanData = data.scanPoint;
    scanResult.ppPos = PP_VALUE_DATA;
    hr = (data.lpThis->CheckFilePath)(scanResult);
    if ( hr != S_FALSE )
        return true;

    // 逐一检查各子键
    scanResult.rstType = data.rstType;
    scanResult.strTitle = data.strTitle;
    scanResult.strProblem = TEXT("\"");
    scanResult.strProblem += scanResult.strInvalidPath;
    scanResult.strProblem += TEXT("\"不存在");
    scanResult.bToClean = true;
    scanResult.cleaner = &CRegistryData::CleanKey;

    // 保存到清理列表中
    tstring strKeyPath;
    scanResult.scanData.GetKeyFullPath(strKeyPath, true);

//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

    if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
    {
        pairResult = data.lpThis->m_scanResult.insert(scanResult);

        // 调用回调函数
        if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
        {
            data.lpThis->m_bStopScanning = true;
            return false;
        }
    }

    return true;
}


HRESULT CRegistryData::ScanMenuOrder(const SCAN_DATA& scanData)
{
    HRESULT						hr = E_FAIL;
    CRegistryData::SCAN_DATA	data;

    data = scanData;
    data.lpThis = this;
    if (IsX64System())
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckMenuOrder,
                                &data,
                                FORCE_WOW64_64);
    }
    else
    {
        EnumRegSubKey_WithCallBack(scanData.scanPoint.hRootKey, 
                                scanData.scanPoint.strSubKey.c_str(), 
                                &CRegistryData::CheckMenuOrder,
                                &data,
                                WOW64_DEFAULT);
    }
  
    return m_bStopScanning ? S_FALSE : S_OK;
}


bool CRegistryData::CheckMenuOrder(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type)
{
    HRESULT				hr = E_FAIL;
    bool				bResult = false;

    // 参数不可为空
    if ( param == NULL )
        return true;

    CRegistryData::SCAN_DATA		data = *(CRegistryData::SCAN_DATA*)param;
    SCAN_RESULT						scanResult;
    std::pair<std::set<SCAN_RESULT>::iterator,bool>	pairResult;

    // 检查是否需要退出
    if ( data.lpThis->m_bStopScanning )
        return false;

    data.scanPoint.strSubKey += lpszSubKeyName;
    data.scanPoint.strSubKey += TEXT("\\");
    data.scanPoint.strValueName = TEXT("");
    data.scanPoint.wow64type = wow64type;

    // 检查子键对应的菜单路径是否存在, 目前只检查第一层目录
    if ( CheckIfMenuItemExist(lpszSubKeyName, wow64type) )
        return true;

    scanResult.scanData = data.scanPoint;
    scanResult.ppPos = PP_VALUE_DATA;
    scanResult.rstType = data.rstType;
    scanResult.strTitle = data.strTitle;
    scanResult.strProblem = TEXT("开始菜单项\"");
    scanResult.strProblem += lpszSubKeyName;
    scanResult.strProblem += TEXT("\"已不存在");
    scanResult.bToClean = true;
    scanResult.cleaner = &CRegistryData::CleanKey;

    // 保存到清理列表中
	tstring strKeyPath;
	scanResult.scanData.GetKeyFullPath(strKeyPath, true);
    
//    KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Find RegTrash:" << UnicodeToAnsi(strKeyPath);

	if (!CStubbornRegs::Instance().IsStubbornReg(strKeyPath.c_str()))
	{
		pairResult = data.lpThis->m_scanResult.insert(scanResult);

		// 调用回调函数
		if ( data.lpThis->m_fnCallback != NULL && !(*data.lpThis->m_fnCallback)(&*pairResult.first, data.lpThis->m_lpCallbackParam) )
		{
			data.lpThis->m_bStopScanning = true;
			return false;
		}
	}

    return true;
}


//////////////////////////////////////////////////////////////////////////
// 传入一个命令行参数，检查对应的文件是否存在
// 若存在，返回S_OK；若不存在，返回S_FALSE; 其他值表示失败
//
HRESULT	CRegistryData::CheckCmdline(SCAN_RESULT& scanResult)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;
	tstring				strCmdline;
	std::vector<TCHAR>	bufFile(MAX_PATH, 0);

	// 取命令行参数
	if ( scanResult.ppPos == PP_VALUE_DATA )
	{
		DWORD				dwType = REG_SZ;
		DWORD				cbData = 0;
		std::vector<TCHAR>	bufData(3 * MAX_PATH, 0);

		cbData = (DWORD)bufData.size() * sizeof(TCHAR);
		bResult = GetRegValueAll(scanResult.scanData.hRootKey, 
                                scanResult.scanData.strSubKey.c_str(),  
                                scanResult.scanData.strValueName.c_str(), 
                                &dwType, 
                                (LPBYTE)&bufData[0], 
                                &cbData,
                                scanResult.scanData.wow64type);
		if ( !bResult )
			return E_FAIL;

		if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
			return E_FAIL;

		::StrTrim(&bufData[0], TEXT(" \t"));
		strCmdline = &bufData[0];
	}
	else if ( scanResult.ppPos == PP_VALUE_NAME )
	{
		strCmdline = scanResult.scanData.strValueName;
	}
	else
	{
		return E_INVALIDARG;
	}

    // 去除两个分斜杠
    if (strCmdline.find(_T("\\\\")) != tstring::npos)
    {
        tstring::size_type it = strCmdline.find(_T("\\\\"));
        while (it != tstring::npos)
        {
            strCmdline.erase(it, 1);
            it = strCmdline.find(_T("\\\\"));
        }
    }

	// 检查命令行参数
	if ( strCmdline.empty() )
		return E_FAIL;

	if ( !GetFirstPath(strCmdline.c_str(), &bufFile[0], (int)bufFile.size()) )
	{
		LPWSTR*		argv = NULL;
		int			argc = 0;
		
		argv = ::CommandLineToArgvW(strCmdline.c_str(), &argc);
		if ( argv == NULL )
			return E_FAIL;

		::_tcscpy(&bufFile[0], argv[0]);
		ExpandPath(&bufFile[0],scanResult.scanData.wow64type);
		::LocalFree(argv);
	}

	scanResult.strInvalidPath = strCmdline;
	if ( DoesFileExist(&bufFile[0]))
		return S_OK;
	
	return S_FALSE;
}


HRESULT	CRegistryData::CheckFilePath(SCAN_RESULT& scanResult)
{
	HRESULT				hr = E_FAIL;
	bool				bResult = false;
	tstring				strPath;
	std::vector<TCHAR>	bufFile(3*MAX_PATH, 0);
	LPCTSTR				lpValueName = NULL;

	// 取命令行参数
	if ( scanResult.ppPos == PP_VALUE_DATA )
	{
		DWORD				dwType = REG_SZ;
		DWORD				cbData = 0;
		std::vector<TCHAR>	bufData(3 * MAX_PATH, 0);

		cbData = (DWORD)bufData.size() * sizeof(TCHAR);
		bResult = GetRegValueAll(scanResult.scanData.hRootKey, 
                            scanResult.scanData.strSubKey.c_str(),  
                            scanResult.scanData.strValueName.c_str(), 
                            &dwType, 
                            (LPBYTE)&bufData[0], 
                            &cbData,
                            scanResult.scanData.wow64type);
		if ( !bResult )
			return E_FAIL;

		if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
			return E_FAIL;

		strPath = &bufData[0];
	}
	else if ( scanResult.ppPos == PP_VALUE_NAME )
	{
		strPath = scanResult.scanData.strValueName;
	}
	else
	{
		return E_INVALIDARG;
	}

	// 检查路径
	hr = ::CheckFilePath(strPath, scanResult.scanData.wow64type, scanResult.ptPath);
	scanResult.strInvalidPath = strPath;

	return hr;
}


//////////////////////////////////////////////////////////////////////////
HRESULT	CRegistryData::CleanValue(const CRegistryData::REG_DATA& scanData)
{
    HRESULT		hr = E_FAIL;
    HKEY       hKey = NULL;
    DWORD      dwretcode = 0;


    //hr = ::SHDeleteValue(scanData.hRootKey, scanData.strSubKey.c_str(), scanData.strValueName.c_str());

    dwretcode = ::RegOpenKeyEx(scanData.hRootKey, 
                            scanData.strSubKey.c_str(),
                            0,
                            KEY_SET_VALUE | GetRegAccess(scanData.wow64type),
                            &hKey);
    if (ERROR_SUCCESS != dwretcode)
        return hr;
  
    dwretcode = ::RegDeleteValueW(hKey, scanData.strValueName.c_str());

    hr = dwretcode;

    tstring strLog;
    scanData.GetKeyFullPath(strLog, true);

    if (ERROR_SUCCESS == dwretcode)
    {      
        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Clean Succeed:" << UnicodeToAnsi(strLog);
    }
    else
    {
        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Clean Failed:" << UnicodeToAnsi(strLog);
    }

    ::RegCloseKey(hKey);

    return hr;
}


//HRESULT	CRegistryData::CleanKey(const CRegistryData::REG_DATA& scanData)
//{
//	HRESULT		hr = E_FAIL;
//
//	// 递归删除子键
//	::SHDeleteKey(scanData.hRootKey, scanData.strSubKey.c_str());
//
//	return hr;
//}


//////////////////////////////////////////////////////////////////////////
// 检查文件（不包括目录）是否存在。
//
//////////////////////////////////////////////////////////////////////////
bool DoesFileExist(IN const TCHAR* lpszFilePath, BOOL bWow64Misc)
{
    bool				bReturn = false;
    void              *pFsRedirect = NULL;
    DWORD             dwAttrib;

	if ( lpszFilePath == NULL )
	{
		bReturn = false;
		goto Label_Ret;
	}

   dwAttrib = GetFileAttributes(lpszFilePath);
   if (INVALID_FILE_ATTRIBUTES == dwAttrib )
   {
       if (IsX64System())
       {
           DisableWow64FsRedirection(&pFsRedirect);
           if (GetFileAttributes(lpszFilePath) != INVALID_FILE_ATTRIBUTES)
               bReturn = true;
           RevertWow64FsRedirection(pFsRedirect);

           if (bWow64Misc && !bReturn)
           {
               CString strTemp = lpszFilePath;
               strTemp.MakeLower();
               strTemp.Replace(_T("\\windows\\system32"), _T("\\windows\\syswow64"));
               if (GetFileAttributes(strTemp) != INVALID_FILE_ATTRIBUTES)
                   bReturn = true;
           }

           if (bReturn)
               goto Label_Ret;
           
           dwAttrib = GetFileAttributes(lpszFilePath);
           if (INVALID_FILE_ATTRIBUTES == dwAttrib)
           {
               bReturn  = false;
               goto Label_Ret;
           }
       }
       else
       {
           bReturn  = false;
           goto Label_Ret;
       }
      
   }

	bReturn = true;
	goto Label_Ret;

Label_Ret:	
	return bReturn;
}


//////////////////////////////////////////////////////////////////////////
// 检查目录是否存在。
//
//////////////////////////////////////////////////////////////////////////
bool DoesDirectoryExist(IN const TCHAR* lpszFilePath)
{
    bool				bReturn = false;
    WIN32_FIND_DATA		wfd = {0};
    HANDLE				hFind = INVALID_HANDLE_VALUE;
    tstring				strPath;
    void              *pFsRedirect = NULL;
    DWORD             dwAttrib;

	if ( lpszFilePath == NULL )
	{
		bReturn = false;
		goto Label_Ret;
	}

    strPath = lpszFilePath;
    if ( strPath.size() >= 1 && (strPath[strPath.size() - 1] == TEXT('\\') || strPath[strPath.size() - 1] == TEXT('/')) )
        strPath.erase(strPath.size() - 1);

   dwAttrib = GetFileAttributes(strPath.c_str());
   if (INVALID_FILE_ATTRIBUTES == dwAttrib)
   {
       if (IsX64System())
       {
           DisableWow64FsRedirection(&pFsRedirect);
           dwAttrib = GetFileAttributes(strPath.c_str());
           if (INVALID_FILE_ATTRIBUTES == dwAttrib || !(FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
           {
               bReturn = false;
               goto Label_Ret;
           }

           else
           {
               bReturn = true;
               goto Label_Ret;
           }
       }

       else
       {
           bReturn = false;
           goto Label_Ret;
       }
     
   }

   else if (!(FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
   {
       bReturn = false;
       goto Label_Ret;
   }

	bReturn = true;
	goto Label_Ret;

Label_Ret:

    if (pFsRedirect)
        RevertWow64FsRedirection(pFsRedirect);

	return bReturn;
}


bool ExpandPath(TCHAR *lpszDllPath, Wow64Type wow64type)
{
	bool					bReturn = false;
	bool					bResult = false;
	UINT					uResult = 0;
	UINT					nChars = 0;
	std::vector<TCHAR>		szPath(3*MAX_PATH, 0);
	std::vector<TCHAR>		szTemp(3*MAX_PATH, 0);
	TCHAR					*lpszPrev = NULL;
	TCHAR					*lpszNext = NULL;

	// 可能的文件路径形式
	// 1. c:\windows\system32\a.dll
	// 2. %SystemRoot%\system32\a.dll
	// 3. "c:\windows\system32\a.dll"
	// 4. a.dll
	// 5. \\?\server\share\a.dll
	// 6. http://..., ftp://...

	// 去除引号
	szPath[0] = TEXT('\0');
	lpszPrev = lpszDllPath;
	while ( (lpszNext = ::_tcschr(lpszPrev, '"')) != NULL)
	{
		*lpszNext = TEXT('\0');
		++lpszNext;
		::_tcscat(&szPath[0], lpszPrev);
		lpszPrev = lpszNext;
	}
	::_tcscat(&szPath[0], lpszPrev);

	// 剔除""内的无效空格, 若文件名为空，返回失败
	::StrTrim(&szPath[0], TEXT(" \t"));
	if ( ::_tcslen(&szPath[0]) <= 0 )
	{
		bReturn = false;
		goto Label_Ret;
	}

    if (WOW64_MISC == wow64type && IsX64System())
    {
        CString strTemp = &szPath[0];
        BOOL bExist = FALSE;
        void* pOldFsRedir = NULL;

        strTemp.MakeLower();

        if (strTemp.Find(_T("%programfiles%")) != -1)
        {
            strTemp.Replace(_T("%programfiles%"), _T("%ProgramW6432%"));
            ::ExpandEnvironmentStrings(strTemp, &szTemp[0], (DWORD)szTemp.size());

            DisableWow64FsRedirection(&pOldFsRedir);
            if (::GetFileAttributes(&szTemp[0]) != INVALID_FILE_ATTRIBUTES)
            {
                _tcscpy(lpszDllPath, &szTemp[0]);
                bExist = TRUE;
            }
            RevertWow64FsRedirection(pOldFsRedir);
        }

        if (strTemp.Find(_T("%commonprogramfiles%")) != -1)
        {
            strTemp.Replace(_T("%commonprogramfiles%"), _T("%CommonProgramW6432%"));
            ::ExpandEnvironmentStrings(strTemp, &szTemp[0], (DWORD)szTemp.size());
            DisableWow64FsRedirection(&pOldFsRedir);
            if (::GetFileAttributes(&szTemp[0]) != INVALID_FILE_ATTRIBUTES)
            {
                _tcscpy(lpszDllPath, &szTemp[0]);
                bExist = TRUE;
            }
            RevertWow64FsRedirection(pOldFsRedir);
        }

        if (bExist)
        {
            bReturn = true;
            goto Label_Ret;
        }
    }

	// 扩展环境变量
	::ExpandEnvironmentStrings(&szPath[0], &szTemp[0], (DWORD)szTemp.size());
	::_tcscpy(&szPath[0], &szTemp[0]);
	nChars = (UINT)::_tcslen(&szPath[0]);

	// 检查是否有路径
	if ( nChars >= 3 && ::_istalpha(szPath[0]) && szPath[1] == TEXT(':') && szPath[2] == TEXT('\\') )
	{
		::_tcscpy(lpszDllPath, &szPath[0]);
		bReturn = true;
		goto Label_Ret;
	}

	// 是否为UNC
	if ( nChars > 2 &&  szPath[0] == TEXT('\\') && szPath[1] == TEXT('\\') )
	{
		::_tcscpy(lpszDllPath, &szPath[0]);
		bReturn = true;
		goto Label_Ret;
	}

	// 检查是否为网络协议
	if ( ::_tcsstr(&szPath[0], TEXT("://")) != NULL )
	{
		::_tcscpy(lpszDllPath, &szPath[0]);
		bReturn = true;
		goto Label_Ret;
	}

	// 补充为%sys32dir%
	uResult = ::GetSystemDirectory(&szTemp[0], (DWORD)szTemp.size());
	if ( uResult == 0 )
	{
		bReturn = false;
		goto Label_Ret;
	}

	if ( szTemp[::_tcslen(&szTemp[0]) - 1] != TEXT('\\'))
		::_tcscat(&szTemp[0], TEXT("\\"));
	::_tcscat(&szTemp[0], &szPath[0]);

	if ( DoesFileExist(&szTemp[0]) || DoesDirectoryExist(&szTemp[0]) )
	{
		::_tcscpy(lpszDllPath, &szTemp[0]);
		bReturn = true;
		goto Label_Ret;
	}

	// 若文件不存在，尝试%windir%
	uResult = ::GetWindowsDirectory(&szTemp[0], (DWORD)szTemp.size());
	if ( uResult == 0 )
	{
		bReturn = false;
		goto Label_Ret;
	}
	if ( szTemp[::_tcslen(&szTemp[0]) - 1] != TEXT('\\'))
		::_tcscat(&szTemp[0], TEXT("\\"));
	::_tcscat(&szTemp[0], &szPath[0]);
	
	if ( DoesFileExist(&szTemp[0], (wow64type == WOW64_MISC))|| DoesDirectoryExist(&szTemp[0]) )
	{
		::_tcscpy(lpszDllPath, &szTemp[0]);
		bReturn = true;
		goto Label_Ret;
	}

	// 使用源路径串
	bReturn = true;
	goto Label_Ret;

Label_Ret:
	return bReturn;
}


// 若子键不存在或为空子键，返回true
bool CheckIfKeyEmptyOrNonExist(HKEY hKey, const TCHAR* lpszSubKey,Wow64Type wow64type)
{
	bool		bReturn = false;
	HKEY		hSubKey = NULL;
	LONG		lResult = ERROR_SUCCESS;
	DWORD		dwType = REG_SZ;
	DWORD		cSubKey = 0;
	DWORD		cValue = 0;
	
	lResult = ::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ | GetRegAccess(wow64type), &hSubKey);
	if ( lResult == ERROR_FILE_NOT_FOUND )
    {
		bReturn = false;
        goto Exit0;
    }
    else if ( lResult == ERROR_ACCESS_DENIED )
    {
        bReturn = false;
        goto Exit0;
    }
	else if ( lResult != ERROR_SUCCESS )
    {
        bReturn = false;
        goto Exit0;
    }

	lResult = ::RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &cSubKey, NULL, NULL, &cValue, NULL, NULL, NULL, NULL);
	if ( lResult == ERROR_SUCCESS && cSubKey == 0 && cValue == 0 )
		bReturn = true;
	else if ( cSubKey == 0 && cValue == 1 && GetRegValueType(hKey, lpszSubKey, NULL, &dwType) ) // 若只有默认值，也为空
		bReturn = true;
	else
		bReturn = false;
Exit0:
	::RegCloseKey(hSubKey);
	return bReturn;
}


// 若子键为空（无下层子键、键值,默认值除外），返回true
bool CheckIfKeyEmpty(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type)
{
	bool		bReturn = false;
	HKEY		hSubKey = NULL;
	LONG		lResult = ERROR_SUCCESS;
	DWORD		cSubKey = 0;
	DWORD		cValue = 0;
	DWORD		dwType = REG_SZ;

	lResult = ::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ | GetRegAccess(wow64type), &hSubKey);
	if ( lResult != ERROR_SUCCESS )
		return false;

	lResult = ::RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &cSubKey, NULL, NULL, &cValue, NULL, NULL, NULL, NULL);
	if ( lResult == ERROR_SUCCESS && cSubKey == 0 && cValue == 0 )
		bReturn = true;
	else if ( cSubKey == 0 && cValue == 1 && GetRegValueType(hKey, lpszSubKey, NULL, &dwType) ) // 若只有默认值，也为空
		bReturn = true;
	else
		bReturn = false;

	::RegCloseKey(hSubKey);
	return bReturn;
}


// 若有下层子键，返回true
bool CheckIfHasSubKey(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type)
{
	bool		bReturn = false;
	HKEY		hSubKey = NULL;
	LONG		lResult = ERROR_SUCCESS;
	DWORD		cSubKey = 0;

	lResult = ::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ | GetRegAccess(wow64type), &hSubKey);
	if ( lResult == ERROR_FILE_NOT_FOUND )
		return false;
	else if ( lResult == ERROR_ACCESS_DENIED )
		return true;
	else if ( lResult != ERROR_SUCCESS )
		return true;

	lResult = ::RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &cSubKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if ( lResult == ERROR_SUCCESS && cSubKey == 0 )
		bReturn = false;
	else
		bReturn = true;

	::RegCloseKey(hSubKey);
	return bReturn;
}


// 检查子键是否存在, 存在返回true
bool CheckIfKeyExist(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type)
{
	bool		bReturn = false;
	HKEY		hSubKey = NULL;
	LONG		lResult = ERROR_SUCCESS;

	lResult = ::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ | GetRegAccess(wow64type), &hSubKey);
	if ( lResult == ERROR_FILE_NOT_FOUND )
		return false;
	else if ( lResult == ERROR_ACCESS_DENIED )
		return true;
	else if ( lResult != ERROR_SUCCESS )
		return false;

	::RegCloseKey(hSubKey);
	return true;
}



// 检查子键是否存在, 存在返回S_OK, 不存在返回S_FALSE
HRESULT CheckIfValueExist(HKEY hKey, const TCHAR* lpszSubKey, const TCHAR* lpszValueName)
{
	HRESULT		hr = S_OK;
	HKEY		hSubKey = NULL;
	LONG		lResult = ERROR_SUCCESS;
	DWORD		dwType = 0;

	lResult = ::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ, &hSubKey);
	if ( lResult == ERROR_FILE_NOT_FOUND )
		return S_FALSE;
	else if ( lResult == ERROR_ACCESS_DENIED )
		return E_FAIL;
	else if ( lResult != ERROR_SUCCESS )
		return E_FAIL;

	lResult = ::RegQueryValueEx(hKey, lpszValueName, NULL, &dwType, NULL, NULL);
	if ( lResult == ERROR_FILE_NOT_FOUND )
		hr = S_FALSE;
	else
		hr = E_FAIL;

	::RegCloseKey(hSubKey);
	return hr;
}

// 检查开始菜单程序中的菜单项是否存在，路径相对于"程序"目录
bool CheckIfMenuItemExist(const tstring& strItemPath, Wow64Type wow64type)
{
    bool                bReturn = false;
    std::vector<TCHAR>  buffer(MAX_PATH, 0);
    BOOL                bCUFailed = FALSE, bAUFailed = FALSE;

    if ( strItemPath.empty() )
        return true;
    
    // 检查当前用户的程序目录:CSIDL_PROGRAMS 
    bCUFailed = !::SHGetSpecialFolderPath(NULL, &buffer[0], CSIDL_PROGRAMS, FALSE);
    if ( !bCUFailed )
    {
        ::PathAppend(&buffer[0], strItemPath.c_str());
        if ( ::DoesDirectoryExist(&buffer[0]) )
        {
            bReturn = true;
            goto Label_Ret;
        }
    }

    // 检查共用的程序目录:CSIDL_COMMON_PROGRAMS 
    bAUFailed = !::SHGetSpecialFolderPath(NULL, &buffer[0], CSIDL_COMMON_PROGRAMS, FALSE);
    if ( !bAUFailed )
    {
        ::PathAppend(&buffer[0], strItemPath.c_str());
        if ( ::DoesDirectoryExist(&buffer[0]))
        {
            bReturn = true;
            goto Label_Ret;
        }
    }

    // 菜单目录不存在
    if ( bCUFailed || bAUFailed )
        bReturn = true;
    else
      bReturn = false;
    goto Label_Ret;

Label_Ret:
    return bReturn;
}


HRESULT	CheckFilePath(tstring& strPath, Wow64Type wow64type, CRegistryData::PATH_TYPE ptPath)
{
	HRESULT				hr = E_FAIL;
	std::vector<TCHAR>	bufFile(3*MAX_PATH, 0);
	LPCTSTR				lpValueName = NULL;

	// 检查路径, 若路径为空，返回
	if ( strPath.empty() )
		return E_FAIL;

	// 为file:///,去除前部
	if ( ::_tcsnicmp(strPath.c_str(), TEXT("file:///"), 8) == 0 )
		strPath.erase(0, 8);

	// 为网络协议路径，返回
	if ( strPath.find(TEXT("://")) != tstring::npos )
		return E_FAIL;

	if ( bufFile.size() < strPath.size() )
		bufFile.resize(strPath.size() + MAX_PATH, 0);

	// 展开路径
	::_tcscpy(&bufFile[0], strPath.c_str());
	ExpandPath(&bufFile[0], wow64type);

	// 去除前后的空白
	::StrTrim(&bufFile[0], TEXT(" \t"));
	strPath = &bufFile[0];
	if ( strPath.empty() )
		return E_FAIL;

	// 判断文件是否存在
	if ( (ptPath == CRegistryData::PT_FILE || ptPath == CRegistryData::PT_ALL) && DoesFileExist(&bufFile[0], (WOW64_MISC == wow64type)))
		return S_OK;

	if ( (ptPath == CRegistryData::PT_FOLDER || ptPath == CRegistryData::PT_ALL) && DoesDirectoryExist(&bufFile[0]))
		return S_OK;

	return S_FALSE;
}


HRESULT	CRegistryData::CleanKey(const REG_DATA& scanData)
{
	HRESULT			hr = E_FAIL;
	CRegKey			regKey;
	LONG			lRetCode = ERROR_SUCCESS;
	wstring			strLog;
	unsigned int	dwHandle = 0;
	WCHAR			szTemp[20] = {0};

	lRetCode = regKey.Open(scanData.hRootKey, scanData.strSubKey.c_str(), READ_CONTROL|WRITE_DAC | GetRegAccess(scanData.wow64type));
	if (lRetCode)
		goto clean0;

	GrantRegkeyAccess(regKey);
	regKey.Close();

	lRetCode = regKey.Open(scanData.hRootKey, scanData.strSubKey.c_str(), KEY_ALL_ACCESS | GetRegAccess(scanData.wow64type));
	if (lRetCode)
		goto clean0;

	GrantAllAccess(regKey);
	regKey.Close();

	lRetCode = regKey.Open(scanData.hRootKey, scanData.strSubKey.c_str(), KEY_ALL_ACCESS | GetRegAccess(scanData.wow64type));
	if (lRetCode)
		goto clean0;

	hr = ::SHDeleteKeyW(regKey, L"");

	if ( ERROR_SUCCESS == hr )
	{
        scanData.GetKeyFullPath(strLog, true);

        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Clean Succeed:" << UnicodeToAnsi(strLog);
	}
    else
    {
        scanData.GetKeyFullPath(strLog, true);

        KCLEAR_LOG(LOG_LEVEL_REGCLEAN) << "Clean Failed:" << UnicodeToAnsi(strLog);
    }

	regKey.Close();

clean0:
	return hr;
}
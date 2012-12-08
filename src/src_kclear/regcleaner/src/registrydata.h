#pragma once

//////////////////////////////////////////////////////////////////////////

#include <set>
#include <vector>
#include <string>
#include <tchar.h>
#include <windows.h>
#include "wow64helper.h"

//////////////////////////////////////////////////////////////////////////

int GrantRegkeyAccess(HKEY hKey);
void GrantAllAccess(HKEY hKey);

typedef std::basic_string<TCHAR>	tstring;


class CRegistryData
{
public:
	CRegistryData(void);
	~CRegistryData(void);

private:
	CRegistryData(const CRegistryData&);
	const CRegistryData& operator=(const CRegistryData&);

public:
	enum REG_SCAN_TYPE{
		RST_INVALID = -1,
		RST_BEGIN = 0,
		RST_SHAREDDLL = RST_BEGIN, 
		RST_OPENSAVEMRU, RST_EXTHISTORY, 
		//RST_WINRAR, RST_WINZIP, RST_OFFICE, 
		RST_STARTMENU, RST_MUICACHE, RST_HELP, RST_APPPATH,
		RST_UNINSTALL, RST_FILEASSOC, RST_COM, 
		RST_END};

	static const TCHAR* GetTitle(REG_SCAN_TYPE rst) 
	{
		static const TCHAR* rstTitle[] = 
		{
			TEXT("不存在的DLL文件记录"), TEXT("打开对话框历史记录"), TEXT("文件类型操作记录"),
			//TEXT("Winrar打开历史记录"), TEXT("Winzip打开历史记录"), TEXT("Office打开历史记录"),
			TEXT("过期的开始菜单项"), TEXT("缺失的MUI引用"), TEXT("无效的帮助文件"),
			TEXT("无效的应用程序路径"),
			TEXT("安装信息残留项"), TEXT("不完整的文件关联"), TEXT("没有用的COM组件"),
		};

		if ( (int)rst < RST_BEGIN ||  rst >= RST_END )
			return NULL;

		return rstTitle[rst];
	}

	BOOL m_bHaveUndelReg;

	struct REG_DATA
	{
		HKEY		hRootKey;			// 根键句柄
		tstring		strSubKey;			// 子键路径, 以\结尾
		tstring		strValueName;		// 键值名，若为子键，为空
        Wow64Type   wow64type;          //兼容64位操作系统

		const REG_DATA& operator=(const REG_DATA& rhs)
		{
			hRootKey = rhs.hRootKey;
			strSubKey = rhs.strSubKey;
			strValueName = rhs.strValueName;
            wow64type = rhs.wow64type;

			return *this;
		}

		bool operator<(const REG_DATA& rhs) const
		{
			if ( this->hRootKey < rhs.hRootKey )
				return true;
			else if ( this->hRootKey > rhs.hRootKey )
				return false;

			if ( this->strSubKey < rhs.strSubKey )
				return true;
			else if ( this->strSubKey > rhs.strSubKey )
				return false;

			if ( this->strValueName < rhs.strValueName )
				return true;
			else
				return false;
		}

		void GetKeyFullPath(tstring& strKeyPath, bool bValue) const
		{
			LPCTSTR		lpRootKey = NULL;

			switch ((int)(ULONG_PTR)hRootKey)
			{
			case HKEY_CLASSES_ROOT: 
				lpRootKey = TEXT("HKCR"); break;
			case HKEY_CURRENT_USER: 
				lpRootKey = TEXT("HKCU"); break;
			case HKEY_LOCAL_MACHINE: 
				lpRootKey = TEXT("HKLM"); break;
			case HKEY_USERS: 
				lpRootKey = TEXT("HKU"); break;
			case HKEY_CURRENT_CONFIG: 
				lpRootKey = TEXT("HKEY_CURRENT_CONFIG"); break;
			default:
				lpRootKey = TEXT("");
				break;
			}
			
			strKeyPath = lpRootKey;
			strKeyPath += TEXT("\\");
			strKeyPath += strSubKey;
			if ( bValue )
				strKeyPath += strValueName;
		}
	};

	struct SCAN_DATA;
	struct SCAN_RESULT;
	// 返回true表示扫描此类型，返回false表示跳过此类型
	typedef bool	(*TYPE_SCAN_CALLBACK)(REG_SCAN_TYPE rstType, LPVOID lpParam);
	typedef TYPE_SCAN_CALLBACK	TYPE_CLEAN_CALLBACK;
	// 返回值表示是否继续扫描，true继续，false中止扫描
	typedef	bool	(*SCAN_CALLBACK)(SCAN_RESULT* result, LPVOID lpParam);

	// scanner成功返回S_OK, 由回调函数中止返回S_FALE, 其它值为失败
	typedef HRESULT (CRegistryData::*SCANNER)(const SCAN_DATA& scanPoint);
	typedef	HRESULT	(CRegistryData::*CLEANER)(const REG_DATA& scanData);
	typedef HRESULT	(CRegistryData::*CHECKER)(SCAN_RESULT& scanResult);

	struct SCAN_DATA
	{
		REG_SCAN_TYPE	rstType;			// 扫描类型
		tstring			strTitle;			// 扫描的描述
		REG_DATA		scanPoint;			// 要扫描的位置
		SCANNER			scanner;			// 扫描器
		CRegistryData*	lpThis;				// 指向源对象

		const SCAN_DATA& operator=(const SCAN_DATA& rhs)
		{
			rstType = rhs.rstType;
			strTitle = rhs.strTitle;
			scanPoint = rhs.scanPoint;
			scanner = rhs.scanner;
			lpThis = rhs.lpThis;

			return *this;
		}

		bool operator<(const SCAN_DATA& rhs) const
		{
			if ( (int)this->rstType < (int)rhs.rstType )
				return true;
			else if ( (int)this->rstType > (int)rhs.rstType )
				return false;

			return this->scanPoint < rhs.scanPoint;
		}
	};

	enum PATH_POS{PP_VALUE_NAME, PP_VALUE_DATA};
	enum PATH_TYPE{PT_ALL = 0, PT_FILE, PT_FOLDER};
	struct SCAN_RESULT
	{
	public:
		friend CRegistryData;

		REG_SCAN_TYPE	rstType;
		tstring			strTitle;			// 扫描的描述
		REG_DATA		scanData;			// 扫描到数据
		PATH_POS		ppPos;				// 路径所在的位置
		PATH_TYPE		ptPath;				// 路径类型			
		tstring			strInvalidPath;		// 扫描到的无效文件路径
		tstring			strProblem;			// 问题描述
		bool			bToClean;			// 是否清除, 外部调用都可以设置此值
		CLEANER			cleaner;			// 清理器

		SCAN_RESULT():
			rstType(RST_INVALID), ppPos(PP_VALUE_DATA), ptPath(PT_ALL), bToClean(true), cleaner(NULL)
		{
		}

	public:
		void SetToClean(bool bToClean)
		{
			this->bToClean = bToClean;
		}

		bool GetToClean()
		{
			return this->bToClean;
		}

		void GetProblem(tstring& lpProblem) const
		{
			lpProblem = this->strProblem;
		}

		void GetInfo( REG_SCAN_TYPE& rstType, REG_DATA& lpRegData, tstring& lpProblem) const
		{
			rstType = this->rstType;
			lpProblem = strProblem;
			lpRegData = scanData;
		}

		bool operator<(const SCAN_RESULT& rhs) const
		{
			if ( (int)this->rstType < (int)rhs.rstType )
				return true;
			else if ( (int)this->rstType > (int)rhs.rstType )
				return false;

			return this->scanData < rhs.scanData;
		}
	};

private:
	std::set<SCAN_DATA>			m_scanPoints;		// 要扫描的位置
	std::set<SCAN_RESULT>		m_scanResult;		// 扫描结果
	bool						m_bCleaned;			// 是否已经清除过

	SCAN_CALLBACK				m_fnCallback;
	LPVOID						m_lpCallbackParam;

	bool						m_bStopScanning;	// 是否停止扫描

public:
	HRESULT	Scan(TYPE_SCAN_CALLBACK fnTypeCallback, LPVOID lpTypeParam, SCAN_CALLBACK fnCallback, LPVOID lpParam);
	HRESULT	Clean(TYPE_CLEAN_CALLBACK fnTypeCallback, LPVOID lpTypeParam);
	HRESULT	Stop();

private:
    HRESULT	ScanUninstall(const SCAN_DATA& scanPoint);
    HRESULT	ScanSharedDlls(const SCAN_DATA& scanPoint);
    HRESULT	ScanOpenSaveMRU(const SCAN_DATA& scanPoint);
    HRESULT	ScanFileAssociate(const SCAN_DATA& scanPoint);
    HRESULT	ScanTypeLib(const SCAN_DATA& scanPoint);
    HRESULT	ScanClsid(const SCAN_DATA& scanPoint);
    HRESULT	ScanFileExts(const SCAN_DATA& scanPoint);
    HRESULT	ScanWinRarOpenHistory(const SCAN_DATA& scanPoint);
    HRESULT	ScanWinZipOpenHistory(const SCAN_DATA& scanPoint);
    HRESULT	ScanOfficeOpenHistory(const SCAN_DATA& scanData);
    HRESULT	ScanMUICache(const SCAN_DATA& scanData);
    HRESULT	ScanHelp(const SCAN_DATA& scanData);
    HRESULT	ScanAppPath(const SCAN_DATA& scanData);
    HRESULT ScanMenuOrder(const SCAN_DATA& scanData);

    HRESULT	CheckCmdline(SCAN_RESULT& scanResult);
    HRESULT	CheckFilePath(SCAN_RESULT& scanResult);

    HRESULT	CleanValue(const REG_DATA& scanData);
    HRESULT	CleanKey(const REG_DATA& scanData);

private:
    static bool CheckUninstall(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckValueNamePath(HKEY hKey, const TCHAR* lpszValueName, const void* lpValueData, DWORD dwType, DWORD dwDataLen, LPVOID param, Wow64Type wow64type);
    static bool CheckOpenSaveMRUSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckOpenSaveMRUDataPath(HKEY hKey, const TCHAR* lpszValueName, const void* lpValueData, DWORD dwType, DWORD dwDataLen, LPVOID param, Wow64Type wow64type);
    static bool CheckFileAssociateSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckTypeLibSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckTypeLibWin32Key(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckClsidSubKey(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckClsidCodeBase(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckFileExts(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckOfficeOpenHistory(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckMUICache(HKEY hKey, const TCHAR* lpszValueName, const void* lpValueData, DWORD dwType, DWORD dwDataLen, LPVOID param, Wow64Type wow64type);
    static bool CheckHelp(HKEY hKey, const TCHAR* lpszValueName, const void* lpValueData, DWORD dwType, DWORD dwDataLen, LPVOID param, Wow64Type wow64type);
    static bool CheckAppPath(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
    static bool CheckMenuOrder(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);

    bool		CheckUninstallValue(const SCAN_DATA& data);
    bool		CheckUninstallString(const SCAN_DATA& data);
    bool		CheckExtAssociateKey(const SCAN_DATA& data);
    bool		CheckShellOpenCommand(const SCAN_DATA& scanData);
    bool		CheckDefaultIcon(const SCAN_DATA& scanData);

    HRESULT		CheckDisplayName(const SCAN_DATA& data);
};

HRESULT	CheckFilePath(tstring& strPath, Wow64Type wow64type, CRegistryData::PATH_TYPE ptPath = CRegistryData::PT_ALL);
bool	DoesFileExist(IN const TCHAR* lpszFilePath, BOOL bWow64Misc = FALSE);
bool	DoesDirectoryExist(IN const TCHAR* lpszFilePath);
bool	ExpandPath(TCHAR *lpszDllPath, Wow64Type wow64type);
bool	CheckIfKeyEmptyOrNonExist(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type);
bool	CheckIfKeyEmpty(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type);
bool	CheckIfKeyExist(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type);
bool	CheckIfHasSubKey(HKEY hKey, const TCHAR* lpszSubKey, Wow64Type wow64type);
bool    CheckIfMenuItemExist(const tstring& strItemPath, Wow64Type wow64type);

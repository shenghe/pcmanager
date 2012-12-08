#pragma once
#include "comproxy\comdata_scan_def.h"
#include "skylark2\bklog.h"




inline VOID InitFileResult( BK_FILE_RESULT& FileResult )
{
    FileResult.FileName     = L"";                         //文件名
    FileResult.Type         = BkQueryInfoFileTypeNull;     //扫描结果类型
    FileResult.Status       = BkFileStateNull;			   //文件状态（需要清除、清除成功、清除失败等）
    FileResult.Track        = BkFileTrackNull;			   //被检测出来时所在的扫描步骤（内存，启动项等）
    FileResult.VirusType    = BkVirusTypeNull;		       //病毒类型
    FileResult.MD5          = L"";                         //文件MD5
    FileResult.CRC          = 0;			               //文件CRC
    FileResult.VirusName    = L"";                         //病毒名 
    FileResult.SecLevel     = 0;
    FileResult.bHashed      = FALSE;
}


struct BK_FILE_RESULT_EX
{
    CString		FileName;		                //文件名
    DWORD		Type;                           //扫描结果类型
    DWORD		Status;			                //文件状态（需要清除、清除成功、清除失败等）
    DWORD		Track;			                //文件来源（内存，启动项、系统关键目录或者是普通文件枚举等）
    DWORD		VirusType;		                //病毒类型
    BYTE		MD5[16];			            //文件MD5
    DWORD		CRC;			                //文件CRC
    CString		VirusName;		                //病毒名
    //一下变量，目前只在保持日志时用到。
    UINT32          uScanResult;        ///< 扫描结果,取值为BKAVE_SCAN_RESULT_xxx
    UINT32          uCleanResult;       ///< 清除结果,取值为BKAVE_CLEAN_RESULT_xxx
    UINT32          uSecLevel;          ///< 文件安全等级,取值为SLPTL_LEVEL_MARK___xxx
    BOOLEAN         bHashed;            ///< 是否有hash结果
    BOOLEAN         bNeedUpload;        ///< 是否需要上传
};

inline VOID InitFileResultEx( BK_FILE_RESULT_EX& FileResultEx )
{
    FileResultEx.FileName     = L"";                         //文件名
    FileResultEx.Type         = BkQueryInfoFileTypeNull;     //扫描结果类型
    FileResultEx.Status       = BkFileStateNull;			   //文件状态（需要清除、清除成功、清除失败等）
    FileResultEx.Track        = BkFileTrackNull;			   //被检测出来时所在的扫描步骤（内存，启动项等）
    FileResultEx.VirusType    = BkVirusTypeNull;		       //病毒类型
    ::ZeroMemory( FileResultEx.MD5, sizeof(FileResultEx.MD5) );                       //文件MD5
    FileResultEx.CRC          = 0;			               //文件CRC
    FileResultEx.VirusName    = L"";                         //病毒名 
    
    FileResultEx.bHashed      = FALSE;
    FileResultEx.uCleanResult = 0;
    FileResultEx.uScanResult  = 0;
    FileResultEx.uSecLevel    = 0;
}

class CMD5
{
public:
	static void GetStrMd5( const BYTE md5[], CString& StrMd5 )
	{
		StrMd5.Format(
			L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			md5[0],  md5[1],  md5[2],  md5[3], 
			md5[4],  md5[5],  md5[6],  md5[7], 
			md5[8],  md5[9],  md5[10], md5[11], 
			md5[12], md5[13], md5[14], md5[15] );
	}
};


class CCrc
{
public:
	static void GetStrCrc( DWORD dwCRC32, CString& StrCrc )
	{
		StrCrc.Format(
			L"%08x", 
			StrCrc );
	}
};

class COther
{
	public:
		static DWORD GetVirusType( UINT32 ScanResult );
		static DWORD GetScanResult( DWORD SrcResult );
        static DWORD GetCleanResult( DWORD SrcResult );
        static int TranslateScanStepToTrack( int ScanStep );
        static int TranslateScanStep( int ScanStep );
        static void TranslateFileResultExToScanLog( const BK_FILE_RESULT_EX& FileResultEx, Skylark::BKENG_SCAN_LOG& ScanLog );
        static BOOL GetQQInstallDir( CString& InstallDir  );
        static BOOL GetMsnReceiveDir( CAtlArray<CString>& ReceiveDirArray );
};

class CDiskCheck
{
public:
    static BOOL IsDriveDeviceAccessible( WCHAR cRoot );
};


class CSetting
{
public:
    static void TranslateSetting( const BK_SCAN_SETTING& ScanSet, Skylark::BKENG_SETTINGS& BkengSet );


    static DWORD   ReadDWORDValue(LPCWSTR lpszFile, LPCWSTR lpszApp, LPCWSTR lpszKey, DWORD dwDefaultValue);
    static void    WriteDWORDValue(LPCWSTR lpszFile, LPCWSTR lpszApp, LPCWSTR lpszKey, DWORD dwValue);

};


#define BUFSIZ  512
class COtherUserCurReg
{
public:
    static HRESULT GetOtherUsrCurReg( CAtlArray<HKEY>& OtherUserCurReg )
    {
        OtherUserCurReg.RemoveAll();

        WCHAR szSID[BUFSIZ] = {0};
        DWORD dwIndex = 0;
        while (ERROR_SUCCESS == ::RegEnumKey(HKEY_USERS, dwIndex, szSID, _countof(szSID) - 1))
        {
            szSID[_countof(szSID) - 1]  = L'\0';
            HKEY hUser = NULL;
            LONG lRet = ::RegOpenKeyEx(HKEY_USERS, szSID, 0, KEY_ALL_ACCESS, &hUser);
            if (ERROR_SUCCESS == lRet)
            {
                OtherUserCurReg.Add(hUser);
            }

            ++dwIndex;
        }
        return S_OK;
    }
};
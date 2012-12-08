#include "stdafx.h"
#include "common.h"
#include "other.h"


#define MSG_INI_SETTING_FILE_NAME           L"msg.ini"
#define MSG_INI_SETTING_APP__HEURIST        L"he"
#define MSG_INI_SETTING_KEY__DISABLE        L"dis"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD COther::GetVirusType( UINT32 ScanResult )
{
    switch (ScanResult)
    {
    case (BKENG_SCAN_RESULT_VIRUS_NEED_DELETE):
        {
            return BkVirusTypeNeedDelete;
        }
        break;

    case (BKENG_SCAN_RESULT_VIRUS_NEED_DISINFECT):
        {
            return BkVirusTypeNeedClean;
        }
        break;

    case (BKENG_SCAN_RESULT_VIRUS_NO_OP):
        {
            return BkVirusTypeNoOperation;
        }
        break;

    default:
        {
            _ASSERT(0);
            return BkVirusTypeNull;
        }
        break;
    }
}



DWORD COther::GetScanResult( DWORD SrcResult )
{
    switch (SrcResult)
    {
    case (BKENG_SCAN_RESULT_SAFE):
        {
            return BkFileStateSafe;
        }
        break;

    case (BKENG_SCAN_RESULT_FAILED_TO_SCAN):
        {
            return BkFileStateScanFailure;
        }
        break;

    case (BKENG_SCAN_RESULT_VIRUS_NEED_DELETE):
    case (BKENG_SCAN_RESULT_VIRUS_NEED_DISINFECT):
        {
            return BkFileStateNeedClean;
        }
        break;

    case (BKENG_SCAN_RESULT_VIRUS_NO_OP):
        {
            return BkFileStateCannotClean;
        }
        break;

    case (BKENG_SCAN_RESULT_VIRUS_NEED_DISABLE_AUTORUN):
        {
            return BkFileStateNeedDisable;
        }
        break;

    default:
        {
            _ASSERT(0);
            return BkFileStateNull;
        }
        break;
    }
}

DWORD COther::GetCleanResult( DWORD SrcResult )
{
    switch (SrcResult)
    {
    case (BKENG_CLEAN_RESULT_NO_OP):
        {
            //目前这个返回结果，出现的情况是。
            //扫描是是病毒，但清除时有不是病毒了。
            return BkFileStateRepairSuccess;
        }
        break;

    case (BKENG_CLEAN_RESULT_DELETE):
        {
            return BkFileStateDeleteSuccess;
        }
        break;

    case (BKENG_CLEAN_RESULT_DISINFECT):
        {
            return BkFileStateRepairSuccess;
        }
        break;

    case (BKENG_CLEAN_RESULT_FAILED_TO_DELETE):
        {
            return BkFileStateDeleteFailed;
        }
        break;

    case (BKENG_CLEAN_RESULT_FAILED_TO_DISINFECT):
        {
            return BkFileStateRepairFailed;
        }
        break;

    case (BKENG_CLEAN_RESULT_DELETE_NEED_REBOOT):
        {
            return BkFileStateDeleteReboot;
        }
        break;

    case (BKENG_CLEAN_RESULT_DISINFECT_NEED_REBOOT):
        {
            return BkFileStateRepairReboot;
        }
        break;

    case (BKENG_CLEAN_RESULT_DISABLE_AUTORUN):
        {
            return BkFileStateDisabled;
        }
        break;

    default:
        {
            _ASSERT(0);
            return BkFileStateNull;
        }
        break;
    }
}



int COther::TranslateScanStepToTrack( int ScanStep )
{


    switch (ScanStep)
    {
    case (BkScanRelationDirStepMemory):
    case (BkScanStepMemory):
        {
            return BkFileTrackMemory;
        }
        break;

    case (BkScanRelationDirStepAutorun):
    case (BkScanStepAutorun):
        {
            return BkFileTrackAutorun;
        }
        break;

    case (BkScanStepCriticalDir):
        {
            return BkFileTrackCriticalDir;
        }
        break;

    case (BkScanStepCustomDir):
    case (BkScanStepAllDisk):
    case (BkScanStepMoveDisk):
        {
            return BkFileTrackNormalDir;
        }
        break;

    default:  
        {
            _ASSERT(0);
            return BkFileTrackNull;
        }
        break;
    }
}

int COther::TranslateScanStep( int ScanStep )
{


    switch (ScanStep)
    {
    case (BkScanRelationDirStepAutorun):
    case (BkScanRelationDirStepMemory):
        {
            return BkScanStepCriticalDir;
        }
        break;

    default:
        {
            return ScanStep;
        }
    }
}

void COther::TranslateFileResultExToScanLog( const BK_FILE_RESULT_EX& FileResultEx, Skylark::BKENG_SCAN_LOG& ScanLog )
{
    ScanLog.uSize = sizeof(ScanLog);
    ScanLog.bHashed         = FileResultEx.bHashed;
    ScanLog.uSecLevel       = FileResultEx.uSecLevel;
    ScanLog.uCleanResult    = FileResultEx.uCleanResult;
    ScanLog.uScanResult     = FileResultEx.uScanResult;
    ScanLog.uCRC32          = FileResultEx.CRC;

    wcsncpy( ScanLog.szPath, (LPCWSTR)(FileResultEx.FileName), MAX_PATH-1 );
    ScanLog.szPath[MAX_PATH-1] = NULL;
    wcsncpy( ScanLog.szVirusName, (LPCWSTR)(FileResultEx.VirusName), 64-1 );
    ScanLog.szVirusName[64-1] = NULL;

    ::CopyMemory( ScanLog.byMD5, FileResultEx.MD5, sizeof(FileResultEx.MD5) );
}


BOOL COther::GetQQInstallDir( CString& InstallDir  )
{
    BOOL bRet = FALSE;

    HKEY hKey = HKEY_LOCAL_MACHINE;
    CString SubKeyPath = L"SOFTWARE\\Tencent";

    LONG Result = ERROR_SUCCESS;
    CRegKey   QQMainKey;
    Result = QQMainKey.Open( hKey, SubKeyPath, KEY_READ );
    if ( ERROR_SUCCESS != Result )
    {
        return bRet;
    }

    DWORD Index = 0;
    DWORD BufTCharNum = 0;
    while ( true )
    {
        //得子键值名
        CString EnumSubKeyName;
        BufTCharNum = MAX_PATH+1;
        Result = QQMainKey.EnumKey( Index, EnumSubKeyName.GetBuffer(BufTCharNum), &BufTCharNum );
        EnumSubKeyName.ReleaseBuffer( (ERROR_SUCCESS!=Result)?(0):(-1) );
        if ( ERROR_SUCCESS != Result )
        {
            break;
        }
        ++Index;

        //打开子键
        CString EnumSubKeyPath;
        EnumSubKeyPath.Format( L"%s\\%s", SubKeyPath, EnumSubKeyName );

        CRegKey SubKey;
        Result = SubKey.Open( hKey, EnumSubKeyPath, KEY_READ );
        if ( ERROR_SUCCESS != Result )
        {
            continue;
        }

        //得安装路径
        CString InstallPath;
        BufTCharNum = MAX_PATH+1;
        Result = SubKey.QueryStringValue( L"install", InstallPath.GetBuffer(BufTCharNum), &BufTCharNum );
        InstallPath.ReleaseBuffer( (ERROR_SUCCESS!=Result)?(0):(-1) );
        if ( (ERROR_SUCCESS == Result)
            && (FALSE == InstallPath.IsEmpty()) )
        {
            InstallDir = InstallPath;
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}

BOOL COther::GetMsnReceiveDir( CAtlArray<CString>& ReceiveDirArray )
{

    HKEY hKey = NULL;
    CString SubKeyPath = L"Software\\Microsoft\\MSNMessenger";

    CAtlArray<HKEY> OtherUserCurReg;
    HRESULT hr = COtherUserCurReg::GetOtherUsrCurReg( OtherUserCurReg );
    if ( FAILED(hr) )
    {
        return FALSE;
    }

    size_t Num = OtherUserCurReg.GetCount();
    size_t i = 0;
    for (; i<Num; ++i )
    {
        hKey = OtherUserCurReg[i];

        CRegKey MsnMainKey;
        LONG Result = ERROR_SUCCESS;
        Result = MsnMainKey.Open( hKey, SubKeyPath, KEY_READ );
        if ( ERROR_SUCCESS != Result )
        {
            continue;
        }

        WCHAR TmpMsnReceivePath[MAX_PATH+1] ={0};
        DWORD BufByteNum = (MAX_PATH+1)*sizeof(TCHAR);
        Result = MsnMainKey.QueryBinaryValue( L"FtReceiveFolder", TmpMsnReceivePath, &BufByteNum );
        TmpMsnReceivePath[MAX_PATH] = NULL;
        if ( ERROR_SUCCESS == Result )
        {
            BOOL bAdd = TRUE;
            size_t NumRv = ReceiveDirArray.GetCount();
            size_t j = 0;
            for (; j<NumRv; ++j )
            {
                if ( 0 == ReceiveDirArray[j].CompareNoCase(TmpMsnReceivePath) )
                {
                    bAdd = FALSE;
                    break;
                }
            }
            if ( bAdd )
            {
                ReceiveDirArray.Add( TmpMsnReceivePath );
            }
        }
    }
    
    size_t nNum = OtherUserCurReg.GetCount();
    size_t Index = 0;
    for(; Index<nNum; ++Index )
    {
        ::RegCloseKey( OtherUserCurReg[Index] );
    }

    if ( ReceiveDirArray.IsEmpty() )
    {
        return FALSE;
    }
    return TRUE;
}


BOOL CDiskCheck::IsDriveDeviceAccessible( WCHAR cRoot )
{
    WCHAR szRootPath[MAX_PATH] = L"\\\\?\\A:\0";
    szRootPath[4] = cRoot;

    HANDLE hDevice = ::CreateFile(
        szRootPath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if ( INVALID_HANDLE_VALUE == hDevice )
    {
        return FALSE;
    }

    DWORD dwBytesAct  = 0;
    BOOL  bAccessible = ::DeviceIoControl(  
        hDevice,  
        IOCTL_DISK_CHECK_VERIFY,  
        NULL,  
        0,  
        NULL,  
        0,  
        &dwBytesAct,  
        NULL );

    ::CloseHandle(hDevice);
    hDevice = INVALID_HANDLE_VALUE;

    return bAccessible;
}



void CSetting::TranslateSetting( const BK_SCAN_SETTING& Setting, Skylark::BKENG_SETTINGS& ScanSetting )
{
    ////////////////////////////////
    //文件大小设置
    ScanSetting.uMaxFileSize = Setting.ScanFileSize;

    if ( BkScanFileTypeAllFile == Setting.nScanFileType )
    {
        //扫描所有文件
        BKENG_UNSET_BITS( ScanSetting.uAveScanMask, BKAVE_SCAN_MASK_SCAN_EXE_ONLY );
    }
    else if ( BkScanFileTypeExe == Setting.nScanFileType )
    {
        //只扫描 exe 文件
        BKENG_SET_BITS( ScanSetting.uAveScanMask, BKAVE_SCAN_MASK_SCAN_EXE_ONLY );
    }
    else
    {
        _ASSERT(0);
    }
    //扫描压缩包设置。
    if ( Setting.bScanArchive )
    {
        BKENG_SET_BITS( ScanSetting.uAveScanMask, BKAVE_SCAN_MASK_SCAN_FILE_IN_ARCHIVE );//设置
    }
    else
    {
        BKENG_UNSET_BITS( ScanSetting.uAveScanMask, BKAVE_SCAN_MASK_SCAN_FILE_IN_ARCHIVE );
    }
    //自动清除设置
    if ( Setting.bAutoClean )
    {
        BKENG_SET_BITS( ScanSetting.uAveCleanMask, BKAVE_CLEAN_MASK_AUTO_CLEAN );//设置
        //清除失败处理
        switch( Setting.CleanFailedOp )
        {
        case ( CleanFailedDelete ):
            {
                BKENG_SET_BITS( ScanSetting.uAveCleanMask, BKAVE_CLEAN_MASK_DELETE_IF_FAIL );
            }
            break;
        }
    }
    else
    {
        BKENG_UNSET_BITS( ScanSetting.uAveCleanMask, BKAVE_CLEAN_MASK_AUTO_CLEAN );//取消设置
    }
    //扫描策略 文件查杀
    if ( SCAN_POLICY_AVE == (Setting.nScanPolicy & SCAN_POLICY_AVE) )
    {
        BKENG_SET_BITS ( ScanSetting.uEnableReviewScanMask, BKENG_REVIEW_SCAN_MASK_AVENGINE );
    }
    else
    {
        BKENG_UNSET_BITS( ScanSetting.uEnableReviewScanMask, BKENG_REVIEW_SCAN_MASK_AVENGINE );
    }
    //扫描策略 云查杀
    if ( SCAN_POLICY_CLOUD == (Setting.nScanPolicy & SCAN_POLICY_CLOUD) )
    {
        BKENG_UNSET_BITS( ScanSetting.uCloudMask, BKCLOUD_MASK_DISABLE_NETWORK_QUERY );
    }
    else
    {
        BKENG_SET_BITS( ScanSetting.uCloudMask, BKCLOUD_MASK_DISABLE_NETWORK_QUERY );
    }
    //扫描策略 缓存查杀
    if ( SCAN_POLICY_CACHE == (Setting.nScanPolicy & SCAN_POLICY_CACHE) )
    {
        //需要缓存
        BKENG_SET_BITS( ScanSetting.uEnableQueryCacheMask, BKENG_CACHE_MASK_ALL );
    }
    else
    {
        BKENG_UNSET_BITS( ScanSetting.uEnableQueryCacheMask, BKENG_CACHE_MASK_ALL );
    }
    //扫描策略 黑缓存处理
    if ( SCAN_POLICY_BLACKCACHE == ( Setting.nScanPolicy & SCAN_POLICY_BLACKCACHE ) )
    {
        //需要黑缓存
        BKENG_SET_BITS( ScanSetting.uCloudMask, BKCLOUD_MASK_QUERY_BLACK_CACHE );
    }
    else
    {
        BKENG_UNSET_BITS( ScanSetting.uCloudMask, BKCLOUD_MASK_QUERY_BLACK_CACHE );
    }
    //扫描策略 强制重启删除
    if ( BKENG_MATCH_BITS( Setting.nScanPolicy, SCAN_POLICY_FORCE_BOOTDELETE ) )
    {
        //需要黑缓存
        BKENG_SET_BITS( ScanSetting.uCloudMask, BKCLOUD_MASK_FORCE_BOOT_CLEAN );
    }
    else
    {
        BKENG_UNSET_BITS( ScanSetting.uCloudMask, BKCLOUD_MASK_FORCE_BOOT_CLEAN );
    }
    ////////////////////////////////




    //////////////////////////////////////////////////////////////////////////
    // 扩展设置,覆盖基础设置
    if ( BKENG_MATCH_BITS( Setting.nScanPolicy, SCAN_PLOICY_FOR_RESCAN ) )
    {
        BKENG_UNSET_BITS(ScanSetting.uEnableQueryCacheMask, BKENG_CACHE_MASK_HASH_TO_LEVEL);
    }
    else
    {
        BKENG_SET_BITS(ScanSetting.uEnableQueryCacheMask, BKENG_CACHE_MASK_HASH_TO_LEVEL);
    }


    // ----------------------------------------
    // 读取msg.ini
    BOOL bDisabledHeur = TRUE;  // 默认启用启发
    Skylark::CWinPath MsgIniFile;
    HRESULT hr = Skylark::CSkylarkPath::GetLogPath(MsgIniFile.m_strPath, FALSE);
    if (SUCCEEDED(hr))
    {
        MsgIniFile.Append(MSG_INI_SETTING_FILE_NAME);

        bDisabledHeur = ReadDWORDValue(
            MsgIniFile.m_strPath,
            MSG_INI_SETTING_APP__HEURIST,
            MSG_INI_SETTING_KEY__DISABLE,
            0);
    }

    if (bDisabledHeur)
    {
        BKENG_UNSET_BITS(ScanSetting.uCloudMask,            BKCLOUD_MASK_USE_CLOUD_HEURIST);
        BKENG_UNSET_BITS(ScanSetting.uEnableReviewScanMask, BKENG_REVIEW_SCAN_MASK_USE_LOCAL_HEURIST);
    }
    else
    {
        BKENG_SET_BITS(ScanSetting.uCloudMask,             BKCLOUD_MASK_USE_CLOUD_HEURIST);
        BKENG_SET_BITS(ScanSetting.uEnableReviewScanMask,  BKENG_REVIEW_SCAN_MASK_USE_LOCAL_HEURIST);
    }
}


DWORD CSetting::ReadDWORDValue(LPCWSTR lpszFile, LPCWSTR lpszApp, LPCWSTR lpszKey, DWORD dwDefaultValue)
{
    return ::GetPrivateProfileIntW(lpszApp, lpszKey, dwDefaultValue, lpszFile);
}

void CSetting::WriteDWORDValue(LPCWSTR lpszFile, LPCWSTR lpszApp, LPCWSTR lpszKey, DWORD dwValue)
{
    CString strValue;
    strValue.Format(L"%lu", dwValue);

    ::WritePrivateProfileStringW(lpszApp, lpszKey, strValue, lpszFile);
}
#pragma once
#include "comproxy/comdata_scan_def.h"
#include "common/utility.h"
#include "winmod/winpath.h"
#include "common/runtimelog.h"

#define SETTING_NAME        L"scansetting.ini"
#define SETTING_SEC_SCAN    L"scan"
#define SETTING_SEC_SKYLARK L"skylark"

#define SETTING_SCAN_ScanArchive    L"ScanArchive"
#define SETTING_SCAN_ScanFileType   L"ScanFileType"
#define SETTING_SCAN_AutoClean      L"AutoClean"
#define SETTING_SKYLARK_JoinSkylark L"JoinSkylark"

class CSvcSetting
{
public:
    __declspec(noinline) static  CSvcSetting& Instance()
    {
        static  CSvcSetting    _ins;
        return _ins;
    }
public:
    CSvcSetting()
    {
        m_bAutoReport = TRUE;
        m_default.Reset();

		m_bAutoReportLast = TRUE;
		m_settingLast.Reset();

		_GetSettingPath(m_strFilePath);
		m_attrFile = WinMod::CWinPathAttr(m_strFilePath);
    }

    HRESULT LoadScanSetting( BK_SCAN_SETTING& setting )
    {
        HRESULT hr = S_OK;
		BOOL bScanArchive  = FALSE;
		BOOL bAutoClean    = FALSE;
		int  nScanFileType = 0;

		if ( m_attrFile.IsChanged() )
		{
			bScanArchive = ( BOOL )GetPrivateProfileInt( 
				SETTING_SEC_SCAN, 
				SETTING_SCAN_ScanArchive, 
				m_default.bScanArchive, 
				m_strFilePath
				);
			bAutoClean = ( BOOL )GetPrivateProfileInt( 
				SETTING_SEC_SCAN, 
				SETTING_SCAN_AutoClean, 
				m_default.bAutoClean, 
				m_strFilePath
				);
			nScanFileType = GetPrivateProfileInt( 
				SETTING_SEC_SCAN, 
				SETTING_SCAN_ScanFileType, 
				m_default.nScanFileType, 
				m_strFilePath
				);

			m_settingLast.bScanArchive  = bScanArchive;
			m_settingLast.bAutoClean    = bAutoClean;
			m_settingLast.nScanFileType = nScanFileType;
		}
		else
		{
			bScanArchive = m_settingLast.bScanArchive;
			bAutoClean = m_settingLast.bAutoClean;
			nScanFileType = m_settingLast.nScanFileType;
		}

        CRunTimeLog::WriteLog(
            WINMOD_LLVL_DEBUG,
            L"[CSvcSetting] change bScanArchive: %lu -> %lu",
            setting.bScanArchive,
            bScanArchive);

        CRunTimeLog::WriteLog(
            WINMOD_LLVL_DEBUG,
            L"[CSvcSetting] change bAutoClean: %lu -> %lu",
            setting.bAutoClean,
            bAutoClean);

        CRunTimeLog::WriteLog(
            WINMOD_LLVL_DEBUG,
            L"[CSvcSetting] change nScanFileType: %lu -> %lu",
            setting.nScanFileType,
            nScanFileType);

        setting.bScanArchive  = bScanArchive;
        setting.bAutoClean    = bAutoClean;
        setting.nScanFileType = nScanFileType;

        return hr;
    }

    HRESULT SaveScanSetting( BK_SCAN_SETTING& setting )
    {
        HRESULT hr = S_OK;
        CString value;   


        value.Format( L"%d", setting.nScanFileType );
        WritePrivateProfileString( 
            SETTING_SEC_SCAN, 
            SETTING_SCAN_ScanFileType, 
            value, 
            m_strFilePath
            );

        value.Format( L"%d", setting.bScanArchive );
        WritePrivateProfileString( 
            SETTING_SEC_SCAN, 
            SETTING_SCAN_ScanArchive, 
            value, 
            m_strFilePath
            );

        value.Format( L"%d", setting.bAutoClean );
        WritePrivateProfileString( 
            SETTING_SEC_SCAN, 
            SETTING_SCAN_AutoClean, 
            value, 
            m_strFilePath
            );

        return hr;
    }

    HRESULT GetAutoReport( BOOL& bAutoReport )
    {
        HRESULT hr = S_OK;
		BOOL bTempAutoReport;

		if ( m_attrFile.IsChanged() )
		{
			bTempAutoReport = ( BOOL )GetPrivateProfileInt( 
				SETTING_SEC_SKYLARK, 
				SETTING_SKYLARK_JoinSkylark, 
				m_bAutoReport, 
				m_strFilePath
				);

			m_bAutoReportLast = bTempAutoReport;
		}
		else
		{
			bTempAutoReport = m_bAutoReportLast;
		}

        CRunTimeLog::WriteLog(
            WINMOD_LLVL_DEBUG,
            L"[CSvcSetting] change bAutoReport: %lu -> %lu",
            bAutoReport,
            bTempAutoReport
			);

        bAutoReport = bTempAutoReport;
        return hr;
    }

    HRESULT SetAutoReport( BOOL& bAutoReport )
    {
        HRESULT hr = S_OK;
        CString value;
		
        value.Format( L"%d", bAutoReport );
        WritePrivateProfileString( 
            SETTING_SEC_SKYLARK, 
            SETTING_SKYLARK_JoinSkylark, 
            value, 
            m_strFilePath
            );

        return hr;
    }
private:
    HRESULT _GetSettingPath( CString& strPath )
    {
        HRESULT hr = S_OK;
        WinMod::CWinPath    path;

        hr = CAppPath::Instance().GetLeidianCfgPath( path.m_strPath );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        path.Append( SETTING_NAME );
        strPath = path.m_strPath;
        return hr;
    }

private:
    BK_SCAN_SETTING				m_default;
    BOOL						m_bAutoReport;

private:
	BOOL						m_bAutoReportLast;
	BK_SCAN_SETTING				m_settingLast;
	CString						m_strFilePath;
	WinMod::CWinPathAttr		m_attrFile;
};
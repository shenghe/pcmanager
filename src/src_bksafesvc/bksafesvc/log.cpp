#include "stdafx.h"
#include "common.h"
#include "log.h"




HRESULT CScanInfoLog::SaveScanInfo( BK_SCAN_INFO& ScanInfo )
{
	HRESULT RetResult = E_FAIL;

	CString LogFilePath;
	if ( FALSE == _GetLogFilePath(LogFilePath, ScanInfo.ScanMode) )
	{
		return RetResult;
	}

	BkXDX::CBKJsonCursor JsonCursor;
	JsonCursor.WriteData( ScanInfo );
	JsonCursor.m_doc.SaveFile( LogFilePath );

	RetResult = S_OK;
	return RetResult;
}


HRESULT CScanInfoLog::LoadScanInfo( BK_SCAN_INFO& ScanInfo )
{
	CString LogFilePath;
	if ( FALSE == _GetLogFilePath(LogFilePath,  ScanInfo.ScanMode ) )
	{
		return E_FAIL;
	}
    if ( FALSE == WinMod::CWinPathApi::IsFileExisting(LogFilePath) )
    {
        return S_FALSE;
    }

	BkXDX::CBKJsonCursor JsonCursor;
	if ( FALSE == JsonCursor.m_doc.LoadFile(LogFilePath) )
    {
        return E_FAIL;
    }

	JsonCursor.ReadData( ScanInfo );
	return S_OK;
}


BOOL CScanInfoLog::_GetLogFilePath( CString& FilePath, DWORD ScanMode  )
{

    WinMod::CWinPath LastScanFilePath;
    HRESULT hr = CAppPath::Instance().GetLeidianLogPath( LastScanFilePath.m_strPath );
    if ( FAILED( hr ) )
    {
        return FALSE;
    }

    switch (ScanMode)
    {
    case (BkScanModeBootSystemScan):
        {
            LastScanFilePath.Append( L"lastscan\\1.dat" );
        }
        break;

    case (BkScanModeCheckScan):
        {
            LastScanFilePath.Append( L"lastscan\\2.dat" );
        }
        break;

    case (BkScanModeCustomScan):
        {
            LastScanFilePath.Append( L"lastscan\\3.dat" );
        }
        break;

    case (BKScanModeMoveDiskScan):
        {
            LastScanFilePath.Append( L"lastscan\\4.dat" );
        }
        break;

    case (BkScanModeRightScan):
        {
            LastScanFilePath.Append( L"lastscan\\5.dat" );
        }
        break;

    case (BkScanModeFullScan):
        {
             LastScanFilePath.Append( L"lastscan\\6.dat" );
        }
        break;

    case (BkScanModeFastScan):
        {
            LastScanFilePath.Append( L"lastscan\\7.dat" );
        }
        break;

    case (BkScanModeOnlyCustomScan):
        {
            LastScanFilePath.Append( L"lastscan\\8.dat" );
        }
        break;

    case (BkScanModeIeFix):
        {
            LastScanFilePath.Append( L"lastscan\\9.dat" );
        }
        break;

    case (BkScanModeOnlyMemoryScan):
        {
            LastScanFilePath.Append( L"lastscan\\10.dat" );
        }
        break;

    default:
        {
            _ASSERT(0);
        }
        break;
    }
    FilePath = LastScanFilePath.m_strPath;
	return TRUE;
}
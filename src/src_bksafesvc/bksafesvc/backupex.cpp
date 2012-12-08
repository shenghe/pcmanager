#include "stdafx.h"
#include "common.h"
#include "backupex.h"

#include "..\..\src_bksafesvc\bksafesvc\unknownreport.h"

#define BACKUP_DLL_NAME L"ksafebak.dll"

HRESULT CBkBackupProxy::Initialize()
{
    HRESULT hr = E_FAIL;

    WinMod::CWinPath    Path;

    //
    hr = CAppPath::Instance().GetLeidianAppPath( Path.m_strPath ); 
    if ( SUCCEEDED(hr) )
    {   
        Path.Append( BACKUP_DLL_NAME );
        hr = _LoadBkBackup( Path.m_strPath );
        if ( SUCCEEDED(hr) )
        {
            //成功返回
            return hr;
        }
    }
    //执行到这说名之前失败。
    hr = Path.GetModuleFileName(NULL);
    if ( SUCCEEDED(hr) )
    {
        Path.RemoveFileSpec();
        Path.Append( BACKUP_DLL_NAME );
        hr = _LoadBkBackup( Path.m_strPath );
        if ( SUCCEEDED(hr) )
        {
            //成功返回
            return hr;
        }
    }
    //
    return hr;
}

HRESULT CBkBackupProxy::Uninitialize()
{
    if ( NULL != m_hModBackup )
    {
        ::FreeLibrary( m_hModBackup );
        m_hModBackup = NULL;
    }
    m_pfnBKBakCreateObject = NULL;

    return S_OK;
}


HRESULT CBkBackupProxy::BKBackupCreateObject( REFIID riid, void** ppvObj, DWORD dwBKEngVer )
{
    if ( FALSE == _CheckOpeateData() )
    {
        return E_FAIL;
    }
    return m_pfnBKBakCreateObject( riid, ppvObj, dwBKEngVer );
}


HRESULT CBkBackupProxy::_LoadBkBackup( LPCTSTR pPathDll )
{
    if ( NULL == pPathDll )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    do 
    {
        if ( NULL == m_hModBackup )
        {
            m_hModBackup = ::LoadLibrary( pPathDll );
            if ( NULL == m_hModBackup )
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }
        if ( NULL == m_pfnBKBakCreateObject )
        {
            m_pfnBKBakCreateObject = (PFN_BKBakCreateObject)::GetProcAddress( m_hModBackup, FN_BKBakCreateObject );
            if ( NULL == m_pfnBKBakCreateObject )
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }
        hr = S_OK;
    } while ( false );


    if ( FAILED(hr) )
    {
        if ( NULL != m_hModBackup )
        {
            ::FreeLibrary( m_hModBackup );
            m_hModBackup = NULL;
        }
        m_pfnBKBakCreateObject = NULL;
    }

    return hr;
}

HRESULT CBackupVirusFile::BackupFile( 
    LPCWSTR lpszFilePath, 
    const Skylark::BKENG_RESULT* pResult,
    UINT64&  uBackupID,
    void* pvFlag 
    )
{
    if ( FALSE == _CheckOperateData() )
    {
        return E_FAIL;
    }
    Skylark::BKBAK_BACKUP_PARAM BackupParam;
    BackupParam.uSize       = sizeof(BackupParam);
    BackupParam.bHashed     = pResult->bHashed;
    ::CopyMemory( BackupParam.byMD5, pResult->byMD5, sizeof(pResult->byMD5) );
    ::CopyMemory( BackupParam.cVirusName, pResult->szVirusName, sizeof(pResult->szVirusName) );
    BackupParam.uCRC32      = pResult->uCRC32;
    BackupParam.uSecLevel   = pResult->uSecLevel;

    Skylark::BKBAK_BACKUP_ID BackupID;
    BKENG_INIT(&BackupID);
    HRESULT hr = m_piBKFileBackup->BackupFile( lpszFilePath, &BackupParam, &BackupID );
    if (FAILED(hr))
        return hr;

    uBackupID = BackupID.uBackupID;

    return S_OK;
}
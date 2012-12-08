#pragma  once
#include "skylark2\bkbak.h"

class CBkBackupProxy
{
public:
    static CBkBackupProxy& Instance()
    {
        static CBkBackupProxy _BkBackupProxy;
        return _BkBackupProxy;
    }

    HRESULT Initialize();
    
    HRESULT Uninitialize();

    HRESULT BKBackupCreateObject( REFIID riid, void** ppvObj, DWORD dwBKEngVer = BKENG_CURRENT_VERSION );


private:
    CBkBackupProxy():m_hModBackup(NULL),m_pfnBKBakCreateObject(NULL){}
    ~CBkBackupProxy()
    {
        if ( NULL != m_hModBackup )
        {
            ::FreeLibrary( m_hModBackup );
            m_hModBackup = NULL;
        }
    }

    HRESULT _LoadBkBackup( LPCTSTR pPathDll );

    BOOL _CheckOpeateData()
    {
        if ( NULL == m_pfnBKBakCreateObject )
        {
            return FALSE;
        }
        return TRUE;
    }

private:
    HMODULE m_hModBackup;
    PFN_BKBakCreateObject m_pfnBKBakCreateObject;
};

class CBackupVirusFile
{
public:
    CBackupVirusFile():m_piBKFileBackup(NULL){};
    ~CBackupVirusFile()
    {
        if ( NULL != m_piBKFileBackup )
        {
            m_piBKFileBackup->Release();
            m_piBKFileBackup = NULL;
        }
    };

    HRESULT Initialize()
    {
        HRESULT hr = CBkBackupProxy::Instance().BKBackupCreateObject( __uuidof(Skylark::IBKFileBackup), (void**)&m_piBKFileBackup );
        if ( FAILED(hr) )
        {
            m_piBKFileBackup = NULL;
        }
        return hr;
    }

    HRESULT Uninitialize()
    {
        if ( NULL != m_piBKFileBackup )
        {
            m_piBKFileBackup->Release();
            m_piBKFileBackup = NULL;
        }
        return S_OK;
    }

    HRESULT BackupFile( 
        LPCWSTR lpszFilePath, 
        const Skylark::BKENG_RESULT* pResult, 
        UINT64&  uBackupID,
        void* pvFlag = NULL 
        );

private:
    BOOL _CheckOperateData()
    {
        return (NULL == m_piBKFileBackup) ? FALSE : TRUE;
    }
    Skylark::IBKFileBackup* m_piBKFileBackup;
};
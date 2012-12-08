#include "common/whitelist.h"
#include "winmod/winpath.h"
#include "comproxy/comdata_scan_def.h"
#include "skylark2/bkwhitelist.h"

class CGlobalWhiteList : public IBKUserWhiteList
{
public:
    static CGlobalWhiteList& Instance()
    {
        static CGlobalWhiteList _ins;
        return _ins;
    }

    void LoadGlobalWhiteList()
    {
        CObjGuard   guard( m_lock );
        CWhiteList  whitelist;

        m_globalwhite.RemoveAll();

        if ( SUCCEEDED( whitelist.Initialize() ) )
        {
            CAtlArray<CWhiteListItem>   itemArray;

            if ( SUCCEEDED( whitelist.LoadWhiteList( itemArray ) ) )
            {
                for ( size_t i = 0; i < itemArray.GetCount(); i++ )
                {
                    WinMod::CWinPath path = itemArray[ i ].m_filepath;

                    path.ExpandNormalizedPathName();
                    m_globalwhite[ path ] = itemArray[ i ].m_nType;
                }
            }

            whitelist.Uninitialize();
        }
    }

    BOOL Lookup( const CString& FilePath )
    {
        WinMod::CWinPath    path;

        path.m_strPath = FilePath;

        path.ExpandNormalizedPathName();

        {
            CObjGuard   guard( m_lock );

            return ( m_globalwhite.Lookup( path.m_strPath ) ? TRUE : FALSE );
        }
    }

    void BuildScanResult( BK_FILE_RESULT& ScanResult, const CString& FilePath )
    {
        ScanResult.bHashed = FALSE;
        ScanResult.FileName = FilePath;
        ScanResult.SecLevel = SLPTL_LEVEL_MARK___LOCAL_WHITE_FILE_NAME;
        ScanResult.Status = BkFileStateNull;
        ScanResult.Type = BkQueryInfoFileTypeSafe;
    }

	/**
	* @brief        查询文件是否存在
	* @return       非0为失败
	*/
	virtual HRESULT STDMETHODCALLTYPE Lookup(
		LPCTSTR lpszFileName,
		BOOL& bIsExist
		)
	{
		bIsExist = Lookup(lpszFileName);
		return 0;
	}
public:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;
    CObjLock                 m_lock;
    CAtlMap<CString, int>    m_globalwhite;
};
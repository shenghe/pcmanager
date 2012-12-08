#pragma once
#include "uplive.h"
#include <kscbase/kscfilepath.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

template< typename I >
class CProxy
{
public:
    CProxy() :
      m_pfnGetInterface( NULL ),
          m_pi( NULL )
      {
		m_hModule = NULL;
      }

      virtual ~CProxy()
      {
   //       ATLASSERT(NULL == m_pi);

          if (m_pi)
          {
              Uninitialize();
          }
      }

      HRESULT Initialize()
      {
          HRESULT hr = S_OK;
		  
        /*  KFilePath    PathLoadDllFile;
          PathLoadDllFile.GetFilePath(NULL);
          PathLoadDllFile.RemoveFileSpec();
		  PathLoadDllFile.RemoveFileSpec();
          PathLoadDllFile.Append( L"ksafeup.dll" );*/
		  WCHAR szPath[MAX_PATH] = {0};

		  GetModuleFileName(NULL, szPath, MAX_PATH);
		  PathRemoveFileSpec(szPath);
		  PathRemoveFileSpec(szPath);
		  PathAppend(szPath, L"ksafeup.dll");

		  m_hModule = ::LoadLibrary(/*PathLoadDllFile*/szPath);
          if (!m_hModule)
		  {
			  hr = S_FALSE;
			  goto Exit0;
		  }

          m_pfnGetInterface = ( PFN_CreateObject )GetProcAddress(m_hModule, FN_CreateObject );
          if ( NULL == m_pfnGetInterface )
          {
              hr = HRESULT_FROM_WIN32( GetLastError() );
              goto Exit0;
          }

          hr = m_pfnGetInterface( __uuidof( I ), ( LPVOID* )&m_pi );
          if ( FAILED( hr ) )
          {
              goto Exit0;
          }
Exit0:
          if ( FAILED( hr ) )
          {
              if ( m_pi )
              {
                  m_pi->Release();
                  m_pi = NULL;
              }
			  if (m_hModule != NULL)
			  {	  
				  ::FreeLibrary(m_hModule);
				  m_hModule = NULL;
			  }
		}
          return hr;
      }

      HRESULT Uninitialize()
      {
          if ( m_pi )
          {
              m_pi->Release();
              m_pi = NULL;
          }
		  if (m_hModule != NULL)
		  {	  
			  ::FreeLibrary(m_hModule);
			  m_hModule = NULL;
		  }
          return S_OK;
      }

      I*  GetInterface() { return m_pi; }
protected:
    PFN_CreateObject m_pfnGetInterface;
private:
    I*  m_pi;
	HMODULE m_hModule;
};

class CUpliveProxy : public CProxy<IUplive>
{
public:
    CUpliveProxy() :
      m_piUplive( NULL )
    {

    }

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = CProxy::Initialize();
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }

        m_piUplive = CProxy::GetInterface();
        if ( NULL == m_piUplive )
        {
            hr = E_NOINTERFACE;
            goto Exit0;
        }

        hr = m_piUplive->Initialize();
Exit0:
        if ( FAILED( hr ) )
        {
            CProxy::Uninitialize();
        }

        return hr;
    }

    HRESULT Uninitialize()
    {
        if ( m_piUplive )
        {
            m_piUplive->Uninitialize();
            CProxy::Uninitialize();
            m_piUplive = NULL;
        }
        return S_OK;
    }

    void    SetCallback( 
        IStdUpdateCallback* piStd, 
        IUpdatePolicyCallback* piUpdPolicy 
        )
    {
        if ( m_piUplive )
        {
            m_piUplive->SetCallback( piStd, piUpdPolicy );
        }
    }

    HRESULT Start()
    {
        if ( NULL == m_piUplive )
        {
            return E_NOINTERFACE;
        }

        return m_piUplive->Start();
    }

    void  Stop()
    {
        if ( m_piUplive )
        {
            m_piUplive->Stop();
        }
    }

    void  WaitExit( DWORD dwWaitTime = INFINITE )
    {
        if ( m_piUplive )
        {
            m_piUplive->WaitExit( dwWaitTime );
        }
    }

    HRESULT NotifyUpdateMessage()
    {
        if ( NULL == m_piUplive )
        {
            return E_NOINTERFACE;
        }

        return m_piUplive->NotifyUpdateMessage();
    }

    HRESULT NotifyUpdate( int nType )
    {
        if ( NULL == m_piUplive )
        {
            return E_NOINTERFACE;
        }

        return m_piUplive->NotifyUpdate( nType );
    }
private:
    IUplive*        m_piUplive;
};

class CUpdUtilityProxy : public CProxy<IUpdUtility>
{
public:
    CUpdUtilityProxy() :
      m_piUpdUtility( NULL )
    {

    }

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = CProxy::Initialize();
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }

        m_piUpdUtility = CProxy::GetInterface();
        if ( NULL == m_piUpdUtility )
        {
            hr = E_NOINTERFACE;
            goto Exit0;
        }

Exit0:
        if ( FAILED( hr ) )
        {
            CProxy::Uninitialize();
        }

        return hr;
    }

    HRESULT Uninitialize()
    {
        if ( m_piUpdUtility )
        {
            CProxy::Uninitialize();
            m_piUpdUtility = NULL;
        }
        return S_OK;
    }

    HRESULT DownloadFile(
        LPCWSTR lpwszUrl,
        LPCWSTR lpwszTargetPath, 
        BOOL bPack, 
        LPCWSTR lpwszHashCheck ,/* = NULL */ 
        IUpdateDownloadFileCallback* piCallback = NULL
        )
    {
        if ( NULL == m_piUpdUtility )
        {
            return E_NOINTERFACE;
        }

        return  m_piUpdUtility->DownloadFile(
            lpwszUrl,
            lpwszTargetPath,
            bPack,
            lpwszHashCheck,
            piCallback
            );
    }

    virtual BOOL    DoExpression( LPCWSTR lpwszExpression )
    {
        if ( NULL == m_piUpdUtility )
        {
            return E_NOINTERFACE;
        }

        return  m_piUpdUtility->DoExpression( lpwszExpression );
    }
private:
    IUpdUtility*        m_piUpdUtility;
};
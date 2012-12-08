#include "stdafx.h"
#include "autorun.h"
#include "bkautorun\bkautorun.h"



VOID TestAutoRun()
{
    IAutorunHandle * piAutorunHandle = NULL;


    HMODULE hModule = ::LoadLibrary( L"bkautorun.dll" );
    if ( NULL == hModule )
    {
        return;
    }

    PFN_BkCreateAutorunHandle pfnBkCreateAutorunHandle = (PFN_BkCreateAutorunHandle)::GetProcAddress( hModule, FN_BkCreateAutorunHandle );
    if ( NULL == pfnBkCreateAutorunHandle )
    {
        return;
    }

    HRESULT hr = pfnBkCreateAutorunHandle( &piAutorunHandle );
    if ( FAILED(hr) )
    {
        return;
    }



    
    hr = piAutorunHandle->Create();
    if ( FAILED(hr) )
    {
        return;
    }

    if (piAutorunHandle->FindFirstEntryItem())
    {
        do 
        {
            DWORD dwFileCount = piAutorunHandle->GetFileCount();

            for (DWORD dwFileIndex = 0; dwFileIndex < dwFileCount; dwFileIndex ++)
            {
                LPCWSTR lpszFileName = piAutorunHandle->GetFilePathAt(dwFileIndex);
                if (lpszFileName && lpszFileName[0])
                {
                    wprintf( L"%ws \n", lpszFileName );
                }
            }

        } while (piAutorunHandle->FindNextEntryItem());
    }
    piAutorunHandle->Close();


_Exit0:
    if ( piAutorunHandle )
    {
        piAutorunHandle->Release();
        piAutorunHandle = NULL;
    }
}



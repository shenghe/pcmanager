//////////////////////////////////////////////////////////////////////
///		@file		KxEMemFeatureMemActor.h
///		@author		YangBin
///		@date		2009-01-07
///
///		@brief		直接操作内存特征的实现类，供密保网游处理器使用
//////////////////////////////////////////////////////////////////////
#ifndef _KXE_MEM_FEATURE_MEM_ACTOR_H_
#define _KXE_MEM_FEATURE_MEM_ACTOR_H_

#include <strsafe.h>
#include "IKxEMemFeatureOp.h"

class CKxEMemFeatureMemActor : public IKxEMemFeatureReadOp
{
public:

    CKxEMemFeatureMemActor()
    {
        m_bInited = FALSE;        
        m_hMemFile  = INVALID_HANDLE_VALUE;
        m_dwMemContentsSize  = 0;
        m_pszMemFileContents = NULL;
        m_dwCurrentIndex = 0;
        m_dwIndexNum = 0;
    }

    ~CKxEMemFeatureMemActor()
    {
        m_bInited = FALSE;
        
        if ( m_hMemFile != INVALID_HANDLE_VALUE )
        {
            CloseHandle( m_hMemFile );
        }
        if ( m_pszMemFileContents )
        {
            delete [] m_pszMemFileContents;
            m_pszMemFileContents = NULL;
        }
    }

public:

    // !- IKxEMemFeatureOpInMem implement 
    virtual HRESULT __stdcall Initialize( 
        IN PWCHAR pwszMemResFileName
        )
    {
        HRESULT hr = E_FAIL;

        if ( m_bInited )
        {
            hr = S_OK;
            goto Exit0;
        }

        if ( m_hMemFile == INVALID_HANDLE_VALUE )
        {
            hr = _OpenFile( pwszMemResFileName );
            if ( FAILED(hr) )
            {
                goto Exit0;
            }
        }

        hr = _ReadFile();
        if ( FAILED(hr) )
        {
            goto Exit0;
        }
        m_dwIndexNum = KXE_MEM_FEATURE_GET_INDEX_NUM( m_pszMemFileContents );
        if ( m_dwIndexNum == 0 )
        {
            hr = E_FAIL;
            goto Exit0;
        }
        m_dwCurrentIndex = 0;
        m_bInited = TRUE;

Exit0:
        
        if ( FAILED(hr) )
        {
            if ( m_pszMemFileContents )
            {
                delete [] m_pszMemFileContents;
                m_pszMemFileContents = NULL;
            }
            if ( m_hMemFile != INVALID_HANDLE_VALUE )
            {
                CloseHandle( m_hMemFile );
                m_hMemFile = INVALID_HANDLE_VALUE;
            }            
        }

        return hr;
    }

    virtual HRESULT __stdcall UnInitialize()
    {
        if ( m_hMemFile != INVALID_HANDLE_VALUE )
        {           
            CloseHandle( m_hMemFile );
            m_hMemFile = INVALID_HANDLE_VALUE;
        }
        if ( m_pszMemFileContents )
        {
            delete [] m_pszMemFileContents;
            m_pszMemFileContents = NULL;
            m_dwMemContentsSize = 0;
            m_dwCurrentIndex = 0;
            m_dwIndexNum = 0;
        }

        return S_OK;
    }

    virtual HRESULT __stdcall GetFeaturesByProcId( 
        IN  ULONG  ulId, 
        OUT PUCHAR *ppszBlocks, 
        OUT PDWORD pdwBlockNum 
        )
    {
        HRESULT hr = E_FAIL;
        PKXE_MEM_FEATURE_INDEX pIndexBase = NULL;
        PKXE_MEM_FEATURE_SIGNATURE pSig = NULL;
        DWORD dwIndexNum = 0;
        DWORD dwIndex = 0;
        BOOL  bFound = FALSE;

        if ( m_pszMemFileContents == NULL || ppszBlocks == NULL || pdwBlockNum == NULL )
        {
            goto Exit0;
        }

        if ( m_dwIndexNum == 0 )
        {
            goto Exit0;
        }

        pIndexBase = KXE_MEM_FEATURE_GET_FIRST_INDEX( m_pszMemFileContents );
        for ( dwIndex = 0; dwIndex < m_dwIndexNum; dwIndex++ )
        {
            if ( pIndexBase[dwIndex].dwId == ulId )
            {
                bFound = TRUE;
                break;
            }
        }

        if ( !bFound )
        {
            goto Exit0;
        }

        pSig = KXE_MEM_FEATURE_GET_SIGNATURE( m_pszMemFileContents, pIndexBase[dwIndex].dwSigOffset );
        *pdwBlockNum = pSig->dwBlockNum;
        *ppszBlocks = (PUCHAR)(pSig->blocks);
        hr = S_OK;

    Exit0:

        return hr;
    }

    virtual HRESULT __stdcall GetMemFileVersion( IN SYSTEMTIME *pTime )
    {
        if ( pTime == NULL || !m_bInited || m_pszMemFileContents == NULL )
        {
            return E_FAIL;
        }

        *pTime = ((PKXE_MEM_FEATURE_FILE_HEADER)m_pszMemFileContents)->timestamp;
        return S_OK;
    }

    virtual DWORD __stdcall GetMemProcNum()
    {
        if ( !m_bInited || m_pszMemFileContents == NULL )
        {
            return 0;
        }

        return m_dwIndexNum;        
    }

    virtual BOOL __stdcall GetFirstProc( 
        OUT ULONG  *pulProcId, 
        OUT PUCHAR *ppszBlocks,
        OUT PDWORD pdwBlockNum 
        ) 
    {
        BOOL bExist = FALSE;
        PKXE_MEM_FEATURE_INDEX pIndexBase = NULL;
        PKXE_MEM_FEATURE_SIGNATURE pSig = NULL;

        if ( pulProcId == NULL || ppszBlocks == NULL || pdwBlockNum == NULL )
        {
            goto Exit0;
        }

        if ( !m_bInited )
        {
            goto Exit0;
        }

        pIndexBase = KXE_MEM_FEATURE_GET_FIRST_INDEX( m_pszMemFileContents );
        pSig = KXE_MEM_FEATURE_GET_SIGNATURE( m_pszMemFileContents, pIndexBase[0].dwSigOffset );
                
        *pulProcId = pIndexBase[0].dwId;
        *pdwBlockNum = pSig->dwBlockNum;
        *ppszBlocks = (PUCHAR)pSig->blocks;

        m_dwCurrentIndex = 1;
        bExist = TRUE;

    Exit0:

        return bExist;
    }

    virtual BOOL __stdcall GetNextProc(
        OUT ULONG  *pulProcId, 
        OUT PUCHAR *ppszBlocks,
        OUT PDWORD pdwBlockNum 
        ) 
    {
        BOOL bExist = FALSE;
        PKXE_MEM_FEATURE_INDEX pIndexBase = NULL;
        PKXE_MEM_FEATURE_SIGNATURE pSig = NULL;

        if ( pulProcId == NULL || ppszBlocks == NULL || pdwBlockNum == NULL )
        {
            goto Exit0;
        }

        if ( !m_bInited || m_dwCurrentIndex >= m_dwIndexNum )
        {
            goto Exit0;
        }

        pIndexBase = KXE_MEM_FEATURE_GET_FIRST_INDEX( m_pszMemFileContents );
        pSig = KXE_MEM_FEATURE_GET_SIGNATURE( m_pszMemFileContents, pIndexBase[m_dwCurrentIndex].dwSigOffset );

        *pulProcId = pIndexBase[m_dwCurrentIndex].dwId;
        *pdwBlockNum = pSig->dwBlockNum;
        *ppszBlocks = (PUCHAR)pSig->blocks;

        m_dwCurrentIndex++;
        bExist = TRUE;

    Exit0:

        return bExist;
    }

protected:
    
    HRESULT _OpenFile( 
        IN  PWCHAR pwszFileName        
        )
    {
        HRESULT hr = E_FAIL;
        WCHAR   wszFullName[MAX_PATH + 1];
        PWCHAR  pwszPath = NULL;
        DWORD   dwLen = 0;
        
        if ( pwszFileName == NULL )
        {
            goto Exit0;
        }
        ::memset( wszFullName, 0, sizeof(wszFullName) );

        if ( wcschr( pwszFileName, L':' ) == NULL )
        {
            dwLen = ::GetModuleFileNameW( NULL, wszFullName, MAX_PATH );
            if ( dwLen == 0 )
            {
                goto Exit0;
            }

            pwszPath = ::wcsrchr( wszFullName, L'\\' );
            if ( pwszPath == NULL )
            {	
                goto Exit0;
            }

            pwszPath++;
            *pwszPath = WCHAR(0);

            ::StringCchCatW( wszFullName, sizeof(wszFullName), pwszFileName );
        }
        else
        {
            wcscpy_s( wszFullName, sizeof(wszFullName) / sizeof(WCHAR), pwszFileName );
        }

        m_hMemFile = ::CreateFileW( 
            wszFullName, 
            GENERIC_READ, 
            0, 
            NULL, 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, 
            NULL
            );
        if ( m_hMemFile != INVALID_HANDLE_VALUE )
        {
            hr = S_OK;
        }

    Exit0:

        return hr;
    }

    HRESULT _ReadFile()
    {
        HRESULT hr = E_FAIL;
        DWORD   dwFileSizeLow = 0;
        DWORD   dwFileSizeHigh = 0;
        DWORD   dwNumberOfBytesRead = 0;
        BOOL    bOk = FALSE;

        if ( m_hMemFile == INVALID_HANDLE_VALUE )
        {
            goto Exit0;
        }

        dwFileSizeLow = GetFileSize( m_hMemFile, &dwFileSizeHigh );
        if ( dwFileSizeLow == 0 )
        {   
            goto Exit0;
        }

        if ( m_pszMemFileContents )
        {
            delete [] m_pszMemFileContents;
            m_pszMemFileContents = NULL;
        }

        m_pszMemFileContents = new UCHAR[dwFileSizeLow];
        if ( m_pszMemFileContents == NULL )
        {
            goto Exit0;
        }

        bOk = ::ReadFile( 
            m_hMemFile, 
            (PVOID)m_pszMemFileContents,
            dwFileSizeLow,
            &dwNumberOfBytesRead, 
            NULL 
            );
        if ( !bOk )
        {
            goto Exit0;
        }

        m_dwMemContentsSize = dwNumberOfBytesRead;
        hr = S_OK;

    Exit0:

        return hr;
    }

protected:

    BOOLEAN m_bInited;
    
    HANDLE  m_hMemFile;
    PUCHAR  m_pszMemFileContents;
    DWORD   m_dwMemContentsSize;
    DWORD   m_dwIndexNum;

    DWORD   m_dwCurrentIndex;
};

#endif
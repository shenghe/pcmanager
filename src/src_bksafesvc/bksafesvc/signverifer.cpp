#include "StdAfx.h"
#include "signverifer.h"

CSignVerifer::CSignVerifer(void)
{
}

CSignVerifer::~CSignVerifer(void)
{
}


HRESULT CSignVerifer::VerifyFile( LPCTSTR lpPath )
{
    DWORD   dwWinTrust;
    HRESULT hr = S_OK;
    WinMod::CWinTrustSignerInfoChain signInfoChain;

    dwWinTrust = m_verifer.VerifyFile( lpPath, &signInfoChain );
    if (dwWinTrust <= 0x0000FFFF)
        hr = AtlHresultFromWin32(dwWinTrust);
    else
        hr = dwWinTrust;

    if ( SUCCEEDED( hr ) )
    {
        if ( !signInfoChain.m_chain.IsEmpty() 
            && IsTrustSign( signInfoChain.m_chain.GetHead().m_strIsserName ) )
        {
            hr = S_OK;
        }
        else
        {
            hr = E_ACCESSDENIED;
        }
    }

    return hr;
}

BOOL CSignVerifer::IsTrustSign( LPCTSTR lpIsserName )
{
    if ( ( 0 == StrCmp( lpIsserName, L"Kingsoft Security Co.,Ltd" ) ) 
        || ( 0 == StrCmp( lpIsserName, L"Beike Internet Security Technology Co.,Ltd" ) ) 
        || ( 0 == StrCmp( lpIsserName, L"Keniu Network Technology (Beijing) Co., Ltd." ) )
		|| ( 0 == StrCmp( lpIsserName, L"Baidu Online Network Technology (Beijing) Co., Ltd." ) ))
    {
        return TRUE;
    }

    return FALSE;
}
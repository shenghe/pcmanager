////////////////////////////////////////////////////////////////////////////////
//      
//      File for WebSitRep
//      
//      File      : WebSiteRep
//      Version   : 1.0
//      Comment   : 网址可信认证
//      
//      Create at : 2008-10-9
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "kis/kpfw/websiterep.h"
#include "3rdparty/md5.h"
#include "scom/SCOM/SCOMDef.h"
#include "scom/SCOM/SCOMBase.h"
#include "Base64Util.h"
#include <vector>
#include <string>
using namespace std;

struct UrlItem
{
	CString				md5;
	CString				base64;
	string				url;
	eWebSiteTrustLevel	Level;
	PVOID				data;
};

class KUrlProvider: public IUrlProvider,
					public KSComRoot<KUrlProvider>
{
private:
	typedef vector<UrlItem*>		UrlList;

	UrlList			m_UrlList;

public:
	KUrlProvider()
	{

	}

	~KUrlProvider()
	{
		Clear();
	}

	void	AddWebUrl(const char* pUrl, PVOID data)
	{
		// todo: 先进行切分
		UrlItem* pItem = new UrlItem;
		pItem->data = data;
		pItem->url = pUrl;
		GetAnsiMD5String(pUrl, pItem->md5);
		pItem->base64 = KBase64Encode(pUrl);
		m_UrlList.push_back(pItem);
	}

	UrlItem* GetUrlInfo(int index)
	{
		if (index < 0 || index >= (INT)m_UrlList.size())
			return NULL;
		return m_UrlList[index];
	}

	STDMETHODIMP_(INT) GetUrlCount()
	{
		return (INT)m_UrlList.size();
	}

	STDMETHODIMP_(const WCHAR*) GetUrlMd5(INT index)
	{
		if (index < 0 || index >= (INT)m_UrlList.size())
			return NULL;
		return m_UrlList[index]->md5;
	}

	STDMETHODIMP_(const WCHAR*) GetUrlBase64(INT index)
	{
		if (index < 0 || index >= (INT)m_UrlList.size())
			return NULL;
		return m_UrlList[index]->base64;
	}

	STDMETHODIMP_(const char*) GetUrl(INT index)
	{
		if (index < 0 || index >= (INT)m_UrlList.size())
			return NULL;
		return m_UrlList[index]->url.c_str();
	}

	STDMETHODIMP SetUrlInfo(const char* purl, const char* phit_key, eWebSiteTrustLevel nTrustLevel)
	{
		for (int i = 0; i < (INT)m_UrlList.size(); i++)
		{
			if (stricmp(purl, m_UrlList[i]->url.c_str()) == 0)
			{
				m_UrlList[i]->Level = nTrustLevel;
				return S_OK;
			}
		}
		return E_FAIL;
	}

	KS_DEFINE_GETCLSID(CLSID_KisUrlProvider);
	DECLARE_INDEPENDENT_OBJECT();

	KSCOM_QUERY_BEGIN
		KSCOM_QUERY_ENTRY(IUrlProvider)
	KSCOM_QUERY_END

private:

	BOOL GetAnsiMD5String(LPCSTR lpAnsiText,CString& strOut)
	{
		return GetBufferMD5String((LPBYTE)lpAnsiText, (DWORD)strlen(lpAnsiText), strOut);
	}

	BOOL GetBufferMD5String(LPBYTE lpBuf,DWORD cbSize, CString& strOut)
	{
		BYTE md5bin[16] = {0};
		if (!GetBufferMD5(lpBuf, cbSize, md5bin))
			return FALSE;
		return FormatToMD5String(md5bin, strOut);
	}

	BOOL GetBufferMD5(
		/* in  */ LPBYTE   lpBuf,
		/* in  */ DWORD    cbSize,
		/* out */ BYTE     md5bin[16])
	{
		Md5     md5calc;
		md5calc.md5_update(lpBuf, cbSize);
		md5calc.md5_finish(md5bin);
		return TRUE;
	}

	BOOL FormatToMD5String( const BYTE  md5bin[16], CString& strOut)
	{
		strOut.Empty();
		for ( int i = 0; i < 16; i++ )
			strOut.AppendFormat(_T("%02x"), md5bin[i]);
		return TRUE;
	}

	void Clear()
	{
		UrlList::iterator it = m_UrlList.begin();
		for (; it != m_UrlList.end(); it++)
			delete *it;
		m_UrlList.clear();
	}
};
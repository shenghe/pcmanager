#include "StdAfx.h"
#include "XmlManager.h"
#include "Utils.h"

BOOL CXmlManager::LoadFromFile( LPCTSTR szfilename )
{
	BOOL ret = FALSE;
	CAtlFile file;
	if(S_OK == file.Create(szfilename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING) )
	{
		int nsize = ::GetFileSize(file, NULL);
		char* szbuf = new char[nsize + 1];
		if(szbuf)
		{
			if(S_OK == file.Read(szbuf, nsize))
			{
				szbuf[nsize] = 0;
				ret = LoadFromString(szbuf, nsize);
			}
			delete [] szbuf;
		}
	}
	return ret;
}

BOOL CXmlManager::LoadFromXMLString( LPCTSTR lpXMLStr )
{
	m_pXmlDocument = NULL;
	if(FAILED(m_pXmlDocument.CoCreateInstance(CLSID_DOMDocument))
		|| !m_pXmlDocument
		|| FAILED(m_pXmlDocument->put_async(VARIANT_FALSE)) )
	{
		return FALSE;
	}
	
	
	static int nWinVer = -1;
	if(nWinVer==-1)
		nWinVer = GetWinVer();
	
	CString		strHeader;
	if(nWinVer!=WIN2K)
		strHeader  = _T("<?xml version=\"1.0\" encoding=\"gbk\" ?>");
	strHeader += lpXMLStr;
	
	CComBSTR	bstrtext = strHeader;
	VARIANT_BOOL vbResult;
	if (FAILED(m_pXmlDocument->loadXML(bstrtext, &vbResult)) || !vbResult)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CXmlManager::LoadFromString( const char *szBuf, const size_t len )
{
	if(!szBuf)
		return FALSE;

	m_pXmlDocument = NULL;
	if(FAILED(m_pXmlDocument.CoCreateInstance(CLSID_DOMDocument))
		|| !m_pXmlDocument
		|| FAILED(m_pXmlDocument->put_async(VARIANT_FALSE)) )
	{
		return FALSE;
	}
	
	const char *pContent = SkipXmlHeaderForWin2K( szBuf );
	CComBSTR bstrtext = pContent;
	VARIANT_BOOL vbResult;
	if (FAILED(m_pXmlDocument->loadXML(bstrtext, &vbResult)) || !vbResult)
	{
		return FALSE;
	}
	return TRUE;
}


void CXmlManager::ReadXmlNodeString( IXMLDOMNode *lpNode, CString& strValue )
{
	ATLASSERT(lpNode);
	if ( !lpNode ) return;
	CComBSTR bstrText;
	lpNode->get_text(&bstrText);
	strValue =  bstrText;	
}

BOOL CXmlManager::ReadSubXmlNodeString( IXMLDOMNode *lpNode, LPCTSTR lpszName, CString& strValue, LPCTSTR lpszDefault )
{
	if ( !lpNode ) return FALSE;
	CComBSTR bstrNode = lpszName;
	CComPtr<IXMLDOMNode> lpSubNode; 
	if (SUCCEEDED(lpNode->selectSingleNode(bstrNode, &lpSubNode)) && lpSubNode)
	{
		ReadXmlNodeString(lpSubNode, strValue);
		return TRUE;
	}

	if(lpszDefault)
		strValue = lpszDefault;
	return FALSE;
}

BOOL CXmlManager::ReadSubXmlNodeInt( IXMLDOMNode *lpNode, LPCTSTR lpszName, int& nValue, int nDefault ) 
{
	if ( !lpNode ) return FALSE;
	CComBSTR bstrNode = lpszName;
	CComPtr<IXMLDOMNode> lpSubNode; 
	if (SUCCEEDED(lpNode->selectSingleNode(bstrNode, &lpSubNode)) && lpSubNode)
	{
		CString strValue;
		ReadXmlNodeString(lpSubNode, strValue);
		nValue = _ttoi(strValue);
		return TRUE;
	}
	nValue = nDefault;
	return FALSE;
}

BOOL CXmlManager::ReadSubXmlNodeKBID( IXMLDOMNode *lpNode, LPCTSTR lpszName, int& nValue, int nDefault )
{
	CString strValue;
	if( ReadSubXmlNodeString(lpNode, lpszName, strValue) )
	{
		nValue = ParseKBString( strValue );
		return TRUE;
	}
	nValue = nDefault;
	return FALSE;
}

BOOL CXmlManager::ReadXmlNodeAttribString( IXMLDOMNamedNodeMap* lpNodeMap, LPCTSTR lpszName, CString& strValue, LPCTSTR lpszDefault ) 
{
	CComBSTR bstrName = lpszName;
	CComPtr<IXMLDOMNode> lpNodeAttrib;
	if( SUCCEEDED(lpNodeMap->getNamedItem( bstrName, &lpNodeAttrib )) && lpNodeAttrib )
	{
		ReadXmlNodeString(lpNodeAttrib, strValue);
		return TRUE;
	}
	if(lpszDefault)
		strValue = lpszDefault;
	return FALSE;
}

BOOL CXmlManager::ReadXmlNodeAttribString( IXMLDOMNode* lpNode, LPCTSTR lpszName, CString & strValue, LPCTSTR lpszDefa /*= NULL*/ )
{
	if ( lpszDefa )
		strValue = lpszDefa;

	if ( lpNode == NULL )
		return FALSE;

	CComPtr<IXMLDOMElement>	pEle;
	if ( !SUCCEEDED(lpNode->QueryInterface(IID_IXMLDOMElement, (VOID**)&pEle)) || !pEle )
		return FALSE;

	CComBSTR	bstrAttribName(lpszName);
	CComVariant	vAttribValue;
	
	if ( !SUCCEEDED(pEle->getAttribute(bstrAttribName, &vAttribValue)) )
		return FALSE;

	strValue = vAttribValue;

	return !strValue.IsEmpty();
}

BOOL CXmlManager::ReadXmlNodeAttribInt( IXMLDOMNamedNodeMap* lpNodeMap, LPCTSTR lpszName, int& nValue, int nDefault ) 
{
	CComBSTR bstrName = lpszName;
	CComPtr<IXMLDOMNode> lpNodeAttrib;
	if( SUCCEEDED(lpNodeMap->getNamedItem( bstrName, &lpNodeAttrib )) && lpNodeAttrib )
	{
		CString strValue;
		ReadXmlNodeString(lpNodeAttrib, strValue);
		nValue = _ttoi( strValue );
		return TRUE;
	}
	nValue = nDefault;
	return FALSE;
}

BOOL CXmlManager::selectSingleNode( LPCTSTR queryString, IXMLDOMNode **resultNode )
{
	CComBSTR bstrNodeName = queryString;
	return SUCCEEDED( m_pXmlDocument->selectSingleNode(bstrNodeName, resultNode) ) && resultNode;
}

BOOL CXmlManager::selectSingleNode( IXMLDOMNode *lpNode, LPCTSTR queryString, IXMLDOMNode **resultNode )
{
	if(!lpNode)
		return FALSE;
	CComBSTR bstrNodeName = queryString;
	return SUCCEEDED( lpNode->selectSingleNode(bstrNodeName, resultNode) ) && resultNode;
}

BOOL CXmlManager::SaveToFile( LPCTSTR lpFileName )
{
	if ( m_pXmlDocument )
	{
		CComVariant	dstPath = lpFileName;
		return SUCCEEDED(m_pXmlDocument->save(dstPath));
	}
	return FALSE;
}

BOOL CXmlManager::WriteNodeAttribInt( IXMLDOMNode* lpNode, LPCTSTR lpszAttrib, INT iValue )
{
	CComPtr<IXMLDOMElement>		pNodeEle;
	CComPtr<IXMLDOMAttribute>	pAttribute = NULL;
	CComBSTR					bstrAttrib(lpszAttrib);
	CComVariant					varValue(iValue);

	lpNode->QueryInterface(IID_IXMLDOMElement, (VOID**)&pNodeEle);

	if ( !pNodeEle )
		return FALSE;

	return SUCCEEDED( pNodeEle->setAttribute(bstrAttrib,varValue) );
}

long CXmlManager::GetSubNodeNumber( IXMLDOMNode* lpNode, LPCTSTR lpNodesName )
{
	CComPtr<IXMLDOMNodeList>	lpItemNewList;
	CComBSTR					bstrName(lpNodesName);
	long						iLength = 0;

	if ( !SUCCEEDED(lpNode->selectNodes( bstrName, &lpItemNewList) ) )
		return iLength;

	lpItemNewList->get_length(&iLength);
	return iLength;
}
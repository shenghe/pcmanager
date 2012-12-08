#pragma once

class CLibLoader;

class CXmlManager{	
	friend class CLibLoader;

protected:
	BOOL LoadFromFile( LPCTSTR szfilename );
	BOOL LoadFromString( const char *szBuf, const size_t len );

public:
	BOOL LoadFromXMLString( LPCTSTR lpXMLStr );
	BOOL selectSingleNode( LPCTSTR queryString, IXMLDOMNode **resultNode);
	BOOL selectSingleNode( IXMLDOMNode *lpNode, LPCTSTR queryString, IXMLDOMNode **resultNode);
	
	template<class Function>
	BOOL loopSubNode( IXMLDOMNode *lpNode, LPCTSTR queryString, Function &func)
	{
		if(!lpNode)
			return FALSE;

		CComBSTR bstrNodeName = queryString;
		CComPtr<IXMLDOMNodeList> lpNodes;
		if (SUCCEEDED(lpNode->selectNodes(bstrNodeName, &lpNodes)) && lpNodes)
		{
			long nListLength = 0;
			lpNodes->get_length(&nListLength);
			for (int i = 0; i < nListLength; i++)
			{
				CComPtr<IXMLDOMNode> lpSubNode;
				lpNodes->get_item(i, &lpSubNode);
				if (lpSubNode)
				{
					if( !func(*this, lpSubNode) )
						break;
				}
			}
		}
		return TRUE;
	}
	
	void ReadXmlNodeString( IXMLDOMNode *lpNode, CString& strValue );
	
	BOOL ReadSubXmlNodeString( IXMLDOMNode *lpNode, LPCTSTR lpszName, CString& strValue, LPCTSTR lpszDefault=NULL );
	BOOL ReadSubXmlNodeInt( IXMLDOMNode *lpNode, LPCTSTR lpszName, int& nValue, int nDefault );
	BOOL ReadSubXmlNodeKBID( IXMLDOMNode *lpNode, LPCTSTR lpszName, int& nValue, int nDefault=0 );

	BOOL ReadXmlNodeAttribString( IXMLDOMNamedNodeMap* lpNodeMap, LPCTSTR lpszName, CString& strValue, LPCTSTR lpszDefault=NULL );
	BOOL ReadXmlNodeAttribInt( IXMLDOMNamedNodeMap* lpNodeMap, LPCTSTR lpszName, int& nValue, int nDefault );
	
	BOOL WriteNodeAttribInt( IXMLDOMNode* lpNode, LPCTSTR lpszAttrib, INT iValue);

	BOOL SaveToFile(LPCTSTR lpFileName);
	long GetSubNodeNumber( IXMLDOMNode* lpNode, LPCTSTR lpNames);	

	CComPtr<IXMLDOMDocument>&	GetDoc(){return m_pXmlDocument;}

	static BOOL ReadXmlNodeAttribString( IXMLDOMNode* lpNode, LPCTSTR lpszName, CString & strValue, LPCTSTR lpszDefa = NULL);
protected:
	CComPtr<IXMLDOMDocument> m_pXmlDocument;
};


template<class T, class TArg>
class XmlLoopCallback
{
public:
	explicit XmlLoopCallback(T *p, bool(T::*pf)(CXmlManager&, IXMLDOMNode*, TArg&), TArg &refArg )
		: m_pObj(p), m_pf(pf), m_refArg(refArg)
	{}

	bool operator()(CXmlManager &xml, IXMLDOMNode *lpNode) 
	{
		return (m_pObj->*m_pf)(xml, lpNode, m_refArg);
	}

protected:
	T *m_pObj;
	bool(T::*m_pf)(CXmlManager&, IXMLDOMNode*, TArg&);
	TArg &m_refArg;
};

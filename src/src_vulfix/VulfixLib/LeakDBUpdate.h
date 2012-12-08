#pragma once

struct SYSTEM_XML_UP_PARAM 
{
	BOOL bUpdateHost;
	CString strNewHost;
	CSimpleArray<CString>	listDelExp;
	CSimpleArray<CString>	listAddExp;
	CSimpleArray<CString>	listDelReplace;
	CSimpleArray<CString>	listAddReplace;
	CSimpleArray<CString>	listDelUPS;
	CSimpleArray<CString>	listAddUPS;
};

struct OFFICE_XML_UP_PARAM
{
	BOOL bUpdateHost;
	CString strNewHost;
	CSimpleArray<CString>	listDelOffice;
	CSimpleArray<CString>	listAddOffice;
	CSimpleArray<CString>	listDelItem;
	CSimpleArray<CString>	listAddItem;
};

class CXmlManager;
class CLeakDBUpdate
{
public:
	CLeakDBUpdate(void);
	~CLeakDBUpdate(void);

	HRESULT Combine( LPCTSTR lpwszDifflib );

	HRESULT CombineFile( CString strDBPath , LPCTSTR lpwszDifflib);
	BOOL UpdateSoftXML(LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName);
	BOOL UpdateSystemXML(LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName);
	BOOL UpdateOfficeXML(LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName);

protected:
	BOOL CombineSoftXML( LPCTSTR lpszBaseFileName, LPCTSTR lpFileName, CSimpleArray<INT>& idDelList, CSimpleArray<CString> & strAddList);
	BOOL CombineSystemXML( LPCTSTR lpszBaseFileName, LPCTSTR lpFileName, SYSTEM_XML_UP_PARAM & combin);
	BOOL CombineOfficeXML( LPCTSTR lpszBaseFileName, LPCTSTR lpFileName, OFFICE_XML_UP_PARAM & combin);

	long DeleteSubNodeByTextArray( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strArray);
	long DeleteSubNodeByAttribArray( IXMLDOMNode* pNode, LPCTSTR lpName, LPCTSTR lpAttrib, CSimpleArray<CString> & strArray);
	long DeleteSubNodeBySubNodeTextArray( IXMLDOMNode* pNode, LPCTSTR lpName, LPCTSTR lpSubName, CSimpleArray<CString> & strArray);

	long AddSubNodeByXMLStr( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strXMLArray);
protected:
	BOOL GetStringValueList( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strArray );
	BOOL GetNodeXMLStrList( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strArray );
	BOOL _SaveXml( CXmlManager &xml, LPCTSTR szUpdatePkgFile, LPCTSTR szPath );
};

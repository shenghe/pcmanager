#include "StdAfx.h"
#include "LeakDBUpdate.h"
#include "XmlManager.h"
#include <vulfix\BeikeUtils.h>
#include <Vulfix\IVulFix.h>
#include "HotfixUtils.h"
#include "LibLoader.h"
#include "EnvUtils.h"
#include <comutil.h>
#include <libheader/libheader.h>
#pragma comment(lib, "comsuppw.lib")

CLeakDBUpdate::CLeakDBUpdate(void)
{
}

CLeakDBUpdate::~CLeakDBUpdate(void)
{
}

// 
// soft_2010.3.2.1.dat
// soft_2010.3.2.2_2010.3.2.4_inc.dat
DWORD LibTypeFromName(LPCTSTR szFilename)
{
	static struct {
		LPCTSTR szprefix;
		DWORD dwType;
	} magics[] = {
		{ _T("system_"),	enumLibTypeVulFixSystem },
		{_T("system64_"),	enumLibTypeVulFixSystem64},
		{_T("office_"),		enumLibTypeVulFixOffice},
		{_T("office64_"),	enumLibTypeVulFixOffice64},
		{_T("soft_"),		enumLibTypeVulFixSoft},
	};
	
	CString str = PathFindFileName(szFilename);
	str.MakeLower();
	for(size_t i=0; i<sizeof(magics)/sizeof(magics[0]); ++i )
	{
		if(_tcsnicmp(str, magics[i].szprefix, _tcslen(magics[i].szprefix)) == 0)
		{
			return magics[i].dwType;
		}
	}
	return 0;
}

HRESULT CLeakDBUpdate::Combine( LPCTSTR lpwszDifflib )
{
	// 根据文件名来判断lib 类型 
	//DWORD dwIncLibType= UnMarkdatUpdate(upinfo.dwType);
	DWORD dwIncLibType = LibTypeFromName( lpwszDifflib );
	
	VulType vtype = VTYPE_UNKOWN;
	BOOL bX64 = FALSE;

	switch( dwIncLibType )
	{
	case enumLibTypeVulFixSystem:
		{
			vtype = VTYPE_WINDOWS;
			bX64 = FALSE;
		}
		break; 
	case enumLibTypeVulFixSystem64:
		{
			vtype = VTYPE_WINDOWS;
			bX64 = TRUE;
		}
		break;
	case enumLibTypeVulFixOffice:
		{
			vtype = VTYPE_OFFICE;
			bX64 = FALSE;
		}
		break;
	case enumLibTypeVulFixSoft:
		{
			vtype = VTYPE_SOFTLEAK;
			bX64 = FALSE;
		}
		break;
	}
	
	if( VTYPE_UNKOWN==vtype )
		return E_UNEXPECTED;
	
	//
	CString strDBPath;
	GetXmlDBFileName( vtype, strDBPath, bX64 );
	if( strDBPath.IsEmpty() )
		return E_UNEXPECTED;
	
	return CombineFile(strDBPath, lpwszDifflib);
}

HRESULT CLeakDBUpdate::CombineFile(CString strDBPath , LPCTSTR lpwszDifflib)
{
	FixLocale();
	T_ComInit __init__com__;
	BkDatLibHeader upinfo = {0};
	CLibLoader::GetFileInfo(lpwszDifflib, upinfo);	

	if( !IsUpdateDat(upinfo.dwType) )
		return E_FAIL;
	
	BkDatLibHeader headerBase = {0};
	if( !CLibLoader::GetFileInfo(strDBPath, headerBase) )
		return E_UNEXPECTED;

	if(upinfo.llUpdateForVer.QuadPart!=headerBase.llVersion.QuadPart)
		return E_UNEXPECTED;
	
	DWORD dwIncLibType = LibTypeFromName(lpwszDifflib);
	VulType vtype = VTYPE_UNKOWN;
	// 只升级64 位的库
	switch( dwIncLibType )
	{
	case enumLibTypeVulFixSystem:
	case enumLibTypeVulFixSystem64:
		vtype = VTYPE_WINDOWS;
		break;
	case enumLibTypeVulFixOffice:
		vtype = VTYPE_OFFICE;
		break;
	case enumLibTypeVulFixSoft:
		vtype = VTYPE_SOFTLEAK;
		break;
	}
	ATLASSERT( dwIncLibType==headerBase.dwType);

	
	BOOL bRet = FALSE;
	switch(vtype)
	{
	case VTYPE_WINDOWS:
		bRet = UpdateSystemXML( strDBPath, lpwszDifflib );
		break;
	case VTYPE_OFFICE:
		bRet = UpdateOfficeXML( strDBPath, lpwszDifflib );
		break;
	case VTYPE_SOFTLEAK:
		bRet = UpdateSoftXML( strDBPath, lpwszDifflib );
		break;
	default:
		ATLASSERT(FALSE);
	}
	return bRet ? S_OK : E_FAIL;
}

BOOL CLeakDBUpdate::UpdateSoftXML( LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName )
{
	CXmlManager	xml;
	if ( !CLibLoader::LoadFile(lpIncFileName, xml) )
		return FALSE;
	
	CSimpleArray<CString>	strAddList;
	CSimpleArray<INT>		idDelList;
	CComPtr<IXMLDOMNode>	lpXmlRoot;
	CComPtr<IXMLDOMNode>	lpRoot;

	if ( !xml.selectSingleNode(_T("upcontent"), &lpXmlRoot) || !lpXmlRoot )
		return FALSE;
	
	if ( !xml.selectSingleNode(lpXmlRoot, _T("Updates"), &lpRoot) || !lpRoot)
		return FALSE;
	
	CString	strDelString;
	if ( xml.ReadSubXmlNodeString(lpRoot, _T("del"), strDelString) && !strDelString.IsEmpty() )
	{
		CSimpleArray<CString>	strVet;
		Split(strDelString,strVet, _T(','));

		for ( INT i = 0; i < strVet.GetSize(); i++)
		{
			INT	iDel = 0;
			iDel = _ttoi( strVet[i] );
			if (iDel != 0 && (idDelList.Find(iDel)==-1) )
				idDelList.Add(iDel);
		}
	}
	
	CComPtr<IXMLDOMNode>	lpAddNodes;
	if ( xml.selectSingleNode(lpRoot,_T("add"),&lpAddNodes) && lpAddNodes)
	{
		CComPtr<IXMLDOMNodeList>	lpAddList;
		if ( SUCCEEDED(lpAddNodes->selectNodes(_T("Item"), &lpAddList)) && lpAddList )
		{
			long nListLength = 0;
			lpAddList->get_length(&nListLength);
			for (int i = 0; i < nListLength; i++)
			{
				CComPtr<IXMLDOMNode> lpSubNode;
				lpAddList->get_item(i, &lpSubNode);
				if (lpSubNode)
				{
					CComBSTR	bstrValue;
					CString		strAdd;
					lpSubNode->get_xml(&bstrValue);
					strAdd = bstrValue;
					if ( !strAdd.IsEmpty() )
						strAddList.Add(strAdd);
				}
			}
		}
	}

	return CombineSoftXML(lpszBaseFileName, lpIncFileName, idDelList, strAddList);
}

BOOL CLeakDBUpdate::CombineSoftXML( LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName, CSimpleArray<INT>& idDelList, CSimpleArray<CString> & strAddList )
{
	CXmlManager		xml;
	if( !CLibLoader::LoadFile(lpszBaseFileName, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode>	lpNodeHotFix;
	if ( !xml.selectSingleNode( _T("Hotfix"), &lpNodeHotFix) || !lpNodeHotFix )
		return FALSE;
	
	CComPtr<IXMLDOMNode>	lpNodeUpdates;
	if ( !xml.selectSingleNode(lpNodeHotFix, _T("Updates"), &lpNodeUpdates) || !lpNodeUpdates )
		return FALSE;

	long	nTotalNumber = xml.GetSubNodeNumber( lpNodeUpdates, _T("Item") );
	CComPtr<IXMLDOMNodeList>	lpItemList;
	if ( idDelList.GetSize() > 0 && SUCCEEDED(lpNodeUpdates->selectNodes( L"Item", &lpItemList)) && lpItemList )
	{
		long nListLength = 0;
		lpItemList->get_length(&nListLength);
		for (int i = 0; i < nListLength; i++)
		{
			CComPtr<IXMLDOMNode> lpSubNode;
			lpItemList->get_item(i, &lpSubNode);
			if (lpSubNode)
			{
				INT	nItemId = 0;
				if ( xml.ReadSubXmlNodeInt(lpSubNode,_T("VID"),nItemId,0) && nItemId != 0)
				{
					if ( idDelList.Find(nItemId) != -1 )
					{
						lpNodeUpdates->removeChild(lpSubNode,NULL);
						nTotalNumber--;
					}
				}
			}
		}
	}

	CComPtr<IXMLDOMNode> lpFirstNode;
	lpNodeUpdates->get_firstChild( &lpFirstNode );
	for ( int i = 0; i < strAddList.GetSize(); i++ )
	{
		CXmlManager	xmlNew;
		if ( !xmlNew.LoadFromXMLString(strAddList[i]) )
			continue;

		CComPtr<IXMLDOMNode>	lpRootNode;
		if ( xmlNew.selectSingleNode( _T("Item"), &lpRootNode) && lpRootNode )
		{
			lpNodeUpdates->appendChild(lpRootNode, NULL);
			nTotalNumber++;
		}
	}
	{
		long nChildNum = 0;
		CComPtr<IXMLDOMNodeList> lpNodes;
		if( SUCCEEDED(lpNodeUpdates->get_childNodes( &lpNodes )) && lpNodes )
		{
			lpNodes->get_length( & nChildNum );
		}
		xml.WriteNodeAttribInt( lpNodeUpdates, _T("num"), nChildNum );
	}
	
	return _SaveXml(xml, lpIncFileName, lpszBaseFileName);
}

BOOL CLeakDBUpdate::UpdateOfficeXML( LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName )
{
	CXmlManager	xml;
	if ( !CLibLoader::LoadFile(lpIncFileName, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode>	nodeRoot;
	if ( !xml.selectSingleNode( _T("upcontent"), &nodeRoot) || !nodeRoot )
		return FALSE;
	
	OFFICE_XML_UP_PARAM		comb;
	comb.bUpdateHost = xml.ReadSubXmlNodeString(nodeRoot, _T("ServerInfo/add/MirrorHost"), comb.strNewHost);
	GetStringValueList(nodeRoot, _T("Offices/del/Office"), comb.listDelOffice );
	GetNodeXMLStrList (nodeRoot, _T("Offices/add/Office"), comb.listAddOffice );
	GetStringValueList(nodeRoot, _T("Updates/del/KBID"), comb.listDelItem);
	GetNodeXMLStrList(nodeRoot, _T("Updates/add/Item"), comb.listAddItem);

	return CombineOfficeXML(lpszBaseFileName, lpIncFileName, comb);
}

BOOL CLeakDBUpdate::UpdateSystemXML( LPCTSTR lpszBaseFileName, LPCTSTR lpIncFileName )
{
	CXmlManager	xml;
	if ( !CLibLoader::LoadFile(lpIncFileName, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode>	nodeRoot;
	if ( !xml.selectSingleNode( _T("upcontent"), &nodeRoot) || !nodeRoot )
		return FALSE;
	
	SYSTEM_XML_UP_PARAM		comb;
	comb.bUpdateHost = xml.ReadSubXmlNodeString(nodeRoot, _T("ServerInfo/add/MirrorHost"), comb.strNewHost);
	GetStringValueList(nodeRoot, _T("Expired/del/KBID"), comb.listDelExp );
	GetNodeXMLStrList (nodeRoot, _T("Expired/add/KBID"), comb.listAddExp );
	GetStringValueList(nodeRoot, _T("ReplacedUPs/del/ID"), comb.listDelReplace);
	GetNodeXMLStrList (nodeRoot, _T("ReplacedUPs/add/ReplacedUP"), comb.listAddReplace );
	GetStringValueList(nodeRoot, _T("Updates/del/KBID"), comb.listDelUPS);
	GetNodeXMLStrList(nodeRoot, _T("Updates/add/Item"), comb.listAddUPS);

	return CombineSystemXML(lpszBaseFileName, lpIncFileName, comb);
}

BOOL CLeakDBUpdate::GetStringValueList( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strArray )
{
	if ( pNode == NULL )
		return FALSE;

	CComBSTR					bstrName(lpName);
	CComPtr<IXMLDOMNodeList>	nodeList;
	if ( !SUCCEEDED(pNode->selectNodes( bstrName, &nodeList)) )
		return FALSE;

	long	iLength = 0;
	nodeList->get_length(&iLength);
	for ( long i = 0; i < iLength; i++ )
	{
		CComPtr<IXMLDOMNode>	pNode;
		CComBSTR				bstrValue;
		CString					strValue;
		nodeList->get_item(i, &pNode);

		if ( pNode )
		{
			pNode->get_text(&bstrValue);
			strValue = bstrValue;
			if ( !strValue.IsEmpty() )
				strArray.Add(strValue);
		}
	}

	return (strArray.GetSize()>0);
}

BOOL CLeakDBUpdate::GetNodeXMLStrList( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strArray )
{
	if ( pNode == NULL )
		return FALSE;

	CComBSTR					bstrName(lpName);
	CComPtr<IXMLDOMNodeList>	nodeList;
	if ( !SUCCEEDED(pNode->selectNodes( bstrName, &nodeList)) )
		return FALSE;

	long	iLength = 0;
	nodeList->get_length(&iLength);
	for ( long i = 0; i < iLength; i++ )
	{
		CComPtr<IXMLDOMNode>	pNode;
		CComBSTR				bstrValue;
		CString					strValue;
		nodeList->get_item(i, &pNode);

		if ( pNode )
		{
			pNode->get_xml(&bstrValue);
			strValue = bstrValue;
			if ( !strValue.IsEmpty() )
			{
				strValue += _T("\r\n");
				strArray.Add(strValue);
			}
		}
	}

	return (strArray.GetSize()>0);
}

BOOL CLeakDBUpdate::CombineOfficeXML( LPCTSTR lpszBaseFileName, LPCTSTR lpFileName, OFFICE_XML_UP_PARAM & combin )
{
	CXmlManager	xml;
	if ( !CLibLoader::LoadFile(lpszBaseFileName, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode>	pRootNode;
	if ( !xml.selectSingleNode(_T("Hotfix"),&pRootNode) || !pRootNode )
		return FALSE;

	
	if( combin.bUpdateHost )
	{
		CComPtr<IXMLDOMNode> pNodeMirrorHost;
		if ( xml.selectSingleNode(pRootNode, _T("ServerInfo/MirrorHost"),&pNodeMirrorHost) && pNodeMirrorHost )
		{
			// node 已经存在了 
			CComBSTR bstrText = combin.strNewHost;
			pNodeMirrorHost->put_text(bstrText);
		}
	}
	
	if ( combin.listDelOffice.GetSize() > 0 || combin.listAddOffice.GetSize() > 0 )
	{	
		CComPtr<IXMLDOMNode>	pOfSNode;
		if ( xml.selectSingleNode(pRootNode,_T("Offices"),&pOfSNode) && pOfSNode )
		{
			long	nExpNum = xml.GetSubNodeNumber( pOfSNode, _T("Office") );
			if ( nExpNum > 0 )
			{
				if ( combin.listDelOffice.GetSize() > 0 )
					nExpNum -= DeleteSubNodeByAttribArray(pOfSNode, _T("Office"), _T("name"), combin.listDelOffice );

				if ( combin.listAddOffice.GetSize() > 0 )
					nExpNum += AddSubNodeByXMLStr(pOfSNode, _T("Office"), combin.listAddOffice );

				xml.WriteNodeAttribInt( pOfSNode, _T("num"), nExpNum);
			}
		}
	}
	
	if ( combin.listDelItem.GetSize() > 0 || combin.listAddItem.GetSize() > 0 )
	{	
		CComPtr<IXMLDOMNode>	pUPSNode;
		if ( xml.selectSingleNode(pRootNode,_T("Updates"),&pUPSNode) && pUPSNode )
		{
			long	nUPSNum = xml.GetSubNodeNumber( pUPSNode, _T("Item") );
			if ( nUPSNum > 0 )
			{
				if ( combin.listDelItem.GetSize() > 0 )
					nUPSNum -= DeleteSubNodeBySubNodeTextArray(pUPSNode, _T("Item"), _T("KBID"), combin.listDelItem );

				if ( combin.listAddItem.GetSize() > 0 )
					nUPSNum += AddSubNodeByXMLStr(pUPSNode, _T("Item"), combin.listAddItem );
				
				CComPtr<IXMLDOMNodeList> lpNodes;
				if( SUCCEEDED(pUPSNode->get_childNodes( &lpNodes )) && lpNodes )
				{
					lpNodes->get_length( & nUPSNum );
				}
				xml.WriteNodeAttribInt( pUPSNode, _T("num"), nUPSNum);
			}
		}
	}
	
	return _SaveXml(xml, lpFileName, lpszBaseFileName);
}

BOOL CLeakDBUpdate::CombineSystemXML( LPCTSTR lpszBaseFileName, LPCTSTR lpFileName, SYSTEM_XML_UP_PARAM & combin )
{
	CXmlManager	xml;
	if ( !CLibLoader::LoadFile(lpszBaseFileName, xml) )
		return FALSE;

	CComPtr<IXMLDOMNode>	pRootNode;
	if ( !xml.selectSingleNode(_T("Hotfix"),&pRootNode) || !pRootNode )
		return FALSE;
	
	if( combin.bUpdateHost )
	{
		CComPtr<IXMLDOMNode> pNodeMirrorHost;
		if ( xml.selectSingleNode(pRootNode, _T("ServerInfo/MirrorHost"),&pNodeMirrorHost) && pNodeMirrorHost )
		{
			// node 已经存在了 
			CComBSTR bstrText = combin.strNewHost;
			pNodeMirrorHost->put_text(bstrText);
		}
	}
	
	if ( combin.listDelExp.GetSize() > 0 || combin.listAddExp.GetSize() > 0 )
	{	
		CComPtr<IXMLDOMNode>	pExpNode;
		if ( xml.selectSingleNode(pRootNode,_T("Expired"),&pExpNode) && pExpNode )
		{
			long	nExpNum = xml.GetSubNodeNumber( pExpNode, _T("KBID") );
			if ( nExpNum > 0 )
			{
				if ( combin.listDelExp.GetSize() > 0 )
					nExpNum -= DeleteSubNodeByTextArray(pExpNode, _T("KBID"), combin.listDelExp );

				if ( combin.listAddExp.GetSize() > 0 )
					nExpNum += AddSubNodeByXMLStr(pExpNode, _T("KBID"), combin.listAddExp );

				CComPtr<IXMLDOMNodeList> lpNodes;
				if( SUCCEEDED(pExpNode->get_childNodes( &lpNodes )) && lpNodes )
				{
					lpNodes->get_length( &nExpNum );
				}
				xml.WriteNodeAttribInt( pExpNode, _T("num"), nExpNum);
			}
		}
	}

	if ( combin.listDelReplace.GetSize() > 0 || combin.listAddReplace.GetSize() > 0 )
	{	
		CComPtr<IXMLDOMNode>	pRepsNode;
		if ( xml.selectSingleNode(pRootNode,_T("ReplacedUPs"),&pRepsNode) && pRepsNode )
		{
			if ( combin.listDelReplace.GetSize() > 0 )
				DeleteSubNodeByAttribArray(pRepsNode, _T("ReplacedUP"), _T("id"), combin.listDelReplace );

			if ( combin.listAddReplace.GetSize() > 0 )
				AddSubNodeByXMLStr(pRepsNode, _T("ReplacedUP"), combin.listAddReplace );
		}
	}

	if ( combin.listDelUPS.GetSize() > 0 || combin.listAddUPS.GetSize() > 0 )
	{	
		CComPtr<IXMLDOMNode>	pUPSNode;
		if ( xml.selectSingleNode(pRootNode,_T("Updates"),&pUPSNode) && pUPSNode )
		{
			long	nUPSNum = xml.GetSubNodeNumber( pUPSNode, _T("Item") );
			if ( nUPSNum > 0 )
			{
				if ( combin.listDelUPS.GetSize() > 0 )
					nUPSNum -= DeleteSubNodeBySubNodeTextArray(pUPSNode, _T("Item"), _T("KBID"), combin.listDelUPS );

				if ( combin.listAddUPS.GetSize() > 0 )
					nUPSNum += AddSubNodeByXMLStr(pUPSNode, _T("Item"), combin.listAddUPS );

				CComPtr<IXMLDOMNodeList> lpNodes;
				if( SUCCEEDED(pUPSNode->get_childNodes( &lpNodes )) && lpNodes )
				{
					lpNodes->get_length( &nUPSNum );
				}
				xml.WriteNodeAttribInt( pUPSNode, _T("num"), nUPSNum);
			}
		}
	}

	return _SaveXml(xml, lpFileName, lpszBaseFileName);
}

long CLeakDBUpdate::DeleteSubNodeByTextArray( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strArray )
{
	if ( pNode == NULL )
		return 0;

	CComPtr<IXMLDOMNodeList>	pSubNodeList;
	CComBSTR					bstrName(lpName);
	if ( !SUCCEEDED( pNode->selectNodes(bstrName,&pSubNodeList) ) || !pSubNodeList )
		return 0;

	long	nCount = 0;
	long	iLength = 0;
	pSubNodeList->get_length(&iLength);
	for ( long i = 0; i < iLength; i++)
	{
		CComPtr<IXMLDOMNode>	node;
		pSubNodeList->get_item(i, &node);
		if ( node )
		{
			CString		strValue;
			CComBSTR	bstrValue;
			node->get_text(&bstrValue);
			strValue = bstrValue;
			if ( !strValue.IsEmpty() )
			{
				for ( INT j = 0; j < strArray.GetSize(); j++)
				{
					if ( strValue.CompareNoCase(strArray[j]) == 0 )
					{
						pNode->removeChild(node,NULL);
						nCount++;
						break;
					}
				}
			}
		}
	}
	return nCount;
}

long CLeakDBUpdate::AddSubNodeByXMLStr( IXMLDOMNode* pNode, LPCTSTR lpName, CSimpleArray<CString> & strXMLArray )
{
	CComPtr<IXMLDOMNode> lpFirstNode;
	pNode->get_firstChild( &lpFirstNode );

	long	nAppendNumber = 0;
	for ( int i = 0; i < strXMLArray.GetSize(); i++ )
	{
		CXmlManager	xmlNew;
		if ( !xmlNew.LoadFromXMLString(strXMLArray[i]) )
			continue;

		CComPtr<IXMLDOMNode>	lpNewRootNode;
		if ( xmlNew.selectSingleNode( lpName, &lpNewRootNode) && lpNewRootNode )
		{
			if ( SUCCEEDED(pNode->insertBefore( lpNewRootNode, CComVariant(lpFirstNode), NULL)) )
				nAppendNumber++;
		}
	}
	return nAppendNumber;
}

long CLeakDBUpdate::DeleteSubNodeBySubNodeTextArray( IXMLDOMNode* pNode, LPCTSTR lpName, LPCTSTR lpSubName, CSimpleArray<CString> & strArray )
{
	if ( pNode == NULL )
		return 0;

	CComPtr<IXMLDOMNodeList>	pSubNodeList;
	CComBSTR					bstrName(lpName);
	if ( !SUCCEEDED( pNode->selectNodes(bstrName,&pSubNodeList) ) || !pSubNodeList )
		return 0;

	long	nCount = 0;
	long	iLength = 0;
	pSubNodeList->get_length(&iLength);
	for ( long i = 0; i < iLength; i++)
	{
		CComPtr<IXMLDOMNode>	node;
		pSubNodeList->get_item(i, &node);
		if ( node )
		{
			CComBSTR	bstrSub(lpSubName);
			CComPtr<IXMLDOMNode>	subNode;
			node->selectSingleNode( bstrSub, &subNode);

			if ( subNode )
			{
				CString		strValue;
				CComBSTR	bstrValue;

				subNode->get_text(&bstrValue);
				strValue = bstrValue;
				if ( !strValue.IsEmpty() )
				{
					for ( INT j = 0; j < strArray.GetSize(); j++)
					{
						if ( strValue.CompareNoCase(strArray[j]) == 0 )
						{
							pNode->removeChild(node,NULL);
							nCount++;
						}
					}
				}
			}
		}
	}
	return nCount;
}

long CLeakDBUpdate::DeleteSubNodeByAttribArray( IXMLDOMNode* pNode, LPCTSTR lpName, LPCTSTR lpAttrib, CSimpleArray<CString> & strArray )
{
	if ( pNode == NULL )
		return 0;

	CComPtr<IXMLDOMNodeList>	pSubNodeList;
	CComBSTR					bstrName(lpName);
	if ( !SUCCEEDED( pNode->selectNodes(bstrName,&pSubNodeList) ) || !pSubNodeList )
		return 0;

	long	nCount = 0;
	long	iLength = 0;
	pSubNodeList->get_length(&iLength);
	for ( long i = 0; i < iLength; i++)
	{
		CComPtr<IXMLDOMNode>	node;
		pSubNodeList->get_item(i, &node);
		if ( node )
		{
			CString		strValue;
			CXmlManager::ReadXmlNodeAttribString( node, lpAttrib,strValue);
	
			if ( !strValue.IsEmpty() )
			{
				for ( INT j = 0; j < strArray.GetSize(); j++)
				{
					if ( strValue.CompareNoCase(strArray[j]) == 0 )
					{
						pNode->removeChild(node,NULL);
						nCount++;
					}
				}
			}
		}
	}
	return nCount;
}

BOOL CLeakDBUpdate::_SaveXml( CXmlManager &xml, LPCTSTR szUpdatePkgFile, LPCTSTR szPath )
{
	// get_xml 的头有问题, <?xml version="1.0"?>
	// 需要手动添加 <?xml version="1.0" encoding="gbk" ?>	
	
	CStringA strXml;
	CString strTmp = szPath;
	strTmp += _T(".tmp");
	if( !xml.SaveToFile(strTmp) || !file_get_contents(strTmp, strXml) )
		return FALSE;
	
	BkDatLibHeader upinfo = {0};
	CLibLoader::GetFileInfo(szUpdatePkgFile, upinfo);
	if( !CLibLoader::SaveFile(strTmp, strXml, strXml.GetLength(), UnMarkdatUpdate(upinfo.dwType), upinfo.llVersion) )
		return FALSE;
	
	if( IsFileExist(szPath) )
		SetFileAttributes( szPath, FILE_ATTRIBUTE_NORMAL);

#ifdef _DEBUG
	CString strBakup = szPath;
	strBakup += _T(".bak");
	MoveFileEx( szPath, strBakup, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
#endif
	return MoveFileEx( strTmp, szPath, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
}

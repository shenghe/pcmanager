#include "StdAfx.h"
#include <atlfile.h>
#include <libheader/libheader.h>
#include "LibLoader.h"
#include "XmlManager.h"
#include "Utils.h"
#include "HotfixUtils.h"
#include "EnvUtils.h"
#include <bksafe/bksafeconfig.h>

// if use xml other than data
// #define USE_XML 

CString &GetXmlDBFileName( int type, CString &strPath, BOOL get64 )
{
	LPCTSTR lpszName = NULL;

	switch ( type )
	{
	case VTYPE_OFFICE:
		lpszName = _T("office");
		break;
	case VTYPE_WINDOWS:
		{
			if ( get64 )
			{
				lpszName = _T("system64");
			}
			else
			{
				lpszName = _T("system");
			}
		}
		break;
	case VTYPE_SOFTLEAK:
		lpszName = _T("soft");
		break;
	default:
		ATLASSERT( FALSE );
		break;
	}
	
	strPath = lpszName;

#ifdef USE_XML
	strPath.Append(_T(".xml"));
#else
	strPath.Append(_T(".dat"));
#endif

	CString	strDir;
	BKSafeConfig::GetProductDataPath(strDir, TRUE);
	strDir += _T("\\");
	strDir += strPath;
	strPath = strDir;

	return strPath;
}

BOOL CLibLoader::LoadLibFile2( int vtype, CXmlManager &xmlmgr )
{
	CString strPath;
	BOOL bRet = FALSE;
	switch ( vtype )
	{
	case VTYPE_WINDOWS:
		bRet = LoadFile( GetXmlDBFileName( vtype, strPath, IsWin64() ), xmlmgr );
		break;
	case VTYPE_OFFICE:
		bRet = LoadFile( GetXmlDBFileName( vtype, strPath, FALSE ), xmlmgr );
		break;
	case VTYPE_SOFTLEAK:
		bRet = LoadFile( GetXmlDBFileName( vtype, strPath, FALSE ), xmlmgr );
		break;
	default:
		ATLASSERT( FALSE );
		bRet = FALSE;
		break;
	}

	return bRet;
}

BOOL CLibLoader::LoadFile( LPCTSTR szFileName, CXmlManager &xmlmgr )
{
	if(!PathFileExists(szFileName))
		return FALSE;

	BOOL bRet = FALSE;
#ifdef USE_XML
	bRet = xmlmgr.LoadFromFile( szFileName );
#else
	CDataFileLoader	loader;
	BkDatLibHeader	header;
	if( loader.GetLibDatHeader( szFileName,header ) )
	{
		BkDatLibContent ct;
		loader.GetLibDatContent( szFileName, ct );		
		bRet = xmlmgr.LoadFromString( (const char*)ct.pBuffer, ct.nLen );
	}
#endif
	return bRet;
}

BOOL CLibLoader::LoadFileXml( LPCTSTR szFileName, CXmlManager &xmlmgr )
{
	return xmlmgr.LoadFromFile( szFileName );
}

BOOL CLibLoader::GetFileInfo( LPCTSTR szFileName, BkDatLibHeader &info )
{
	if(!PathFileExists(szFileName))
		return FALSE;

	BkDatLibHeader header = {0};
	BOOL bRet = TRUE;
#ifdef USE_XML
	info = header;
#else
	CDataFileLoader	loader;
	bRet = loader.GetLibDatHeader( szFileName, header );
	info = header;
#endif
	return bRet;	
}

BOOL CLibLoader::SaveFile( LPCTSTR szFileName, LPCSTR szBuffer, INT nLen, INT nLibType, LARGE_INTEGER nFileVersion )
{
	BOOL bRet = FALSE;
#ifdef USE_XML
	bRet = file_put_contents(szFileName, (BYTE*)szBuffer, nLen);
#else
	CDataFileLoader	loader;
	BkDatLibEncodeParam	paramx(nLibType, nFileVersion, (BYTE*)szBuffer, nLen, 1);
	bRet = loader.Save( szFileName, paramx );
#endif
	return bRet;
}

BOOL CLibLoader::ConvertXml2Lib( LPCTSTR szFilenameXml, LPCTSTR szFileNameLib, INT nFileVersion )
{
	CString strContent;
	if( file_get_contents(szFilenameXml, strContent) )
	{
		CDataFileLoader	loader;
		LARGE_INTEGER ll= {nFileVersion};
		BkDatLibEncodeParam2 param2(enumLibTypePlugine, ll, strContent, 1);
		return loader.Save( szFileNameLib, param2.GetEncodeParam());
	}
	return FALSE;
}

BOOL CLibLoader::ConvertLib2Xml( LPCTSTR szFilenameLib, LPCTSTR szFileNameXml, INT nFileVersion )
{
	BOOL bRet = FALSE;

	CDataFileLoader	loader;
	BkDatLibHeader	header;
	if( loader.GetLibDatHeader( szFilenameLib,header ) )
	{
		// TODO : verify the header param 
		
		BkDatLibContent2 ct2;
		if( loader.GetLibDatContent2( szFilenameLib, ct2 ) )
			bRet = file_put_contents( szFileNameXml, ct2.strXML);
	}
	return bRet;
}

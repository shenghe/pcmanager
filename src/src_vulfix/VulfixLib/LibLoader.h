#pragma once

CString &GetXmlDBFileName( int type, CString &strPath, BOOL get64 );

class CXmlManager;
struct BkDatLibHeader;
class CLibLoader
{
public:
	static BOOL LoadLibFile2(int vtype, CXmlManager &xmlmgr);
	static BOOL LoadFile(LPCTSTR szFileName, CXmlManager &xmlmgr);
	static BOOL LoadFileXml(LPCTSTR szFileName, CXmlManager &xmlmgr);
	static BOOL GetFileInfo(LPCTSTR szFileName, BkDatLibHeader &info);
	static BOOL SaveFile(LPCTSTR szFileName, LPCSTR szBuffer, INT nLen,  INT nLibType, LARGE_INTEGER nFileVersion);
	
	static BOOL ConvertXml2Lib(LPCTSTR szFilenameXml, LPCTSTR szFileNameLib, INT nFileVersion );
	static BOOL ConvertLib2Xml(LPCTSTR szFilenameLib, LPCTSTR szFileNameXml, INT nFileVersion );
};

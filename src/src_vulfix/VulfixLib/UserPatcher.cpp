#include "StdAfx.h"
#include "UserPatcher.h"
#include <vulfix\\BeikeUtils.h>
#include <vulfix\\IVulFix.h>
#include "libpack\\WcxModule.h"
#include "libpack\\SevenZipModule.h"
#include <algorithm>
#include "pkgupk/LibPkgUpk.h"

void _GetFileDir( const char *szpath, std::string &str );
void _GetFilename( const char *szpath, std::string &str, bool removeExt=false );

void _GetFilename( const char *szpath, std::string &str, bool removeExt )
{
	const char *p = strrchr(szpath, '\\');
	if( p )
		str = p + 1;
	else
		str = szpath;

	if( removeExt )
	{
		size_t pos = str.rfind('.');
		if( pos!=std::string::npos )
		{
			str[pos] = '\0';
		}
	}
}

void _GetFileDir( const char *szpath, std::string &str )
{
	const char *p = strrchr(szpath, '\\');
	if( p )
		str.assign( szpath, p-szpath );
	else
		str = szpath;
}

bool CUserPatcher::Initialize( LPCTSTR sz7z, LPCTSTR szwcx )
{
	return m_objUpk->Initialize(sz7z, szwcx);
}

int CUserPatcher::InstallPatch( LPCTSTR szfile, const T_UserPatchInfo &unpkinfo )
{
	USES_CONVERSION;
	if(!m_objUpk->IsValid())
		return KPATCHER_ERR_EXDLL;

	if(unpkinfo.files.GetSize()==0)
		return KPATCHER_ERR_XMLFILES;
	
	CString strLogfile;
	int code = _Patch(szfile, unpkinfo, NULL, strLogfile);
	if( code!=KPATCHER_OK )
		return code;
	
	//_FillRegInfo(unpkinfo.nKBID, unpkinfo.strName, unpkinfo.strProductKey, unpkinfo.strPatchKey, unpkinfo.strPatchValue, strLogfile);
	return KPATCHER_OK;
}


int CUserPatcher::_Patch( LPCTSTR szfile, const T_UserPatchInfo &unpkinfo, const char *, CString &strLogfile )
{
	USES_CONVERSION;
	TCHAR szTempPath[MAX_PATH];
	
#if 1 
	TCHAR szTempName[MAX_PATH];
	GetTempPath(MAX_PATH, szTempName);
	GetTempFileName(szTempName, _T("BK.UPK"), 0, szTempPath);
	DeleteFile( szTempPath );
#else
	strcpy(szTempPath, "c:\\office.unpack\\");
#endif
	
	INT err = KPATCHER_ERR_GENERAL;
	do
	{
		strings files;
		if(!m_objUpk->Extract(szfile, szTempPath, files))
		{
			err = KPATCHER_ERR_EXTRACT;
			break;
		}

		typedef std::pair<std::string, std::string> stringPair;
		typedef std::vector< stringPair >  stringPairs;
		stringPairs ps;
		for(int i=0; i<unpkinfo.files.GetSize(); ++i)
		{
			// 
			const T_UserPatchInfo::PatchFileInfo &pi = unpkinfo.files[i];
			std::string strfromfilename;

			_GetFilename( CT2CA(pi.strFilenameFrom), strfromfilename);
			for(strings::iterator it2=files.begin(); it2!=files.end(); ++it2)
			{
				std::string strfilename2;
				_GetFilename( it2->c_str(), strfilename2 );

				if(stricmp(strfromfilename.c_str(), strfilename2.c_str())==0)
				{
					std::string sto = CT2CA(pi.strFilenameTo);
					ps.push_back( stringPair(*it2, sto) );
					break;
				}
			}
		}
		if( ps.size()< unpkinfo.files.GetSize() )
		{
			err = KPATCHER_ERR_FILENOTMATCH;
			break;
		}
		
		// 2. check file is not used 
		BOOL bFileIsUsing = FALSE;
		for(unsigned int i=0; i<ps.size(); ++i)
		{
			CString strFilename = CA2CT(ps[i].second.c_str());
			if( PathFileExists(strFilename) && IsFileUsing( strFilename ) )
			{
				bFileIsUsing = TRUE;
				break;
			}
		}
		if(bFileIsUsing)
		{
			err = KPATCHER_ERR_FILEBUSY;
			break;
		}
				
		// 3. replace files
		struct T_ReplaceInfo
		{
			std::string strTo, strTmpBackup, strTmp;
			LARGE_INTEGER llfrom, llto;
			bool skipReplace;
		};
		bool hasError = false;
		std::vector<T_ReplaceInfo> tmpfiles;
		// copy all files into target path 
		for( unsigned int i=0; i<ps.size() && !hasError; ++i)
		{
			// BAKCUP in local DIR 
			std::string	strfrom, strto, strtmp, strbackup;
			strfrom = ps[i].first;
			strto = ps[i].second;
			strtmp = strto + "_TMP";
			strbackup = ps[i].second + "_TMPBK";

			T_ReplaceInfo r;
			r.strTo = strto;
			r.strTmpBackup = strbackup;
			r.strTmp = strtmp;
			if( GetFileVersion( CA2CT(strfrom.c_str()), r.llfrom) && GetFileVersion( CA2CT(strto.c_str()), r.llto) && r.llfrom.QuadPart==r.llto.QuadPart )
			{
				r.skipReplace = true;
			}
			else
			{
				CreateDirs( strto.c_str() );				

				BOOL b1 = true, b2 = true;
				if( IsFileExist( CA2CT(strto.c_str()) ) )
					b1 = MyMoveFileA( strto.c_str(), strbackup.c_str());
				b2 = MyMoveFileA(strfrom.c_str(), strtmp.c_str());
				if( !b1	|| !b2 )
				{
#ifdef _DEBUG
					DWORD dwErr = GetLastError();
					CStringA strA;
					strA.Format("MOVE FILE ERROR %d\r\n%d %s -> %s\r\n%d %s -> %s \r\n", dwErr, b1, strto.c_str(), strbackup.c_str(), b2, strfrom.c_str(), strtmp.c_str() );
					MessageBoxA(NULL, strA, NULL, MB_OK);
#endif 
					hasError = true;
				}
				r.skipReplace = false;
			}						
			tmpfiles.push_back( r );			
		}
		
		// 4. rollback or commit replace operation  		
		if( hasError )
		{
			for( unsigned int i=0; i<tmpfiles.size(); ++i)
			{
				T_ReplaceInfo &r = tmpfiles[i];
				if( r.skipReplace )
					continue;				
				MyMoveFileA( r.strTmpBackup.c_str(), r.strTo.c_str() );
			}
			err = KPATCHER_ERR_REPLACE;
		}
		else
		{
			// Assume all move operation success 
			CStringA slog;
			CStringA strProductKey = CT2CA(unpkinfo.strProductKey), strPatchKey = CT2CA(unpkinfo.strPatchKey);
			slog.AppendFormat("REG:%s\t%s\r\n", strProductKey, strPatchKey);
			for( unsigned int i=0; i<tmpfiles.size(); ++i)
			{
				T_ReplaceInfo &r = tmpfiles[i];
				std::string strbackup = r.strTo + "_bk";
				CString strVfrom, strVto;
				CStringA strVfromA, strVtoA;
				GenVersionStr( r.llfrom, strVfrom);
				GenVersionStr( r.llto, strVto);
				strVfromA = CT2CA( strVfrom );
				strVtoA = CT2CA( strVto );
				slog.AppendFormat("VER:%s\t%s\r\n", strVfromA, strVtoA);
				slog.AppendFormat("FILE:%d\t%s\t%s\r\n", r.skipReplace, strbackup.c_str(), r.strTo.c_str() );

				if( r.skipReplace )
					continue;				
				// 
				if( IsFileExist( CA2CT(r.strTmpBackup.c_str()) ) )
					MyMoveFileA(r.strTmpBackup.c_str(), strbackup.c_str());
				MyMoveFileA(r.strTmp.c_str(), r.strTo.c_str());
			}

			std::string dir;
			_GetFileDir( ps[0].second.c_str(), dir );
			char logfilename[MAX_PATH];
			sprintf(logfilename, "$NTUninstKB%d$.log", unpkinfo.nKBID);
			dir += '\\';
			dir += logfilename;

			strLogfile = CA2CT(dir.c_str());
			file_put_contents( strLogfile, (BYTE*)(LPCSTR)slog, slog.GetLength());
			err = KPATCHER_OK;
		}
	}while(FALSE);
	

#ifndef _DEBUG
	{
		TCHAR szPath[MAX_PATH] = {0};
		_tcscpy(szPath, szTempPath);
		SHFILEOPSTRUCT shfileopstruct = {0};
		shfileopstruct.wFunc = FO_DELETE;
		shfileopstruct.pFrom = szPath;   
		shfileopstruct.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;   
		SHFileOperation(&shfileopstruct); 
	}
#endif
	return err;
}

void CUserPatcher::_FillRegInfo( INT nKBID, LPCTSTR szPatchName, LPCTSTR szProductKey, LPCTSTR szPatchKey, LPCTSTR szPatchValue, LPCTSTR szLogfile )
{
	LPCTSTR _key_patch = _T("Patches");
	
	CString strProduct;
	strProduct.Format(_T("Installer\\Products\\%s\\Patches"), szProductKey);
	WriteRegString(HKEY_CLASSES_ROOT, strProduct, szPatchKey, szPatchValue);

	CSimpleArray<CString> ms;
	ReadRegMString(HKEY_CLASSES_ROOT, strProduct, _key_patch, ms);
	if( ms.Find(szPatchKey)==-1 )
	{
		ms.Add( szPatchKey );
		WriteRegMString(HKEY_CLASSES_ROOT, strProduct, _key_patch, ms);
	}

	// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Products\4080110900063D11C8EF10054038389C\Patches\CEA540E1AE6DD1D41A6E01E6EF2B271C
	CString strPatchInfo;
	strPatchInfo.Format(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\%s\\Patches\\%s"), szProductKey, szPatchKey);

	CString strMoreInfoURL;
	FormatKBWebUrl( strMoreInfoURL, nKBID );
	
	CString strDate;
	_GetDateString(strDate);

	WriteRegString(HKEY_LOCAL_MACHINE, strPatchInfo, _T("DisplayName"), szPatchName);
	WriteRegString(HKEY_LOCAL_MACHINE, strPatchInfo, _T("Installed"), strDate);
	WriteRegString(HKEY_LOCAL_MACHINE, strPatchInfo, _T("MoreInfoURL"), strMoreInfoURL);

	WriteRegDWord(HKEY_LOCAL_MACHINE, strPatchInfo, _T("LUAEnabled"),	0);
	WriteRegDWord(HKEY_LOCAL_MACHINE, strPatchInfo, _T("MSI3"),			1);
	WriteRegDWord(HKEY_LOCAL_MACHINE, strPatchInfo, _T("PatchType"),	0);
	WriteRegDWord(HKEY_LOCAL_MACHINE, strPatchInfo, _T("State"),		1);
	WriteRegDWord(HKEY_LOCAL_MACHINE, strPatchInfo, _T("Uninstallable"),0);
}

void CUserPatcher::_GetDateString( CString &strDate )
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	strDate.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
}



bool CExeUnpackPkg::IsValid() const
{
	return IsLibPkgUpkValid();
}

bool CExeUnpackPkg::Extract( LPCTSTR szfile, LPCTSTR szpath, strings &files )
{
	USES_CONVERSION;
	Files	filelist;
	if ( EE_SUCCESS == ExtractOfficePackage(szfile, szpath, filelist) )
	{
		for (int i=0; i<filelist.size(); i++)
		{
			std::string	str = CT2CA(filelist[i].GetString());
			files.push_back(str);
		}
		return true;
	}
	return false;
}

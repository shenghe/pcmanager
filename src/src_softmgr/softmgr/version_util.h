#ifndef SOFTMGR_VERSION_UTIL_H
#define SOFTMGR_VERSION_UTIL_H

#include <shobjidl.h>
#include <ShlGuid.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "path_util.h"
#pragma comment(lib,"Version.lib")
#include <Shellapi.h>
#pragma comment(lib, "shell32.lib")
#include "Wildcard.h"

const static int MAX_VALUE_LENGTH=16383;

static int GetFileVersion(CString dir,CString file,CString& loc,DWORD& dwMS,DWORD& dwLS)
{
	dwLS=dwMS=0;

	TCHAR buff[MAX_VALUE_LENGTH]={0};
	if(NULL==PathCombine(buff,dir.GetBuffer(),file.GetBuffer()))
		return 1;

	file=buff;

	PathRemoveFileSpec(buff);
	loc=buff;
	if(loc==L"\\")
		loc=L"";


	if(PathFileExists(file.GetBuffer()) == FALSE 
		|| PathIsDirectory(file.GetBuffer() ) != FALSE )
		return -1;


	DWORD dwHandle;     
	DWORD dwDataSize = ::GetFileVersionInfoSize(file.GetBuffer(), &dwHandle); 
	if ( dwDataSize == 0 ) 
	{
		dwMS=0;
		dwLS=1;
		return 0;
	}

	LPBYTE  lpVersionData; 

	lpVersionData = new BYTE[dwDataSize]; 
	if (!::GetFileVersionInfo(file.GetBuffer(), dwHandle, dwDataSize, (void**)lpVersionData) )
	{
		delete [] lpVersionData;
		return 3;
	}

	//ASSERT(lpVersionData != NULL);

	if(lpVersionData==NULL)
		return 5;

	UINT nQuerySize;
	VS_FIXEDFILEINFO* pVsffi;
	if ( ::VerQueryValue((void **)lpVersionData, _T("\\"),
		(void**)&pVsffi, &nQuerySize) )
	{
		dwMS = pVsffi->dwFileVersionMS;
		dwLS = pVsffi->dwFileVersionLS;
		delete [] lpVersionData;
		return 0;
	}

	delete [] lpVersionData;
	return 4;
}


static void ver4ws(const TCHAR* ver,DWORD& dwMS,DWORD& dwLS)
{
	TCHAR buff[MAX_VALUE_LENGTH]={0};

	wcscpy_s(buff,MAX_VALUE_LENGTH,ver);

	TCHAR* next_tok = NULL;
	TCHAR* tok=wcstok_s( buff, _T("."), &next_tok );

	WORD wVer[4] = {0};
	for ( int i = 0 ; tok ; i++ )
	{
		wVer[i]=_wtoi(tok);
		tok=wcstok_s( NULL, _T("."), &next_tok );
	}
	dwMS=(DWORD)MAKELONG( wVer[1], wVer[0] );
	dwLS=(DWORD)MAKELONG( wVer[3], wVer[2] );
}

static CString ver2ws(DWORD& dwMS,DWORD& dwLS)
{
	CString ret;
	ret.Format(L"%d.%d.%d.%d",HIWORD(dwMS),LOWORD(dwMS),HIWORD(dwLS),LOWORD(dwLS));
	return ret;
}

static CString PreKey2ws(HKEY key)
{
	if(key==HKEY_LOCAL_MACHINE)
		return L"HKLM";
	else if(HKEY_CURRENT_USER==key)
		return L"HKCU";
	else if(HKEY_CLASSES_ROOT==key)
		return L"HKCR";
	else if(HKEY_USERS)
		return L"HKU";
	return L"";
}

static CString PreKey2wsLong(HKEY key)
{
	if(key==HKEY_LOCAL_MACHINE)
		return L"HKEY_LOCAL_MACHINE";
	else if(HKEY_CURRENT_USER==key)
		return L"HKEY_CURRENT_USER";
	else if(HKEY_CLASSES_ROOT==key)
		return L"HKEY_CLASSES_ROOT";
	else if(HKEY_USERS)
		return L"HKEY_USERS";
	return L"";
}


static void Translate(CString pos,HKEY& parent,CString& reg_key)
{
	if(pos==_T("00"))
	{
		reg_key=_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall")+reg_key;
		parent=HKEY_LOCAL_MACHINE;
	}
	else if(pos==_T("10"))
	{
		reg_key=_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"+)reg_key;
		parent=HKEY_CURRENT_USER;
	}
	else if(pos==_T("05"))
	{
		parent=HKEY_CLASSES_ROOT;
	}
	else if(pos==_T("06"))
	{
		reg_key=_T("SOFTWARE")+reg_key;
		parent=HKEY_LOCAL_MACHINE;
	}
	else if(pos==_T("16"))
	{
		reg_key=_T("SOFTWARE")+reg_key;
		parent=HKEY_CURRENT_USER;
	}
	else if(pos==_T("02"))
	{
		reg_key=_T("SOFTWARE\\Microsoft")+reg_key;
		parent=HKEY_LOCAL_MACHINE;
	}
	else if(pos==_T("03"))
	{
		reg_key=_T("TypeLib")+reg_key;
		parent=HKEY_CLASSES_ROOT;
	}
	else if(pos==_T("04"))
	{
		reg_key=_T("CLSID")+reg_key;
		parent=HKEY_CLASSES_ROOT;
	}
	else
	{
		//暂定其他都是错误
		//cout<<pos<<endl;
	}
}

CString GetRegValue(CString key)
{
	HKEY hKey=NULL;
	HKEY parent=NULL;


	CString reg_key = key.Mid( 2, key.Find( _T("\\\\") ) - 2 );
	CString reg_value_name = key.Mid( 
		key.Find( _T("\\\\") ) + 2, 
		key.GetLength() );

	Translate(key.Left(2),parent,reg_key);

	LONG	ret = 0;

	ret = RegOpenKeyEx( 
		parent,
		reg_key.GetBuffer(),
		0,
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ, 
		&hKey 
		);

	TCHAR buff[MAX_VALUE_LENGTH]={0};
	TCHAR buff_file[MAX_VALUE_LENGTH]={0};

	DWORD type=REG_SZ;
	DWORD len=MAX_VALUE_LENGTH;


	if(ret!=ERROR_SUCCESS)
	{
		return _T("");
	}



	ret = RegQueryValueEx(hKey,reg_value_name.GetBuffer(),NULL,&type,(LPBYTE)buff,&len);


	RegCloseKey(hKey);

	if(ret!=ERROR_SUCCESS)
		return _T("");

	return buff;
}

int GetFileVersionFromReg(CString& ins_loc,CString& ver,CString path_key,CString path_a,CString path_b)
{
	if(path_a.IsEmpty()) return 1;

	CString path = GetRegValue(path_key);
	if(path.IsEmpty()) return 1;

	// 处理命令行模式
	if(!PathFileExistsW(path))
	{
		int argc = 0;
		LPWSTR* ppArgv = ::CommandLineToArgvW(path, &argc);
		if(ppArgv == NULL || argc == 0) return 1;

		path = ppArgv[0];

		::LocalFree(ppArgv);
		if(!::PathFileExistsW(path)) return 1;
	}

	// 正规化为目录
	LPWSTR pPath = path.GetBuffer(MAX_PATH);
	if(!::PathIsDirectoryW(pPath))
	{
		::PathRemoveFileSpecW(pPath);
	}

	::PathAddBackslashW(pPath);
	path.ReleaseBuffer();

	DWORD dwLS = 0, dwMS = 0;
	if(GetFileVersion(path, path_a, ins_loc, dwMS, dwLS) != 0) 
	{
		// 获取版本号失败时，认为已经安装
		ver = L"0.0.0.0";
		return 1;
	}
	ver = ver2ws(dwMS, dwLS);

	if(!path_b.IsEmpty())
	{
		// 不符合版本模式的则为未安装
		CWildcard wildcard(path_b, false);
		if(!wildcard.IsMatch(ver)) return 1;
	}

	return 0;
}

int GetFileVersionFromRegValue(CString& key, CString& ver)
{
	DWORD dwMajor = 0;
	DWORD dwLower = 0;

	CString _strRegValue = GetRegValue(key);
	if ( _strRegValue.IsEmpty() )
		return 1;

	ver4ws(_strRegValue.GetBuffer(),dwMajor,dwLower);

	_strRegValue.ReleaseBuffer();

	ver = ver2ws(dwMajor, dwLower);

	return (dwMajor || dwLower) ? 0 : 1;
}

int GetLnkFullPath(const TCHAR *dir,const TCHAR *file, TCHAR *szFullPath)
/*该函数用于获得快捷方式的参数，快捷方式指向的全路径
*lpszPath 快捷方式的路径
*szFullPath快捷方式所指向的文件的全路径
*/
{ 
	TCHAR lpszPath[MAX_PATH]={0};
	if(NULL == PathCombine( lpszPath,dir,file ))
	{
		return 1;
	}
	if(PathFileExists(lpszPath)==FALSE)
		return -1;

	HRESULT hres;
	hres = CoInitialize(NULL);

	IShellLink		*psl;
	IPersistFile	*ppf; 
	TCHAR			szArg[1024] = {0};
	TCHAR			szPath[1024] = {0};
	int				re = 0;
	WIN32_FIND_DATA filedata;

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, ( LPVOID* ) &psl );
	if( SUCCEEDED( hres ) )
	{  
		hres = psl->QueryInterface( IID_IPersistFile, ( void** ) &ppf );
		if( SUCCEEDED( hres ) )
		{  
			hres = ppf->Load( lpszPath, STGM_READ );
			if( SUCCEEDED( hres ) )
			{   
				//psl->GetArguments( szArg, 1024 );		   
				psl->GetPath( szPath, 1024, &filedata, SLGP_RAWPATH );
				//memcpy( szArgStr, szArg, strlen( szArg ) );
				wcscpy_s( szFullPath, 1024,szPath);
			    re = 1;

				/// 应该是这句导致了 “写link”的行为
				// ppf->Save( NULL, FALSE );
			}
			ppf->Release();
		}
		psl->Release();
	}
	
	return re;
}

int GetFileVersionFromLnk(CString& ins_loc,CString& ver,CString path_lnk,CString path_a,CString path_b)
{
	if(path_a.IsEmpty()) return 1;

	CAtlMap<CString,CString> possible_dir;

	const TCHAR* lnk_dirs[4]={ NULL };
	lnk_dirs[0] = CPathUtil::Instance()->GetShellFolder(CSIDL_PROGRAMS);
	lnk_dirs[1] = CPathUtil::Instance()->GetShellFolder(CSIDL_COMMON_PROGRAMS);
	lnk_dirs[2] = CPathUtil::Instance()->GetShellFolder(CSIDL_DESKTOP);
	//lnk_dirs[3] = CPathUtil::Instance()->GetShellFolder(CSIDL_STARTMENU);
	//lnk_dirs[4] = CPathUtil::Instance()->GetShellFolder(CSIDL_COMMON_STARTMENU);

	const TCHAR* lnks[]={path_lnk,NULL,NULL};

	CString dlnk=_T("");

	if(path_lnk.ReverseFind('\\')!=-1)
	{
		dlnk=path_lnk.Mid( path_lnk.ReverseFind('\\')+1,path_lnk.GetLength());
		lnks[1]=dlnk;
	}

	CString _strLnk;
	static TCHAR possible_buff[1024]={0};
	for (int dir_index=0;lnk_dirs[dir_index];dir_index++)
	{
		for (int lnk_index=0;lnks[lnk_index];lnk_index++)
		{
			_strLnk = lnk_dirs[dir_index];
			_strLnk += lnks[lnk_index];

			if(0 != CPathUtil::Instance()->GetLnkFullPath(_strLnk, possible_buff, 1024) )//      GetLnkFullPath(lnk_dirs[dir_index],lnks[lnk_index],possible_buff))
				continue;
			PathRemoveFileSpec(possible_buff);
			PathAddBackslash(possible_buff);
			possible_dir[possible_buff]=L"";
		}
	}

	if ( possible_dir.IsEmpty() )
		return 1;

	//判断文件在不在
	CString _strPath;
	CString _strLoc;
	CString _strFile;
	ver = L"0.0.0.0";

	BOOL exist = FALSE;
	CWildcard wildcard;
	if(!path_b.IsEmpty())
	{
		wildcard.SetPattern(path_b, false);
	}

	for(POSITION pos=possible_dir.GetStartPosition();pos;)
	{
		_strPath = possible_dir.GetNextKey(pos);
		if(::PathFileExistsW(_strPath))
		{
			// 若目录存在，则首先认为存在
			exist = TRUE;

			DWORD dwMS = 0, dwLS = 0;
			if(GetFileVersion(_strPath, path_a, ins_loc, dwMS, dwLS) == 0)
			{
				ver = ver2ws(dwMS, dwLS);

				if(!path_b.IsEmpty())
				{
					// 若与版本模式匹配，则直接返回匹配，否则不匹配
					if(!wildcard.IsMatch(ver)) return 1;
					else return 0;
				}
			}
		}
	}

	return (exist ? 0 : 1);
}






#endif
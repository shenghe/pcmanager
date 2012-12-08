#ifndef _PLAYERTRACE_H_
#define _PLAYERTRACE_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"
#include "getinstallpath.h"
#include <string>
#include "sqlite3.h"
#include "ksconversion.h"
#include <algorithm>
#include <vector>
#include <string>
extern std::vector<std::wstring> g_listProcessName;

using namespace std;
extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;
class CPPSPlayer
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanPPS()
	{
		g_fnScanFile(g_pMain,BEGINPROC(PPS_PLAYER) ,0 ,0 ,0);
		g_fnScanReg(g_pMain, BEGINPROC(PPS_PLAYER), 0, 0, 0);			
		std::wstring str;
		std::vector<std::wstring>::iterator it;
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"ppstream.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}

		str = L"";

		if(m_bScan)
		{
			wstring strPath;
			wstring strPath1;
			wstring strPath2;
			BOOL bRet = FALSE;
			WCHAR szPath[MAX_PATH] = {0};
			bRet = ::SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
			strPath = szPath;
			strPath1 = szPath;
			strPath2 = szPath;
			strPath += L"\\PPStream\\PPStream.ini";
			strPath1 += L"\\PPStream\\MediaList";
			strPath2 += L"\\PPStream\\MediaList\\PlayList";

			CSimpleArray<CString> vec_file;
			vec_file.Add(L"LocalMedia.xml");
			vec_file.Add(L"MediaDirs.xml");

			CSimpleArray<CString> vec_file1;
			vec_file1.Add(L"默认播放列表.xml");

			CSimpleArray<CString> vec_reg;

			
			ScanSection_PPS(PPS_PLAYER,_T("recent"),strPath.c_str());
			m_appHistory.CommfunFile(PPS_PLAYER, strPath1.c_str(), vec_file);
			m_appHistory.CommfunFile(PPS_PLAYER, strPath2.c_str(), vec_file1);	
			
			
			m_appHistory.CommfunReg(PPS_PLAYER, L"HKEY_CURRENT_USER\\Software\\PPStream\\recent", vec_reg);
			

		}
_exit_:
		g_fnScanReg(g_pMain, ENDPROC(PPS_PLAYER), str.c_str(), 0, 0);

		g_fnScanFile(g_pMain,ENDPROC(PPS_PLAYER), str.c_str(), 0 ,0);
		return TRUE;
	}

	BOOL ScanSection_PPS(int iType,LPCTSTR lpcszAppPath,LPCTSTR lpcszIniPath)
	{
		BOOL retval = FALSE;
		TCHAR* szBuffer = NULL;
		TCHAR szKey[1024] = {0};
		CString strKey = _T("");
		CString strKeyName = _T("");
		CString strKeyValue = _T("");
		int nBufferSize;

		szBuffer = new TCHAR[65536];
		if (!szBuffer)
			goto clean0;

		nBufferSize = GetPrivateProfileSection(
			lpcszAppPath, 
			szBuffer, 
			65536,
			lpcszIniPath
			);

		for (int n = 0, i = 0; n < nBufferSize; n++)
		{
			if (szBuffer[n] == 0)
			{
				szKey[i] = 0;
				strKey = szKey;

				strKeyName = strKey.Left(strKey.Find('='));
				strKeyValue = strKey.Mid(strKey.Find('=') + 1);

				CString strPath;
				strPath.Format(_T("%s|%s|%s|%s"),lpcszIniPath,lpcszAppPath,strKeyName,strKeyValue);

				g_fnScanFile(g_pMain,iType,strPath,0,0);

				i = 0;

			}
			else
			{
				szKey[i] = szBuffer[n];

				i++;
			}

		}

		retval = TRUE;

	clean0:
		if (szBuffer)
		{
			delete[] szBuffer;
			szBuffer = NULL;
		}

		return retval;
	}

	BOOL CleanPPS(LPCTSTR lpcszPath)
	{	
		CString strPath = lpcszPath;
		g_regClean.ConvetPath(strPath);

		CSimpleArray<CString> vec_String;
				
		if (SplitCString_PPS(strPath,vec_String,'|') < 3)
		{
			return FALSE;
		}

		if (TRUE !=WritePrivateProfileStruct(vec_String[1],vec_String[2], NULL, 0,vec_String[0]))
		{
			DWORD dwErrCode = GetLastError();
			if (dwErrCode == ERROR_FILE_NOT_FOUND)	//系统找不到指定文件
			{
				OutputDebugString(_T("系统找不到指定文件"));
				return TRUE;
			}
			else if (dwErrCode == ERROR_ACCESS_DENIED)
			{	
				OutputDebugString(_T("拒绝访问"));
				return FALSE;
			}
			else
			{	
				OutputDebugString(_T("其他错误"));
				return FALSE;
			}
		}
	
		return TRUE;
	}

	int SplitCString_PPS(CString strIn, CSimpleArray<CString>& vec_String, TCHAR division) 
	{ 
		vec_String.RemoveAll(); 
		if (!strIn.IsEmpty()) 
		{ 
			int nCount = 0; 
			int nPos = -1; 
			nPos = strIn.Find(division); 
			CString strTemp = strIn; 
			while (nPos != -1) 
			{ 
				CString strSubString = strTemp.Left(nPos);  
				strTemp = strTemp.Right(strTemp.GetLength() - nPos-1);  
				nPos = strTemp.Find(division); 
				nCount++; 
				vec_String.Add(strSubString); 
			} 

			if (nCount == vec_String.GetSize()) 
			{ 
				CString str; 
				int nSize = strIn.ReverseFind(division); 
				str = strIn.Right(strIn.GetLength()-nSize-1); 
				vec_String.Add(str); 
			} 
		} 

		return vec_String.GetSize(); 
	}

private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;

};

class CPPTVPlayer
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanPPTV()
	{
		sqlite3* pDB = NULL;
		sqlite3_stmt* sspStart = NULL;
		std::wstring str;
		std::vector<std::wstring>::iterator it;
		g_fnScanFile(g_pMain,BEGINPROC(PPTV_PLAYER),0,0,0);
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"pplive.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}

		str = L"";

		if(m_bScan)
		{
			
			wstring strPath;
	
			int nResult = -1;

			BOOL bRet = FALSE;
			WCHAR szPath[MAX_PATH] = {0};
			wstring strPath1;
			bRet = ::SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
			strPath = szPath;
			char* szError = NULL;

			strPath += L"\\PPLive\\PPTV\\Favorites\\Favorites.s3db";
			strPath1 = strPath;
			KW2UTF8  szDataPath(strPath.c_str());
			nResult = sqlite3_open(szDataPath, &pDB);
			if (nResult != SQLITE_OK)
			{
				goto _exit_;
			}
			nResult = sqlite3_prepare(pDB, "select * from Channel where vt=3", -1, &sspStart, 0);
			if (nResult != SQLITE_OK)
			{
				goto _exit_;
			}

			nResult = sqlite3_step(sspStart);

			while(nResult == SQLITE_ROW)
			{
				const char* szName = (char*)sqlite3_column_text(sspStart, 15);
				const char* szTaskID = (char*)sqlite3_column_text(sspStart, 5);
				wstring strName;
				wstring strId;
				strId = Utf8ToUnicode(szTaskID);	
				strName = Utf8ToUnicode(szName);
				wstring strOutPut;
				strOutPut = strPath;
				strOutPut += L"|";
				strOutPut += strId;
				strOutPut += L"|";
				strOutPut += strName;
				g_fnScanFile(g_pMain, PPTV_PLAYER, strOutPut.c_str(), 0, 0);

				nResult = sqlite3_step(sspStart);
			}
		}
_exit_:
		if (sspStart)
		{
			sqlite3_finalize(sspStart);
			sspStart = NULL;
		}
		g_fnScanFile(g_pMain,ENDPROC(PPTV_PLAYER),str.c_str(),0,0);
		if (pDB)
		{
			sqlite3_close(pDB);
			pDB = NULL;
		}
		return TRUE;
	}

	BOOL CleanPPTV(CString strFullPathName)
	{
		CString strPath;
		sqlite3* pDB;
		int nResult = -1;
		CString strName;
		CString strId;
		int nPos = -1;
		string szPath;
		string szName;
		string szId;
		char* szError = NULL;
        CStringA strSql;

		nPos = strFullPathName.Find('|');
		if (nPos == -1)
		{
			return FALSE;
		}
		strPath = strFullPathName.Left(nPos);
		strFullPathName = strFullPathName.Right(strFullPathName.GetLength() - nPos - 1);
        nPos = strFullPathName.Find('|');
		if (nPos == -1)
		{
			goto _exit_;
		}
		strId = strFullPathName.Left(nPos);
		strName = strFullPathName.Right(strFullPathName.GetLength() - nPos - 1);

		szPath = UnicodeToUtf8(strPath.GetBuffer());
		szName = UnicodeToUtf8(strName.GetBuffer());
		szId = UnicodeToUtf8(strId.GetBuffer());

		nResult = sqlite3_open(szPath.c_str(), &pDB);
		if (nResult != SQLITE_OK)
		{
			goto _exit_;
		}
		char szBuff[MAX_PATH] = {0};
		if(szName.empty())
			goto _exit_;
        strSql.Format("delete from Channel where nm='%s'", szName.c_str());

		nResult = sqlite3_exec(pDB, strSql, 0, 0, &szError);
		if (nResult != SQLITE_OK)
		{
			goto _exit_;
		}
		ZeroMemory(szBuff, MAX_PATH);
		
		if(szId.empty())
			goto _exit_;

        strSql.Format("delete from StoreType where id=%s", szId.c_str());

		nResult = sqlite3_exec(pDB, strSql, 0, 0, &szError);
		if (nResult != SQLITE_OK)
		{
			goto _exit_;
		}

		if(pDB)
		{
			sqlite3_close(pDB);
			pDB = NULL;
		}
		return TRUE;
_exit_:
		if(pDB)
		{
			sqlite3_close(pDB);
			pDB = NULL;
		}
		return TRUE;
	}
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;

};

class CQVODPlayer
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanQVOD()
	{
		std::wstring str;
		std::vector<std::wstring>::iterator it;
		g_fnScanFile(g_pMain,BEGINPROC(QVOD_PLAYER),0,0,0);
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"qvodplayer.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}

		str = L"";

		if(m_bScan)
		{
			BOOL bRet = FALSE;
			wstring strPath;

			KSearchSoftwareStruct sss;
			wstring strPath1;
			WCHAR szPath[MAX_PATH] = {0};
			WCHAR szPath2[MAX_PATH] = {0};
			sss.pszMainFileName      = TEXT( "QvodPlayer.exe" );
			sss.hRegRootKey          = HKEY_CURRENT_USER;
			sss.pszRegSubKey         = TEXT( "Software\\QvodPlayer\\Insert" );
			sss.pszPathValue      	 = TEXT( "Insertpath" );
			sss.bFolder              = TRUE;

			bRet = SearchSoftwarePath( &sss, strPath1 );
			wcscpy_s(szPath, MAX_PATH - 1, strPath1.c_str());
			PathRemoveFileSpec(szPath);
			strPath1 = szPath;
			strPath = szPath;
			strPath1 += L"\\Playlist";
			strPath += L"\\QvodPlayer.xml";

//			strPath = L"HKEY_CURRENT_USER\\Software\\QvodPlayer\\Option";	
//			SimpleRegister_QueryStringValue(HKEY_CURRENT_USER, L"Software\\QvodPlayer\\Option", L"defaultsavepath", szPath2, MAX_PATH);

			CSimpleArray<CString> vec_value;
			CSimpleArray<CString> vec_value1;
			vec_value1.Add(_T("Playlist.xml"));
			vec_value1.Add(_T("Channel.xml"));

			
			ParXml(strPath);
		//	m_appHistory.CommfunReg(QVOD_PLAYER, szPath, vec_value);
			m_appHistory.CommfunFile(QVOD_PLAYER, strPath1.c_str(), vec_value1);

		
		}
_exit_:
		g_fnScanFile(g_pMain,ENDPROC(QVOD_PLAYER),str.c_str(),0,0);
		return TRUE;
	}

	BOOL ParXml(wstring strPath)
	{
		FILE* lpFile = _tfopen(strPath.c_str(), _T("rb"));
		TiXmlDocument xmlDoc;
		if ( lpFile != NULL )
		{	
			xmlDoc.LoadFile(lpFile);

			fclose(lpFile);

			if ( xmlDoc.Error() == 0 )
			{
				TiXmlElement* rootNode = xmlDoc.RootElement();
				if (rootNode!=NULL)
				{
					TiXmlElement* nodeGeneral = rootNode->FirstChildElement("NewLy");
					if ( nodeGeneral != NULL )
					{	
						const char* name =  nodeGeneral->Attribute("NewLy_Count");
						USES_CONVERSION;
						wchar_t* lpwName=A2W(name);
						if (_wcsicmp(lpwName, L"0") != 0)
						{
							CString strFind;
							strFind.Format(_T("%s|NewLy"), strPath.c_str(), lpwName);

							g_fnScanFile(g_pMain,QVOD_PLAYER,strFind,0,0);
							}
					}
				}
			}
		}

		return TRUE;
	}

	BOOL CleanQcod(int iType, LPCTSTR lpcszAppPath)
	{
		CString strPath = lpcszAppPath;

		CSimpleArray<CString> vec_String;
		int iCount = SplitCString1(strPath,vec_String,'|');

		if(iCount<2)
		{
			return FALSE;
		}

		TiXmlDocument xmlDoc;


		//获得文件夹名称改变其属性
		iCount = vec_String[0].ReverseFind('\\');
		CString strTmp;
		if (iCount>0)
		{
			strTmp = vec_String[0].Mid(0,iCount);
		}

		DWORD dwErrCode=0;
		if(FALSE == SetFileAttributes(strTmp, FILE_ATTRIBUTE_NORMAL))
		{
			dwErrCode=GetLastError();
		}

		USES_CONVERSION;
		FILE* lpFile = _tfopen(vec_String[0], _T("a+b"));
		if ( lpFile != NULL )
		{
			xmlDoc.LoadFile(lpFile);

			fclose(lpFile);

			if ( xmlDoc.Error() == 0 )
			{
				TiXmlElement* rootNode = xmlDoc.RootElement();
				if (rootNode !=NULL)
				{
					TiXmlElement* nodeGeneral = rootNode->FirstChildElement(W2A(vec_String[1]));
					if ( nodeGeneral != NULL )
					{		
						if (!rootNode->RemoveChild(nodeGeneral))
						{
							OutputDebugString(_T("\n删除子节点失败\n"));
						}
						else
						{	
							if (!xmlDoc.SaveFile(W2A(vec_String[0])))
							{
								OutputDebugString(_T("保存失败"));
							}

						}

					}
				}
			}
		}

		return TRUE;
	}

	inline int SplitCString1(CString strIn, CSimpleArray<CString>& vec_String, TCHAR division) 
	{ 
		vec_String.RemoveAll(); 
		if (!strIn.IsEmpty()) 
		{ 
			int nCount = 0; 
			int nPos = -1; 
			nPos = strIn.Find(division); 
			CString strTemp = strIn; 
			while (nPos != -1) 
			{ 
				CString strSubString = strTemp.Left(nPos);  
				strTemp = strTemp.Right(strTemp.GetLength() - nPos-1);  
				nPos = strTemp.Find(division); 
				nCount++; 
				vec_String.Add(strSubString); 
			} 

			if (nCount == vec_String.GetSize()) 
			{ 
				CString str; 
				int nSize = strIn.ReverseFind(division); 
				str = strIn.Right(strIn.GetLength()-nSize-1); 
				vec_String.Add(str); 
			} 
		} 

		return vec_String.GetSize(); 
	} 
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;

};

class CKFengxingPlay
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;}
	BOOL    ScanFengxingPlayer()
	{
		BOOL bRet = FALSE;
		std::wstring str;
		std::wstring strPath;
		CSimpleArray<CString> vec_file;
		g_fnScanFile(g_pMain, BEGINPROC(FENGXING_PLAYER), 0, 0, 0);
		if (m_bScan)
		{
			strPath = L"%userprofile%\\funshion\\HistoryTorrent";
			m_appHistory.CommfunFile(FENGXING_PLAYER, strPath.c_str(), vec_file);
		}
//clean0:
		g_fnScanFile(g_pMain, ENDPROC(FENGXING_PLAYER), str.c_str(), 0, 0);
		return bRet;
	}
private:
	BOOL m_bScan;
	CAppHistory m_appHistory;
};

class CKShenshouPlay
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;}
	BOOL    ScanSheshouPlayer()
	{
		BOOL bRet = FALSE;
		std::wstring str;
		std::wstring strPath;
		CSimpleArray<CString> vec_file;
		WCHAR szBuffer[MAX_PATH] = {0};
		std::vector<std::wstring>::iterator it;

		SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_APPDATA, FALSE);
		PathAppend(szBuffer, L"Splayer");
		g_fnScanFile(g_pMain, BEGINPROC(SHESHOU_PLAYER), 0, 0, 0);
		g_fnScanReg(g_pMain, BEGINPROC(SHESHOU_PLAYER), 0, 0, 0);

		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"splayer.exe")
			{
				str = L"正在运行，跳过";
				goto clean0;
			}
		}
		str = L"";
		if (m_bScan)
		{
			strPath = L"HKEY_CURRENT_USER\\Software\\SPlayer\\射手影音播放器\\Recent File List";
			m_appHistory.CommfunReg(SHESHOU_PLAYER, strPath.c_str(), vec_file);

			strPath = szBuffer;
			vec_file.Add(L"default.mpcpl");
			m_appHistory.CommfunFile(SHESHOU_PLAYER, strPath.c_str(), vec_file);
		}
clean0:
		g_fnScanReg(g_pMain, ENDPROC(SHESHOU_PLAYER), str.c_str(), 0, 0);
		g_fnScanFile(g_pMain, ENDPROC(SHESHOU_PLAYER), str.c_str(), 0, 0);
		return bRet;
	}
private:
	BOOL m_bScan;
	CAppHistory m_appHistory;
};

class CKKuwoMusicPlay
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;}
	BOOL    ScanKuwoPlayer()
	{
		KSearchSoftwareStruct sss;
		std::vector<std::wstring>::iterator it;
		WCHAR szPath[MAX_PATH] = {0};
		sss.pszMainFileName      = TEXT( "KwMusic.exe" );
		sss.nDefPathFolderCSIDL  = CSIDL_PROGRAM_FILES;
		sss.pszDefPath           = TEXT( "KWMUSIC" );
		sss.hRegRootKey          = HKEY_LOCAL_MACHINE;
		sss.pszRegSubKey         = TEXT( "SOFTWARE\\KWMUSIC" );
		sss.pszPathValue      	 = TEXT( "KWMUSIC_HOME" );
		sss.bFolder              = TRUE;
		BOOL bRet = FALSE;
		std::wstring str;
		std::wstring strPath;
		std::wstring strTemp;
		bRet = SearchSoftwarePath( &sss, strPath);
		wcscpy_s(szPath, MAX_PATH - 1, strPath.c_str());
		PathRemoveFileSpec(szPath);
		strPath = szPath;
		strTemp = szPath;
		CSimpleArray<CString> vec_file;
		g_fnScanFile(g_pMain, BEGINPROC(KUWOMUSIC_PLAYER), 0, 0, 0);
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"kwmusic.exe")
			{
				str = L"正在运行，跳过";
				goto clean0;
			}
		}
		str = L"";
		if (m_bScan)
		{
			strPath += L"\\playlist";
			m_appHistory.CommfunFile(KUWOMUSIC_PLAYER, strPath.c_str(), vec_file);

			strPath = strTemp;
			strPath += L"\\config.ini";
			ScanKuWoIni(strPath.c_str(), L"Playlist", L"CurPlaylist");
			ScanKuWoIni(strPath.c_str(), L"Playlist", L"PlaylistShowing");
			ScanKuWoIni(strPath.c_str(), L"Search", L"LastKeyword");
			ScanKuWoIni(strPath.c_str(), L"Search", L"LastKeywordType");
		}
clean0:
		g_fnScanFile(g_pMain, ENDPROC(KUWOMUSIC_PLAYER), str.c_str(), 0, 0);
		return bRet;
	}

	BOOL ScanKuWoIni(CString strPath, CString strSection, CString strKeyName)
	{
		BOOL retval = FALSE;
		TCHAR* szBuffer = NULL;
		TCHAR szKey[1024] = {0};
		int nBufferSize;
		CString strOut;

		szBuffer = new TCHAR[65536];
		if (!szBuffer)
			goto clean0;

		nBufferSize = GetPrivateProfileString(strSection, strKeyName, NULL, szBuffer, 65536, strPath);

		if (nBufferSize == 0)
		{
			goto clean0;
		}

		strOut.Format(_T("%s|%s|%s|%s"),strPath,strSection,strKeyName,szBuffer);

		g_fnScanFile(g_pMain,KUWOMUSIC_PLAYER,strOut,0,0);

clean0:
		if (szBuffer)
		{
			delete[] szBuffer;
			szBuffer = NULL;
		}

		return retval;
	}

	BOOL CleanKuWo(CString strFullInfo)
	{
		CString strPath = strFullInfo;

		std::vector<std::wstring> vec_String;

		if (SplitCString(strPath.GetBuffer(),vec_String,'|') < 3)
		{
			return FALSE;
		}

		if (TRUE !=WritePrivateProfileStruct(vec_String[1].c_str(),vec_String[2].c_str(), NULL, 0,vec_String[0].c_str()))
		{
			DWORD dwErrCode = GetLastError();
			if (dwErrCode == ERROR_FILE_NOT_FOUND)	//系统找不到指定文件
			{
				return TRUE;
			}
		}

		return TRUE;
	}
private:
	BOOL m_bScan;
	CAppHistory m_appHistory;
};

class CKFixitPlay
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;}
	BOOL    ScanFixitPlayer()
	{
		BOOL bRet = FALSE;
		std::wstring strPath;
		CSimpleArray<CString> vec_file;
		g_fnScanReg(g_pMain, BEGINPROC(FUXINPDF_READER), 0, 0, 0);
		if (m_bScan)
		{
			strPath = L"HKEY_CURRENT_USER\\Software\\Foxit Software\\福昕阅读器\\Recent File List";
			m_appHistory.CommfunReg(FUXINPDF_READER, strPath.c_str(), vec_file);
			strPath = L"HKEY_CURRENT_USER\\Software\\Foxit Software\\Foxit Reader\\Recent File List";
			m_appHistory.CommfunReg(FUXINPDF_READER, strPath.c_str(), vec_file);
		}
//clean0:
		g_fnScanReg(g_pMain, ENDPROC(FUXINPDF_READER), 0, 0, 0);
		return bRet;
	}
private:
	BOOL m_bScan;
	CAppHistory m_appHistory;
};

class CKAliIm
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;}
	BOOL    ScanAliim()
	{
		KSearchSoftwareStruct sss;
		std::vector<std::wstring>::iterator it;
		WCHAR szPath[MAX_PATH] = {0};
		sss.pszMainFileName      = TEXT("AliIM.exe");
		sss.pszUninstallListName = TEXT("阿里旺旺");
		sss.bFolder              = TRUE;
		BOOL bRet = FALSE;
		std::wstring str;
		std::wstring strPath;
		std::wstring strTemp;
		bRet = SearchSoftwarePath( &sss, strPath);
		wcscpy_s(szPath, MAX_PATH - 1, strPath.c_str());
		PathRemoveFileSpec(szPath);
		strPath = szPath;
		PathAppend(szPath, L"profiles\\system");
		strTemp = szPath;
		CSimpleArray<CString> vec_file;
		vec_file.Add(L"logininfo.xml");
		vec_file.Add(L"lastlogininfo2.xml");
		vec_file.Add(L"lastlogininfo.xml");
		vec_file.Add(L"gconf.db");
		CSimpleArray<CString> vec_Temp;
		vec_Temp.Add(L"log.log");
		g_fnScanFile(g_pMain, BEGINPROC(ALIIM_IM), 0, 0, 0);

		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"aliim.exe")
			{
				str = L"正在运行，跳过";
				goto clean0;
			}
		}
		str = L"";
		if (m_bScan)
		{
			m_appHistory.CommfunFile(ALIIM_IM, strTemp.c_str(), vec_file);
			m_appHistory.CommfunFile(ALIIM_IM,strPath.c_str(), vec_Temp);
		}
clean0:
		g_fnScanFile(g_pMain, ENDPROC(ALIIM_IM), str.c_str(), 0, 0);
		return bRet;
	}
private:
	BOOL m_bScan;
	CAppHistory m_appHistory;
};

class CKWinLiveMsg
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;}
	BOOL    ScanWinlivemsg()
	{
		BOOL bRet = FALSE;
		std::wstring strPath;
		std::wstring str;
		CSimpleArray<CString> vec_file;
		std::vector<std::wstring>::iterator it;
		WCHAR szPath[MAX_PATH] = {0};
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
		strPath = szPath;
		if (strPath.rfind(L'\\') != strPath.size() - 1)
		{
			strPath += L"\\";
		}

		strPath += L"Microsoft\\MSN Messenger";
		vec_file.Add(L"*.sqm");
		g_fnScanFile(g_pMain, BEGINPROC(WINLIVE_MSG), 0, 0, 0);

		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"msnmsgr.exe")
			{
				str = L"正在运行，跳过";
				goto clean0;
			}
		}
		str = L"";
		if (m_bScan)
		{
			m_appHistory.CommfunFile(WINLIVE_MSG, strPath.c_str(), vec_file);
		}
clean0:
		g_fnScanFile(g_pMain, ENDPROC(WINLIVE_MSG), str.c_str(), 0, 0);
		return bRet;
	}
private:
	BOOL m_bScan;
	CAppHistory m_appHistory;
};
#endif
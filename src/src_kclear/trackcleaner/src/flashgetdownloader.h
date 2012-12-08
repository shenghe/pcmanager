#pragma once
#include "CleanProc.h"
#include "i_clean.h"
#include <algorithm>
#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"
#include "getinstallpath.h"
#include <string>
#include "kscbase/kscconv.h"

using namespace std;
extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;
extern std::vector<std::wstring> g_listProcessName;

class CFlashGetDownloader
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanFlashGet()
	{
		std::wstring str;
		g_fnScanFile(g_pMain,BEGINPROC(FLASHGETDOWNLOADER),0,0,0);

		std::vector<std::wstring>::iterator it;
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"flashget3.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}

	   str = L"";
		if (m_bScan)
		{
			KSearchSoftwareStruct sss;
			wstring strPath;
			wstring strPath2;
			BOOL bRet = FALSE;
			WCHAR szPath[MAX_PATH] = {0};
			sss.pszMainFileName      = TEXT( "flashget3.exe" );
			sss.hRegRootKey = HKEY_LOCAL_MACHINE;
			sss.pszRegSubKey = _T("SOFTWARE\\FlashGet Network\\FlashGet 3");
			sss.pszPathValue = _T("Path");

			bRet = SearchSoftwarePath( &sss, strPath );

			wcscpy_s(szPath, MAX_PATH - 1, strPath.c_str());
			PathRemoveFileSpec(szPath);
			strPath = szPath;

			strPath += L"\\dat";
			CSimpleArray<CString> vec_file;
			vec_file.Add(_T("FlashGet3db.db"));
			vec_file.Add(_T("FlashGet3db.bak"));

			bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);

			strPath2 = szPath;
			strPath2 += L"\\FlashGet\\v3\\dat";

			
			m_appHistory.CommfunFile(FLASHGETDOWNLOADER, strPath.c_str(), vec_file);
			OutputDebugString(strPath2.c_str());
			m_appHistory.CommfunFile(FLASHGETDOWNLOADER, strPath2.c_str(), vec_file);

		}
_exit_:
		g_fnScanFile(g_pMain,ENDPROC(FLASHGETDOWNLOADER),str.c_str(),0,0);
		return TRUE;
	}
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;

};

class CBitcometDownloader
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanBitComet()
	{
		std::wstring str;
		g_fnScanFile(g_pMain,BEGINPROC(BITCOMETDOWNLOADER),0,0,0);

		std::vector<std::wstring>::iterator it;
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"bitcomet.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}

		str = L"";
		if (m_bScan)
		{
			KSearchSoftwareStruct sss;
			wstring strPath;
			wstring strPath2;
			wstring strFile;
			BOOL bRet = FALSE;
			WCHAR szPath[MAX_PATH] = {0};
			sss.pszMainFileName      = TEXT( "BitComet.exe" );
			sss.nDefPathFolderCSIDL  = CSIDL_PROGRAM_FILES;
			sss.pszDefPath           = TEXT( "BitComet" );
			sss.pszUninstallListName = TEXT( "BitComet" );
			sss.pszFileDesc          = TEXT( "BitComet" );
			sss.hRegRootKey          = HKEY_CURRENT_USER;
			sss.pszRegSubKey         = TEXT( "Software\\BitComet" );
			sss.pszPathValue      	 = TEXT( "" );
			sss.bFolder              = TRUE;

			bRet = SearchSoftwarePath( &sss, strPath );

			wcscpy_s(szPath, MAX_PATH - 1, strPath.c_str());
			PathRemoveFileSpec(szPath);
			strPath = szPath;
			CSimpleArray<CString> vec_file;
			vec_file.Add(_T("Downloads.xml"));

			ZeroMemory(szPath, MAX_PATH);
			bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
			strPath2 = szPath;
			strPath2 += L"\\";
			strPath2 += L"BitComet";

			CSimpleArray<CString> vec_file2;
			vec_file2.Add(_T("Downloads.xml"));
			vec_file2.Add(_T("Downloads.xml.bak"));

			g_fnScanFile(g_pMain,BEGINPROC(BITCOMETDOWNLOADER),0,0,0);
//			m_appHistory.CommfunFile(BITCOMETDOWNLOADER, strPath.c_str(), vec_file);
			strFile = L"Downloads.xml";
			CheckComplete(strPath, strFile);
			OutputDebugString(strPath2.c_str());
			CheckComplete(strPath2, strFile);

			//m_appHistory.CommfunFile(BITCOMETDOWNLOADER, strPath2.c_str(), vec_file2);
			
			
		}
_exit_:
		g_fnScanFile(g_pMain,ENDPROC(BITCOMETDOWNLOADER),str.c_str(),0,0);
		return TRUE;
	}

	BOOL CheckComplete(wstring strPath, wstring strFile)
	{
		strPath += L"\\";
		strPath += strFile;
		FILE* lpFile = _tfopen(strPath.c_str(), _T("rb"));
		TiXmlDocument xmlDoc;
		if ( lpFile != NULL )
		{	
			xmlDoc.LoadFile(lpFile);

			fclose(lpFile);

			if ( xmlDoc.Error() == 0 )
			{
				OutputDebugString(_T("打开文件成功"));

				TiXmlElement* rootNode = xmlDoc.RootElement();
				if (rootNode!=NULL)
				{
					TiXmlElement* nodeGeneral = rootNode->FirstChildElement("Torrents");
					if ( nodeGeneral != NULL )
					{	
						TiXmlElement* node = nodeGeneral->FirstChildElement("Torrent");

						while (node)
						{
							const char* szSelectSize = node->Attribute("Size");
							const char* szDataDownload = node->Attribute("DataDownload");
							USES_CONVERSION;

							if(_stricmp(szSelectSize, szDataDownload) == 0)
							{
								const char* szShowName =  node->Attribute("ShowName");
								CString strShowName = KUTF8_To_UTF16(szShowName);
								CString strFind;
								strFind.Format(_T("%s|Torrents|Torrent|%s"), strPath.c_str(), strShowName);
								g_fnScanFile(g_pMain,BITCOMETDOWNLOADER,strFind,0,0);
//								const char szTorrentFile = 

							}
							node = node->NextSiblingElement("Torrent");

						}
						node = nodeGeneral->FirstChildElement("Download");

						while (node)
						{
							const char* szSelectSize = node->Attribute("LeftSize");
							const char* szDataDownload = node->Attribute("DataDownload");
							USES_CONVERSION;

							if(_stricmp(szSelectSize, "0") == 0)
							{
								const char* szShowName =  node->Attribute("DetailFile");
								CString strShowName = KUTF8_To_UTF16(szShowName);
								CString strFind;
								strFind.Format(_T("%s|Torrents|Download|%s"), strPath.c_str(), strShowName);
								g_fnScanFile(g_pMain,BITCOMETDOWNLOADER,strFind,0,0);
								//								const char szTorrentFile = 

							}
							node = node->NextSiblingElement("Download");

						}

					}
					else
						OutputDebugString(_T("\n没找到相应属性\n"));
				}
			}
			else
				OutputDebugString(_T("\n读取文件失败\n"));
		}
		else
			OutputDebugString(_T("\n打开文件失败\n"));

		return TRUE;
	}

	BOOL CleanBitcomet(LPCTSTR strDownload)
	{
		CString strPath = strDownload;

		CSimpleArray<CString> vec_String;

		CString strFileTemp;
		int iCount = SplitCString1(strPath,vec_String,'|');

		if(iCount<3)
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
				OutputDebugString(_T("\n打开文件成功\n"));

				TiXmlElement* rootNode = xmlDoc.RootElement();
				if (rootNode !=NULL)
				{
					TiXmlElement* nodeGeneral = rootNode->FirstChildElement(W2A(vec_String[1]));
					if ( nodeGeneral != NULL )
					{	
						TiXmlElement* node = nodeGeneral->FirstChildElement(W2A(vec_String[2]));

						while(node)
						{	
							const char* szFinishData=  node->Attribute("ShowName");
                            CString strFinishData = KUTF8_To_UTF16(szFinishData);
							const char* szHash=  node->Attribute("InfoHashHex");
							wchar_t* szTemp = A2W(szHash);
							if (szFinishData == NULL)
							{
								goto Next;
							}
							if (_wcsicmp(strFinishData, vec_String[3]) == 0)
							{
								if (!nodeGeneral->RemoveChild(node))
								{
									OutputDebugString(_T("\n删除子节点失败\n"));

								}
								else
								{	
									strFileTemp.Format(_T("%s\\archive"), strTmp);
									CleanShareAndHistory(strFileTemp, L"MyHistory", vec_String[3], szTemp, L"my_history.xml");
									strFileTemp.Format(_T("%s\\share"), strTmp);
									CleanShareAndHistory(strFileTemp, L"MyHistory", vec_String[3], szTemp, L"my_shares.xml");
									if (!xmlDoc.SaveFile(W2A(vec_String[0])))
									{
										OutputDebugString(_T("保存失败"));
									}

								}
								break;
							}
Next:
							szFinishData=  node->Attribute("DetailFile");
                            strFinishData = KUTF8_To_UTF16(szFinishData);
							szHash=  node->Attribute("FileHash");
							szTemp = A2W(szHash);
							if (_wcsicmp(strFinishData, vec_String[3]) == 0)
							{
								OutputDebugString(A2W(szFinishData));
								if (!nodeGeneral->RemoveChild(node))
								{
									OutputDebugString(_T("\n删除子节点失败\n"));

								}
								else
								{	
									strFileTemp.Format(_T("%s\\archive"), strTmp);
									CleanShareAndHistory(strFileTemp, L"MyHistory", vec_String[3], szTemp, L"my_history.xml");
									strFileTemp.Format(_T("%s\\share"), strTmp);
									CleanShareAndHistory(strFileTemp, L"MyHistory", vec_String[3], szTemp, L"my_shares.xml");

									strFileTemp.Format(_T("%s\\torrents"), strTmp);
									CleanShareAndHistory(strFileTemp, NULL, vec_String[3], szTemp, NULL);
									if (!xmlDoc.SaveFile(W2A(vec_String[0])))
									{
										OutputDebugString(_T("保存失败"));
									}

								}
								break;
							}
							node = node->NextSiblingElement(W2A(vec_String[2]));
						}
					}
				}
			}
		}

		return TRUE;
	}

	BOOL CleanShareAndHistory(LPCTSTR szFilePath, LPCTSTR strHead, LPCTSTR strTitle, LPCTSTR strTorrentName, LPCTSTR strXmlName)
	{
		CString strFilePath = szFilePath;
		strFilePath += L"\\";
		strFilePath += strXmlName;
		TiXmlDocument xmlDoc;
		wstring strFileTrrorent;

		//获得文件夹名称改变其属性
		int nCount = strFilePath.ReverseFind('\\');
		CString strTmp;
		if (nCount>0)
		{
			strTmp = strFilePath.Mid(0,nCount);
		}

		DWORD dwErrCode=0;
		if(FALSE == SetFileAttributes(strTmp, FILE_ATTRIBUTE_NORMAL))
		{
			dwErrCode=GetLastError();
		}

		USES_CONVERSION;
		FILE* lpFile = _tfopen(strFilePath, _T("a+b"));
		if ( lpFile != NULL )
		{
			xmlDoc.LoadFile(lpFile);

			fclose(lpFile);

			if ( xmlDoc.Error() == 0 )
			{
				OutputDebugString(_T("\n打开文件成功\n"));

				TiXmlElement* rootNode = xmlDoc.RootElement();
				if (rootNode !=NULL)
				{
					TiXmlElement* nodeGeneral = rootNode->FirstChildElement(W2A(strHead));
					if ( nodeGeneral != NULL )
					{	
						TiXmlElement* node = nodeGeneral->FirstChildElement("bt");

						while(node)
						{	

							const char* szFinishData=  node->Attribute("title");
							if (_stricmp(szFinishData, W2A(strTitle)) == 0)
							{
								if (!nodeGeneral->RemoveChild(node))
								{
									OutputDebugString(_T("\n删除子节点失败\n"));
								}
								else
								{	
									if (!xmlDoc.SaveFile(W2A(strFilePath)))
									{
										OutputDebugString(_T("保存失败"));
									}
								}
								break;
							}
							node = node->NextSiblingElement("bt");
						}
					}
				}
			}
		}

		strFileTrrorent = szFilePath;
		strFileTrrorent += L"\\";
		strFileTrrorent += strTorrentName;
		strFileTrrorent += L".torrent";
        ::DeleteFile(strFileTrrorent.c_str());
		strFileTrrorent = szFilePath;
		strFileTrrorent += L"\\";
		strFileTrrorent += strTitle;

		::DeleteFile(strFileTrrorent.c_str());

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

class CQQDownloader
{
public:
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanQQDownloader()
	{
		std::wstring str;
		g_fnScanFile(g_pMain,BEGINPROC(QQDOWNLOADER),0,0,0);

		std::vector<std::wstring>::iterator it;
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"qqdownload.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}

		str = L"";

		if (m_bScan)
		{
			wstring strPath;
			wstring strPath1;
			wstring strPath2;
			BOOL bRet = FALSE;
			WCHAR szPath[MAX_PATH] = {0};
		
			CSimpleArray<CString> vec_file;
			vec_file.Add(_T("History.dat"));
			vec_file.Add(_T("BTHistory.dat"));
			vec_file.Add(_T("History.dat.xfbak"));
            vec_file.Add(_T("HistoryBk.dat"));

			WIN32_FIND_DATA wfd = {0};
			HANDLE hFile = INVALID_HANDLE_VALUE;

			ZeroMemory(szPath, MAX_PATH);
			bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
			strPath2 = szPath;
			strPath2 += L"\\";
			strPath2 += L"Tencent\\QQDownload";
			strPath = strPath2;
			strPath += L"\\*.*";

			hFile = FindFirstFile(strPath.c_str(), &wfd);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				do 
				{
					if (_wcsicmp(L".", wfd.cFileName) != 0 && _wcsicmp(L"..", wfd.cFileName) != 0 && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						strPath1 += strPath2;
						strPath1 += L"\\";
						strPath1 += wfd.cFileName;
						strPath1 += L"\\Setting";
						m_appHistory.CommfunFile(QQDOWNLOADER, strPath1.c_str(), vec_file);
						strPath1 = L"";
					}
				} while(FindNextFile(hFile, &wfd));
				::FindClose(hFile);
			}

			
		
		}
_exit_:
		g_fnScanFile(g_pMain,ENDPROC(QQDOWNLOADER),str.c_str(),0,0);
		return TRUE;
	}
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;

};

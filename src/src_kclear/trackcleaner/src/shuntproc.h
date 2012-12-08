#ifndef	 _SHUNTPROC_H_
#define  _SHUNTPROC_H_

#include "ieclean.h"
#include "windowshistory.h"
#include "regclean.h"
#include "garbageclean.h"
#include "apphistory.h"
#include "specialapp.h"
#include "kclear/icleanmgr.h"
#include "kclear/icleanproc.h"
#include <algorithm>

extern std::vector<std::wstring> g_listProcessName;
extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;

void IEClean(int iType,CIEClean& pIEClean)
{
	switch (iType)
	{
	case IECLEAN_TEMPFOLDER:
		pIEClean.ScanIETempFolder();
		break;
	case IECLEAN_VISITHOST:
		pIEClean.ScanVisitHost();
		break;
	case IECLEAN_COOKIE:
		pIEClean.ScanIECookie(IECLEAN_COOKIE);
		break;
	case IECLEAN_ADDRBSR:
		pIEClean.ScanIEAddrBar();
		break;
	case IECLEAN_INDEXDATA:
		pIEClean._CleanIndexDat();
		break;
	case IECLEAN_HISTORY:
		pIEClean.ScanIEHistory();
		break;
	case MAXTHON_COOKIES:
		pIEClean.ScanIECookie(MAXTHON_COOKIES);
		break;
	case THEWORLD_COOKIES:
		pIEClean.ScanIECookie(THEWORLD_COOKIES);
		break;
	//case IECLEAN_PASSWORD:
	//	pIEClean.ScanIEPass();
	//	break;
	//case IECLEAN_SAVEFROM:
	//	pIEClean.ScanIEFrom();
	//	break;
	}
}


void WinsHistory(int iType,CWindowsHistory& pWindowsHistory)
{
	switch (iType)
	{
	case WINSHISTORY_RUNDLG:
		pWindowsHistory.ScanRunDlg();
		break;
	case WINSHISTORY_RUNHISTORY:
		pWindowsHistory.ScanRunHistory();
		break;
	case WINSHISTORY_OPENSAVE:
		pWindowsHistory.ScanOpenOrSave();
		break;
	case WINSHISTORY_WINSIZE:
		pWindowsHistory.ScanWinSize();
		break;
	case WINSHISTORY_RECENTDOCREG:
		//pWindowsHistory.ScanRecentDocReg();
		pWindowsHistory.ScanRecentDocFile();
		break;
	case WINSHISTORY_FILEEXTS:
		pWindowsHistory.ScanFileExtsHistory();		
		break;
	case WINSHISTORY_USERASSIST:
		pWindowsHistory.ScanUserAssist();
		break;
	case WINSHISTORY_TRAYNOTIFY:
		pWindowsHistory.ScanTrayNotify();
		break;
	case WINSHISTORY_NETERIVE:
		pWindowsHistory.ScanNetDerive();
		break;
	case WINSHISTORY_FINDCMP:
		pWindowsHistory.ScanFindCmp();
		break;
	case WINSHISTORY_FINDDOC:
		pWindowsHistory.ScanFindDoc();
		break;
	case WINSHISTORY_PRTPORT:
		pWindowsHistory.ScanPrtPort();
		break;
	case WINSHISTORY_WINLOG:
		pWindowsHistory.ScanLogFolder();
		break;
	case WINSHISTORY_WINTEMP:
		pWindowsHistory.ScanTempFolder();
		break;
	case WINSHISTORY_STREAM:
		pWindowsHistory.ScanRegeditStream();
		break;
	case WINSHISTORY_REGEDIT:
		pWindowsHistory.ScanRegeditHistory();
		break;
	case GOOGL_TOOLBAR:
		pWindowsHistory.ScanGoogleToolBar();
		break;
	case BAIDU_TOOLBAR:
		pWindowsHistory.ScanBaiduToolBar();
		break;
	case WINSHISTORY_PRGHISTORY:
		pWindowsHistory.ScanWindowsPrgHistory();
		break;
	case MEMORY_DMP:
		pWindowsHistory.ScanWindowsMemoryDMP();
		break;
	case WINSHISTORY_CLIPBOARD:
		pWindowsHistory.CleanClipBoard();
		break;
	case FILEGARBAGE_RECYCLED:
		pWindowsHistory.ScanRecycled();
		break;
	case WINDOWS_SEARCH:
		pWindowsHistory.ScanWindowsSearch();
		break;
	case WINDOWS_NETHOOD:
		pWindowsHistory.ScanNethood();
		break;	
	case  WINDOWS_RECYCLE:
		pWindowsHistory.ScanRecycle();
		break;
	case  WINDOWS_JUMPLIST:
		pWindowsHistory.ScanWindows7JumpList();
		break;
	case  QQ_TOOLBAR:
		pWindowsHistory.ScanQQToolBar();
		break;
	case  WINLIVE_TOOLBAR:
		pWindowsHistory.ScanWindowLiveToolBar();
		break;
	case  WINDOWS_SUOLIETU:
		pWindowsHistory.ScanWinSuol();
		break;
	}
}


void RegClean(int iType,CRegClean& pRegClean)
{
	switch (iType)
	{
	case REGCLEAN_INVALIDDLL:
		pRegClean.ScanInvalidDll();
		break;

	case REGCLEAN_INVALIDMENU:
		pRegClean.ScanInvalidMenu();
		break;

	case REGCLEAN_INVALIDAPPPATH:
		pRegClean.ScanInvalidAppPath();
		break;

	case REGCLEAN_INVALIDFIRE:
		pRegClean.ScanIvalidFire();
		break;

	case REGCLEAN_INVALIDMUI:
		pRegClean.ScanIvalidMUICache();
		break;

	case REGCLEAN_INVALIDHELP:
		pRegClean.ScanIvalidHelp();
		break;

	case REGCLEAN_INVALIDFONT:
		pRegClean.ScanIvalidFont();
		break;

	case REGCLEAN_INVALIDINSTALL:
		pRegClean.ScanIvalidInstall();
		break;

	case REGCLEAN_INVALIDUNINSTALL:
		pRegClean.ScanInvalidUnInstall();
		break;

	case REGCLEAN_INVALIDSTARTMENU:
		pRegClean.ScanInvalidStartMenu();
		break;

	case REGCLEAN_INVALIDSTARTSRUN:
		pRegClean.ScanInvelidRun();
		break;

	case REGCLEAN_FAILACTIVEX:
		pRegClean.ScanInvelidActiveX();
		break;

	case REGCLEAN_FAILAFILEASS:
		pRegClean.ScanValidFileAss();
		break;

	case REGCLEAN_FAILCLASS:
		pRegClean.ScanInvalidClass();
		pRegClean.ScanInvalidClass2();
		break;

	case REGCLEAN_REDUNDANCY:
		pRegClean.ScanRegRedundancey();
		break;
	}
}


void GarbClean(int iType,CWindowsHistory& pWindowsHistory)
{
	switch (iType)
	{

	case FILEGARBAGE_RECYCLED:
		pWindowsHistory.ScanRecycled();
		break;

	case FILEGARBAGE_STARTMENULINK:
		pWindowsHistory.ScanInvaidStartLink();
		break;

	case FILEGARBAGE_DESKTOPLINK:
		pWindowsHistory.ScanInvaidDesktopLink();
		break;


	}
}

void IsProcessRun(std::wstring &strOut,std::wstring strProName,BOOL &bRun)
{
	std::vector<std::wstring>::iterator it;
	bRun = FALSE;

	 for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	 {
		 std::wstring str = *it;
		 transform(str.begin(), str.end(), str.begin(), towlower);
		 if (str == strProName)
		 {
			 strOut = L"正在运行，跳过";
			 bRun = TRUE;
			 break;
		 }
	 }

}

BOOL RunProcessExit(int iType)
{
	BOOL bIsRun = FALSE;
	std::wstring str = L"";
	switch (iType)
	{
	case 9002:
		{	
			IsProcessRun(str,L"acrord32.exe",bIsRun);
		}
		break;
	case 9003:
		{	
			IsProcessRun(str,L"mspaint.exe",bIsRun);
		}
		break;
	case 9004:
		{	
			IsProcessRun(str,L"wordpad.exe",bIsRun);
		}
		break;
	case 9005:
		{	
			IsProcessRun(str,L"wmplayer.exe",bIsRun);
		}
		break;
	case 9007:
		{	

			IsProcessRun(str,L"acdsee.exe",bIsRun);
		}
		break;
	case 9008:
		{	
			IsProcessRun(str,L"storm.exe",bIsRun);
		}
		break;
	case 9009:
		{	
			IsProcessRun(str,L"kmplayer.exe",bIsRun);
		}
		break;
	case 9010:
		{
			IsProcessRun(str,L"kugoo.exe",bIsRun);
		}
		break;
	case 9011:
		{	
			IsProcessRun(str,L"qqplayer.exe",bIsRun);
		}
		break;
	case 9012:
		{	
			IsProcessRun(str,L"qqlive.exe",bIsRun);
		}
		break;
	case 9014:
		{	
			IsProcessRun(str,L"xmp.exe",bIsRun);

		}
		break;
	case 9015:
		{	
			IsProcessRun(str,L"thunder.exe",bIsRun);
		}
		break;
	case 9016:
		{	


		}
		break;
	case 9017:
		{	


		}
		break;
	case 9018:
		{	

			IsProcessRun(str,L"winword.exe",bIsRun);
		}
		break;
	case 9019:
		{	
			IsProcessRun(str,L"excel.exe",bIsRun);

		}
		break;
	case 9020:
		{	
			IsProcessRun(str,L"powerpoint.exe",bIsRun);

		}
		break;
	case 9021:
		{	
			IsProcessRun(str,L"msaccess.exe",bIsRun);

		}
		break;
	case 9022:
		{	
			IsProcessRun(str,L"frontpage.exe",bIsRun);

		}
		break;
	case 9023:
		{	

			IsProcessRun(str,L"groove.exe",bIsRun);
		}
		break;
	case 9024:
		{	
			IsProcessRun(str,L"mspub.exe",bIsRun);

		}
		break;
	case 9025:
		{	
			IsProcessRun(str,L"visio.exe",bIsRun);

		}
		break;
	case 9026:
		{	

			IsProcessRun(str,L"wps.exe",bIsRun);
		}
		break;
	case 9027:
		{	
			IsProcessRun(str,L"et.exe",bIsRun);

		}
		break;
	case 9028:
		{	
			IsProcessRun(str,L"wpp.exe",bIsRun);

		}
		break;
	case 9029:
		{	
			IsProcessRun(str,L"ultraedit.exe",bIsRun);

		}
		break;
	case 9030:
		{	
			IsProcessRun(str,L"editplus.exe",bIsRun);

		}
		break;
	case 9031:
		{	

			IsProcessRun(str,L"winrar.exe",bIsRun);
		}
		break;
	case 9032:
		{	
			IsProcessRun(str,L"winzip32.exe",bIsRun);

		}
		break;
	case 9033:
		{	
			IsProcessRun(str,L"7zg.exe",bIsRun);

		}
		break;
	}
	
	if(bIsRun)
	{
		g_fnScanReg(g_pMain,BEGINPROC(iType),0,0,0);
		g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);
		g_fnScanReg(g_pMain,ENDPROC(iType),str.c_str(),0,0);
		g_fnScanFile(g_pMain,ENDPROC(iType),str.c_str(),0,0);

		return TRUE;
	}

	return FALSE;
}

void AppHistory(int iType,CAppHistory& pAppHistory,CSpecialApp& pSpecialApp,CAtlMap<int,SOFTWAREINFO>& map_softinfo)
{	
	#ifdef _DEBUG
		CString strTest;
		strTest.Format(_T("\n应用程序分流%d\n"),iType);
		OutputDebugString(strTest);
	#endif
		
		
		if(RunProcessExit(iType))
			return;
	

	if (NULL != map_softinfo.Lookup(iType))
	{	

		//实际处理
		BOOL bIsReg = FALSE;
		BOOL bIsFile = FALSE;
		BOOL bIsRun = FALSE;
		std::wstring str = L"";
		
		for (int i=0;
				i<map_softinfo.Lookup(iType)->m_value.softInfo.GetSize();
				i++
			)
		{
			
			SoftInfo*	pSoftInfo = &map_softinfo.Lookup(iType)->m_value.softInfo[i];

			if (pSoftInfo->strmethod == _T("delreg"))
			{	
				CSimpleArray<CString> vec_regValue;
				
				if( _T("") != pSoftInfo->strkeyname )
					vec_regValue.Add(pSoftInfo->strkeyname);
				
				if (bIsReg ==FALSE)
				{
					g_fnScanReg(g_pMain,BEGINPROC(iType),0,0,0);
					bIsReg =TRUE;
				}

				pAppHistory.CommfunReg(iType,pSoftInfo->strpath,vec_regValue);
		

			}
			else if (pSoftInfo->strmethod == _T("delfile"))
			{	
				CSimpleArray<CString> vec_filePath;
				
				CString strPath = pSoftInfo->strpath;

				if (bIsFile ==FALSE)
				{
					g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);
					bIsFile =TRUE;
				}

				if (_T("all") == pSoftInfo->strpara)
				{
					
				}
				else if (_T("files") == pSoftInfo->strpara)
				{

				}
				else if (_T("file") == pSoftInfo->strpara)
				{	
					int iCount = strPath.ReverseFind('\\');
					if (iCount>0)
					{
						CString strTmp = strPath.Mid(0,iCount);
						CString strTmp2 =strPath.Mid(iCount+1);

						vec_filePath.Add(strTmp2);
						strPath = strTmp;

					}
						


				}
				else if (_T("match") == pSoftInfo->strpara)
				{	
					int iCount = strPath.ReverseFind('\\');
					if (iCount>0)
					{
						CString strTmp = strPath.Mid(0,iCount);
						CString strTmp2 =strPath.Mid(iCount+1);

						vec_filePath.Add(strTmp2);
						strPath = strTmp;

					}
				}
				
				if (bIsFile == FALSE)
				{
				#ifdef _DEBUG
					CString strTest;
					strTest.Format(_T("\n%d 开始\n"),iType);
					OutputDebugString(strTest);
				#endif
					bIsFile =TRUE;
				}
				pAppHistory.CommfunFile(iType,strPath,vec_filePath);
				
			}
			else if (pSoftInfo->strmethod == _T("func"))
			{	

				switch (iType)
				{
				case 9008:
					{	
						if (bIsFile == FALSE)
						{
							g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);

							bIsFile = TRUE;
						}
						
						pSpecialApp.CleanStorm(iType,pSoftInfo->strpath);

					}
					break;
				case 9009:
					{	
						if (bIsFile == FALSE)
						{
							g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);
							bIsFile = TRUE;
						}
						
						
// 						std::vector<std::wstring>::iterator it;
// 						for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
// 						{
// 							str = *it;
// 							transform(str.begin(), str.end(), str.begin(), towlower);
// 							if (str == L"kmplayer.exe")
// 							{
// 								str = L"正在运行，跳过";
// 								bIsRun = TRUE;
// 								goto _exit_;
// 							}
// 						}

						if(!bIsRun)
							pSpecialApp.CleanKMPlayer(iType,NULL,NULL);
			
					}
					break;
				case 9010:
					{

						if (bIsFile == FALSE)
						{
							g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);
							bIsFile = TRUE;
						}
						
						pSpecialApp.CleanKuGoo(iType,pSoftInfo->strpath);

					}
					break;
				case 9012:
					{	
						if (bIsFile == FALSE)
						{
							g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);
							bIsFile = TRUE;
						}

						pSpecialApp.CleanQQLive(iType,pSoftInfo->strpath);
					}
					break;
				case 9029:
					{	
						if (bIsFile == FALSE)
						{
							g_fnScanFile(g_pMain,BEGINPROC(iType),0,0,0);
							bIsFile = TRUE;
						}
						pSpecialApp.CleanUltraEdit_Scan(iType,pSoftInfo->strpath);
					}
					break;

				}

				OutputDebugString(_T("未处理fun\n"));
			}
			else
			{
				OutputDebugString(_T("未处理其他\n"));
			}
			
		}

		if (bIsFile ==TRUE)
		{	

#ifdef _DEBUG
			CString strTest;
			strTest.Format(_T("\n%d 结束\n"),iType);
			OutputDebugString(strTest);
#endif
			if(bIsRun)
				g_fnScanFile(g_pMain,ENDPROC(iType),str.c_str(),0,0);
			else
				g_fnScanFile(g_pMain,ENDPROC(iType),0,0,0);
		}

		if (bIsReg ==TRUE)
		{

			if(bIsRun)
				g_fnScanReg(g_pMain,ENDPROC(iType),str.c_str(),0,0);
			else
				g_fnScanReg(g_pMain,ENDPROC(iType),0,0,0);

#ifdef _DEBUG
			CString strTest;
			strTest.Format(_T("\n%d 结束\n"),iType);
			OutputDebugString(strTest);
#endif

		}

	
	}
	else
	{
		OutputDebugString(_T("无此应用程序\n"));
	}
	


}

#endif


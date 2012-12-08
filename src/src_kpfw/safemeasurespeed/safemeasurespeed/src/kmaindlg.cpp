#include "stdafx.h"
#include "kmaindlg.h"
#include "winsock2.h" 
#include "iptypes.h"
#include "iphlpapi.h"
#include "beikesafemsgbox.h"
#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "Version.lib")


static const wchar_t* g_szShowUnit[9] = {L"B", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB"};

KMainDlg::~KMainDlg()
{
}

void KMainDlg::BtnClose( void )
{
    EndDialog(0);
}

void KMainDlg::_GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet)
{
	BOOL bRet = FALSE;
	UINT uCount = 0;
	UINT uRetSize = MAX_PATH - 1;
	LPWSTR lpszValue = NULL;

	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	bRet = ::VerQueryValue(pbyInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &uCount);
	if (!bRet || uCount == 0)
		return;

	CString strSubBlock;

	strSubBlock.Format(
		L"\\StringFileInfo\\%04x%04x\\%s",
		lpTranslate[0].wLanguage,
		lpTranslate[0].wCodePage, 
		lpszFileInfoKey
		);
	bRet = ::VerQueryValue(
		pbyInfo, (LPWSTR)(LPCWSTR)strSubBlock, 
		(LPVOID *)&lpszValue, &uRetSize
		);
	if (!bRet)
		return;

	wcsncpy(lpszFileInfoRet, lpszValue, wcslen(lpszValue) + 1);
}

BOOL KMainDlg::_GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion)
{
	BOOL bResult = FALSE;
	BOOL bRet = FALSE;
	DWORD dwHandle          = 0;
	DWORD dwFileInfoSize    = 0;
	VS_FIXEDFILEINFO *pFixFileInfo = NULL;
	BYTE *pbyInfo           = NULL;

	dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
	if (0 == dwFileInfoSize)
		goto Exit0;

	pbyInfo = new BYTE[dwFileInfoSize];
	if (!pbyInfo)
		goto Exit0;

	bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
	if (!bRet)
		goto Exit0;

	_GetFileInfo(pbyInfo, L"ProductVersion", strProductVersion.GetBuffer(MAX_PATH + 1));
	strProductVersion.ReleaseBuffer();

	bResult = TRUE;

Exit0:

	if (pbyInfo)
	{
		delete[] pbyInfo;
		pbyInfo = NULL;
	}

	return bResult;
}

CString KMainDlg::GetVersionType(void)
{
	WCHAR bufPath[MAX_PATH] = {0};
	DWORD dwRet = ::GetModuleFileName(NULL, bufPath, MAX_PATH);
	if (0 == dwRet)
		return _T("");

	::PathRemoveFileSpecW(bufPath);
	::PathAppend(bufPath, TEXT("ksafever.dll"));

	CString strAppVersion;
	_GetPEProductVersion(bufPath, strAppVersion);


	return strAppVersion;
}

BOOL KMainDlg::OnInitDialog( CWindow /*wndFocus*/, LPARAM /*lInitParam*/ )
{
	SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));
	SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_SMALL)), FALSE);


	m_bIsFinished = FALSE;
	int nRet = m_DownFile.GetDownSpeed(this);

    return TRUE;
}

void KMainDlg::OnStatusCallback( ProcessState enumState, int64 nSpeed, int64 nAvgSpeed, int nProcess )
{
	if (m_bIsFinished)
		return;

	m_fAvgSpeed = nAvgSpeed;

	if (enumState == ProcessState_Finished)
	{
		nProcess = 100;
		m_fAvgSpeed = nAvgSpeed;
	}
	if (enumState == ProcessState_Failed)
	{
		nProcess = 0;
		m_fAvgSpeed = 0;
	}
	
	_SetDownSpeed(nSpeed);
	//_SetCurProcess(nProcess);

	if (nProcess > 0 && m_dwBegin == 0)
		m_dwBegin = _time32(NULL);

	DWORD dwEnd = _time32(NULL);

/*	if (dwEnd - m_dwBegin > 12 && m_dwBegin > 0)
	{
		m_bIsFinished = TRUE;
		PostMessage(WM_FINISHMEASURESPEED);
	}
	else if (m_dwBegin > 0)
	{
	//	_SetCurProcess((dwEnd - m_dwBegin)* 100/12);
	//	int* pPocess = new int;
	//	*pPocess = ((dwEnd - m_dwBegin)* 100/12);
	//	WPARAM pParam = pPocess;
	//	PostMessage(WM_FINISHMEASUREPROCESS, (WPARAM)pPocess);
	}
**/	
	if (enumState == ProcessState_Finished || enumState == ProcessState_Failed)
	{
		KLocker locker(m_pLocker);
		if (m_bIsFinished)
			return;

		m_bIsFinished = TRUE;
		PostMessage(WM_FINISHMEASURESPEED);
	}

	return;
	
}

void KMainDlg::_SetDownSpeed(IN int nSpeed)
{
	CString strSpeed = _GetNeedShowData(nSpeed/1024.0);
	SetItemText(DEFSHOWCURSPEED, L"");
	FormatItemText(DEFSHOWCURSPEED, BkString::Get(DefString2), strSpeed);
}

void KMainDlg::_SetCurProcess(IN int nCurProcess)
{

// 	for (int i = m_nCurSpeed + 1; i <= nCurProcess; i+= 5)
// 	{
// 		SetItemIntAttribute(DEFPROCESSCHECKSPEED, "value", i);
// 		Sleep(10);
// 	}

	SetItemIntAttribute(DEFPROCESSCHECKSPEED, "value", nCurProcess);

//	m_nCurSpeed = nCurProcess;
}

CString KMainDlg::_GetNeedShowDataAvg(IN double nData)
{
	int nUnitCur	= 1;
	CString strRet	= _T("0KB");

	while (TRUE)
	{
		if (nData > 1024)
		{
			nData /= 1024;
			nUnitCur ++;
		}
		else
			break;
	}

	if (nUnitCur < 9)
	{
		int nDataT = (int)(nData * 10);
		WCHAR szRet[20] = {0};
		if (nUnitCur == 1 && nData >= 10)
			swprintf_s(szRet, 20, L"%5ld", nDataT/10);
		else
			swprintf_s(szRet, 20, L"%5.1lf", nDataT/10.0);

		strRet = szRet;
		strRet.Append(g_szShowUnit[nUnitCur]);
	}

	return strRet;
}

CString KMainDlg::_GetNeedShowData(IN double fData)
{
	int nUnitCur	= 1;
	CString strRet	= _T("0KB");

	while (TRUE)
	{
		if (fData > 1024)
		{
			fData /= 1024;
			nUnitCur ++;
		}
		else
			break;
	}

	if (nUnitCur < 9)
	{
		int nData = (int)(fData * 10);
		WCHAR szRet[20] = {0};
		swprintf_s(szRet, 20, L"%5.1lf", nData/10.0);
		strRet = szRet;
		strRet.Append(g_szShowUnit[nUnitCur]);
	}

	return strRet;
}

void KMainDlg::BtnCancel( void )
{
	m_DownFile.StopDown();

	EndDialog(0);
}

int KMainDlg::_GetProcessPos(IN int nAvgSpeed)
{
	float fRet = 0;
	float nTrueSpeed = nAvgSpeed * 8;

	if (nTrueSpeed <= 256)
		fRet = 0 + nTrueSpeed * 10 / 256 ;
	else if (nTrueSpeed <= 256 * 2)
		fRet = 10 + (nTrueSpeed - 256) * 10.0 / 256 ;
	else if (nTrueSpeed <= 256 * 4)
		fRet = 20 + (nTrueSpeed - 256 * 2) * 10.0 / (256 * 2) ;
	else if (nTrueSpeed <= 256 * 8)
		fRet = 30 + (nTrueSpeed - 256 * 4)* 10.0 / (256 * 4) ;
	else if (nTrueSpeed <= 256 * 16)
		fRet = 40 + (nTrueSpeed - 256 * 8)* 10.0 / (256 * 8) ;
	else if (nTrueSpeed <= 256 * 32)
		fRet = 50 + (nTrueSpeed - 256 * 16)* 10.0 / (256 * 16) ;
	else if (nTrueSpeed <= 256 * 64)
		fRet = 60 + (nTrueSpeed - 256 * 32)* 10.0 / (256 * 32) ;
	else if (nTrueSpeed > 256 * 64)
		fRet = 70;

//	if (fRet >= 30)
//		fRet ++;

	return (int)(fRet + 0.5);
}


void KMainDlg::_GetSpeedType(IN int nAvgSpeed, CString& strBetween, CString& strRet, CString& strInfo)
{
	float fTrueSpeed = nAvgSpeed * 8;
	CString strSpeed = _GetNeedShowData(fTrueSpeed);
	strInfo = BkString::Get(DefString8);
	strBetween = _T("接近于 ");


	if (fTrueSpeed <= 256)
		strInfo = BkString::Get(DefString5);
	else if (fTrueSpeed <= 512)
		strInfo = BkString::Get(DefString6);
	else if (fTrueSpeed <= 1024)
	{
		strInfo = BkString::Get(DefString7);
		strBetween.Format(BkString::Get(13), _T("512K ADSL"), _T("1M ADSL"));
	}
	else if (fTrueSpeed > 1024 && fTrueSpeed < 8 * 1024)
	{
		for (int i = 2; i < 9; i++)
		{
			if (fTrueSpeed < 1024 * i)
			{
				CString strFirst, strSeconed;
				strInfo = BkString::Get(DefString7);
				if (i > 2)
					strInfo = BkString::Get(DefString8);

				strFirst.Format(_T("%ldM ADSL"), i - 1);
				if (i == 8)
					strSeconed.Format(_T("专线"));
				else 
					strSeconed.Format(_T("%ldM ADSL"), i);
				strBetween.Format(BkString::Get(13), strFirst, strSeconed);
				break;
			}
		}
	}
	else if (fTrueSpeed > 1024 * 8)
		strBetween = _T("");



	float fSpeed = nAvgSpeed * 8.0 / 1024.0;
	int   nSpeed = (int)fSpeed;

	if (fabs(fSpeed - nSpeed) < 0.001) 
	{
		if (nSpeed >= 8)
			strBetween.Empty();
		else
			strBetween.Format(_T("属于"));
	}


	if (fTrueSpeed <= 256)
		strRet.Format(_T("拨号网络"));
	else if (fTrueSpeed <= 768)
		strRet.Format(_T("512K ADSL 宽带业务"));
	else if (fTrueSpeed <= 1536)
		strRet.Format(_T("1M ADSL 宽带"));
	else if (fTrueSpeed < 1024 * 8 - 512)
	{
		for (int i = 2; i < 8; i++)
		{
			if (fTrueSpeed <= 512 + i * 1024) 
			{
				strRet.Format(_T("%dM ADSL 宽带"), i);
				break;
			}
		}
	}
	else if (fTrueSpeed < 1024 * 16 || fTrueSpeed > 1024* 16)
		strRet.Format(_T("专线"));

}

CString	KMainDlg::_GetIpAddr(void)
{
	CString strIP = _T(""); 
	WSADATA wsaData; 
	char name[256]; 
	char *ip; 
	PHOSTENT hostinfo; 
	bool bRet=true; 
	if ( WSAStartup(MAKEWORD(2,0), &wsaData ) == 0 ) 
	{ 
		if( gethostname ( name, sizeof(name)) == 0) 
		{ 
			if((hostinfo = gethostbyname(name)) != NULL) //这些就是获得IP的函数 
			{ 
				ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list); 
				strIP = ip; 
			} 
		} 
		WSACleanup(); 
	} 

	return strIP;
}

CString KMainDlg::_GetMacAddr(void)
{
	int nError = 0;
	int nMacLen = 0;
	int nFind = 0;
	static char  s_szMac[512] = "";
	char	szTemp[MAX_PATH] = { 0 };
	ULONG len = 0;
	CStringA strDescription("");

	PIP_ADAPTER_INFO pinfo = NULL;
	PIP_ADAPTER_INFO pTempInfo = NULL;

	pinfo = new IP_ADAPTER_INFO;
	if (pinfo == NULL)
	{
		goto Exit0;
	}
	len = sizeof(IP_ADAPTER_INFO);
	ZeroMemory(pinfo, len);
	nError = GetAdaptersInfo(pinfo,&len); 

	if (nError == ERROR_BUFFER_OVERFLOW) 
	{ 
		delete pinfo;
		pinfo = NULL;
		pinfo= new IP_ADAPTER_INFO[len / sizeof(IP_ADAPTER_INFO) + 1]; 
		if (pinfo == NULL)
		{
			goto Exit0;
		}
		ZeroMemory(pinfo, len);
		nError = GetAdaptersInfo(pinfo,&len); 
	} 
	pTempInfo = pinfo;

	if (nError == 0 && pinfo != NULL) 
	{ 

		do
		{
			strDescription = pinfo->Description;
			strDescription.MakeLower();
			nFind = strDescription.Find("vmware");
			if (nFind == -1)
			{
				sprintf(szTemp,
					"%02X:%02X:%02X:%02X:%02X:%02X", 
					pinfo->Address[0], 
					pinfo->Address[1], 
					pinfo->Address[2], 
					pinfo->Address[3], 
					pinfo->Address[4], 
					pinfo->Address[5]
				); 
				nMacLen += strlen(szTemp) + 1;
				if (nMacLen < 512)
				{
					if (*s_szMac == NULL)
					{
						strcpy(s_szMac, szTemp);
						break;
					}
				}
				else
				{
					goto Exit0;
				}
			}
			pinfo = pinfo->Next;
		} 
		while(pinfo!= NULL); 
	} 

Exit0:
	if (pTempInfo != NULL) 
	{
		delete []pTempInfo; 
		pTempInfo = NULL;
	}

	CString strMac = s_szMac;
	return strMac;
}

CString	KMainDlg::_ParseRichText(IN const CString& strRichText)
{
	CString strRichTextTemp = strRichText;
	CString strResult = _T("");
	int nPos = -1;
	while (TRUE)
	{
		nPos = strRichTextTemp.Find('<');
		if (nPos >= 0)
			strResult.Append(strRichTextTemp.Left(nPos));
		else
		{
			strResult.Append(strRichTextTemp);
			break;
		}

		nPos = strRichTextTemp.Find('>');
		if (nPos >= 0)
			strRichTextTemp = strRichTextTemp.Mid(nPos + 1);
	}

	return strResult;
}

void KMainDlg::_ShowTipInfo(void)
{
	if (m_fAvgSpeed * 8 < 1024 * 2)
		SetItemVisible(DEFSHOWAWOKE, TRUE);
	else
	{
		SetItemVisible(DEFSHOWAWOKE, FALSE);
		return;
	}

	SetItemVisible(125, FALSE);
	SetItemVisible(126, FALSE);
	SetItemVisible(127, FALSE);

	if (m_fAvgSpeed * 8 <= 0)
		SetItemVisible(127, TRUE);
	else if (m_fAvgSpeed * 8 <= 512)
		SetItemVisible(125, TRUE);
	else if (m_fAvgSpeed * 8 < 1024 * 2)
		SetItemVisible(126, TRUE);
}

LRESULT KMainDlg::OnSetFinishDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	//完成最上面的页面
	_SetCurProcess(100);
	m_DownFile.StopDown();
	m_DownFile.WaitDown();
	
	m_fAvgSpeed /= 1024.0;

	if (m_fAvgSpeed < 0.1)
		m_fAvgSpeed = 0;


	_ShowFristPage(FALSE);
	CString strSpeed = _GetNeedShowDataAvg(m_fAvgSpeed);
	strSpeed.Trim();
	FormatItemText(DEFSHOWHEADRESULTTEXT, BkString::Get(DefString3), strSpeed);
	int nSpeed = _GetProcessPos(m_fAvgSpeed);
	SetItemIntAttribute(DEFPROCESSRESULTSHOW, "value", nSpeed);

	_ShowTipInfo();

	//完成中间部分的描叙
	CString strBewteenInfo, strMeasureRet, strMeasureInfo;
	if (m_fAvgSpeed > 0)
	{
		_GetSpeedType(m_fAvgSpeed, strBewteenInfo, strMeasureRet, strMeasureInfo);
		
		m_strClipboardText.Format(BkString::Get(14), strSpeed);
		SetItemText(DEFSHOWRESULTEXT, m_strClipboardText);
	//	FormatRichText(DEFSHOWRESULSPEEDINFO, BkString::Get(DefString4), strBewteenInfo, strMeasureRet);
		CString strSpeedText;
		if (m_fAvgSpeed >= 1024)
			strSpeedText.Format(BkString::Get(15), strBewteenInfo, strMeasureRet);
		else
			strSpeedText.Format(BkString::Get(DefString4), strBewteenInfo, strMeasureRet);
		SetRichText(DEFSHOWRESULSPEEDINFO, strSpeedText);
		strSpeedText = _ParseRichText(strSpeedText);
		m_strClipboardText.AppendFormat(_T("\r\n%s"), strSpeedText);

		SetItemText(DEFSHOWRESULTEXTINFO, strMeasureInfo);
		SetItemVisible(DEFSHOWMSPEEDFAIL, FALSE);
		SetItemVisible(DEFSHOWRESULTEXT, TRUE);
		SetItemVisible(DEFSHOWRESULSPEEDINFO, TRUE);
		SetItemVisible(113, FALSE);

		//写下面的小贴士相关的内容

	}
	else
	{
		SetItemText(DEFSHOWRESULTEXTINFO, FALSE);
		SetItemVisible(DEFSHOWRESULTEXT, FALSE);
		SetItemVisible(DEFSHOWMSPEEDFAIL, TRUE);
		SetItemVisible(DEFSHOWRESULSPEEDINFO, FALSE);
		SetItemVisible(113, TRUE);

	//	GetDlgItemText(DEFSHOWMSPEEDFAIL, m_strClipboardText);
		m_strClipboardText = GetItemText(DEFSHOWMSPEEDFAIL);
		
	}
	
// 	CString strIp = _GetIpAddr();
// 	CString strMac = _GetMacAddr();
// 	FormatItemText(DEFSHOWRESULTEXTIP, BkString::Get(DefString9), strIp);
// 	FormatItemText(DEFSHOWRESULTEXTMAX, BkString::Get(DefString10), strMac);

	//完成下面部分的描叙
	return TRUE;
}

void KMainDlg::_ShowFristPage(IN BOOL bShowFirstPage)
{
	_SetDownSpeed(0);
	SetItemIntAttribute(DEFPROCESSCHECKSPEED, "value", 0);

	SetItemVisible(DEFSHOWFIRSTPAGE, bShowFirstPage);
	if (m_fAvgSpeed > 0)
		SetItemVisible(DEFSHOWFIRSTPAGESUCCESS, !bShowFirstPage);
	else
		SetItemVisible(122, !bShowFirstPage);

	SetItemVisible(DEFSHOWDES, bShowFirstPage);
	SetItemVisible(DEFSHOWRESULTINFO, !bShowFirstPage);

	SetItemVisible(DEFSHOWFOOTCANCELDLG, bShowFirstPage);
	SetItemVisible(DEFSHOWTRYAGAIN, !bShowFirstPage);
}

void KMainDlg::BtnTryAgain( void )
{
	m_nCurTime		= 0;
	m_bIsFinished	= FALSE; 
	m_dwBegin		= 0;
	m_nCurSpeed		= 0;
	_ShowFristPage(TRUE);
	SetItemText(DEFSHOWCURSPEED, L"");
	SetItemText(DEFSHOWCURSPEED, BkString::Get(17));
	m_DownFile.GetDownSpeed(this);
}

void KMainDlg::BtnCloseDialog( void )
{
	EndDialog(0);
}


void KMainDlg::BtnCopyClipboard( void )
{
	if (!OpenClipboard())		return;
	
	if(!EmptyClipboard())		return;
	
	CString strBegin, strClipboard = _T("");
	strBegin = GetItemText(128);
	strClipboard = GetItemText(115);
	if (m_fAvgSpeed > 0)
		strClipboard.Format(_T("%s\r\n%s\r\n%s\r\n"), strBegin, m_strClipboardText, GetItemText(DEFSHOWRESULTEXTINFO));
	else
		strClipboard.Format(_T("%s\r\n%s\r\n%s\r\n"), strBegin, m_strClipboardText, GetItemText(113));
	strClipboard.AppendFormat(_T("%s"), BkString::Get(16));

	int nSize = strClipboard.GetLength() * 2 + 2;
	HGLOBAL   simpleData   =   GlobalAlloc(GPTR, nSize); 
	LPBYTE   lpSmpData   =   (LPBYTE)GlobalLock(simpleData); 
	memcpy(lpSmpData, strClipboard, nSize);

	::SetClipboardData( CF_UNICODETEXT, lpSmpData );  

	CloseClipboard();

	CBkSafeMsgBox2 dlg;
	dlg.ShowMutlLineMsg(BkString::Get(11), BkString::Get(12), MB_OK | MB_ICONEXCLAMATION);

}

void KMainDlg::BtnLnkOpenMonter( void )
{

	//打开监控
	WCHAR bufPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, bufPath, MAX_PATH);
	::PathRemoveFileSpecW(bufPath);
	::PathAppend(bufPath, TEXT("netmon.exe"));
	::ShellExecute(NULL, NULL, bufPath, NULL, NULL, SW_HIDE);
}

LRESULT KMainDlg::OnSetProcess( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int* pProcess = (int *)wParam;
	_SetCurProcess(* pProcess);

	if (pProcess != NULL)
	{
		delete pProcess;
		pProcess = NULL;
	}
	return S_OK;
}

void KMainDlg::OnSysCommand( UINT nID, CPoint pt )
{
	if ( nID == SC_CLOSE )
		BtnClose();
	else
		DefWindowProc();
}

LRESULT KMainDlg::OnCreate( LPCREATESTRUCT lp )
{
	SetTimer(1, 46);
	SetMsgHandled(false);

	return S_OK;
}

void KMainDlg::OnDestroy( void )
{
	KillTimer(1);
	SetMsgHandled(false);

}

void KMainDlg::OnTimer( UINT_PTR pUTimer )
{
	if ( 1 != pUTimer )
		SetMsgHandled(false);
	else
	{
		int nCurTime = (++m_nCurTime) / 4 ;
		if (nCurTime == 100)
		{
			KLocker locker(m_pLocker);
			if (m_bIsFinished)
				return;

			m_bIsFinished = TRUE;
			PostMessage(WM_FINISHMEASURESPEED);
			//OnSetFinishDown(0, NULL, NULL);
		}

		_SetCurProcess(nCurTime);
	}
}

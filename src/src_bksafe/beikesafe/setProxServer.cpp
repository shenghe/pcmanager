#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "setProxServer.h"

#include <iefix/cinifile.h>
using namespace IniFileOperate;

#define PROX_SERVER_TYPE_NONE			L"不使用"
#define PROX_SERVER_TYPE_IE				L"使用IE代理设置"
#define PROX_SERVER_TYPE_HTTP			L"HTTP代理"
#define PROX_SERVER_TYPE_SOCK			L"SOCKS5代理"
#define PROX_SERVER_VERIFY_NO			L"不需要验证"
#define PROX_SERVER_VERIFY_YES			L"需要验证"
#pragma comment(lib, "ws2_32.lib")



LRESULT CSetProxServer::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{		
	return 0;
}
BOOL CSetProxServer::OnInitDialog(HWND wParam, LPARAM lParam)
{
	_init();//初始化
	readCfg();//读配置文件
	m_bFlag = TRUE;
	return TRUE;
}	

LRESULT CSetProxServer::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_ComBoHttp.IsWindow())
	{
		m_ComBoHttp.DestroyWindow();
	}
	if (m_ComBoVerify.IsWindow())
	{
		m_ComBoVerify.DestroyWindow();
	}
	if (m_wndEditAddr.IsWindow())
	{
		m_wndEditAddr.DestroyWindow();
	}
	if (m_wndEditPort.IsWindow())
	{
		m_wndEditPort.DestroyWindow();
	}
	if (m_wndEditAccount.IsWindow())
	{
		m_wndEditAccount.DestroyWindow();
	}
	if (m_wndEditPass.IsWindow())
	{
		m_wndEditPass.DestroyWindow();
	}
	return 0;
}
void CSetProxServer::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
	return;
}
void CSetProxServer::OnBkBtnSure()//确定
{
	if (m_bChanged)
	{
		if (!doApplySetting())
		{
			return;
		}		
	}	
	EndDialog(IDOK);
	return;
}

int CSetProxServer::readInt(ATL::CString str)
{
	int nValue = 0;
// 	if (str.Compare(PROX_SERVER_TYPE_IE) == 0)
// 	{
// 		nValue = 1;
// 	}
	if (str.Compare(PROX_SERVER_TYPE_NONE) == 0)
	{
		nValue = 0;
	}
	else if (str.Compare(PROX_SERVER_TYPE_HTTP) == 0)
	{
		nValue = 1;
	}
	else if (str.Compare(PROX_SERVER_TYPE_SOCK) == 0)
	{
		nValue = 2;
	}
	else if (str.Compare(PROX_SERVER_VERIFY_NO) == 0)//验证过程
	{
		nValue = 0;
	}
	else if (str.Compare(PROX_SERVER_VERIFY_YES) == 0)
	{
		nValue = 1;
	}	
	else
	{
		nValue = -1;
	}
	return nValue;
}
BOOL CSetProxServer::CheckIp( ATL::CString& str )//检查IP地址是否合法
{
	LPCWSTR   lpIpString = str.GetBuffer(-1);
	str.ReleaseBuffer(-1);
	int      nStrLen = 0; 
	int      nDotCount = 0; 

	nStrLen = wcslen(lpIpString); 
	if((nStrLen > 15) || (nStrLen < 7)) 
		return FALSE; 

	for(int i=0; i < nStrLen; ++i) 
	{ 
		if( lpIpString[i] == L'.' ) 
		{ 
			if( (lpIpString[i+1] == '.') || (i==0) || (i==nStrLen-1))
				return FALSE; 
			nDotCount++; 
			continue; 
		} 

		if( (lpIpString[i] < L'0') || (lpIpString[i] > L'9') )
			return   FALSE; 
	} 

	if( nDotCount == 3) 
	{ 
		if(inet_addr(CW2A(str.GetBuffer(-1))) != INADDR_NONE) 
		{ 
			return TRUE; 
		} 
	} 

	return FALSE;
}

BOOL CSetProxServer::doApplySetting()
{
	//写配置文件	
	CIniFile IniFile(m_strFilePath);
	ATL::CString strHttp	= L"";
	int			 nType;
	ATL::CString strVerify	= L"";
	int			 nVerify;
	ATL::CString strAddr	= L"";
	ATL::CString strPort	= L"";
	int			 nPort;
	ATL::CString strAccount = L"";
	ATL::CString strPass	= L"";
	HRESULT hr;
	EM_KXE_NETPROXY_TYPE emType;
	int nIndex = m_ComBoHttp.GetCurSel();
	if ( nIndex!= -1)
	{
		int n = m_ComBoHttp.GetLBTextLen( nIndex );
		m_ComBoHttp.GetLBText( nIndex, strHttp.GetBuffer(n) );
		strHttp.ReleaseBuffer();
	}
	nIndex = m_ComBoVerify.GetCurSel();
	if (nIndex != -1)
	{
		int n = m_ComBoVerify.GetLBTextLen( nIndex );
		m_ComBoVerify.GetLBText( nIndex, strVerify.GetBuffer(n) );
		strVerify.ReleaseBuffer();
	}
	//获得地址
	m_wndEditAddr.GetWindowText(strAddr);
	//ip地址过滤
	if ((strAddr == L"" || !CheckIp(strAddr)) && m_ComBoHttp.GetCurSel() != 0)
	{
		CBkSafeMsgBox2::ShowMultLine( L"IP地址为空或者不合法，请您重新输入！", L"金山卫士",  MB_OK|MB_ICONWARNING);
		return FALSE;
	}
	
	m_wndEditPort.GetWindowText(strPort);
	//port检测
	if (strPort == L"" && m_ComBoHttp.GetCurSel() != 0)
	{
		CBkSafeMsgBox2::ShowMultLine( L"端口设置不能为空，请您重新输入！", L"金山卫士",  MB_OK|MB_ICONWARNING);
		return FALSE;
	}
	m_wndEditAccount.GetWindowText(strAccount);
	m_wndEditPass.GetWindowText(strPass);	
	nType   = readInt(strHttp);
	nVerify = readInt(strVerify);
	if (strPort == L"")
		nPort = -1;
	else
		nPort = _ttoi(strPort);
	//判断验证情况下  用户名和密码是否为空
	if (nVerify == 1)//需要验证的情况
	{
		if (strAccount == L"" || strPass == L"")
		{
			CBkSafeMsgBox2::ShowMultLine( L"用户名或者密码不能为空，请您重新输入！", L"金山卫士",  MB_OK|MB_ICONWARNING);
			return FALSE;
		}
	}
	//写配置
	if (CBkSafeMsgBox2::ShowMultLine( L"您确定要对您刚修改的配置进行保存吗？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING) == IDOK)
	{
		//向后台进行设置
		if (nType == 0)
			emType = EM_KXE_NETPROXY_TYPE_NOPROXY;
		if (nType == 1)
			emType = EM_KXE_NETPROXY_TYPE_HTTP;
		else if (nType == 2)
			emType = EM_KXE_NETPROXY_TYPE_SOCKET5;

		std::wstring sHost(strAddr);
		std::wstring sUser(strAccount);
		std::wstring sPass(strPass);
		KXE_CLOUD_PROXY_INFO ProxInfo;
		ProxInfo.emProxyType = emType;
		ProxInfo.strProxyHost = sHost;
		ProxInfo.dwProxyPort = nPort;
		ProxInfo.strProxyUsername = sUser;
		ProxInfo.strProxyPassword = sPass;
		hr = m_kwsScanner.SetCloudProxyInfo(ProxInfo);
		if (hr != 0)//设置失败
		{
			CBkSafeMsgBox2::ShowMultLine( L"保存失败，请您重新设置？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING);
			return FALSE;  
		}
		//后台设置成功后  再写配置文件
		IniFile.SetIntValue(TEXT("proxy"),TEXT("type"),		nType);
		IniFile.SetStrValue(TEXT("proxy"),TEXT("host"),		strAddr);
		IniFile.SetIntValue(TEXT("proxy"),TEXT("port"),		nPort);
		IniFile.SetIntValue(TEXT("proxy"),TEXT("validate"), nVerify);
		IniFile.SetStrValue(TEXT("proxy"),TEXT("user"),		strAccount);
		IniFile.SetStrValue(TEXT("proxy"),TEXT("password"), strPass);
		NotifyDownModuleProxyChange( nType, strAddr, nPort, (BOOL)nVerify, strAccount, strPass );
		GetParent().PostMessage( MSG_PROXY_CHANGE, NULL, NULL );
	}
	else
	{
		return FALSE;
	}
	
	return TRUE;
}

void CSetProxServer::NotifyDownModuleProxyChange(int type, LPCWSTR pHost, int port, BOOL validate, LPCWSTR pUser, LPCWSTR pPwd)
{
	CString strIniPath;
	CAppPath::Instance().GetLeidianCfgPath(strIniPath);
	strIniPath += _T("\\pubconfig.ini");

	if(type != 1)
	{
		::DeleteFileW(strIniPath);
		return;
	}

	::WritePrivateProfileStringW(L"Proxy", L"Mode", L"2", strIniPath);
	::WritePrivateProfileStringW(L"Proxy", L"Host", pHost, strIniPath);

	static const int BUF_SIZE = 64;
	wchar_t szBuffer[BUF_SIZE];

	swprintf_s(szBuffer, BUF_SIZE, L"%d", port);
	::WritePrivateProfileStringW(L"Proxy", L"Port", szBuffer, strIniPath);

	swprintf_s(szBuffer, BUF_SIZE, L"%d", validate);
	::WritePrivateProfileStringW(L"Proxy", L"Needauth", szBuffer, strIniPath);
	if(validate)
	{
		::WritePrivateProfileStringW(L"Proxy", L"Login", pUser, strIniPath);
		::WritePrivateProfileStringW(L"Proxy", L"Password", pPwd, strIniPath);
	}
}

void CSetProxServer::_init()
{
	if (m_ComBoHttp.IsWindow() == FALSE)
	{
		m_ComBoHttp.Create( GetViewHWND(), NULL, _T(""), 
			WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST , 0, IDC_COMBO_PROX_SERVER_HTTP);
		m_ComBoHttp.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_ComBoHttp.AddString(PROX_SERVER_TYPE_NONE);
		//m_ComBoHttp.AddString(PROX_SERVER_TYPE_IE);
		m_ComBoHttp.AddString(PROX_SERVER_TYPE_HTTP);
		m_ComBoHttp.AddString(PROX_SERVER_TYPE_SOCK);//??		
	}
	
	if (m_ComBoVerify.IsWindow() == FALSE)
	{
		m_ComBoVerify.Create( GetViewHWND(), NULL, _T(""), 
			WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 0, IDC_COMBO_PROX_SERVER_VERIFY);
		m_ComBoVerify.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_ComBoVerify.AddString(PROX_SERVER_VERIFY_NO);	
		m_ComBoVerify.AddString(PROX_SERVER_VERIFY_YES);			
	}	

	m_wndEditAddr.Create( GetViewHWND(), 
		NULL,
		NULL,
		WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL,
		0,
		IDC_EDITBOX_PROX_SERVER_ADDR);//地址
	m_wndEditAddr.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	m_wndEditPort.Create( GetViewHWND(), 
		NULL,
		NULL,
		WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL,
		0,
		IDC_EDITBOX_PROX_SERVER_PORT);//端口
	m_wndEditPort.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	m_wndEditAccount.Create( GetViewHWND(), 
		NULL,
		NULL,
		WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL,
		0,
		IDC_EDITBOX_PROX_SERVER_ACCOUNT);//账号
	m_wndEditAccount.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	m_wndEditPass.Create( GetViewHWND(), 
		NULL,
		NULL,
		WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL | ES_PASSWORD,
		0,
		IDC_EDITBOX_PROX_SERVER_PASS);//密码
	m_wndEditPass.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	ReSet();
	return;
}	
void CSetProxServer::ReSet()//复位
{	
	m_ComBoVerify.SetCurSel(-1);
	m_wndEditAddr.SetWindowText( TEXT("") );
	m_wndEditPort.SetWindowText( TEXT("") );
	m_wndEditAccount.SetWindowText( TEXT("") );
	m_wndEditPass.SetWindowText( TEXT("") );
	return;
}
LRESULT CSetProxServer::OnComboChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled)//combo的改变
{
	m_bChanged = TRUE;//已经改变
	int nIndex = m_ComBoHttp.GetCurSel();
	ReadSettingsAndSet(nIndex);
	return TRUE;
}
LRESULT CSetProxServer::OnComboVeriChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled)//验证的框
{
	m_bChanged = TRUE;//已经改变
	HWND hWndAccount  = ::GetDlgItem(GetViewHWND(), IDC_EDITBOX_PROX_SERVER_ACCOUNT);
	HWND hWndPass     = ::GetDlgItem(GetViewHWND(), IDC_EDITBOX_PROX_SERVER_PASS);
	::EnableWindow(hWndAccount, m_ComBoVerify.GetCurSel());
	::EnableWindow(hWndPass,	m_ComBoVerify.GetCurSel());
	if (m_ComBoVerify.GetCurSel() <= 0)
	{
		m_wndEditAccount.SetWindowText(L"");
		m_wndEditPass.SetWindowText(L"");
	}
	return TRUE;
}
LRESULT CSetProxServer::OnEditChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_bFlag)
	{
		m_bChanged = TRUE;//已经改变
	}
	
	return TRUE;
}

void CSetProxServer::ReadSettingsAndSet(int nFlag = -1)
{
	int				nType;
	TCHAR			sUser[256] = {0};
	TCHAR			sPass[256] = {0};
	TCHAR			sHost[256] = {0};		
	int				nPort = 80;
	int				nValidate;	
	ATL::CString	strPort = L"";
	CIniFile iniFile(m_strFilePath);
	iniFile.GetIntValue(L"proxy",	L"type",		nType);

	if (nFlag == -1)//
	{
		if (nType == -1)//第一次使用的情况
		{
			nType = 1;
			m_bChanged = TRUE;
		}		
	}	
	else
	{
		nType = nFlag;
	}	
	m_ComBoHttp.SetCurSel(nType);
	
	iniFile.GetIntValue(L"proxy",	L"validate",	nValidate);
	if (nValidate == -1)
	{
		nValidate = 1;//默认情况下是需要验证的
	}
	iniFile.GetStrValue(L"proxy",	L"host",		sHost,	sizeof(sHost));
	iniFile.GetIntValue(L"proxy",	L"port",		nPort);			
	iniFile.GetStrValue(L"proxy",	L"user",		sUser,	sizeof(sUser));
	iniFile.GetStrValue(L"proxy",	L"password",	sPass,	sizeof(sPass));
	
	HWND hWndVerify   = ::GetDlgItem(GetViewHWND(), IDC_COMBO_PROX_SERVER_VERIFY);
	HWND hWndAddr     = ::GetDlgItem(GetViewHWND(), IDC_EDITBOX_PROX_SERVER_ADDR);
	HWND hWndPort     = ::GetDlgItem(GetViewHWND(), IDC_EDITBOX_PROX_SERVER_PORT);
	HWND hWndAccount  = ::GetDlgItem(GetViewHWND(), IDC_EDITBOX_PROX_SERVER_ACCOUNT);
	HWND hWndPass     = ::GetDlgItem(GetViewHWND(), IDC_EDITBOX_PROX_SERVER_PASS);
	if (nType == 0)//不使用的情况
 	{		
		::EnableWindow(hWndVerify,  FALSE);
		::EnableWindow(hWndAddr,	FALSE);
		::EnableWindow(hWndPort,	FALSE);
		::EnableWindow(hWndAccount, FALSE);
		::EnableWindow(hWndPass,	FALSE);
		ReSet();
	}
	else
	{		
		if (nPort == -1)
		{
			if (nType == 1)
				nPort = 80;
			else if (nType == 2)
				nPort = 1080;
		}
		strPort.Format(L"%d", nPort);
		::EnableWindow(hWndVerify,  TRUE);
		::EnableWindow(hWndAddr,	TRUE);
		::EnableWindow(hWndPort,	TRUE);
		::EnableWindow(hWndAccount, nValidate);
		::EnableWindow(hWndPass,	nValidate);
		if (nValidate <= 0)//不需要验证的时候
		{	
			memset(sUser, 0, sizeof(sUser));
			memset(sPass, 0, sizeof(sUser));
		}		
		m_ComBoVerify.SetCurSel(nValidate);
		m_wndEditAddr.SetWindowText(sHost);
		m_wndEditPort.SetWindowText(strPort);
		m_wndEditAccount.SetWindowText(sUser);
		m_wndEditPass.SetWindowText(sPass);
	}	
	return;
}

void CSetProxServer::readCfg()//读配置文件
{
	CAppPath::Instance().GetLeidianAppPath(m_strFilePath);
	m_strFilePath.Append(L"\\Cfg\\bksafe.ini");
	ReadSettingsAndSet();
	return;
}	
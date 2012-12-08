#pragma once
#define MSG_PROXY_CHANGE		(WM_APP+100)
//设置代理服务器
#include "kws/kwssanner.h"
class CSetProxServer 
	: public CBkDialogImpl<CSetProxServer>
	,public CWHRoundRectFrameHelper<CSetProxServer>
{
public:
	CSetProxServer()
		: CBkDialogImpl<CSetProxServer>(IDR_BK_PROX_SERVER_DLG)
	{
		m_bChanged = FALSE;
		m_bFlag = FALSE;
	}

	~CSetProxServer(void)
	{		
	}

	LRESULT		OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	BOOL		OnInitDialog(HWND wParam, LPARAM lParam);	
	LRESULT		OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void		OnBkBtnClose();
	void		OnBkBtnSure();	
	void		_init();
	void		ReSet();
	void		ReadSettingsAndSet(int nFlag);
	void		readCfg();//读配置文件
	LRESULT		OnEditChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled);
	BOOL		doApplySetting();
	int			readInt(ATL::CString str);
	LRESULT		OnComboChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT		OnComboVeriChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& bHandled);//verify 改变
	BOOL		CheckIp( ATL::CString& str );//
	void		NotifyDownModuleProxyChange(int type, LPCWSTR pHost, int port, BOOL validate, LPCWSTR pUser, LPCWSTR pPwd);
	
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN);
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,					OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PROX_SERVER_SURE,			OnBkBtnSure)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PROX_SERVER_CANCEL,		OnBkBtnClose)		
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CSetProxServer)	
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CSetProxServer>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CSetProxServer>)
		MESSAGE_HANDLER(WM_CREATE,				OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,				OnDestroy)	
		MSG_WM_INITDIALOG(OnInitDialog)	
		COMMAND_HANDLER( IDC_EDITBOX_PROX_SERVER_ADDR,		 EN_CHANGE,				OnEditChange)//edit框  进行了改动
		COMMAND_HANDLER( IDC_EDITBOX_PROX_SERVER_PORT,		 EN_CHANGE,				OnEditChange)
		COMMAND_HANDLER( IDC_EDITBOX_PROX_SERVER_ACCOUNT,	 EN_CHANGE,				OnEditChange)
		COMMAND_HANDLER( IDC_EDITBOX_PROX_SERVER_PASS,		 EN_CHANGE,				OnEditChange)
 		COMMAND_HANDLER( IDC_COMBO_PROX_SERVER_HTTP,		 CBN_SELCHANGE,			OnComboChange)//下拉框改变
 		COMMAND_HANDLER( IDC_COMBO_PROX_SERVER_VERIFY,		 CBN_SELCHANGE,			OnComboVeriChange)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()		 

private:
	CComboBox		m_ComBoHttp;
	CComboBox		m_ComBoVerify;
	CEdit			m_wndEditAddr;
	CEdit			m_wndEditPort;
	CEdit			m_wndEditAccount;
	CEdit			m_wndEditPass;
	ATL::CString	m_strFilePath;	
	BOOL			m_bChanged;
	BOOL			m_bFlag;
	KwsScanner		m_kwsScanner;	
};
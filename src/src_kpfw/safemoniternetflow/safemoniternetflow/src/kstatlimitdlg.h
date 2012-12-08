#pragma once
#include "bkres/bkres.h"
#include <wtlhelper/whwindow.h>
#include "bkwin/bklistbox.h"


#define		DEFCANCELMAINBUTTON			102
#define		DEFMORESEETING				108
#define		DEFNETMONITERC				120
#define		DEFSTATINFO					121
#define		DEFCHECKDAY					104
#define		DEFCHECKMONTH				105
#define		DEFINFOTEXT					106

#define		DEFRESULTMORESETTING		10
#define		DEFRESULTNETMOINTERCONTRAL	11
#define		DEFRESULTSTATINFO			12

#define		DEFMODEISDAY				1
#define		DEFMODEISMONTH				2

class KStatListDlg
	: public CBkDialogImpl<KStatListDlg>,
	public CWHRoundRectFrameHelper<KStatListDlg>
{

public:
	KStatListDlg()
		: CBkDialogImpl<KStatListDlg>(IDR_BK_STAT_REMAID)
	{
		m_nMode = 1;
		m_nSetSize = 0;
		m_bIsShowDlg = FALSE;
	}

	~KStatListDlg(){}

private:
	int				m_nMode; //1--表示日 2--表示月
	int				m_nSetSize; //设置的报警值
	BOOL			m_bIsShowDlg;

private:
	void			_SetConfigInfo(IN const CString& strTerm, IN const CString& strName, IN const CString& strValue);
	void			_SetShowInfo(void);

public:

	void			SetConfig(IN int nMode, IN int nSetSize);
	void			SetIsShowDlg(BOOL bIsShowDlg) { m_bIsShowDlg = bIsShowDlg; }
	BOOL			IsShowDlg(void) { return m_bIsShowDlg; }
	void			BtnClose(void);
	BOOL			OnInitDialog(CWindow wndFocus, LPARAM lInitParam);		

	//详细设置
	void			OnClickMoreSetting(void);
	//流量控制
	void			OnClickShowNetMointer(void);
	//流量详情
	void			OnClickShowStatResult(void);
	//检查日
	void			OnClickCheckDay(void);
	//检查月
	void			OnClickCheckMonth(void);

	void			OnSysCommand(UINT nID, CPoint pt);
	
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(DEFCANCELMAINBUTTON, BtnClose)

		BK_NOTIFY_ID_COMMAND(DEFMORESEETING, OnClickMoreSetting)
		BK_NOTIFY_ID_COMMAND(DEFNETMONITERC, OnClickShowNetMointer)
		BK_NOTIFY_ID_COMMAND(DEFSTATINFO, OnClickShowStatResult)
		BK_NOTIFY_ID_COMMAND(DEFCHECKDAY, OnClickCheckDay)
		BK_NOTIFY_ID_COMMAND(DEFCHECKMONTH, OnClickCheckMonth)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(KStatListDlg)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KStatListDlg>)
		CHAIN_MSG_MAP(CBkDialogImpl<KStatListDlg>)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

};
#pragma once
#include <wtlhelper/whwindow.h>
#include "BeikeVulfix.h"
#include "ViewSoftVulHandler.h"


template<typename T>
class CIconAnimate
{
public:
	CIconAnimate() : m_nTimer(0), m_uItemID(0)
	{ }

	void StartIconAnimate(int uItemID)
	{
		m_uItemID = uItemID;
		if(!m_nTimer)
		{
			T *dlg = static_cast<T*>(this);
			m_nTimer = dlg->SetTimer(TIMER_RUNNING, 200, NULL);
		}
	}

	void StopIconAnimate()
	{
		if(m_nTimer)
		{
			T *dlg = static_cast<T*>(this);
			dlg->KillTimer(TIMER_RUNNING);
			m_nTimer = 0;
		}
	}

	void OnTimer( UINT_PTR nIDEvent )
	{
		if(nIDEvent==TIMER_RUNNING && m_uItemID>0)
		{
			static int nSubImage = 0;
			nSubImage = ++nSubImage%8;

			T *dlg = static_cast<T*>(this);
			dlg->SetItemIntAttribute(m_uItemID, "sub", nSubImage);
		}
		return;
	}

private:
	UINT_PTR m_nTimer;
	int m_uItemID;
};


class CInnerView : public CBkRichViewImpl<CInnerView>, 	public CIconAnimate<CInnerView>
{
public:
	CViewSoftVulHandler m_viewSoftVul;

	CInnerView() : m_viewSoftVul(*this)
	{

	}
	void Init()
	{
		m_viewSoftVul.Init(m_hWnd);
	}

public:
	BEGIN_MSG_MAP_EX(CInnerView)
		//MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkRichViewImpl<CInnerView>)
		MSG_WM_TIMER(OnTimer)

		CHAIN_MSG_MAP_MEMBER(m_viewSoftVul)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};


class CDlgMain 
	: public CBkRichWinImpl<CDlgMain>
	, public CWHRoundRectFrameHelper<CDlgMain>
{
public:
	CDlgMain(void);
	~CDlgMain(void);
	
public:
	void RunBackGround();
	void ShutDownComputer(BOOL toReboot);
	void SetNoDisturb(BOOL bNoDisturb, BOOL bReparing, int uItemID=-1);
	
protected:
	void OnBkBtnClose();
	void OnBkBtnMax();
	void OnBkBtnMin();
	void OnBkBtnOption();
	
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);

	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if( nChar==_T('0') )
		{
		}
		else if(nChar==_T('1'))
		{
		}
	}
	void OnTimer(UINT_PTR nIDEvent);
	
	LRESULT OnScanDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRepaireDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	BOOL OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew);
	
	
protected:
	BOOL m_firstLoaded;
	int m_nCurrentTab;
	BOOL m_bNoDisturb;	// 正在执行操作,不能被切换TAB 
	BOOL m_bReparing;	// 正在修复
	
	CWHHtmlContainerWindow m_ieCtrl;	// 相关热点
	UINT_PTR m_nTimer;
	int m_nItemID;

	CInnerView m_viewMain;
	//CViewSoftVulHandler m_viewSoftVul;
	
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MAX, OnBkBtnMax)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MIN, OnBkBtnMin)
		BK_NOTIFY_ID_COMMAND(1003, OnBkBtnOption)

		BK_NOTIFY_TAB_SELCHANGE(IDBK_TAB_MAIN, OnBkTabMainSelChange)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgMain)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkRichWinImpl<CDlgMain>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgMain>)
		
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_CHAR(OnChar)
		MSG_WM_TIMER(OnTimer)
		
		CHAIN_MSG_MAP_MEMBER(m_viewMain.m_viewSoftVul)

		MESSAGE_HANDLER(WMH_SCAN_DONE, OnScanDone)
		MESSAGE_HANDLER(WMH_REPAIR_DONE, OnRepaireDone)	
		MESSAGE_RANGE_HANDLER(WMH_VULFIX_BASE, WMH_VULFIX_BASE+EVulfix_Task_Error, OnVulFixEventHandle)
		
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};

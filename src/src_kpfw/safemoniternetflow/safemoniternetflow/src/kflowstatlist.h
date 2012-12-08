#pragma once
#include "bkres/bkres.h"
#include <wtlhelper/whwindow.h>
#include "bkwin/bklistbox.h"
#include "kpfw/netstastic.h"
#include "kflowstat.h"


#define		DEFCANCELMAINBUTTON			102
#define		DEFLISTWINDOW				205

#define		DEF_LIST_BEGIN					212
#define		DEF_LIST_212					DEF_LIST_BEGIN+0
#define		DEF_LIST_213					DEF_LIST_BEGIN+1
#define		DEF_LIST_214					DEF_LIST_BEGIN+2
#define		DEF_LIST_215					DEF_LIST_BEGIN+3
#define		DEF_LIST_216					DEF_LIST_BEGIN+4
#define		DEF_LIST_217					DEF_LIST_BEGIN+5
#define		DEF_LIST_218					DEF_LIST_BEGIN+6

struct KFluxStasticProcItemEx
{
	KFluxStasticProcItem itemElem;
	CString	 m_strProcessName;
	void Init(void)
	{
		m_strProcessName = _T("");
	}
};

class KFlowStatList
	: public CBkDialogImpl<KFlowStatList>,
	public CWHRoundRectFrameHelper<KFlowStatList>
{

public:
	KFlowStatList()
		: CBkDialogImpl<KFlowStatList>(IDR_BK_STAT_PROCESSLIST)
	{
		m_pNetProcessList = NULL;
//		m_pFluxStatRead	  = NULL;
		m_pVFlowStatPList = NULL;
		m_nNeedShowCount  = 0;
	}

	~KFlowStatList(){}

public:
	LRESULT			OnTimer(UINT timerId);

	BOOL			OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	void			BtnClose(void);
	void			BtnCancel(void);
	void			GetRangleQuery(OUT CString& strFrom, OUT CString& strTo);

	BOOL			OnInitDialog(CWindow wndFocus, LPARAM lInitParam);		

	//œÏ”¶listœ˚œ¢
	LRESULT			OnListGetDispInfo(LPNMHDR pnmh);
	LRESULT			OnListGetmaxHeight(LPNMHDR pnmh);
	LRESULT			OnListGetItemHeight(LPNMHDR pnmh);

	void			SetInfoPointer(IN std::vector<KFluxStasticProcItemEx>* pFluxItem, IN char* pdefIcon, IN KFlowStat* pStat);


	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(DEFCLOSEMAINBUTTON, BtnClose)
		BK_NOTIFY_ID_COMMAND(DEFCANCELMAINBUTTON, BtnCancel)

		
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(KFlowStatList)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KFlowStatList>)
		CHAIN_MSG_MAP(CBkDialogImpl<KFlowStatList>)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		NOTIFY_HANDLER_EX(DEFLISTWINDOW, BKLBM_GET_DISPINFO, OnListGetDispInfo)
		NOTIFY_HANDLER_EX(DEFLISTWINDOW, BKLBM_CALC_MAX_HEIGHT, OnListGetmaxHeight)
		NOTIFY_HANDLER_EX(DEFLISTWINDOW, BKLBM_CALC_ITEM_HEIGHT, OnListGetItemHeight)

		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	void			_SetMessage(IN const CString& strAllFlow, IN const CString& strUpFlow, IN const CString& strDownFlow);
	CString			_GetNeedShowData( IN ULONGLONG uData );
	void			_SetCountData();
	BOOL			_GetCurLogInfo(OUT LONGLONG& uUpData, OUT LONGLONG& uDownData, OUT LONGLONG& uAllData);

private:
	char								m_defIcon[260];
	CBkNetMonitorListBox*				m_pNetProcessList;
//	KStasticFluxProcessList*			m_pFluxStatRead;
	int									m_nNeedShowCount;
	KFlowStat*							m_pFlowStatLog;

	std::vector<KFluxStasticProcItemEx>*	m_pVFlowStatPList;
};
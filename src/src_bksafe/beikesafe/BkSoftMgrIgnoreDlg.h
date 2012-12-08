#pragma  once
#include "bkwin/bklistbox.h"
#include "SoftMgrItemData.h"

class CBeikeSafeSoftmgrUIHandler;		// ¸¸´°¿Ú

class CBkIgnoreDlg
	: public CBkDialogImpl<CBkIgnoreDlg>
	, public CWHRoundRectFrameHelper<CBkIgnoreDlg>
{
public:
	CBkIgnoreDlg(CBeikeSafeSoftmgrUIHandler * pHandler)
		: CBkDialogImpl<CBkIgnoreDlg>(IDR_BK_IGNORE_DLG)
		, m_pHandler( pHandler )
	{
		m_necessList = NULL;
		m_datalistUpdate = NULL;
	}

	~CBkIgnoreDlg()
	{
		if (m_necessList)
		{
			delete m_necessList;
			m_necessList = NULL;
		}
	}

public:
	UINT_PTR DoModal(HWND hWndParent);

protected:
	CBeikeSafeSoftmgrUIHandler * m_pHandler;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnBkBtnClose();
	void OnIgnoreSelect();
	void OnIgnoreCheckAll();

	LRESULT OnListBoxGetDispInfo(LPNMHDR pnmh);
	LRESULT OnListBoxGetmaxHeight(LPNMHDR pnmh);
	LRESULT OnListBoxGetItemHeight(LPNMHDR pnmh);
	LRESULT OnBkListBoxClickCtrl(LPNMHDR pnmh);

	void NeedUpdateSoft(CSoftListItemData*	datalist);

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_CTL_UPDATE_IGNORE_SEL, OnIgnoreSelect)
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_CHECK_IGNORE_ALL, OnIgnoreCheckAll)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkIgnoreDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkIgnoreDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkIgnoreDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		NOTIFY_HANDLER_EX(IDC_CTL_UPDATE_IGNORE_WND, BKLBM_GET_DISPINFO, OnListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_CTL_UPDATE_IGNORE_WND, BKLBM_CALC_MAX_HEIGHT, OnListBoxGetmaxHeight)
		NOTIFY_HANDLER_EX(IDC_CTL_UPDATE_IGNORE_WND, BKLBM_CALC_ITEM_HEIGHT, OnListBoxGetItemHeight)
		NOTIFY_HANDLER_EX(IDC_CTL_UPDATE_IGNORE_WND, BKLBM_ITEMCLICK, OnBkListBoxClickCtrl)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:
	CBkListBox* m_necessList;
	CSimpleArray<CSoftListItemData*>	m_arrData;
	CSoftListItemData*	m_datalistUpdate;
};


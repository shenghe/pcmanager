#pragma once


#include <bkres/bkres.h>
#include <runoptimize/interface.h>
#include "SoftMgrListView.h"
#include "SoftMgrItemData.h"
#include "softmgrqueryedit.h"
#include "softmgrdetail.h"
#include "TypeListBox.h"
#include "bkwin/bklistbox.h"
#include <softmgr/ISoftManager.h>
#include <bksafe/bksafeconfig.h>
#include <softmgr\URLEncode.h>

class CBeikeSafeMainDlg;
class CBeikeSafeSoftmgrUIHandler;
// class CBkListBox;

struct NECESS_SOFT_LIST_DATA
{
	BOOL		bTitle;
	CString		strTitleName;
	CString		strSoftId;
};

class CBkSoftOneKeyDlg;

class CBeikeSafeSoftmgrNecessHandler : 
	 public IClickCallback
	,public IEditCallBack
{
public:
	friend class CBeikeSafeSoftmgrUIHandler;

	CBeikeSafeSoftmgrNecessHandler(CBeikeSafeMainDlg *pDialog, CBeikeSafeSoftmgrUIHandler* pSoftMgrUI)
	{
		m_bShowType = FALSE;
		m_pMainDlg			= pDialog;
		m_pSoftMgrMainUI	= pSoftMgrUI;
		m_bShowed		= FALSE;
		m_nLastSelType		= -1;
		m_pOnekeyDlg		= NULL;
	}

	~CBeikeSafeSoftmgrNecessHandler();

public:
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnTimer(UINT_PTR nIDEvent);	
	void OnSize(UINT nType, CSize size);

	void InitCtrl();
	void OnFirstShow();

	// 若果CSoftListItemData更新，根据ID来刷新列表
	VOID RefreshItemBySoftID(LPCTSTR lpstrID);


	// 
	VOID OnBtnQuerySoft();
	VOID OnBtnSearchBack();
	void OnClearQeuryEdit();

	void OnShowNoPlugSoft();
	void OnShowFreeSoft();

	void OnOneKeyInstall();

	LRESULT OnExitOneKeyDlg(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void GetSoftInfoByCondition();

	void BibeiChangeType(LPCTSTR lpstrType);

protected:
	LRESULT OnListBoxGetDispInfo(LPNMHDR pnmh);
	LRESULT OnListBoxGetmaxHeight(LPNMHDR pnmh);
	LRESULT OnListBoxGetItemHeight(LPNMHDR pnmh);
	LRESULT OnBkListBoxClickCtrl(LPNMHDR pnmh);
	LRESULT OnBkListBoxDbClick(LPNMHDR pnmh);
	LRESULT OnChangeQeuryEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

protected:
	NECESS_GROUP&	GetGroupByID(int nId);
	VOID			GenerateSoftList(int nId);

	VOID			GenerateId2DataMap();
	CSoftListItemData* GetSoftDataByID(LPCTSTR lpstrSoftID);
	CSoftListItemData* GetSoftDataByIDFromMap(LPCTSTR lpstrSoftID);

	DWORD			GetGroupSoftNumber( NECESS_GROUP& group );

	VOID			ResetRightListByLeft();

	VOID			EnterDetailPage(CSoftListItemData* pData);	// 查看详情页面

	VOID			OnBtnBackFromDetail();		// 详情页面返回
	VOID			OnBtnInstFromDetail();		// 详情页面安装

	VOID			OnFreeBack();
	VOID			OnBtnBtmAddSoft();		// 底部的提交软件
	VOID			OnBtnBtmViewDown();		// 底部的查看详情

	VOID			OnBtnDetailFreeback();	// 完善软件

	int				CloseCheck();

	void			OnReStart();
	void			OnIgnoreReStartTip();
	void			OnCloseReStartTip();
public:
	CSoftMgrQueryEdit	m_editQuery;

protected:
	BOOL		m_bShowType;
	CTypeListBox		m_leftList;
	CWHHtmlContainerWindow	m_IEDetail;	//ie显示详情页面
	CDetailSoftMgrDlg		m_IconDlg;		//显示软件图标
	CBkListBox*			m_necessList;
	BOOL				m_bShowed;
	DWORD				m_nLastSelType;
	
	CBeikeSafeMainDlg*	m_pMainDlg;
	CBeikeSafeSoftmgrUIHandler*	m_pSoftMgrMainUI;
	CSimpleArray<NECESS_GROUP>	m_arrNecess;
	CSimpleArray<ONE_KEY_GROUP>	m_arrOneKey;

	CAtlMap<CString, CSoftListItemData*> m_mapIdData;		// 记录装机必备的ID到结构体的map
	CSimpleArray<NECESS_SOFT_LIST_DATA>	m_arrRightList;
	CString								m_strViewDetailSoftId;	// 详情页面的soft ID
	CString								m_strKeyWord;

	CBkSoftOneKeyDlg*					m_pOnekeyDlg;
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_BTN_QUERY,OnBtnQuerySoft);
		BK_NOTIFY_ID_COMMAND(IDC_BACK_QUERY_DAQUAN_SOFTMGR_ICO,OnBtnSearchBack)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_BTN_SEARCH_BACK,OnBtnSearchBack)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_BACK,OnBtnBackFromDetail)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_INST,OnBtnInstFromDetail)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_BTM_BTN_ADD,OnBtnBtmAddSoft)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_LBL_RIGHT_DETAIL_ADD,OnFreeBack)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_LBL_VIEW_DOWN,OnBtnBtmViewDown)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_BIBEI_BTN_QEURY_EDIT, OnClearQeuryEdit)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT, OnShowNoPlugSoft)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_SHOW_FREE_SOFT, OnShowFreeSoft)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_ONE_KEY_INST,OnOneKeyInstall)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_LEFT_RESTART,OnReStart)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_LEFT_IGNORE_VER_TIP,OnIgnoreReStartTip)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_NECESS_LEFT_CLOSE_VER_TIP,OnCloseReStartTip)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeSoftmgrNecessHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		COMMAND_HANDLER(IDC_SOFT_NECESS_EDT_QUERY, EN_CHANGE, OnChangeQeuryEdit)
		MESSAGE_HANDLER_EX(WM_SOFT_ONEKEY_DLG_EXIT,OnExitOneKeyDlg)
		NOTIFY_HANDLER_EX(IDC_SOFTMGR_SOFTLIST_NECESS, BKLBM_GET_DISPINFO, OnListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_SOFTMGR_SOFTLIST_NECESS, BKLBM_CALC_MAX_HEIGHT, OnListBoxGetmaxHeight)
		NOTIFY_HANDLER_EX(IDC_SOFTMGR_SOFTLIST_NECESS, BKLBM_CALC_ITEM_HEIGHT, OnListBoxGetItemHeight)
		NOTIFY_HANDLER_EX(IDC_SOFTMGR_SOFTLIST_NECESS, BKLBM_ITEMCLICK, OnBkListBoxClickCtrl)
		NOTIFY_HANDLER_EX(IDC_SOFTMGR_SOFTLIST_NECESS, BKLBM_ITEMDBCLICK, OnBkListBoxDbClick)

	END_MSG_MAP()


public:
	virtual void OnClick( int nListId, CTypeListItemData * pData);
	virtual void OnEditEnterMsg( DWORD nEditId );
	virtual void OnEditMouseWheelMsg(WPARAM wParam, LPARAM lParam);
};

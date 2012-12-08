
#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafesoftmgrnecess.h"
#include "beikesafesoftmgrHeader.h"
#include "BkSoftMgrOneKeyDlg.h"
//#include <bkwin/bklistbox.h>


#define SOFT_LIST_TITLE_HEIGHT		25
#define SOFT_LIST_ITEM_HEIGHT		55

CBeikeSafeSoftmgrNecessHandler::~CBeikeSafeSoftmgrNecessHandler()
{
	if (m_pOnekeyDlg)
	{
		delete m_pOnekeyDlg;
		m_pOnekeyDlg = NULL;
	}

	if (m_necessList)
	{
		delete m_necessList;
		m_necessList = NULL;
	}
}

BOOL CBeikeSafeSoftmgrNecessHandler::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	if (m_pMainDlg->IsItemVisible(IDC_SOFT_NECESS_DIV_ALL,TRUE))
	{
		CRect		rcWin;
		if (m_leftList.IsWindow() && m_leftList.IsWindowVisible())
		{
			m_leftList.GetWindowRect(&rcWin);

			if(rcWin.PtInRect(pt))
				SendMessage(m_leftList.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		}

		if ( m_necessList && m_necessList->IsWindow() && m_necessList->IsWindowVisible() )
		{
			m_necessList->GetWindowRect(&rcWin);

			if (rcWin.PtInRect(pt))
				SendMessage(m_necessList->m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		}
		
		if ( m_IEDetail.IsWindow() && m_IEDetail.IsWindowVisible() )
		{
			m_IEDetail.GetWindowRect(&rcWin);

			if(rcWin.PtInRect(pt))
				SendMessage(m_IEDetail.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		}
	}

	SetMsgHandled(FALSE);
	return TRUE;		 
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnChangeQeuryEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CString strText;
	m_editQuery.GetWindowText(strText);
	if (strText.GetLength() != 0 && strText == BkString::Get(IDS_SOFTMGR_8016))
		m_pMainDlg->SetItemVisible(IDC_SOFT_BIBEI_BTN_QEURY_EDIT, FALSE);
	else if (strText.GetLength() == 0)
		m_pMainDlg->SetItemVisible(IDC_SOFT_BIBEI_BTN_QEURY_EDIT, FALSE);
	else
		m_pMainDlg->SetItemVisible(IDC_SOFT_BIBEI_BTN_QEURY_EDIT, TRUE);

	return TRUE;
}

void CBeikeSafeSoftmgrNecessHandler::OnClearQeuryEdit()
{
	CString strKey;
	m_editQuery.GetWindowText(strKey);
	if (strKey.GetLength() != 0)
	{
		OnBtnBackFromDetail();
		OnBtnSearchBack();
	}
}

void CBeikeSafeSoftmgrNecessHandler::OnShowNoPlugSoft()
{
	BOOL bCheck = m_pMainDlg->GetItemCheck(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT);
	BKSafeConfig::SetShowNoPlugSet( bCheck );

	GetSoftInfoByCondition();
}

void CBeikeSafeSoftmgrNecessHandler::OnShowFreeSoft()
{
	BOOL bCheck = m_pMainDlg->GetItemCheck(IDC_SOFT_NECESS_SHOW_FREE_SOFT);
	BKSafeConfig::SetShowFreeSoftSet( bCheck );

	GetSoftInfoByCondition();
}

void CBeikeSafeSoftmgrNecessHandler::GetSoftInfoByCondition()
{
	CString	strSearch;
	m_editQuery.GetWindowText(strSearch);
	if (strSearch == BkString::Get(IDS_SOFTMGR_8016) )
		ResetRightListByLeft();
	else
		OnBtnQuerySoft();
}

void CBeikeSafeSoftmgrNecessHandler::OnTimer( UINT_PTR nIDEvent )
{
	if( nIDEvent == TIMER_ID_NECESS_KEYWORD )
	{

		CString	strSearch;
		if( m_editQuery.IsWindow() )
			m_editQuery.GetWindowText(strSearch);


		m_pSoftMgrMainUI->m_pDlg->KillTimer( TIMER_ID_NECESS_KEYWORD );
	}

	SetMsgHandled(FALSE);
}

void CBeikeSafeSoftmgrNecessHandler::OnSize( UINT nType, CSize size )
{
	SetMsgHandled(FALSE);
}

void CBeikeSafeSoftmgrNecessHandler::BibeiChangeType(LPCTSTR lpstrType)
{
	m_leftList.SetCurSelTypeName(lpstrType, TRUE);
}

void CBeikeSafeSoftmgrNecessHandler::InitCtrl()
{
	m_leftList.Create(m_pMainDlg->GetViewHWND(), 
		NULL, NULL, 
		WS_CHILD | LBS_OWNERDRAWVARIABLE, 0, 
		IDC_SOFTMGR_TYPELIST_NECESS);
	m_leftList.SetListID(IDC_SOFTMGR_TYPELIST_NECESS);
	m_leftList.SetCallBack(this);

	m_editQuery.Create(m_pMainDlg->GetViewHWND(), 
		NULL, NULL,
		WS_CHILD | WS_TABSTOP, ES_NOHIDESEL | ES_AUTOHSCROLL, 
		IDC_SOFT_NECESS_EDT_QUERY);

	m_editQuery.SetFont((HFONT)m_pSoftMgrMainUI->m_font);
	m_editQuery.SetCallBack(this);
	m_editQuery.SetOnCharDelayTime();
	m_editQuery.SetDefaultString(BkString::Get(IDS_SOFTMGR_8016));
	m_editQuery.LimitText(20);

	m_necessList = new CBkListBox;
	m_necessList->Create( m_pMainDlg->GetViewHWND(), IDC_SOFTMGR_SOFTLIST_NECESS);
	m_necessList->Load(IDR_BK_SOFTMGR_NECESS_TEMPLATE);
	m_necessList->SetCanGetFocus(FALSE);

	BOOL bCheck = (BOOL)BKSafeConfig::GetShowFreeSoftSet();
	m_pMainDlg->SetItemCheck( IDC_SOFT_NECESS_SHOW_FREE_SOFT, bCheck);
	bCheck = (BOOL)BKSafeConfig::GetShowNoPlugSet();
	m_pMainDlg->SetItemCheck( IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT, bCheck);

	m_IconDlg.Create(m_pMainDlg->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, NULL, IDC_SOFT_NECESS_IMG_RIGHT_DETAIL_LOGO);
	m_IEDetail.Create(m_pMainDlg->GetViewHWND(), IDC_SOFT_NECESS_IE_RIGHT_DETAIL, FALSE, RGB(0xFB, 0xFC, 0xFD));
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnListBoxGetDispInfo( LPNMHDR pnmh )
{
	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;

	if ( pdi->nListItemID >= m_arrRightList.GetSize() )
		return 0;

	NECESS_SOFT_LIST_DATA&	datalist = m_arrRightList[pdi->nListItemID];

	if (datalist.bTitle)
	{
		// 标题的绘制

		pdi->nHeight = SOFT_LIST_TITLE_HEIGHT;

		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_TITLE,TRUE);
		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_ITEM,FALSE);

		m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_TITLE_NAME,datalist.strTitleName);
	}
	else
	{
		// 里面软件list的绘制

		pdi->nHeight = SOFT_LIST_ITEM_HEIGHT;
		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_TITLE,FALSE);
		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_ITEM,TRUE);

		m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_DIV_ITEM,"crbg",pdi->bSelect?"EBF5FF":"FFFFFF");

		CSoftListItemData* pSoftData = GetSoftDataByID(datalist.strSoftId);

		int nPosX = 0;
		Gdiplus::Image *pImage = NULL;
		{
			CDC dcx = GetDC(m_pMainDlg->m_hWnd);
			HFONT			hFntTmp;

			int nTypeWidth = 0;
			if (m_bShowType)
			{
				hFntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
				CRect rcType;
				CString strType;
				strType.Format(L"[%s]", pSoftData->m_strTypeShort);
				dcx.DrawText(strType, -1, &rcType, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				nTypeWidth = rcType.Width();
				dcx.SelectFont(hFntTmp);
			}

			hFntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_BOLDFONT));
			CRect rcProb;
			dcx.DrawText(pSoftData->m_strName, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			dcx.SelectFont(hFntTmp);

			ReleaseDC(m_pMainDlg->m_hWnd, dcx);

			CRect rcWin;
			GetWindowRect(m_necessList->m_hWnd, &rcWin);

			int nLablesWidth = 0;
			if ((pSoftData->m_attri&SA_Green) == SA_Green)
			{
				pImage = BkPngPool::Get(IDP_SOFTMGR_GREEN_SOFT);
				nLablesWidth += pImage->GetWidth();
			}
			if (pSoftData->m_bCharge == TRUE)
			{
				if (nLablesWidth != 0)
					nLablesWidth += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_CHARGE_SOFT);
				nLablesWidth += pImage->GetWidth();
			}
			if (pSoftData->m_bPlug == TRUE)
			{
				if (nLablesWidth != 0)
					nLablesWidth += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_PLUGIN_SOFT);
				nLablesWidth += pImage->GetWidth();
			}
			if ((pSoftData->m_attri&SA_New) == SA_New)
			{
				if (nLablesWidth != 0)
					nLablesWidth += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_NEW_SOFT);
				nLablesWidth += pImage->GetWidth();
			}

			int nLeft = 50 + nTypeWidth;
			nPosX = rcWin.Width() - 310 - nLablesWidth;
			if (rcProb.Width() < rcWin.Width() - 310 - nLablesWidth - nLeft)
				nPosX = nLeft + rcProb.Width();
		
			CStringA strPosA;
			strPosA.Format("%d,12,%d,27", nLeft, nPosX);
			m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_TXT_SOFT_TITLE, "pos", strPosA);

			if (m_bShowType)
			{
				CString strTypeShort;
				strTypeShort.Format(L"[%s]", pSoftData->m_strTypeShort);
				m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_TYPE, strTypeShort);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_TYPE,TRUE);
			}
			else
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_TYPE,FALSE);
		}

		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_GREEN,FALSE);
		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_CHARGE,FALSE);
		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_PLUGIN,FALSE);
		m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_NEW,FALSE);
		int nPosY = 10;
		if ((pSoftData->m_attri&SA_Green) == SA_Green)
		{
			nPosX += 2;
			CStringA strPosA;
			strPosA.Format("%d,%d", nPosX, nPosY);
			m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_GREEN, "pos", strPosA);
			m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_GREEN,TRUE);
			nPosX += 45;
		}
		if (pSoftData->m_bCharge == TRUE)
		{
			nPosX += 2;
			CStringA strPosA;
			strPosA.Format("%d,%d", nPosX, nPosY);
			m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_CHARGE, "pos", strPosA);
			m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_CHARGE,TRUE);
			nPosX += 45;
		}
		if (pSoftData->m_bPlug == TRUE)
		{
			nPosX += 2;
			CStringA strPosA;
			strPosA.Format("%d,%d", nPosX, nPosY);
			m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_PLUGIN, "pos", strPosA);
			m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_PLUGIN,TRUE);
			nPosX += 45;
		}
		if ((pSoftData->m_attri&SA_New) == SA_New)
		{
			nPosX += 2;
			CStringA strPosA;
			strPosA.Format("%d,%d", nPosX, nPosY);
			m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_NEW, "pos", strPosA);
			m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_NEW,TRUE);
		}

		if (pSoftData != NULL)
		{
			// 图标的绘制
			if (TRUE)
			{
				CStringA	strMem;
				strMem.Format("%d",pSoftData->m_pImage);
				m_necessList->SetItemAttribute( IDC_SOFTMGR_LISTTMP_ICON_SOFT, "mempointer",strMem );
			}
			
			//画评分
			for (int i = IDC_SOFTMGR_LISTTMP_STAR_ONE; i <= IDC_SOFTMGR_LISTTMP_STAR_FIVE; i++)
			{
				CStringA strSkin;
				if ((i - IDC_SOFTMGR_LISTTMP_STAR_ONE + 1)*2 <= pSoftData->m_fMark)
					strSkin = "star";
				else if ((i - IDC_SOFTMGR_LISTTMP_STAR_ONE + 1)*2 - 1 <= pSoftData->m_fMark)
					strSkin = "star_half";
				else
					strSkin = "star_off";
				
				m_necessList->SetItemAttribute(i, "skin", strSkin);
			}

			CString strMark;
			strMark.Format(L"%.1f分 投票", pSoftData->m_fMark);
			m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_MARK, strMark);

			// 下载图标
			if (!pSoftData->m_bIcon)
				m_pSoftMgrMainUI->OnDownLoadIcon( pSoftData->m_strSoftID );

			// 标题，描述，大小
			m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_TITLE, pSoftData->m_strName);
			m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_DESC, pSoftData->m_strDescription);
			m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_SIZE, pSoftData->m_strSize);
			m_necessList->SetItemStringAttribute(IDC_SOFTMGR_LISTTMP_TXT_SOFT_DESC,"tip",pSoftData->m_strDescription);
			m_necessList->SetItemStringAttribute(IDC_SOFTMGR_LISTTMP_TXT_SOFT_TITLE,"tip",BkString::Get(IDS_SOFTMGR_8100));
			m_necessList->SetItemStringAttribute(IDC_SOFTMGR_LISTTMP_ICON_SOFT,"tip",BkString::Get(IDS_SOFTMGR_8100));

			if (pSoftData->m_bUpdate)
			{
				CDC dcx = GetDC(m_pMainDlg->m_hWnd);
				HFONT			hFntTmp;
				hFntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

				CRect rcProb;
				dcx.DrawText(pSoftData->m_strDescription, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);

				dcx.SelectFont(hFntTmp);
				ReleaseDC(m_pMainDlg->m_hWnd, dcx);

				CRect rcWin;
				GetWindowRect(m_necessList->m_hWnd, &rcWin);

				int nPos = 0;
				CStringA strPosDes;
				CStringA strPosNew;
				if (rcProb.Width() > rcWin.Width() - 50 - 310 - 50 - 2)
				{
					strPosDes = "50,32,-52,47";
					strPosNew = "-50,32,-0,47";
				}
				else
				{
					strPosDes.Format("50,32,%d,47", 50 + rcProb.Width());
					strPosNew.Format("%d,32,-0,47", 50 + rcProb.Width() + 2);
				}
				m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_TXT_SOFT_DESC, "pos", strPosDes);
				m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_TXT_SOFT_NEW, "pos", strPosNew);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_NEW,TRUE);
			}
			else
			{
				m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_TXT_SOFT_DESC, "pos", "50,32,-0,47");
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_NEW,FALSE);
			}

			if ( /*!pSoftData->m_bSetup && */pSoftData->m_bDownloading && !pSoftData->m_bWaitDownload && !pSoftData->m_bUsingForOneKey )
			{
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_SIZE,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_MARK,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_MARK,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_DOWN,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_UPDATE,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_REINST,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_USE_ONKEY,FALSE);

				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_PROG_DOWN_DOWNING,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_DOWN_PROG,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_SOFT_DOWNING_DOWN,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_SOFT_DOWNING_DOWNED,FALSE);

				CStringA	strProg;
				strProg.Format("%d",pSoftData->m_dwProgress);
				m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_PROG_DOWN_DOWNING,"value",strProg);

				if (pSoftData->m_bLinking)
				{				 
					if (pSoftData->m_bLinkFailed)
						m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_DOWN_PROG,BkString::Get(IDS_SOFTMGR_8089));
					else
						m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_DOWN_PROG,BkString::Get(IDS_SOFTMGR_8090));				
				}
				else
				{
					strProg += "%";
					m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_DOWN_PROG, CA2W(strProg) );
				}

				if ( !pSoftData->m_bDownLoad )
				{
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_PROG_DOWN_DOWNING,TRUE);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_DOWN_PROG,TRUE);
					// 正在下载中的
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_SOFT_DOWNING_DOWN,TRUE);

					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_LNK_SOFT_RETRY,FALSE);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_FEEDBACK,FALSE);
					if (pSoftData->m_bLinking)
					{
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_DOWN_STATE,FALSE);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWN_PAUSE,FALSE);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWN_CONTINUE,FALSE);

						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWN_STOP,!pSoftData->m_bLinkFailed);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_LNK_SOFT_RETRY,pSoftData->m_bLinkFailed);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_FEEDBACK,pSoftData->m_bLinkFailed);
					}
					else
					{
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_DOWN_STATE,TRUE);
						if (pSoftData->m_bPause)
							m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_DOWN_STATE,BkString::Get(IDS_SOFTMGR_8091));				
						else 
							m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_DOWN_STATE,pSoftData->m_strSpeed);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWN_PAUSE,!pSoftData->m_bPause);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWN_CONTINUE,pSoftData->m_bPause);
					}
				}
				else
				{
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_SIZE,TRUE);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_MARK,TRUE);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_MARK,TRUE);

					// 已下载完毕的
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_SOFT_DOWNING_DOWNED,TRUE);

					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWNED_INSTALL,FALSE);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_INST_STATE,TRUE);

					if (pSoftData->m_bWaitInstall)
						m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_INST_STATE,BkString::Get(IDS_SOFTMGR_8102));
					else if (pSoftData->m_bInstalling)
						m_necessList->SetItemText(IDC_SOFTMGR_LISTTMP_TXT_SOFT_INST_STATE,BkString::Get(IDS_SOFTMGR_8085));
					else
					{
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_INST_STATE,FALSE);
						m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_DOWNED_INSTALL,TRUE);
					}

					if ((pSoftData->m_attri&SA_Green) == SA_Green)
					{
						m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_BTN_DOWNED_INSTALL, "class", "btndetaildownloaded");
					}
					else
					{
						m_necessList->SetItemAttribute(IDC_SOFTMGR_LISTTMP_BTN_DOWNED_INSTALL, "class", "btndetaildown");
					}
				}
			}
			else
			{
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_SIZE,TRUE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_MARK,TRUE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_MARK,TRUE);

				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_PROG_DOWN_DOWNING,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_DOWN_PROG,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_SOFT_DOWNING_DOWN,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_DIV_SOFT_DOWNING_DOWNED,FALSE);

				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_DOWN,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_UPDATE,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_REINST,FALSE);
				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_USE_ONKEY,FALSE);

				if (!pSoftData->m_bUsingForOneKey)
				{
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_DOWN,!pSoftData->m_bUpdate&&!pSoftData->m_bSetup);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_UPDATE,pSoftData->m_bUpdate);
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_BTN_SOFT_REINST,pSoftData->m_bSetup&&!pSoftData->m_bUpdate);
				}
				else
				{
					m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_USE_ONKEY,TRUE);
				}

				m_necessList->SetItemVisible(IDC_SOFTMGR_LISTTMP_TXT_SOFT_CHARGE,FALSE);
			}
		}	
	}
	

	return 0;
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnListBoxGetmaxHeight( LPNMHDR pnmh )
{
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = SOFT_LIST_ITEM_HEIGHT;
	return 0;
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnListBoxGetItemHeight( LPNMHDR pnmh )
{
	
	BKLBITEMCACLHEIGHT *pdi	= (BKLBITEMCACLHEIGHT*)pnmh;
	if ( pdi->nListItemId >= m_arrRightList.GetSize() )
		return 0;

	NECESS_SOFT_LIST_DATA&	datalist	= m_arrRightList[pdi->nListItemId];

	if (datalist.bTitle)
		pdi->nHeight = SOFT_LIST_TITLE_HEIGHT;
	else
		pdi->nHeight = SOFT_LIST_ITEM_HEIGHT;
		

	return 0;
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnBkListBoxClickCtrl( LPNMHDR pnmh )
{
	
	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if ( pnms->nListItemID >= m_arrRightList.GetSize() )
		return 0;

	NECESS_SOFT_LIST_DATA&	datalist	= m_arrRightList[pnms->nListItemID];
	CSoftListItemData* pSoftData = GetSoftDataByID(datalist.strSoftId);

	if ( pSoftData != NULL )
	{
		switch( pnms->uCmdID )
		{
		case IDC_SOFTMGR_LISTTMP_TXT_SOFT_NEW:
			m_pSoftMgrMainUI->OnViewNewInfo(pSoftData);
			break;
		case IDC_SOFTMGR_LISTTMP_ICON_SOFT:
		case IDC_SOFTMGR_LISTTMP_TXT_SOFT_TITLE:		
			{
				EnterDetailPage(pSoftData);
			}
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_SOFT_REINST://重装
			m_pSoftMgrMainUI->_downLoadSoft(pSoftData, ACT_SOFT_FROM_NECESS, FALSE );
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_DOWNED_INSTALL:// 安装	
			m_pSoftMgrMainUI->AddToNewInst( pSoftData->m_strSoftID );
			m_pSoftMgrMainUI->_downLoadSoft(pSoftData, ACT_SOFT_FROM_NECESS, FALSE );
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_SOFT_DOWN:	// 下载	
			m_pSoftMgrMainUI->AddToNewInst( pSoftData->m_strSoftID );
			m_pSoftMgrMainUI->_downLoadSoft(pSoftData, ACT_SOFT_FROM_NECESS, FALSE );
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_SOFT_UPDATE://升级
			m_pSoftMgrMainUI->_downLoadSoft(pSoftData, ACT_SOFT_FROM_NECESS, TRUE );
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_DOWN_PAUSE:	// 暂停				
			m_pSoftMgrMainUI->_PauseDownLoad(pSoftData, ACT_SOFT_FROM_NECESS);
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_DOWN_CONTINUE:	// 继续			
			m_pSoftMgrMainUI->_ContinueDownLoad(pSoftData, ACT_SOFT_FROM_NECESS);
			break;
		case IDC_SOFTMGR_LISTTMP_BTN_DOWN_STOP:	// 退出			
			m_pSoftMgrMainUI->_CancelDownLoad(pSoftData, ACT_SOFT_FROM_NECESS);
			break;
		case IDC_SOFTMGR_LISTTMP_LNK_SOFT_RETRY:// 重拾
			m_pSoftMgrMainUI->_TautilogyFromList(pSoftData,ACT_SOFT_FROM_NECESS);
			break;
		case IDC_SOFTMGR_LISTTMP_TXT_SOFT_FEEDBACK:	// 反馈
			m_pSoftMgrMainUI->_FreebackFromList(pSoftData,ACT_SOFT_FROM_NECESS);
			break;
		case IDC_SOFTMGR_LISTTMP_STAR_ONE:
		case IDC_SOFTMGR_LISTTMP_STAR_TWO:
		case IDC_SOFTMGR_LISTTMP_STAR_THREE:
		case IDC_SOFTMGR_LISTTMP_STAR_FOUR:
		case IDC_SOFTMGR_LISTTMP_STAR_FIVE:
		case IDC_SOFTMGR_LISTTMP_TXT_SOFT_MARK: // 打分
			m_pSoftMgrMainUI->OnBiBeiSoftMark(pSoftData);
			break;
		default:
			break;
		}
	}
	

	return 0;
}

void CBeikeSafeSoftmgrNecessHandler::OnFirstShow()
{
	if (m_bShowed)
		return;
	m_bShowed = TRUE;

	if (m_pSoftMgrMainUI->m_pSoftMgr)
	{
		CSimpleArray<NECESS_GROUP>*	arrNecess;
		m_pSoftMgrMainUI->m_pSoftMgr->GetNecessSoftInfo(&arrNecess);
		if (arrNecess!=NULL)
		m_arrNecess = *arrNecess;

		CSimpleArray<ONE_KEY_GROUP>*	arrOneKey;
		m_pSoftMgrMainUI->m_pSoftMgr->GetOneKeySoftInfo(&arrOneKey);
		if (arrOneKey)
			m_arrOneKey = *arrOneKey;

		m_pSoftMgrMainUI->m_pSoftMgr->FreeNecessInfo();
	}
	GenerateId2DataMap();

	m_leftList.ShowScrollBtn(FALSE);
	for ( int i=0; i < m_arrNecess.GetSize(); i++)
	{
		CTypeListItemData*	pdata = new CTypeListItemData(20, i,COL_TYPE_NAME_NORMAL,NULL,0);
		pdata->nId			= i;
		pdata->nNumber		= GetGroupSoftNumber(m_arrNecess[i]);
		pdata->strTypeName	= m_arrNecess[i].strGroupName;

		m_leftList.InsertString(i,pdata->strTypeName );
		m_leftList.SetItemDataX(i, (DWORD_PTR)pdata);
		m_leftList.SetItemHeight(i, TYPE_ITEM_HEIGHT);
	}

	m_nLastSelType = 0;
	m_leftList.SetCurSel(0);
	GenerateSoftList(0);
	m_necessList->SetItemCount( m_arrRightList.GetSize() );

	m_leftList.SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
	m_leftList.Invalidate();
}

void CBeikeSafeSoftmgrNecessHandler::OnClick( int nListId, CTypeListItemData * pData )
{
	if (m_nLastSelType != pData->nId )
	{
		m_bShowType = FALSE;
		m_nLastSelType = pData->nId;


		if (!m_strViewDetailSoftId.IsEmpty())
		{
			m_pSoftMgrMainUI->_QuerySoftMark(m_strViewDetailSoftId);

			m_strViewDetailSoftId.Empty();

			m_IEDetail.Show2(_T("about:blank"),NULL);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_LIST_SHOW,TRUE);

			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_DETAIL_SHOW,FALSE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_FREE_SOFT,TRUE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT,TRUE);

			if ( ::GetFocus() != m_editQuery.m_hWnd )
				m_pSoftMgrMainUI->m_pDlg->SetFocus();
		}

		if (m_pMainDlg->IsItemVisible(IDC_SOFT_NECESS_DIV_SEARCH_TIP) )
		{
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_SEARCH_TIP,FALSE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_NO_FIND, FALSE);
			m_pMainDlg->SetItemAttribute(IDC_SOFTMGR_SOFTLIST_NECESS, "pos", "0,0,-0,-0");
			m_pMainDlg->SetItemVisible(IDC_SOFTMGR_SOFTLIST_NECESS, TRUE);
		}

		GenerateSoftList(pData->nId);
		m_necessList->SetItemCount( m_arrRightList.GetSize() );
		m_necessList->RefreshIntern( TRUE, TRUE, TRUE );
	}
}

NECESS_GROUP& CBeikeSafeSoftmgrNecessHandler::GetGroupByID( int nId )
{
	return m_arrNecess[nId];
}

VOID CBeikeSafeSoftmgrNecessHandler::GenerateSoftList( int nId )
{
	BOOL bFreeChecked = m_pMainDlg->GetItemCheck(IDC_SOFT_NECESS_SHOW_FREE_SOFT);
	BOOL bPlugChecked = m_pMainDlg->GetItemCheck(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT);

	m_arrRightList.RemoveAll();

	if (m_arrNecess.GetSize() <= nId)
		return;

	BOOL			bQuerySoft = FALSE;
	NECESS_GROUP&	groupInfo = GetGroupByID(nId);
	for ( int i=0; i < groupInfo.typeList.GetSize(); i++ )
	{
		NECESS_SOFT_LIST_DATA		datax;
		NECESS_TYPE&				typeInfo = groupInfo.typeList[i];
		BOOL						bPushType = FALSE;

		datax.bTitle			= TRUE;
		datax.strTitleName		= typeInfo.strTypeName;

		for ( int kk=0; kk < typeInfo.softList.GetSize(); kk++)
		{
			NECESS_SOFT&				softInfo = typeInfo.softList[kk];
			NECESS_SOFT_LIST_DATA		datasoft;
			CSoftListItemData*			pSoftData = GetSoftDataByIDFromMap(softInfo.strSoftID);
			BOOL						bCanInsert = FALSE;

			if (pSoftData!=NULL)
			{
				if( bPlugChecked && bFreeChecked )
				{
					if ( !pSoftData->m_bCharge && !pSoftData->m_bPlug )
						bCanInsert = TRUE;
				}
				else if ( bPlugChecked && !bFreeChecked )
				{
					if ( !pSoftData->m_bPlug )
						bCanInsert = TRUE;
				}
				else if ( !bPlugChecked && bFreeChecked )
				{
					if ( !pSoftData->m_bCharge )
						bCanInsert = TRUE;
				}
				else
					bCanInsert = TRUE;

				if (bCanInsert)
				{
					if (!bPushType)
					{
						bPushType = TRUE;
						m_arrRightList.Add(datax);
					}

					datasoft.bTitle		= FALSE;
					datasoft.strSoftId	= softInfo.strSoftID;
					m_arrRightList.Add(datasoft);

					if ( m_pSoftMgrMainUI->m_pInfoQuery )
					{
						CSoftListItemData*	pData = GetSoftDataByID(datasoft.strSoftId);

						if ( pData && pData->m_fMark == 0 )
						{
							m_pSoftMgrMainUI->m_pInfoQuery->AddSoft( _ttoi(datasoft.strSoftId) );
							bQuerySoft = TRUE;
						}
					}
				}
			}
		}
	}
	if (bQuerySoft)
	{
		m_pSoftMgrMainUI->m_pInfoQuery->Query();
	}
}

// 内部生成一个map方便快速查找
VOID CBeikeSafeSoftmgrNecessHandler::GenerateId2DataMap()
{
	for ( int indexGroup=0; indexGroup < m_arrNecess.GetSize(); indexGroup++)
	{
		NECESS_GROUP&	groupInfo = m_arrNecess[indexGroup];
		for ( int i=0; i < groupInfo.typeList.GetSize(); i++ )
		{
			NECESS_TYPE&				typeInfo = groupInfo.typeList[i];
			for ( int kk=0; kk < typeInfo.softList.GetSize(); kk++)
			{
				NECESS_SOFT&				softInfo = typeInfo.softList[kk];
				CSoftListItemData*			pSoftData = GetSoftDataByIDFromMap(softInfo.strSoftID);
				
				if (pSoftData!=NULL)
					m_mapIdData.SetAt( softInfo.strSoftID, pSoftData);
			}
		}
	}
}

CSoftListItemData* CBeikeSafeSoftmgrNecessHandler::GetSoftDataByID( LPCTSTR lpstrSoftID )
{
	CString		strId = lpstrSoftID;
	if ( m_mapIdData.Lookup(strId) )
		return m_mapIdData[strId];
	else
	{
		return GetSoftDataByIDFromMap(lpstrSoftID);
	}
}

CSoftListItemData* CBeikeSafeSoftmgrNecessHandler::GetSoftDataByIDFromMap( LPCTSTR lpstrSoftID )
{
	CString		strId = lpstrSoftID;
	if ( m_pSoftMgrMainUI->m_arrDataMap.Lookup(strId) )
		return m_pSoftMgrMainUI->m_arrDataMap[strId];
	else
		return NULL;
}

DWORD CBeikeSafeSoftmgrNecessHandler::GetGroupSoftNumber( NECESS_GROUP& group )
{
	DWORD	nCount = 0;
	for ( int i=0; i < group.typeList.GetSize(); i++ )
	{
		NECESS_TYPE&				typeInfo = group.typeList[i];
		for ( int kk=0; kk < typeInfo.softList.GetSize(); kk++)
		{
			NECESS_SOFT&				softInfo = typeInfo.softList[kk];
			CSoftListItemData*			pSoftData = GetSoftDataByID(softInfo.strSoftID);

			if (pSoftData!=NULL)
				nCount++;
		}
	}
	return nCount;
}

VOID CBeikeSafeSoftmgrNecessHandler::RefreshItemBySoftID( LPCTSTR lpstrID )
{
	if ( m_pOnekeyDlg != NULL )
	{
		m_pOnekeyDlg->RefreshItemState(lpstrID);
	}

	if (!m_bShowed)
		return;

	CString	strID = lpstrID;

	// 由于搜索结果显示的是大全的结果，所以不再判断是否在当前map里
// 	// 如果不在列表中，直接返回，
// 	if (m_mapIdData.Lookup(strID)==NULL)
// 		return;

	for ( int i=0; i < m_arrRightList.GetSize(); i++)
	{
		NECESS_SOFT_LIST_DATA&	data = m_arrRightList[i];

		if ( !data.bTitle && data.strSoftId == strID )
		{
			m_necessList->RedrawItem(i);
			break;
		}
	}
}

void CBeikeSafeSoftmgrNecessHandler::OnEditEnterMsg( DWORD nEditId )
{
	OnBtnQuerySoft();
}

void CBeikeSafeSoftmgrNecessHandler::OnEditMouseWheelMsg( WPARAM wParam, LPARAM lParam )
{
	CPoint pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

	OnMouseWheel(LOWORD(wParam), HIWORD(wParam), pt);
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnQuerySoft()
{
	CString	strSearch;
	m_editQuery.GetWindowText(strSearch);

	if (strSearch == BkString::Get(IDS_SOFTMGR_8016) )
		return;
	else if (strSearch.IsEmpty())
	{
		if (!m_strViewDetailSoftId.IsEmpty())
		{
			m_pSoftMgrMainUI->_QuerySoftMark(m_strViewDetailSoftId);
			m_strViewDetailSoftId.Empty();

			m_IEDetail.Show2(_T("about:blank"),NULL);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_LIST_SHOW,TRUE);

			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_DETAIL_SHOW,FALSE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_FREE_SOFT,TRUE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT,TRUE);

			if ( ::GetFocus() != m_editQuery.m_hWnd )
				m_pSoftMgrMainUI->m_pDlg->SetFocus();
		}
		OnBtnSearchBack();
	}
	else 
	{
		m_bShowType = TRUE;
		BOOL bFreeChecked = m_pMainDlg->GetItemCheck(IDC_SOFT_NECESS_SHOW_FREE_SOFT);
		BOOL bPlugChecked = m_pMainDlg->GetItemCheck(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT);

		ATLASSERT (m_pSoftMgrMainUI->m_pSoftMgr!=NULL);
		m_arrRightList.RemoveAll();

		//读取
		CSimpleArray<CSoftListItemData*> arrQuery;
		void *pos = m_pSoftMgrMainUI->m_pSoftMgr->SearchSoft(strSearch, L"ar");
		CAtlMap<CString,CString> soft;
		while(0 == m_pSoftMgrMainUI->m_pSoftMgr->GetNextSoft(pos, GetInfoUseMap,&soft))
		{	
			CString strTmp = soft[_T("softid")];
			CSoftListItemData*	pData = GetSoftDataByID(strTmp);
			BOOL bCanInsert = FALSE;
			if (pData)
			{
				if( bPlugChecked && bFreeChecked )
				{
					if ( !pData->m_bCharge && !pData->m_bPlug )
						bCanInsert = TRUE;
				}
				else if ( bPlugChecked && !bFreeChecked )
				{
					if ( !pData->m_bPlug )
						bCanInsert = TRUE;
				}
				else if ( !bPlugChecked && bFreeChecked )
				{
					if ( !pData->m_bCharge )
						bCanInsert = TRUE;
				}
				else
					bCanInsert = TRUE;

				if (bCanInsert)
					arrQuery.Add(pData);
			}
		}
		m_pSoftMgrMainUI->m_pSoftMgr->FinalizeGet(pos);

		//排序
		CAtlList<CSoftListItemData*> arrOrder;
		m_pSoftMgrMainUI->_SortArrayByOrder(arrQuery, arrOrder);

		//构造
		BOOL bQueryMark = FALSE;
		CSoftListItemData *pDataTemp = NULL;
		POSITION posTemp =  arrOrder.GetHeadPosition();
		while (posTemp)
		{
			pDataTemp = arrOrder.GetAt(posTemp);
			if (pDataTemp)
			{
				NECESS_SOFT_LIST_DATA	data;
				data.bTitle		= FALSE;
				data.strSoftId	= pDataTemp->m_strSoftID;
				m_arrRightList.Add(data);

				if (m_pSoftMgrMainUI->m_pInfoQuery)
				{
					// 获取分数
					if (pDataTemp->m_fMark==0 )
					{
						int nTmp = _wtoi( pDataTemp->m_strSoftID );
						m_pSoftMgrMainUI->m_pInfoQuery->AddSoft(nTmp);
						bQueryMark = TRUE;
					}
				}
			}

			arrOrder.GetNext(posTemp);
		}

		m_necessList->SetItemCount(m_arrRightList.GetSize());
		if (!m_strViewDetailSoftId.IsEmpty())
		{
			m_pSoftMgrMainUI->_QuerySoftMark(m_strViewDetailSoftId);
			m_strViewDetailSoftId.Empty();

			m_IEDetail.Show2(_T("about:blank"),NULL);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_LIST_SHOW,TRUE);

			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_DETAIL_SHOW,FALSE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_FREE_SOFT,TRUE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT,TRUE);

			if ( ::GetFocus() != m_editQuery.m_hWnd )
				m_pSoftMgrMainUI->m_pDlg->SetFocus();
		}
		if (m_arrRightList.GetSize() > 0 )
		{
			m_pMainDlg->FormatRichText(
				IDC_SOFT_NECESS_BTN_SEARCH_TEXT, 
				BkString::Get(IDS_RESULT_QUERY_SOFTMGR), 
				strSearch, m_arrRightList.GetSize());

			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_SEARCH_TIP,TRUE);
			m_pMainDlg->SetItemAttribute(IDC_SOFTMGR_SOFTLIST_NECESS, "pos", "0,24,-0,-0");

			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_NO_FIND, FALSE);
			m_pMainDlg->SetItemVisible(IDC_SOFTMGR_SOFTLIST_NECESS, TRUE);
		}
		else
		{
			m_pMainDlg->FormatRichText(
				IDC_SOFT_NECESS_BTN_SEARCH_TEXT, 
				BkString::Get(IDS_NO_QUERY_SOFTMGR), 
				strSearch);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_NO_FIND, TRUE);
			m_pMainDlg->SetItemVisible(IDC_SOFTMGR_SOFTLIST_NECESS, FALSE);
			m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_SEARCH_TIP,TRUE);

		}
		m_necessList->InvalidateRect(NULL,FALSE);

		if (bQueryMark)
		{
			m_pSoftMgrMainUI->m_pInfoQuery->Query();
		}

		m_strKeyWord = strSearch;
		m_pSoftMgrMainUI->m_pDlg->SetTimer( TIMER_ID_NECESS_KEYWORD, 5000, NULL );
		
	}
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnSearchBack()
{
	m_bShowType = FALSE;
	ResetRightListByLeft();
	m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_SEARCH_TIP,FALSE);
	m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_NO_FIND, FALSE);
	m_pMainDlg->SetItemAttribute(IDC_SOFTMGR_SOFTLIST_NECESS, "pos", "0,0,-0,-0");
	m_pMainDlg->SetItemVisible(IDC_SOFTMGR_SOFTLIST_NECESS, TRUE);

	m_editQuery.SetWindowText(L"");
	m_editQuery.SetFocus();
}

VOID CBeikeSafeSoftmgrNecessHandler::ResetRightListByLeft()
{
	int nCur = m_leftList.GetCurSel();
	m_nLastSelType = nCur;
	GenerateSoftList(nCur);
	m_necessList->SetItemCount( m_arrRightList.GetSize() );
	m_necessList->InvalidateRect(NULL);
}

VOID CBeikeSafeSoftmgrNecessHandler::EnterDetailPage(CSoftListItemData* pData)
{
	if (pData == NULL)
		return;

	m_strViewDetailSoftId = pData->m_strSoftID;
	m_nLastSelType = -1;

	//软件信息
	CString strVersion;
	if (pData->m_strNewVersion.IsEmpty() == FALSE && 
		pData->m_strNewVersion.CompareNoCase(L"0.0.0.0") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"1.0.0.0") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"0.0.0.1") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"1.0.0.1") != 0)
	{
		strVersion = pData->m_strNewVersion;
	}
	else if (pData->m_strVersion.IsEmpty() == FALSE && 
		pData->m_strVersion.CompareNoCase(L"0.0.0.0") != 0 &&
		pData->m_strVersion.CompareNoCase(L"1.0.0.0") != 0 &&
		pData->m_strVersion.CompareNoCase(L"0.0.0.1") != 0 &&
		pData->m_strVersion.CompareNoCase(L"1.0.0.1") != 0)
	{
		strVersion = pData->m_strVersion;
	}
	
	if (strVersion.IsEmpty())
	{
		m_pMainDlg->FormatRichText(
			IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_CPT, 
			BkString::Get(IDS_SOFTMGR_8128), 
			pData->m_strName, pData->m_strSize);
	}
	else
	{
		m_pMainDlg->FormatRichText(
			IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_CPT, 
			BkString::Get(IDS_INFO_FORMAT_SOFTMGR), 
			pData->m_strName, pData->m_strSize,strVersion);
	}

	//收费信息
	if (pData->m_bCharge)
	{
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_ICON_RIGHT_DETAIL_FREE, "sub", "2");
		m_pMainDlg->SetItemText(IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_FREE, BkString::Get(IDS_SOFTMGR_8039));
	}
	else
	{
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_ICON_RIGHT_DETAIL_FREE, "sub", "1");
		m_pMainDlg->SetItemText(IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_FREE, BkString::Get(IDS_SOFTMGR_8040));
	}
	//插件信息
	if (pData->m_bPlug)
	{
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_ICON_RIGHT_DETAIL_PLUG, "sub", "2");
		m_pMainDlg->SetItemText(IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_PLUG, BkString::Get(IDS_SOFTMGR_8041));
	}
	else
	{
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_ICON_RIGHT_DETAIL_PLUG, "sub", "1");
		m_pMainDlg->SetItemText(IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_PLUG, BkString::Get(IDS_SOFTMGR_8042));
	}


	//系统信息
	CString strCurOS = m_pSoftMgrMainUI->GetCurrentSystemVersion();
	int nPos = pData->m_strOSName.Find(strCurOS);
	if (nPos >= 0 && nPos < pData->m_strOSName.GetLength())
	{
		strVersion = BkString::Get(IDS_SOFTMGR_8043);
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_ICON_RIGHT_DETAIL_SUIT, "sub", "1");
	}
	else
	{
		strVersion = BkString::Get(IDS_SOFTMGR_8044);
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_ICON_RIGHT_DETAIL_SUIT, "sub", "2");
	}
	strVersion.Append(strCurOS + _T(")"));
	m_pMainDlg->SetItemText(IDC_SOFT_NECESS_TXT_RIGHT_DETAIL_SUIT, strVersion);

	//详情页右侧按钮显示
	if (pData->m_bSetup)
	{
		if (pData->m_bUpdate)//需要升级
			m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_INST, "skin", "btndetailupdate");
		else
		{
			if ((pData->m_attri&SA_Green) == SA_Green)
				m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_INST, "skin", "btndetaildownloaded");
			else
				m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_INST, "skin", "btndetailsetup");
		}
	}
	else if (pData->m_bDownLoad)
	{
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_INST, "skin", "btndetaildown");
	}
	else
	{
		m_pMainDlg->SetItemAttribute(IDC_SOFT_NECESS_BTN_RIGHT_DETAIL_INST, "skin", "btndetailundown");
	}

	//详情IE
	if(pData->GetInfoUrl() == _T(""))	//没有详情信息
	{
		m_IEDetail.Show2(m_pSoftMgrMainUI->m_strErrorHtml,NULL);
	}
	else	//有详情页URL
	{
		m_IEDetail.Show2(pData->GetInfoUrl(), m_pSoftMgrMainUI->m_strErrorHtml);
	}


	m_IconDlg.SetDataRes(pData->m_pImage);

	m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_LIST_SHOW,FALSE);

	m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_DETAIL_SHOW,TRUE);
	m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_FREE_SOFT,FALSE);
	m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT,FALSE);
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnBackFromDetail()
{
	if (!m_strViewDetailSoftId.IsEmpty())
	{
		m_pSoftMgrMainUI->_QuerySoftMark(m_strViewDetailSoftId);

		m_strViewDetailSoftId.Empty();
		m_IEDetail.Show2(_T("about:blank"),NULL);
		m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_LIST_SHOW,TRUE);

		m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_RIGHT_DETAIL_SHOW,FALSE);
		m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_FREE_SOFT,TRUE);
		m_pMainDlg->SetItemVisible(IDC_SOFT_NECESS_SHOW_NOPLUG_SOFT,TRUE);

		m_pMainDlg->SetFocus();

	}
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnInstFromDetail()
{
	CSoftListItemData*	pData = GetSoftDataByID(m_strViewDetailSoftId);

	if ( pData )
	{
		if( !pData->m_bSetup || !pData->m_bUpdate )
			m_pSoftMgrMainUI->AddToNewInst( pData->m_strSoftID );
			
		m_pSoftMgrMainUI->_downLoadSoft(pData, ACT_SOFT_FROM_NECESS, pData->m_bSetup && pData->m_bUpdate );
		OnBtnBackFromDetail();
	}
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnBtmAddSoft()
{
	m_pSoftMgrMainUI->OnSubmitSoft();
}

VOID CBeikeSafeSoftmgrNecessHandler::OnFreeBack()
{
	CSoftListItemData*	pData = GetSoftDataByID(m_strViewDetailSoftId);
	if ( pData )
	{
		CString strURL;
		CURLEncode url_encode;	
		strURL.Format(_T("http://baike.ijinshan.com/ksafe/baike/customer_supplementsoft.html?name=%s"),
			url_encode.URLEncode(pData->m_strName));
		ShellExecute( NULL, _T("open"), strURL, NULL, NULL , SW_SHOW);
	}
}

VOID CBeikeSafeSoftmgrNecessHandler::OnReStart()
{
	if (m_pSoftMgrMainUI)
		m_pSoftMgrMainUI->OnReStart();
}
VOID CBeikeSafeSoftmgrNecessHandler::OnIgnoreReStartTip()
{
	if (m_pSoftMgrMainUI)
		m_pSoftMgrMainUI->OnIgnoreReStartTip();
}
VOID CBeikeSafeSoftmgrNecessHandler::OnCloseReStartTip()
{
	if (m_pSoftMgrMainUI)
		m_pSoftMgrMainUI->OnCloseReStartTip();
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnBtmViewDown()
{
	m_pSoftMgrMainUI->OnDownloadMgr();
}

VOID CBeikeSafeSoftmgrNecessHandler::OnBtnDetailFreeback()
{
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnBkListBoxDbClick( LPNMHDR pnmh )
{
	LPBKLBMITEMDBCLICK pnms = (LPBKLBMITEMDBCLICK)pnmh;
	if ( pnms->nListItemID >= m_arrRightList.GetSize() )
		return 0;

	NECESS_SOFT_LIST_DATA&	datalist	= m_arrRightList[pnms->nListItemID];

	if (datalist.bTitle)
		return 0;

	CSoftListItemData* pSoftData		= GetSoftDataByID(datalist.strSoftId);

	if ( pSoftData != NULL )
	{
		EnterDetailPage(pSoftData);
	}

	return 0;
}

void CBeikeSafeSoftmgrNecessHandler::OnOneKeyInstall()
{

	if (m_pOnekeyDlg)
	{
		m_pOnekeyDlg->OnShowX(m_pMainDlg->m_hWnd);
	}
	else
	{
		m_pOnekeyDlg = new CBkSoftOneKeyDlg(m_pSoftMgrMainUI,m_arrOneKey, m_pSoftMgrMainUI->m_arrDataMap);
		m_pOnekeyDlg->Create(NULL);
		m_pOnekeyDlg->OnShowX(m_pMainDlg->m_hWnd);
	}
}

LRESULT CBeikeSafeSoftmgrNecessHandler::OnExitOneKeyDlg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (m_pOnekeyDlg)
	{
		delete m_pOnekeyDlg;
		m_pOnekeyDlg = NULL;
	}
	return 0;
}

int CBeikeSafeSoftmgrNecessHandler::CloseCheck()
{
	if (m_pOnekeyDlg==NULL)
		return -1;

	if (m_pOnekeyDlg)
	{
		LRESULT lResult = ::SendMessage(m_pOnekeyDlg->m_hWnd, WM_CLOSE_DLG, 0, 0);
		if (lResult == 1)
			return 1;//要退出
		else
			return 0;//不要退出
	}

	return -1;
}

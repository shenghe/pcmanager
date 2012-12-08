#include "stdafx.h"
#include "kflowstatlist.h"
#include "kpfw/netflowformat.h"

BOOL KFlowStatList::OnInitDialog( CWindow wndFocus, LPARAM lInitParam )
{
	assert(m_pVFlowStatPList != NULL);

	m_pNetProcessList = new CBkNetMonitorListBox;
	if (NULL == m_pNetProcessList)
		return FALSE;

//	_GetCurLogInfo(enumQueryTypeEveryDay);

	//listbox
	m_pNetProcessList->Create( GetViewHWND(), DEFLISTWINDOW);
	m_pNetProcessList->Load(IDR_BK_STAT_PROCESSLISTINFO);
	m_pNetProcessList->SetCanGetFocus(FALSE);

	if (m_pVFlowStatPList)
		m_pNetProcessList->SetItemCount(m_pVFlowStatPList->size());

	_SetCountData();

	SetTimer(3, 1000, NULL);

	return TRUE;
}

void KFlowStatList::GetRangleQuery( OUT CString& strFrom, OUT CString& strTo )
{
}

void KFlowStatList::BtnClose( void )
{
	EndDialog(IDCANCEL);
}

void KFlowStatList::BtnCancel( void )
{	
	BtnClose();
}

void KFlowStatList::_SetMessage(IN const CString& strAllFlow, IN const CString& strUpFlow, IN const CString& strDownFlow)
{
	CString strFileMessage;
	strFileMessage.Format(_T("今日网络总流量%s，上传总流量%s，下载总流量%s"), strAllFlow, strUpFlow, strDownFlow);
	SetItemText(200, strFileMessage);
}

CString KFlowStatList::_GetNeedShowData( IN ULONGLONG uData )
{
	CString strRet	= _T("0KB");

	NetFlowToString(uData, strRet);

	return strRet;
}

BOOL KFlowStatList::_GetCurLogInfo(OUT LONGLONG& uUpData, OUT LONGLONG& uDownData, OUT LONGLONG& uAllData)
{
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	CString strDayQuery;
	strDayQuery.Format(_T("%4d-%02d-%02d"), systm.wYear, systm.wMonth, systm.wDay);

	CComPtr<ISQLiteComResultSet3> spiRet = NULL;
	if (m_pFlowStatLog->QueryData(enumQueryTypeEveryDay, strDayQuery, spiRet))
	{
		if (!spiRet->IsEof())
		{
			uUpData = _wtoi64(spiRet->GetAsString(2));
			uDownData = _wtoi64(spiRet->GetAsString(3));
			uAllData = _wtoi64(spiRet->GetAsString(4));
		}
	}

	return TRUE;
}

void KFlowStatList::_SetCountData()
{
	LONGLONG uUpData = 0, uDownData = 0, uAllData = 0;
	_GetCurLogInfo(uUpData, uDownData, uAllData);
	_SetMessage(_GetNeedShowData(uAllData * 1024),
				_GetNeedShowData(uUpData * 1024),
				_GetNeedShowData(uDownData * 1024));

// 	KNetFluxStasticCacheReader	fluxStatRead;
// 	if (E_FAIL == fluxStatRead.Init())
// 		return;
// 
// 	KStasticFluxProcessList* pFluxStatRead = fluxStatRead.GetStasticFluxList();
// 	if (pFluxStatRead == NULL)
// 		return ;
// 
// 	pFluxStatRead->m_lock.LockRead();
// 	_SetMessage(_GetNeedShowData((pFluxStatRead->m_nTotalRecv + pFluxStatRead->m_nTotalSend)/ 1024 * 1024),
// 		_GetNeedShowData(pFluxStatRead->m_nTotalSend/ 1024 * 1024),
// 		_GetNeedShowData(pFluxStatRead->m_nTotalRecv/ 1024 * 1024));
// 
// 	pFluxStatRead->m_lock.UnLockRead();

	if (m_nNeedShowCount != (*m_pVFlowStatPList).size())
	{
		m_nNeedShowCount = (*m_pVFlowStatPList).size();
		m_pNetProcessList->SetItemCount(m_nNeedShowCount);
		//m_pNetProcessList->SetItemCount(8);
	}
	else
	{
		m_pNetProcessList->ForceRefresh();
		m_pNetProcessList->ResetMouseMoveMsg();
	}	
	
}

LRESULT KFlowStatList::OnTimer( UINT timerId )
{
	//m_pNetProcessList->SetItemCount();
	_SetCountData();
	return S_OK;
}

LRESULT KFlowStatList::OnListGetDispInfo( LPNMHDR pnmh )
{
	if (NULL == m_pNetProcessList)
		return E_POINTER;

	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;
	int nListItemID = pdi->nListItemID;
	if (nListItemID < 0 || nListItemID >= m_pVFlowStatPList->size())  // 如果越界，则返回，ListBox的实现也存在问题，nListItemID也可为一个无效值
	{
		return E_FAIL;
	}

	pdi->nHeight = 31;

	CString strPath = (*m_pVFlowStatPList)[nListItemID].itemElem.m_strProcPath;

	int nPos = strPath.ReverseFind('\\');
	CString strNameProcess = strPath.Mid(nPos + 1);

	CString strName = (*m_pVFlowStatPList)[nListItemID].m_strProcessName;
	if (strName.IsEmpty())
	{
		strName = strNameProcess;
	}
	
	strName.AppendFormat(L"(%s)", strNameProcess);

	m_pNetProcessList->SetItemAttribute(DEF_LIST_213, "iconhandle", "0");
	m_pNetProcessList->SetItemAttribute(DEF_LIST_213, "srcfile", "0");

	if (::PathFileExists(strPath))
		m_pNetProcessList->SetItemAttribute(DEF_LIST_213, "srcfile", (CStringA)strPath);
	else
		m_pNetProcessList->SetItemAttribute(DEF_LIST_213, "iconhandle", m_defIcon);

	CStringA strPosA;
	strPosA.Format("%d,8,%d,31", 30, 300);

	m_pNetProcessList->SetItemStringAttribute(DEF_LIST_214, "tip", strName);
	m_pNetProcessList->SetItemAttribute(DEF_LIST_214, "pos", strPosA);

	m_pNetProcessList->SetItemText(DEF_LIST_214, strName);
	m_pNetProcessList->SetItemText(DEF_LIST_215, _GetNeedShowData((*m_pVFlowStatPList)[nListItemID].itemElem.m_nTotalRecv + (*m_pVFlowStatPList)[nListItemID].itemElem.m_nTotalSend));
	m_pNetProcessList->SetItemText(DEF_LIST_216, _GetNeedShowData((*m_pVFlowStatPList)[nListItemID].itemElem.m_nTotalSend));
	m_pNetProcessList->SetItemText(DEF_LIST_217, _GetNeedShowData((*m_pVFlowStatPList)[nListItemID].itemElem.m_nTotalRecv));

	
/*
	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;
	int nListItemID = pdi->nListItemID + (m_nCurPage[m_nCurShowType] - 1) * DEF_PAGE_COUNT;

	if (nListItemID >= m_vLogListInfo.size())
		return S_FALSE;

	m_pNetStatListBox->SetItemText(286, _GetStatListInfoTime(m_vLogListInfo[nListItemID].strMinTime, m_vLogListInfo[nListItemID].strMaxTime, m_nCurShowType));
	m_pNetStatListBox->SetItemText(287, _GetStatListInfoUpAllFlow(m_vLogListInfo[nListItemID].lUpFlow));
	m_pNetStatListBox->SetItemText(288, _GetStatListInfoDownAllFlow(m_vLogListInfo[nListItemID].lDownFlow));
	m_pNetStatListBox->SetItemText(289, _GetStatListInfoAllFlow(m_vLogListInfo[nListItemID].lAllFlow));
	m_pNetStatListBox->SetItemText(290, _GetStatListMoniterTime(m_vLogListInfo[nListItemID].lAllTime));
*/
	return S_OK;
}

LRESULT KFlowStatList::OnListGetmaxHeight( LPNMHDR pnmh )
{
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = 31;
	return 0;
}

LRESULT KFlowStatList::OnListGetItemHeight( LPNMHDR pnmh )
{
	BKLBITEMCACLHEIGHT *pdi	= (BKLBITEMCACLHEIGHT*)pnmh;
	pdi->nHeight = 31;

	return 0;
}

void KFlowStatList::SetInfoPointer( IN std::vector<KFluxStasticProcItemEx>* pFluxItem, IN char* pDefIcon, IN KFlowStat* pStat)
{
	CopyMemory(m_defIcon, pDefIcon, 260);
//	m_pFluxStatRead		= pFluxList;
	m_pVFlowStatPList	= pFluxItem;
	m_pFlowStatLog		= pStat;
}

BOOL KFlowStatList::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	CRect rcWin;
	if ( m_pNetProcessList && m_pNetProcessList->IsWindow() && m_pNetProcessList->IsWindowVisible() )
	{
		m_pNetProcessList->GetWindowRect(&rcWin);

		if (rcWin.PtInRect(pt))
			SendMessage(m_pNetProcessList->m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
	}


	SetMsgHandled(FALSE);
	return TRUE;	
}
#include "stdafx.h"
#include "kflowstatdefine.h"
#include "beikesafemsgbox.h"

BOOL KFlowStatDefine::OnInitDialog( CWindow wndFocus, LPARAM lInitParam )
{
	m_TimeFrom.Create(GetViewHWND(), NULL, NULL, WS_CHILD, 0/*ES_NOHIDESEL | ES_AUTOHSCROLL | ES_NUMBER*/, 102);
	m_TimeTo.Create(GetViewHWND(), NULL, NULL, WS_CHILD, 0/*ES_NOHIDESEL | ES_AUTOHSCROLL | ES_NUMBER*/, 103);
	m_TimeFrom.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	m_TimeTo.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	return TRUE;
}

void KFlowStatDefine::GetRangleQuery( OUT CString& strFrom, OUT CString& strTo )
{
	strFrom = m_strTimeFrom;
	strTo	= m_strTimeTo;
}

void KFlowStatDefine::BtnClose( void )
{
	EndDialog(IDCANCEL);
}

void KFlowStatDefine::BtnOK( void )
{
	SYSTEMTIME sysFrom, sysTo;
	m_TimeFrom.GetSystemTime(&sysFrom);
	m_TimeTo.GetSystemTime(&sysTo);

	m_strTimeFrom.Format(_T("%d-%02d-%02d"), sysFrom.wYear, sysFrom.wMonth, sysFrom.wDay);
	m_strTimeTo.Format(_T("%d-%02d-%02d"), sysTo.wYear, sysTo.wMonth, sysTo.wDay);

	if (m_strTimeTo < m_strTimeFrom)
	{
		CBkSafeMsgBox2 msg;
		msg.ShowMutlLineMsg(L"您指定的结束时间低于指定的开始时间，请重新选择！", L"金山卫士-流量监控");
		return;
	}

	EndDialog(IDOK);
}

void KFlowStatDefine::BtnCancel( void )
{	
	BtnClose();
}
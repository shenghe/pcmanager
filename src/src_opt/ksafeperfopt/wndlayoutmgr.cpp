#include "StdAfx.h"
#include "wndlayoutmgr.h"
using namespace Wnd_LayOut_Mgr;

CWndLayOutMgr::CWndLayOutMgr(void)
{
}

CWndLayOutMgr::~CWndLayOutMgr(void)
{
}

int CWndLayOutMgr::PushUIGroup(DWORD dwGroupID, UI_Group uiGroup)
{
	int nCount = 0;
	m_mapUI[dwGroupID]	= uiGroup;

	return m_mapUI.GetCount();
}

int CWndLayOutMgr::SwitchUI(DWORD dwGroupID)
{
	int nRet = -1;

	POSITION pos = m_mapUI.GetStartPosition();

	while(NULL != pos)
	{
		UI_Group ui = m_mapUI.GetValueAt(pos);
/*
		if (dwGroupID == ui.dwGroupID)
		{
			SetItemVisible(ui.dwGroupID, TRUE);
		}
		else
		{
			SetItemVisible(ui.dwGroupID, FALSE);
		}
*/
		m_mapUI.GetNext(pos);
	}

	return nRet;
}
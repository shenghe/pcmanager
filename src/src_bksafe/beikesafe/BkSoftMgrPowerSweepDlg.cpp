#include "stdafx.h"
#include "BkSoftMgrPowerSweepDlg.h"
#include "beikesafesoftmgrHeader.h"


void CBeikeSoftMgrPowerSweepDlg::OnDelSelected()
{
	//显示进度
	SetItemVisible(IDC_TEXT_TIP_POWERSWEEP, FALSE);
	SetItemVisible(IDC_PROG_POWERSWEEP, TRUE);

	HTREEITEM	hRoot = m_Tree.GetRootItem();

	m_nCheckNum			= GetTreeCheckedCount( hRoot, TRUE );
	m_nSweepedNumber	= 0;

	if ( m_nCheckNum > 0 )
	{
		KillTimer( 1 );
		EnableItem( IDC_BTN_DEL_SELECTED, FALSE );
		EnableItem( IDCANCEL, FALSE);
		m_Tree.EnableWindow(FALSE);

		m_bSweeping = TRUE;

		//强力清扫行为统计
		int nReportFlag = 0;
		if ( m_bDeleteReg && m_bDeleteFile )
		{
			nReportFlag = 3;
		}
		else if ( m_bDeleteFile && !m_bDeleteReg )
		{
			nReportFlag = 1;
		}
		else if ( !m_bDeleteFile && m_bDeleteReg )
		{
			nReportFlag = 2;
		}


		_PowerSweepThread();

		m_bDeleteReg = FALSE;
		m_bDeleteFile = FALSE;
		m_bHasClearAction = TRUE;
	}

}

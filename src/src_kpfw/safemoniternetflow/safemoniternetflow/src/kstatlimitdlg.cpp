#include "stdafx.h"
#include "kstatlimitdlg.h"
#include <time.h>
#include "kpfw/kpfw_def.h"
#include "common/kopermemfile.h"


void KStatListDlg::BtnClose( void )
{
	EndDialog(IDCANCEL);
	m_bIsShowDlg = FALSE;
}

BOOL KStatListDlg::OnInitDialog( CWindow wndFocus, LPARAM lInitParam )
{
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	_SetShowInfo();
	CenterWindow();
	m_bIsShowDlg = TRUE;
	return TRUE;
}

void KStatListDlg::OnClickMoreSetting( void )
{
	EndDialog(DEFRESULTMORESETTING);
}

void KStatListDlg::OnClickShowNetMointer( void )
{
	EndDialog(DEFRESULTNETMOINTERCONTRAL);
}

void KStatListDlg::OnClickShowStatResult( void )
{
	EndDialog(DEFRESULTSTATINFO);
}

void KStatListDlg::OnClickCheckDay( void )
{
	int nTime = _time32(NULL);
	CString strCurTime;
	strCurTime.Format(_T("%ld"), nTime);
	_SetConfigInfo(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_DAY_MARK_NAME, strCurTime);
	KOperMemFile::Instance().SetDayMark(nTime);
}

void KStatListDlg::OnClickCheckMonth( void )
{
	CString strCurTime;
	SYSTEMTIME sysTm;
	GetLocalTime(&sysTm);
	strCurTime.Format(_T("%ld"), (sysTm.wYear * 100 + sysTm.wMonth));
	_SetConfigInfo(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_MONTH_MARK_NAME, strCurTime);
	int nTime = sysTm.wYear * 100 + sysTm.wMonth;
	KOperMemFile::Instance().SetMonthMark(nTime);
}

void KStatListDlg::_SetConfigInfo( IN const CString& strTerm, IN const CString& strName, IN const CString& strValue )
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	::WritePrivateProfileStringW(strTerm, strName, strValue, cfgFileNamePath);
}

void KStatListDlg::SetConfig( IN int nMode, IN int nSetSize )
{
	m_nMode = nMode;
	m_nSetSize = nSetSize;
	_SetShowInfo();

	//::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

void KStatListDlg::_SetShowInfo( void )
{
	int nTime = _time32(NULL);
	CString strCurTime;
	strCurTime.Format(_T("%ld"), nTime);
	if (m_nMode == 1)//每日模式
	{
		CString str;
		str.Format(_T("您今日的网络总流量已经超过%dMB"), m_nSetSize);
		SetItemText(DEFINFOTEXT, str);
		SetItemVisible(DEFCHECKDAY, TRUE);
		SetItemVisible(DEFCHECKMONTH, FALSE);

		_SetConfigInfo(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_DAY_DATA_NAME, strCurTime);
		KOperMemFile::Instance().SetDayLastData(nTime);
	}
	else if (m_nMode == 2)//每月模式
	{
		CString str;
		str.Format(_T("您本月的网络总流量已经超过%dMB"), m_nSetSize);
		SetItemText(DEFINFOTEXT, str);
		SetItemVisible(DEFCHECKDAY, FALSE);
		SetItemVisible(DEFCHECKMONTH, TRUE);

		_SetConfigInfo(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_MONTH_DATA_NAME, strCurTime);
		KOperMemFile::Instance().SetMonthLastData(nTime);
	}
}

void KStatListDlg::OnSysCommand( UINT nID, CPoint pt )
{
	if ( nID == SC_CLOSE )
		EndDialog(IDCANCEL);
	else if (nID == SC_MAXIMIZE)
		return;
	else if (nID == SC_MINIMIZE)
		DefWindowProc();

	DefWindowProc();
}
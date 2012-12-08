
#include "stdafx.h"
#include "dialogsysperfopt.h"
#include "iefix/cinifile.h"
#include <time.h>
#include "atltime.h"
#include "kpfw/netflowformat.h"

union ULONG32TO64
{
	ULONGLONG u64;
	DWORD dw32[2];
};


void pfnCallBack(CallBackStruct& param, void* pContext)
{
	CBKSafeSysPerfOpt* pThis = (CBKSafeSysPerfOpt*)pContext;

	if (NULL != pThis)
		pThis->SetProgress(param.nTotalProc, param.nFreeProc);
}

void pfnFreeMem(void* param)
{
	CBKSafeSysPerfOpt* _pThis = (CBKSafeSysPerfOpt*)param;

	ULONGLONG uSize = CSysPerfOpt::Instance()->ForceFreeMem();

	if (NULL != _pThis)
	{
		ULONG32TO64 value;
		value.u64 = uSize;
		_pThis->SendMessage(MSG_FINISHED, value.dw32[0], value.dw32[1]);
	}
		//_pThis->FreeMemFinished(uSize);
}

BOOL CBKSafeSysPerfOpt::OnInitDialog(HWND wParam, LPARAM lParam)
{
	//m_sysPerf.SetCallBack(pfnCallBack, this);
	CSysPerfOpt::Instance()->SetCallBack(pfnCallBack, this);

	UpDateOneKeyFreeMemHistory();

	if (FALSE == IsItemVisible(3))
	{
		HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pfnFreeMem, this, 0, 0);

		if (NULL != h)
			CloseHandle(h);
		h = NULL;
	}

	SetItemIntAttribute(101, "value", 0);
	return TRUE;
}

void CBKSafeSysPerfOpt::OnBkClose()
{
	m_pMainDlg->SetPerfOptDlgHasDoModal(FALSE);
	EndDialog(0);
}

void CBKSafeSysPerfOpt::OnOk()
{
	m_pMainDlg->SetPerfOptDlgHasDoModal(FALSE);
	EndDialog(0);
}
void CBKSafeSysPerfOpt::UpDateOneKeyFreeMemHistory()
{
	CString m_strCfgPath;
	CAppPath::Instance().GetLeidianCfgPath(m_strCfgPath, TRUE);
	m_strCfgPath.Append(L"\\bksafe.ini");

	IniFileOperate::CIniFile iniFile(m_strCfgPath);
	ULONGLONG uTime = 0, uByte = 0, uNowTime = 0;
	TCHAR pszTime[50] = {0}, pszByte[50] = {0};
	iniFile.GetStrValue(L"FreeMem", L"FreeSize", pszByte, sizeof(pszByte));
	iniFile.GetStrValue(L"FreeMem", L"FreeTime", pszTime, sizeof(pszTime));

	uTime = _wtoi64_l(pszTime, 0);
	uByte = _wtoi64_l(pszByte, 0);

	uNowTime = _time64(NULL);
	CTimeSpan spanTime(uNowTime - uTime);

	CString strValue = L"", strMem = L"";

	SetItemVisible(1, FALSE);
	SetItemVisible(2, FALSE);
	SetItemVisible(3, FALSE);

	if (0 == uTime || uNowTime < uTime)
	{//以前没有做过优化
		SetItemVisible(1, TRUE);
	}else if (spanTime.GetTotalMinutes() <= 5 )
	{//30分钟之内使用了系统优化
		SetItemVisible(3, TRUE);
	}
	else
	{
		SetItemVisible(1, TRUE);
	}
}

void CBKSafeSysPerfOpt::SetProgress(int nTotal, int nCur)
{
	if (nTotal > 0 && nCur > 0)
	{
		int n = 100.0 / (float)nTotal + 0.5,
			curSize = nCur*n;
		if (curSize > nTotal)
			curSize = nTotal;

		//
		SendMessage(MSG_PROGRESS, nTotal, curSize);
	}
}

void CBKSafeSysPerfOpt::FreeMemFinished(ULONGLONG uSize)
{
	CString m_strCfgPath, strValue, strHistoryValue, strXmlValue;
	CAppPath::Instance().GetLeidianCfgPath(m_strCfgPath, TRUE);
	m_strCfgPath.Append(L"\\bksafe.ini");

	IniFileOperate::CIniFile iniFile(m_strCfgPath);
	ULONGLONG uHistoryByte = 0;
	TCHAR pszByte[50] = {0};

	//获取历史
	iniFile.GetStrValue(L"FreeMem", L"FreeSize", pszByte, sizeof(pszByte));
	uHistoryByte = _wtoi64_l(pszByte, 0);
	NetFlowToString(uHistoryByte + uSize, strHistoryValue);
	strXmlValue.Format(BkString::Get(PERF_MONITOR_STR_524), strHistoryValue);
	SetRichText(202, strXmlValue);

	//写入新的
	strValue.Format(L"%I64d", uSize + uHistoryByte);
	iniFile.SetStrValue(L"FreeMem", L"FreeSize", strValue);

	strValue.Format(L"%I64d", _time64(NULL));
	iniFile.SetStrValue(L"FreeMem", L"FreeTime", strValue);
	//记录下最新的记录
	strValue.Format(L"%I64d", uSize);
	iniFile.SetStrValue(L"FreeMem", L"LastFreeSize", strValue);

	NetFlowToString(uSize, strValue);
	strXmlValue.Format(BkString::Get(PERF_MONITOR_STR_523), strValue);
	SetRichText(201, strXmlValue);

	SetItemVisible(1, FALSE);
	SetItemVisible(2, TRUE);
	SetItemVisible(3, FALSE);
}

LRESULT CBKSafeSysPerfOpt::OnProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& )
{
	int nTotal = wParam,
		nCur = lParam;

	if (nTotal >0 && nCur > 0)
	{
		SetItemIntAttribute(101, "value", nCur);
	}

	return S_OK;
}

LRESULT CBKSafeSysPerfOpt::OnFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& )
{
	SetItemIntAttribute(101, "value", 100);

	ULONG32TO64 value;
	value.dw32[0] = wParam;
	value.dw32[1] = lParam;

	FreeMemFinished(value.u64);

	
	return S_OK;
}
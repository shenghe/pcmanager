#include "stdafx.h"
#include "kupdateversiondlg.h"
#include "bkupctl/bkupddownloader.h"       /* for CBkUpdDownloader */
#include "bkupctl/bkupclterr.h"
#include "miniutil/fileversion.h"          /* for VersionLess() */
#include "miniutil/bkini.h"                /* for BKIni::Document */
#include "zlib/zlibimp.h"                  /* for CZlib */
#include "common/utility.h"                /* for GetLeidianLogPath */
#include "zlibcrc32/crc32.h"               /* for CRC32 */
#include "winmod/wintrustverifier.h"       /* for VerifyFile */
#include "beikesafemsgbox.h"               /* for CBkSafeMsgBox */
#include <winhttp.h>


#define UPDATE_INI_URL      _T("http://up.ijinshan.com/safe/ksafeupdate.ini")
#define UPDATE_HIT_CHECK    _T("正在检测您使用的金山卫士是否为最新版本, 请等待...")
#define UPDATE_HIT_FOUND    _T("检测到有更新版本, 请点击“立即升级”升级到最新版本。")
#define UPDATE_HIT_FINISH   _T("您的金山卫士已经是最新版本。")
#define UPDATE_HIT_BUY      _T("网络正忙，请稍后再试。")
#define UPDATE_HIT_FAILED1  _T("升级失败，请稍后再试。")
#define UPDATE_HIT_FAILED2  _T("版本检测失败，请稍后再试。")  
#define UPDATE_HIT_DOWN     _T("正在下载升级包，请等待...")
#define UPDATE_DOWNLOAD_URL _T("http://www.ijinshan.com/ws/index.shtml")
#define UPDATE_EDIT_STYLE  (WS_CHILD | WS_VSCROLL | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY)

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CUpdateVersionDlg


CUpdateVersionDlg::CUpdateVersionDlg() 
: CBkDialogImpl<CUpdateVersionDlg>(IDR_BK_UPDATE_VERSION)
{
}

CUpdateVersionDlg::~CUpdateVersionDlg()
{
}

BOOL CUpdateVersionDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	SetItemVisible(IDC_DLG_UPDATE_WARNING, FALSE);
	SetItemVisible(IDC_DLG_UPDATE_DETAIL,  FALSE);
	SetItemVisible(IDC_BTN_UPDATE_OK,      FALSE);
	SetItemVisible(IDC_BTN_UPDATE_START,   FALSE);
	ShowOfflineInstallHint(FALSE);

	SetItemText(IDC_TXT_UPDATE_HIT, UPDATE_HIT_CHECK); 
	SetItemVisible(IDC_TXT_UPDATE_HIT,     TRUE);
	SetItemVisible(IDC_BTN_UPDATE_CANCEL,  TRUE);
	SetItemVisible(IDC_PROGRESS_UPDATE,    TRUE);

	m_edtDetail.Create(GetViewHWND(), NULL, NULL, 
		UPDATE_EDIT_STYLE, 0, IDC_EDIT_UPDATE_DETAIL);	

	PARAFORMAT2 pf2;
	pf2.cbSize = sizeof(PARAFORMAT2);
	pf2.dwMask = PFM_LINESPACING|PFM_BORDER;
	pf2.dyLineSpacing     = 330;
	pf2.bLineSpacingRule  = 4;

	m_edtDetail.SetParaFormat(pf2);
	m_edtDetail.SetFont(BkFontPool::GetFont(FALSE, FALSE, FALSE, 0));
	m_edtDetail.SetBackgroundColor(RGB(240, 248, 255));

	m_updater.Attach(m_hWnd);
	m_updater.Start(CManualUpdater::CHECKVER);
	return TRUE;
}


void CUpdateVersionDlg::OnBtnClose()
{
	if (IsItemVisible(IDC_PROGRESS_UPDATE))
	{
		UINT_PTR confirm = CBkSafeMsgBox::Show( 
			L"正在下载升级文件，你确定要取消吗？", 
			L"金山卫士在线升级",
			MB_OKCANCEL|MB_ICONWARNING);

		if (confirm == IDCANCEL)
			return;
	}
	m_updater.Stop();
	EndDialog(0);
}

void CUpdateVersionDlg::OnBtnMini()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE|HTCAPTION, 0);
}

void CUpdateVersionDlg::OnBtnCancel()
{
	m_updater.Stop();
	EndDialog(0);
}

void CUpdateVersionDlg::OnBtnLink()
{
	::ShellExecute(NULL, _T("Open"), UPDATE_DOWNLOAD_URL, NULL, NULL, 0);
}

void CUpdateVersionDlg::OnBtnStart()
{
	SetItemVisible(IDC_BTN_UPDATE_START,  FALSE);
	SetItemVisible(IDC_DLG_UPDATE_DETAIL, FALSE);
	SetItemText(IDC_TXT_UPDATE_HIT, UPDATE_HIT_DOWN);
	SetItemVisible(IDC_PROGRESS_UPDATE, TRUE);
	SetItemIntAttribute(IDC_PROGRESS_UPDATE, "value", (int)0);
	m_updater.Start(CManualUpdater::DOWNLAOD);
}

LRESULT CUpdateVersionDlg::OnProgress(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SetItemIntAttribute(IDC_PROGRESS_UPDATE, "value", (int)lParam);
	return TRUE;
}


void CUpdateVersionDlg::ShowUpdateDetail()
{
	int count =  m_updater.GetNumberOfDetail();
	CString detail;
	CString version;
	const int rowwide = 30;

	version.Format(_T("新版本号:\n  %s\n"), m_updater.GetVersion());

	for (int i=0; i<count; ++i)
	{
		CString item = m_updater.GetDetailItem(i);
		item.Format(_T("  %d.%s\n"), i+1, item);
		int line =  item.GetLength() / rowwide;

		for (int j=0; j<line; ++j)
		{
			if (item[(j+1)*(rowwide-1)] != _T('\n'))
				item.Insert((j+1)*(rowwide-1), _T("\n"));
		}

		detail += item;
	}

	detail.Insert(0, _T("更新内容:\n"));
	m_edtDetail.SetWindowText((version + detail));
	SetItemVisible(IDC_DLG_UPDATE_DETAIL, TRUE);
}

void CUpdateVersionDlg::ShowOfflineInstallHint(BOOL fShow)
{
	SetItemVisible(IDC_LNK_UPDATE_OFFLINE,  fShow);
	SetItemVisible(IDC_TXT_UPDATE_OFFLINE1, fShow);
	SetItemVisible(IDC_TXT_UPDATE_OFFLINE2, fShow);
}

void CUpdateVersionDlg::ShowWarnning(LPCTSTR lpMsg)
{
	SetItemVisible(IDC_TXT_UPDATE_HIT,     FALSE);
	SetItemVisible(IDC_PROGRESS_UPDATE,    FALSE);
	SetItemVisible(IDC_BTN_UPDATE_CANCEL,  FALSE);
	SetItemVisible(IDC_BTN_UPDATE_OK,      TRUE);

	SetItemText(IDC_TXT_UPDATE_WARNING, lpMsg);
	SetItemVisible(IDC_DLG_UPDATE_WARNING, TRUE);
}

LRESULT CUpdateVersionDlg::OnCheckVersion(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (lParam) /* found new version */
	{
		SetItemText(IDC_TXT_UPDATE_HIT, UPDATE_HIT_FOUND);
		SetItemVisible(IDC_BTN_UPDATE_CANCEL, TRUE);
		SetItemVisible(IDC_BTN_UPDATE_START,  TRUE);
		SetItemVisible(IDC_PROGRESS_UPDATE,   FALSE);

		ShowUpdateDetail(); /* show details */
	}
	else 
	{
		HRESULT hr = (HRESULT)wParam;

		if (SUCCEEDED(hr)) /* have been new version */
		{
			SetItemText(IDC_TXT_UPDATE_HIT, UPDATE_HIT_FINISH);
			SetItemVisible(IDC_PROGRESS_UPDATE,   FALSE);
			SetItemVisible(IDC_BTN_UPDATE_CANCEL, FALSE);
			SetItemVisible(IDC_BTN_UPDATE_OK,     TRUE);

			CString detail = _T("当前版本号:");
			detail += m_updater.GetVersion();

			m_edtDetail.SetWindowText(detail);
			SetItemVisible(IDC_DLG_UPDATE_DETAIL, TRUE);
		}
		else  /* maybe timeout */
		{
			if (hr == ERROR_WINHTTP_TIMEOUT)
				ShowWarnning(UPDATE_HIT_BUY);
			else
				ShowWarnning(UPDATE_HIT_FAILED2);

			ShowOfflineInstallHint();
		}
	}
	return TRUE;
}

LRESULT CUpdateVersionDlg::OnDownlaodFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = (HRESULT)wParam;

	if (SUCCEEDED(hr))
	{
		m_updater.Execute(); /* do install */
		EndDialog(0);
	}
	else 
	{
		if (hr == ERROR_WINHTTP_TIMEOUT)
			ShowWarnning(UPDATE_HIT_BUY);
		else
			ShowWarnning(UPDATE_HIT_FAILED1);

		ShowOfflineInstallHint();

		m_edtDetail.SetWindowText(_T(""));
		m_edtDetail.ShowWindow(SW_HIDE);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CManualUpdater


CManualUpdater::CManualUpdater() 
: m_hWnd(NULL), 
  m_hThread(NULL),
  m_fCompress(FALSE), 
  m_dwFileSize(0)
{
	m_BKSafeLog.SetLogForModule(BKSafeLog::enumBKSafeUpliveLog);
	m_hStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CManualUpdater::~CManualUpdater()
{
	if (m_hStopEvent)
		::CloseHandle(m_hStopEvent);

	if (m_hThread)
		::CloseHandle(m_hThread);
}

BOOL CManualUpdater::DownloadProcess(DWORD dwTotalSize, DWORD dwReadSize)
{
	if (dwTotalSize == 0)
		return TRUE;

	/* Stop the downlaod */
	if (WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
		return FALSE;

	if (m_hWnd)
	{
		/* Update progress */

		DWORD dwDegree = dwReadSize * 100 / dwTotalSize;
		::PostMessage(m_hWnd, WM_UPDATE_PROGRESS, 0, dwDegree);
	}
	return TRUE;
}

void CManualUpdater::Attach(HWND hWnd)
{
	m_hWnd = hWnd; 
}


void CManualUpdater::Start(UINT option)
{
	if (m_hWnd == NULL || m_hStopEvent == NULL)
		return;

	::ResetEvent(m_hStopEvent);

	if (m_hThread)
		::CloseHandle(m_hThread);

	m_uOption = option;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, WorkThread, this, 0, NULL);
}

void CManualUpdater::Stop()
{
	if (m_hStopEvent)
		::SetEvent(m_hStopEvent);

	if (m_hThread)
	{
		if (::WaitForSingleObject(m_hThread, 1000) != WAIT_OBJECT_0)
			::TerminateThread(m_hThread, 0);

		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

unsigned CManualUpdater::WorkThread(void* param)
{
	CManualUpdater* pSelf = (CManualUpdater*)param;
	return pSelf->DoWork();
}

unsigned CManualUpdater::DoWork()
{
	if (m_uOption == CHECKVER)
	{
		BOOL fFoundNew = FALSE;
		HRESULT hr = CheckNewVersion(&fFoundNew);

		/* Don't send the message if interrupt by user */
		if (hr != E_BKUPCTL_INTERRUPT)
		{
			m_BKSafeLog.WriteLog(L"CheckNewVersion: Error with 0x%x, bFount = %d", hr, fFoundNew);
			::SendMessage(m_hWnd, WM_UPDATE_CHECK_VERSION, hr, fFoundNew);
		}
	}
	else if (m_uOption == DOWNLAOD)
	{
		HRESULT hr = DownloadInstallPack();

		/* Don't send the message if interrupt by user */
		if (hr != E_BKUPCTL_INTERRUPT) 
			::SendMessage(m_hWnd, WM_UPDATE_DOWNLOAD_FINISH, hr, 0);
	}
	return 0;
}


HRESULT CManualUpdater::ParseCfg(LPCTSTR lpCfgName)
{
	BKIni::Document ini;

	HRESULT hr = ini.LoadFile(lpCfgName);

	if (FAILED(hr))
		return hr;

	if (!ini.HasSection(L"lastversion1"))
		return E_FAIL;

	BKIni::Section section = ini[L"lastversion1"];
	LPCTSTR lpCurChannel = L"";//hub 2011.0308

	if (section.HasKey(CT2W(lpCurChannel)))
		m_strNewVersion = section[CT2W(lpCurChannel)].GetValueString();
	else if (section.HasKey(L"h_home"))
		m_strNewVersion = section[L"h_home"].GetValueString();
	else
		return E_FAIL;

	if (!ini.HasSection(m_strNewVersion))
		return E_FAIL;

	section = ini[m_strNewVersion];
	
	if (!section.HasKey(L"url")      ||
		!section.HasKey(L"filename") ||
		!section.HasKey(L"crc")      ||
		!section.HasKey(L"filesize"))
		return E_FAIL;

	m_details.clear();

	m_strUrl     = section[L"url"].GetValueString();
	m_strExeName = section[L"filename"].GetValueString();
	m_strCrc     = section[L"crc"].GetValueString();
	m_dwFileSize = section[L"filesize"].GetValueDWORD();

	for (int i=1; i<100; ++i)
	{
		wchar_t szItem[10] = {0};
		wsprintf(szItem, L"detail%d", i);

		if (!section.HasKey(szItem))
			break;

		m_details.push_back(section[szItem].GetValueString());
	}

	if (section.HasKey(L"compress"))
		m_fCompress = (section[L"compress"].GetValueDWORD() != 0);

	if (section.HasKey(L"paramete"))
		m_strParamete = section[L"paramete"].GetValueString();

	return S_OK;
}


/** 
* @brief Check new version
* @param pfFoundNew - TRUE if found new version
*                     FALSE if don't need update it
*/		
HRESULT CManualUpdater::CheckNewVersion(BOOL *pfFoundNew)
{
	CBkUpdDownloader downloader(TRUE);

	// 支持本地http代理 [1/12/2011 zhangbaoliang]
	downloader.UseBKSafeProxyInfo(PROXY_TYPE::PROXY_TYPE_HTTP);

	CBkUpdWebFile file;

	if (pfFoundNew == NULL)
		return E_INVALIDARG;

	*pfFoundNew = FALSE;
	
	/* Get path of download */

	CString strUpdatePath;
	HRESULT hr = CAppPath::Instance().GetLeidianLogPath(strUpdatePath);

	if (FAILED(hr))
	{
		m_BKSafeLog.WriteLog(L"CheckNewVersion: 获取日志文件路径失败");
		return hr;
	}

	strUpdatePath += _T("\\update");

	if (::GetFileAttributes(strUpdatePath) == INVALID_FILE_ATTRIBUTES)
	{
		TCHAR szTempPath[MAX_PATH + 1] = {0};

		if (::GetTempPath(MAX_PATH, szTempPath) == 0)
		{
			m_BKSafeLog.WriteLog(L"CheckNewVersion: GetTempPath Error");
			return HRESULT_FROM_WIN32(::GetLastError());
		}
		
		strUpdatePath = szTempPath;
	}

	strUpdatePath += _T("\\ksafeupdate.ini");
	file.SetFileName(strUpdatePath);

	/* Download the configure */

	hr = downloader.Download(UPDATE_INI_URL, (IBkUpdWebFile*)&file, this);
	m_BKSafeLog.WriteLog(L"CheckNewVersion: DownloadIni hr = 0x%x", hr);

	if (SUCCEEDED(hr))
	{
		/* parse the cfg */
		hr = ParseCfg(strUpdatePath);

		if (FAILED(hr))
		{
			m_BKSafeLog.WriteLog(L"CheckNewVersion: ParseCfg hr = 0x%x", hr);
			return hr;
		}

		/* Get current version */
		LPCTSTR lpCurVer = _Module.GetProductVersion();

		/* Compare version */
		if (VersionLess(lpCurVer, m_strNewVersion))
		{
			*pfFoundNew = TRUE;
			return hr;
		}
		m_strNewVersion = lpCurVer;
	}
	return hr;
}

DWORD CManualUpdater::CalcFileCRC(LPCTSTR lpFileName)
{
	HANDLE hFile = ::CreateFile(lpFileName, GENERIC_READ, 
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	BYTE byBuffer[1024] = {0};
	DWORD dwCRC = 0;
	DWORD dwFileSize = ::GetFileSize(hFile, NULL);

	if (dwFileSize == m_dwFileSize) /* check size of file */
	{
		while (dwFileSize > 0)
		{
			DWORD dwRead = 0;

			if (!::ReadFile(hFile, byBuffer, sizeof(byBuffer), &dwRead, NULL))
				break;

			dwFileSize -= dwRead;
			dwCRC = CRC32(dwCRC, byBuffer, dwRead);
		}
	}

	::CloseHandle(hFile);
	return dwCRC;
}

BOOL CManualUpdater::VerifyFileSign(LPCTSTR lpFileName)
{
	CAtlArray<CString> singer;

	singer.Add(_T("Zhuhai  Kingsoft Software Co.,Ltd"));
	singer.Add(_T("Kingsoft Security Co.,Ltd"));

	WinMod::CWinTrustVerifier verifier;
	verifier.TryLoadDll();

	HRESULT hr;
	WinMod::CWinTrustSignerInfoChain SingerInfo;
	DWORD dwWinTrust = verifier.VerifyFile(CT2W(lpFileName), &SingerInfo);

	if (dwWinTrust <= 0x0000FFFF)
		hr = AtlHresultFromWin32(dwWinTrust);
	else
		hr = dwWinTrust;

	if (SUCCEEDED(hr))
	{
		if (!SingerInfo.m_chain.IsEmpty())
		{
			for (int i=0; i<singer.GetCount(); i++)
			{
				if (0 == SingerInfo.m_chain.GetHead().m_strIsserName.Compare(singer[i]))
					return TRUE;
			}
		}
	}
	return FALSE;
}

HRESULT CManualUpdater::DownloadInstallPack()
{
	/* get path of download */

	CString strUpdatePath;
	HRESULT hr = CAppPath::Instance().GetLeidianLogPath(strUpdatePath);

	if (FAILED(hr))
		return hr;

	TCHAR szPackPath[MAX_PATH + 1] = {0};
	if (::GetTempFileName(strUpdatePath, _T("upd"), 0, szPackPath) == 0)
		return  HRESULT_FROM_WIN32(GetLastError());

	/* dowlaod a pack */

	CBkUpdDownloader downloader(TRUE);

	// 支持本地http代理 [1/12/2011 zhangbaoliang]
	downloader.UseBKSafeProxyInfo(PROXY_TYPE::PROXY_TYPE_HTTP);

	CBkUpdWebFile pack;

	hr = pack.SetFileName(szPackPath);
	hr = downloader.Download(m_strUrl, (IBkUpdWebFile*)&pack, this);

	if (SUCCEEDED(hr))
	{
		/* get file name of exectue */
		m_strCmd.Format(_T("%s\\%s"), strUpdatePath, m_strExeName);

		if (m_fCompress)
		{
			/* uncompress */

			hr = CZlib::Instance().InitZlib();

			if (SUCCEEDED(hr))
			{
				hr = CZlib::Instance().DecodeFile(szPackPath, m_strCmd, m_dwFileSize);
				CZlib::Instance().UninitZlib();
				::DeleteFile(szPackPath);
			}
		}
		else
		{		
			/* rename */
			if (!::MoveFileEx(szPackPath, m_strCmd, MOVEFILE_REPLACE_EXISTING))
				hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (SUCCEEDED(hr))
		{
			/* verify this file's crc and sign */

			DWORD dwCRC = 0;
			_stscanf(m_strCrc, _T("%x"), &dwCRC);

			if (CalcFileCRC(m_strCmd) != dwCRC || 
				!VerifyFileSign(m_strCmd)) 
			{
				::DeleteFile(m_strCmd);
				hr = E_FAIL;
			}
		}
	}

	if (FAILED(hr))
	{
		/* delete this pack if failed */
		::DeleteFile(szPackPath);
	}
	return hr;	
}

void CManualUpdater::Execute()
{
	if (m_strCmd.IsEmpty())
		return;

	if (!m_strParamete.IsEmpty())
		m_strCmd.Format(_T("%s %s"), m_strCmd, m_strParamete);

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

	if (::CreateProcess(NULL, m_strCmd.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

int CManualUpdater::GetNumberOfDetail()
{
	return (int)m_details.size();
}

const CString& CManualUpdater::GetDetailItem(int index)
{
	if (index < m_details.size())
		return m_details[index];

	return _T("");
}

const CString& CManualUpdater::GetVersion()
{
	return m_strNewVersion;
}

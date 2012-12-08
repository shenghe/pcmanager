#include "stdafx.h"
#include "bkdownloadsafedlg.h"
#include "kdrawframe.h"
#include <atlpath.h>
#include "kws/KwsSettingOld.h"

#define  DLSP_DLL_NAME   _T("pedownui.dll")

// 定时更新界面
#define KWS_TIMER_UPDATEUI		    (WM_USER + 4001)
#define KWS_TIMER_UPDATEUITIME       1000
#if 0
CBKDownloadSafeDlg::CBKDownloadSafeDlg()
: m_bAutoScanEnable(FALSE)
, m_hBkIcon(NULL)
, m_hCursor(NULL)
, m_dwBlockFileCount(0)
, m_dwUnknowFileCount(0)
{
	
}

CBKDownloadSafeDlg::~CBKDownloadSafeDlg()
{
	if (m_hBkIcon)
	{
		::DestroyIcon(m_hBkIcon);
		m_hBkIcon = NULL;
	}

	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
}

LRESULT CBKDownloadSafeDlg::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{	
	_CreateFont();
	_GetImgPath();
	_LoadBmp();	
	KDrawFrame::Instance().Init();	
	_DrawCloseBtn();	
	_DrawOpenFileScanBtn();
	_InitDLSP();

	CenterWindow();
	SetTimer(KWS_TIMER_UPDATEUI, KWS_TIMER_UPDATEUITIME);

	return TRUE;
}

LRESULT CBKDownloadSafeDlg::OnHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt;
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	::ScreenToClient(this->m_hWnd, &pt);

	CRect rctCaption = _GetCaptionRect();
	CRect rctSettting = _GetSettingRect();
	if (rctCaption.PtInRect(pt) && !rctSettting.PtInRect(pt))	
		return HTCAPTION;	
	else
		return HTCLIENT;	
}

LRESULT CBKDownloadSafeDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (wParam == KWS_TIMER_UPDATEUI)
	{
		_ChkFileAutoScan();

		DWORD dwBlockFileCount = 0, dwUnknowFileCount = 0;
		if (m_DownloadSafePage.GetPlugin())
		{
			m_DownloadSafePage.GetPlugin()->GetBlockVirusCount(dwBlockFileCount);	
			m_DownloadSafePage.GetPlugin()->GetGrayFileCount(dwUnknowFileCount);

			if ((dwBlockFileCount != m_dwBlockFileCount) || (dwUnknowFileCount != m_dwUnknowFileCount))
			{
				m_dwBlockFileCount = dwBlockFileCount;
				m_dwUnknowFileCount = dwUnknowFileCount;
				Invalidate();
			}
		}
	}

	return TRUE;
}

LRESULT CBKDownloadSafeDlg::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt;
	ATL::CString strTest;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);		
	
	CRect rctSetting = _GetSettingRect();
	if (rctSetting.PtInRect(pt))
	{
		OutputDebugStringA("设置被点击");
	}
	return TRUE;
}



LRESULT CBKDownloadSafeDlg::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	CPoint pt;

	::GetCursorPos(&pt);
	::ScreenToClient(this->m_hWnd, &pt);

	CRect rctSetting = _GetSettingRect();
	if (rctSetting.PtInRect(pt))
	{
		bHandled = TRUE;
		if( !m_hCursor )
			m_hCursor = (HCURSOR)::LoadCursor(NULL, IDC_HAND);
		// ::SetCursor(m_hCursor);
	}
	else
		bHandled = FALSE;

	return TRUE;
}

LRESULT CBKDownloadSafeDlg::OnBtnClose(WORD, WORD id, HWND hWnd, BOOL& /*bHandled*/)
{
	/*DestroyWindow();*/
	ShowWindow(SW_HIDE);

	return TRUE;
}

LRESULT CBKDownloadSafeDlg::OnBtnOpenFileScan(WORD, WORD id, HWND hWnd, BOOL& )
{
	kws_old_ipc::KwsSetting aSetting;
	aSetting.SetAutoScanEnable(TRUE);

	return S_OK;
}

LRESULT CBKDownloadSafeDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	/*::PostQuitMessage(0);*/

	return TRUE;
}

void CBKDownloadSafeDlg::DoPaint(CDCHandle dc)
{
	_DrawBkg(dc);
	_DrawContent(dc);
	_DrawBmp(dc);
}

CRect CBKDownloadSafeDlg::GetWinPos()
{
	CRect rc;
	rc.left = 0;
	rc.right = m_PageRect.left + 805;
	rc.top = 0;
	rc.bottom = m_PageRect.top + 547;

	return rc;
}

#define PLUGIN_MARGEN_L		10
#define PLUGIN_MARGEN_T		95
#define PLUGIN_MARGEN_R		795
#define PLUGIN_MARGEN_B		538

void CBKDownloadSafeDlg::_InitDLSP()
{
	m_PageRect.left = PLUGIN_MARGEN_L+1;
	m_PageRect.right = PLUGIN_MARGEN_R-1;
	m_PageRect.top = PLUGIN_MARGEN_T+3;
	m_PageRect.bottom = PLUGIN_MARGEN_B-1;

	if (!m_DownloadSafePage.GetPlugin(  ) )
	{		
		m_DownloadSafePage.Init( DLSP_DLL_NAME );
		if (m_DownloadSafePage.GetPlugin())
		{
			m_DownloadSafePage.GetPlugin()->Create(m_hWnd, m_PageRect);
			m_DownloadSafePage.GetPlugin()->ShowWindow(SW_SHOW);
		}
	}
}



void CBKDownloadSafeDlg::_DrawBkg(CDCHandle dc)
{
	CRect rectClient;
	COLORREF crl = RGB(0xFB, 0xFC, 0xFE);
	GetClientRect(&rectClient);
	KDrawFrame::Instance().Draw(dc, rectClient, crl);

	CBrush brush;
	brush.CreateSolidBrush(RGB(255,255,255));
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0xA3, 0xBB, 0xD3));
	HPEN hOldPen = dc.SelectPen(pen);
	HBRUSH hOldBrush = dc.SelectBrush(brush);
	// 绘制背景
	CRect rct(PLUGIN_MARGEN_L, PLUGIN_MARGEN_T, PLUGIN_MARGEN_R, PLUGIN_MARGEN_B);
	dc.Rectangle(&rct);	

	dc.SelectBrush(hOldBrush);
	dc.SelectPen(hOldPen);
}

#define TEXT_HINT_Y	48
#define TEXT_HINT_Y2	68

void CBKDownloadSafeDlg::_DrawContent(CDCHandle dc)
{
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
	HBRUSH hOldBrush = dc.SelectBrush((HBRUSH)::GetStockObject(NULL_BRUSH));
	HPEN hOldPen = dc.SelectPen(pen);

	dc.SelectPen(hOldPen);
	dc.SelectBrush(hOldBrush);

	HFONT hOldFont = dc.SelectFont(m_fontTitle);
	DWORD nBkMode = dc.SetBkMode(TRANSPARENT);
	COLORREF clr = dc.SetTextColor(RGB(255, 255, 255));	

	if (KisPublic::Instance()->Init())
	{
		int nColor = 0;
		KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_DownProtectedColor, nColor);

		dc.SetTextColor(nColor);	
	}

	dc.TextOut(28, 9, BkString::Get(10021));	

	dc.SelectFont(m_fontUrl);
	// dc.TextOut(715,5, TEXT("设置"));
	
	if (m_bAutoScanEnable)
	{
		ATL::CString strBlockFileCount, strUnkownFileCount;
		// DWORD dwBlockFileCount = 0, dwUnknowFileCount = 0;
		if (m_DownloadSafePage.GetPlugin())
		{
			m_DownloadSafePage.GetPlugin()->GetBlockVirusCount(m_dwBlockFileCount);	
			m_DownloadSafePage.GetPlugin()->GetGrayFileCount(m_dwUnknowFileCount);
		}
		strBlockFileCount.Format(TEXT("%d"), m_dwBlockFileCount);
		strUnkownFileCount.Format(TEXT("%d"), m_dwUnknowFileCount);
		dc.SelectFont(m_fontBold);

		int x = 80, y = TEXT_HINT_Y2;
		{
			dc.SetTextColor(RGB(0x75, 0xB7, 0xD6));
			LPCWSTR str = TEXT("拦截病毒下载");
			dc.TextOut(x, y, str);
			SIZE sizeTxt = {0};
			dc.GetTextExtentExPoint(str, wcslen(str), &sizeTxt, 400, 0, 0);
			x += sizeTxt.cx;
		}

		{
			dc.SetTextColor(RGB(0xED, 0x5, 0x10));
			LPCWSTR str = strBlockFileCount;
			dc.TextOut(x, y, str);
			SIZE sizeTxt = {0};
			dc.GetTextExtentExPoint(str, wcslen(str), &sizeTxt, 400, 0, 0);
			x += sizeTxt.cx;
		}
		{
			dc.SetTextColor(RGB(0x75, 0xB7, 0xD6));
			LPCWSTR str = TEXT("次，鉴定文件");
			dc.TextOut(x, y, str);
			SIZE sizeTxt = {0};
			dc.GetTextExtentExPoint(str, wcslen(str), &sizeTxt, 400, 0, 0);
			x += sizeTxt.cx;
		}

		{
			dc.SetTextColor(RGB(0xED, 0x5, 0x10));
			LPCWSTR str = strUnkownFileCount;
			dc.TextOut(x, y, str);
			SIZE sizeTxt = {0};
			dc.GetTextExtentExPoint(str, wcslen(str), &sizeTxt, 400, 0, 0);
			x += sizeTxt.cx;
		}
		
		{
			dc.SetTextColor(RGB(0x75, 0xB7, 0xD6));
			LPCWSTR str = TEXT("个");
			dc.TextOut(x, y, str);
			SIZE sizeTxt = {0};
			dc.GetTextExtentExPoint(str, wcslen(str), &sizeTxt, 400, 0, 0);
			x += sizeTxt.cx;
		}

		dc.SelectFont(m_fontNormal);
		dc.SetTextColor(RGB(0x0, 0x0, 0x0));
		dc.TextOut(80, TEXT_HINT_Y, TEXT("已经开启下载保护功能，防止您下载文件时感染病毒木马"));

		// m_btnOpenFileScan.ShowWindow(SW_HIDE);
	}
	else
	{
		dc.SelectFont(m_fontBold);
		dc.SetTextColor(RGB(0xED, 0x5, 0x10));
		dc.TextOut(80, TEXT_HINT_Y, TEXT("您尚未开启下载保护功能，可能在下载文件时感染病毒木马"));
		dc.SelectFont(m_fontNormal);
		dc.SetTextColor(RGB(0x74, 0x74, 0x74));
		dc.TextOut(80, TEXT_HINT_Y2, TEXT("建议您立即开启下载保护功能，保护您的下载安全"));

		// m_btnOpenFileScan.ShowWindow(SW_SHOW);
	}

	dc.SelectFont(hOldFont);
	dc.SelectBrush(hOldBrush);
	dc.SelectPen(hOldPen);
}

void CBKDownloadSafeDlg::_DrawBmp(CDCHandle dc)
{
	CSize sizeBmp;
	CDC hTmpDC;

	if ( m_hBkIcon )
		dc.DrawIconEx(8, 7, m_hBkIcon, 16, 16, 0, 0, DI_NORMAL);
	
	hTmpDC.CreateCompatibleDC(dc);
	HBITMAP bOldbmp = hTmpDC.SelectBitmap(m_hBannerHint);	

	m_hBannerHint.GetSize(sizeBmp);	
	dc.BitBlt(30, 48, abs(sizeBmp.cx), abs(sizeBmp.cy), 
		hTmpDC, 0, 0, SRCCOPY);	

	m_hSettingLogoBmp.GetSize(sizeBmp);
	hTmpDC.SelectBitmap(m_hSettingLogoBmp);
	/*
	dc.BitBlt(700, 5, abs(sizeBmp.cx), abs(sizeBmp.cy), 
		hTmpDC, 0, 0, SRCCOPY);	
	*/

	hTmpDC.SelectBitmap(bOldbmp);
}

void CBKDownloadSafeDlg::_DrawCloseBtn()
{
	CRect rectClose = _GetCloseBtnRect();
	m_btnClose.Create(this->m_hWnd, &rectClose, NULL, 0, 0, IDC_BTN_CLOSE);
	m_btnClose.SetBmp(ATL::CString(m_strImgPath+TEXT("gb_normal.bmp")),
		ATL::CString(m_strImgPath+TEXT("gb_down.bmp")),
		ATL::CString(m_strImgPath+TEXT("gb_hover.bmp")));
	m_btnClose.SetBtnCursor(IDC_HAND);
}

void CBKDownloadSafeDlg::_DrawOpenFileScanBtn()
{
	CRect rectDownFileScan = _GetFileScanBtnRect();
	m_btnOpenFileScan.Create(this->m_hWnd, &rectDownFileScan, NULL, 0, 0, IDC_BTN_OPENFILESCAN);
	m_btnOpenFileScan.SetBmp(ATL::CString(m_strImgPath+TEXT("button_18_normal.bmp")),
		ATL::CString(m_strImgPath+TEXT("button_18_down.bmp")),
		ATL::CString(m_strImgPath+TEXT("button_18_hover.bmp")));
	m_btnOpenFileScan.SetBtnCursor(IDC_HAND);
	m_btnOpenFileScan.SetBtnText(TEXT("立即开启"));
	m_btnOpenFileScan.SetFont(&m_fontNormal);
}

void CBKDownloadSafeDlg::_CreateFont()
{
	LOGFONT ft = {0};
	HFONT hDefaultFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	::GetObject(hDefaultFont, sizeof(ft), &ft);

	wcscpy_s(ft.lfFaceName, LF_FACESIZE, TEXT("宋体"));
	ft.lfHeight = -12;
	m_fontNormal.CreateFontIndirect(&ft);

	ft.lfUnderline = TRUE;
	ft.lfHeight = -12;
	m_fontUrl.CreateFontIndirect(&ft);

	ft.lfUnderline = FALSE;
	ft.lfHeight = -12;
	ft.lfWeight = 650;
	m_fontBold.CreateFontIndirect(&ft);

	ft.lfUnderline = FALSE;
	ft.lfHeight = -12;
	ft.lfWeight = 700;
	m_fontTitle.CreateFontIndirect(&ft);
}

void CBKDownloadSafeDlg::_LoadBmp()
{	
	m_hBannerHint.LoadBitmap(IDB_KWS_DLSDLG_BANNERHINT);
	m_hSettingLogoBmp.LoadBitmap(IDB_KWS_DLSDLG_SETTINGLOGO);

	int nBig = 0;

	if (KisPublic::Instance()->Init())
	{
		BOOL bRet = KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_IconBig, nBig);
	}

	if (nBig > 0)
		m_hBkIcon = SetIcon(::LoadIcon((HINSTANCE)&__ImageBase, MAKEINTRESOURCE(nBig)));
	else
		m_hBkIcon = SetIcon(::LoadIcon((HINSTANCE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));

	if (m_hBkIcon == NULL)
		m_hBkIcon = SetIcon(::LoadIcon((HINSTANCE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));
	
	/*_LoadImg(m_hLogoBmp, ATL::CString(m_strImgPath + TEXT("erji_08.bmp")));*/
}

void CBKDownloadSafeDlg::_LoadImg(CBitmap& hBmp, LPCTSTR strBmp)
{
	HBITMAP handle = (HBITMAP)::LoadImage( NULL,
		strBmp, IMAGE_BITMAP, 
		0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	if (handle)
	{
		hBmp.Attach(handle);
	}
}

void CBKDownloadSafeDlg::_GetImgPath()
{
	TCHAR tszPath[MAX_PATH*2] = {0};
	GetModuleFileName( (HINSTANCE)&__ImageBase, tszPath, MAX_PATH * 2 );
	CPath path( tszPath );
	path.RemoveFileSpec();
	path.Append( TEXT("webui\\splock\\images\\") );

	m_strImgPath = ATL::CString(path);
}

void CBKDownloadSafeDlg::_ChkFileAutoScan()
{
	kws_old_ipc::KwsSetting aSetting;
	BOOL bDownFileScan = aSetting.IsAutoScanEnable();

	BOOL bRes = (m_bAutoScanEnable != bDownFileScan);
	m_bAutoScanEnable = bDownFileScan;
	if (bRes)
	{
		if (m_bAutoScanEnable)
			m_btnOpenFileScan.ShowWindow(SW_HIDE);
		else
			m_btnOpenFileScan.ShowWindow(SW_SHOW);
		Invalidate();
	}
}

CRect CBKDownloadSafeDlg::_GetCloseBtnRect()
{
	CRect rc;
	GetClientRect(&rc);
	rc.right -= 1;
	rc.left = rc.right - 43;
	rc.top = 0;
	rc.bottom = rc.top + 21;

	return rc;
}

CRect CBKDownloadSafeDlg::_GetFileScanBtnRect()
{
	CRect rc;
	GetClientRect(&rc);
	rc.left = 480;
	rc.right = rc.left + 83;
	rc.top = 45;
	rc.bottom = rc.top + 27;

	return rc;
}

CRect CBKDownloadSafeDlg::_GetCaptionRect()
{
	CRect rc;
	GetClientRect(&rc);	
	rc.top = 0;
	rc.bottom = rc.top + 28;

	return rc;
}

CRect CBKDownloadSafeDlg::_GetSettingRect()
{
	CRect rc;
	GetClientRect(&rc);
	rc.left = 715;
	rc.right = rc.left + 15;
	rc.top = 0;
	rc.bottom = rc.top + 28;

	return rc;
}

void CBKDownloadSafeDlg::OnSysCommand(UINT nID, CPoint point)
{
	SetMsgHandled(FALSE);

	switch (nID & 0xFFF0)
	{
	case SC_CLOSE:
		SetMsgHandled(TRUE);
		break;
	}
}

#endif

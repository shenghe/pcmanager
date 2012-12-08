// the main dlg for download safe dlg

#pragma once
#include <atlbase.h>
#include <wtl/atlapp.h>
#include <atlwin.h>
#include <wtl/atlframe.h>
#include <wtl/atlmisc.h>
#include <bkres/bkres.h>
#include "kws/KPluginLoader.h"
#include "plugin/pedownplugindef.h"
#include "bmpbtn.h"

#define IDC_BTN_CLOSE          100
#define IDC_BTN_OPENFILESCAN   101
#if 0
class CBKDownloadSafeDlg : public CWindowImpl<CBKDownloadSafeDlg>,
						public CDoubleBufferImpl<CBKDownloadSafeDlg>
{
public:
	CBKDownloadSafeDlg();
	~CBKDownloadSafeDlg();

public:

	BEGIN_MSG_MAP(CBKDownloadSafeDlg)
		CHAIN_MSG_MAP(CDoubleBufferImpl<CBKDownloadSafeDlg>)

		MESSAGE_HANDLER(WM_CREATE,         OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,		   OnDestroy)
		MESSAGE_HANDLER(WM_NCHITTEST,      OnHitTest)
		MESSAGE_HANDLER(WM_TIMER,          OnTimer)
		MESSAGE_HANDLER(WM_LBUTTONUP,      OnLButtonUp)
		MESSAGE_HANDLER(WM_SETCURSOR,      OnSetCursor)
		COMMAND_ID_HANDLER(IDC_BTN_CLOSE,  OnBtnClose)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		COMMAND_ID_HANDLER(IDC_BTN_OPENFILESCAN, OnBtnOpenFileScan)
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT	OnBtnClose(WORD, WORD id, HWND hWnd, BOOL& /*bHandled*/);
	LRESULT	OnBtnOpenFileScan(WORD, WORD id, HWND hWnd, BOOL& /*bHandled*/);

	void OnSysCommand(UINT nID, CPoint point);

	void DoPaint(CDCHandle /*dc*/);
	CRect GetWinPos();	

private:
	void    _InitDLSP();
	void	_DrawBkg(CDCHandle dc);
	void	_DrawContent(CDCHandle dc);
	void    _DrawBmp(CDCHandle dc);
	void    _DrawCloseBtn();
	void    _DrawOpenFileScanBtn();

	void	_CreateFont();
	void	_LoadBmp();
	void    _GetImgPath();
	void    _ChkFileAutoScan();
	void	_LoadImg(CBitmap& hBmp, LPCTSTR strBmp);

	CRect   _GetCloseBtnRect();
	CRect   _GetFileScanBtnRect();
	CRect   _GetCaptionRect();
	CRect   _GetSettingRect();

private:

	KPluginLoader<IKDonwloadUIPlugIn>	m_DownloadSafePage;
	ATL::CString m_strImgPath;

	CRect        m_PageRect;
	CBitmap      m_hSettingLogoBmp;
	CBitmap      m_hBannerHint;
	HICON        m_hBkIcon;
	HCURSOR      m_hCursor;
	CBmpBtn      m_btnClose;
	CBmpBtn      m_btnOpenFileScan;

	CFont	     m_fontTitle;
	CFont	     m_fontNormal;
	CFont        m_fontBold;
	CFont        m_fontUrl;

	BOOL         m_bAutoScanEnable;	
	DWORD		 m_dwBlockFileCount;
	DWORD		 m_dwUnknowFileCount;
};
#endif

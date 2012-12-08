#pragma once
#include "bkres/bkres.h"
#include "kdownfile.h"
#include <wtlhelper/whwindow.h>
#include "kdownfile.h"
//#include "KLocker.h"

#define		WM_FINISHMEASURESPEED			WM_USER + 1000 //测速完成后，发送消息
#define		WM_FINISHMEASUREPROCESS			WM_USER + 1001 //发送速度消息


class KMainDlg : public CBkDialogImpl<KMainDlg> , 
	public IStatusCallback , 
	public CWHRoundRectFrameHelper<KMainDlg>
{
public:
    KMainDlg()
        : CBkDialogImpl<KMainDlg>(IDR_BK_NETSPEED_DIALOG)
    {
		m_fAvgSpeed = 0;
		m_nCurSpeed = 0;
		m_dwBegin	= 0;
		m_dwEnd		= 0;
		m_nCurTime	= 0;
		m_bIsFinished = FALSE;
		m_pLocker	= new KLockerCS;
    }

    ~KMainDlg();

    void			BtnClose(void);
	void			BtnCancel(void);
	void			BtnTryAgain(void);
	void			BtnCloseDialog(void);
	void			BtnCopyClipboard(void); //复制文件到剪切板
	void			BtnLnkOpenMonter(void); //打开
	void			OnSysCommand(UINT nID, CPoint pt);
	void			OnDestroy(void);
	void			OnTimer(UINT_PTR pUTimer);

    BOOL			OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
	virtual void	OnStatusCallback(ProcessState enumState, int64 nSpeed, int64 nAvgSpeed, int nProcess);
	LRESULT			OnSetFinishDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			OnSetProcess(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			OnCreate(LPCREATESTRUCT lp);
	

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(DEFCLOSEMAINBUTTON, BtnClose)
		BK_NOTIFY_ID_COMMAND(DEFCANCELBUTTON, BtnCancel)
		BK_NOTIFY_ID_COMMAND(DEFSHOWTRYAGAINBUTTON, BtnTryAgain)
		BK_NOTIFY_ID_COMMAND(DEFSHOWCLOSEBUTTON, BtnCloseDialog)
		BK_NOTIFY_ID_COMMAND(DEFSHOWCOPYTOBAN, BtnCopyClipboard)
		BK_NOTIFY_ID_COMMAND(DEFSHOWLINKOPENMONTER, BtnLnkOpenMonter)
    BK_NOTIFY_MAP_END()


    BEGIN_MSG_MAP_EX(KMainDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<KMainDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KMainDlg>)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_FINISHMEASURESPEED, OnSetFinishDown)
		MESSAGE_HANDLER(WM_FINISHMEASUREPROCESS, OnSetProcess)
		REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
	void		_ShowFristPage(IN BOOL bShowFirstPage);
	void		_SetDownSpeed(IN int nSpeed);
	void		_SetCurProcess(IN int nCurProcess);
	void		_GetSpeedType(IN int nAvgSpeed, CString& strBetween, CString& strRet, CString& strInfo);
	int			_GetProcessPos(IN int nAvgSpeed);
	
	CString		_GetNeedShowData(IN double fData);
	CString		_GetNeedShowDataAvg(IN double nData);
	CString		_GetIpAddr(void);
	CString		_GetMacAddr(void);
	CString		_ParseRichText(IN const CString& strRichText);
	void		_ShowTipInfo(void);

	void		_GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet);
	BOOL		_GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion);
	CString		GetVersionType(void);

private:
	double		m_fAvgSpeed;
	int			m_nCurSpeed;
	int			m_nCurTime;
	KDownFile	m_DownFile;
	CString		m_strClipboardText;
	CString		m_strVersion;
	DWORD		m_dwBegin;
	DWORD		m_dwEnd;
	BOOL		m_bIsFinished;
	ILockerMode* m_pLocker;
};
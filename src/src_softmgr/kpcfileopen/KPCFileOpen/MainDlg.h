#include "stdafx.h"
#include <bkwin/bklistview.h>
#include <bkres/bkres.h>
#include <wtlhelper/whwindow.h>
#include <softmgr\IDTManager.h>
#include <stlsoft/stlsoft.h>

#include <bksafe/bksafeconfig.h>
#include <common/utility.h>
#include "beikesafemsgbox.h"

#define TIMER_LOADINFO_BIGBTN			1204 

typedef BOOL (*pCreateObject)( REFIID , void** );

class CWebBrowserExternal : public CWHHtmlContainerWindow
{
public:
	HRESULT SetExternalDispatch(IDispatch* pDisp) 
	{ return m_wndIE.SetExternalDispatch(pDisp); }
};

class CMainDlg
	: public CBkDialogImpl<CMainDlg>
	, public CWHRoundRectFrameHelper<CMainDlg>
	, public IDispatch
	, public IWHHtmlContainerWindowCallback
{
public:
	CMainDlg()
		: CBkDialogImpl<CMainDlg>(IDR_BK_MAIN_DIALOG)
	{
		CreateObject = NULL;

		m_phoneLoaded = FALSE;
		m_pPhoneDTMgr = NULL;
	}

	~CMainDlg()
	{
	}

public:
	UINT_PTR DoModal(HWND hWndParent);

protected:
	void OnTimer(UINT_PTR nIDEvent);
	void OnDestroy();
	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
	void OnBkBtnClose();
	void OnBkBtnOK();
	void OnBkBtnDefault();
	void OnNavigateComplete(int id);

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDOK, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_OPEN_FILE_DEAULT, OnBkBtnDefault)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkDialogImpl<CMainDlg>)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CMainDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMainDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_TIMER(OnTimer)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:

	enum PhoneDownState
	{
		PDS_NONE = 0,
		PDS_INIT,
		PDS_DOWNING,
		PDS_DONE,
		PDS_PAUSED,
		PDS_ERROR_MD5,
		PDS_ERROR,
	};

	typedef struct PhoneSoft
	{
		void*			idDown;		// 下载标识
		PhoneDownState	state;		// 下载状态
		ULONG			speed;		// 速度
		ULONG			recved;		// 已下载字节数
	} *PPhoneSoft;

	typedef CAtlMap<void*, LONG> Dt2IdMap;
	typedef CAtlMap<LONG, PhoneSoft> Id2PhoneSoftMap;

	typedef Dt2IdMap::CPair* Dt2IdIter;
	typedef Id2PhoneSoftMap::CPair* Id2PhoneSoftIter;

	typedef CComCritSecLock<CComCriticalSection> CriticalSectionScoped;

private:
	BOOL				m_phoneLoaded;
	IDTManager			*m_pPhoneDTMgr;
	CWebBrowserExternal	m_IECtrl;
	CComCriticalSection	m_csPhoneNess;
	Dt2IdMap			m_dt2Id;
	Id2PhoneSoftMap		m_id2PhoneSoft;

	HINSTANCE			hInstance;
	pCreateObject		CreateObject;
	
	CString				m_strFileName;

protected:
	void RollTheTip();
	LONG safe_atol(LPCWSTR pStr, LONG def = 0);
	CString _PathAddBackslash(const CString &path);
	BOOL InitInterface();
	HRESULT GetKSoftDirectory(CString &strDir);
	BOOL CreateProcessSyn(CString strCmd);
	BOOL GetRegExtInfo(CString& strInfo);

public:
	// 获取下载接口（仅在必要时才创建下载管理对象）
	IDTManager* GetDTMgrForPhone();
	static void PhoneSoftDownInfoCallback(__int64 cur,__int64 all,__int64 speed,DWORD time,void* para);
	static void PhoneSoftDownCallback(DTManager_Stat st,void* tk,void* para);
	void PhoneSoftDownProcess(DTManager_Stat st, void *tk);

	// Js接口
	HRESULT PCSoftExists(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftStartDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftPauseDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftResumeDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftStopDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftGetDownState(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftOpenDownedSoft(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftRun(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PCSoftRunDefault(DISPID, DISPPARAMS*, VARIANT*);

private:
	// window.external实现接口
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return E_NOTIMPL; }
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
};

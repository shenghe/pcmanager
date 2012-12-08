#pragma once
#include "kws/ctrls/rulelistctrl.h"
#include "kws/util.h"
#include "kws/ctrls/URLWhiteList.h"
//设置广告过滤
#define SFP_MODULE_NAME		"kswebshield.dll"
#define SFP_FN_ANME			"GetUWL"
class CSetAdFilterPage 
	: public CBkDialogImpl<CSetAdFilterPage>
	,public CWHRoundRectFrameHelper<CSetAdFilterPage>
{
public:
	CSetAdFilterPage()
		: CBkDialogImpl<CSetAdFilterPage>(IDR_BK_SAFEKWS_ADFILTER_SETTING_DLG)
	{
	}

	~CSetAdFilterPage(void)
	{
	}
	typedef HRESULT (__cdecl *GetUWL_Fn)(IURLWhiteList **ppUWL);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{		
		return 0;
	}
	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		InitCtrls();
		UpdateConfig();
		return TRUE;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		UninitCtrls();
		return 0;
	}
	void OnBkBtnClose()
	{
		EndDialog(0);
		return;
	}
	void InitCtrls()
	{	
		m_rtList = CRect( 13, 198, 13 + 463, 198 + 169);
		m_wndListView.SetRowHeight( 24 );
		m_wndListView.Create( GetViewHWND(), m_rtList, NULL, 
			WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_OWNERDRAWFIXED, 
			0, IDC_LIST_ADFILTER_DLG);
		m_rtList.InflateRect( 1, 1 );
		m_wndListView.BringWindowToTop();
		m_wndListView.InsertColumn( 0, TEXT(""), LVCFMT_CENTER, 450 );
		m_wndListView.ShowWindow( SW_SHOWNORMAL ); 
		m_wndListView.SetExtendedListViewStyle ( LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE 
			| m_wndListView.GetExtendedListViewStyle() );

		m_wndEdit.Create( GetViewHWND(), 
			NULL,
			NULL,
			WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL,
			0,
			IDC_EDITBOX_ADFILTER_DLG);
		m_wndEdit.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		return;
	}
	void UpdateDelLink()
	{
		if( m_wndEdit.IsWindow() )
		{
			int nCnt = m_wndListView.GetItemCount();
			if (nCnt > 0)
			{
				//导入 导出  清空  全部为可选
				EnableItem( IDC_LINKTEXT_ADFILTER_CLEAR, TRUE );
			}
			else//除导入以外  其他全部为不可选状态
			{
				EnableItem( IDC_LINKTEXT_ADFILTER_DELETE, FALSE );			
				EnableItem( IDC_LINKTEXT_ADFILTER_CLEAR,  FALSE );
			}
			int nIndex = m_wndListView.GetSelectedIndex();
			if (nIndex != -1)
			{
				EnableItem( IDC_LINKTEXT_DELETE,  TRUE);
			}
			else
			{
				EnableItem( IDC_LINKTEXT_DELETE,  FALSE);
			}
		}
		
		return;
	}
	void LoadWhiteListAndDisplay()//加载白名单列表
	{
		IURLWhiteList *purlWhiteList = NULL;
		HRESULT hr = GetWhiteURLList(&purlWhiteList);
		if (FAILED(hr) || !purlWhiteList)
			return;

		LPCWSTR lpwzUrl = NULL;
		hr = purlWhiteList->First(&lpwzUrl);
		if (FAILED(hr) || !lpwzUrl)
			return;
		do 
		{
			if (lpwzUrl)
			{
				int nIndex = m_wndListView.GetItemCount();
				int nRet = m_wndListView.InsertItem( nIndex, CW2T(lpwzUrl) );
				LocalFree((HLOCAL)lpwzUrl);
			}
		} while (S_OK == purlWhiteList->Next(&lpwzUrl));
		return;
	}
	HRESULT GetWhiteURLList(IURLWhiteList **ppUWL)
	{
		CImportFunction<GetUWL_Fn> ifnGetUWL( SFP_MODULE_NAME, SFP_FN_ANME, TRUE);
		if (!ifnGetUWL)
			return E_POINTER;

		IURLWhiteList *purlWhiteList = NULL;
		HRESULT hr = ifnGetUWL(&purlWhiteList);
		if (FAILED(hr) || !purlWhiteList)
			return E_NOINTERFACE;

		*ppUWL = purlWhiteList;
		return NOERROR;
	}
	void UpdateConfig()
	{
		EnableItem( IDC_BTN_ADFILTER_ADD_ID, FALSE );
		EnableItem( IDC_LINKTEXT_ADFILTER_DELETE,  FALSE);
		//todo 显示所有白名单
		LoadWhiteListAndDisplay();
		UpdateDelLink();
		return;
	}
	void UninitCtrls()
	{
		if( m_wndListView.IsWindow() )
			m_wndListView.DestroyWindow();

		if( m_wndEdit.IsWindow() )
			m_wndEdit.DestroyWindow();
		return;
	}
	void OnBkBtnDel()//“删除”的链接
	{
		int nRet = 0;
		nRet = CBkSafeMsgBox2::ShowMultLine( L"您是否确定删除选中的网址？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING );

		if( nRet == IDCANCEL )
			return;

		int nIndex = m_wndListView.GetSelectedIndex( );

		m_wndListView.DeleteItem( nIndex );
		DoSettingApply();
		UpdateDelLink();
		return;
	}
	void OnBkBtnClear()
	{
		int nRet = 0;
		nRet = CBkSafeMsgBox2::ShowMultLine( L"您是否确定删除全部网址？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING );

		if( nRet == IDCANCEL )
			return;

		if( m_wndListView.IsWindow() )
		{
			m_wndListView.DeleteAllItems();
		}
		DoSettingApply();
		UpdateDelLink();
		return;
	}

	BOOL HasRepeatUrl(ATL::CString& str)//判断是否有重复网址
	{
		BOOL bRet = FALSE;
		for(int i=0;i< m_wndListView.GetItemCount();i++)
		{
			CString url;
			m_wndListView.GetItemText(i, 0, url);
			if(url == str)
			{
				bRet = TRUE;
				break;
			}
		}
		return bRet;
	}
	void OnBkBtnAddUrl()//添加广告过滤白名单的网址
	{
		ATL::CString strUrl;
		m_wndEdit.GetWindowText( strUrl );
		if( strUrl.GetLength() < 1 )
			return;

		// if user type an URL without scheme, use "http" as the default scheme.
		int nPosPathStart = strUrl.Find('/');
		int nPosSchemeStart = strUrl.Find(_T("://"));
		if (-1 == nPosSchemeStart || 
			((-1 != nPosPathStart) && (nPosSchemeStart > nPosPathStart)))
		{
			strUrl = _T("http://") + strUrl;
		}


		// extract main domain name from URL.
		CURLCracker uc(strUrl, CURLCracker::UCF_MAINDOMAIN);
		if (!uc.MainDomainName(NULL))
			return;
		ATL::CString strMainDomainName = uc.MainDomainName(NULL);

		if( !HasRepeatUrl( strMainDomainName ) )
		{
			if (CBkSafeMsgBox2::ShowMultLine( L"您是否确定添加这个网址？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING) == IDOK)
			{
				int nIndex = m_wndListView.GetItemCount();
				int nRet = m_wndListView.InsertItem( nIndex, strMainDomainName );
				strUrl = TEXT("");
				m_wndEdit.SetWindowText( strUrl );
				DoSettingApply();
			}
			
		}
		else
		{
			CBkSafeMsgBox2::ShowMultLine( L"您输入的网址已经存在，请重新输入！", L"金山卫士",  MB_OK|MB_ICONWARNING );
		}
		UpdateDelLink();
		return;
	}

	LRESULT OnListViewItemChange( int wParam, LPNMHDR lParam, BOOL& bHandled )
	{
		if( m_wndListView.IsWindow() )
		{
			int nIndex = m_wndListView.GetSelectedIndex();
			if (nIndex != -1)
			{
				EnableItem( IDC_LINKTEXT_ADFILTER_DELETE,  TRUE);
			}
			else
			{
				EnableItem( IDC_LINKTEXT_ADFILTER_DELETE,  FALSE);
			}
		}

		return TRUE;
	}

	LRESULT OnEditChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
	{
		if( m_wndEdit.IsWindow() )
		{
			ATL::CString strUrl;
			m_wndEdit.GetWindowText( strUrl );
			if (strUrl.GetLength() > 0)
			{
				EnableItem( IDC_BTN_ADFILTER_ADD_ID, TRUE );
			}
			else
			{
				EnableItem( IDC_BTN_ADFILTER_ADD_ID, FALSE );
			}
		}

		return TRUE;
	}
	BOOL DoSettingApply()//修改设置应用
	{
		IURLWhiteList *purlWhiteList = NULL;
		HRESULT hr = GetWhiteURLList(&purlWhiteList);
		if (FAILED(hr) || !purlWhiteList)
			return FALSE;

		purlWhiteList->Clear();
		int nCount = m_wndListView.GetItemCount();
		ATL::CString strUrl;
		for ( int i = 0; i < nCount; i++ )
		{
			m_wndListView.GetItemText( i, 0, strUrl );
			hr = purlWhiteList->Add( strUrl );
		}

		purlWhiteList->ApplyChanges();
		hr = purlWhiteList->Flush();
		if (FAILED(hr))
			return FALSE;
		return TRUE;
	}

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN);	
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,				 OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_ADFILTER_DELETE,	 OnBkBtnDel)
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_ADFILTER_CLEAR,	 OnBkBtnClear)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_ADFILTER_ADD_ID,		 OnBkBtnAddUrl)
		
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CSetAdFilterPage)	
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER(WM_CREATE,				OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,				OnDestroy)	
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER( IDC_LIST_ADFILTER_DLG,		 LVN_ITEMCHANGED,		OnListViewItemChange)
		COMMAND_HANDLER( IDC_EDITBOX_ADFILTER_DLG,	 EN_CHANGE,				OnEditChange)
		CHAIN_MSG_MAP(CBkDialogImpl<CSetAdFilterPage>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CSetAdFilterPage>)
	END_MSG_MAP()
private:
	KRuleListCtrl		m_wndListView;
	CEdit				m_wndEdit;
	CRect				m_rtList;
};
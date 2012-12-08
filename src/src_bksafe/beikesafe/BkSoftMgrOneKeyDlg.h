#pragma  once

// 快速装机，一键装机

#include <bkwin/bklistbox.h>
#include "beikesafesoftmgrHeader.h"
#include "bkmsgdefine.h"
#include "BkSoftMgrOneKeySelPlugDlg.h"
#include "SoftInstProxy.h"
#include "LockHelper.h"


#define HEIGHT_SELECT_SOFT			70
#define HEIGHT_INSTALL_SOFT			55
#define HEIGHT_INSTALLED_SOFT		85

#define WM_SOFT_OK_DOWN_DONE		(WM_APP+0x100)
#define WM_SOFT_OK_INST_DONE		(WM_APP+0x101)
#define WM_SOFT_OK_INST_PROGRESS	(WM_APP+0x102)

#define WM_CLOSE_DLG				(WM_USER+100)

extern CString CalcStrFormatByteSize(LONGLONG qdw);

class CBeikeSafeSoftmgrUIHandler;

class CBkSoftOneKeyDlg;

static	CBkSoftOneKeyDlg*	m_pThisCheck = NULL;
static	CLock				m_lockState;

class CBkSoftOneKeyDlg
	: public CBkDialogImpl<CBkSoftOneKeyDlg>
	, public CWHRoundRectFrameHelper<CBkSoftOneKeyDlg>
{
public:

	struct ONE_KEY_INST_RESULT
	{
	public:
		ONE_KEY_INST_RESULT()
		{
			bDownSuccess	= FALSE;
			nDownError	= 0;
		}

		BOOL				bDownSuccess;
		DWORD				nDownError;
		S_KSAI_INST_RESULT	instST;
	};

	enum StateSoftOneKeyInst
	{
		STATE_ONEKINST_INIT = 0,
		STATE_ONEKINST_RUNNING,
		STATE_ONEKINST_END
	};

	struct SOFT_INST_STATE
	{
	public:
		SOFT_INST_STATE()
		{
			downState = STATE_ONEKINST_INIT;
		}

		operator CString& ()
		{
			return strSoftID;
		}

		CString				strSoftID;
		StateSoftOneKeyInst	downState;
		CString				strDownPath;
		CString				strName;
	};

	struct SOFT_SELECT_STATE
	{
	public:
		SOFT_SELECT_STATE()
		{
			bExist = FALSE;
		}

		BOOL		bExist;
		CString		strSoftId;
		BOOL		bCheck;
		BOOL		bDefaCheck;
	};

	struct ARRAY_SOFT_SELECT
	{
		CString		strType;
		SOFT_SELECT_STATE	softx[3];
	};


	CBkSoftOneKeyDlg(
		  CBeikeSafeSoftmgrUIHandler* pUIHandler,
		  CSimpleArray<ONE_KEY_GROUP>& arrOneKey, 
		  CAtlMap<CString, CSoftListItemData*>& arrIdMap
		  )
		: CBkDialogImpl<CBkSoftOneKeyDlg>(IDR_BK_SOFTMGR_ONE_KEY_INST_DLG)
		, m_arrOneKey(arrOneKey)
		, m_arrSoftMap(arrIdMap)
		, m_pUIHandler(pUIHandler)
		, m_bDownComplete(FALSE)
		, m_nllDiskLeftSize(0)
	{
		{
			CSimpleLock2_(m_lockState);
			m_pThisCheck = this;
		}
	}

	virtual ~CBkSoftOneKeyDlg()
	{
		{
			CSimpleLock2_(m_lockState);
			m_pThisCheck = NULL;
		}
	}

	ULONGLONG GetPathDiskFreeSize(LPCTSTR lpPath)
	{
		CString		strpath = lpPath;

		if (strpath.GetLength() < 2 )
			return 0;

		strpath = strpath.Left(2);

		ULONGLONG	uFreeBytes = 0;
		GetDiskFreeSpaceEx(strpath,(PULARGE_INTEGER)&uFreeBytes,NULL,NULL );

		return uFreeBytes;
	}

	void GetDefaultInstDir(ULONGLONG nSize)
	{
		CString		strDefa;
		ULONGLONG	uulSize = 0;

		//  先获取系统目录啊目录
		TCHAR		szSysRoot[MAX_PATH]  = {0};
		GetSystemWindowsDirectory(szSysRoot,MAX_PATH);
		if (szSysRoot[0] != 0)
		{
			TCHAR	szRoot[] = L"A:\\";
			
			szRoot[0] = szSysRoot[0];
			if ( PathFileExists(szRoot) )
			{
				ULONGLONG	uFreeBytes = 0;
				GetDiskFreeSpaceEx(szRoot,(PULARGE_INTEGER)&uFreeBytes,NULL,NULL );
				if (uFreeBytes > nSize )
				{
					m_strInstDir = szRoot;
					m_strInstDir += L"Program Files";
					m_nllDiskLeftSize = uFreeBytes;
					return;
				}
			}
		}

		for (int i=0; i < 'Z'-'C' ; i++)
		{
			TCHAR	szRoot[] = L"C:\\";

			szRoot[0] += (TCHAR)i;

			if ( PathFileExists(szRoot) )
			{
				ULONGLONG	uFreeBytes = 0;
				GetDiskFreeSpaceEx(szRoot,(PULARGE_INTEGER)&uFreeBytes,NULL,NULL );

				if (i==0)
				{
					strDefa = szRoot;
					strDefa += L"Program Files";
					uulSize = uFreeBytes;
				}

				if (uFreeBytes > nSize )
				{
					m_strInstDir = szRoot;
					m_strInstDir += L"Program Files";
					m_nllDiskLeftSize = uFreeBytes;
					return;
				}
			}
		}

		m_strInstDir		= strDefa;
		m_nllDiskLeftSize	= uulSize;
		return;
	}

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		int nBig = 0;
		int nSmall = 0;

		if (KisPublic::Instance()->Init())
		{
			BOOL bRet = KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_IconBig, nBig);
			bRet = KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_IconSmall, nSmall);
		}

		if (nBig > 0 && nSmall > 0)
		{
			SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(nBig)));
			SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(nSmall)), FALSE);
		}
		else
		{
			SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));
			SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_SMALL)), FALSE);
		}


		if (m_pUIHandler->m_bInitInstProxy == FALSE)
		{
			// 如果安装中退出，这时m_softInstEng.Uninitialize()会消耗时间太久，所以争取只Initialize/Uninitialize一次

			m_pUIHandler->m_softInstEng.SetNotify(m_pUIHandler);
			m_pUIHandler->m_softInstEng.Initialize( _Module.GetAppDirPath() );
			m_pUIHandler->m_bInitInstProxy = TRUE;
		}

		m_listSelect.Create( this->GetViewHWND(), IDC_SOFT_ONEK_LIST_SELECT);
		m_listSelect.Load(IDR_BK_SOFTMGR_NECESS_ONE_KEY_SEL_TEMPLATE);
		m_listSelect.SetItemFixHeight(HEIGHT_SELECT_SOFT);

		m_listInstComplete.Create( this->GetViewHWND(), IDC_SOFT_ONEK_LIST_INSTED);
		m_listInstComplete.Load(IDR_BK_SOFTMGR_NECESS_ONE_KEY_INSTED_TEMPLATE);
		m_listInstComplete.SetItemFixHeight(HEIGHT_INSTALLED_SOFT);

		DWORD dwEditFlags = WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY | ES_NOHIDESEL;
		m_edtSelDir.Create( GetViewHWND(), NULL, NULL, dwEditFlags, WS_EX_NOPARENTNOTIFY, IDC_SOFT_ONEK_EDT_SEL_DIR);	
		m_edtSelDir.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_edtSelDir.SetBkColor( COL_DEFAULT_WHITE_BG );
		m_edtSelDir.SetTextColor( RGB(145, 145, 145) );
		//m_edtSelDir.EnableWindow(FALSE);
		m_edtSelDir.HideCaret();

		SetItemCheck(IDC_SOFT_ONEK_RADIO_BEGIN,TRUE);

		for ( int i=0; i <= IDC_SOFT_ONEK_RADIO_END-IDC_SOFT_ONEK_RADIO_BEGIN; i++ )
		{
			if (m_arrOneKey.GetSize() > i)
			{
				SetItemVisible(IDC_SOFT_ONEK_RADIO_BEGIN+i, TRUE);
				SetItemText(IDC_SOFT_ONEK_RADIO_BEGIN+i, m_arrOneKey[i].strGroupName);
			}
			else
			{
				SetItemVisible(IDC_SOFT_ONEK_RADIO_BEGIN+i, FALSE);
			}
		}

		GenerateSelList(0);
		m_listSelect.SetItemCount(m_curListArray.GetSize());
		RefreshCheckNumberSize();

		m_pDTManager = m_pUIHandler->GetDTManagerForOneKey();
		ATLASSERT(m_pDTManager!=NULL);

		CAtlMap<DtStateChangeNotifyCallBack,void*>* calls=new CAtlMap<DtStateChangeNotifyCallBack,void*>;
		calls->SetAt(DownloadCallBack, this);
		m_pDTManager->Init(calls);

		return TRUE;
	}

	VOID OnShowX( HWND hParWnd )
	{
		if (!IsWindowVisible())
		{
			CRect	rc;
			::GetWindowRect(hParWnd,&rc);
			CRect	thisRect;
			GetWindowRect(&thisRect);

			CRect	rcNew;

			rcNew.left	= rc.left + (rc.Width()-thisRect.Width())/2;
			rcNew.right	= rcNew.left + thisRect.Width();
			rcNew.top	= rc.top  + (rc.Height()-thisRect.Height())/2;
			rcNew.bottom= rcNew.top + thisRect.Height();

			MoveWindow(&rcNew,FALSE);
			ShowWindow(SW_SHOW);
		}
		
		if (IsIconic())
			SendMessage(WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);

		BringWindowToTop();
		SetFocus();
	}

	void GenerateInstList()
	{
		m_arrInstSoft.RemoveAll();
		for ( int i=0; i < m_curListArray.GetSize(); i++)
		{
			ARRAY_SOFT_SELECT&		sel = m_curListArray[i];

			for ( int kk = 0; kk < 3; kk++)
			{
				SOFT_SELECT_STATE&	sfst = sel.softx[kk];
				CSoftListItemData*	pData = GetSoftListItemDataByID(sfst.strSoftId);
				if (sfst.bCheck && sfst.bExist && pData && !pData->m_bSetup )
				{
					SOFT_INST_STATE	st;
					st.strSoftID = sfst.strSoftId;
					st.strName = pData->m_strName;
					m_arrInstSoft.Add(st);

				}
			}
		}
	}

	void GenerateSelList(DWORD nIndex)
	{
		if ( nIndex >= m_arrOneKey.GetSize() )
			return;

		ONE_KEY_GROUP&	group = m_arrOneKey[nIndex];

		BOOL bInKsafe = FALSE;
		TCHAR tszFilePath[MAX_PATH*2] = {0};
		::GetModuleFileName( NULL, tszFilePath, MAX_PATH*2 );
		LPCTSTR pszFileName = ATLPath::FindFileName( tszFilePath );
		bInKsafe = (_tcsicmp( pszFileName, TEXT("ksafe.exe")) == 0 );

		m_curListArray.RemoveAll();

		for ( int i=0; i < group.typeList.GetSize(); i++)
		{
			ARRAY_SOFT_SELECT		sel;
			ONE_KEY_TYPE&			okType = group.typeList[i];
			BOOL					bExist = FALSE;

			sel.strType	= okType.strTypeName;
			
			for ( int kk = 0, mm = 0; kk < min(okType.softList.GetSize(),3); kk++)
			{
				CString&			strSoftId	= okType.softList[kk].strSoftID;
				CSoftListItemData*	pData		= GetSoftListItemDataByID(strSoftId);
				if ( IsSoftIdAutoList(strSoftId) && pData )
				{
					if( bInKsafe )
					{
						if( pData->m_strSoftID == TEXT("60000411")
							|| pData->m_strName == TEXT("金山卫士") )
						{
							continue;
						}
					}

					sel.softx[mm].strSoftId		= okType.softList[kk].strSoftID;
					sel.softx[mm].bCheck		= okType.softList[kk].bCheck;
					sel.softx[mm].bDefaCheck	= okType.softList[kk].bCheck;
					sel.softx[mm].bExist		= TRUE;

					if (okType.softList[kk].strShowName.IsEmpty() )
						m_mapShowName[strSoftId]	= pData->m_strName;
					else
						m_mapShowName[strSoftId]	= okType.softList[kk].strShowName;


					mm++;
					bExist = TRUE;
				}
			}
			if (bExist)
				m_curListArray.Add(sel);
		}
	}

	void OnBkBtnClose()
	{
		OnBkBtnCancelAll();
	}

	CSoftListItemData*	GetSoftListItemDataByID(LPCTSTR lpstrID)
	{
		if ( m_arrSoftMap.Lookup( CString(lpstrID) ) != NULL )
			return m_arrSoftMap.Lookup( CString(lpstrID) )->m_value;
		else 
			return NULL;
	}

	// 全部安装完成后的结果展示
	LRESULT OnInstedListBoxGetDispInfo( LPNMHDR pnmh )
	{
		BKLBMGETDISPINFO*	pdi			= (BKLBMGETDISPINFO*)pnmh;
		DWORD				nStIndex	= pdi->nListItemID * 5;		// 一行5个软件
		DWORD				nStIndxNum	= 0;

		if (m_arrInstCompleteState.GetSize() <= 0 )
			return 0;

		if (nStIndex >= m_arrInstCompleteState.GetSize())
			return 0;

		nStIndxNum = min(m_arrInstCompleteState.GetSize() - nStIndex, 5);

		for ( int i=0; i < 5; i++)
		{
			DWORD	nDivID	= i+1;
			DWORD	nDivSt	= nDivID*10;
			if ( nStIndxNum > i )
			{
				STATE_INST_COMPLETE	sfState		= m_arrInstCompleteState[nStIndex+i];
				CString&			strSoftID	= sfState.strSoftID;
				CSoftListItemData*  pSoftData   = GetSoftListItemDataByID(strSoftID);

				if (strSoftID.IsEmpty())
					continue;

				if (pSoftData==NULL)
					continue;

				m_listInstComplete.SetItemVisible(nDivID,TRUE);

				// 画图标
				if (TRUE)
				{
					CStringA	strMem;
					strMem.Format("%d",pSoftData->m_pImage);
					m_listInstComplete.SetItemAttribute( nDivSt+1, "mempointer",strMem );
				}
				// 下载图标
				if (!pSoftData->m_bIcon)
					m_pUIHandler->OnDownLoadIcon( pSoftData->m_strSoftID );

				// 标题，描述
				m_listInstComplete.SetItemText(nDivSt+2, GetSoftShowName(pSoftData) );
				m_listInstComplete.SetItemStringAttribute(nDivSt+2,"tip",pSoftData->m_strDescription);
			
				if (sfState.bInstSuccess)
				{
					m_listInstComplete.SetItemStringAttribute(nDivSt+1,"tip",L"");
					m_listInstComplete.SetItemAttribute(nDivSt+3,"sub","2");
					m_listInstComplete.SetItemAttribute(nDivSt+1, "class", "");
				}
				else
				{
					m_listInstComplete.SetItemStringAttribute(nDivSt+1,"tip",L"双击图标进行手动安装");
					m_listInstComplete.SetItemAttribute(nDivSt+3,"sub","1");
					m_listInstComplete.SetItemAttribute(nDivSt+1, "class", "linkimage");
				}
			}
			else
			{
				m_listInstComplete.SetItemVisible(nDivID,FALSE);
			}
		}
		return 0;
	}

	LRESULT OnInstedListBoxDbClick(LPNMHDR pnmh)
	{
		BKLBMITEMDBCLICK*	pdi			= (BKLBMITEMDBCLICK*)pnmh;
		DWORD				nStIndex	= pdi->nListItemID * 5;		// 一行5个软件
		DWORD				nStIndxNum	= 0;

		if (m_arrInstCompleteState.GetSize() <= 0 )
			return 0;

		if (nStIndex >= m_arrInstCompleteState.GetSize())
			return 0;

		nStIndxNum = min(m_arrInstCompleteState.GetSize() - nStIndex, 5);

		int i = -1;
		switch(pdi->uCmdID)
		{
		case 11:i = 0;break;
		case 21:i = 1;break;
		case 31:i = 2;break;
		case 41:i = 3;break;
		case 51:i = 4;break;
		}
		if (i != -1)
		{
			if ( nStIndxNum > i )
			{
				STATE_INST_COMPLETE	sfState		= m_arrInstCompleteState[nStIndex+i];

				DWORD					nId			= (DWORD)_ttoi(sfState.strSoftID);
				const ksm::InstInfo*	pAutoInfo	= NULL;
				CString					strLocalPath;

				pAutoInfo = m_pUIHandler->m_softInstEng.GetInstInfo( nId );
				if (pAutoInfo)
					strLocalPath = GetInstLocalPath(pAutoInfo->_downName);

				// 允许调安装包
				if (sfState.bInstSuccess == FALSE)
				{
					if (strLocalPath.GetLength() && 
						PathFileExists(strLocalPath))
					{
						ShellExecute(NULL, _T("open"), strLocalPath, NULL, NULL, SW_SHOW);
					}
					else
					{
						CBkSafeMsgBox2::Show( L"未发现软件完整安装包，请重新下载再试。",NULL, MB_ICONWARNING|MB_OK, NULL, m_hWnd);
					}
				}
			}
		}

		return 0;
	}

	LRESULT OnSelListBoxGetDispInfo( LPNMHDR pnmh )
	{
		BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;
		if ( pdi->nListItemID >= m_curListArray.GetSize() )
			return 0;

		ARRAY_SOFT_SELECT&	selType = m_curListArray[pdi->nListItemID];


		m_listSelect.SetItemText(IDC_SOFT_OKLIST_TYPE_NAME,selType.strType );
		for ( int i=0; i < 3; i++)
		{
			int		nIDStart = IDC_SOFT_OKLIST_SOFT_DISTANCE*i;

			m_listSelect.SetItemVisible(nIDStart+IDC_SOFT_OKLIST_DIV_SOFT,FALSE);
			if ( selType.softx[i].bExist )
			{
				CSoftListItemData*	pData   = GetSoftListItemDataByID(selType.softx[i].strSoftId );
				if ( pData != NULL)
				{
					m_listSelect.SetItemVisible(nIDStart+IDC_SOFT_OKLIST_ICON_INSTED,FALSE);
					m_listSelect.SetItemVisible(nIDStart+IDC_SOFT_OKLIST_CHECK_SOFT,FALSE);

					m_listSelect.SetItemVisible(nIDStart+IDC_SOFT_OKLIST_DIV_SOFT,TRUE);
					m_listSelect.SetItemCheck(nIDStart+IDC_SOFT_OKLIST_CHECK_SOFT, selType.softx[i].bCheck );
					CStringA	strMem;
					strMem.Format( "%d", pData->m_pImage);
					m_listSelect.SetItemAttribute(nIDStart+IDC_SOFT_OKLIST_IMG_SOFT, "mempointer",strMem);
				
					if (!pData->m_bIcon)
						m_pUIHandler->OnDownLoadIcon(pData->m_strSoftID);

					if (pData->m_bSetup)
					{
						m_listSelect.SetItemVisible(nIDStart+IDC_SOFT_OKLIST_ICON_INSTED,TRUE);

						CString	str;
						str.Format( L"%s（已安装）", GetSoftShowName(pData) );
						m_listSelect.SetItemText(nIDStart+IDC_SOFT_OKLIST_TXT_SOFT_NAME, str);
						m_listSelect.SetItemStringAttribute(nIDStart+IDC_SOFT_OKLIST_TXT_SOFT_NAME,"crtext",L"808080");
					}
					else
					{
						m_listSelect.SetItemVisible(nIDStart+IDC_SOFT_OKLIST_CHECK_SOFT,TRUE);
						m_listSelect.SetItemText(nIDStart+IDC_SOFT_OKLIST_TXT_SOFT_NAME, GetSoftShowName(pData) );
						m_listSelect.SetItemStringAttribute(nIDStart+IDC_SOFT_OKLIST_TXT_SOFT_NAME,"crtext",L"000000");
					}
					m_listSelect.SetItemStringAttribute(nIDStart+IDC_SOFT_OKLIST_TXT_SOFT_NAME,"tip",pData->m_strDescription);
				}
			}
		}
		return 0;
	}

	LRESULT OnBkListBoxClickCtrl( LPNMHDR pnmh )
	{

		LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
		if ( pnms->nListItemID >= m_curListArray.GetSize() )
			return 0;

		switch ( pnms->uCmdID )
		{
		//case IDC_SOFT_OKLIST_TXT_SOFT_NAME:
		//case IDC_SOFT_OKLIST_TXT_SOFT_NAME+1*IDC_SOFT_OKLIST_SOFT_DISTANCE:
		//case IDC_SOFT_OKLIST_TXT_SOFT_NAME+2*IDC_SOFT_OKLIST_SOFT_DISTANCE:
		//	{
		//		ARRAY_SOFT_SELECT&  arrSoft = m_curListArray[pnms->nListItemID];
		//		int					nIndex  = (pnms->uCmdID-IDC_SOFT_OKLIST_TXT_SOFT_NAME)/IDC_SOFT_OKLIST_SOFT_DISTANCE;

		//		if (arrSoft.softx[nIndex].bExist)
		//		{
		//			arrSoft.softx[nIndex].bCheck = !arrSoft.softx[nIndex].bCheck;
		//			m_listSelect.RedrawItem(nIndex);
		//		}
		//		RefreshCheckNumberSize();
		//	}
		//	break;
		case IDC_SOFT_OKLIST_IMG_SOFT:
		case IDC_SOFT_OKLIST_IMG_SOFT+1*IDC_SOFT_OKLIST_SOFT_DISTANCE:
		case IDC_SOFT_OKLIST_IMG_SOFT+2*IDC_SOFT_OKLIST_SOFT_DISTANCE:
			{
				ARRAY_SOFT_SELECT&  arrSoft = m_curListArray[pnms->nListItemID];
				int					nIndex  = (pnms->uCmdID-IDC_SOFT_OKLIST_IMG_SOFT)/IDC_SOFT_OKLIST_SOFT_DISTANCE;

				if (arrSoft.softx[nIndex].bExist)
				{
					arrSoft.softx[nIndex].bCheck = !arrSoft.softx[nIndex].bCheck;
					m_listSelect.RedrawItem(nIndex);
				}
				RefreshCheckNumberSize();
			}
			break;
		case IDC_SOFT_OKLIST_CHECK_SOFT:
		case IDC_SOFT_OKLIST_CHECK_SOFT+1*IDC_SOFT_OKLIST_SOFT_DISTANCE:
		case IDC_SOFT_OKLIST_CHECK_SOFT+2*IDC_SOFT_OKLIST_SOFT_DISTANCE:
			{
				ARRAY_SOFT_SELECT&  arrSoft = m_curListArray[pnms->nListItemID];
				int					nIndex  = (pnms->uCmdID-IDC_SOFT_OKLIST_CHECK_SOFT)/IDC_SOFT_OKLIST_SOFT_DISTANCE;
				
				if (arrSoft.softx[nIndex].bExist)
				{
					arrSoft.softx[nIndex].bCheck = m_listSelect.GetItemCheck(pnms->uCmdID);
				}
				RefreshCheckNumberSize();
			}
			break;
		}
		return 0;
	}

	DWORD GetSelSoftNumber()
	{
		return m_nSelSoftNumber;
	}

	void RefreshCheckNumberSize()
	{
		BOOL	bAllCheck = TRUE;

		m_nSelSoftNumber	= 0;
		m_nPlugNumber		= 0;
		m_ullNeedSize		= 0;
		for ( int i=0; i < m_curListArray.GetSize(); i++ )
		{
			for ( int k=0; k < 3; k++)
			{
				SOFT_SELECT_STATE&	st = m_curListArray[i].softx[k];

				if (st.bExist )
				{
					CSoftListItemData*	pData = GetSoftListItemDataByID(st.strSoftId);

					if (pData && !pData->m_bSetup )
					{
						if ( st.bCheck )
						{
							const ksm::InstInfo*	pAutoInfo = GetAutoInstInfo(pData->m_strSoftID);

							if (pAutoInfo)
							{
								m_nPlugNumber += pAutoInfo->_pluginInfoList.GetSize();
								m_ullNeedSize += pAutoInfo->_instSize;
								m_nSelSoftNumber++;			
							}
						}
						else
						{
							bAllCheck = FALSE;
						}
					}
				}
			}
		}

		if ( this->IsItemVisible(IDC_SOFT_ONEK_DIV_SEL,TRUE) )
		{
			if (m_nSelSoftNumber==0)
			{
				this->EnableItem(IDC_SOFT_ONEK_BTN_INSTALL,FALSE);
				this->SetItemVisible(IDC_SOFT_ONEK_TXT_SEL_NUMBER,FALSE);
			}
			else
			{
				this->EnableItem(IDC_SOFT_ONEK_BTN_INSTALL,TRUE);
				this->SetItemVisible(IDC_SOFT_ONEK_TXT_SEL_NUMBER,TRUE);					

				this->FormatRichText(IDC_SOFT_ONEK_TXT_SEL_NUMBER,
					L"<c color=FFFFFF>已选择 <b><c color=AAFF06>%d</c></b> 款软件，预计共需 <b><c color=AAFF06>%s</c></b> 空间</c>",
					m_nSelSoftNumber, GetSizeStr(m_ullNeedSize) );
			}

			this->EnableItem(IDC_SOFT_ONEK_LBL_SEL_PLUG,m_nPlugNumber != 0);
			this->FormatItemText(IDC_SOFT_ONEK_LBL_SEL_PLUG,L"选择安装各软件附带的安装选项(%d)", m_nPlugNumber);

			this->SetItemCheck(IDC_SOFT_ONEK_CHECK_ALL,bAllCheck);
		}
		else if (this->IsItemVisible(IDC_SOFT_ONEK_DIV_SEL_DIR))
		{
			RefreshSelDirDiv();
		}
	}

	BOOL CheckSoftCanInstall(CSoftListItemData* pData, BOOL bFromDownOK)
	{
		return TRUE;
	}

	DWORD GetInstSuccessNumber()
	{
		DWORD	num = 0;
		for ( int i=0 ;i< m_arrInstCompleteState.GetSize(); i++)
		{
			if (m_arrInstCompleteState[i].bInstSuccess)
				num++;
		}
		return num;
	}

	void NotifySoftInstallComplete(DWORD nIndex, ONE_KEY_INST_RESULT* pRes)
	{
		ATLASSERT(nIndex < m_arrInstSoft.GetSize() );

		if (m_arrInstSoft.GetSize() <= 0)
			return;

		SOFT_INST_STATE&	st = m_arrInstSoft[nIndex];
		st.downState	= STATE_ONEKINST_END;

		STATE_INST_COMPLETE	stcomplt;

		stcomplt.bInstSuccess	= TRUE;
		stcomplt.strSoftID		= st.strSoftID;

		if (pRes)
		{
			if (!pRes->bDownSuccess)
			{
				stcomplt.bInstSuccess = FALSE;

			}
			else if ( pRes->instST.struStatus.dwInstallStatus != em_Status_Complete )
			{
				stcomplt.bInstSuccess = FALSE;

			}
			else
			{
				stcomplt.bInstSuccess = TRUE;

			}
		}
		m_arrInstCompleteState.Add(stcomplt);

		m_nCurInstIndex++;

		if ( m_nCurInstIndex >= m_arrInstSoft.GetSize() )
			m_bDownComplete = TRUE;
		else
			_downLoadSoft(m_nCurInstIndex);

		_refreshInstingDiv();
		if (m_nCurInstIndex == m_arrInstSoft.GetSize())
		{
			DWORD	nSuccces = GetInstSuccessNumber();

			RestoreTrayByPassMode();
			this->SetItemText(IDC_SOFT_ONEK_BTN_CANCL_ALL,L"关闭");
			this->SetItemVisible(IDC_SOFT_ONEK_DIV_INSTING,FALSE);
			this->SetItemVisible(IDC_SOFT_ONEK_DIV_INSTED,TRUE);

			if (nSuccces==m_nSelSoftNumber)
			{
				this->FormatRichText(IDC_SOFT_ONEK_TXT_INSTED,
				L"<b><c color=FFFFFF>一键装机已经完成！<c color=AAFF06>%d</c> 款必备软件已安装成功！</c></b>",
				nSuccces);

				this->SetItemAttribute(IDC_SOFT_ONEK_LIST_INSTED, "pos", "0,0,-0,-0");
				this->SetItemAttribute(IDC_SOFT_ONEK_ICO_COMPLETE_TOP, "skin", "onekeytopicon2");
			}
			else
			{
				this->FormatRichText(IDC_SOFT_ONEK_TXT_INSTED,
					L"<b><c color=FFFFFF>一键装机已经完成！<c color=AAFF06>%d</c> 款软件安装成功，<c color=FF0000>%d</c>款软件安装失败！</c></b>",
					nSuccces,
					m_nSelSoftNumber-nSuccces);

				this->SetItemVisible(IDC_SOFT_ONEK_LBL_DB_CUSTOM_SETUP,TRUE);
				this->SetItemAttribute(IDC_SOFT_ONEK_LIST_INSTED, "pos", "0,30,-0,-0");
				this->SetItemAttribute(IDC_SOFT_ONEK_ICO_COMPLETE_TOP, "skin", "onekeytopicon2_fail");
			}
			DWORD	nSize = m_arrInstCompleteState.GetSize();

			DWORD	nLine = nSize/5;

			if (nSize%5!=0)
				nLine++;

			m_listInstComplete.SetItemCount( nLine );
			KillTimer(1);

			CSimpleArray<CString>	strIDS;
			for ( int i=0; i < m_arrInstCompleteState.GetSize(); i++)
			{
				strIDS.Add( m_arrInstCompleteState[i].strSoftID );
			}
			if (strIDS.GetSize() > 0 )
			{
				m_pUIHandler->CheckSoftInstallState(strIDS);
			}
		}
	}

	void NotifySoftInstallComplete(CSoftListItemData* pData)
	{
	}

	struct PARAM 
	{
		void *pTask;
		void *pDlg;
	};


	//获得下载信息
	void _DownloadInfoCallBack(int64 cur,int64 all,int64 speed,DWORD time,void* para)
	{
		PARAM				*pParam = (PARAM*)para;
		if ( all != 0 )
		{
			if(speed < 0)
				speed=0;
			CString strSpeed = CalcStrFormatByteSize(speed);
			strSpeed.Remove(_T(' '));

			m_strDownSpeed.Format(_T("%s/s"), strSpeed);
			m_nCurDownProg = (DWORD)((cur * 100) / all);

		}
		
		if (pParam != NULL)
		{
			delete pParam;
			pParam = NULL;
		}
	}
	static void DownloadInfoCallBack(int64 cur,int64 all,int64 speed,DWORD time,void* para)
	{
		{
			CSimpleLock2_(m_lockState);
			if (m_pThisCheck==NULL)
				return;
		}

		PARAM *pParam = (PARAM*)para;
		CBkSoftOneKeyDlg *pDlg = (CBkSoftOneKeyDlg *)pParam->pDlg;
		pDlg->_DownloadInfoCallBack(cur,all,speed,time,para);
	}

	void _DownloadCallBack(DTManager_Stat st, void* tk,void* para)
	{
		PARAM *pParam = new PARAM;
		pParam->pDlg = para;
		pParam->pTask = tk;

		switch (st)
		{
		case TASK_DONE:
			{
				m_pDTManager->QueryTask(tk, DownloadInfoCallBack, (void*)pParam);
				m_pDTManager->CancelTask(tk);
				tk = NULL;
				m_pDownTask = NULL;

				PostMessage(WM_SOFT_OK_DOWN_DONE,(WPARAM)st,NULL);
			}
			break;
		case TASK_DOWNING:
			m_pDTManager->QueryTask(tk, DownloadInfoCallBack, (void*)pParam);
			break;
		case TASK_ERROR_MD5:
		case TASK_ERROR:
			{
				m_pDTManager->PauseTask(tk);
//				m_pDTManager->CancelTask(tk);

				tk = NULL;
				m_pDownTask = NULL;
				PostMessage(WM_SOFT_OK_DOWN_DONE,(WPARAM)st,NULL);

				if (pParam != NULL)
				{
					delete pParam;
					pParam = NULL;
				}
			}
			break;
		default:
			break;
		}	
	}

	static void DownloadCallBack(DTManager_Stat st, void* tk,void* para)
	{
		{
			CSimpleLock2_(m_lockState);
			if (m_pThisCheck==NULL)
				return;
		}

		CBkSoftOneKeyDlg *pDlg = (CBkSoftOneKeyDlg*)para;
		pDlg->_DownloadCallBack(st,tk,para);
		return;
	}

	void RefreshItemState(LPCTSTR lpstrSoftId)
	{
		if ( this->IsItemVisible(IDC_SOFT_ONEK_DIV_SEL,TRUE) )
		{
			for ( int i=0; i < m_curListArray.GetSize(); i++)
			{
				for ( int k=0; k < 3; k++)
				{
					if ( m_curListArray[i].softx[k].strSoftId.CompareNoCase(lpstrSoftId) == 0 )
					{
						m_listSelect.Referesh();
						break;
					}
				}
			}
		}
		else if ( this->IsItemVisible(IDC_SOFT_ONEK_DIV_INST,TRUE) )
		{
			
		}
	}

	void ForceClose()
	{
		UnInit();
	}

	void OnBtnBeginInstToSelDir()
	{
		if (m_nSelSoftNumber==0)
		{
			CBkSafeMsgBox2::Show( L"请至少选择一款软件进行安装!",NULL, MB_ICONWARNING|MB_OK, NULL, m_hWnd);
			return;
		}

		this->SetItemVisible(IDC_SOFT_ONEK_DIV_SEL,FALSE);
		this->SetItemVisible(IDC_SOFT_ONEK_DIV_SEL_DIR,TRUE);


		GetDefaultInstDir(0);

		RefreshSelDirDiv();
	}

	CString GetSizeStr(ULONGLONG nSize)
	{
		CString	str;
		if ( nSize > ( 1024 * 1024 * 1024 ) )
		{
			double	freeGB = nSize / ( 1024 * 1024 * 1024.0 );
			str.Format( _T( "%.2fGB" ), freeGB );
		}
		else
		{
			ULONGLONG		freeMB = nSize / ( 1024 * 1024 );
			str.Format( _T( "%dMB" ), freeMB );
		}
		return str;
	}

	void RefreshSelDirDiv()
	{
		this->FormatRichText(IDC_SOFT_ONEK_TXT_SEL_TOP_TIP,
			L"<c color=FFFFFF><b>您选择了<c color=AAFF06>%d</c>款必备软件安装，预计共需<c color=AAFF06>%s</c>空间</b></c>",
			m_nSelSoftNumber,
			GetSizeStr(m_ullNeedSize)
			);


		if (m_ullNeedSize < m_nllDiskLeftSize )
		{
			this->SetItemAttribute(IDC_SOFT_ONEK_ICON_SEL_ENOUGH,"sub","2");
			this->FormatRichText(IDC_SOFT_ONEK_TXT_SEL_SIZE,
				L"<c color=000000>可用空间：  <c color=008000>%s</c>   空间充足，适合安装!</c>",
				GetSizeStr(m_nllDiskLeftSize));
		}
		else
		{
			this->SetItemAttribute(IDC_SOFT_ONEK_ICON_SEL_ENOUGH,"sub","1");
			this->FormatRichText(IDC_SOFT_ONEK_TXT_SEL_SIZE,
				L"<c color=000000>可用空间：  <c color=008000>%s</c>  <c color=FF0000>空间不足，建议更换目录!</c></c>",
				GetSizeStr(m_nllDiskLeftSize));
		}

		m_edtSelDir.SetWindowText(m_strInstDir);
	}

	void OnBtnBeginInst()
	{
		if (m_ullNeedSize > m_nllDiskLeftSize)
		{
			CBkSafeMsgBox2::Show( L"当前磁盘空间不足，建议您更换磁盘目录进行安装!" ,NULL, MB_OK|MB_ICONWARNING, NULL, m_hWnd);
			return;
		}

		GenerateInstList();
		ATLASSERT(m_arrInstSoft.GetSize()>0);

		if ( TRUE )
		{
			CSafeMonitorTrayShell MonitorShell;
			m_bSetSafeByPassExit = TRUE;
			MonitorShell.EnableRiskMonitorSafeBypass(TRUE);
		}

		m_nCurInstIndex	= 0;
		m_bDownComplete = FALSE;
		SetItemVisible(IDC_SOFT_ONEK_DIV_SEL_DIR,FALSE);
		SetItemVisible(IDC_SOFT_ONEK_DIV_INST,FALSE);
		SetItemVisible(IDC_SOFT_ONEK_DIV_INSTING,TRUE);
		m_nCompleteNubmer = 0;
		m_bInstCancelExit = FALSE;
		m_bCloseParentWnd = FALSE;
		_downLoadSoft(m_nCurInstIndex);
		_refreshInstingDiv();
		SetTimer(1,1000,NULL);

	}

	SOFT_INST_STATE& GetInstingStat(const CString& strID)
	{
		for ( int i=0; i < m_arrInstSoft.GetSize(); i++)
		{
			if (m_arrInstSoft[i].strSoftID.CompareNoCase(strID) == 0 )
				return m_arrInstSoft[i];
		}
		ATLASSERT(NULL);

		// should never gone
		static SOFT_INST_STATE	nu;
		return nu;
	}

	CString	GetInstLocalPath(LPCTSTR lpLocalName)
	{
		CString		strStore;

		BKSafeConfig::GetStoreDir(strStore);
		if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
			strStore.Append( CString(_T("\\")) + lpLocalName);
		else
			strStore.Append(lpLocalName);

		return strStore;
	}

	VOID _refreshInstingDiv()
	{
		if (m_nCurInstIndex >= m_arrInstSoft.GetSize())
			return;

		DWORD				nPerSoft = (1*100/m_arrInstSoft.GetSize());
		DWORD				nAllProg = nPerSoft * m_nCurInstIndex;
		DWORD				nDownRate= 90;
		SOFT_INST_STATE&	st = m_arrInstSoft[m_nCurInstIndex];
		CSoftListItemData*	pData = GetSoftListItemDataByID(st.strSoftID);
		DWORD				nCurSoftProg = 0;
	

		if (pData==NULL)
			return;

		CStringA	str;
		str.Format("%d", pData->m_pImage);
		this->SetItemAttribute(IDC_SOFT_ONEK_ICON_INSTING_SOFT,"mempointer",str );

		this->SetItemText(IDC_SOFT_ONEK_TXT_INSTING_SOFT_NAME, pData->m_strName);
		this->SetItemText(IDC_SOFT_ONEK_TXT_INSTING_SOFT_DESC, pData->m_strDescription);
		this->SetItemStringAttribute(IDC_SOFT_ONEK_TXT_INSTING_SOFT_DESC,"tip", pData->m_strDescription);

		if (pData->m_fMark == 0.0)
			m_pUIHandler->QuerySoftMark(pData->m_strSoftID);
		else
		{
			//画评分
			for (int i = IDC_SOFT_ONEK_STAR_INSTING_SOFT; i < IDC_SOFT_ONEK_STAR_INSTING_SOFT+5; i++)
			{
				CStringA strSkin;
				if ((i - IDC_SOFT_ONEK_STAR_INSTING_SOFT + 1)*2 <= pData->m_fMark)
					strSkin = "star";
				else if ((i - IDC_SOFT_ONEK_STAR_INSTING_SOFT + 1)*2 - 1 <= pData->m_fMark)
					strSkin = "star_half";
				else
					strSkin = "star_off";
				this->SetItemAttribute(i, "skin", strSkin);
			}

			this->FormatItemText(IDC_SOFT_ONEK_TXT_INSTING_SOFT_SCORE, L"评分：%.1f 分", pData->m_fMark);
		}


		if (nPerSoft==0)
			nPerSoft = 1;

		if (m_bDowning)
			nAllProg += ((float)m_nCurDownProg)*(nPerSoft)*nDownRate/(100*100);
		else
			nAllProg += (nPerSoft*nDownRate/100 + nPerSoft*m_nCurInstProg*(100-nDownRate)/(100*100) );

		if (m_bDowning)
			nCurSoftProg = m_nCurDownProg*nDownRate/100;
		else
			nCurSoftProg = nDownRate + m_nCurInstProg*(100-nDownRate)/100;
		
		str.Format("%d", nAllProg);
		this->SetItemAttribute(IDC_SOFT_ONEK_PROG_INSTING,"value",str);

		if (m_nCurInstIndex < m_arrInstSoft.GetSize() - 1 )
		{
			this->FormatRichText(IDC_SOFT_ONEK_TXT_INSTING_NUM,
				L"<b><c color=FFFFFF>一键装机正在安装第 <c color=AAFF06>%d</c>款软件，还有<c color=AAFF06>%d</c>款软件准备安装...</c></b>",
				m_nCurInstIndex+1,
				m_arrInstSoft.GetSize()-m_nCurInstIndex-1
				);
		}
		else
		{
			this->FormatRichText(IDC_SOFT_ONEK_TXT_INSTING_NUM,
				L"<b><c color=FFFFFF>一键装机正在安装第 <c color=AAFF06>%d</c>款软件，还有<c color=AAFF06>%d</c>款软件准备安装...</c></b>",
				m_nCurInstIndex+1,
				m_arrInstSoft.GetSize()-m_nCurInstIndex-1
				);
		}

		if (m_bDowning)
			this->FormatItemText(IDC_SOFT_ONEK_TXT_INSTING_SPEED, L"正在下载中 %s", m_strDownSpeed);
		else
			this->FormatItemText(IDC_SOFT_ONEK_TXT_INSTING_SPEED, L"正在安装...");

		this->FormatRichText(IDC_SOFT_ONEK_TXT_INSTING_TIP, L"<c color=FFFFFF>正在安装 <c color=F5E885>%s</c> %d%%</c>", pData->m_strName, nCurSoftProg);
	}

	BOOL _downLoadSoft(DWORD nIndex)
	{
		ATLASSERT( nIndex < m_arrInstSoft.GetSize() );

		SOFT_INST_STATE&		softST		= m_arrInstSoft[nIndex];
		DWORD					nId			= (DWORD)_ttoi(softST.strSoftID);
		const ksm::InstInfo*	pAutoInfo	= m_pUIHandler->m_softInstEng.GetInstInfo( nId );
	
		ATLASSERT(pAutoInfo!=NULL);
		if (pAutoInfo==NULL)
			return FALSE;

		CString		strLocalPath;

		strLocalPath		= GetInstLocalPath(pAutoInfo->_downName);
		softST.strDownPath	= strLocalPath;
		m_nCurInstIndex		= nIndex;
		m_bDowning			= FALSE;
		m_nCurDownProg		= 0;
		m_nCurInstProg		= 0;

		if ( ::PathFileExists(strLocalPath) )
		{
			// 已经下载过直接安装
			m_bDowning			= FALSE;
			m_nCurDownProg		= 100;
			m_pUIHandler->m_softInstEng.SetInstDir(m_strInstDir);
			m_pUIHandler->m_softInstEng.SetInstPackPath(nId,strLocalPath);
			m_pUIHandler->m_softInstEng.StartInstSoft(nId);
		}
		else
		{
			CAtlArray<CString>*	strUrls = new CAtlArray<CString>;

			m_bDowning = TRUE;
			strUrls->Add( pAutoInfo->_downUrl );
			m_pUIHandler->InitDownloadDir();
			
			// 我靠，谁能告诉我怎么释放
			m_pDownTask	= m_pDTManager->NewTask( strUrls, pAutoInfo->_downMd5, strLocalPath);
		}

		return TRUE;
	}

	void OnRadioType(UINT uCmdId)
	{
		DWORD	nIndex = uCmdId-IDC_SOFT_ONEK_RADIO_BEGIN;

		ATLASSERT(nIndex < m_arrOneKey.GetSize());

		GenerateSelList(nIndex);
		RefreshCheckNumberSize();
		m_listSelect.SetItemCount(m_curListArray.GetSize());
	}

	void OnBkBtnCheckSelAll()
	{
		BOOL	bCheck = this->GetItemCheck(IDC_SOFT_ONEK_CHECK_ALL);
		for ( int i=0; i < m_curListArray.GetSize(); i++)
		{
			for (int k=0; k < 3; k++)
			{
				SOFT_SELECT_STATE&	st = m_curListArray[i].softx[k];

				if (st.bExist)
				{
					st.bCheck = bCheck;
				}
			}
		}
		RefreshCheckNumberSize();
		m_listSelect.ForceRefresh();
	}

	void OnBkBtnSelDefa()
	{
		for ( int i=0; i < m_curListArray.GetSize(); i++)
		{
			for (int k=0; k < 3; k++)
			{
				SOFT_SELECT_STATE&	st = m_curListArray[i].softx[k];

				if (st.bExist)
				{
					CSoftListItemData*	pData = GetSoftListItemDataByID(st.strSoftId);

					if (pData && !pData->m_bSetup)
					{
						st.bCheck = st.bDefaCheck;
					}
				}
			}
		}
		RefreshCheckNumberSize();
		m_listSelect.ForceRefresh();
	}

	VOID RestoreTrayByPassMode()
	{
		if (m_bSetSafeByPassExit)
		{
			CSafeMonitorTrayShell	trayshell;
			trayshell.EnableRiskMonitorSafeBypass(FALSE);
			m_bSetSafeByPassExit = FALSE;
		}
	}

	BOOL OnBkBtnCancelAll(BOOL bCloseFromParent = FALSE)
	{
		BOOL	bCanExit = TRUE;
		if ( this->IsItemVisible(IDC_SOFT_ONEK_DIV_INSTING,TRUE) )
		{
			::EnableWindow(m_pUIHandler->m_pDlg->m_hWnd, FALSE);

			if ( CBkSafeMsgBox2::Show(L"一键装机尚未完成，现在退出可能导致当前\r\n正在安装的软件不可用，您是否要退出？",NULL, MB_YESNO|MB_ICONWARNING, NULL, m_hWnd) != IDYES )
			{
				::EnableWindow(m_pUIHandler->m_pDlg->m_hWnd, TRUE);


				return FALSE;// 用户选择取消退出
			}
		}

		if ( this->IsItemVisible(IDC_SOFT_ONEK_DIV_INSTING, TRUE) && m_nCurInstIndex < m_arrInstSoft.GetSize() )
		{
			SOFT_INST_STATE&	st		= m_arrInstSoft[m_nCurInstIndex];

			if (m_pDownTask && m_bDowning )
			{
				m_pDTManager->PauseTask(m_pDownTask);
				m_pDownTask = NULL;
			}
			if (!m_bDowning)
			{
				m_pUIHandler->m_softInstEng.StopInstSoft();
				bCanExit = FALSE;
			}
		}

		if (bCanExit)//（下载中）
		{
			::EnableWindow(m_pUIHandler->m_pDlg->m_hWnd, TRUE);
			UnInit();
			return TRUE;// 可以退出
		}
		else//（安装中）
		{
			ShowWindow(SW_HIDE);

			::EnableWindow(m_pUIHandler->m_pDlg->m_hWnd, FALSE);
			m_bInstCancelExit = TRUE;// 需要延后退出
			if (bCloseFromParent)
				m_bCloseParentWnd = TRUE;

			return FALSE;// 暂时取消退出 延后退出
		}

		return TRUE;
	}

	void UnInit()
	{
		RestoreTrayByPassMode();
		DestroyWindow();
		::PostMessage(m_pUIHandler->m_pDlg->m_hWnd,WM_SOFT_ONEKEY_DLG_EXIT,NULL,NULL);
		//EndDialog(0);
	}

	VOID GetSelSoftIDList(CSimpleArray<CString>& strIdLists)
	{
		for ( int i=0; i < m_curListArray.GetSize(); i++ )
		{
			for ( int k=0; k < 3; k++)
			{
				SOFT_SELECT_STATE&	st = m_curListArray[i].softx[k];

				if (st.bExist )
				{
					CSoftListItemData*	pData = GetSoftListItemDataByID(st.strSoftId);

					if (pData && !pData->m_bSetup )
					{
						if ( st.bCheck )
						{
							strIdLists.Add(pData->m_strSoftID);
						}
					}
				}
			}
		}
	}

	void OnBtnSelPlug()
	{
		CSimpleArray<SOFT_PLUG_INFO_IN>	arrInfo;
		CSimpleArray<CString>			strIdList;

		GetSelSoftIDList(strIdList);

		if (strIdList.GetSize()==0)
			return;

		for (int i=0; i < strIdList.GetSize(); i++)
		{
			const ksm::InstInfo*	pInstInfo = m_pUIHandler->m_softInstEng.GetInstInfo( _ttoi(strIdList[i]) );
			CSoftListItemData*		pData     = GetSoftListItemDataByID(strIdList[i]);
			if (pInstInfo && pInstInfo->_pluginInfoList.GetSize() > 0 )
			{
				SOFT_PLUG_INFO_IN	ininfo;

				ininfo.nId			= _ttoi(strIdList[i]);
				ininfo.strName		= pData->m_strName;

				for (int kk=0; kk < pInstInfo->_pluginInfoList.GetSize(); kk++)
				{
					const ksm::PluginInfo&	infoplug = pInstInfo->_pluginInfoList[kk];
					SOFT_PLUG_PLUG_IN		plugin;

					plugin.bCheck	= infoplug._curState;
					plugin.strName	= infoplug._name;
					plugin.strKey	= infoplug._key;

					ininfo.arrPlug.Add(plugin);
				}
				arrInfo.Add(ininfo);
			}
		}

		if (arrInfo.GetSize() > 0)
		{
			CBkSoftOneKeySelPlugDlg	dlg(arrInfo);
			if ( IDOK == dlg.DoModal(m_hWnd) )
			{
				CSimpleArray<SOFT_PLUG_INFO_IN>&  arrResInfo = dlg.GetSelResult();

				for ( int i=0; i < arrResInfo.GetSize(); i++)
				{
					SOFT_PLUG_INFO_IN&	info = arrResInfo[i];
					for (int kk=0; kk < info.arrPlug.GetSize() ;kk++)
					{
						SOFT_PLUG_PLUG_IN&	plug = info.arrPlug[kk];

						m_pUIHandler->m_softInstEng.SetPluginInfo(info.nId, plug.strKey, plug.bCheck);
					}
				}
			}
		}
	}

	LRESULT OnDownSoftDone(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pDownTask = NULL;

		DTManager_Stat			downState	= (DTManager_Stat)wParam;
		SOFT_INST_STATE&		instST		= m_arrInstSoft[m_nCurInstIndex];
		DWORD					nId			= _ttoi(instST.strSoftID);
		CSoftListItemData*		pData		= GetSoftListItemDataByID(instST.strSoftID);
		const ksm::InstInfo*	pAutoInfo	= m_pUIHandler->m_softInstEng.GetInstInfo(nId);

		if (downState == TASK_DONE)
		{
			if ( pData && pAutoInfo)
			{
				m_pUIHandler->m_softInstEng.SetInstDir(m_strInstDir);
				m_pUIHandler->m_softInstEng.SetInstPackPath( nId, instST.strDownPath);
				m_pUIHandler->m_softInstEng.StartInstSoft(nId);
				m_bDowning = FALSE;
			}
		}
		else
		{
			m_bDowning = FALSE;
			::DeleteFile(instST.strDownPath);

			ONE_KEY_INST_RESULT*	pNewST = new ONE_KEY_INST_RESULT;
			pNewST->bDownSuccess	= FALSE;
			pNewST->nDownError		= downState;
			PostMessage(WM_SOFT_OK_INST_DONE,(WPARAM)pNewST,NULL);
		}

		_refreshInstingDiv();
		return S_OK;
	}

	LRESULT OnInstSoftDone(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ONE_KEY_INST_RESULT* p = (ONE_KEY_INST_RESULT*)wParam;
		if (m_bInstCancelExit)
		{
			UnInit();

			::EnableWindow(m_pUIHandler->m_pDlg->m_hWnd, TRUE);
			if (m_bCloseParentWnd)
				::PostMessage(m_pUIHandler->m_pDlg->m_hWnd, WM_SYSCOMMAND, SC_CLOSE | HTCAPTION, 0);
		}
		else
			NotifySoftInstallComplete(m_nCurInstIndex,p );

		if (p)delete p;

		return S_OK;
	}

	LRESULT OnInstSoftProcess(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DWORD nValue = (DWORD)wParam;
		m_nCurInstProg = nValue;

		return S_OK;
	}

	BOOL IsPathValid(const CString strDir)
	{
		if ( strDir.GetLength() < 2 )
			return FALSE;
		
		if ( strDir.GetAt(1) != L':' )
			return FALSE;
		
		DWORD nType = GetDriveType( strDir.Left(2) );

		if (nType == DRIVE_CDROM)
			return FALSE;
		if (nType ==DRIVE_REMOTE )
			return FALSE;
		if (nType == DRIVE_NO_ROOT_DIR )
			return FALSE;

		return TRUE;
	}

	VOID OnBtnSelChangeDir()
	{
		CString		strDir;
		m_edtSelDir.GetWindowText( strDir );	

		while( SelectFolderEx(strDir, m_hWnd) )
		{
			// 检查路径是否过长
			if ( strDir.GetLength() > 200 )
			{
				CBkSafeMsgBox::Show( BkString::Get(IDS_SOFTMGR_8078), NULL, MB_OK | MB_ICONWARNING, NULL, m_hWnd);
			}
			else
			{
				if ( IsPathValid(strDir) )
				{
					if (strDir.Right(1) == L'\\')
						strDir = strDir.Left( strDir.GetLength()-1 );

					m_nllDiskLeftSize	= GetPathDiskFreeSize(strDir);
					m_strInstDir		= strDir;
					RefreshSelDirDiv();

					break;
				}
				else
				{
					CBkSafeMsgBox2::Show(L"目录无效，请重新选择！",NULL,MB_OK|MB_ICONWARNING, NULL, m_hWnd);
				}
			}	
		}
	}

	void OnTimer( UINT_PTR nIDEvent )
	{
		_refreshInstingDiv();
	}

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		CRect rcList;
		m_listSelect.GetWindowRect(&rcList);
		if (rcList.PtInRect(pt))
		{
			SendMessage(m_listSelect.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		}

		SetMsgHandled(FALSE);
		return FALSE;
	}

	void OnBkBtnMin()
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
	}

	void OnBtnSelDefaDir()
	{
		GetDefaultInstDir(m_ullNeedSize);
		RefreshSelDirDiv();
	}

	void OnBtnInstedOpenDir()
	{
		::ShellExecute( NULL, L"open", m_strInstDir, NULL,NULL, SW_SHOW);
	}

	CString GetSoftShowName(CSoftListItemData* pData)
	{
		CString		strSoftID = pData->m_strSoftID;
		
		CAtlMap<CString,CString>::CPair*	parx;

		parx = m_mapShowName.Lookup(strSoftID);
		if (parx != NULL)
		{
			return parx->m_value;
		}
		return pData->m_strName;
	}

	BOOL IsOneKeyInsting()
	{
		return this->IsItemVisible(IDC_SOFT_ONEK_DIV_INSTING,TRUE);
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MIN, OnBkBtnMin)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_CHECK_ALL, OnBkBtnCheckSelAll)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_CANCL_ALL, OnBkBtnCancelAll)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_SEL_CANCEL, OnBkBtnCancelAll)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_INSTING_CANCEL, OnBkBtnCancelAll)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_INSTED_EXIT, OnBkBtnCancelAll)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_LBL_SEL_DEFA,OnBkBtnSelDefa)
		BK_NOTIFY_ID_COMMAND_EX(IDC_SOFT_ONEK_RADIO_BEGIN,IDC_SOFT_ONEK_RADIO_END,OnRadioType)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_INSTALL, OnBtnBeginInstToSelDir)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_SEL_GO, OnBtnBeginInst)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_SEL_GO2, OnBtnBeginInst)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_LBL_SEL_PLUG, OnBtnSelPlug)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_BTN_SEL_CHANGE_DIR, OnBtnSelChangeDir)	
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_LBL_SEL_RESET_DEF_DIR,OnBtnSelDefaDir)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_ONEK_LBL_INSTED_OPEN_DIR,OnBtnInstedOpenDir)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkSoftOneKeyDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkSoftOneKeyDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkSoftOneKeyDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MESSAGE_HANDLER(WM_SOFT_OK_DOWN_DONE, OnDownSoftDone)
		MESSAGE_HANDLER(WM_SOFT_OK_INST_DONE, OnInstSoftDone)
		MESSAGE_HANDLER(WM_SOFT_OK_INST_PROGRESS, OnInstSoftProcess)
			
		MESSAGE_HANDLER_EX(WM_CLOSE_DLG, OnCloseDlg)
		NOTIFY_HANDLER_EX(IDC_SOFT_ONEK_LIST_SELECT, BKLBM_GET_DISPINFO, OnSelListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_SOFT_ONEK_LIST_SELECT, BKLBM_ITEMCLICK, OnBkListBoxClickCtrl)
		NOTIFY_HANDLER_EX(IDC_SOFT_ONEK_LIST_INSTED, BKLBM_GET_DISPINFO, OnInstedListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_SOFT_ONEK_LIST_INSTED, BKLBM_ITEMDBCLICK, OnInstedListBoxDbClick)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

protected:


	CBeikeSafeSoftmgrUIHandler*				m_pUIHandler;
	CSimpleArray<ONE_KEY_GROUP>&			m_arrOneKey;
	CAtlMap<CString,CString>				m_mapShowName;
	CAtlMap<CString,CSoftListItemData*>&	m_arrSoftMap;
	CBkListBox								m_listSelect;		// 选择软件
	CSimpleArray<SOFT_INST_STATE>			m_arrInstSoft;		// 安装软件的数组
	BOOL									m_bDownComplete;	// 快速装机任务完成
	DWORD									m_nCompleteNubmer;
	CSimpleArray<ARRAY_SOFT_SELECT>			m_curListArray;

	struct STATE_INST_COMPLETE 
	{
		CString		strSoftID;
		BOOL		bInstSuccess;
	};
	CSimpleArray<STATE_INST_COMPLETE>		m_arrInstCompleteState;
	CBkListBox								m_listInstComplete;

	DWORD									m_nPlugNumber;
	IDTManager*								m_pDTManager;
	VOID*									m_pDownTask;
	CString									m_strInstDir;
	ULONGLONG								m_nllDiskLeftSize;
	CWHEdit									m_edtSelDir;
	DWORD									m_nCurInstIndex;
	DWORD									m_nCurDownProg;
	BOOL									m_bDowning;
	CString									m_strDownSpeed;
	ULONGLONG								m_ullNeedSize;
	DWORD									m_nSelSoftNumber;
	DWORD									m_nCurInstProg;
	BOOL									m_bInstCancelExit;
	BOOL									m_bCloseParentWnd;
	BOOL									m_bSetSafeByPassExit;

public:

	LRESULT OnCloseDlg(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bRetCode = OnBkBtnCancelAll(TRUE);
		return bRetCode;
	};

	BOOL IsSoftIdAutoList(const CString& str)
	{
		DWORD	nId = _ttoi(str);

		if ( m_pUIHandler->m_softInstEng.GetInstInfoMap().Lookup( nId ) == NULL )
			return FALSE;
		else
			return TRUE;
	}

	const ksm::InstInfo*	GetAutoInstInfo(const CString& str)
	{
		DWORD	nId = _ttoi(str);

		if ( m_pUIHandler->m_softInstEng.GetInstInfoMap().Lookup( nId ) != NULL )
			return &(m_pUIHandler->m_softInstEng.GetInstInfoMap().Lookup( nId )->m_value) ;
		else
			return NULL;
	}
};


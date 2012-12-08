#pragma  once

// 快速装机，一键装机

#include <bkwin/bklistbox.h>
#include "beikesafesoftmgrHeader.h"
#include "bkmsgdefine.h"


#define SEL_PLUG_LIST_CAPTION_HEIGHT	30
#define SEL_PLUG_LIST_ITEM_HEIGHT		40

#define PLUG_PER_LINE_NUMBER			2

struct SOK_PLUG_STATE
{
public:
	SOK_PLUG_STATE()
	{
		bExist = FALSE;
		bCheck = FALSE;
	}

	CString	strName;
	CString	strKey;
	BOOL	bCheck;
	BOOL	bExist;
};

struct STATE_ONE_KEY_SEL_PLUG
{
	BOOL							bCaption;
	DWORD							nSoftId;
	CString							strSoftName;

	SOK_PLUG_STATE					arrPlugSt[PLUG_PER_LINE_NUMBER];
};

struct SOFT_PLUG_PLUG_IN
{
	BOOL		bCheck;
	CString		strName;
	CString		strKey;
};


struct SOFT_PLUG_INFO_IN
{
	DWORD							nId;
	CString							strName;
	CSimpleArray<SOFT_PLUG_PLUG_IN>	arrPlug;
};

class CBkSoftOneKeySelPlugDlg
	: public CBkDialogImpl<CBkSoftOneKeySelPlugDlg>
	, public CWHRoundRectFrameHelper<CBkSoftOneKeySelPlugDlg>
{
public:
	CBkSoftOneKeySelPlugDlg(CSimpleArray<SOFT_PLUG_INFO_IN>& arrPlug
		  )
		: CBkDialogImpl<CBkSoftOneKeySelPlugDlg>(IDR_BK_SOFTMGR_NECESS_ONE_KEY_SEL_PLUG_DLG)

	{
		m_arrPlugInfo = arrPlug;
	}

	virtual ~CBkSoftOneKeySelPlugDlg()
	{
	}

	VOID MakeplugData()
	{

		for ( int i=0; i < m_arrPlugInfo.GetSize(); i++)
		{
			SOFT_PLUG_INFO_IN&			ininfo = m_arrPlugInfo[i];
			{
				STATE_ONE_KEY_SEL_PLUG		selPlug;

				selPlug.nSoftId			= ininfo.nId;
				selPlug.strSoftName		= ininfo.strName;
				selPlug.bCaption		= TRUE;

				m_softPlugSt.Add(selPlug);
			}

			STATE_ONE_KEY_SEL_PLUG	plugsel;
			plugsel.bCaption	= FALSE;
			plugsel.nSoftId		= ininfo.nId;
			plugsel.strSoftName = ininfo.strName;

			BOOL	bAdd = FALSE;
			for(int i=0; i < ininfo.arrPlug.GetSize() ; i++)
			{
				SOK_PLUG_STATE&		st = plugsel.arrPlugSt[i%PLUG_PER_LINE_NUMBER];


				st.bCheck		= ininfo.arrPlug[i].bCheck;
				st.strName		= ininfo.arrPlug[i].strName;
				st.strKey		= ininfo.arrPlug[i].strKey;
				st.bExist		= TRUE;
				bAdd			= FALSE;
				if (i%PLUG_PER_LINE_NUMBER==(PLUG_PER_LINE_NUMBER-1))
				{
					bAdd = TRUE;
					m_softPlugSt.Add(plugsel);

					for ( int jj=0; jj < PLUG_PER_LINE_NUMBER; jj++)
					{
						plugsel.arrPlugSt[jj].bExist = FALSE;

					}
				}
			}
			if ( !bAdd )
				m_softPlugSt.Add(plugsel);
		}	
	}


	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		m_listSel.Create(this->GetViewHWND(), 1);
		m_listSel.Load(IDR_BK_SOFTMGR_NECESS_ONE_KEY_SEL_PLUG_TEMPLATE);

		MakeplugData();
		
		m_listSel.SetItemCount( m_softPlugSt.GetSize() );

		return TRUE;
	}

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		CRect rcList;
		m_listSel.GetWindowRect(&rcList);
		if (rcList.PtInRect(pt))
		{
			SendMessage(m_listSel.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		}

		SetMsgHandled(FALSE);
		return FALSE;
	}

	void OnBkBtnClose()
	{
		EndDialog(IDCANCEL);
	}

	LRESULT OnInstListBoxGetDispInfo( LPNMHDR pnmh )
	{
		BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;
		
		if (pdi->nListItemID >= m_softPlugSt.GetSize() )
			return 0;

		STATE_ONE_KEY_SEL_PLUG&		st = m_softPlugSt[pdi->nListItemID];

		m_listSel.SetItemVisible(1,FALSE);
		m_listSel.SetItemVisible(100,FALSE);

		if (st.bCaption)
		{
			m_listSel.SetItemVisible(100,TRUE);
			m_listSel.SetItemText(101, st.strSoftName);
			pdi->nHeight	= SEL_PLUG_LIST_CAPTION_HEIGHT;
		}
		else
		{
			pdi->nHeight	= SEL_PLUG_LIST_ITEM_HEIGHT;

			m_listSel.SetItemVisible(1,TRUE);
			for (int i=0; i < PLUG_PER_LINE_NUMBER; i++)
			{
				DWORD	nId = i*10 + 10;

				SOK_PLUG_STATE&		stplug = st.arrPlugSt[i];

				m_listSel.SetItemVisible(nId,FALSE);
				if (stplug.bExist)
				{
					m_listSel.SetItemVisible(nId,TRUE);
					m_listSel.SetItemCheck(nId+1,stplug.bCheck);
					m_listSel.SetItemText(nId+1,stplug.strName);
				}
			}
		}

		return 0;
	}

	LRESULT OnBkListBoxClickCtrl( LPNMHDR pnmh )
	{
		LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;

		if (pnms->nListItemID >= m_softPlugSt.GetSize() )
			return 0;

		STATE_ONE_KEY_SEL_PLUG&	st = m_softPlugSt[pnms->nListItemID];
		if (!st.bCaption)
		{
			switch(pnms->uCmdID)
			{
			case 11:
				{
					if (st.arrPlugSt[0].bExist)
					{
						st.arrPlugSt[0].bCheck = m_listSel.GetItemCheck(pnms->uCmdID);
					}
				}
				break;
			case 21:
				{
					if (st.arrPlugSt[1].bExist)
					{
						st.arrPlugSt[1].bCheck = m_listSel.GetItemCheck(pnms->uCmdID);
					}
				}
				break;
			case 31:
				{
					if (st.arrPlugSt[2].bExist)
					{
						st.arrPlugSt[2].bCheck = m_listSel.GetItemCheck(pnms->uCmdID);
					}
				}
				break;
			default:
				break;
			}
		}

		return 0;
	}

	LRESULT OnListBoxGetmaxHeight(LPNMHDR pnmh)
	{
		LPBKLBITEMCALCMAXITEM pnms = (LPBKLBITEMCALCMAXITEM)pnmh;

		pnms->nMaxHeight = SEL_PLUG_LIST_ITEM_HEIGHT;

		return 0;
	}

	LRESULT OnListBoxGetItemHeight(LPNMHDR pnmh)
	{		
		LPBKLBITEMMEASUREITEM pnms = (LPBKLBITEMMEASUREITEM)pnmh;

		if (pnms->nListItemId >= m_softPlugSt.GetSize() )
			return 0;

		if ( m_softPlugSt[pnms->nListItemId].bCaption )
			pnms->nHeight = SEL_PLUG_LIST_CAPTION_HEIGHT;
		else
			pnms->nHeight = SEL_PLUG_LIST_ITEM_HEIGHT;

		return 0;
	}

	VOID OnBkBtnSure()
	{
		m_arrPlugInfo.RemoveAll();

		for ( int i=0; i < m_softPlugSt.GetSize() ;i++)
		{
			STATE_ONE_KEY_SEL_PLUG&		st = m_softPlugSt[i];

			if (st.bCaption)
				continue;

			SOFT_PLUG_INFO_IN	ininfo;
			
			ininfo.nId		= st.nSoftId;
			ininfo.strName	= st.strSoftName;

			for ( int kk=0; kk<PLUG_PER_LINE_NUMBER; kk++)
			{
				SOK_PLUG_STATE&	plugst = st.arrPlugSt[kk];

				if (plugst.bExist)
				{
					SOFT_PLUG_PLUG_IN	inplug;

					inplug.bCheck	= plugst.bCheck;
					inplug.strName	= plugst.strName;
					inplug.strKey	= plugst.strKey;

					ininfo.arrPlug.Add(inplug);
				}
			}
			m_arrPlugInfo.Add(ininfo);
		}

		EndDialog(IDOK);
	}

	CSimpleArray<SOFT_PLUG_INFO_IN>& GetSelResult()
	{
		return m_arrPlugInfo;
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(3, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(2, OnBkBtnSure)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkSoftOneKeySelPlugDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkSoftOneKeySelPlugDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkSoftOneKeySelPlugDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		NOTIFY_HANDLER_EX(1, BKLBM_GET_DISPINFO, OnInstListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(1, BKLBM_CALC_MAX_HEIGHT, OnListBoxGetmaxHeight)
		NOTIFY_HANDLER_EX(1, BKLBM_CALC_ITEM_HEIGHT, OnListBoxGetItemHeight)
		NOTIFY_HANDLER_EX(1, BKLBM_ITEMCLICK, OnBkListBoxClickCtrl)

		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

protected:
	CBkListBox								m_listSel;
	CSimpleArray<STATE_ONE_KEY_SEL_PLUG>	m_softPlugSt;
	CSimpleArray<SOFT_PLUG_INFO_IN>			m_arrPlugInfo;
};


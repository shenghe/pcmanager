#include "stdafx.h"
#include "kscmain.h"
#include "kscmaindlg.h"
#include "uihandler_onekey.h"
#include "kclear/ionekeyclean.h"
#include "kscbase/kscres.h"
#include "kscbase/kscfilepath.h"
#include "kscbase/kscconv.h"
#include "tinyxml.h"
#include "kuiwin/kuimsgbox.h"
#include "uihandler_onekeyconfig.h"
#include "kscbase/kscsys.h"
#include "trashonekey.h"
#include "kscbase/kcslogging.h"

//////////////////////////////////////////////////////////////////////////

CUIHandlerOnekey::CUIHandlerOnekey(CKscMainDlg* refDialog): m_dlg(refDialog)
{
    m_fShowSettingLink  = TRUE;
    m_fFirstUse         = TRUE;
    m_fCanceled         = FALSE;

    m_nProgress         = 0;

    m_piTrashCleaner    = NULL;
    m_piTrackCleaner    = NULL;
    m_piRegCleaner      = NULL;
    m_hRegCleaner       = NULL;
	m_bSettingByClean   = FALSE;
	m_theLastPop        = 0;
	m_bCancel = FALSE;

    m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CUIHandlerOnekey::~CUIHandlerOnekey()
{
    CloseHandle(m_hStopEvent);
    m_hStopEvent        = NULL;
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerOnekey::Init()
{
    m_ctrlClean.Create(m_dlg->GetViewHWND(), NULL, NULL, WS_CHILD, 0, ID_CTL_ONEKEY_CLEAN, NULL);
    m_ctrlClean.ShowWindow(SW_SHOW);
}

void CUIHandlerOnekey::InitDelay()
{
	LoadSetting();

	UpdateHeader();
	UpdateMiddle();
	m_ctrlClean.UpdateUI();
	_LoadProvider();
}

void CUIHandlerOnekey::UnInit()
{
    _FreeProvider();
}

void CUIHandlerOnekey::_LoadProvider()
{
    GetCleaner_t _GetCleaner = NULL;

    GetTrashCleanerForOnekey(__uuidof(ICleanTask), (void**)&m_piTrashCleaner);

    m_piTrackCleaner = m_dlg->GetCleanProvider();

    m_hRegCleaner = ::LoadLibraryW(L"regcleaner.dll");
    if (m_hRegCleaner)
    {
        _GetCleaner = (GetCleaner_t)::GetProcAddress(
            m_hRegCleaner, "GetCleaner");
        if (_GetCleaner)
        {
            _GetCleaner(
                __uuidof(ICleanTask), 
                (void**)&m_piRegCleaner
                );
        }
    }

    m_cleanStates[0].SetFeedback(m_dlg->m_hWnd, &m_ctrlClean.m_recordTrashClean);
    m_cleanStates[1].SetFeedback(m_dlg->m_hWnd, &m_ctrlClean.m_recordTrackClean);
    m_cleanStates[2].SetFeedback(m_dlg->m_hWnd, &m_ctrlClean.m_recordRegClean);

    if (m_piTrashCleaner)
    {
        m_piTrashCleaner->SetCallback(m_cleanStates + 0);
    }

    if (m_piTrackCleaner)
    {
        m_piTrackCleaner->SetCallback(m_cleanStates + 1);
    }

    if (m_piRegCleaner)
    {
        m_piRegCleaner->SetCallback(m_cleanStates + 2);
    }
}

void CUIHandlerOnekey::_FreeProvider()
{
    // 痕迹清理是从内部获得的，不释放
    if (m_piTrashCleaner)
    {
        m_piTrashCleaner->Release();
        m_piTrashCleaner = NULL;
    }

    if (m_piRegCleaner)
    {
        m_piRegCleaner->Release();
        m_piRegCleaner = NULL;
    }

    if (m_hRegCleaner)
    {
        FreeLibrary(m_hRegCleaner);
        m_hRegCleaner = NULL;
    }
}

BOOL CUIHandlerOnekey::_CheckProvider()
{
    BOOL retval = FALSE;
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strWarning;

    if (m_piTrashCleaner && m_piTrackCleaner && m_piRegCleaner)
    {
        retval = TRUE;
    }

    if (!retval)
    {
        appRes.GetString("IDS_MODULE_NOT_COMPLETE", strWarning);
        CKuiMsgBox::Show(strWarning.c_str(), NULL, MB_ICONWARNING|MB_OK);
    }

    return retval;
}

void CUIHandlerOnekey::OnReturn()
{
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_START, FALSE);
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_SCAN, TRUE);
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_SCANHEAD, TRUE);
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_STARTHEAD, FALSE);
	m_dlg->SetItemVisible(131, FALSE);
}
void CUIHandlerOnekey::OnCleanButton()
{
	DWORD dwRet = 0;
	//if(/*IsPopSetting()*/)
	{
		LoadSetting();
		if (m_strMakeSure.compare(L"yes") == 0 || m_strMakeSure.compare(L"") == 0)
		{
			m_bSettingByClean = TRUE;
			dwRet = OnSetting();
			if (dwRet == 0)
			{
				return;
			}
		}
	}

	m_dlg->SetItemVisible(ID_DIV_ONEKEY_START, TRUE);
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_SCAN, FALSE);
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_SCANHEAD, FALSE);
	m_dlg->SetItemVisible(ID_DIV_ONEKEY_STARTHEAD, TRUE);

    _FakeClean();
}

void CUIHandlerOnekey::OnCleanLink()
{
	DWORD dwRet = 0;
	if(IsPopSetting() && m_ctrlClean.HasBegin() && m_ctrlClean.HasEnd())
	{
		LoadSetting();
		if (m_strMakeSure.compare(L"yes") == 0)
		{
			m_bSettingByClean = TRUE;
			dwRet = OnSetting();
			if (dwRet == 0)
			{
				return;
			}
		}
	}
    _FakeClean();
}

DWORD CUIHandlerOnekey::OnSetting()
{
    LoadSetting();
	time(&m_theLastPop);
	UINT_PTR uRet = 0;
    if(m_strCustom.length() == 0)
        m_strCustom = m_strDefault;
    CKscOneKeyConfig test(m_dlg, m_bSettingByClean);
    CString cstrdefaulttmp(m_strDefault.c_str());
    CString cstrcustmp(m_strCustom.c_str());
 
    test.SplitString(cstrdefaulttmp,L',',test.m_defaultvec);
    test.SplitString(cstrcustmp,L',',test.m_customvec);
    uRet = test.DoModal();
	m_bSettingByClean = FALSE;
	return (DWORD)uRet;
}

void CUIHandlerOnekey::OnViewTrashCleanResult()
{

}

void CUIHandlerOnekey::OnViewTrackCleanResult()
{

}

void CUIHandlerOnekey::OnViewRegCleanResult()
{

}

void CUIHandlerOnekey::OnCtlReSize(CRect rcWnd)
{
    m_ctrlClean.SetWindowPos(NULL, rcWnd, 0);
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerOnekey::UpdateHeader()
{
    if (m_fShowSettingLink)
    {
        m_dlg->SetItemVisible(ID_LNK_ONEKEY_CLEAN_SETTING, TRUE);
    }
    else
    {
        m_dlg->SetItemVisible(ID_LNK_ONEKEY_CLEAN_SETTING, FALSE);
    }
}

void CUIHandlerOnekey::UpdateMiddle()
{
    KAppRes& appRes = KAppRes::Instance();

    _UpdateIcon();
    _UpdateTitle();
    _UpdateButtonAndLink();
    _UpdateProgress();
}

void CUIHandlerOnekey::UpdateButtom()
{
    m_ctrlClean.UpdateUI();
}

void CUIHandlerOnekey::_UpdateIcon()
{
    if (!m_ctrlClean.HasBegin())
     {
//         m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_2, FALSE);
//         m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_1, "skin", _T("protectionstatus3"));
//         m_dlg->SetItemIntAttribute(ID_IMG_ONEKEY_MIDDLE_1, "sub", 1);
//         m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_1, TRUE);
			m_dlg->SetItemVisible(ID_DIV_ONEKEY_STARTHEAD, FALSE);
			m_dlg->SetItemVisible(ID_DIV_ONEKEY_SCANHEAD, TRUE);
			m_dlg->SetItemVisible(131, FALSE);
    }
    else if (m_ctrlClean.HasBegin() && !m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_1, "skin", _T("scan_animation_bg"));
//        m_dlg->SetItemIntAttribute(ID_IMG_ONEKEY_MIDDLE_1, "sub", 1);
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_1, FALSE);
    }
    else if (m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_2, FALSE);
        m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_1, "skin", _T("onekeyflag"));
        if (m_ctrlClean.IsCanceled())
        {
 //           m_dlg->SetItemIntAttribute(ID_IMG_ONEKEY_MIDDLE_1, "sub", 0);
        }
        else
        {
          //  m_dlg->SetItemIntAttribute(ID_IMG_ONEKEY_MIDDLE_1, "sub", 0);
        }
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_1, TRUE);
    }
}

void CUIHandlerOnekey::_UpdateTitle()
{
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strTimeUse;
    std::wstring strSummary;
    std::wstring strTaskState;
    std::wstring strFormat;
    CString strTemp;
	for (int i = 123; i < 131; i++)
	{
		m_dlg->SetItemVisible(i, FALSE);
		m_dlg->SetItemText(i, L"");
	}

    if (!m_ctrlClean.HasBegin())
    {
        if (IsFirstUse())
        {
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "pos", _T("118,29")); 
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "skin", _T("onekey_clean_first"));
        }
        else
        {
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "pos", _T("118,19")); 
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "skin", _T("onekey_clean_advice"));

            m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_MIDDLE_1, "pos", _T("300,22"));

            appRes.GetString("IDS_LAST_CLEAN_TIME", strFormat);
            strTemp.Format(strFormat.c_str(), m_strLastUse.c_str());
            m_dlg->SetItemText(ID_TXT_ONEKEY_MIDDLE_1, strTemp);
            appRes.GetString("IDS_LAST", strFormat);
            strTemp.Format(strFormat.c_str(), m_strLastSummary.c_str());
            m_dlg->SetItemText(ID_TXT_ONEKEY_MIDDLE_2, strTemp);
        }
        
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_3, TRUE);
        m_dlg->SetItemVisible(ID_TXT_ONEKEY_TASK, FALSE);
		m_dlg->SetItemVisible(ID_DIV_ONEKEY_START, FALSE);
		m_dlg->SetItemText(ID_TXT_ONEKEY_STARTHEAD_2, L"一键清理可以帮您节约磁盘空间，优化注册表，让您的系统运行更有效率。");
		m_dlg->SetItemText(ID_TXT_ONEKEY_STARTHEAD_1, L"轻轻一点，即可完成系统优化。为您释放更多的可用空间、清理使用痕迹和冗余注册表，还您一个运行流畅的系统。");
    }
    else if (m_ctrlClean.HasBegin() && !m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_3, FALSE);
        m_dlg->SetItemVisible(ID_TXT_ONEKEY_MIDDLE_1, FALSE);
        m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_MIDDLE_2, "pos", _T("120,58"));
        m_dlg->SetItemVisible(ID_TXT_ONEKEY_MIDDLE_2, FALSE);

        m_ctrlClean.GetCurrentTask(strTaskState);
        m_dlg->SetItemVisible(ID_TXT_ONEKEY_TASK, TRUE);
        m_dlg->SetItemText(ID_TXT_ONEKEY_TASK, strTaskState.c_str());
	}
    else if (m_ctrlClean.HasBegin() && m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemVisible(ID_TXT_ONEKEY_TASK, FALSE);

        if (m_ctrlClean.IsCanceled())
        {
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "pos", _T("118,19")); 
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "skin", _T("onekey_clean_stoped"));
            m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_MIDDLE_1, "pos", _T("413,24"));
			m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_COLOR, "pos", _T("450,24"));
        }
        else
        {
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "pos", _T("118,19")); 
            m_dlg->SetItemStringAttribute(ID_IMG_ONEKEY_MIDDLE_3, "skin", _T("onekey_clean_end"));
            m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_MIDDLE_1, "pos", _T("227,24"));
			m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_COLOR, "pos", _T("264,24"));
        }

        m_ctrlClean.GetTimeUse(strTimeUse);
        m_dlg->SetItemText(ID_TXT_ONEKEY_MIDDLE_1, L"用时：");
		m_dlg->SetItemText(ID_TXT_ONEKEY_COLOR, strTimeUse.c_str());
		m_dlg->SetItemVisible(ID_TXT_ONEKEY_COLOR, TRUE);
        m_dlg->SetItemVisible(ID_TXT_ONEKEY_MIDDLE_1, TRUE);
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_3, TRUE);

		std::map<int, std::wstring> mapSummary;
		m_ctrlClean.GetSummaryEx(mapSummary);
        m_ctrlClean.GetSummary(strSummary);
        m_strLastSummary = strSummary;

		//m_dlg->SetItemText(ID_TXT_ONEKEY_MIDDLE_2, strSummary.c_str());
		m_dlg->SetItemStringAttribute(ID_TXT_ONEKEY_MIDDLE_2, "pos", _T("120,52"));
		m_dlg->SetItemVisible(ID_TXT_ONEKEY_MIDDLE_2, TRUE);
		if (mapSummary.size() == 0)
		{
			m_dlg->SetItemText(ID_TXT_ONEKEY_MIDDLE_2, strSummary.c_str());
		}
		else
		{
			m_dlg->SetItemText(108, L"一共清理：");
			if (mapSummary.size() == 3)
			{
				for (int i = 124; i < 131; i++)
				{
					m_dlg->SetItemVisible(i, TRUE);
				}
				m_dlg->SetItemText(124, L"垃圾项");
				m_dlg->SetItemStringAttribute(125, "pos", L"220,52");
				m_dlg->SetItemText(125, mapSummary[1].c_str());
				int nPos = 0;
				WCHAR szPos[100] = {0};
				nPos = (int)mapSummary[1].size();
				wsprintf(szPos, L"%d,52", 30+190+nPos*8);
				m_dlg->SetItemStringAttribute(126, "pos", szPos);

				m_dlg->SetItemText(126,L"，痕迹项");

				ZeroMemory(szPos, 100);
				wsprintf(szPos, L"%d,52", 30+190+nPos*8+50);
				m_dlg->SetItemStringAttribute(127, "pos", szPos);

				m_dlg->SetItemText(127, mapSummary[2].c_str());

				ZeroMemory(szPos, 100);
				int nPos1 = 0;
				nPos1 = (int)mapSummary[2].size();
				wsprintf(szPos, L"%d,52", 30+190+nPos*8+50+nPos1*8+2-nPos1);
				m_dlg->SetItemStringAttribute(128, "pos", szPos);

				m_dlg->SetItemText(128, L"，注册表项");

				ZeroMemory(szPos, 100);
				wsprintf(szPos, L"%d,52", 30+190+nPos*8+52+nPos1*8+60+2-nPos1);
				m_dlg->SetItemStringAttribute(129, "pos", szPos);

				m_dlg->SetItemText(129, mapSummary[3].c_str());				

				int nPos2 = 0;
				ZeroMemory(szPos, 100);
				nPos2 = (int)mapSummary[3].size();
				wsprintf(szPos, L"%d,52", 30+190+nPos*8+52+nPos1*8+60+nPos2*8+4-nPos1-nPos2);
				m_dlg->SetItemStringAttribute(130, "pos", szPos);
				m_dlg->SetItemText(130, L"。");
			}
			else if (mapSummary.size() == 2)
			{
				for (int i = 124; i < 130; i++)
				{
					m_dlg->SetItemVisible(i, TRUE);
				}
				if (mapSummary.find(1) == mapSummary.end())
				{
					m_dlg->SetItemText(124, L"痕迹项");
					m_dlg->SetItemStringAttribute(125, "pos", L"220,52");
					m_dlg->SetItemText(125, mapSummary[2].c_str());

					int nPos = 0;
					WCHAR szPos[100] = {0};
					nPos = (int)mapSummary[2].size();
					wsprintf(szPos, L"%d,52", 30+190+nPos*8+2-nPos);
					m_dlg->SetItemStringAttribute(126, "pos", szPos);

					m_dlg->SetItemText(126, L"，注册表项");

					ZeroMemory(szPos, 100);
					wsprintf(szPos, L"%d,52", 30+200+nPos*8+52+2-nPos);
					m_dlg->SetItemStringAttribute(127, "pos", szPos);

					m_dlg->SetItemText(127, mapSummary[3].c_str());

					ZeroMemory(szPos, 100);
					int nPos1 = 0;
					nPos1 = (int)mapSummary[3].size();
					wsprintf(szPos, L"%d,52", 30+200+nPos*8+50+nPos1*8+4-nPos-nPos1);
					m_dlg->SetItemStringAttribute(128, "pos", szPos);
					m_dlg->SetItemText(128, L"，没有发现垃圾项。");
				}
				else if (mapSummary.find(2) == mapSummary.end())
				{
					m_dlg->SetItemText(124, L"垃圾项");
					m_dlg->SetItemStringAttribute(125, "pos", L"220,52");
					m_dlg->SetItemText(125, mapSummary[1].c_str());
					int nPos = 0;
					WCHAR szPos[100] = {0};
					nPos = (int)mapSummary[1].size();
					wsprintf(szPos, L"%d,52", 30+190+nPos*8-nPos);
					m_dlg->SetItemStringAttribute(126, "pos", szPos);

					m_dlg->SetItemText(126, L"，注册表项");

					ZeroMemory(szPos, 100);
					wsprintf(szPos, L"%d,52", 30+200+nPos*8+52-nPos);
					m_dlg->SetItemStringAttribute(127, "pos", szPos);

					m_dlg->SetItemText(127, mapSummary[3].c_str());

					ZeroMemory(szPos, 100);
					int nPos1 = 0;
					nPos1 = (int)mapSummary[3].size();
					wsprintf(szPos, L"%d,52", 30+200+nPos*8+50+nPos1*8+2-nPos1);
					m_dlg->SetItemStringAttribute(128, "pos", szPos);
					m_dlg->SetItemText(128, L"，没有发现痕迹项。");
				}
				else if (mapSummary.find(3) == mapSummary.end())
				{
					m_dlg->SetItemText(124, L"垃圾项");
					m_dlg->SetItemStringAttribute(125, "pos", L"220,52");
					m_dlg->SetItemText(125, mapSummary[1].c_str());
					int nPos = 0;
					WCHAR szPos[100] = {0};
					nPos = (int)mapSummary[1].size();
					wsprintf(szPos, L"%d,52", 30+190+nPos*8);
					m_dlg->SetItemStringAttribute(126, "pos", szPos);

					m_dlg->SetItemText(126, L"，痕迹项");

					ZeroMemory(szPos, 100);
					wsprintf(szPos, L"%d,52", 30+190+nPos*8+50);
					m_dlg->SetItemStringAttribute(127, "pos", szPos);

					m_dlg->SetItemText(127, mapSummary[2].c_str());

					ZeroMemory(szPos, 100);
					int nPos1 = 0;
					nPos1 = (int)mapSummary[2].size();
					wsprintf(szPos, L"%d,52", 30+190+nPos*8+52+nPos1*8+2-nPos1);
					m_dlg->SetItemStringAttribute(128, "pos", szPos);
					m_dlg->SetItemText(128, L"，没有发现注册表项。");
				}
			}
			else if (mapSummary.size() == 1)
			{
				for (int i = 124; i < 127; i++)
				{
					m_dlg->SetItemVisible(i, TRUE);
				}
				if (mapSummary.find(1) != mapSummary.end())
				{
					m_dlg->SetItemText(124, L"垃圾项");
					m_dlg->SetItemStringAttribute(125, "pos", L"220,52");
					m_dlg->SetItemText(125, mapSummary[1].c_str());
					int nPos = 0;
					WCHAR szPos[100] = {0};
					nPos = (int)mapSummary[1].size();
					wsprintf(szPos, L"%d,52", 30+190+nPos*8+2-nPos);
					m_dlg->SetItemStringAttribute(126, "pos", szPos);
					m_dlg->SetItemText(126, L"，没有发现痕迹项和注册表项。");
				}
				else if (mapSummary.find(2) != mapSummary.end())
				{
					m_dlg->SetItemText(124, L"痕迹项");
					m_dlg->SetItemStringAttribute(125, "pos", L"220,52");
					m_dlg->SetItemText(125, mapSummary[2].c_str());
					int nPos = 0;
					WCHAR szPos[100] = {0};
					nPos = (int)mapSummary[2].size();
					wsprintf(szPos, L"%d,52", 30+190+nPos*8+2-nPos);
					m_dlg->SetItemStringAttribute(126, "pos", szPos);
					m_dlg->SetItemText(126, L"，没有发现垃圾项和注册表项。");
				}
				else if (mapSummary.find(3) != mapSummary.end())
				{
					m_dlg->SetItemText(124, L"注册表项");
					m_dlg->SetItemStringAttribute(125, "pos", L"230,52");
					m_dlg->SetItemText(125, mapSummary[3].c_str());
					int nPos = 0;
					WCHAR szPos[100] = {0};
					nPos = (int)mapSummary[3].size();
					wsprintf(szPos, L"%d,52", 30+200+nPos*8+2-nPos);
					m_dlg->SetItemStringAttribute(126, "pos", szPos);
					m_dlg->SetItemText(126, L"，没有发现垃圾项和痕迹项。");
				}
			}
		}
    }
}

void CUIHandlerOnekey::_UpdateButtonAndLink()
{
    if (!m_ctrlClean.HasBegin())
    {
        m_dlg->SetItemVisible(ID_LNK_ONEKEY_MIDDLE, FALSE);
        m_dlg->SetItemVisible(ID_BTN_ONEKEY_MIDDLE, TRUE);
    }
    else if (m_ctrlClean.HasBegin() && !m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemVisible(ID_LNK_ONEKEY_MIDDLE, TRUE);
        m_dlg->SetItemVisible(ID_BTN_ONEKEY_MIDDLE, FALSE);
        m_dlg->SetItemText(ID_LNK_ONEKEY_MIDDLE, _T("停止清理"));
		m_dlg->SetItemStringAttribute(ID_LNK_ONEKEY_MIDDLE, "pos", _T("-63,52"));
    }
    else if (m_ctrlClean.HasEnd())
    {
        if (m_ctrlClean.IsCanceled())
        {
            m_dlg->SetItemVisible(ID_LNK_ONEKEY_MIDDLE, FALSE);
            m_dlg->SetItemVisible(ID_BTN_ONEKEY_MIDDLE, TRUE);
            m_dlg->SetItemText(ID_BTN_ONEKEY_MIDDLE, _T("重新清理"));
			m_dlg->SetItemVisible(131, TRUE);
        }
        else
        {
            m_dlg->SetItemVisible(ID_LNK_ONEKEY_MIDDLE, TRUE);
            m_dlg->SetItemVisible(ID_BTN_ONEKEY_MIDDLE, FALSE);
            m_dlg->SetItemText(ID_LNK_ONEKEY_MIDDLE, _T("重新进行清理"));
			m_dlg->SetItemStringAttribute(ID_LNK_ONEKEY_MIDDLE, "pos", _T("-86,52"));
			m_dlg->SetItemVisible(131, TRUE);
        }
    }
}

void CUIHandlerOnekey::_UpdateProgress()
{
    if (m_ctrlClean.HasBegin() && !m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemVisible(ID_PRO_ONEKEY_CLEAN, TRUE);
		m_dlg->SetItemVisible(ID_LNK_ONEKEY_HELP, FALSE);
    }
    else
    {
        m_dlg->SetItemVisible(ID_PRO_ONEKEY_CLEAN, FALSE);
		m_dlg->SetItemVisible(ID_LNK_ONEKEY_HELP, TRUE);
    }
}

void CUIHandlerOnekey::OnScanAnim()
{
    static int nIndex = 0;

    if (m_ctrlClean.HasBegin() && !m_ctrlClean.HasEnd())
    {
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_MIDDLE_2, TRUE);
        m_dlg->SetItemIntAttribute(ID_IMG_ONEKEY_MIDDLE_2, "sub", nIndex);
        ++nIndex;
        if (5 == nIndex)
            nIndex = 0;
    }
}

void CUIHandlerOnekey::OnScanProgress()
{
    CleanTask task;

    ++m_nProgress;

    task = m_ctrlClean.GetCurrentTask();

    switch (task)
    {
    case enumCT_Trash:
        m_cleanStates[0].PushData();
        break;

    case enumCT_Track:
        m_cleanStates[1].PushData();
        break;

    case enumCT_Reg:
        m_cleanStates[2].PushData();
        break;
    }
    
    if (enumCT_Trash == task && m_nProgress > 32)
        m_nProgress = 32;

    if (enumCT_Track == task)
    {
        if (m_nProgress < 33)
            m_nProgress = 33;
        if (m_nProgress > 65)
            m_nProgress = 65;
    }

    if (enumCT_Reg == task)
    {
        if (m_nProgress < 66)
            m_nProgress = 66;
        if (m_nProgress > 99)
            m_nProgress = 99;
    }

    m_dlg->SetItemIntAttribute(ID_PRO_ONEKEY_CLEAN, "value", m_nProgress);

    // 更新界面
    UpdateMiddle();
    UpdateButtom();
}

BOOL CUIHandlerOnekey::LoadSetting()
{
    COneCleanSetting& setting = COneCleanSetting::Instance();

    setting.Get("default_entrys", m_strDefault);
    setting.Get("custom_entrys",  m_strCustom);

    setting.Get("last_use", m_strLastUse);
    setting.Get("last_summary", m_strLastSummary);
	setting.Get("makesure", m_strMakeSure);

    if (m_strLastUse.length() && m_strLastSummary.length())
        m_fFirstUse = FALSE;
    else
        m_fFirstUse = TRUE;

    return TRUE;
}

BOOL CUIHandlerOnekey::IsFirstUse()
{
    return m_fFirstUse;
}

static int g_nFake = 0;

void CUIHandlerOnekey::_FakeClean()
{
	m_bCancel = FALSE;
    if (!m_ctrlClean.HasBegin())
    {
		m_dlg->SetItemVisible(131, FALSE);
        // 开始扫描
        _StartClean();
    }
    else if (m_ctrlClean.HasBegin() && !m_ctrlClean.HasEnd())
    {
        // 停止扫描
        _StopClean();
    }
    else if (m_ctrlClean.HasBegin() && m_ctrlClean.HasEnd())
    {
		m_dlg->SetItemVisible(131, FALSE);
        // 重现扫描
        _StartClean();
    }
}

void CUIHandlerOnekey::_StartClean()
{
    if (!_CheckProvider())
        return;

    m_cleanStates[0].ReSet();
    m_cleanStates[1].ReSet();
    m_cleanStates[2].ReSet();
    m_ctrlClean.ReSet();

    // 重置&启动进度条
    m_nProgress = 3;
    m_dlg->SetItemIntAttribute(ID_PRO_ONEKEY_CLEAN, "value", m_nProgress);
    SetTimer(m_dlg->m_hWnd, 108, 200, NULL);

    // 播放扫描动画
    SetTimer(m_dlg->m_hWnd, 107, 100, NULL);       // 扫描动画

    _GetCleanTask();
    _CleanTrash();
}

void CUIHandlerOnekey::_CleanTrash()
{
    if (m_piTrashCleaner)
    {
        m_piTrashCleaner->StartClean(
            (int)m_strCleanTasks.length(), 
            m_strCleanTasks.c_str()
            );
    }
}

void CUIHandlerOnekey::_CleanTrack()
{
    if (m_piTrackCleaner)
    {
        m_piTrackCleaner->StartClean(
            (int)m_strCleanTasks.length(), 
            m_strCleanTasks.c_str()
            );
    }
}

void CUIHandlerOnekey::_CleanReg()
{
    if (m_piRegCleaner)
    {
        m_piRegCleaner->StartClean(
            (int)m_strCleanTasks.length(), 
            m_strCleanTasks.c_str()
            );
    }
}

void CUIHandlerOnekey::_StopClean(BOOL fCanceled)
{
    KillTimer(m_dlg->m_hWnd, 100);
    KillTimer(m_dlg->m_hWnd, 107);
    KillTimer(m_dlg->m_hWnd, 108);

    if (fCanceled)
    {
        m_piTrashCleaner->StopClean();
        m_piTrackCleaner->StopClean();
        m_piRegCleaner->StopClean();
        m_ctrlClean.Cancel();
    }
    m_ctrlClean.UpdateUI();
    UpdateMiddle();
    _SaveReport();
}

void CUIHandlerOnekey::_SaveReport()
{
    COneCleanSetting& setting = COneCleanSetting::Instance();
    SYSTEMTIME time;
    wchar_t szTime[64] = { 0 };

    GetLocalTime(&time);
    wsprintf(szTime, L"%04d.%02d.%02d", time.wYear, time.wMonth, time.wDay);
    setting.Set("last_use", szTime);
    setting.Set("last_summary", m_strLastSummary);
}

LRESULT CUIHandlerOnekey::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT_PTR nIDEvent = (UINT_PTR)wParam;
    if (107 == nIDEvent)
    {
        OnScanAnim();
        return 0;
    }

    if (108 == nIDEvent)
    {
        OnScanProgress();
        return 0;
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CUIHandlerOnekey::OnCleanState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CleanRecord* pCleanRecord = (CleanRecord*)(PVOID)wParam;
    //CurrentCleanEntry* pCleanEntry = NULL;
    CleanEntry entry;

    if (!pCleanRecord)
        goto clean0;

    if (0 == lParam)
    {
        pCleanRecord->m_fBegined = TRUE;
        pCleanRecord->m_dwBeginTime = GetTickCount();
        pCleanRecord->m_dwEndTime = GetTickCount();
        goto clean0;
    }
    
    if (1 == lParam && !m_bCancel)
    {
        pCleanRecord->m_fFinished = TRUE;
        pCleanRecord->m_dwEndTime = GetTickCount();

        // 开始下一项扫描
        if (pCleanRecord == &m_ctrlClean.m_recordTrashClean)
        {
            m_cleanStates[0].PushData();
            _CleanTrack();
        }
        else if (pCleanRecord == &m_ctrlClean.m_recordTrackClean)
        {
            m_cleanStates[1].PushData();
            _CleanReg();
        }
        else if (pCleanRecord == &m_ctrlClean.m_recordRegClean)
        {
            m_cleanStates[2].PushData();
            _StopClean(FALSE);
        }

        goto clean0;
    }

    if (2 == lParam)
    {
        if (!pCleanRecord->m_fFinished)
        {
            pCleanRecord->m_fFinished = TRUE;
            pCleanRecord->m_fCanceled = TRUE;
        }
		m_bCancel = TRUE;
    }

clean0:
    UpdateHeader();
    UpdateMiddle();
    UpdateButtom();

    return 0L;
}

void CUIHandlerOnekey::_GetCleanTask()
{
    std::wstring strCleanTasks;
    COneCleanSetting::Instance().Get("custom_entrys", strCleanTasks);
    m_strCleanTasks = UnicodeToAnsi(strCleanTasks);
}

BOOL CUIHandlerOnekey::IsPopSetting()
{
	BOOL bRet = FALSE;
	time_t theCurrentTime;
	time(&theCurrentTime);
	double dDif = difftime(theCurrentTime, m_theLastPop);
	if (dDif > 60)
	{
		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}
	return bRet;
}


//////////////////////////////////////////////////////////////////////////

COneCleanSetting::COneCleanSetting()
{
    Load();
	LoadDefault();
}

COneCleanSetting::~COneCleanSetting()
{
}

COneCleanSetting& COneCleanSetting::Instance()
{
    static COneCleanSetting _singleton;
    return _singleton;
}

bool COneCleanSetting::Load()
{
    bool retval = false;
    TiXmlDocument xmlDoc;
    const TiXmlElement *pXmlSetting = NULL;
    const TiXmlElement *pXmlStrings = NULL;
    const TiXmlElement *pXmlIntegers = NULL;
    const TiXmlElement *pXmlChild = NULL;
    KFilePath settingPath = KFilePath::GetFilePath(_Module.m_hInst);

    settingPath.RemoveFileSpec();
    settingPath.Append(L"cfg\\onekeyclean.xml");
    if (!xmlDoc.LoadFile(UnicodeToAnsi(settingPath.value()).c_str(), TIXML_ENCODING_UTF8))
        goto clean0;

    pXmlSetting = xmlDoc.FirstChildElement("setting");
    if (!pXmlSetting)
        goto clean0;

    pXmlStrings = pXmlSetting->FirstChildElement("strings");
    if (pXmlStrings)
    {
        pXmlChild = pXmlStrings->FirstChildElement("entry");
        while (pXmlChild)
        {
            std::string strName, strValue;

            strName = pXmlChild->Attribute("name");
            strValue = pXmlChild->Attribute("value");
			if (strcmp(strName.c_str(), "default_entrys") != 0)
			{
				m_vStringStore[strName] = Utf8ToUnicode(strValue);
			}

            pXmlChild = pXmlChild->NextSiblingElement("entry");
        }
    }

    pXmlIntegers = pXmlSetting->FirstChildElement("integers");
    if (pXmlIntegers)
    {
        pXmlChild = pXmlIntegers->FirstChildElement("entry");
        while (pXmlChild)
        {
            std::string strName;
            int nValue;

            strName = pXmlChild->Attribute("name");
            pXmlChild->QueryIntAttribute("value", &nValue);
            m_vIntegerStore[strName] = nValue;

            pXmlChild = pXmlChild->NextSiblingElement("entry");
        }
    }

    retval = true;

clean0:
    return retval;
}

bool COneCleanSetting::LoadDefault()
{
	bool retval = false;
	TiXmlDocument xmlDoc;
	const TiXmlElement *pXmlSetting = NULL;
	const TiXmlElement *pXmlStrings = NULL;
	const TiXmlElement *pXmlIntegers = NULL;
	const TiXmlElement *pXmlChild = NULL;
	KFilePath settingPath = KFilePath::GetFilePath(_Module.m_hInst);

	settingPath.RemoveFileSpec();
	settingPath.Append(L"cfg\\defaultoclean.xml");
	if (!xmlDoc.LoadFile(UnicodeToAnsi(settingPath.value()).c_str(), TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlSetting = xmlDoc.FirstChildElement("setting");
	if (!pXmlSetting)
		goto clean0;

	pXmlStrings = pXmlSetting->FirstChildElement("strings");
	if (pXmlStrings)
	{
		pXmlChild = pXmlStrings->FirstChildElement("entry");
		while (pXmlChild)
		{
			std::string strName, strValue;

			strName = pXmlChild->Attribute("name");
			strValue = pXmlChild->Attribute("value");
			m_vStringStore[strName] = Utf8ToUnicode(strValue);

			pXmlChild = pXmlChild->NextSiblingElement("entry");
		}
	}

	pXmlIntegers = pXmlSetting->FirstChildElement("integers");
	if (pXmlIntegers)
	{
		pXmlChild = pXmlIntegers->FirstChildElement("entry");
		while (pXmlChild)
		{
			std::string strName;
			int nValue;

			strName = pXmlChild->Attribute("name");
			pXmlChild->QueryIntAttribute("value", &nValue);
			m_vIntegerStore[strName] = nValue;

			pXmlChild = pXmlChild->NextSiblingElement("entry");
		}
	}

	retval = true;

clean0:
	return retval;
}

bool COneCleanSetting::Save()
{
    bool retval = false;
    TiXmlDocument xmlDoc;  
    TiXmlDeclaration *pXmlDecl = new TiXmlDeclaration("1.0", "utf-8", "yes"); 
    TiXmlElement *pXmlSetting = new TiXmlElement("setting");
    TiXmlElement *pXmlStrings = new TiXmlElement("strings");
    TiXmlElement *pXmlIntegers = new TiXmlElement("integers");
    TiXmlElement *pXmlChild = NULL;
    StringStore::const_iterator i;
    IntegerStore::const_iterator j;
    KFilePath savePath = KFilePath::GetFilePath(_Module.m_hInst);

    savePath.RemoveFileSpec();
    savePath.Append(L"cfg\\onekeyclean.xml");
    xmlDoc.LinkEndChild(pXmlDecl);

    for (i = m_vStringStore.begin(); i != m_vStringStore.end(); ++i)
    {
        std::string strName;
        std::wstring strValue;

        strName = i->first;
        strValue = i->second;

        pXmlChild = new TiXmlElement("entry");
        if (!pXmlChild)
            goto clean0;
		if (strcmp(strName.c_str(), "default_entrys") == 0)
			continue;
        pXmlChild->SetAttribute("name", strName.c_str());
        pXmlChild->SetAttribute("value", UnicodeToUtf8(strValue).c_str());
        pXmlStrings->LinkEndChild(pXmlChild);
    }
    pXmlSetting->LinkEndChild(pXmlStrings);

    for (j = m_vIntegerStore.begin(); j != m_vIntegerStore.end(); ++j)
    {
        std::string strName;
        int nValue;

        strName = j->first;
        nValue = j->second;

        pXmlChild = new TiXmlElement("entry");
        if (!pXmlChild)
            goto clean0;

        pXmlChild->SetAttribute("name", strName.c_str());
        pXmlChild->SetAttribute("value", nValue);
        pXmlIntegers->LinkEndChild(pXmlChild);
    }
    pXmlSetting->LinkEndChild(pXmlIntegers);

    xmlDoc.LinkEndChild(pXmlSetting);

    retval = xmlDoc.SaveFile(UnicodeToAnsi(savePath.value()).c_str());

clean0:
    return retval;
}

void COneCleanSetting::Get(const std::string& strName, std::wstring& strValue)
{
    StringStore::const_iterator i;

    i = m_vStringStore.find(strName);
    if (i != m_vStringStore.end())
        strValue = i->second;
}

void COneCleanSetting::Get(const std::string& strName, int& nValue)
{
    IntegerStore::const_iterator i;

    i = m_vIntegerStore.find(strName);
    if (i != m_vIntegerStore.end())
        nValue = i->second;
}

void COneCleanSetting::Set(const std::string& strName, const std::wstring& strValue)
{
    m_vStringStore[strName] = strValue;
    Save();
}

void COneCleanSetting::Set(const std::string& strName, int nValue)
{
    m_vIntegerStore[strName] = nValue;
    Save();
}

//////////////////////////////////////////////////////////////////////////

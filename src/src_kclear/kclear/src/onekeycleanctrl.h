#pragma once

//////////////////////////////////////////////////////////////////////////

#include "kuires/kuifontpool.h"
#include "kscbase/kscres.h"
#include "kclear/ionekeyclean.h"
#include "kscviewlog.h"
#include "pphtmldrawer.h"
#include <map>

//////////////////////////////////////////////////////////////////////////

class DCanvas
{
public:
    DCanvas(HDC dc, HFONT font) : m_hDC(dc), m_hFont(font)
    {
    }

    void drawBitmap(HBITMAP hBitmap, int x, int y, int cx, int cy, int x1, int y1)
    {
        CDC memdc;
        memdc.CreateCompatibleDC(m_hDC);
        SelectObject(memdc, hBitmap);
        BitBlt(m_hDC, x, y, cx, cy, memdc, x1, y1, SRCCOPY);
    }

    void drawText(const std::wstring& strText, RECT& rcText, COLORREF dwColor=RGB(0,0,0))
    {
        HFONT hOldFont = (HFONT)SelectObject(m_hDC, m_hFont);
        SetTextColor(m_hDC, dwColor);
        SetBkMode(m_hDC, TRANSPARENT);
        DrawText(m_hDC, strText.c_str(), (int)strText.length(), &rcText, DT_VCENTER|DT_CENTER|DT_SINGLELINE);
        SelectObject(m_hDC, hOldFont);
    }

	void drawHtml(const std::wstring& strText, RECT& rcText)
	{
		POINT pt = { rcText.left, rcText.top };
		fillRect(rcText, RGB(255,255,255));
		m_htmlDrawer.Draw(m_hDC, strText.c_str(), &pt);
		CDCHandle dc(m_hDC);
		/*dc.SetBkColor(RGB(251,252,253));
		dc.SetBkMode(TRANSPARENT);*/
	//	fillRect(rcText, RGB(251,252,253));
	}

    void drawText2(const std::wstring& strText, RECT& rcText, COLORREF dwColor=RGB(0,0,0))
    {
        HFONT hOldFont = (HFONT)SelectObject(m_hDC, m_hFont);
        SetTextColor(m_hDC, dwColor);
        SetBkMode(m_hDC, TRANSPARENT);
        DrawText(m_hDC, strText.c_str(), (int)strText.length(), &rcText, DT_TOP|DT_LEFT|DT_SINGLELINE);
        SelectObject(m_hDC, hOldFont);
    }

    void fillRect(RECT& rcZone, COLORREF dwColor=RGB(255,255,255))
    {
        CDCHandle dc(m_hDC);
        dc.FillSolidRect(&rcZone, dwColor);
    }

private:
    HDC m_hDC;
    HFONT m_hFont;
	CPPHtmlDrawer m_htmlDrawer;
};

class DImage
{
public:
    DImage(int x, int y, HBITMAP hBitmap, int nCount, int nSub)
        : m_nCount(nCount)
        , m_nSub(nSub)
        , m_hBitmap(hBitmap)
    {
        m_rcZone.left = x;
        m_rcZone.right = 0;
        m_rcZone.top = y;
        m_rcZone.bottom = 0;
        _AutoSize();
    }

    void GetZone(RECT& rcZone)
    {
        rcZone = m_rcZone;
    }

    void OnDraw(DCanvas& canvas)
    {
        canvas.drawBitmap(m_hBitmap, 0, 0, m_rcZone.Width(), m_rcZone.Height(),
            m_rcZone.Width() * m_nSub, 0);
    }

    void GetClientRect(RECT& rcClient)
    {
        rcClient.left = 0;
        rcClient.right = m_rcZone.Width();
        rcClient.top = 0;
        rcClient.bottom = m_rcZone.Height();
    }

    void _AutoSize()
    {
        BITMAP bm;
        ::GetObject(m_hBitmap, sizeof(BITMAP), (PSTR)&bm); 
        m_rcZone.right = m_rcZone.left + bm.bmWidth / m_nCount;
        m_rcZone.bottom = m_rcZone.top + bm.bmHeight;
    }

private:
    HBITMAP m_hBitmap;
    CRect m_rcZone;
    int m_nCount;
    int m_nSub;
};

class DLabel
{
public:
    DLabel(
        HWND hDelegate,
        const std::wstring& strText, 
        int x, int y, 
        COLORREF dwColor
        ) : m_hDelegate(hDelegate), m_strText(strText), m_dwColor(dwColor)
    {
        m_rcZone.left = x;
        m_rcZone.right = 0;
        m_rcZone.top = y;
        m_rcZone.bottom = 0;
    }

    void GetZone(RECT& rcZone)
    {
        rcZone = m_rcZone;
    }

    BOOL HitTest(POINT pt)
    {
        return PtInRect(&m_rcZone, pt);
    }

    void AutoSize(HFONT hFont)
    {
        CPaintDC dc(m_hDelegate);
        CRect rcTest;
        SelectObject(dc, hFont);
        SIZE size;
        dc.GetTextExtent(m_strText.c_str(), (int)m_strText.length(), &size);
        dc.DrawText(m_strText.c_str(), (int)m_strText.length(), &rcTest, 
            DT_SINGLELINE|DT_CALCRECT);
        m_rcZone.right = m_rcZone.left + rcTest.Width() + 5;
        m_rcZone.bottom = m_rcZone.top + rcTest.Height() + 5;
    }

    void OnDraw(DCanvas& canvas)
    {
		CRect rcClient;
		GetClientRect(rcClient);
		canvas.fillRect(rcClient);
		canvas.drawHtml(m_strText, rcClient);
    }

    void GetClientRect(RECT& rcClient)
    {
        rcClient.left = 0;
        rcClient.right = m_rcZone.Width();
        rcClient.top = 0;
        rcClient.bottom = m_rcZone.Height();
    }

private:
    HWND m_hDelegate;
    std::wstring m_strText;
    COLORREF m_dwColor;
    CRect m_rcZone;
};

class DLink
{
public:
    DLink(
        HWND hDelegate,
        const std::wstring& strText, 
        int nCommand,
        int x, int y
        ) : m_hDelegate(hDelegate), m_strText(strText), m_nCommand(nCommand)
        , m_fDown(FALSE), m_fHover(FALSE)
    {
        m_rcZone.left = x;
        m_rcZone.right = 0;
        m_rcZone.top = y;
        m_rcZone.bottom = 0;
    }

    void SetText(const std::wstring& strText)
    {
        m_strText = strText;
    }

    void AutoSize(HFONT hFont)
    {
        CPaintDC dc(m_hDelegate);
        CRect rcTest;
        SelectObject(dc, hFont);
        dc.DrawText(m_strText.c_str(), (int)m_strText.length(), &rcTest, 
            DT_TOP|DT_LEFT|DT_SINGLELINE|DT_CALCRECT);
        m_rcZone.right = m_rcZone.left + rcTest.Width();
        m_rcZone.bottom = m_rcZone.top + rcTest.Height();
    }

    void GetClientRect(RECT& rcClient)
    {
        rcClient.left = 0;
        rcClient.right = m_rcZone.Width();
        rcClient.top = 0;
        rcClient.bottom = m_rcZone.Height();
    }

    void GetZone(RECT& rcZone)
    {
        rcZone = m_rcZone;
    }

    BOOL HitTest(POINT pt)
    {
        return PtInRect(&m_rcZone, pt);
    }

    void OnMouseHover()
    {
        m_fHover = TRUE;
    }

    void OnMouseLeave()
    {
        m_fHover = FALSE;
    }

    void OnLButtonDown()
    {
        m_fDown = TRUE;
    }

    void OnClick()
    {
        PostMessage(m_hDelegate, WM_COMMAND, m_nCommand & 0xFF, 0);
    }

    void OnLButtonUp()
    {
        m_fDown = FALSE;
    }

    void OnDraw(DCanvas& canvas)
    {
        CRect rcClient;
        GetClientRect(rcClient);
        canvas.fillRect(rcClient);
        canvas.drawText2(m_strText, rcClient, RGB(10,73,140));
    }

private:
    HWND m_hDelegate;
    std::wstring m_strText;
    int m_nCommand;
    BOOL m_fDown;
    BOOL m_fHover;
    CRect m_rcZone;
};

//////////////////////////////////////////////////////////////////////////

typedef enum tagCleanState {
    enumCS_NotStart = 0,    // 还没有开始清理
    enumCS_Cleaning,        // 正在清理
    enumCS_End              // 清理完成
} CleanState;

typedef enum tagCleanTask {
    enumCT_Trash = 0,       // 清理垃圾
    enumCT_Track,           // 清理痕迹
    enumCT_Reg,             // 清理注册表
    enumCT_None
} CleanTask;

typedef struct tagCleanEntry {
    std::wstring strType;
    std::wstring strContent;
    double nSize;
} CleanEntry;

//////////////////////////////////////////////////////////////////////////

class CleanRecord 
{
public:
    CleanRecord()
    {
        m_fInCount = TRUE;

        m_fBegined = FALSE;
        m_fFinished = FALSE;
        m_fCanceled = FALSE;

        m_dwBeginTime = 0;
        m_dwEndTime = 0;
    }

    void ReSet()
    {
        m_fBegined = FALSE;
        m_fFinished = FALSE;
        m_fCanceled = FALSE;

        m_dwBeginTime = 0;
        m_dwEndTime = 0;

        m_vEntrys.clear();
    }

    std::wstring m_strName;
    std::vector<CleanEntry> m_vEntrys;
    std::wstring m_strCurrent;
    BOOL m_fBegined;
    BOOL m_fFinished;
    BOOL m_fCanceled;
    ULONG m_dwBeginTime;
    ULONG m_dwEndTime;
    BOOL m_fInCount;

    int GetSize()
    {
        int retval = 0;
        double file_size = 0;

        if (m_fInCount)
        {
            retval = (int)m_vEntrys.size();
        }
        else
        {
            for (size_t i = 0; i < m_vEntrys.size(); ++i)
            {
                file_size += m_vEntrys[i].nSize;
            }

            if (file_size > 0 && file_size < 1)
                file_size = 1.0;
            retval =  (int)file_size;
        }

        return retval;
    }

    int GetTime()
    {
        int retval;

        retval = (m_dwEndTime - m_dwBeginTime) / 1000;
        if (0 == retval)
            retval = 1;

        return retval;
    }

	void GetCurrentItem(std::wstring& strState, COLORREF& dwColor)
	{
		KAppRes& appRes = KAppRes::Instance();
		std::wstring strText;
		CString strTemp;
		dwColor = RGB(0, 125, 1);

		strState = L"";

		// 清理前
		if (!m_fBegined && !m_fFinished)
		{
			dwColor = RGB(130, 130, 130);
			strState = L"<font color=#828282>";
			strState += L"未扫描";
			strState += L"</font>";
			
			return;
		}

		// 清理中
		if (m_fBegined && !m_fFinished)
		{
			dwColor = RGB(0, 125, 1);
			strState = L"<font color=#007F01>";
			strState += L"正在扫描：";

			if (m_vEntrys.empty())
			{
				strState += m_strCurrent;
			}
			else
			{
				strState += m_strCurrent;
			}
			strState += L"</font>";
			return;
		}

		// 清理后
		if (!m_fBegined && m_fFinished)
		{
			// 还没有开始的
			strState = L"未扫描";
			return;
		}

		if (m_fBegined && m_fFinished)
		{
			dwColor = RGB(0, 125, 1);
			strState = L"<font color=#007F01>";
			if (m_fCanceled)
			{
				strState += L"扫描已被取消";
			}
			else
			{
				strState += L"扫描已完成";
			}
			strState += L"</font>";
		}

	}

    void GetCleanState(std::wstring& strState, COLORREF& dwColor, BOOL* pfCouldView = NULL)
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strText;
        std::wstring strFormat;
        CString strTemp;

        strState = L"";
        dwColor = RGB(0,0,0);

        if (pfCouldView)
            *pfCouldView = FALSE;

        // 清理前
        if (!m_fBegined && !m_fFinished)
        {
            appRes.GetString("IDS_CTL_WAIT_CLEAN", strText);
            strState += strText;
            return;
        }

        // 清理中
        if (m_fBegined && !m_fFinished)
        {
            dwColor = RGB(0,0,0);

            if (m_vEntrys.empty())
            {
                appRes.GetString("IDS_CTL_CLEANING2", strFormat);
                strTemp.Format(
                    strFormat.c_str(),
                    m_strName.c_str(),
                    GetTime(),
                    m_strCurrent.c_str()
                    );
                strState += (const wchar_t*)strTemp;
            }
            else
            {
                std::wstring strUnit;
                if (m_fInCount)
                {
                    appRes.GetString("IDS_COUNT", strUnit);
                }
                else
                {
                    strUnit = L"M";
                }

                appRes.GetString("IDS_CTL_CLEANING1", strFormat);
                strTemp.Format(
                    strFormat.c_str(),
                    GetSize(),
                    strUnit.c_str(),
                    m_strName.c_str(),
                    GetTime(),
                    m_strCurrent.c_str()
                    );
                strState += (const wchar_t*)strTemp;
            }
        }

        // 清理后
        if (!m_fBegined && m_fFinished)
        {
            // 还没有开始的
            appRes.GetString("IDS_NOT_START", strFormat);
            strTemp.Format(strFormat.c_str(), m_strName.c_str());
            strState += (const wchar_t*)strTemp;
        }

        if (m_fBegined && m_fFinished)
        {
            if (m_fCanceled)
            {
                appRes.GetString("IDS_CTL_CLEAN_END_WITH_CANCLE", strText);
                strState += strText;
            }

            if (m_vEntrys.empty())
            {
                appRes.GetString("IDS_CTL_CLEAN_END_WITHOUT_RESULT", strFormat);
                strTemp.Format(
                    strFormat.c_str(),
                    m_strName.c_str()
                    );
                strState += (const wchar_t*)strTemp;
            }
            else
            {
                std::wstring strUnit;

                if (pfCouldView)
                    *pfCouldView = TRUE;

                if (m_fInCount)
                {
                    appRes.GetString("IDS_COUNT", strUnit);
                }
                else
                {
                    strUnit = L"M";
                }

                appRes.GetString("IDS_CTL_CLEAN_END_WITH_RESULT", strFormat);
                strTemp.Format(
                    strFormat.c_str(),
                    GetSize(),
                    strUnit.c_str(),
                    m_strName.c_str(),
                    GetTime()
                    );
                strState += (const wchar_t*)strTemp;
            }
        }
    }

    void GetReport(std::wstring& strText1, std::wstring& strText2)
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strUnit;
        std::wstring strFormat;
        CString strTemp;
        size_t i;

        strText1 = L"";
        strText2 = L"";

        if (m_fInCount)
        {
            appRes.GetString("IDS_COUNT", strUnit);
        }
        else
        {
            strUnit = L"M";
        }

        appRes.GetString("IDS_CLEAN_REPORT_TITLE", strFormat);
        strTemp.Format(
            strFormat.c_str(),
            m_strName.c_str(),
            GetSize(),
            strUnit.c_str(),
            GetTime()
            );
        strText1 = (const wchar_t*)strTemp;

        for (i = 0; i < m_vEntrys.size(); ++i)
        {
            strText2 += m_vEntrys[i].strContent;
            strText2 += L"\r\n";
        }
    }

};

//////////////////////////////////////////////////////////////////////////

class COnekeyCleanCtrl : public CWindowImpl<COnekeyCleanCtrl>
{
public:
    DECLARE_WND_CLASS(L"OnekeyCleanCtrl")

    COnekeyCleanCtrl()
    {
        KAppRes& appRes = KAppRes::Instance();

        m_hHandCursor = LoadCursor(NULL, IDC_HAND);
        m_hNormalCursor = LoadCursor(NULL, IDC_ARROW);

 //       m_hSmallIcons = appRes.GetImage("IDB_ONEKEY_CLEAN_SMALL_ICONS");
		m_hRegIcon = appRes.GetImage("IDB_ONEKEY_REG");
		m_hTrashIcon = appRes.GetImage("IDB_ONEKEY_TRASH");
		m_hTrackIcon = appRes.GetImage("IDB_ONEKEY_TRACK");
		m_hSpliteLine = appRes.GetImage("IDB_ONEKEY_SPLITELINE");

        appRes.GetString("IDS_TRASH", m_recordTrashClean.m_strName);
        appRes.GetString("IDS_TRACK", m_recordTrackClean.m_strName);
        appRes.GetString("IDS_REGTRASH", m_recordRegClean.m_strName);

        m_recordTrashClean.m_fInCount = FALSE;
    }

    ~COnekeyCleanCtrl()
    {

    }

    CleanRecord m_recordTrashClean;
    CleanRecord m_recordTrackClean;
    CleanRecord m_recordRegClean;


//     CleanRecord m_trashCleanRecord;
//     CleanRecord m_trackCleanRecord;
//     CleanRecord m_regCleanRecord;

    void Init()
    {
        GetClientRect(&m_rcClient);
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL retval = FALSE;
        retval = CWindowImpl<COnekeyCleanCtrl>::SubclassWindow(hWnd);
        Init();
        return retval;
    }

    void ReSet()
    {
        m_recordTrashClean.ReSet();
        m_recordTrackClean.ReSet();
        m_recordRegClean.ReSet();
    }

    void Cancel()
    {
        CleanRecord* pCleanRecords[3] = { 0 };
        int i;
        BOOL fStoped = FALSE;

        pCleanRecords[0] = &m_recordTrashClean;
        pCleanRecords[1] = &m_recordTrackClean;
        pCleanRecords[2] = &m_recordRegClean;

        for (i = 0; i < 3; ++i)
        {
            if (pCleanRecords[i]->m_fBegined && !pCleanRecords[i]->m_fFinished)
            {
                pCleanRecords[i]->m_fFinished = TRUE;
                pCleanRecords[i]->m_fCanceled = TRUE;
                fStoped = TRUE;
                continue;
            }

            if (fStoped)
            {
                pCleanRecords[i]->m_fFinished = TRUE;
                pCleanRecords[i]->m_fCanceled = TRUE;
            }
        }
    }

    void GetCleanState(std::wstring& strState)
    {
        
    }

    void GetCurrentTask(std::wstring& strTask)
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strFormat;
        CString strTemp;
        CleanRecord* pCleanRecord = _GetCleanRecord(GetCurrentTask());

        if (!pCleanRecord)
        {
            strTask = L"";
            return;
        }

        appRes.GetString("IDS_CLEAN_TASK", strFormat);
//         strTemp.Format(
//             strFormat.c_str(), 
//             pCleanRecord->m_strName.c_str(),
//             pCleanRecord->m_strCurrent.c_str()
//             );
        strTask = strFormat;
    }

    CleanRecord* _GetCleanRecord(CleanTask nTask)
    {
        if (enumCT_Trash)
            return &m_recordTrashClean;

        if (enumCT_Track)
            return &m_recordTrackClean;

        if (enumCT_Reg)
            return &m_recordRegClean;

        return NULL;
    }

    CleanTask GetCurrentTask()
    {
        CleanTask retval = enumCT_None;

        if (m_recordTrashClean.m_fBegined &&
            !m_recordTrashClean.m_fFinished)
        {
            retval = enumCT_Trash;
            goto clean0;
        }

        if (m_recordTrackClean.m_fBegined &&
            !m_recordTrackClean.m_fFinished)
        {
            retval = enumCT_Track;
            goto clean0;
        }

        if (m_recordRegClean.m_fBegined &&
            !m_recordRegClean.m_fFinished)
        {
            retval = enumCT_Reg;
            goto clean0;
        }

clean0:
        return retval;
    }
	void GetSummaryEx(std::map<int ,std::wstring>& mapSummary)
	{
		WCHAR szSummary[100] = {0};
		std::wstring strSummary;
		int nRegSize = 0;
		int nTrashSize = 0;
		int nTrackSize = 0;

		if (m_recordTrashClean.m_vEntrys.empty() &&
			m_recordTrackClean.m_vEntrys.empty() &&
			m_recordRegClean.m_vEntrys.empty())
		{
			return;
		}

		nRegSize = m_recordRegClean.GetSize();
		nTrackSize = m_recordTrackClean.GetSize();
		nTrashSize = m_recordTrashClean.GetSize();
		if (nRegSize != 0)
		{
			swprintf_s(szSummary, 99, L"%d条", nRegSize);
			mapSummary[3] = szSummary;
			ZeroMemory(szSummary, 100);
		}
		if (nTrashSize != 0)
		{
			swprintf_s(szSummary, 99, L"%dM", nTrashSize);
			mapSummary[1] = szSummary;
			ZeroMemory(szSummary, 100);
		}
		if (nTrackSize != 0)
		{
			swprintf_s(szSummary, 99, L"%d条", nTrackSize);
			mapSummary[2] = szSummary;
			ZeroMemory(szSummary, 100);
		}
	}

    void GetSummary(std::wstring& strSummary)
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strHeader;
        std::wstring strTail;
        std::wstring strFormat1;
        std::wstring strFormat2;
        std::wstring strUnit;
        CString strTemp;
        CleanRecord* pCleanRecords[3] = { 0 };
        int i;

        strSummary = L"";
        appRes.GetString("IDS_CLEAN_COUNT", strFormat1);
        appRes.GetString("IDS_NOT_FOUND", strFormat2);
        
        if (m_recordTrashClean.m_vEntrys.empty() &&
            m_recordTrackClean.m_vEntrys.empty() &&
            m_recordRegClean.m_vEntrys.empty())
        {
            appRes.GetString("IDS_ALL_NOT_FOUND", strSummary);
            return;
        }

        pCleanRecords[0] = &m_recordTrashClean;
        pCleanRecords[1] = &m_recordTrackClean;
        pCleanRecords[2] = &m_recordRegClean;

        for (i = 0; i < 3; ++i)
        {
            if (pCleanRecords[i]->m_fInCount)
            {
                appRes.GetString("IDS_COUNT", strUnit);
            }
            else
            {
                strUnit = L"M";
            }

            if (pCleanRecords[i]->m_vEntrys.size())
            {
                strTemp.Format(
                    strFormat1.c_str(), 
                    pCleanRecords[i]->m_strName.c_str(),
                    pCleanRecords[i]->GetSize(),
                    strUnit.c_str()
                    );
                if (strHeader.length())
                    strHeader += L"，";
                strHeader += (const wchar_t*)strTemp;
            }
            else
            {
                strTemp.Format(
                    strFormat2.c_str(),
                    pCleanRecords[i]->m_strName.c_str()
                    );
                if (strTail.length())
                    strTail += L"，";
                strTail += (const wchar_t*)strTemp;
            }
        }

        strSummary = L"一共清理";
        strSummary += strHeader;
        if (strTail.length())
        {
            strSummary += L"，"; 
            strSummary += strTail;
        }
        strSummary += L"。";
    }

    int _GetTime()
    {
        int retval = 0;

        retval += (m_recordTrashClean.m_dwEndTime - m_recordTrashClean.m_dwBeginTime);
        retval += (m_recordTrackClean.m_dwEndTime - m_recordTrackClean.m_dwBeginTime);
        retval += (m_recordRegClean.m_dwEndTime - m_recordRegClean.m_dwBeginTime);

        retval = retval / 1000;
        if (0 == retval)
            retval = 1;

        return retval;
    }

    void GetTimeUse(std::wstring& strTime)
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strFormat;
        CString strTemp;
        int nTime = _GetTime();

        //appRes.GetString("IDS_TIME_USE", strFormat);
        strTemp.Format(_T("%d秒"), nTime);

        strTime = (const wchar_t*)strTemp;
    }

    BOOL HasBegin()
    {
       if (m_recordTrashClean.m_fBegined || 
           m_recordTrackClean.m_fBegined ||
           m_recordRegClean.m_fBegined)
       {
           return TRUE;
       }
       else
       {
           return FALSE;
       }
    }

    BOOL HasEnd()
    {
        if (m_recordTrashClean.m_fFinished &&
            m_recordTrackClean.m_fFinished &&
            m_recordRegClean.m_fFinished)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    BOOL IsCanceled()
    {
        if (m_recordTrashClean.m_fCanceled || 
            m_recordTrackClean.m_fCanceled ||
            m_recordRegClean.m_fCanceled)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    void UpdateUI()
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strText;
        COLORREF dwColor;

        m_vLabels.clear();
        m_vLinks.clear();
        m_vImages.clear();

        {
            DImage image_1(85, 24+33, m_hTrashIcon, 1, 0);
            m_vImages.push_back(image_1);

			DImage image_4(65, 24+33+51, m_hSpliteLine, 1, 0);
			m_vImages.push_back(image_4);

            DImage image_2(85, 100+33, m_hTrackIcon, 1, 0);
            m_vImages.push_back(image_2);

			DImage image_5(65, 100+33+51, m_hSpliteLine, 1, 0);
			m_vImages.push_back(image_5);

            DImage image_3(85, 180+33, m_hRegIcon, 1, 0);
            m_vImages.push_back(image_3);
        }

        if (!HasBegin())
        {
            // 还没有开始清理
            // 显示"系统垃圾","使用痕迹","冗余注册表"的介绍
            appRes.GetString("IDS_CLEAN_TRASH_AD", strText);
            DLabel adLabel_1(m_hWnd, strText, 125, 50+30, RGB(0,0,0));
            adLabel_1.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_1);

			m_recordTrashClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_11(m_hWnd, strText, 229, 56, dwColor);
			adLabel_11.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_11);

            appRes.GetString("IDS_CLEAN_TRACK_AD", strText);
            DLabel adLabel_2(m_hWnd, strText, 125, 125+30, RGB(0,0,0));
            adLabel_2.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_2);


			m_recordTrackClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_12(m_hWnd, strText, 229, 132, dwColor);
			adLabel_12.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_12);

            appRes.GetString("IDS_CLEAN_REG_AD", strText);
            DLabel adLabel_3(m_hWnd, strText, 125, 203+30, RGB(0,0,0));
            adLabel_3.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_3);


			m_recordRegClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_13(m_hWnd, strText, 229, 212, dwColor);
			adLabel_13.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_13);
        }
        else if (HasBegin() && !HasEnd())
        {
            // 清理过程中
            m_recordTrashClean.GetCleanState(strText, dwColor);
            DLabel adLabel_1(m_hWnd, strText, 125, 50+30, dwColor);
            adLabel_1.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_1);

			m_recordTrashClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_11(m_hWnd, strText, 229, 56, dwColor);
			adLabel_11.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_11);

            m_recordTrackClean.GetCleanState(strText, dwColor);
            DLabel adLabel_2(m_hWnd, strText, 125, 125+30, dwColor);
            adLabel_2.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_2);

			m_recordTrackClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_12(m_hWnd, strText, 229, 132, dwColor);
			adLabel_12.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_12);

            m_recordRegClean.GetCleanState(strText, dwColor);
            DLabel adLabel_3(m_hWnd, strText, 125, 203+30, dwColor);
            adLabel_3.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_3);

			m_recordRegClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_13(m_hWnd, strText, 229, 212, dwColor);
			adLabel_13.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_13);
        }
        else if (HasBegin() && HasEnd())
        {
            // 清理结束
            BOOL fCouldView = FALSE;

            m_recordTrashClean.GetCleanState(strText, dwColor, &fCouldView);
            DLabel adLabel_1(m_hWnd, strText, 125, 50+30, dwColor);
            adLabel_1.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_1);

			m_recordTrashClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_11(m_hWnd, strText, 229, 56, dwColor);
			adLabel_11.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_11);

            if (fCouldView)
            {
                CRect rcZone;
                adLabel_1.GetZone(rcZone);
                DLink link(m_hWnd, L"查看详情", 0, /*rcZone.left + rcZone.Width()*/470, rcZone.top);
                link.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
                m_vLinks.push_back(link);
            }

            m_recordTrackClean.GetCleanState(strText, dwColor, &fCouldView);
            DLabel adLabel_2(m_hWnd, strText, 125, 125+30, dwColor);
            adLabel_2.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_2);

			m_recordTrackClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_12(m_hWnd, strText, 229, 132, dwColor);
			adLabel_12.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_12);

            if (fCouldView)
            {
                CRect rcZone;
                adLabel_2.GetZone(rcZone);
                DLink link(m_hWnd, L"查看详情", 1, /*rcZone.left + rcZone.Width()*/470, rcZone.top);
                link.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
                m_vLinks.push_back(link);
            }

            m_recordRegClean.GetCleanState(strText, dwColor, &fCouldView);
            DLabel adLabel_3(m_hWnd, strText, 125, 203+30, dwColor);
            adLabel_3.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
            m_vLabels.push_back(adLabel_3);

			m_recordRegClean.GetCurrentItem(strText, dwColor);
			DLabel adLabel_13(m_hWnd, strText, 229, 212, dwColor);
			adLabel_13.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
			m_vLabels.push_back(adLabel_13);

            if (fCouldView)
            {
                CRect rcZone;
                adLabel_3.GetZone(rcZone);
                DLink link(m_hWnd, L"查看详情", 2, /*rcZone.left + rcZone.Width()*/470, rcZone.top);
                link.AutoSize(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
                m_vLinks.push_back(link);
            }
        }

        InvalidateRect(NULL);
    }

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return 1L;
    }

    void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
    {
        std::wstring strText1, strText2;
        std::wstring strName;
        KAppRes& appRes = KAppRes::Instance();

        appRes.GetString("IDS_VIEW_DETAIL", strName);

        if (0 == nID)
        {
            m_recordTrashClean.GetReport(strText1, strText2);
            CKscViewLog viewLog(strName.c_str(), strText1.c_str(), strText2.c_str());
            viewLog.DoModal();
        }
        else if (1 == nID)
        {
            m_recordTrackClean.GetReport(strText1, strText2);
            CKscViewLog viewLog(strName.c_str(), strText1.c_str(), strText2.c_str());
            viewLog.DoModal();
        }
        else if (2 == nID)
        {
            m_recordRegClean.GetReport(strText1, strText2);
            CKscViewLog viewLog(strName.c_str(), strText1.c_str(), strText2.c_str());
            viewLog.DoModal();
        }
    }

    void OnSize(UINT nType, CSize size)
    {
        GetClientRect(&m_rcClient);
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        size_t i;
        int nHover = -1;
        BOOL fHand = FALSE;

        for (i = 0; i < m_vLinks.size(); ++i)
        {
            if (m_vLinks[i].HitTest(point))
            {
                fHand = TRUE;
                break;
            }
        }

        if (fHand)
        {
            if (GetCursor() != m_hHandCursor)
                SetCursor(m_hHandCursor);
        }
        else
        {
            SetCursor(m_hNormalCursor);
        }
    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        size_t i;
        int nDown = -1;

        for (i = 0; i < m_vLinks.size(); ++i)
        {
            if (m_vLinks[i].HitTest(point))
            {
                m_vLinks[i].OnClick();
				SetMsgHandled(TRUE);
                break;
            }
        }

		SetMsgHandled(FALSE);
    }

    void OnPaint(CDCHandle /*dc*/)
    {
        CPaintDC dc(m_hWnd);
        CBitmap membmp;
        CDC memdc;
        CRect rcClient(m_rcClient);

        membmp.CreateCompatibleBitmap(dc, rcClient.Width(), rcClient.Height());
        memdc.CreateCompatibleDC(dc);
        SelectObject(memdc, membmp);
        OnRealPaint(memdc);
        dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), memdc, 0, 0, SRCCOPY);
    }

    void OnRealPaint(CDC& dc)
    {
        KAppRes& appRes = KAppRes::Instance();
        std::wstring strText;
        CString strPaint;
        size_t i;
        CDC memdc;
        HGDIOBJ hOldObj = NULL;
        HFONT hOldFont = NULL;
        HBITMAP hSmallIcons = appRes.GetImage("IDB_ONEKEY_CLEAN_SMALL_ICONS");

        dc.FillSolidRect(&m_rcClient, RGB(255, 255, 255));

        memdc.CreateCompatibleDC(dc);
        dc.SetBkMode(TRANSPARENT);
        hOldFont = (HFONT)SelectObject(dc, KuiFontPool::GetFont(0x0300));
        SetTextColor(dc, RGB(51,100,163));
        dc.TextOut(125, 24+32, _T("系统垃圾文件"));
        dc.TextOut(125, 100+32, _T("软件使用痕迹"));
        dc.TextOut(125, 180+32, _T("注册表冗余项"));
        
        SelectObject(dc, hOldFont);

        CDC memdc2;
        CBitmap membmp;
        memdc2.CreateCompatibleDC(dc);
        membmp.CreateCompatibleBitmap(dc, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top);
        SelectObject(memdc2, membmp);
        DCanvas canvas(memdc2, KuiFontPool::GetFont(FALSE, FALSE, FALSE));
        DCanvas canvas2(memdc2, KuiFontPool::GetFont(FALSE, TRUE, FALSE));

        for (i = 0; i < m_vLabels.size(); ++i)
        {
            CRect rcLabel;
            m_vLabels[i].GetZone(rcLabel);
            m_vLabels[i].OnDraw(canvas);
            dc.BitBlt(
                rcLabel.left, 
                rcLabel.top, 
                rcLabel.Width(), 
                rcLabel.Height(), 
                memdc2, 
                0, 
                0, 
                SRCCOPY
                );
        }

        for (i = 0; i < m_vLinks.size(); ++i)
        {
            CRect rcLink;
            m_vLinks[i].GetZone(rcLink);
            m_vLinks[i].OnDraw(canvas2);
            dc.BitBlt(
                rcLink.left, 
                rcLink.top, 
                rcLink.Width(), 
                rcLink.Height(), 
                memdc2, 
                0, 
                0, 
                SRCCOPY
                );
        }

        for (i = 0; i < m_vImages.size(); ++i)
        {
            CRect rcImage;
            m_vImages[i].GetZone(rcImage);
            m_vImages[i].OnDraw(canvas2);
            dc.BitBlt(
                rcImage.left, 
                rcImage.top, 
                rcImage.Width(), 
                rcImage.Height(), 
                memdc2, 
                0, 
                0, 
                SRCCOPY
                );
        }
    }

    BEGIN_MSG_MAP_EX(COnekeyCleanCtrl)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_COMMAND(OnCommand)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT(OnPaint)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

protected:
    RECT m_rcClient;
    std::vector<DLabel> m_vLabels;
    std::vector<DLink> m_vLinks;
    std::vector<DImage> m_vImages;
    HCURSOR m_hHandCursor;
    HCURSOR m_hNormalCursor;
    
    //CleanTask m_nCleanTask;

//     CleanRecord m_recordTrashClean;
//     CleanRecord m_recordTrackClean;
//     CleanRecord m_recordRegClean;
//    HBITMAP m_hSmallIcons;
	HBITMAP m_hTrackIcon;
	HBITMAP m_hTrashIcon;
	HBITMAP m_hRegIcon;
	HBITMAP m_hSpliteLine;
};

//////////////////////////////////////////////////////////////////////////

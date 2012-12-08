//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "skinprogress.h"
#include "guidef.h"

//////////////////////////////////////////////////////////////////////////

const WCHAR gsThreeDots[] = L"...";

//////////////////////////////////////////////////////////////////////////

CSkinProgress::CSkinProgress()
{
}

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSkinProgress, CStatic)
    //{{AFX_MSG_MAP(CProgressST)
    ON_WM_PAINT()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSkinProgress::SubclassWindow(
	HWND hWnd,
	UINT unBmpID)
{
    BOOL bRetCode = CStatic::SubclassWindow(hWnd);
	if (bRetCode == FALSE)
		return FALSE;

	CBitmap TempBmp;

	TempBmp.LoadBitmap(unBmpID);

	Init(&TempBmp);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::Init(
	CBitmap *pBmp,
	int nUpper,
	int nType,
	BOOL bReverse)
{
	mpBaseBmp = NULL;
	mpCompatibleBmp = NULL;
	mpStretchBmp = NULL;
	mpProgressBmp = NULL;
	mPaneRect.SetRect(0, 0, 0, 0);

	if (!SetBitmap(pBmp, false))
	{
		return false;
	}else
	{
		mnType = nType;

		mnLower = 0;
		mnUpper = nUpper;
		mnLeft = 0;
		mnRight = 0;

		SetRange(0, nUpper, 1, false ,false);

		mbReverse = bReverse;

		Reset(false);

		SetTimer((UINT) this, 500, NULL);

		OnCalcPos();

		return true;
	}
}

//////////////////////////////////////////////////////////////////////////

CSkinProgress::~CSkinProgress()
{
	Clear();

	if (IsWindow(m_hWnd))
	{
		KillTimer((UINT)this);
	}

	if (mpBaseBmp != NULL)
	{
		delete mpBaseBmp;
	}

	if (mpCompatibleBmp != NULL)
	{
		delete mpCompatibleBmp;
	}

	if (mpStretchBmp != NULL)
	{
		delete mpStretchBmp;
	}

	if (mpProgressBmp != NULL)
	{
		delete mpProgressBmp;
	}
}

//////////////////////////////////////////////////////////////////////////

void CSkinProgress::Clear()
{
	if (IsWindow(m_hWnd))
	{
		ModifyStyle(WS_VISIBLE, 0);
		DestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::SetBitmap(
	CBitmap *pTempBitmap,
	BOOL bDisplay)
{ 
	BITMAP sBmpSrc;
	HGDIOBJ hOldBitmap;
	HGDIOBJ hOldStretch;
	CRect WndRect;
	CDC BmpDC;
	CDC StretchDC;

	mPaintRect.SetRect(0, 0, 0, 0);

	GetClientRect(&WndRect);

	CPaintDC PaintDC(CWnd::FromHandle(m_hWnd));

	if (mpBaseBmp != NULL)
		delete mpBaseBmp;

	mpBaseBmp = new CBitmap;

	if (mpCompatibleBmp != NULL)
		delete mpCompatibleBmp;

	mpCompatibleBmp = new CBitmap;

	if (pTempBitmap == NULL)
		return FALSE;

	CopyBitmap(mpBaseBmp, pTempBitmap);

	ConvertBitmap(mpCompatibleBmp, mpBaseBmp, &PaintDC);

	mpCompatibleBmp->GetBitmap(&sBmpSrc);
	mnBmpWidth  = sBmpSrc.bmWidth / SKIN_PROGRESS_BITMAP_COUNT;
	mnBmpHeight = sBmpSrc.bmHeight;

	if (mpStretchBmp != NULL)
		delete mpStretchBmp;

	mpStretchBmp = new CBitmap;

	mpStretchBmp->CreateCompatibleBitmap(&PaintDC, sBmpSrc.bmWidth, WndRect.Height() - 2);

	BmpDC.CreateCompatibleDC(&PaintDC);
	hOldBitmap = BmpDC.SelectObject(*mpCompatibleBmp);

	StretchDC.CreateCompatibleDC(&PaintDC);
	hOldStretch = StretchDC.SelectObject(*mpStretchBmp);

	StretchDC.SetStretchBltMode(HALFTONE);

	StretchDC.StretchBlt(0, 0, sBmpSrc.bmWidth, WndRect.Height() - 2,&BmpDC, 0, 0, sBmpSrc.bmWidth, sBmpSrc.bmHeight, SRCCOPY);

	StretchDC.SelectObject(hOldStretch);
	StretchDC.DeleteDC();

	BmpDC.SelectObject(hOldBitmap);
	BmpDC.DeleteDC();

	mpStretchBmp->GetBitmap(&sBmpSrc);

	mnBmpWidth  = sBmpSrc.bmWidth / SKIN_PROGRESS_BITMAP_COUNT;
	mnBmpHeight = sBmpSrc.bmHeight;

	mBarImageList.Detach();

	if(!mBarImageList.Create(mnBmpWidth, mnBmpHeight, ILC_COLOR32, 0, 0))
	{
		return false;
	}else
	{
		mBarImageList.Add(mpStretchBmp, RGB(0, 255, 255));

		if (bDisplay != false)
		{
			OnCalcPos();
		}

		return true;
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::CopyBitmap(
	CBitmap *pSrcBmp,
	CBitmap *pDestBmp)
{ 
	BITMAP sBmpSrc;
	unsigned char *pDataBmp;
	DWORD nDataCount;

	if (pSrcBmp == NULL || pDestBmp == NULL)
	{
		return false;
	}else
	{
		pDestBmp->GetBitmap(&sBmpSrc);
		nDataCount = sBmpSrc.bmWidthBytes * sBmpSrc.bmHeight;
		pDataBmp = new unsigned char[nDataCount];

		if (pDataBmp != NULL)
		{
			pDestBmp->GetBitmapBits(nDataCount, pDataBmp);
			sBmpSrc.bmBits = pDataBmp;

			pSrcBmp->CreateBitmapIndirect(&sBmpSrc);

			delete[] pDataBmp;

			return TRUE;
		}else
		{
			return FALSE;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::ConvertBitmap(
	CBitmap *pDestBmp,
	CBitmap *pSrcBmp,
	CDC *pDC)
{ 
	BITMAP SrcBmpInfo;
	BITMAPINFO sBmpInfoSrc;
	HBITMAP BMPInfo;
	unsigned char *pDataBmp;
	DWORD dwDataCount;
	int nResult;

	pDataBmp = NULL;

	if (pDestBmp != NULL && pSrcBmp != NULL)
	{
		if (pDC == NULL)
		{
            CPaintDC PaintDC(CWnd::FromHandle(m_hWnd));
			pDC = &PaintDC;
		}

		pSrcBmp->GetBitmap(&SrcBmpInfo);

		if (SrcBmpInfo.bmBitsPixel == GetDeviceCaps(*pDC, BITSPIXEL))
		{
			nResult = (int) CopyBitmap(pDestBmp, pSrcBmp);
		}else
		{
			dwDataCount = SrcBmpInfo.bmWidthBytes * SrcBmpInfo.bmHeight;
			pDataBmp = new unsigned char[dwDataCount];

			if (pDataBmp != NULL)
			{
				pSrcBmp->GetBitmapBits(dwDataCount, pDataBmp);
				SrcBmpInfo.bmBits = pDataBmp;

				sBmpInfoSrc.bmiColors[0].rgbBlue = 0;
				sBmpInfoSrc.bmiColors[0].rgbGreen = 0;
				sBmpInfoSrc.bmiColors[0].rgbRed = 0;
				sBmpInfoSrc.bmiColors[0].rgbReserved = 0;

				sBmpInfoSrc.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				sBmpInfoSrc.bmiHeader.biWidth = SrcBmpInfo.bmWidth;
				sBmpInfoSrc.bmiHeader.biHeight = 0 - SrcBmpInfo.bmHeight;
				sBmpInfoSrc.bmiHeader.biPlanes = SrcBmpInfo.bmPlanes;
				sBmpInfoSrc.bmiHeader.biBitCount = SrcBmpInfo.bmBitsPixel;
				sBmpInfoSrc.bmiHeader.biCompression = BI_RGB;
				sBmpInfoSrc.bmiHeader.biSizeImage = 0;
				sBmpInfoSrc.bmiHeader.biXPelsPerMeter = 0;
				sBmpInfoSrc.bmiHeader.biYPelsPerMeter = 0;
				sBmpInfoSrc.bmiHeader.biClrUsed = 0;
				sBmpInfoSrc.bmiHeader.biClrImportant = 0;

				BMPInfo = CreateDIBitmap(pDC->m_hDC, &sBmpInfoSrc.bmiHeader, CBM_INIT, pDataBmp, &sBmpInfoSrc, DIB_RGB_COLORS);

                
                CBitmap TempBmp;
                TempBmp.Attach(BMPInfo);
                

				nResult = (int) CopyBitmap(pDestBmp, &TempBmp);

				DeleteObject(BMPInfo);
				delete[] pDataBmp;

				if (nResult != 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::SetRange(
	int nLower,
	int nUpper,
	int nStep,
	BOOL bResamble,
	BOOL bDisplay)
{
	if (IsWindow(m_hWnd))
	{
		if(bResamble != false)
		{
			mnLeft = (int)((((LONGLONG)(mnLeft  - mnLower) * (nUpper - nLower)) / (mnUpper - mnLower) ) + nLower);
			mnPrevLeft  = mnLeft;
	
			mnRight = (int)((((LONGLONG)(mnRight - mnLower) * (nUpper - nLower)) / (mnUpper - mnLower)) + nLower);
			m_nPrevRight = mnRight;
		}

		mnLower = nLower;
		mnUpper = nUpper;
		mnStep = nStep;

		if(bDisplay != false)
		{
			OnCalcPos();
		}

		return true;
	}else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::SetPos(
	int nPos,
	BOOL bDisplay)
{
	if (IsWindow(m_hWnd))
	{
		if (nPos > mnUpper)
		{
			nPos = mnUpper;
		}

		if (nPos < mnLower)
		{
			nPos = mnLower;
		}

		if (nPos < mnLeft)
		{
			nPos = mnLeft;
		}

		m_nPrevRight = mnRight;
		mnRight = nPos;

		if (bDisplay != false)
		{
			OnCalcPos();
		}

		return m_nPrevRight;
	}else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::OffsetPos(
	int nOffset,
	BOOL bDisplay)
{ 
	if (IsWindow(m_hWnd))
	{
		m_nPrevRight = mnRight;
		mnRight += nOffset;

		if (mnRight > mnUpper)
		{
			mnRight = mnUpper;
		}

		if (mnRight < mnLower)
		{
			mnRight = mnLower;
		}

		if (mnRight < mnLeft)
		{
			mnRight = mnLeft;
		}

		if (bDisplay != false)
		{
			OnCalcPos();
		}

		return m_nPrevRight;
	}else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::SetStep(
	int nStep)             
{ 
	int nPrevStep;

	if (IsWindow(m_hWnd))
	{
		nPrevStep = mnStep;
		mnStep = nStep;

		return nPrevStep;
	}else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::StepIt(
	BOOL bDisplay) 
{ 
	if (IsWindow(m_hWnd))
	{
		m_nPrevRight = mnRight;
		mnRight += mnStep;

		if (mnRight > mnUpper)
		{
			mnRight = mnUpper;
		}

		if (mnRight < mnLower)
		{
			mnRight = mnLower;
		}

		if (mnRight < mnLeft)
		{
			mnRight = mnLeft;
		}

		if (bDisplay != false)
		{
			OnCalcPos();
		}

		return m_nPrevRight;
	}else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::SetSize(
	int i_nSize,
	BOOL i_bDisplay)
{
	if (i_nSize > 0)
	{
		i_nSize += 5;
	}else if(i_nSize < 0)
	{
		i_nSize -= 2;
	}

	mnSize = i_nSize;

	if(i_bDisplay != false)
	{
		OnCalcPos();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetSize()
{
    return mnSize;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetPos(
	BOOL i_bPercent)
{
	if (i_bPercent == false)
	{
		return mnRight;
	}else
	{
		return (int)(((LONGLONG)(mnRight - mnLower) * 25600) / (mnUpper - mnLower));
	}
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetStep()
{
	return mnStep;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetLower()
{
	return mnLower;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetUpper()
{
	return mnUpper;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::SetStart(
	int nStart,
	BOOL bDisplay)
{
	if (IsWindow(m_hWnd))
	{
		if (nStart > mnUpper)
		{
			nStart = mnUpper;
		}

		if (nStart < mnLower)
		{
			nStart = mnLower;
		}

		if (nStart > mnRight)
		{
			nStart = mnRight;
		}

		mnPrevLeft = mnLeft;
		mnLeft = nStart;

		if (bDisplay != false)
		{
			OnCalcPos();
		}

		return mnPrevLeft;
	}else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetStart(
	BOOL i_bPercent)
{
	if (i_bPercent == false)
	{
		return mnLeft;
	}else
	{
		return (int)(((LONGLONG)(mnLeft - mnLower) * 25600) / (mnUpper - mnLower));
	}
}

//////////////////////////////////////////////////////////////////////////

void CSkinProgress::Reset(
	BOOL bDisplay)
{
	FILETIME sStartTimeFile;

	GetLocalTime(&msStartTimeSystem);
	SystemTimeToFileTime(&msStartTimeSystem, &sStartTimeFile);
	mnStartTimeLongLong = (((LONGLONG)sStartTimeFile.dwHighDateTime << 32) | sStartTimeFile.dwLowDateTime);

	SetPos(mnLower, bDisplay);
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::SetProgress(
	int nProgress,
	BOOL bDisplay)
{
	mnType = nProgress;

	if(bDisplay != false)
	{
		OnCalcPos();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::GetProgress()
{
	return mnType;
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::OnCalcPos() 
{
	CRect RectText;
	CRect RectPane;

	if (IsWindow(m_hWnd) == FALSE)
		return FALSE;

	GetClientRect(&RectPane);

	if (IsWindowVisible() == FALSE)
		return TRUE;

	if ( (mPaneRect != RectPane) && ( (RectPane.Width()  > 2) && (RectPane.Height() > 2) ) )
	{
		mPaneRect = RectPane;
		Invalidate();
	}else
	{
		if (CalcPos())
		{
			Invalidate();
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

int CSkinProgress::CompactText(
	CString *psString, 
	CDC *pDC,
	int nMaxWidth,
	int nAddWidth)
{
	int nLenString;
	int nLenDots;
	int nCountChar;
	TCHAR nFirstChar;
  
	if (psString != NULL)
	{
		if (pDC == NULL)
		{
            CPaintDC PaintDC(CWnd::FromHandle(m_hWnd));
			pDC = &PaintDC;
		}

		nLenString = psString->GetLength();
		if (psString->Right(3) == "...")
		{
			nLenString -= 3;
			*psString = psString->Left(nLenString);
		}

		CSize TextSize;

		pDC->GetTextExtent(*psString, nLenString);

		if ((nLenString == 0) || ((TextSize.cx + nAddWidth) <= nMaxWidth))
		{
			return 0;
		}else
		{
			nFirstChar = psString->GetAt(0);

			pDC->GetTextExtent(gsThreeDots, (sizeof(gsThreeDots) / sizeof(WCHAR)) - 1);
			nLenDots = TextSize.cx;

			pDC->GetTextExtent(*psString, nLenString);

			CSize TextSize2;

			pDC->GetTextExtent(&nFirstChar, sizeof(nFirstChar));
			if (TextSize.cx <= TextSize2.cx + nLenDots)
			{
				return (0 - nLenString);
			}else
			{
				nCountChar = 0; 
        
				pDC->GetTextExtent(*psString, nLenString);

				while ( (nLenString > 1) && ( (TextSize.cx + (nLenDots >> 1) + nAddWidth) > nMaxWidth) )
				{
					*psString = psString->Left(psString->GetLength() - 1);
					psString->TrimRight();
					nLenString  = psString->GetLength();
					nCountChar += 1;
				}

				*psString += gsThreeDots;

				return nCountChar;
			}
		}
	}else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::GetTimed(
	CString *o_poTimed)
{
	CString oStrTempo;
	unsigned long nTempo;

	SYSTEMTIME sCurrentTimeSystem;
	SYSTEMTIME sTempoTimeSystem;
	FILETIME sTempoTimeFile;
	LONGLONG nTempoTimeLongLong;
	LONGLONG nDeltaTimeLongLong;

	GetLocalTime(&sTempoTimeSystem);
	sCurrentTimeSystem = sTempoTimeSystem;
	SystemTimeToFileTime(&sTempoTimeSystem, &sTempoTimeFile);
	nTempoTimeLongLong = (((LONGLONG)sTempoTimeFile.dwHighDateTime << 32) | sTempoTimeFile.dwLowDateTime);

	*o_poTimed = " ";
	if (mnType > 0)
	{
		*o_poTimed += "(";
	}

	if ((sTempoTimeSystem.wDay != msStartTimeSystem.wDay) || 
		(sTempoTimeSystem.wMonth != msStartTimeSystem.wMonth) || 
		(sTempoTimeSystem.wYear  != msStartTimeSystem.wYear) )
	{
		oStrTempo.Format(
			L"%04d/%02d/%02d/%02d ", 
			msStartTimeSystem.wYear, 
			msStartTimeSystem.wMonth, 
			msStartTimeSystem.wDay, 
			msStartTimeSystem.wDayOfWeek + 1);

		*o_poTimed += oStrTempo;
	}

	oStrTempo.Format(
		L"%02d:%02d:%02d - ",
		msStartTimeSystem.wHour,
	    msStartTimeSystem.wMinute,
		msStartTimeSystem.wSecond);

	*o_poTimed += oStrTempo;

	nDeltaTimeLongLong = nTempoTimeLongLong - mnStartTimeLongLong;
	nTempoTimeLongLong = nDeltaTimeLongLong;
	sTempoTimeFile.dwLowDateTime  = (DWORD) nTempoTimeLongLong;
	sTempoTimeFile.dwHighDateTime = (DWORD) ((LONGLONG) nTempoTimeLongLong >> 32);
	FileTimeToSystemTime(&sTempoTimeFile, &sTempoTimeSystem);
	sTempoTimeSystem.wYear -= 1600;

	if ((sTempoTimeSystem.wYear  != 1) || 
		(sTempoTimeSystem.wMonth != 1) || 
		(sTempoTimeSystem.wDay   != 1) )
	{
		nTempo = (unsigned long)(nTempoTimeLongLong / (nDeltaTimeLongLong / 864000000000));

		if (nTempo != 0)
		{
			oStrTempo.Format(L"%d:", nTempo);
			*o_poTimed += oStrTempo;
		}
	}

	oStrTempo.Format(
		L"%02d:%02d:%02d - ",
		sTempoTimeSystem.wHour,
		sTempoTimeSystem.wMinute,
		sTempoTimeSystem.wSecond);

	*o_poTimed += oStrTempo;

	nTempo = (int)(((LONGLONG)(mnRight -  mnLeft) * 25600) / (mnUpper - mnLower));
	oStrTempo.Format(L"%d%% / ", nTempo >> 8);
	*o_poTimed += oStrTempo;

	oStrTempo.Format(L"%d%% - ", 100 - (nTempo >> 8));
	*o_poTimed += oStrTempo;

	if (nTempo == 0)
	{
		nTempoTimeLongLong = 0;
	}else
	{
		nTempoTimeLongLong = ((nDeltaTimeLongLong * 25600) / nTempo) - nDeltaTimeLongLong;
	}

	sTempoTimeFile.dwLowDateTime  = (DWORD) nTempoTimeLongLong;
	sTempoTimeFile.dwHighDateTime = (DWORD) ((LONGLONG) nTempoTimeLongLong >> 32);
	FileTimeToSystemTime(&sTempoTimeFile, &sTempoTimeSystem);
	sTempoTimeSystem.wYear -= 1600;

	if ((sTempoTimeSystem.wDay != 1) || 
		(sTempoTimeSystem.wMonth != 1) || 
		(sTempoTimeSystem.wYear  != 1) )
	{
		nTempo = (unsigned long)(nDeltaTimeLongLong / 864000000000);

		if (nTempo != 0)
		{
			oStrTempo.Format(L"%d:", nTempo);
			*o_poTimed += oStrTempo;
		}
	}

	oStrTempo.Format(L"%02d:%02d:%02d - ", sTempoTimeSystem.wHour, sTempoTimeSystem.wMinute, sTempoTimeSystem.wSecond);
	*o_poTimed += oStrTempo;

	if (nTempo == 0)
	{
		nTempoTimeLongLong = mnStartTimeLongLong + nDeltaTimeLongLong;
	}else
	{
		nTempoTimeLongLong = mnStartTimeLongLong + ((nDeltaTimeLongLong * 25600) / nTempo); // Start + 100% (100% = Elapsed / Percent)
	}

	sTempoTimeFile.dwLowDateTime  = (DWORD) nTempoTimeLongLong;
	sTempoTimeFile.dwHighDateTime = (DWORD) ((LONGLONG) nTempoTimeLongLong >> 32);
	FileTimeToSystemTime(&sTempoTimeFile, &sTempoTimeSystem);

	if ( (sTempoTimeSystem.wYear  != sCurrentTimeSystem.wYear) && 
		 (sTempoTimeSystem.wMonth != sCurrentTimeSystem.wMonth) && 
		 (sTempoTimeSystem.wDay   != sCurrentTimeSystem.wDay) )
	{
		oStrTempo.Format(
			L"%04d/%02d/%02d/%02d ",
			sTempoTimeSystem.wYear,
			sTempoTimeSystem.wMonth,
			sTempoTimeSystem.wDay,
			sTempoTimeSystem.wDayOfWeek + 1);

		*o_poTimed += oStrTempo;
	}

	oStrTempo.Format(
		L"%02d:%02d:%02d",
		sTempoTimeSystem.wHour,
		sTempoTimeSystem.wMinute,
		sTempoTimeSystem.wSecond);

	*o_poTimed += oStrTempo;

	if (mnType > 0)
	{
		*o_poTimed += ")";
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::CalcPos(
	CRect *pRectPaint) 
{
	CRect WndRect;
	int nCurrentPos;
	int nLenPos;
	int nStepPos;

	GetClientRect(&WndRect);

	if (pRectPaint != NULL)
	{
		*pRectPaint = WndRect;
	}

    nLenPos  = WndRect.Width();
    nStepPos = mnBmpWidth;

	mnEndAff = nLenPos - nStepPos;

	mnRightAff = (int)(((LONGLONG)(mnRight - mnLower) * (nLenPos  - (nStepPos << 1))) / (mnUpper - mnLower)) + nStepPos;

	mnLeftAff  = (int)(((LONGLONG)(mnLeft - mnLower) * (nLenPos - (nStepPos << 1))) / (mnUpper - mnLower));

	if (mnRightAff > mnEndAff)
	{
		mnRightAff = mnEndAff;
	}

	if (mnLeftAff > (mnRightAff - nStepPos))
	{
		mnLeftAff = mnRightAff - nStepPos;
	}

	if (mbReverse != false)
	{
		nCurrentPos = nLenPos - mnRightAff - nStepPos;
		mnRightAff = nLenPos - mnLeftAff  - nStepPos;
		mnLeftAff  = nCurrentPos;
	}

	if ((mnPrevEndAff   != mnEndAff) || 
		(mnPrevRightAff != mnRightAff) || 
		(mnPrevLeftAff  != mnLeftAff) )
	{
		return true;
	}else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL CSkinProgress::OnEraseBkgnd(
	CDC* dc)
{
	OnCalcPos();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void CSkinProgress::OnPaint() 
{
    CPaintDC PaintDC(CWnd::FromHandle(m_hWnd));
	CDC MemDC;
	HGDIOBJ hOldBitmap;
	CRect PaintRect;
	int nCurrentPos;
	int nStepPos;

	CalcPos(&PaintRect);

	PaintRect.bottom -= 2;

    nStepPos = mnBmpWidth;

	if ( (PaintRect.Width()  != mPaintRect.Width()) || (PaintRect.Height() != mPaintRect.Height()) )
	{
		if (mpProgressBmp != NULL)
		{
			delete mpProgressBmp;
		}

		mpProgressBmp = new CBitmap;
		mpProgressBmp->CreateCompatibleBitmap(
			&PaintDC, 
			PaintRect.Width(), 
			PaintRect.Height());
	}

	MemDC.CreateCompatibleDC(&PaintDC);
	hOldBitmap = MemDC.SelectObject(*mpProgressBmp);

	if (PaintRect.Width() != mPaintRect.Width() || PaintRect.Height() != mPaintRect.Height())
	{
		nCurrentPos = mnEndAff - (mnEndAff % nStepPos);
		for (; nCurrentPos > mnRightAff; nCurrentPos -= nStepPos)
		{
			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BACKGROUND, CPoint(nCurrentPos, 0), ILD_NORMAL);
		}

		mnPrevRightAff = mnRightAff;

		nCurrentPos -= (nCurrentPos % nStepPos);
		for (; nCurrentPos >= mnLeftAff; nCurrentPos -= nStepPos)
		{
			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BAR, CPoint(nCurrentPos, 0), ILD_NORMAL);
		}

		mnPrevLeftAff = mnLeftAff;

		nCurrentPos -= (nCurrentPos % nStepPos);
		for (; nCurrentPos >= nStepPos; nCurrentPos -= nStepPos)
		{
			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BACKGROUND, CPoint(nCurrentPos, 0), ILD_NORMAL);
		}

		mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_CENTER, CPoint((mnLeftAff + mnRightAff) >> 1, 0), ILD_NORMAL);

		if (mnRightAff < (mnEndAff - nStepPos))
		{
			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_AFTER, CPoint(mnRightAff + mnBmpWidth, 0), ILD_NORMAL);
		}

		mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_END, CPoint(mnEndAff, 0), ILD_NORMAL);
		mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_LEFT, CPoint(mnLeftAff, 0), ILD_NORMAL);
		mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_RIGHT, CPoint(mnRightAff, 0), ILD_NORMAL);
		mnPrevEndAff = mnEndAff;
		mPaintRect  = PaintRect;
	}else
	{
		if(mnPrevRightAff != mnRightAff)
		{
			nCurrentPos = mnPrevRightAff;

			if (mnRightAff < mnPrevRightAff)
			{
				mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BACKGROUND, CPoint(nCurrentPos, 0), ILD_NORMAL);
				nCurrentPos -= (nCurrentPos % nStepPos);
				for(; nCurrentPos > mnRightAff; nCurrentPos -= nStepPos)
				{
					mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BACKGROUND, CPoint(nCurrentPos, 0), ILD_NORMAL);
				}
			}else
			{
				mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BAR, CPoint(nCurrentPos, 0), ILD_NORMAL);

				nCurrentPos += nStepPos;
				nCurrentPos -= (nCurrentPos % nStepPos);
				for (; nCurrentPos < mnRightAff; nCurrentPos += nStepPos)
				{
					mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BAR, CPoint(nCurrentPos, 0), ILD_NORMAL);
				}
			}
		}

		if (mnLeftAff != mnPrevLeftAff)
		{
			nCurrentPos = mnPrevLeftAff;

			if (mnLeftAff < mnPrevLeftAff)
			{
				mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BAR, CPoint(nCurrentPos, 0), ILD_NORMAL);
				nCurrentPos -= (nCurrentPos % nStepPos);
				for (; nCurrentPos > mnLeftAff; nCurrentPos -= nStepPos)
				{
					mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BAR, CPoint(nCurrentPos, 0), ILD_NORMAL);
				}
			}else
			{
				mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BACKGROUND, CPoint(nCurrentPos, 0), ILD_NORMAL);
				nCurrentPos += nStepPos;
				nCurrentPos -= (nCurrentPos % nStepPos);
				for (; nCurrentPos < mnLeftAff; nCurrentPos += nStepPos)
				{
					mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_BACKGROUND, CPoint(nCurrentPos, 0), ILD_NORMAL);
				}
			}
		}

		if (mnPrevRightAff != mnRightAff || mnPrevLeftAff  != mnLeftAff)
		{
			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_CENTER, CPoint((mnLeftAff + mnRightAff) >> 1, 0), ILD_NORMAL);
		}

		if (mnRightAff != mnPrevRightAff)
		{
			if(mnRightAff < (mnEndAff - nStepPos))
			{
				mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_AFTER, CPoint(mnRightAff + nStepPos, 0), ILD_NORMAL);
			}
		}

		if(mnPrevRightAff != mnRightAff)
		{
			if(mnRightAff > (mnEndAff - (nStepPos << 1)))
			{
				mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_END, CPoint(mnEndAff, 0), ILD_NORMAL);
			}
		}

		if ( (mnPrevLeftAff != mnLeftAff) || (mnRightAff < (mnLeftAff + (nStepPos << 1))) )
		{
			mnPrevLeftAff = mnLeftAff;
			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_LEFT, CPoint(mnLeftAff, 0), ILD_NORMAL);
		}

		if ( (mnPrevRightAff != mnRightAff) || (mnLeftAff> (mnRightAff - (nStepPos << 1))) )
		{
			mnPrevRightAff = mnRightAff;

			mBarImageList.Draw(&MemDC, SKIN_PROGRESS_BITMAP_RIGHT, CPoint(mnRightAff, 0), ILD_NORMAL);
		}

		if (mnPrevEndAff != mnEndAff)
		{
			mnPrevEndAff = mnEndAff;
		}
	}

	PaintDC.BitBlt(0, 0, PaintRect.Width(), PaintRect.Height(), &MemDC, 0, 0, SRCCOPY);

	PaintDC.SelectObject(gGuiFont.mFont);

	PaintDC.SetBkMode(TRANSPARENT);
	PaintDC.SetTextColor(RGB(0, 0, 0));

	PaintRect.top += 1;

	CString sText;

	sText.Format(L"%d%%", mnRight);

	PaintDC.DrawText(sText, -1, PaintRect, DT_CENTER | DT_VCENTER);

	MemDC.SelectObject(hOldBitmap);
	MemDC.DeleteDC();
}

//////////////////////////////////////////////////////////////////////////

void CSkinProgress::OnTimer(
	UINT nIDEvent)
{
	if (nIDEvent == (UINT)this)
	{
		OnCalcPos();
	}
}

//////////////////////////////////////////////////////////////////////////
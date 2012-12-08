//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

typedef enum SKIN_PROGRESS_BITMAP
{
	SKIN_PROGRESS_BITMAP_LEFT = 0,
	SKIN_PROGRESS_BITMAP_CENTER,
	SKIN_PROGRESS_BITMAP_BAR,
	SKIN_PROGRESS_BITMAP_RIGHT,
	SKIN_PROGRESS_BITMAP_AFTER,
	SKIN_PROGRESS_BITMAP_BACKGROUND,
	SKIN_PROGRESS_BITMAP_END,
	SKIN_PROGRESS_BITMAP_COUNT,
	SKIN_PROGRESS_BITMAP_TOP				= SKIN_PROGRESS_BITMAP_LEFT,
	SKIN_PROGRESS_BITMAP_BOTTOM				= SKIN_PROGRESS_BITMAP_RIGHT,
};

//////////////////////////////////////////////////////////////////////////

typedef enum SKIN_PROGRESS_TEXT
{
	SKIN_PROGRESS_TEXT_NONE = 0,
	SKIN_PROGRESS_TEXT_PERCENT,
	SKIN_PROGRESS_TEXT_TIMED,
	SKIN_PROGRESS_TEXT_AUTOSIZE,
	SKIN_PROGRESS_TEXT_COUNT
};

//////////////////////////////////////////////////////////////////////////

class CSkinProgress;

//////////////////////////////////////////////////////////////////////////

class CSkinProgress : public CStatic
{
public:
	CSkinProgress();

	virtual ~CSkinProgress();

	BOOL SubclassWindow(
		HWND hWnd,
		UINT unBmpID);
    BOOL SetBitmap(
		CBitmap *pBitmap = NULL,
		BOOL bDisplay = true);

    BOOL CopyBitmap(
		CBitmap *pSrcBmp,
		CBitmap *pDestBmp);

    BOOL ConvertBitmap(
		CBitmap *pDestBmp,
		CBitmap *pSrcBmp,
		CDC *pDC = NULL);

	BOOL SetRange(
		int nLower,
		int nUpper,
		int nStep = 1,
		BOOL bResamble = false,
		BOOL bDisplay = true);

	int SetPos(
		int nPos,
		BOOL bDisplay = true);

	int OffsetPos(
		int nOffset,
		BOOL bDisplay = true);

	int SetStep(
		int nStep);

	int StepIt(
		BOOL bDisplay = true);

	BOOL SetSize(
		int nSize,
		BOOL bDisplay = true);

	int GetSize();

	int GetPos(
		BOOL bPercent = false);

	int GetStep();

	int GetLower();

	int GetUpper();

	int SetStart(
		int nStart,
		BOOL bDisplay = true);

	int GetStart(
		BOOL bPercent = false);

    void Reset(
		BOOL bDisplay = true);

	BOOL SetProgress(
		int nProgress,
		BOOL bDisplay = true);

	int GetProgress();

    BOOL OnCalcPos();

protected:
	int mnSize;
    int mnPane;

    int mnType;

    int mnLower;
    int mnUpper;
    int mnStep;

    int mnPrevLeftAff;
    int mnLeftAff;
    int mnPrevLeft;
    int mnLeft;

    int mnPrevRightAff;
    int mnRightAff;
    int m_nPrevRight;
    int mnRight;

    int mnPrevEndAff;
    int mnEndAff;

    BOOL mbReverse;

    int mnBmpWidth;
    int mnBmpHeight;

    CRect mPaneRect;
    CRect mPaintRect;

    CBitmap *mpBaseBmp;
    CBitmap *mpCompatibleBmp;
    CBitmap *mpStretchBmp;
    CImageList mBarImageList;
	CBitmap *mpProgressBmp;

    SYSTEMTIME msStartTimeSystem;
    LONGLONG mnStartTimeLongLong;

	void Clear();

	BOOL Init(
		CBitmap *psBitmap = NULL,
		int nUpper = 100,
		int nProgress = SKIN_PROGRESS_TEXT_PERCENT,
		BOOL bbReverse = false);

	int CompactText(
		CString *io_pString,
		CDC *i_pDC, 
		int i_nMaxWidth, 
		int i_nAddWidth = 0);

	BOOL GetTimed(
		CString *o_poTimed);

    BOOL CalcPos(
		CRect *o_poRectPaint = NULL);

	afx_msg BOOL OnEraseBkgnd(CDC *dc);

	afx_msg void OnPaint();

	afx_msg void OnTimer(UINT_PTR nIDEvent);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

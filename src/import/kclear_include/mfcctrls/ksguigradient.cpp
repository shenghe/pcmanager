//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ksguigradient.h"


//////////////////////////////////////////////////////////////////////////

CKSGuiGradient::CKSGuiGradient(CSize Size)
{
	mSize = Size;
	mpMemDC = NULL;
}

//////////////////////////////////////////////////////////////////////////

CKSGuiGradient::~CKSGuiGradient()
{
	if (mpMemDC != NULL)
		delete mpMemDC;
}

//////////////////////////////////////////////////////////////////////////

void CKSGuiGradient::PrepareVertical(
	CDC *pDC,
	COLORREF TopColor, 
	COLORREF BottomColor)
{
	UINT RTop, GTop, BTop, RBot, GBot, BBot;

	RTop = GetRValue(TopColor);
	GTop = GetGValue(TopColor);
	BTop = GetBValue(TopColor);

	RBot = GetRValue(BottomColor);
	GBot = GetGValue(BottomColor);
	BBot = GetBValue(BottomColor);

	if (mpMemDC != NULL)
		delete mpMemDC;

	CBitmap Bitmap;
	mpMemDC = new CDC;
	mpMemDC->CreateCompatibleDC(pDC);
	mpMemDC->SetMapMode(MM_TEXT);

	Bitmap.CreateCompatibleBitmap(pDC,mSize.cx, mSize.cy);	

	mpMemDC->SelectObject(Bitmap);

	double rStep, gStep, bStep;
	double rCount, gCount, bCount;
	double RectHeight = mSize.cy / 256.0;
	const int NUM_COLORS = 256;

	rCount = RTop;
	gCount = GTop;
	bCount = BTop;

	rStep =- ((double)RTop - RBot) / NUM_COLORS;
	gStep =- ((double)GTop - GBot) / NUM_COLORS;
	bStep =- ((double)BTop - BBot) / NUM_COLORS;

	CBrush FillBrush;
	
	for (int ColorCount = 0; ColorCount < NUM_COLORS; ColorCount ++)
	{
		FillBrush.CreateSolidBrush(RGB(rCount, gCount, bCount));

		mpMemDC->FillRect(CRect(0, (int)(ColorCount * RectHeight), mSize.cx, (int)((ColorCount + 1) * RectHeight)), &FillBrush);

		rCount += rStep;
		gCount += gStep;
		bCount += bStep;

		FillBrush.DeleteObject();
	}
}

//////////////////////////////////////////////////////////////////////////

void CKSGuiGradient::Draw(
	CDC *pDC,
	int xDest,
	int yDest,
	int xSrc, 
	int ySrc, 
	int Width, 
	int Height,
	DWORD Rop)
{
	pDC->BitBlt(xDest, yDest, Width, Height, mpMemDC, xSrc, ySrc, Rop);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

class CKSGuiGradient  
{
public:
	CKSGuiGradient(CSize = CSize(800, 600));

	virtual ~CKSGuiGradient();

	void PrepareVertical(
		CDC *pDC,
		COLORREF TopColor, 
		COLORREF BottomColor);

	void Draw(
		CDC *pDC,
		int xDest,
		int yDest,
		int xSrc, 
		int ySrc, 
		int Width, 
		int Height,
		DWORD Rop);

protected:
	CDC *mpMemDC;
	CSize mSize;
};

//////////////////////////////////////////////////////////////////////////
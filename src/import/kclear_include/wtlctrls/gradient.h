//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

class CGradient  
{
public:
	CGradient(CSize = CSize(800,600));

	virtual ~CGradient();

	void PrepareVertical(
		CDCHandle pDC,
		COLORREF TopColor, 
		COLORREF BottomColor);

	void Draw(
		CDCHandle pDC,
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
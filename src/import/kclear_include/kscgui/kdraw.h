#ifndef KDRAW_INC_
#define KDRAW_INC_

//////////////////////////////////////////////////////////////////////////

inline void GetBitmapSize(HBITMAP hBitmap, int& nWidth, int& nHeight)
{
	BITMAP bm;
	::GetObject(hBitmap, sizeof(BITMAP), (PSTR)&bm); 
	nWidth = bm.bmWidth;
	nHeight = bm.bmHeight;
}

inline void DrawGradualColorRect(
	HDC dc,
	const RECT& drawRC, 
	COLORREF clrFirst = RGB(255, 255, 255),
	COLORREF clrEnd = RGB(0, 0, 0),
	BOOL fVertical = TRUE
	) 
{
	if (fVertical)
	{
		GRADIENT_RECT gRect = {0, 1};

		TRIVERTEX vert[2] = {
			{drawRC.left , drawRC.top   , 0, 0, 0, 0}, 
			{drawRC.right, drawRC.bottom, 0, 0, 0, 0} };

			vert[0].Red     = GetRValue(clrFirst) << 8;
			vert[0].Green   = GetGValue(clrFirst) << 8;
			vert[0].Blue    = GetBValue(clrFirst) << 8;
			vert[1].Red     = GetRValue(clrEnd) << 8;
			vert[1].Green   = GetGValue(clrEnd) << 8;
			vert[1].Blue    = GetBValue(clrEnd) << 8;

			::GradientFill(dc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
	}
	else
	{
		GRADIENT_RECT gRect = {0, 1};

		TRIVERTEX vert[2] = {
			{drawRC.left , drawRC.top   , 0, 0, 0, 0}, 
			{drawRC.right, drawRC.bottom, 0, 0, 0, 0} };

			vert[0].Red     = GetRValue(clrFirst) << 8;
			vert[0].Green   = GetGValue(clrFirst) << 8;
			vert[0].Blue    = GetBValue(clrFirst) << 8;
			vert[1].Red     = GetRValue(clrEnd) << 8;
			vert[1].Green   = GetGValue(clrEnd) << 8;
			vert[1].Blue    = GetBValue(clrEnd) << 8;

			::GradientFill(dc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
	}
}

//////////////////////////////////////////////////////////////////////////

#endif	// KDRAW_INC_

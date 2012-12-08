///////////////////////////////////////////////////////////////////////////////
//
// Draw.cpp : implementation file
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tools.h"
#include "Draw.h"

///////////////////////////////////////////////////////////////////////////////
HLSCOLOR RGB2HLS (COLORREF rgb)
{
    unsigned char minval = min(GetRValue(rgb), min(GetGValue(rgb), GetBValue(rgb)));
    unsigned char maxval = max(GetRValue(rgb), max(GetGValue(rgb), GetBValue(rgb)));
    float mdiff  = float(maxval) - float(minval);
    float msum   = float(maxval) + float(minval);
   
    float luminance = msum / 510.0f;
    float saturation = 0.0f;
    float hue = 0.0f; 

    if ( maxval != minval )
    { 
        float rnorm = (maxval - GetRValue(rgb)  ) / mdiff;      
        float gnorm = (maxval - GetGValue(rgb)) / mdiff;
        float bnorm = (maxval - GetBValue(rgb) ) / mdiff;   

        saturation = (luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

        if (GetRValue(rgb) == maxval) hue = 60.0f * (6.0f + bnorm - gnorm);
        if (GetGValue(rgb) == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
        if (GetBValue(rgb) == maxval) hue = 60.0f * (4.0f + gnorm - rnorm);
        if (hue > 360.0f) hue = hue - 360.0f;
    }
    return HLS ((hue*255)/360, luminance*255, saturation*255);
}

///////////////////////////////////////////////////////////////////////////////
static BYTE _ToRGB (float rm1, float rm2, float rh)
{
  if      (rh > 360.0f) rh -= 360.0f;
  else if (rh <   0.0f) rh += 360.0f;
 
  if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
  else if (rh < 180.0f) rm1 = rm2;
  else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      
                   
  return (BYTE)(rm1 * 255);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF HLS2RGB (HLSCOLOR hls)
{
    float hue        = ((int)HLS_H(hls)*360)/255.0f;
    float luminance  = HLS_L(hls)/255.0f;
    float saturation = HLS_S(hls)/255.0f;

    if ( saturation == 0.0f )
    {
        return RGB (HLS_L(hls), HLS_L(hls), HLS_L(hls));
    }
    float rm1, rm2;
     
    if ( luminance <= 0.5f ) rm2 = luminance + luminance * saturation;  
    else                     rm2 = luminance + saturation - luminance * saturation;
    rm1 = 2.0f * luminance - rm2;   
    BYTE red   = _ToRGB (rm1, rm2, hue + 120.0f);   
    BYTE green = _ToRGB (rm1, rm2, hue);
    BYTE blue  = _ToRGB (rm1, rm2, hue - 120.0f);

    return RGB (red, green, blue);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S)
{
    HLSCOLOR hls = RGB2HLS (rgb);
    BYTE h = HLS_H(hls);
    BYTE l = HLS_L(hls);
    BYTE s = HLS_S(hls);

    if ( percent_L > 0 )
    {
        l = BYTE(l + ((255 - l) * percent_L) / 100);
    }
    else if ( percent_L < 0 )
    {
        l = BYTE((l * (100+percent_L)) / 100);
    }
    if ( percent_S > 0 )
    {
        s = BYTE(s + ((255 - s) * percent_S) / 100);
    }
    else if ( percent_S < 0 )
    {
        s = BYTE((s * (100+percent_S)) / 100);
    }
    return HLS2RGB (HLS(h, l, s));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CBufferDC::CBufferDC (HDC hDestDC, const CRect& rcPaint) : m_hDestDC (hDestDC)
{
    if ( rcPaint.IsRectEmpty() )
    {
        ::GetClipBox (m_hDestDC, m_rect);
    }
    else
    {
        m_rect = rcPaint;
    }
    Attach (::CreateCompatibleDC (m_hDestDC));
    m_bitmap.Attach (::CreateCompatibleBitmap (m_hDestDC, m_rect.right, m_rect.bottom));
    m_hOldBitmap = ::SelectObject (m_hDC, m_bitmap);

    if ( m_rect.top > 0 )
    {
        ExcludeClipRect (0, 0, m_rect.right, m_rect.top);
    }
    if ( m_rect.left > 0 )
    {
        ExcludeClipRect (0, m_rect.top, m_rect.left, m_rect.bottom);
    }
}

///////////////////////////////////////////////////////////////////////////////
CBufferDC::~CBufferDC ()
{
    ::BitBlt (m_hDestDC, m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), m_hDC, m_rect.left, m_rect.top, SRCCOPY);
    ::SelectObject (m_hDC, m_hOldBitmap);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CPenDC::CPenDC (HDC hDC, COLORREF crColor) : m_hDC (hDC)
{
    m_pen.CreatePen (PS_SOLID, 1, crColor);
    m_hOldPen = (HPEN)::SelectObject (m_hDC, m_pen);
}

///////////////////////////////////////////////////////////////////////////////
CPenDC::~CPenDC ()
{
    ::SelectObject (m_hDC, m_hOldPen);
}

///////////////////////////////////////////////////////////////////////////////
void CPenDC::Color (COLORREF crColor)
{
    ::SelectObject (m_hDC, m_hOldPen);
    m_pen.DeleteObject();
    m_pen.CreatePen (PS_SOLID, 1, crColor);
    m_hOldPen = (HPEN)::SelectObject (m_hDC, m_pen);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF CPenDC::Color () const
{
    LOGPEN logPen;

    ((CPenDC*)this)->m_pen.GetLogPen (&logPen);

    return logPen.lopnColor;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CBrushDC::CBrushDC (HDC hDC, COLORREF crColor) : m_hDC (hDC)
{
    if ( crColor == CLR_NONE ) m_brush.Attach ((HBRUSH)::GetStockObject (NULL_BRUSH));
    else                       m_brush.CreateSolidBrush (crColor);
    m_hOldBrush = (HBRUSH)::SelectObject (m_hDC, m_brush);
}

///////////////////////////////////////////////////////////////////////////////
CBrushDC::~CBrushDC ()
{
    ::SelectObject (m_hDC, m_hOldBrush);
}

///////////////////////////////////////////////////////////////////////////////
void CBrushDC::Color (COLORREF crColor)
{
    ::SelectObject (m_hDC, m_hOldBrush);
    m_brush.DeleteObject();
    if ( crColor == CLR_NONE ) m_brush.Attach ((HBRUSH)::GetStockObject (NULL_BRUSH));
    else                       m_brush.CreateSolidBrush (crColor);
    m_hOldBrush = (HBRUSH)::SelectObject (m_hDC, m_brush);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF CBrushDC::Color () const
{
    LOGBRUSH logBrush;

    ((CBrushDC*)this)->m_brush.GetLogBrush (&logBrush);

    return logBrush.lbColor;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CBoldDC::CBoldDC (HDC hDC, bool bBold) : m_hDC (hDC), m_hDefFont (NULL)
{
    LOGFONT lf;
    
    CFontHandle ((HFONT)::GetCurrentObject (m_hDC, OBJ_FONT)).GetLogFont (&lf);

    if ( ( bBold && lf.lfWeight != FW_BOLD) ||
         (!bBold && lf.lfWeight == FW_BOLD) )
    {
        lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;

        m_fontBold.CreateFontIndirect (&lf);
        m_hDefFont = (HFONT)::SelectObject (m_hDC, m_fontBold);
    }
}

/////////////////////////////////////////////////////////////////////////////
CBoldDC::~CBoldDC ()
{
    if ( m_hDefFont != NULL )
    {
        ::SelectObject (m_hDC, m_hDefFont);
    }
}

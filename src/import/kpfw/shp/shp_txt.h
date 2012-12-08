////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shp_txt.h
//      Comment   : simple html parser bmp obj class
//					
//      Create at : 2009-04-24
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define SHP_TXT_ITEM_KEY					TEXT("txt")

#define SHP_TXT_ITEM_FIRST_PADDING_CNT		TEXT("fp_cnt")
#define SHP_TXT_ITEM_FONT_KEY				TEXT("font_face")
#define SHP_TXT_ITEM_FONT_SIZE_KEY			TEXT("font_size")
#define SHP_TXT_ITEM_FONT_WEIGHT_KEY		TEXT("font_weight")
#define SHP_TXT_ITEM_SPE_TEXT_KEY			TEXT("spe_text")
#define SHP_TXT_ITEM_SPE_CLR_KEY			TEXT("spe_clr")	
#define SHP_TXT_ITEM_SPE_CNT_KEY			TEXT("spe_cnt")
#define SHP_TXT_ITEM_LINESPACE_KEY			TEXT("linespace")

#include "shpsupport.h"
#include <atlgdi.h>

class shp_txt_item : public shp_dom_item
{
public:

	shp_txt_item()
	: m_i_fp_cnt(0),
	m_i_font_weight(400),
	m_i_font_size(12),
	m_i_linespace(2),
	m_i_spe_cnt(1),
	m_clr_spen_clr(RGB(0xff, 0x00, 0x00))
	{};

	~shp_txt_item()
	{};

	//set_attribute( p, m )
	BEGIN_KEY_MAP()
		//to add new attribute map( key map to member )
		ADD_INT_KEY( SHP_TXT_ITEM_FIRST_PADDING_CNT,	m_i_fp_cnt )
		ADD_INT_KEY( SHP_TXT_ITEM_FONT_SIZE_KEY,		m_i_font_size )
		ADD_INT_KEY( SHP_TXT_ITEM_FONT_WEIGHT_KEY,		m_i_font_weight )
		ADD_INT_KEY( SHP_TXT_ITEM_LINESPACE_KEY,		m_i_linespace )
		ADD_INT_KEY( SHP_TXT_ITEM_SPE_CNT_KEY,			m_i_spe_cnt )

		ADD_CLR_KEY( SHP_TXT_ITEM_SPE_CLR_KEY,			m_clr_spen_clr )
		ADD_STR_KEY( SHP_TXT_ITEM_FONT_KEY,				m_str_font_face )
		ADD_STR_KEY( SHP_TXT_ITEM_SPE_TEXT_KEY,			m_str_spe_text )
		
		BASE_CLASS_PROCESS(shp_item_win32)
	END_KEY_MAP()

	void draw_item( HDC hdc, CRect* parent_rect, CRect *left_rect )
	{
		if ( !m_b_init )
			_init_font();

		HFONT hOldFont = NULL;
		if ( !m_h_font.IsNull())
			hOldFont = (HFONT)::SelectObject( hdc, m_h_font );

		if ( !m_b_init)
		{
			_init_text();
			_init_rect( hdc, parent_rect, left_rect );
			m_b_init = true;
		}

		if ( get_visible())
		{
			_draw_txt( hdc );
		}

		if ( hOldFont )
			::SelectObject( hdc, hOldFont );
	}

	CRect* get_draw_rect()
	{
		return &m_draw_rect;
	}

	LPCTSTR hit_test( HDC hdc, UINT umsg, int x, int y )
	{
		return NO_PROCESS;
	}

	void set_spe_word( shpstr& str_word )
	{
		m_str_spe_text = str_word;
	}

	void set_spe_clr( shpclr clr_spen )
	{
		m_clr_spen_clr = clr_spen;
	}


protected:


	void _init_font()
	{
		
		if ( m_str_font_face.GetLength() > 0 )
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));
			lf.lfHeight = m_i_font_size ;
			lf.lfWeight = m_i_font_weight;
			_tcscpy( lf.lfFaceName, m_str_font_face );

			if (!m_h_font.IsNull())
			{
				m_h_font.DeleteObject();
			}

			m_h_font.CreateFontIndirect( &lf );
		}
	}

	void _init_text()
	{
		if ( m_i_fp_cnt > 0 )
		{
			TCHAR* pszPadding = new TCHAR[m_i_fp_cnt * 2 + 1];
			wmemset(pszPadding, 32, m_i_fp_cnt * 2 );
			*(pszPadding + m_i_fp_cnt * 2) = TEXT('\0');

			m_str_draw_text = pszPadding;
		}

		m_str_draw_text += m_str_text;
	}

	//这里不需要left_rect, 忽略
	void _init_rect( HDC hdc, CRect *p_rect, CRect *left_rect )
	{
		ASSERT( m_i_width != INT_NO_SET );
		ASSERT( p_rect );
		if ( !p_rect )
			return;

		int i_height = p_rect->Height();
		m_draw_rect.top = p_rect->top + ( m_i_top == INT_NO_SET ? 0 : m_i_top );
		m_draw_rect.left = p_rect->left + ( m_i_left == INT_NO_SET ? 0 : m_i_left);
		m_draw_rect.right = m_draw_rect.left + m_i_width;
		m_draw_rect.bottom = m_draw_rect.top + i_height;

		draw_text_in_rect( hdc, m_str_draw_text, m_str_draw_text.GetLength(), &m_draw_rect, m_i_linespace , TRUE );
	}

	void _draw_txt( HDC hdc )
	{
		shpclr clr_old;
		int i_mode = ::SetBkMode( hdc, TRANSPARENT );

		clr_old = ::SetTextColor( hdc, m_clr_text );
		draw_text_in_rect( hdc, m_str_draw_text, m_str_draw_text.GetLength(), &m_draw_rect, m_i_linespace );
		::SetTextColor( hdc, clr_old );
		
		bool b_code = false;
		POINT point = { 0, 0 };

		clr_old = ::SetTextColor( hdc, m_clr_spen_clr );

		for ( int i = 0; i < m_i_spe_cnt; i++ )
		{
			b_code = get_text_pos(hdc, &point);

			if ( !b_code )
				break;
			
			::TextOut( hdc, point.x, point.y, m_str_spe_text, m_str_spe_text.GetLength() );
		}

		::SetBkMode( hdc, i_mode );
		::SetTextColor( hdc, clr_old );
	}

	bool get_text_pos( HDC hdc, LPPOINT p_point )
	{
		int i_text_pos = m_str_draw_text.Find( m_str_spe_text );

		if( i_text_pos == -1 )
			return false;

		clac_text_point( hdc, p_point, m_str_draw_text, &m_draw_rect, i_text_pos, m_i_linespace );
		return true;
	}

	void clac_text_point( HDC hdc, LPPOINT p_point, LPCTSTR pszText, LPRECT p_rect, int i_pos, int i_linespace )
	{
		LPTSTR pText = (LPTSTR)pszText;
		TEXTMETRIC tm;
		int nWidth = 0;
		int nRectWidth = p_rect->right - p_rect->left;
		int nTop = p_rect->top;
		int nWordCnt = 0;

		::GetTextMetrics( hdc, &tm );

		for( int i = 0; i < i_pos; i ++ )
		{
			if ( *(pszText + i) >=0 && *(pszText + i) <= 255 )
				nWidth += tm.tmAveCharWidth;
			else
				nWidth += tm.tmAveCharWidth * 2;

			if ( nWidth > nRectWidth )
			{
				pText +=  nWordCnt ;

				nWidth = 0;
				i = i -1;
				nTop += tm.tmHeight + tm.tmExternalLeading + i_linespace;
				nWordCnt = 0;
			}
			else
			{
				nWordCnt++;
			}
		}

		p_point->x = nWidth + p_rect->left;
		p_point->y = nTop ;

		return;
	}


private:
	
	int				m_i_fp_cnt;
	int				m_i_font_size;
	int				m_i_font_weight;
	int				m_i_linespace;
	int				m_i_spe_cnt;

	shpclr			m_clr_spen_clr;
	shpstr			m_str_font_face;
	shpstr			m_str_spe_text;

	shpstr			m_str_draw_text;
	CFont			m_h_font;


};
////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shp_link.h
//      Comment   : simple html parser link obj class
//					
//      Create at : 2008-09-12
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shpsupport.h"
#include "shp_div.h"

#define SHP_LINK_ITEM_KEY				TEXT("link")

#define SHP_LINK_ITEM_OVERCOLOR_KEY		TEXT("over_color")
#define SHP_LINK_ITEM_NORMALCOLOR_KEY	TEXT("normal_color")
#define SHP_LINK_ITEM_HITCOLOR_KEY		TEXT("hit_color")
#define SHP_LINK_ITEM_UNDERLINE_KEY		TEXT("underline")
#define SHP_LINK_ITEM_ITALIC_KEY		TEXT("italic")
#define SHP_LINK_ITEM_CUROSOR_KEY		TEXT("cursor")
#define SHP_LINK_ITEM_ENABLE_KEY		TEXT("enable")
#define SHP_LINK_ITEM_DISABELCOLOR_KEY	TEXT("disable_color")

#define SHP_LINK_STATE_OVER				1
#define SHP_LINK_STATE_NORMAL			2
#define SHP_LINK_STATE_HIT				3
#define SHP_LINK_STATE_DISABLE			4

class shp_link_item : public shp_div_item
{
public:

	shp_link_item()
	:m_clr_over(RGB(0xFF,0x33,0x66)),
	m_clr_normal(RGB(0x00,0x33,0xFF)),
	m_clr_hit(RGB(0x00,0x33,0xFF)),
	m_clr_disable(RGB(0x99,0x99,0x99)),
	m_b_over(false),
	m_b_hit(false),
	m_i_underline(1),
	m_i_italic(0),
	m_h_cursor(NULL),
	m_i_enable(1),
	m_i_pre_state(INT_NO_SET)
	{
		m_clr_bg = RGB(255,255,255);
	};

	~shp_link_item()
	{};

	//set_attribute( p, m )
	BEGIN_KEY_MAP()
		//to add new attribute map( key map to member )
		ADD_CLR_KEY( SHP_LINK_ITEM_OVERCOLOR_KEY, m_clr_over )
		ADD_CLR_KEY( SHP_LINK_ITEM_HITCOLOR_KEY, m_clr_hit )
		ADD_CLR_KEY( SHP_LINK_ITEM_NORMALCOLOR_KEY, m_clr_normal )
		ADD_CLR_KEY( SHP_LINK_ITEM_DISABELCOLOR_KEY, m_clr_disable )
		ADD_INT_KEY( SHP_LINK_ITEM_UNDERLINE_KEY, m_i_underline )
		ADD_INT_KEY( SHP_LINK_ITEM_ITALIC_KEY, m_i_italic )
		ADD_INT_KEY( SHP_LINK_ITEM_ENABLE_KEY, m_i_enable )
		//该属性目前无效
		ADD_STR_KEY( SHP_LINK_ITEM_CUROSOR_KEY,m_str_cursor)
		BASE_CLASS_PROCESS(shp_div_item)
	END_KEY_MAP()

	void draw_item( HDC hdc, CRect* parent_rect, CRect *left_rect )
	{
		if ( !m_b_init)
		{
			_init_cursor();
			_init_fmt();
			_init_font();
			_init_rect( hdc, parent_rect, left_rect );
			m_b_init = true;
		}
		
		if ( get_visible())
		{
			if( m_i_enable == 1 )
				_redraw_item( hdc, m_b_over ? SHP_LINK_STATE_OVER : (m_b_hit ? SHP_LINK_STATE_HIT : SHP_LINK_STATE_NORMAL) );
			else
				_redraw_item( hdc, SHP_LINK_STATE_DISABLE );
		}
	}

	void set_enable( int i_enable )
	{
		m_i_enable = i_enable;
		m_b_hit = false;
		m_b_over = false;
	}

	void _init_cursor()
	{
		if ( m_h_cursor == NULL )
			m_h_cursor = ::LoadCursor( NULL, IDC_HAND );
	}

	void _init_rect( HDC hdc, CRect *p_rect, CRect *left_rect )
	{
		HFONT h_old_font = (HFONT) SelectObject( hdc, m_normal_font );

		draw_rect_zero();

		if ( !left_rect )
			m_draw_rect.right = ( m_i_width == INT_NO_SET ? p_rect->right - p_rect->left : m_i_width );
		else
			m_draw_rect.right = ( m_i_width == INT_NO_SET ? p_rect->right - left_rect->right - p_rect->left : m_i_width );

		if ( get_line_space() != INT_NO_SET && m_i_multiline == 1 )
		{
			draw_text_in_rect( hdc, 
				m_str_text, 
				m_str_text.GetLength(), 
				&(m_draw_rect), 
				get_line_space() , true );
		}
		else
		{
			::DrawText( hdc, 
				m_str_text,
				m_str_text.GetLength(),
				&(m_draw_rect),
				DT_CALCRECT | m_u_fmt );
		}

		if ( h_old_font)
			::SelectObject( hdc, h_old_font );

		//这里不做太复杂自动处理，需要更完善脚本来保证
		int n_width = m_draw_rect.Width();
		if ( !left_rect )
		{
			m_draw_rect.left = p_rect->left + ( m_i_left == INT_NO_SET ? 0 : m_i_left );
			m_draw_rect.top = p_rect->top + ( m_i_top == INT_NO_SET ? 0 : m_i_top );
			m_draw_rect.bottom =  (m_i_height == INT_NO_SET ? m_draw_rect.bottom : m_i_height)  + m_draw_rect.top ;
			m_draw_rect.right = ( m_i_width == INT_NO_SET ? n_width : m_i_width ) + m_draw_rect.left;
		}
		else
		{
			if ( get_i_br() == 1 )
			{
				m_draw_rect.left = p_rect->left + ( m_i_left == INT_NO_SET ? 0 : m_i_left );
				m_draw_rect.top = left_rect->bottom + ( m_i_top == INT_NO_SET ? 0 : m_i_top );
				m_draw_rect.bottom = (m_i_height == INT_NO_SET ? m_draw_rect.bottom : m_i_height)  + m_draw_rect.top ;
				m_draw_rect.right = (m_i_width == INT_NO_SET ? n_width : m_i_width )+m_draw_rect.left;
			}
			else
			{
				m_draw_rect.left = left_rect->right +( m_i_left == INT_NO_SET ? 0 : m_i_left );
				m_draw_rect.top = left_rect->top;
				m_draw_rect.bottom = (m_i_height == INT_NO_SET ? m_draw_rect.bottom : m_i_height)  + m_draw_rect.top ;
				//如果在右边，有没有指定宽度，右边的宽带为  p_rect->right - left_rect->right
				m_draw_rect.right = ( m_i_width == INT_NO_SET ? n_width : m_i_width ) + m_draw_rect.left;
			}

		}
	}

	LPCTSTR hit_test(  HDC hdc, UINT umsg, int x, int y  )
	{
		LPCTSTR	lp_str = NULL;

		if ( !get_visible() || m_i_enable == 0 )
		{
			return NULL;
		}

		switch ( umsg )
		{
		case WM_MOUSEMOVE:
			lp_str = _mouse_move( hdc, x, y );
			break;
		case WM_MOUSELEAVE:
			_redraw_item( hdc, m_b_hit ? SHP_LINK_STATE_HIT : SHP_LINK_STATE_NORMAL );
			m_b_over = false;
			break;
		case WM_LBUTTONUP:
			if ( m_draw_rect.PtInRect(CPoint(x,y)) )
			{
				lp_str = _mouse_click( hdc );
			}
			break;
		case WM_LBUTTONDOWN:
			if( m_draw_rect.PtInRect(CPoint(x,y)) )
				if ( m_h_cursor )
					::SetCursor( m_h_cursor );
			break;
		default:
			break;
		}

		return lp_str;
	}

protected:

	void _init_font()
	{
		if ( m_str_fontface.GetLength() > 0 || m_i_fontsize != INT_NO_SET )
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));
			lf.lfHeight = m_i_fontsize;
			if( m_i_underline )
				lf.lfUnderline = (BYTE)TRUE;
			else
				lf.lfUnderline = (BYTE)FALSE;

			_tcscpy(lf.lfFaceName, m_str_fontface );

			if ( !m_hit_font.IsNull())
				m_hit_font.DeleteObject();

			if ( !m_normal_font.IsNull())
				m_normal_font.DeleteObject();

			if ( !m_over_font.IsNull())
				m_over_font.DeleteObject();

			m_hit_font.CreateFontIndirect(&lf);
			m_normal_font.CreateFontIndirect(&lf);
			
			if ( m_i_italic == 1 )
			{
				lf.lfItalic = BYTE(TRUE);
			}
			
			m_over_font.CreateFontIndirect( &lf );
		}
		else
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));
			lf.lfHeight = 12;
			if( m_i_underline )
				lf.lfUnderline = (BYTE)TRUE;
			else
				lf.lfUnderline = (BYTE)FALSE;
			_tcscpy(lf.lfFaceName, TEXT("宋体") );

			if ( m_hit_font.IsNull())
				m_hit_font.CreateFontIndirect(&lf);
			if ( m_normal_font.IsNull())
				m_normal_font.CreateFontIndirect(&lf);

			if ( m_i_italic == 1 )
				lf.lfItalic = BYTE(TRUE);

			if ( m_over_font.IsNull())
				m_over_font.CreateFontIndirect( &lf );
		}

	}

	LPCTSTR _mouse_move( HDC hdc, int x, int y )
	{
		LPCTSTR lp_str_id = NULL;
		if ( m_draw_rect.PtInRect( CPoint(x,y)) )
		{
			if ( m_b_over == false )
			{
				if ( m_i_pre_state != SHP_LINK_STATE_OVER )
				{
					_redraw_item( hdc,SHP_LINK_STATE_OVER );
				}
				m_b_over = true;
			}
			
			// set cursor
			if ( m_h_cursor )
				::SetCursor( m_h_cursor );

			lp_str_id = m_str_id;
		}
		else
		{
			if ( m_b_over == true )
			{

				if ( m_i_pre_state != (m_b_hit ? SHP_LINK_STATE_HIT : SHP_LINK_STATE_NORMAL))
				{
					_redraw_item( hdc, m_b_hit ? SHP_LINK_STATE_HIT : SHP_LINK_STATE_NORMAL );
				}
				
				m_b_over = false;
			}
		}

		return lp_str_id;
	}

	LPCTSTR _mouse_click( HDC hdc )
	{
		_redraw_item( hdc, m_b_over ? SHP_LINK_STATE_OVER : SHP_LINK_STATE_HIT );
		m_b_hit = true;

		// set cursor
		if ( m_h_cursor )
			::SetCursor( m_h_cursor );
		
		return LPCTSTR( m_str_id );
	}

	void _redraw_item( HDC hdc, int nstate )
	{
		_fill_bg( hdc );
		_draw_link_text( hdc, nstate );
		m_i_pre_state = nstate;
	}

	void _draw_link_text( HDC hdc, int nstate )
	{
		shpclr clr_text;
		CFontHandle font_handle;
		::SetBkMode( hdc, TRANSPARENT );

		switch( nstate )
		{
		case SHP_LINK_STATE_OVER:
			clr_text = m_clr_over;
			font_handle = m_over_font;
			break;
		case SHP_LINK_STATE_NORMAL:
			clr_text = m_clr_normal;
			font_handle = m_normal_font;
			break;
		case SHP_LINK_STATE_HIT:
			clr_text = m_clr_hit;
			font_handle = m_hit_font;
			break;
		case SHP_LINK_STATE_DISABLE:
			clr_text = m_clr_disable;
			font_handle = m_normal_font;
			break;
		default:
			clr_text = m_clr_hit;
			font_handle = m_hit_font;
		}

		HFONT h_old_font = (HFONT) ::SelectObject( hdc, font_handle );

		::SetTextColor( hdc, clr_text );

		::DrawText( hdc, 
			m_str_text,
			m_str_text.GetLength(),
			&(m_draw_rect),
			m_u_fmt );

		if ( h_old_font )
		{
			::SelectObject( hdc, h_old_font );
		}

		return;
	}

	shpstr	m_str_cursor;
	shpclr	m_clr_over;
	shpclr	m_clr_hit;
	shpclr	m_clr_normal;
	shpclr	m_clr_disable;

	int		m_i_underline;
	int		m_i_italic;
	int		m_i_enable;
	

private:
	
	CFont	m_normal_font;
	CFont	m_hit_font;
	CFont	m_over_font;

	bool	m_b_over;
	bool	m_b_hit;
	HCURSOR m_h_cursor;

	int		m_i_pre_state;
	

};


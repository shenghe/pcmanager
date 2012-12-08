////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shp_div.h
//      Comment   : simple html parser link obj class
//					
//      Create at : 2008-09-04
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shpsupport.h"

#define SHP_DIV_ITEM_KEY				TEXT("div")

#define SHP_DIV_ITEM_ALIGN_KEY			TEXT("align")
#define SHP_DIV_ITEM_VALIGN_KEY			TEXT("valign")
#define SHP_DIV_ITEM_MULTILINE_KEY		TEXT("multiline")
#define SHP_DIV_ITEM_ELLIP_KEY			TEXT("ellip")
#define SHP_DIV_ITEM_BR_KEY				TEXT("br")
#define SHP_DIV_ITEM_FONT_FACE_KEY		TEXT("font_face")
#define SHP_DIV_ITEM_FONT_SIZE_KEY		TEXT("font_size")
#define SHP_DIV_ITEM_WEIGHT_KEY			TEXT("weight")
#define SHP_DIV_ITEM_NO_BG_KEY			TEXT("no_bgclr")
#define SHP_DIV_ITEM_LINE_SPACE			TEXT("linespace")

class shp_div_item : public shp_dom_item
{
public:

	shp_div_item()
	:m_i_align(0),
	m_i_valign(0),
	m_u_fmt(0),
	m_i_multiline(INT_NO_SET),
	m_i_ellip(INT_NO_SET),
	m_i_br(INT_NO_SET),
	m_h_font(NULL),
	m_b_own_font(false),
	m_i_fontsize(INT_NO_SET),
	m_i_weight(INT_NO_SET),
	m_i_nobgclr(INT_NO_SET),
	m_i_linespace(INT_NO_SET)
	{
	};

	~shp_div_item()
	{
		if ( m_b_own_font )
		{
			::DeleteObject( m_h_font );
			m_h_font = NULL;
		}
	};

	//set_attribute( p, m )
	BEGIN_KEY_MAP()
		//to add new attribute map( key map to member )
		ADD_INT_KEY( SHP_DIV_ITEM_ALIGN_KEY, m_i_align)
		ADD_INT_KEY( SHP_DIV_ITEM_VALIGN_KEY, m_i_valign)
		ADD_INT_KEY( SHP_DIV_ITEM_BR_KEY, m_i_br )
		ADD_INT_KEY( SHP_DIV_ITEM_MULTILINE_KEY, m_i_multiline)
		ADD_INT_KEY( SHP_DIV_ITEM_ELLIP_KEY, m_i_ellip)
		ADD_STR_KEY( SHP_DIV_ITEM_FONT_FACE_KEY, m_str_fontface)
		ADD_INT_KEY( SHP_DIV_ITEM_FONT_SIZE_KEY, m_i_fontsize)
		ADD_INT_KEY( SHP_DIV_ITEM_WEIGHT_KEY, m_i_weight)
		ADD_INT_KEY( SHP_DIV_ITEM_NO_BG_KEY, m_i_nobgclr)
		ADD_INT_KEY( SHP_DIV_ITEM_LINE_SPACE, m_i_linespace)
		BASE_CLASS_PROCESS(shp_item_win32)
	END_KEY_MAP()

	void draw_item( HDC hdc, CRect* parent_rect, CRect *left_rect )
	{
		if ( !m_b_init)
		{
			_init_fmt();

			if ( m_h_font.IsNull() )
				_init_font();
			
			_init_rect( hdc, parent_rect, left_rect );
			m_b_init = true;
			
		}
		
		if ( get_visible())
		{
			if ( get_clr_bg() != INT_NO_SET  )
				_fill_bg( hdc );

			if ( get_text().GetLength() > 0  )
				_draw_text( hdc );
		}
		
	}

	CRect* get_draw_rect()
	{
		return &m_draw_rect;
	}

	virtual LPCTSTR hit_test(  HDC hdc, UINT umsg, int x, int y )
	{
		if ( !get_visible() || !get_display())
			return NO_PROCESS;

		if ( m_str_text.GetLength() > 0)
		{
			if ( WM_MOUSEMOVE == umsg )
			{
				return _mouse_move( hdc, x, y );
			}
			else
				return NO_PROCESS;
		}
		else
			return NO_PROCESS;
	}

	LPCTSTR _mouse_move( HDC hdc, int x, int y )
	{
		bool b_code = false;
		LPCTSTR lp_str = NULL;
		if ( m_draw_rect.PtInRect( CPoint(x,y)) )
		{
			if ( m_str_id.GetLength() > 0 )
			{
				lp_str = m_str_id;
			}
			
		}
		
		return lp_str;
	}

	void set_font( HFONT h_font )
	{
		m_h_font= h_font;
	}

	HFONT get_font( )
	{
		return m_h_font;
	}

	int get_line_space()
	{
		return m_i_linespace;
	}

protected:

	void _init_font()
	{
		assert( m_h_font.IsNull());
		
		if ( m_str_fontface.GetLength() > 0 || m_i_fontsize != INT_NO_SET || m_i_weight != INT_NO_SET )
		{
		
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));
			lf.lfHeight = ( m_i_fontsize == INT_NO_SET ? 12 : m_i_fontsize );
			lf.lfWeight = m_i_weight;
			_tcscpy(lf.lfFaceName, m_str_fontface );
			
			if ( m_b_own_font && !m_h_font.IsNull())
				m_h_font.DeleteObject();

			m_h_font.CreateFontIndirect(&lf);
			m_b_own_font = true;
		
		}
		else
		{
			shp_dom_item *p_parent = get_parent();
			if ( p_parent )
			{
				if ( _tcsicmp(p_parent->get_str_type(), SHP_DIV_ITEM_KEY ) == 0 )
				{
					shp_div_item * p_div = (shp_div_item*)p_parent;
					set_font( p_div->get_font() );
				}
				else
				{
					

					LOGFONT lf;
					memset(&lf, 0, sizeof(LOGFONT));
					lf.lfHeight = m_i_fontsize;
					lf.lfWeight = m_i_weight;
					lstrcpy(lf.lfFaceName, m_str_fontface );

					if( m_b_own_font && !m_h_font.IsNull())
						m_h_font.DeleteObject();

					m_h_font.CreateFontIndirect(&lf);
					m_b_own_font = true;
				}
			}
		}

		return;
	}
	
	void _fill_bg( HDC hdc )
	{
		//不画背景
		if ( m_i_nobgclr == 1 )
			return;
		
		HBRUSH h_brush = ::CreateSolidBrush( m_clr_bg );
		::FillRect( hdc, &m_draw_rect, h_brush );
		::DeleteObject( (HGDIOBJ)h_brush );
	}

	virtual void _draw_text( HDC hdc)
	{
		::SetBkMode( hdc, TRANSPARENT );
		::SetTextColor( hdc, m_clr_text );

		HFONT h_old_font = (HFONT)::SelectObject( hdc, m_h_font );


		if ( m_i_linespace != INT_NO_SET && m_i_multiline == 1 )
		{
			draw_text_in_rect( hdc, 
				m_str_text, 
				m_str_text.GetLength(), 
				&(m_draw_rect), 
				m_i_linespace );
		}
		else
		{
			::DrawText( hdc, 
				m_str_text,
				m_str_text.GetLength(),
				&(m_draw_rect),
				m_u_fmt );
		}

		if ( h_old_font )
		{
			::SelectObject( hdc, h_old_font );
		}

		return;
	}
	void _init_fmt()
	{
		switch( m_i_align )
		{
		case 1:
			m_u_fmt |= DT_LEFT;
			break;
		case 2:
			m_u_fmt |= DT_CENTER;
			break;
		case 3:
			m_u_fmt |= DT_RIGHT;
			break;
		default:
			m_u_fmt |= DT_LEFT;
			break;
		}

		switch( m_i_valign )
		{
		case 1:
			m_u_fmt |= DT_TOP;
			break;
		case 2:
			m_u_fmt |= DT_VCENTER;
			break;
		case 3:
			m_u_fmt |= DT_BOTTOM;
			break;
		default:
			m_u_fmt |= DT_TOP;
			break;
		}

		if ( m_i_multiline == 1 )
			m_u_fmt |= DT_WORDBREAK;
		else
			m_u_fmt |= DT_SINGLELINE;


		if ( m_i_ellip == 1 )
			m_u_fmt |= DT_END_ELLIPSIS;
		
		
	}

	void _init_rect( HDC hdc, CRect *p_rect, CRect *left_rect )
	{
		HFONT h_old_font = (HFONT) SelectObject( hdc, m_h_font );

		draw_rect_zero();

		if ( !left_rect || get_i_br() == 1 )
			m_draw_rect.right = ( m_i_width == INT_NO_SET ? p_rect->right - p_rect->left : m_i_width );
		else
			m_draw_rect.right = ( m_i_width == INT_NO_SET ? p_rect->right - left_rect->right - p_rect->left : m_i_width );

		if ( m_i_linespace != INT_NO_SET && m_i_multiline == 1 )
		{
			draw_text_in_rect( hdc, 
				m_str_text, 
				m_str_text.GetLength(), 
				&(m_draw_rect), 
				m_i_linespace , true );
		}
		else
		{
			::DrawText( hdc, 
				m_str_text,
				m_str_text.GetLength(),
				&(m_draw_rect),
				DT_CALCRECT | m_u_fmt );
		}
		

		//防止div的text显示不出来的错误。
		//assert( m_str_text.GetLength() >0 && m_draw_rect.right != 0 && m_i_visible);

		if ( h_old_font)
			::SelectObject( hdc, h_old_font );

		//这里不做太复杂自动处理，需要更完善脚本来保证
		if ( !left_rect )
		{
			m_draw_rect.left = p_rect->left + ( m_i_left == INT_NO_SET ? 0 : m_i_left );
			m_draw_rect.top = p_rect->top + ( m_i_top == INT_NO_SET ? 0 : m_i_top );
			m_draw_rect.bottom =  (m_i_height == INT_NO_SET ? m_draw_rect.bottom : m_i_height)  + m_draw_rect.top ;
			//m_draw_rect.right = ( m_i_width == INT_NO_SET ? m_draw_rect.right : m_i_width ) + m_draw_rect.left;
			m_draw_rect.right = ( m_i_width == INT_NO_SET ? p_rect->right - m_draw_rect.left : m_i_width ) + m_draw_rect.left;
		}
		else
		{
			if ( get_i_br() == 1 )
			{
				m_draw_rect.left = p_rect->left + ( m_i_left == INT_NO_SET ? 0 : m_i_left );
				m_draw_rect.top = left_rect->bottom + ( m_i_top == INT_NO_SET ? 0 : m_i_top );
				m_draw_rect.bottom = (m_i_height == INT_NO_SET ? m_draw_rect.bottom : m_i_height)  + m_draw_rect.top ;
				m_draw_rect.right = (m_i_width == INT_NO_SET ? p_rect->right - m_draw_rect.left : m_i_width )+m_draw_rect.left;
			}
			else
			{
				m_draw_rect.left = left_rect->right +( m_i_left == INT_NO_SET ? 0 : m_i_left );
				m_draw_rect.top = left_rect->top;
				m_draw_rect.bottom = (m_i_height == INT_NO_SET ? m_draw_rect.bottom : m_i_height)  + m_draw_rect.top ;
				//如果在右边，有没有指定宽度，右边的宽带为  p_rect->right - left_rect->right
				m_draw_rect.right = ( m_i_width == INT_NO_SET ? p_rect->right - left_rect->right - p_rect->left : m_i_width ) + m_draw_rect.left;
			}
			
		}
	}


	int get_i_br()
	{
		return m_i_br;
	}

	

	// 1 left  2 center  3 right
	int		m_i_align;

	// 1 top   2 vcenter  3 bottom 
	int		m_i_valign;
	int		m_i_multiline;
	int		m_i_ellip;
	int		m_i_br;

	UINT	m_u_fmt;

	shpstr	m_str_fontface;
	int		m_i_fontsize;
	int		m_i_nobgclr;

private:

	

	CFontHandle m_h_font;
	bool	m_b_own_font;
	int		m_i_weight;
	int		m_i_linespace;

};


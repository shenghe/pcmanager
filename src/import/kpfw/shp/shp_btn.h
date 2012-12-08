////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shp_link.h
//      Comment   : simple html parser btn(button) object class
//					
//      Create at : 2009-01-13
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shpsupport.h"

#define SHP_BTN_ITEM_KEY					TEXT("btn")

#define SHP_BTN_ITEM_TEXT_KEY					TEXT("btn_text")
#define SHP_BTN_ITEM_BTN_CLR_KEY				TEXT("btn_clr")
#define SHP_BTN_ITEM_BTN_TYPE_KEY				TEXT("btn_type")   // 0 普通按钮，1 图片按钮  2 可拉伸图片按钮

#define SHP_BTN_ITEM_NORMAL_KEY					TEXT("normal_src")
#define SHP_BTN_ITEM_DISABLE_KEY				TEXT("disable_src")
#define SHP_BTN_ITEM_OVER_KEY					TEXT("over_src")
#define SHP_BTN_ITEM_DOWN_KEY					TEXT("down_src")

#define SHP_BTN_ITEM_NORMAL_COLOR_KEY			TEXT("normal_clr")
#define SHP_BTN_ITEM_DISABLE_COLOR_KEY			TEXT("disable_clr")
#define SHP_BTN_ITEM_OVER_COLOR_KEY				TEXT("over_clr")
#define SHP_BTN_ITEM_DOWN_COLOR_KEY				TEXT("down_clr")

#define SHP_BTN_ITEM_DOWN_LEFT_KEY				TEXT("down_left_src")
#define SHP_BTN_ITEM_NORMAL_LEFT_KEY			TEXT("normal_left_src")
#define SHP_BTN_ITEM_DISABLE_LEFT_KEY			TEXT("disable_left_src")
#define SHP_BTN_ITEM_OVER_LEFT_KEY				TEXT("over_left_src")

#define SHP_BTN_ITEM_NORMAL_RIGHT_KEY			TEXT("normal_right_src")
#define SHP_BTN_ITEM_DISABLE_RIGHT_KEY			TEXT("disable_right_src")
#define SHP_BTN_ITEM_OVER_RIGHT_KEY				TEXT("over_right_src")
#define SHP_BTN_ITEM_DOWN_RIGHT_KEY				TEXT("down_right_src")

#define SHP_BTN_ITEM_CHECK_KEY					TEXT("check")
#define SHP_BTN_ITEM_FONT_KEY					TEXT("font_face")
#define SHP_BTN_ITEM_FONT_SIZE_KEY				TEXT("font_size")
#define SHP_BTN_ITEM_FONT_WEIGHT_KEY			TEXT("font_weight")
#define SHP_BTN_ITEM_CLR_MASK_KEY				TEXT("mask_clr")
#define SHP_BTN_ITEM_PAD_TOP_KEY				TEXT("text_pad_top")

#define SHP_BTN_STATE_OVER				2
#define SHP_BTN_STATE_NORMAL			3
#define SHP_BTN_STATE_HIT				4
#define SHP_BTN_STATE_DISABLE			SHP_ITEM_STATE_DISABLE
#define SHP_BTN_STATE_ENABLE			SHP_ITEM_STATE_ENABLE

class shp_btn_item : public shp_dom_item
{
public:
	shp_btn_item()
	:m_clr_btn(RGB(0xEC, 0xE9, 0xD8)), 
	m_str_text(),
	m_b_over(false),
	m_b_hit(false),
	m_i_type(0),
	m_b_bmp_set(false),
	m_i_check(INT_NO_SET),
	m_i_pre_state(INT_NO_SET),
	m_i_font_weight(400),
	m_i_font_size(12),
	m_h_cursor(NULL),
	m_clr_normal(CLR_INVALID),
	m_clr_over(CLR_INVALID),
	m_clr_down(CLR_INVALID),
	m_clr_disable(CLR_INVALID),
	m_clr_mask(CLR_INVALID),
	m_i_pad_top(0)
	{
		m_h_cursor = ::LoadCursor( NULL, IDC_HAND );
	}

	~shp_btn_item()
	{
		
	}

	bool set_bmp_obj( HBITMAP bmp_normal, HBITMAP bmp_over, HBITMAP bmp_down )
	{
		m_bmp_normal.Attach( bmp_normal );
		m_bmp_over.Attach( bmp_over );
		m_bmp_down.Attach( bmp_down );

		m_b_bmp_set = true;

		return true;
	}

	bool set_check( int i_check )
	{
		m_i_check = i_check;
		return true;
	}

	int get_check()
	{
		return m_i_check;
	}

	bool set_text( shpstr& str_text )
	{
		m_str_text = str_text;
		return true;
	}

	//set_attribute( p, m )
	BEGIN_KEY_MAP()
		//to add new attribute map( key map to member )
		ADD_CLR_KEY( SHP_BTN_ITEM_BTN_CLR_KEY,			m_clr_btn)
		ADD_CLR_KEY( SHP_BTN_ITEM_NORMAL_COLOR_KEY,		m_clr_normal)
		ADD_CLR_KEY( SHP_BTN_ITEM_OVER_COLOR_KEY,		m_clr_over)
		ADD_CLR_KEY( SHP_BTN_ITEM_DOWN_COLOR_KEY,		m_clr_down)
		ADD_CLR_KEY( SHP_BTN_ITEM_DISABLE_COLOR_KEY,	m_clr_disable)
		ADD_CLR_KEY( SHP_BTN_ITEM_CLR_MASK_KEY,			m_clr_mask)

		ADD_STR_KEY( SHP_BTN_ITEM_TEXT_KEY,				m_str_text)
		ADD_INT_KEY( SHP_BTN_ITEM_BTN_TYPE_KEY,			m_i_type)
		ADD_INT_KEY( SHP_BTN_ITEM_CHECK_KEY,			m_i_check)

		ADD_STR_KEY( SHP_BTN_ITEM_NORMAL_KEY,			m_str_normal)
		ADD_STR_KEY( SHP_BTN_ITEM_OVER_KEY,				m_str_over)
		ADD_STR_KEY( SHP_BTN_ITEM_DOWN_KEY,				m_str_down)
		ADD_STR_KEY( SHP_BTN_ITEM_DISABLE_KEY,			m_str_disable)

		ADD_STR_KEY( SHP_BTN_ITEM_NORMAL_LEFT_KEY,			m_str_normal_left)
		ADD_STR_KEY( SHP_BTN_ITEM_OVER_LEFT_KEY,			m_str_over_left)
		ADD_STR_KEY( SHP_BTN_ITEM_DOWN_LEFT_KEY,			m_str_down_left)
		ADD_STR_KEY( SHP_BTN_ITEM_DISABLE_LEFT_KEY,			m_str_disable_left)

		ADD_STR_KEY( SHP_BTN_ITEM_NORMAL_RIGHT_KEY,			m_str_normal_right)
		ADD_STR_KEY( SHP_BTN_ITEM_OVER_RIGHT_KEY,			m_str_over_right)
		ADD_STR_KEY( SHP_BTN_ITEM_DOWN_RIGHT_KEY,			m_str_down_right)
		ADD_STR_KEY( SHP_BTN_ITEM_DISABLE_RIGHT_KEY,		m_str_disable_right)

		ADD_STR_KEY( SHP_BTN_ITEM_FONT_KEY,				m_str_font_face )
		ADD_INT_KEY( SHP_BTN_ITEM_FONT_SIZE_KEY,		m_i_font_size)
		ADD_INT_KEY( SHP_BTN_ITEM_FONT_WEIGHT_KEY,		m_i_font_weight)
		ADD_INT_KEY( SHP_BTN_ITEM_PAD_TOP_KEY,			m_i_pad_top)

		BASE_CLASS_PROCESS(shp_item_win32)
	END_KEY_MAP()

protected:

	void draw_item( HDC hdc, CRect* parent_rect, CRect *left_rect )
	{
		if ( !m_b_init )
		{
			_init_rect(parent_rect, left_rect);

			if ( m_i_type > 0 && m_b_bmp_set == false )
			{
				_init_bmp();
			}
		}

		_init_font();

		int i_state = m_b_over ? SHP_BTN_STATE_OVER : SHP_BTN_STATE_NORMAL;
		if ( get_visible() )
		{
			_draw_btn( hdc, get_state() == SHP_BTN_STATE_DISABLE ? SHP_BTN_STATE_DISABLE : i_state );
		}
	}

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

	CRect* get_draw_rect()
	{
		return &m_draw_rect;
	}

	//不处理hit_test 返回 false
	LPCTSTR hit_test( HDC hdc, UINT umsg, int x, int y )
	{
		LPCTSTR lp_str = NULL;

		if ( !get_visible())
		{
			return NULL;
		}

		if ( get_state() == SHP_BTN_STATE_DISABLE )
		{
			return NULL;
		}

		if ( umsg == WM_MOUSELEAVE )
		{
			m_b_over = false;
		}

		if ( umsg == WM_MOUSEMOVE)
		{
			lp_str = _mouse_move( hdc, x, y );
		}

		if ( umsg == WM_LBUTTONUP )
		{
			lp_str = _mouse_up( hdc, x, y );
		}

		if ( umsg == WM_LBUTTONDOWN )
		{
			lp_str = _mouse_down( hdc, x, y );
		}

		if ( umsg == WM_NCHITTEST )
		{
			lp_str = _nchittest( hdc, x, y);
		}

		return lp_str;
	}

	LPCTSTR _nchittest( HDC hdc, int x, int y )
	{
		LPCTSTR lp_str = NULL;
		if ( m_draw_rect.PtInRect(CPoint(x,y)) )
		{
			lp_str = m_str_id;
		}

		return lp_str;
	}

	LPCTSTR _mouse_down( HDC hdc, int x, int y )
	{
		LPCTSTR lp_str = NULL;
		if ( m_draw_rect.PtInRect(CPoint(x,y)) )
		{
			SetCursor( m_h_cursor );
			if ( m_i_check == INT_NO_SET)
				_draw_btn( hdc, SHP_BTN_STATE_HIT );
			
			m_b_hit = true;
			lp_str = m_str_id;
		}

		return lp_str;
	}

	LPCTSTR _mouse_up( HDC hdc, int x, int y )
	{
		LPCTSTR lp_str = NULL;
		if ( m_draw_rect.PtInRect(CPoint(x,y)) )
		{
			SetCursor( m_h_cursor );
			if ( m_i_check == INT_NO_SET)
				_draw_btn( hdc, SHP_BTN_STATE_OVER );
			if ( m_b_hit)
			{
				lp_str = m_str_id;
			}

			m_b_hit = false;
		}
		else
		{
			m_b_hit = false;
		}

		return lp_str;
	}

	LPCTSTR _mouse_move( HDC hdc, int x, int y )
	{
		bool b_code = false;
		LPCTSTR lp_str = NULL;
		if ( m_draw_rect.PtInRect( CPoint(x,y)) )
		{
			SetCursor( m_h_cursor );

			if ( !m_b_over)
			{
				if ( m_i_pre_state != ( m_b_hit ? SHP_BTN_STATE_HIT : SHP_BTN_STATE_OVER ) )
				{
					_draw_btn( hdc, m_b_hit ? SHP_BTN_STATE_HIT : SHP_BTN_STATE_OVER );
				}
				
				m_b_over = true;
			}
			
			lp_str = m_str_id;
		}
		else
		{
			if ( m_i_pre_state != SHP_BTN_STATE_NORMAL )
			{
				_draw_btn( hdc, SHP_BTN_STATE_NORMAL );
			}
			
			m_b_over = false;	
		}

		return lp_str;
	}

	void _init_bmp()
	{
		//center / main bmp
		if ( m_str_normal.GetLength() > 0 )
			loadbmp( m_str_normal, m_bmp_normal );
		
		if ( m_str_over.GetLength() > 0 )
			loadbmp( m_str_over, m_bmp_over );

		if ( m_str_down.GetLength() > 0 )
			loadbmp( m_str_down, m_bmp_down );

		if ( m_str_disable.GetLength() > 0 )
			loadbmp( m_str_disable, m_bmp_disable );


		if ( m_str_normal_left.GetLength() > 0 )
			loadbmp( m_str_normal_left, m_bmp_normal_left );

		if ( m_str_over_left.GetLength() > 0 )
			loadbmp( m_str_over_left, m_bmp_over_left );

		if ( m_str_down_left.GetLength() > 0 )
			loadbmp( m_str_down_left, m_bmp_down_left );

		if ( m_str_disable_left.GetLength() > 0 )
			loadbmp( m_str_disable_left, m_bmp_disable_left );


		if ( m_str_normal_right.GetLength() > 0 )
			loadbmp( m_str_normal_right, m_bmp_normal_right );

		if ( m_str_over_right.GetLength() > 0 )
			loadbmp( m_str_over_right, m_bmp_over_right );

		if ( m_str_down_right.GetLength() > 0 )
			loadbmp( m_str_down_right, m_bmp_down_right );

		if ( m_str_disable_right.GetLength() > 0 )
			loadbmp( m_str_disable_right, m_bmp_disable_right );

	}

	void loadbmp( shpstr& str_bmp, CBitmap& bmp )
	{
		shpstr str_path((LPCTSTR)kis::kisGetPath(kis::path_images));
		str_path += str_bmp;
		HANDLE handle = NULL;
		handle = ::LoadImage(_AtlBaseModule.GetResourceInstance(), 
			str_path, IMAGE_BITMAP, 
			0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
		assert( handle);
		bmp.Attach((HBITMAP) handle );
	}

	void _init_rect( CRect* p_rect, CRect* l_rect )
	{
		if( l_rect )
		{
			m_draw_rect.left	= l_rect->right + m_i_left;
		}
		else
		{
			m_draw_rect.left	= p_rect->left + m_i_left;
		}

		m_draw_rect.top		= p_rect->top + m_i_top ;
		m_draw_rect.right	= m_draw_rect.left + m_i_width;
		m_draw_rect.bottom	= m_draw_rect.top + m_i_height;

		m_b_init = true;
	}

	void _draw_btn( HDC hdc, int i_state )
	{
		switch( m_i_type )
		{
		case 0:
			_draw_common_btn( hdc, i_state );
			break;
		case 1:
			_draw_bmp_btn( hdc, i_state );
			break;
		case 2:
			_draw_strech_bmp_btn( hdc, i_state);
			break;
		default:
			assert( false );
		}

		m_i_pre_state = i_state;
	}

	void _draw_common_btn(HDC hdc, int i_state)
	{
		CDCHandle		dc(hdc);
		int				nMode = 0;
		HFONT			hOldFont = NULL;

		nMode = dc.SetBkMode( TRANSPARENT );

		if ( !m_h_font.IsNull())
			hOldFont = dc.SelectFont( m_h_font );
		

		//draw background
		dc.FillSolidRect( &m_draw_rect, m_clr_btn );

		//draw boder
		if ( get_state() == SHP_BTN_STATE_ENABLE )
		{
			switch ( i_state )
			{
			case SHP_BTN_STATE_OVER:
				dc.Draw3dRect( &m_draw_rect, RGB(0xFF,0xFF,0xFF), RGB(0xAC,0xA8,0x99) );
				break;
			case SHP_BTN_STATE_NORMAL:
				break;
			case SHP_BTN_STATE_HIT:
				dc.Draw3dRect( &m_draw_rect, RGB(0xAC,0xA8,0x99), RGB(0xFF,0xFF,0xFF) );
				break;
			default:
				break;
			}
		}

		//draw text
		COLORREF clr_text = 0;
		if ( get_state() == SHP_BTN_STATE_DISABLE )
		{
			clr_text = dc.SetTextColor( RGB(0xAC,0xA8,0x99) );
		}
		else
		{
			clr_text = dc.SetTextColor( RGB(0x00,0x00,0x00) );
		}

		CRect rect(m_draw_rect);
		rect.top += m_i_pad_top;
		dc.DrawText( m_str_text, m_str_text.GetLength(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if ( clr_text != CLR_INVALID )
			dc.SetTextColor( clr_text );

		if ( hOldFont )
			dc.SelectFont( hOldFont );

		if ( nMode )
			dc.SetBkMode( nMode );
	}

	void _draw_bmp_btn( HDC hdc, int i_state )
	{
		CDCHandle		dc(hdc);
		int				nMode	= 0;
		HBITMAP			h_bmp	= NULL;
		shpclr			clr_text = INT_NO_SET;
		
		//draw btn
		// where the btn was check show check state
		if( m_i_check == 1 )
		{
			h_bmp = m_bmp_down;
		}
		else
		{
			if ( get_state() == SHP_BTN_STATE_ENABLE )
			{
				switch ( i_state )
				{
				case SHP_BTN_STATE_OVER:
					if ( !m_bmp_over.IsNull() )
						h_bmp = m_bmp_over;
					clr_text = m_clr_over;
					break;
				case SHP_BTN_STATE_NORMAL:
					if ( !m_bmp_normal.IsNull() )
						h_bmp = m_bmp_normal;
					clr_text = m_clr_normal;
					break;
				case SHP_BTN_STATE_HIT:
					if ( !m_bmp_down.IsNull() )
						h_bmp = m_bmp_down;
					clr_text = m_clr_down;
					break;  
				case SHP_BTN_STATE_DISABLE:
					if ( !m_bmp_disable.IsNull())
						h_bmp = m_bmp_disable;
					clr_text = m_clr_disable;
					break;
				default:
					assert(false);
					break;
				}
			}
			else
			{
				if ( !m_bmp_disable.IsNull())
					h_bmp = m_bmp_disable;
				clr_text = m_clr_disable;
			}
			
		}

		if ( h_bmp == NULL )
			h_bmp = m_bmp_normal;

		if ( clr_text == CLR_INVALID )
			clr_text = get_text_clr();

		if ( h_bmp)
		{	
			if ( m_clr_mask != CLR_INVALID )
				_draw_bmp_mask( hdc, m_draw_rect, h_bmp, m_clr_mask );
			else
				_draw_bmp( hdc, m_draw_rect, h_bmp );
		}
		

		if ( m_str_text.GetLength() > 0 )
		{
			draw_btn_text( dc, m_str_text, clr_text );
		}
	}

	void _draw_strech_bmp_btn( HDC hdc, int i_state )
	{
		CDCHandle		dc(hdc);
		int				i_mode		= 0;
		HBITMAP			h_bmp		= NULL;
		HBITMAP			h_bmp_left	= NULL;
		HBITMAP			h_bmp_right = NULL;
		shpclr			text_clr	= -1;

		//draw btn
		// where the btn was check show check state
		if( m_i_check == 1 )
		{
			h_bmp = m_bmp_down;
			h_bmp_left = m_bmp_down_left;
			h_bmp_right = m_bmp_down_right;
		}
		else
		{
			if ( get_state() == SHP_ITEM_STATE_ENABLE )
			{
				switch ( i_state )
				{
				case SHP_BTN_STATE_OVER:
					if ( !m_bmp_over.IsNull() )
					{
						h_bmp = m_bmp_over;
						h_bmp_left = m_bmp_over_left;
						h_bmp_right = m_bmp_over_right;
						text_clr = m_clr_over;
					}
					break;
				case SHP_BTN_STATE_NORMAL:
					if ( !m_bmp_normal.IsNull() )
					{
						h_bmp = m_bmp_normal;
						h_bmp_left = m_bmp_normal_left;
						h_bmp_right = m_bmp_normal_right;
						text_clr = m_clr_normal;
					}
					break;
				case SHP_BTN_STATE_HIT:
					if ( !m_bmp_down.IsNull() )
					{
						h_bmp = m_bmp_down;
						h_bmp_left = m_bmp_down_left;
						h_bmp_right = m_bmp_down_right;
						text_clr = m_clr_down;
					}
					break;
				case SHP_BTN_STATE_DISABLE:
					if ( m_bmp_disable.IsNull())
					{
						h_bmp = m_bmp_disable;
						h_bmp_left = m_bmp_disable_left;
						h_bmp_right = m_bmp_disable_right;
						text_clr = m_clr_disable;
					}
				default:
					break;
				}
			}
		}

		if ( h_bmp == NULL )
		{
			h_bmp = m_bmp_normal;
			h_bmp_left = m_bmp_normal_left;
			h_bmp_right = m_bmp_normal_right;
		}

		_draw_strech_bmp( hdc, m_draw_rect, h_bmp, h_bmp_left, h_bmp_right );

		//draw text
		if ( text_clr == CLR_INVALID )
			text_clr = get_text_clr();

		if ( m_str_text.GetLength() > 0 )
		{
			draw_btn_text( dc, m_str_text, text_clr );
		}
	}

	void draw_btn_text( CDCHandle& dc, shpstr& str_text, shpclr& clr_text )
	{
		int nOldMode = dc.SetBkMode( TRANSPARENT );
		shpclr clrOld = dc.SetTextColor( clr_text );
		HFONT hOldFont = NULL;

		if ( !m_h_font.IsNull())
			hOldFont = dc.SelectFont( m_h_font );
		CRect rect(m_draw_rect);
		rect.top += m_i_pad_top;
		
		DrawText( dc, str_text, 
			str_text.GetLength(), 
			&rect, 
			DT_CENTER | DT_VCENTER | DT_SINGLELINE );

		if ( m_h_font )
			dc.SelectFont( hOldFont);

		dc.SetTextColor( clrOld );
		dc.SetBkMode( nOldMode );
	}

private:

	shpclr		m_clr_btn;
	shpclr		m_clr_down;
	shpclr		m_clr_over;
	shpclr		m_clr_normal;
	shpclr		m_clr_disable;
	shpclr		m_clr_mask;

	shpstr		m_str_text;
	shpstr		m_str_font_face;

	int			m_i_pre_state;
	int			m_i_font_size;
	int			m_i_font_weight;
	int			m_i_pad_top;

	bool		m_b_over;
	bool		m_b_hit;
	int			m_i_type;
	int			m_i_check;

	shpstr		m_str_normal;
	shpstr		m_str_over;
	shpstr		m_str_down;
	shpstr		m_str_disable;

	shpstr		m_str_normal_left;
	shpstr		m_str_over_left;
	shpstr		m_str_down_left;
	shpstr		m_str_disable_left;

	shpstr		m_str_normal_right;
	shpstr		m_str_over_right;
	shpstr		m_str_down_right;
	shpstr		m_str_disable_right;

	CBitmap		m_bmp_normal;
	CBitmap		m_bmp_over;
	CBitmap		m_bmp_down;
	CBitmap		m_bmp_disable;

	CBitmap		m_bmp_normal_left;
	CBitmap		m_bmp_over_left;
	CBitmap		m_bmp_down_left;
	CBitmap		m_bmp_disable_left;

	CBitmap		m_bmp_normal_right;
	CBitmap		m_bmp_over_right;
	CBitmap		m_bmp_down_right;
	CBitmap		m_bmp_disable_right;

	bool		m_b_bmp_set;
	CFont		m_h_font;

	HCURSOR		m_h_cursor;
};

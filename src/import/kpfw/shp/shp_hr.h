////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shp_hr.h
//      Comment   : simple html parser hr obj class
//					
//      Create at : 2008-09-12
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shpsupport.h"

#define SHP_HR_ITEM_KEY						TEXT("hr")

#define SHP_HR_ITEM_SHADOW_KEY				TEXT("shadow")
#define SHP_HR_ITEM_SHADOWCOLOR_KEY			TEXT("shadow_color")
#define SHP_HR_ITEM_LINESTYLE_KEY			TEXT("line_style")
#define SHP_HR_ITEM_LINECOLOR_KEY			TEXT("line_color")

class shp_hr_item : public shp_dom_item
{
public:

	shp_hr_item()
	:m_i_shadow(0),
	m_clr_shadow(RGB(0x00,0xcc,0xcc)),
	m_clr_line(RGB(0x99,0x99,0x99)),
	m_i_linestyle(0)
	{}

	~shp_hr_item()
	{}

	BEGIN_KEY_MAP()
		//to add new attribute map( key map to member )
		ADD_INT_KEY( SHP_HR_ITEM_LINESTYLE_KEY, m_i_linestyle )
		ADD_CLR_KEY( SHP_HR_ITEM_SHADOWCOLOR_KEY, m_clr_shadow )
		ADD_INT_KEY( SHP_HR_ITEM_SHADOW_KEY, m_i_shadow )
		ADD_CLR_KEY( SHP_HR_ITEM_LINECOLOR_KEY, m_clr_line )
		BASE_CLASS_PROCESS(shp_item_win32)

	END_KEY_MAP()

	void draw_item( HDC hdc, CRect* parent_rect, CRect *left_rect )
	{
		if ( !m_b_init)
		{
			_init( parent_rect, left_rect );
			m_b_init = true;
		}
		
		if ( get_visible())
		{
			_draw_hr( hdc );
		}
		
	}

	CRect* get_draw_rect()
	{
		return &m_draw_rect;
	}

	LPCTSTR hit_test(  HDC hdc, UINT umsg, int x, int y  )
	{
		return NO_PROCESS;
	}

	void set_line_clr( shpclr clr )
	{
		m_clr_line = clr;
	}

protected:

	void _init( CRect* parent_rect, CRect *left_rect )
	{
		if( m_i_linestyle > 4 || m_i_linestyle < 0)
			m_i_linestyle =0;

		if ( m_i_shadow != 1)
			m_i_shadow = 0;

		if( m_i_height < 1 )
			m_i_height = 1;

		m_draw_rect.left = GETRIGHTVALUE( m_i_left ) + parent_rect->left;
		m_draw_rect.top = GETRIGHTVALUE( m_i_top ) + parent_rect->top;

		m_draw_rect.bottom = m_draw_rect.top + m_i_height;
		m_draw_rect.right = (m_i_width != INT_NO_SET ? 
							m_draw_rect.left + m_i_width : 
							parent_rect->right - m_draw_rect.left);

		//need to draw shadow
		if ( m_i_shadow == 1)
			m_draw_rect.bottom += m_i_height;
	}

	void _draw_hr( HDC hdc )
	{
		HPEN h_pen = ::CreatePen( m_i_linestyle, m_i_height, m_clr_line );
		HPEN h_shadow_pen = ::CreatePen( m_i_linestyle, m_i_height, m_clr_shadow );

		HPEN h_old_pen = (HPEN)::SelectObject( hdc, h_pen );
		::MoveToEx( hdc, m_draw_rect.left, m_draw_rect.top, NULL );
		::LineTo( hdc, m_draw_rect.right, m_draw_rect.top );

		if( m_i_shadow == 1)
		{
			SelectObject(hdc, h_shadow_pen );
			::MoveToEx( hdc, m_draw_rect.left, m_draw_rect.top + m_i_height , NULL );
			::LineTo( hdc, m_draw_rect.right, m_draw_rect.top + m_i_height );
		}
		
		if ( h_old_pen )
			::SelectObject( hdc, h_old_pen );
		
		if ( h_pen )
			::DeleteObject( h_pen );

		if ( h_shadow_pen )
			::DeleteObject( h_shadow_pen );
	}

	int m_i_shadow;
	shpclr m_clr_shadow;
	shpclr m_clr_line;

	/* Pen Styles */
	//#define PS_SOLID            0
	//#define PS_DASH             1       /* -------  */
	//#define PS_DOT              2       /* .......  */
	//#define PS_DASHDOT          3       /* _._._._  */
	//#define PS_DASHDOTDOT       4       /* _.._.._  */
	int m_i_linestyle;

};

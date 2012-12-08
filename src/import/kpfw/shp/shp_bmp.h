////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shp_bmp.h
//      Comment   : simple html parser bmp obj class
//					
//      Create at : 2008-09-11
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shpsupport.h"
#include <atlgdi.h>

#define SHP_BMP_ITEM_KEY					TEXT("bmp")

#define SHP_BMP_ITEM_SRC_KEY				TEXT("src")
#define SHP_BMP_ITEM_HANDLE_KEY				TEXT("handle")
#define SHP_BMP_MASK_CLR_KEY				TEXT("mask_clr")


class shp_bmp_item : public shp_dom_item
{
public:

	shp_bmp_item()
		:m_i_handle(NULL),
		m_b_handle(false),
		m_b_src(false),
		m_clr_mask(CLR_INVALID)
	{};

	~shp_bmp_item()
	{
		delect_bmp();
	};

	//set_attribute( p, m )
	BEGIN_KEY_MAP()
		//to add new attribute map( key map to member )
		ADD_STR_KEY		( SHP_BMP_ITEM_SRC_KEY,			m_str_src )
		ADD_HBITMAP_KEY	( SHP_BMP_ITEM_HANDLE_KEY,		m_i_handle )
		ADD_CLR_KEY		(SHP_BMP_MASK_CLR_KEY,			m_clr_mask )
		BASE_CLASS_PROCESS(shp_item_win32)
	END_KEY_MAP()

	void draw_item( HDC hdc, CRect* parent_rect, CRect *left_rect )
	{
		if ( !m_b_init)
		{
			_init_rect( hdc, parent_rect, left_rect );
			m_b_init = true;
		}
		
		if ( get_visible())
		{
			if ( !m_bmp.IsNull())
			{
				if ( m_clr_mask != CLR_INVALID )
					_draw_bmp_mask( hdc, m_draw_rect, m_bmp, m_clr_mask );
				else
					_draw_bmp( hdc, m_draw_rect, m_bmp );
			}
		}
		
	}

	CRect* get_draw_rect()
	{
		return &m_draw_rect;
	}

	//这个设置一般用的的全路径
	void set_str_src( shpstr str_src)
	{
		m_str_src = str_src;
		m_b_src = true;
	}

	void set_handle( HBITMAP handle )
	{
		//assert( m_i_handle == NULL );
		m_i_handle = handle;
		m_b_handle = true;
	}

	LPCTSTR hit_test( HDC hdc, UINT umsg, int x, int y )
	{
		LPCTSTR lp_id = NO_PROCESS;

		if ( umsg == WM_MOUSEMOVE )
			lp_id = _mouse_move( hdc, x, y );

		return lp_id;
	}

	HBITMAP get_handle()
	{
		return m_i_handle;
	}

	LPCTSTR _mouse_move( HDC hdc, int x, int y )
	{
		LPCTSTR lp_str = NULL;
		if ( m_draw_rect.PtInRect( CPoint(x,y)) )
		{
			if ( m_str_id.GetLength() > 0 )
				lp_str = m_str_id;
		}

		return lp_str;
	}

	shpstr get_str_src()
	{
		return m_str_src;
	}

	bool delect_bmp()
	{
		if ( !m_bmp.IsNull())
		{
			m_bmp.DeleteObject();
		}

		if( m_i_handle )
		{
			::DeleteObject( m_i_handle );
			m_i_handle = NULL;
		}

		return true;
	}


protected:

	void _init_rect( HDC hdc, CRect *p_rect, CRect *left_rect )
	{
		//如果外面主动设置set_i_handle 不使用src
		if ( m_str_src.GetLength() > 0  && m_b_handle == false )
		{
			shpstr str_path;

			//不是通过set_str_src 来指定
			if( !m_b_src )
				str_path = (LPCTSTR)kis::kisGetPath(kis::path_images);
			str_path += m_str_src;
			HANDLE handle = ::LoadImage( _AtlBaseModule.GetResourceInstance(),
										str_path, IMAGE_BITMAP, 
										0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
			assert( handle );

			//清除由src而load的资源, 防止多少set_src 会导致资源泄漏
			if( m_b_src == true )
			{
				if ( !m_bmp.IsNull())
					m_bmp.DeleteObject();
				
				m_b_src = false;
			}

			m_bmp.Attach((HBITMAP) handle );
			m_b_src = true;

			assert( !m_bmp.IsNull() );
		}
		else
		{
			//清除由src而load的资源，防止多少set_bmp_handle 会导致资源泄漏
			if( m_b_src == true )
			{
				if ( !m_bmp.IsNull())
				{
					m_bmp.DeleteObject();
				}
				
				m_b_src = false;
			}

			if ( m_i_handle )
				m_bmp.Attach( (HBITMAP)m_i_handle );
			
		}
		
		if ( !m_bmp.IsNull() )
		{
			BITMAP bmp;
			m_bmp.GetBitmap( &bmp );
			
			if ( !left_rect )
			{
				m_draw_rect.left = GETRIGHTVALUE( m_i_left) + p_rect->left;
				m_draw_rect.top = GETRIGHTVALUE( m_i_top ) + p_rect->top;

				m_draw_rect.right = ( bmp.bmWidth > GETRIGHTVALUE(m_i_width) ? m_i_width : bmp.bmWidth ) 
									+ m_draw_rect.left;

				m_draw_rect.bottom = ( bmp.bmHeight > GETRIGHTVALUE(m_i_height) ? m_i_height : bmp.bmHeight ) 
									+ m_draw_rect.top;
			}
			else
			{
				m_draw_rect.left = GETRIGHTVALUE( m_i_left) + left_rect->left;
				m_draw_rect.top = GETRIGHTVALUE( m_i_top ) + left_rect->top ;

				m_draw_rect.right = ( bmp.bmWidth > GETRIGHTVALUE(m_i_width) ? m_i_width : bmp.bmWidth ) 
									+ p_rect->left + GETRIGHTVALUE(m_i_left);

				m_draw_rect.bottom = ( bmp.bmHeight > GETRIGHTVALUE(m_i_height) ? m_i_height : bmp.bmHeight ) 
									+ m_draw_rect.top + GETRIGHTVALUE( m_i_top );
			}
			
		}
	}

	

	

	

private:
	HBITMAP				m_i_handle;
	shpstr				m_str_src;
	WTL::CBitmapHandle	m_bmp;
	bool				m_b_handle;
	bool				m_b_src;
	shpclr				m_clr_mask;
};


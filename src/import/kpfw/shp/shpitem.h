
////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shpitem.h
//      Comment   : simple html parser item	 class
//					
//      Create at : 2008-09-04
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>
#include <atlstr.h>
#include <atlmisc.h>
#include "shp_misc.h"

#define SHP_ITEM_NAME_KEY		TEXT("name")
#define SHP_ITEM_HEIGHT_KEY		TEXT("height")
#define SHP_ITEM_WIDTH_KEY		TEXT("width")
#define SHP_ITEM_TOP_KEY		TEXT("top")
#define SHP_ITEM_BOTTOM_KEY		TEXT("bottom")
#define SHP_ITEM_LEFT_KEY		TEXT("left")
#define SHP_ITEM_RIGHT_KEY		TEXT("right")
#define SHP_ITEM_CLR_TEXT_KEY	TEXT("clr_text")
#define SHP_ITEM_CLR_BG_KEY		TEXT("clr_bg")
#define SHP_ITEM_ID_KEY			TEXT("id")
#define SHP_ITEM_VISIBLE_KEY	TEXT("visible")
#define SHP_ITEM_DISPLAY_KEY	TEXT("display")

#define INT_NO_SET				-1

#define SHP_ITEM_STATE_ENABLE			0
#define SHP_ITEM_STATE_DISABLE			1

#define GETRIGHTVALUE(m) ( m == INT_NO_SET ? 0 : m )

#define BEGIN_KEY_MAP() \
	virtual void set_attribute( LPTSTR p_key, LPTSTR p_value )\
	{

#define ADD_STR_KEY(lp_key_text, member ) \
	if( _tcsicmp(p_key, lp_key_text ) == 0 )\
	{\
		member = shpstr(p_value);\
	}

#define ADD_INT_KEY( lp_key_text, member ) \
	if( _tcsicmp(p_key, lp_key_text ) == 0 )\
	{\
		member =  _ttoi(p_value);\
	}

#define ADD_HBITMAP_KEY( lp_key_text, member ) \
	if( _tcsicmp(p_key, lp_key_text ) == 0 )\
{\
	member =  (HBITMAP)_ttoi(p_value);\
}



#define ADD_CLR_KEY( lp_key_text, member ) \
	if( _tcsicmp(p_key, lp_key_text ) == 0 )\
	{\
		member = _ttoclr(p_value);\
	}

#define BASE_CLASS_PROCESS(baseclass) \
	baseclass::set_attribute( p_key, p_value );

#define END_KEY_MAP()	};


class shp_item
{
public:
	shp_item()
	:m_clr_text(RGB(0, 0, 0)),
	m_clr_bg(INT_NO_SET),
	m_i_postype(INT_NO_SET),
	m_i_type(INT_NO_SET),
	m_i_top(INT_NO_SET),
	m_i_left(INT_NO_SET),
	m_i_bottom(INT_NO_SET),
	m_i_right(INT_NO_SET),
	m_i_height(INT_NO_SET),
	m_i_width(INT_NO_SET),
	m_b_init(false),
	m_i_visible(1),
	m_i_state(SHP_ITEM_STATE_ENABLE),
	m_i_display(1)
	{

	};

	~shp_item()
	{

	};

	//add key str map to a member
	BEGIN_KEY_MAP()
		ADD_STR_KEY( SHP_ITEM_NAME_KEY, m_str_name )
		ADD_INT_KEY( SHP_ITEM_HEIGHT_KEY, m_i_height )
		ADD_INT_KEY( SHP_ITEM_WIDTH_KEY, m_i_width )
		ADD_INT_KEY( SHP_ITEM_TOP_KEY, m_i_top )
		ADD_INT_KEY( SHP_ITEM_BOTTOM_KEY, m_i_bottom )
		ADD_INT_KEY( SHP_ITEM_LEFT_KEY, m_i_left )
		ADD_INT_KEY( SHP_ITEM_RIGHT_KEY, m_i_right )
		ADD_CLR_KEY( SHP_ITEM_CLR_TEXT_KEY, m_clr_text )
		ADD_CLR_KEY( SHP_ITEM_CLR_BG_KEY, m_clr_bg )
		ADD_STR_KEY( SHP_ITEM_ID_KEY, m_str_id )
		ADD_INT_KEY( SHP_ITEM_VISIBLE_KEY, m_i_visible)
		ADD_INT_KEY( SHP_ITEM_DISPLAY_KEY, m_i_display)
	END_KEY_MAP()

	//set
	inline void set_name( shpstr name)
	{
		m_str_name = name;
	}

	inline void set_str_type( shpstr name)
	{
		m_str_type = name;
	}

	inline void set_text( shpstr text )
	{
		m_str_text = text;
	}

	inline void set_clr_text( shpclr clrtext )
	{
		m_clr_text = clrtext;
	}

	inline void set_clr_bg( shpclr clrbg )
	{
		m_clr_bg = clrbg;
	}

	inline void set_i_top( int i_top )
	{
		m_i_top = i_top;
	}

	inline void set_i_left( int i_left)
	{
		m_i_left = i_left;
	}

	inline void set_i_bottom( int i_bottom )
	{
		m_i_bottom = i_bottom;
	}

	inline void set_i_right( int i_right )
	{
		m_i_right = i_right;
	}

	inline void set_postype( int i_postype)
	{
		m_i_postype = i_postype;
	}

	inline void set_i_width( int i_width)
	{
		m_i_width = i_width;
	}

	inline void set_i_height( int i_height )
	{
		m_i_height = i_height;
	}
	
	inline void set_visible( int i_visible )
	{
		m_i_visible = i_visible;
	}

	inline void set_state( int i_state )
	{
		m_i_state = i_state;
	}

	inline void set_display( int i_display )
	{
		m_i_display = i_display;
	}

	//get
	inline shpstr& get_name()
	{
		return m_str_name;
	}

	inline shpstr& get_str_type()
	{
		return m_str_type;
	}

	inline shpclr& get_text_clr()
	{
		return m_clr_text;
	}

	inline shpstr& get_text()
	{
		return m_str_text;
	}

	inline void get_rect( LPRECT lp_rect )
	{
		lp_rect->bottom = m_draw_rect.bottom ;
		lp_rect->left = m_draw_rect.left;
		lp_rect->right = m_draw_rect.right;
		lp_rect->top = m_draw_rect.top ;
	}

	inline int get_i_top()
	{
		return m_i_top;
	}

	inline int get_i_left()
	{
		return m_i_left;
	}

	inline int get_i_bottom ()
	{
		return m_i_bottom;
	}

	inline int get_i_right()
	{
		return m_i_right;
	}

	inline int get_i_width()
	{
		return m_i_width;
	}

	inline int get_postype()
	{
		return m_i_postype;
	}

	inline COLORREF get_clr_bg()
	{
		return m_clr_bg;
	}

	inline shpstr& get_str_id()
	{
		return m_str_id;
	}

	virtual void set_reinit()
	{
		m_b_init = false;
		//draw_rect_zero();
	}

	inline int get_visible()
	{
		return m_i_visible;
	}

	inline int get_state()
	{
		return m_i_state;
	}

	inline int get_display()
	{
		return m_i_display;
	}

	//virtual void draw_item() = 0; 
	//
	inline void draw_rect_zero()
	{
		m_draw_rect.left = 0;
		m_draw_rect.right = 0;
		m_draw_rect.bottom = 0;
		m_draw_rect.top = 0;
	}

protected:

	shpstr		m_str_name;
	shpstr		m_str_type;
	shpstr		m_str_text;
	shpstr		m_str_id;
	
	shpclr		m_clr_text;
	shpclr		m_clr_bg;

	int			m_i_top;
	int			m_i_left;
	int			m_i_bottom;
	int			m_i_right;

	int			m_i_height;
	int			m_i_width;

	int			m_i_visible;
	int			m_i_state;
	int			m_i_display;


	//0 relativity 1 absolute 
	int			m_i_postype;

	int			m_i_type;


	//¸¨Öú±äÁ¿
	bool m_b_init;
	CRect m_draw_rect;
};
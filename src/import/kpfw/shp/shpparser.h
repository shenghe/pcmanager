////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shpparser.h
//      Comment   : shp parser html file
//					
//      Create at : 2008-09-04
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "shpsupport.h"
#include "shp_div.h"
#include "shp_bmp.h"
#include "shp_hr.h"
#include "shp_link.h"
#include "shp_btn.h"
#include "shp_shape.h"
#include "shp_txt.h"

typedef shp_tree_base< shp_dom_item >						shp_dom;

#define BEGIN_BASE_CLASS_MAP() \
	shp_dom_item* _get_base_class_ptr( LPTSTR tsz_type )\
	{\
		shp_dom_item * p_item = NULL;


#define ADD_TYPE_CLASS( lp_type_key, class_name ) \
	if( _tcsicmp(tsz_type, lp_type_key ) == 0 )\
	{\
		p_item = ( shp_dom_item* )new shp_tree_item< class_name >;\
	}

#define END_BASE_CLASS_MAP() \
		return p_item; \
	}



class shp_shpparser
{
public:
	shp_shpparser()
	{};
	~shp_shpparser(){};

	BEGIN_BASE_CLASS_MAP()
		//to do: add you own class map
		ADD_TYPE_CLASS( SHP_DIV_ITEM_KEY,	shp_div_item )
		ADD_TYPE_CLASS( SHP_BMP_ITEM_KEY,	shp_bmp_item )
		ADD_TYPE_CLASS( SHP_HR_ITEM_KEY,	shp_hr_item )
		ADD_TYPE_CLASS( SHP_LINK_ITEM_KEY,	shp_link_item )
		ADD_TYPE_CLASS( SHP_BTN_ITEM_KEY,	shp_btn_item )
		ADD_TYPE_CLASS( SHP_SHAPE_ITEM_KEY, shp_shape_item)
		ADD_TYPE_CLASS( SHP_TXT_ITEM_KEY,	shp_txt_item)

	END_BASE_CLASS_MAP()

	LPTSTR prase_text( LPCTSTR p_text )
	{
		UINT ucount = _tcslen(p_text);
		LPTSTR tsz_parse = new TCHAR[ ucount + 1];
		LPTSTR p_move = tsz_parse;
		memset( tsz_parse, 0 , sizeof(TCHAR)*ucount );

		while ( *p_text )
		{
			switch( *p_text )
			{
			case TEXT('\r'):
				break;
			case TEXT('\n'):
				break;
			default:
				*p_move = *p_text;
				p_move ++;
				break;
			}
			p_text ++;
		}

		*p_move = TEXT('\0');

		parse( tsz_parse, NULL );

		return tsz_parse;
	}
	//返回已经分析过的文本指针
	LPCTSTR parse( LPCTSTR p_start, shp_dom_item* p_parent )
	{
		LPCTSTR p_end_tag = NULL;
		LPCTSTR	p_start_tag = NULL;
		shp_dom_item* p_item = NULL;
		while ( *p_start )
		{
			//如果找不到'<',退出不需要,当是TEXT
			p_start_tag = _tcschr( p_start, TEXT('<') );
			if( p_start_tag == NULL )
				return NULL;

			if( *( p_start_tag+1 ) != TEXT('/'))
			{
				//有新的节点object
				p_end_tag  = _tcschr( p_start_tag, TEXT('>') );
				if ( p_end_tag == NULL )
					return NULL;
				else
				{
					TCHAR tsz_name[255] = {0};
					if ( p_start_tag )
					{
						memset( tsz_name, 0, sizeof(TCHAR)*255 );
						p_start_tag = _get_type( p_start_tag+1, p_end_tag - 1, tsz_name);
						if ( p_start_tag )
						{
							//p_item = new shp_dom_item;
							p_item = _get_base_class_ptr( tsz_name );
							p_item->set_str_type( shpstr(tsz_name));
						}
						else
							return NULL;
					}

					//p_parent == NULL add as a root item
					//else  add as a comstom item
					m_dom.insert_child( p_item, p_parent, false );

					_parse_attribute( p_start_tag + 1, p_end_tag - 1, p_item ); //进行属性分析
					p_start = p_end_tag + 1;
					if ( !(p_start = parse( p_start, p_item )) )
						return NULL;
				}
			}
			else
			{
				//get text
				TCHAR *psz_text = new TCHAR[p_start_tag - p_start + 1];
				memset( psz_text, 0, sizeof(TCHAR)*(p_start_tag - p_start + 1) );
				_tcsncpy( psz_text, p_start, p_start_tag - p_start );

				//assert( p_item );
				if ( p_parent )
					p_parent->set_text( shpstr(psz_text) );


				//处理 </div>, 指针后移
				//*( p_start+1 ) = TEXT('/')
				p_end_tag = _tcschr( p_start, TEXT('>') );
				if ( p_end_tag == NULL )
				{
					if ( psz_text)
					{
						delete []psz_text;
						psz_text = NULL;
					}
					return NULL;
				}
				else
				{
					p_start = p_end_tag;
					p_start++;
					if ( psz_text)
					{
						delete []psz_text;
						psz_text = NULL;
					}
					break;	//返回上一层
				}
			}

			//p_start ++;
		}

		return p_start;
	}

	LPCTSTR hit_test( HDC hdc, UINT umsg, int x, int y )
	{
		assert( m_dom.get_root_item() );
		if ( m_dom.get_root_item() )
		{
			return _hit_test( m_dom.get_root_item(), hdc, umsg, x ,y );
		}
		return NULL;
	}


	void hit_test_no_return( shp_dom_item* p_item, HDC hdc, UINT umsg, int x, int y  )
	{
		assert( p_item );

		if ( p_item )
		{
			if ( p_item->get_display() )
			{
				p_item->hit_test( hdc,umsg,x, y);
				
				if( p_item->get_firstchild())
					hit_test_no_return( p_item->get_firstchild(), hdc , umsg, x, y );


				if ( p_item->get_right())
					hit_test_no_return( p_item->get_right(), hdc, umsg, x, y );
			}
			else
			{
				if ( p_item->get_right())
					hit_test_no_return( p_item->get_right(), hdc, umsg, x, y );
			}
		}

		return;
	}

	LPCTSTR _hit_test( shp_dom_item* p_item, HDC hdc, UINT umsg, int x, int y )
	{
		assert( p_item );
		LPCTSTR lp_str_id = NULL;

		if ( p_item )
		{
			if ( p_item->get_display() )
			{
				lp_str_id = p_item->hit_test( hdc,umsg,x, y);
				if ( lp_str_id != NULL )
				{
					//对后续节点也进行hit_test (bug)
					if( p_item->get_firstchild())
						hit_test_no_return( p_item->get_firstchild(), hdc , umsg, x, y );
					if ( p_item->get_right())
						hit_test_no_return( p_item->get_right(), hdc, umsg, x, y );
					return lp_str_id;
				}
			
				if( p_item->get_firstchild())
					lp_str_id = _hit_test( p_item->get_firstchild(), hdc , umsg, x, y );

				if ( lp_str_id != NULL )
				{
					//对后续节点也进行hit_test (bug)
					if ( p_item->get_right())
						hit_test_no_return( p_item->get_right(), hdc, umsg, x, y );
					return lp_str_id;
				}

				if ( p_item->get_right())
					lp_str_id = _hit_test( p_item->get_right(), hdc, umsg, x, y );
			}
			else
			{
				if ( p_item->get_right())
					lp_str_id = _hit_test( p_item->get_right(), hdc, umsg, x, y );
			}
			
		}

		return lp_str_id;
	}

	void dom_draw( HDC hdc, CRect* p_rect )
	{
		assert( m_dom.get_root_item() );
		if ( m_dom.get_root_item() )
		{
			_draw( hdc, m_dom.get_root_item(),  p_rect, NULL );
		}
		return;
	}

	void _draw( HDC hdc, shp_dom_item* p_item,  CRect* parent_rect, CRect* left_rect )
	{
		assert( p_item );

		if ( p_item )
		{
			if( p_item->get_display() )
			{
				p_item->draw_item( hdc, parent_rect, left_rect );

				if( p_item->get_firstchild() )
					_draw( hdc, p_item->get_firstchild(), p_item->get_draw_rect(), NULL );

				if ( p_item->get_right())
					_draw( hdc, p_item->get_right(), parent_rect, p_item->get_draw_rect());
			}
			else
			{
				if ( p_item->get_right())
					_draw( hdc, p_item->get_right(), parent_rect, left_rect );
			}
		}

		
	}

	shp_dom_item * get_item_by_id( shpstr* p_str_id )
	{
		return m_dom.find_item_by_id( p_str_id );
	}

	//为了效率优化，才加 i_visible， 还有独立的set_visible() 函数
	void set_item_text( LPCTSTR lp_str_id, LPCTSTR lp_text, int i_visible )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			p_item->set_text( lp_text );
			p_item->set_visible( i_visible );
			p_item->set_reinit();
		}
	}

	void set_item_text_nochange_visible( LPCTSTR lp_str_id, LPCTSTR lp_text )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			p_item->set_text( lp_text );
			p_item->set_reinit();
		}
	}

	void set_item_text_clr( LPCTSTR lp_str_id, COLORREF clr )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			p_item->set_clr_text( clr );
			p_item->set_reinit();
		}
	}


	bool set_item_visible( LPCTSTR lp_str_id, int i_visible )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			p_item->set_visible( i_visible );
			return true;
		}
		return false;
	}

	bool set_item_state( LPCTSTR lp_str_id, int n_state )
	{
		shpstr str_id( lp_str_id);
		shp_dom_item* p_item = get_item_by_id( &str_id );
		assert( p_item );
		if ( p_item )
		{
			p_item->set_state( n_state );
			return true;
		}
		return false;
	}

	bool set_item_left( LPCTSTR lp_str_id, int n_left )
	{
		shpstr str_id( lp_str_id);
		shp_dom_item* p_item = get_item_by_id( &str_id );
		assert( p_item );
		if ( p_item )
		{
			p_item->set_i_left( n_left );
			_re_init( p_item );
			return true;
		}
		return false;
	}

	int get_item_left( LPCTSTR lp_str_id )
	{
		shpstr str_id( lp_str_id);
		shp_dom_item* p_item = get_item_by_id( &str_id );
		assert( p_item );
		if ( p_item )
		{
			return p_item->get_i_left();
		}
		return INT_NO_SET;
	}

	bool set_item_width( LPCTSTR lp_str_id, int n_width )
	{
		shpstr str_id( lp_str_id);
		shp_dom_item* p_item = get_item_by_id( &str_id );
		assert( p_item );
		if ( p_item )
		{
			p_item->set_i_width( n_width );
			_re_init( p_item );
			return true;
		}
		return false;
	}

	bool set_item_height(LPCTSTR lp_str_id, int m_height )
	{
		shpstr str_id( lp_str_id);
		shp_dom_item* p_item = get_item_by_id( &str_id );
		assert( p_item );
		if ( p_item )
		{
			p_item->set_i_height( m_height );
			_re_init( p_item );
			return true;
		}
		return false;
	}

	int get_item_width( LPCTSTR lp_str_id )
	{
		shpstr str_id( lp_str_id);
		shp_dom_item* p_item = get_item_by_id( &str_id );
		assert( p_item );
		if ( p_item )
		{
			return p_item->get_i_width( );
		}
		return INT_NO_SET;
	}

	LPCTSTR get_item_text( LPCTSTR lp_str_id )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			return p_item->get_text();
		}

		return NULL;
	}


	void get_item_rect( LPCTSTR lp_str_id, LPRECT lp_rect )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			p_item->get_rect( lp_rect );
		}

		return;
	}

	void set_item_display( LPCTSTR lp_str_id , int i_display )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			p_item->set_display( i_display );
			re_init();
		}

		return;
	}

	int get_div_height( LPCTSTR lp_str_id )
	{
		int nHeight = 0;
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_DIV_ITEM_KEY) == 0 )
			{
				shp_div_item * p_div = (shp_div_item*)p_item;
				nHeight = p_div->get_draw_rect()->Height();
			}
			else
			{
				assert( false );
			}
		}

		return nHeight;
	}

	//为了效率优化，才加 i_visible， 还有独立的set_visible() 函数
	void set_bmp_src( LPCTSTR lp_str_id, LPCTSTR lp_src, int i_visible )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BMP_ITEM_KEY) == 0 )
			{
				shp_bmp_item * p_bmp = (shp_bmp_item*)p_item;
				p_bmp->set_str_src( lp_src );
				p_bmp->set_visible( i_visible);
				p_bmp->set_reinit();
			}
			else
			{
				assert( false );
			}
		}
	}

	//为了效率优化，才加 i_visible， 还有独立的set_visible() 函数
	void set_bmp_handle( LPCTSTR lp_str_id, HBITMAP hbmp, int i_visible )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BMP_ITEM_KEY) == 0 )
			{
				shp_bmp_item * p_bmp = (shp_bmp_item*)p_item;
				p_bmp->set_handle( hbmp );
				p_bmp->set_visible( i_visible );
				p_bmp->set_reinit();
			}
			else
			{
				assert( false );
			}
		}
	}

	void set_btn_bmp_obj( LPCTSTR lp_str_id, HBITMAP hbmp, int i_visible = 1 )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BTN_ITEM_KEY) == 0 )
			{
				shp_btn_item * p_btn = (shp_btn_item*)p_item;
				p_btn->set_bmp_obj( hbmp, NULL, NULL );
				p_btn->set_visible( i_visible );
				p_btn->set_reinit();
			}
			else
			{
				assert( false );
			}
		}
	}

	void set_btn_check( LPCTSTR lp_str_id, int i_check, int i_visible = 1 )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BTN_ITEM_KEY) == 0 )
			{
				shp_btn_item * p_btn = (shp_btn_item*)p_item;
				p_btn->set_check( i_check );
			}
			else
			{
				assert( false );
			}
		}
	}

	int get_btn_check( LPCTSTR lp_str_id )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BTN_ITEM_KEY) == 0 )
			{
				shp_btn_item * p_btn = (shp_btn_item*)p_item;
				return p_btn->get_check( );
			}
			else
			{
				assert( false );
			}
		}

		return -1;
	}

	void set_btn_text( LPCTSTR lp_str_id, LPCTSTR lp_str_text, int i_visible = 1 )
	{
		shpstr str_id(lp_str_id);
		shpstr str_text(lp_str_text);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BTN_ITEM_KEY) == 0 )
			{
				shp_btn_item * p_btn = (shp_btn_item*)p_item;
				p_btn->set_text( str_text );
				p_btn->set_visible( i_visible );
			}
			else
			{
				assert( false );
			}
		}
	}

	void set_line_clr( LPCTSTR lp_str_id, COLORREF clr )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_HR_ITEM_KEY) == 0 )
			{
				shp_hr_item * p_bmp = (shp_hr_item*)p_item;
				p_bmp->set_line_clr( clr );
				p_bmp->set_reinit();
			}
			else
			{
				assert( false );
			}
		}
	}

	void set_hyperlink_enable(LPCTSTR lp_str_id, int i_enable )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_LINK_ITEM_KEY) == 0 )
			{
				shp_link_item * p_link = (shp_link_item*)p_item;
				return p_link->set_enable( i_enable );
			}
			else
			{
				assert( false );
			}
		}

		return;
	}

	HBITMAP get_bmp_handle( LPCTSTR lp_str_id )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BMP_ITEM_KEY) == 0 )
			{
				shp_bmp_item * p_bmp = (shp_bmp_item*)p_item;
				return p_bmp->get_handle();
			}
			else
			{
				assert( false );
			}
		}
		return NULL;
	}

	int get_shape_fill( LPCTSTR lp_str_id )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_SHAPE_ITEM_KEY) == 0 )
			{
				shp_shape_item * p_shape = (shp_shape_item*)p_item;
				return p_shape->get_fill();
			}
			else
			{
				assert( false );
			}
		}
		return NULL;
	}


	void set_spe_clr( LPCTSTR lp_str_id, shpclr clr_spen )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_TXT_ITEM_KEY) == 0 )
			{
				shp_txt_item * p_txt = (shp_txt_item*)p_item;
				p_txt->set_spe_clr( clr_spen );
			}
			else
			{
				assert( false );
			}
		}
		return;
	}

	void set_spe_word( LPCTSTR lp_str_id, shpstr& str_spe_word  )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_TXT_ITEM_KEY) == 0 )
			{
				shp_txt_item * p_txt = (shp_txt_item*)p_item;
				p_txt->set_spe_word( str_spe_word );
			}
			else
			{
				assert( false );
			}
		}
		return;
	}



	void set_shape_fill( LPCTSTR lp_str_id, int i_fill )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_SHAPE_ITEM_KEY) == 0 )
			{
				shp_shape_item * p_shape = (shp_shape_item*)p_item;
				p_shape->set_fill( i_fill );
			}
			else
			{
				assert( false );
			}
		}
		return ;
	}

	bool delete_bmp( LPCTSTR lp_str_id )
	{
		shpstr str_id(lp_str_id);
		shp_dom_item*p_item = get_item_by_id( &str_id );
		if( p_item )
		{
			if ( _tcsicmp( p_item->get_str_type(), SHP_BMP_ITEM_KEY) == 0 )
			{
				shp_bmp_item * p_bmp = (shp_bmp_item*)p_item;
				return p_bmp->delect_bmp();
			}
			else
			{
				assert( false );
			}
		}
		return false;
	}

	void re_init()
	{
		assert( m_dom.get_root_item() );
		if ( m_dom.get_root_item() )
		{
			_re_init( m_dom.get_root_item() );
		}
		return;
	}

	void _re_init( shp_dom_item* p_item )
	{
		assert( p_item );

		if ( p_item )
		{
			p_item->set_reinit();
		}

		if( p_item->get_firstchild())
		{
			_re_init( p_item->get_firstchild() );
		}

		if ( p_item->get_right())
		{
			_re_init( p_item->get_right() );
		}
	}

protected:
	inline bool is_white_space( TCHAR c )
	{ 
		return ( isspace( c ) || c == TEXT('\n') || c == TEXT('\r') || c == TEXT('\t')); 
	}

	void _parse_attribute( LPCTSTR p_start, LPCTSTR p_end, shp_dom_item* p_item )
	{
		LPCTSTR p_start_key = p_start;
		LPCTSTR p_end_key	= p_end;
		TCHAR tsz_name[255]		= {0};
		TCHAR tsz_key[255]		= {0};
		TCHAR tsz_value[255]	= {0};
		
		assert( p_item );

		while ( p_start_key )
		{
			memset( tsz_key, 0, sizeof(TCHAR)*255 );
			p_start_key = _get_key( p_start_key, p_end_key, tsz_key );
			if ( !p_start_key )
				break;

			memset( tsz_value, 0, sizeof(TCHAR)*255 );
			p_start_key = _get_value( p_start_key, p_end_key, tsz_value);
			if ( !p_start_key )
				break;
			
			p_item->set_attribute( tsz_key, tsz_value );
		}

		return;
	}


	LPCTSTR _get_type( LPCTSTR p_start, LPCTSTR p_end, LPTSTR p_buffer )
	{
		return _get_key( p_start, p_end, p_buffer );
	}

	LPCTSTR _get_key( LPCTSTR p_start, LPCTSTR p_end, LPTSTR p_buffer )
	{
		LPCTSTR p_start_key = p_start;
		LPCTSTR p_end_key	= p_end;

		//find the start position of the key
		while ( is_white_space(*p_start_key) || !_istalnum(*p_start_key) )
		{
			if ( p_start_key > p_end )
				return NULL;
			p_start_key ++;
		}

		p_end_key = p_start_key;

		//find the end position of key
		while ( !is_white_space(*p_end_key) && 
				(*p_end_key != TEXT('=')) && 
				(_istalnum(*p_end_key) || *p_end_key == TEXT('_'))
			)
		{
			if ( p_end_key > p_end)
				return NULL;
			p_end_key ++;
		}

		_tcsncpy( p_buffer, p_start_key, p_end_key - p_start_key );

		//p_end_key is white space
		return p_end_key;
	}

	LPCTSTR _get_value( LPCTSTR p_start, LPCTSTR p_end, LPTSTR p_buffer )
	{
		LPCTSTR p_start_key = p_start;
		LPCTSTR p_end_key	= p_end;

		//find the first "
		p_start_key = _tcschr( p_start_key, TEXT('\"'));
		if ( !p_start_key || p_start_key > p_end )
			return NULL;

		p_end_key = ++p_start_key;

		p_end_key = _tcschr( p_end_key, TEXT('\"') );
		if ( !p_end_key || p_end_key > p_end )
			return NULL;

		_tcsncpy( p_buffer, p_start_key, p_end_key - p_start_key );

		//p_end_key is white space
		return ++p_end_key;
	}

protected:

	shp_dom m_dom;
	
};





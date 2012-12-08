
////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shpbase.h
//      Comment   : simple html parser base class
//					
//      Create at : 2008-09-03
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <assert.h>
#include <atlmisc.h>

//该item类 维护该类成员函数所指向的内存块，
//该类析构函数，会释放该指针所指向的内存块
//在释放内存时会检测其左右兄弟，子节点是否为空。

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete p; p = NULL;}
#endif

//shp_tree_item< shp_item_win32 >
template< class T> class shp_tree_item :public T
{
public:
	shp_tree_item( )
	:m_p_parent(NULL),
	m_p_left(NULL),
	m_p_right(NULL),
	m_p_firstchild(NULL)
	{

	};

	~shp_tree_item()
	{
		
	};
	
	//common opt
	void destory()
	{
		assert( !m_p_firstchild );
		assert( !m_p_left );
		assert( !m_p_right );
	}

	//get
	inline shp_tree_item* get_parent()
	{
		return m_p_parent;
	}

	inline shp_tree_item* get_right()
	{
		return m_p_right;
	}

	inline shp_tree_item* get_left()
	{
		return m_p_left;
	}

	inline shp_tree_item* get_firstchild()
	{
		return m_p_firstchild;
	}

	//set
	inline void set_parent( shp_tree_item * pitem)
	{
		assert( !m_p_parent );// may be leak memory
		m_p_parent = pitem;
	}

	inline void set_left( shp_tree_item *pitem )
	{
		assert( !m_p_left );
		m_p_left = pitem;
	}

	inline void set_right( shp_tree_item * pitem )
	{
		assert( !m_p_right );
		m_p_right = pitem;
	}

	inline void set_firstchild( shp_tree_item * pitem )
	{
		assert( !m_p_firstchild );
		m_p_firstchild = pitem;
	}

	inline void set_left_null( )
	{
		m_p_left = NULL;
	}

	inline void set_right_null()
	{
		m_p_right = NULL;
	}

	inline void set_parent_null()
	{
		m_p_parent = NULL;
	}

	inline void set_firstchild_null()
	{
		m_p_firstchild = NULL;
	}


protected:

	shp_tree_item* m_p_parent;
	shp_tree_item* m_p_left;
	shp_tree_item* m_p_right;
	shp_tree_item* m_p_firstchild;
};


//class T 必须继承 tree_item
//shp_tree_base< shp_tree_item<shp_item_win32> >
template< class T> class shp_tree_base
{
public:
	shp_tree_base()
	:m_p_root_item(NULL),
	m_p_cur_pos(NULL)
	{

	};

	~shp_tree_base()
	{
		clear();
	};

	T * find_item_by_id( shpstr* p_str_id )
	{
		assert( p_str_id );
		if ( m_p_root_item )
		{
			return _find_item( m_p_root_item, p_str_id );
		}
		return NULL;
	}

	T* _find_item( T* p_item, shpstr * p_str_id)
	{
		T* p_result = NULL;
		if ( p_item->get_str_id() == *p_str_id)
			return p_item;

		if ( p_item->get_firstchild() )
			p_result = _find_item( p_item->get_firstchild(), p_str_id );

		if( p_result )
			return p_result;

		if ( p_item->get_right() )
			p_result = _find_item( p_item->get_right(), p_str_id );

		return p_result;
	}

	inline T* move_left()
	{
		T* p_cur_pos = get_cur_pos();
		if ( p_cur_pos && p_cur_pos->get_left())
			m_p_cur_pos = p_cur_pos->get_left();

		return m_p_cur_pos;
	}

	inline T* move_right()
	{
		T* p_cur_pos = get_cur_pos();
		if ( p_cur_pos && p_cur_pos->get_right() )
			m_p_cur_pos = p_cur_pos->get_right();

		return m_p_cur_pos;
	}

	inline T* move_up()
	{
		T* p_cur_pos = get_cur_pos();
		if ( p_cur_pos && p_cur_pos->get_parent())
			m_p_cur_pos = p_cur_pos->get_parent();

		return m_p_cur_pos;
	}

	inline T* move_down()
	{
		T* p_cur_pos = get_cur_pos();
		if ( p_cur_pos &&  p_cur_pos->get_firstchild())
			m_p_cur_pos = p_cur_pos->get_firstchild();

		return m_p_cur_pos;
	}

	inline T* get_cur_pos()
	{
		return m_p_cur_pos;
	}

	//插入同层最右边
	//b_check 用于优化
	bool insert_right( T* item, T* left, bool b_check = true )
	{
		T* right = NULL;

		assert( item && left );
		
		if( !m_p_root_item )
		{
			_insertroot(item);
			return true;
		}

		bool b_exist = b_check ? item_exist( left ): true;

		if ( b_exist )
		{
			while( left->get_right() )
			{
				left = left->get_right();
			}

			left->set_right( item );
			item->set_left( left );
			item->set_parent( left->get_parent() );
			m_p_cur_pos = item;
			return true;
		}
		return false;
	}

	//b_check 用于优化
	bool insert_child( T* item, T* parent , bool b_check = true )
	{
		T* child = NULL;

		assert(item);

		if( !m_p_root_item )
		{
			_insertroot(item);
			return true;
		}

		bool b_exist = b_check ? item_exist( parent ): true;

		if ( b_exist )
		{
			if( parent->get_firstchild())
			{
				insert_right( item, parent->get_firstchild(), false );
			}
			else
			{
				parent->set_firstchild( item );
				item->set_parent( parent );
			}
			
			m_p_cur_pos = item;
			return true;
		}

		return false;
	}

	//检查该位置是否在树上
	bool item_exist( T* pos )
	{
		bool b_code = false;
		assert(pos && m_p_root_item);

		if( pos && m_p_root_item )
		{
			b_code = _check_item( m_p_root_item, pos );
		}

		return b_code;
	}


	
	//不允许删除 root 节点
	//暂时不实现
	bool remove_one( T* item, bool b_check = true )
	{
		bool b_exist = b_check ? item_exist( item ) : true;

		assert( item != m_p_root_item )
		
		if ( b_exist )
		{
			if ( item->get_parent() )
			{
				;
			}
			else
			{
				;
			}
		}

		return b_exist;
	}

	bool remove_tree( T* item, bool b_check = true )
	{
		bool b_exist = b_check ? item_exist( item ) : true;

		if ( b_exist )
		{
			_clearchild( item );
		}

		return b_exist;
	}

	void clear()
	{
		if ( m_p_root_item )
		{
			_clearchild( m_p_root_item );
			m_p_root_item = NULL;
		}
		
	};
	
	T* get_root_item()
	{
		return m_p_root_item;
	}

protected:

	inline void _insertroot( T* pitem )
	{
		//m_p_root_item->set_right(pitem->get_right());
		//m_p_root_item->set_firstchild(pitem->get_firstchild());
		m_p_root_item = pitem;
	}

	bool _check_item( T* tree, T* pos )
	{
		bool b_code =  false;

		if ( tree == pos)
		{
			return true;
		}

		if( pos->get_firstchild())
			b_code  = _check_item( tree->get_firstchild(), pos );

		if (b_code)
			return b_code;

		if ( pos->get_right())
			b_code = _check_item( tree->get_right(), pos );

		return b_code;
	}	

	void _clearchild( T* pitem)
	{
		if ( pitem->get_firstchild() )
		{
			_clearchild( pitem->get_firstchild() );
			pitem->set_firstchild_null();
		}

		if ( pitem->get_right())
		{
			_clearchild( pitem->get_right() );
			pitem->set_right_null();
		}

		SAFE_DELETE( pitem );
	}


	T*	m_p_root_item;
	T*	m_p_cur_pos;

};

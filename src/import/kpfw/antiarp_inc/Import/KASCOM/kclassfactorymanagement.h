///////////////////////////////////////////////////////////////
//
//	Filename: 	KClassFactoryManagement.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-8-26   10:55
//	Comment:	类厂管理器
//
//              类厂管理器用于管理Dll中所有向它注册过的类厂.
//              通常向其注册的方式为使用宏:
//                  OBJECT_ENTRY_AUTO( clsid, comImpl )
//
///////////////////////////////////////////////////////////////

#ifndef _KClassFactoryManagement_h_
#define _KClassFactoryManagement_h_
#include <vector>
#include <assert.h>


//.-----------------------------------------------------------------------------
// struct CLASS_FACTORY_ITEM, 
// 保存了组件的Class ID 和 组件对应的类厂
//.-----------------------------------------------------------------------------
struct CLASS_FACTORY_ITEM
{
	CLSID          clsid;
	IClassFactory* pIClassFactory;
};



//.-----------------------------------------------------------------------------
// class KClassFactoryManagement, 
// 类厂管理器
//.-----------------------------------------------------------------------------

class KClassFactoryManagement
{
public:
	/**
	* @brief   类厂管理器在Dll中存在唯一实例, 通过GetInstance()获取该唯一实例
	* @return  类厂管理器地址
	*/
	static KClassFactoryManagement* GetInstance();


	/**
	* @brief   向类厂管理器注册一个类厂
	* @param   [in]  rclsid,          组件class ID
	* @param   [in]  pIClassFactory,  类厂接口
	*/
	void    Add( const CLSID& rclsid, IClassFactory* pIClassFactory );


	/**
	* @brief   清除类厂管理器中所有注册的类厂 
	*/
	void    Clear();


	/**
	* @brief   获取类厂管理器中的类厂数量
	*/
	ULONG   GetCount() const;


	/**
	* @brief   调用对应的类厂, 获取组件
	* @param   [in]   rclsid, 目标组件的class id, 如果为CLSID_NULL, 则遍历所有类厂查询
	* @param   [in]   riid,   查询的接口
	* @param   [out]  ppv,    返回查询的接口
	* @return  标准HRESULT
	* @remark  当rclsid为CLSID_NULL时, 该函数会遍历所有类厂进行查询, 如果该接口在该Dll
	           中有唯一组件对其实现, 则查询成功; 如果没有实现或有多个组件对其实现, 则
			   查询失败. 多个实现下会有assert提示.
	*/
	HRESULT GetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv );


private:
	KClassFactoryManagement();
	~KClassFactoryManagement();
	IClassFactory* _Find( REFCLSID rclsid );
	std::vector<CLASS_FACTORY_ITEM>  m_vecClassFactory;
};




//.-----------------------------------------------------------------------------
// Implementation of class KClassFactoryManagement, 
//.-----------------------------------------------------------------------------


inline
KClassFactoryManagement* KClassFactoryManagement::GetInstance()
{
	static KClassFactoryManagement _Instance;
	return &_Instance;
}


inline
KClassFactoryManagement::KClassFactoryManagement()
{
}


inline
KClassFactoryManagement::~KClassFactoryManagement()
{
	if( m_vecClassFactory.size() != 0 )
	{
		Clear();
	}
}


inline
void KClassFactoryManagement::Add( const CLSID& rclsid, IClassFactory* pIClassFactory )
{
	assert( pIClassFactory != NULL );


	IClassFactory* pILast = _Find( rclsid );
	if( pILast != NULL )
	{
		assert( false );
		pILast->Release();
	}


	CLASS_FACTORY_ITEM cfItem;
	cfItem.clsid = rclsid;
	cfItem.pIClassFactory = pIClassFactory;

	m_vecClassFactory.push_back( cfItem );
}


inline
void KClassFactoryManagement::Clear()
{
	for( size_t i = 0; i < m_vecClassFactory.size(); i++ )
	{
		m_vecClassFactory[i].pIClassFactory->Release();
	}

	m_vecClassFactory.clear();
}


inline
ULONG KClassFactoryManagement::GetCount() const
{
	return (ULONG)m_vecClassFactory.size();
}


inline
HRESULT KClassFactoryManagement::GetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
	HRESULT hRet = E_NOINTERFACE;
	*ppv = NULL;

	// 如果CLSID为空, 则遍历所有类厂
	if( rclsid == CLSID_NULL )
	{
		void*   pIRet = NULL;
		HRESULT hr    = E_FAIL;

		for( size_t i = 0; i < m_vecClassFactory.size(); i++ )
		{
			hr = m_vecClassFactory[i].pIClassFactory->CreateInstance( NULL, riid, (void**)&pIRet );
			if( SUCCEEDED( hr ) )
			{
				if( *ppv == NULL )
				{
					hRet = S_OK;
					*ppv = pIRet;
				}
				else
				{
					// 该DLL中存在不只一个该接口的实现
					assert( false ); 
					((IUnknown*)pIRet)->Release();
					((IUnknown*)*ppv)->Release();
					*ppv = NULL;
					hRet = E_FAIL;
					break;
				}
			}
		}
	}

	// 如果CLSID不为空, 则直接找到对应的类厂进行创建
	else
	{
		IClassFactory* pIClassFactory = _Find( rclsid );
		if( pIClassFactory != NULL )
		{
			hRet = pIClassFactory->CreateInstance( NULL, riid, ppv );
		}
	}

	return hRet;
}



inline
IClassFactory* KClassFactoryManagement::_Find( REFCLSID rclsid )
{
	IClassFactory* pIClassFactory = NULL;

	for( size_t i = 0; i < m_vecClassFactory.size(); i++ )
	{
		if( rclsid == m_vecClassFactory[i].clsid )
		{
			pIClassFactory = m_vecClassFactory[i].pIClassFactory;
			break;
		}
	}

	return pIClassFactory;
}



#endif // _KClassFactoryManagement_h_
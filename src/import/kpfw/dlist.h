//////////////////////////////////////////////////////////////////////
///		@file		DList.h
///		@author		liuzongqiang
///		@date		2008-8-19-17:00
///
///		@brief	    用于wdm环境的map模板的链表实现
//////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _DLIST_H
#define _DLIST_H

extern unsigned long DefaultPoolTag;

template<typename K,typename T>
struct tagDList
{
#ifndef _FOR_AMD64_
	LIST_ENTRY NextNode;
#else
	LIST_ENTRY64 NextNode;
#endif
	K ulKey;
	T tValue;
};


template<typename K, typename T>
class KDList
{
public:
    typedef struct tagDList<K,T> DList;
	typedef struct tagDList<K,T>*PDList ;
	typedef struct tagDList<K,T> DNode;
	typedef PVOID iterator;
public:

	VOID  _Init();
	VOID  _Uninit();
	VOID clear();
	/*
	*@brief 如果ulKey不存在添加新项，否则修改原来的项 
	*/
	BOOLEAN insert(K ulKey, T ulPtrValue, BOOLEAN NeedLock = TRUE);
	BOOLEAN erase(K ulKey, T* tValue, BOOLEAN NeedLock = TRUE);
	BOOLEAN find(K ulKey, T* tValue, BOOLEAN NeedLock = TRUE);
	
	BOOLEAN push_back( T ulPtrValue );
	BOOLEAN pop_front( T* ulPtrValue );
	LONG size()
	{
		return m_size;
	}

	PVOID begin()
	{
		return (PVOID)(m_list.NextNode.Flink);
	}

	PVOID end()
	{
		return (PVOID)&m_list.NextNode;
	}

	PVOID next(PVOID item)
	{
		return ((PLIST_ENTRY)item)->Flink ;
	}

	void remove(PVOID item)
	{
		PLIST_ENTRY pEntry = (PLIST_ENTRY)item;
		if ( pEntry != ((PLIST_ENTRY)&m_list) )
		{
			RemoveEntryList( pEntry ) ;
			ExFreePoolWithTag(pEntry, DefaultPoolTag);
			InterlockedDecrement( &m_size );
		}
	}

	T get_data( PVOID item )
	{
		return ((DNode*)item)->tValue;
	}

	K get_key( PVOID item )
	{
		return ((DNode*)item)->ulKey;
	}

	void lock( KIRQL *pOldIrq )
	{
		KeAcquireSpinLock(&m_spinLock , pOldIrq);
	}

	void unlock( KIRQL OldIrq )
	{
		KeReleaseSpinLock( &m_spinLock , OldIrq);
	}

private:	

	VOID  *_Find(K ulKey);
	DList m_list;
	KSPIN_LOCK m_spinLock;
	LONG m_size;
};


template<typename K,typename T>
BOOLEAN KDList<K,T>::insert(K ulKey, T ulPtrValue, BOOLEAN NeedLock)
{
	BOOLEAN bResult = FALSE;
	PDList pdl = NULL ; 
	KIRQL oldirq = 0;

	if ( NeedLock )
	{
		KeAcquireSpinLock(&m_spinLock , &oldirq);
	}
	//pdl = (PDList)_Find( ulKey ) ;
	//if ( NULL != pdl)
	//{
	//	pdl->tValue = ulPtrValue ;
	//	bResult = TRUE;
	//}
	//else
	{
		pdl =(PDList) ExAllocatePoolWithTag(NonPagedPool, sizeof(DNode), DefaultPoolTag );
		if ( NULL != pdl )
		{
			InitializeListHead( &(pdl->NextNode));
			pdl->ulKey = ulKey ;
			pdl->tValue = ulPtrValue ;
			InsertTailList(&m_list.NextNode , &(pdl->NextNode));
			InterlockedIncrement( &m_size );
			bResult = TRUE;
		}
	}

	if ( NeedLock )
	{
		KeReleaseSpinLock( &m_spinLock , oldirq);
	}

	return bResult;
}	

template<typename K,typename T>
VOID  KDList<K,T>::_Init()
{
	InitializeListHead( &m_list.NextNode);
	KeInitializeSpinLock( &m_spinLock );
	m_size = 0;
}


template<typename K,typename T>
BOOLEAN KDList<K,T>::erase(K ulKey, T* tValue, BOOLEAN NeedLock)
{
	BOOLEAN bResult = FALSE;
	PLIST_ENTRY pleLoop = NULL;
	KIRQL oldirq = 0;

	if ( NeedLock )
	{
		KeAcquireSpinLock(&m_spinLock , & oldirq);
	}
	
	pleLoop =(PLIST_ENTRY) (m_list.NextNode.Flink);
	while ( pleLoop != ((PLIST_ENTRY)&m_list) && ((PDList)pleLoop)->ulKey != ulKey)
	{
		pleLoop = pleLoop->Flink ;
	}

	if ( pleLoop != ((PLIST_ENTRY)&m_list) )
	{
		if( tValue != NULL )
			*tValue = ((PDList)pleLoop)->tValue;

		RemoveEntryList( pleLoop ) ;
		ExFreePoolWithTag(pleLoop, DefaultPoolTag);
		pleLoop = NULL ;
		bResult = TRUE ;

		InterlockedDecrement( &m_size );
	}

	if ( NeedLock )
	{
		KeReleaseSpinLock( &m_spinLock , oldirq);
	}
	
	return bResult;
}


template<typename K,typename T>
BOOLEAN KDList<K,T>::find(K ulKey, T* tValue, BOOLEAN NeedLock)
{
	DNode* pNode = NULL;
	KIRQL oldirq = 0;

	if ( NeedLock )
	{
		KeAcquireSpinLock(&m_spinLock , & oldirq);
	}
	
	pNode = (DNode*) _Find( ulKey ) ;
	
	if ( NeedLock )
	{
		KeReleaseSpinLock( &m_spinLock , oldirq);
	}
	
	if( NULL == pNode )
		return FALSE;
	
	*tValue = pNode->tValue;
	return TRUE;
}

template<typename K,typename T>
VOID KDList<K,T>::clear()
{
	PLIST_ENTRY pleLoop = NULL;
	PLIST_ENTRY pleFree = NULL;
	pleLoop =(PLIST_ENTRY) (m_list.NextNode.Flink);
	while ( pleLoop != ((PLIST_ENTRY)&m_list))
	{
		pleFree = pleLoop ;
		pleLoop = pleLoop->Flink ;
		ExFreePoolWithTag(pleFree, DefaultPoolTag);
	}
	InitializeListHead( &(m_list.NextNode));
	m_size = 0;
}

template<typename K,typename T>
VOID  KDList<K,T>::_Uninit()
{
	KIRQL oldirq;
	KeAcquireSpinLock(&m_spinLock , & oldirq);
	clear();
	KeReleaseSpinLock( &m_spinLock , oldirq);
}


template<typename K,typename T>
VOID * KDList<K,T>::_Find(K ulKey)
{

	PLIST_ENTRY pleLoop = NULL;
	pleLoop =(PLIST_ENTRY)(m_list.NextNode.Flink);
	while ( (pleLoop != (PLIST_ENTRY)&m_list) && (((PDList)pleLoop)->ulKey != ulKey) )
	{
		pleLoop = pleLoop->Flink ;
	}

	return ( pleLoop != ((PLIST_ENTRY)&m_list) ) ? ((DNode*) pleLoop) : NULL ;
}

template<typename K,typename T>
BOOLEAN KDList<K,T>::push_back( T ulPtrValue )
{
	PDList pdl =(PDList) ExAllocatePoolWithTag(NonPagedPool  , sizeof(DNode), DefaultPoolTag );
	if( pdl == NULL )
		return FALSE;

	pdl->tValue = ulPtrValue;
	
	ExInterlockedInsertTailList( &m_list.NextNode , &(pdl->NextNode), &m_spinLock );
	InterlockedIncrement( &m_size );
	return TRUE;
}


template<typename K,typename T>
BOOLEAN KDList<K,T>::pop_front( T* ulPtrValue )
{
	PDList pdl = NULL;

	if( ulPtrValue == NULL )
		return FALSE;
	
	pdl = (PDList)ExInterlockedRemoveHeadList( (PLIST_ENTRY)&m_list.NextNode, &m_spinLock );
	if( pdl == NULL )
		return FALSE;

	*ulPtrValue = pdl->tValue;
	ExFreePoolWithTag( (PVOID)pdl, DefaultPoolTag );
	InterlockedDecrement( &m_size );
	return TRUE;
}

#endif
//////////////////////////////////////////////////////////////////////
///		@file		WDMMap.h
///		@author		liuzongqiang
///		@date		2008-10-31
///
///		@brief	    用于wdm环境的map模板的哈希表实现
//////////////////////////////////////////////////////////////////////



#ifndef _WDMMAP_H
#define _WDMMAP_H
#include "wdm.h"

#pragma warning( push ) 
#pragma warning( disable : 4127 )
#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )


#define CALC_HASH(fileobj)  (((ULONG)(fileobj) >> 5) % HASH_SIZE)
#pragma pack(push , 1)
template< typename T, typename KeyType = PVOID>
struct _Hash_Node
{
	_Hash_Node * next;
	KeyType ulKey;
	T tValue;
};
#pragma pack(pop)

const ULONG DefaultWDMPoolTag = 'KSFW';
template< typename T , BOOLEAN LOCK = TRUE ,int HASH_SIZE = 1000 , typename KeyType = PVOID>
class KWDMMap
{
public:
	typedef struct _Hash_Node<T, KeyType> Hash_Node;
	typedef struct _Hash_Node<T, KeyType>* PHash_Node;
	struct Iterator
	{
		Iterator(ULONG _index, PHash_Node _Node):index(_index), Node(_Node)
		{

		}
		BOOLEAN operator !=( const Iterator & i)
		{
			return (i.index != index) || (i.Node != Node);
		}
		Iterator()
		{
			index = 0;
			Node = NULL;
		}

		ULONG index;
		PHash_Node Node;
	};

public:
	//KWDMMap(){Initialize();}
	//~KWDMMap(){UnInitialize();}
	/*
	*@brief 如果ulKey不存在添加新项，否则修改原来的项 
	*/

	BOOLEAN insert(KeyType ulKey, T ulPtrValue)
	{
		KIRQL oldirq;
		PHash_Node pEntry = NULL;
		ULONG index = 0;
		BOOLEAN bRet = TRUE;

		index = CALC_HASH((ULONG)ulKey);
		if (LOCK)
		KeAcquireSpinLock(&m_spinLock , & oldirq);
		//pEntry = m_pTable[index];
		//while ( NULL != pEntry)
		//{
		//	if ( pEntry->ulKey == ulKey)
		//	{
		//		pEntry->tValue = ulPtrValue;
		//		goto EXIT0;
		//	}
		//	pEntry = pEntry->next;
		//}

		pEntry =(PHash_Node) _AllocatePool();
		if ( NULL == pEntry)
		{
			bRet = FALSE;
			goto EXIT0;
		}
		pEntry->tValue = ulPtrValue;
		pEntry->ulKey = ulKey;

		pEntry->next = m_pTable[index];
		m_pTable[index] = pEntry;

		m_size ++;
EXIT0:	
		if (LOCK)
		KeReleaseSpinLock( &m_spinLock , oldirq);
		return bRet;
	}	
	

	BOOLEAN erase_nolock(KeyType ulKey, T* tValue)
	{
		PHash_Node pEntry = NULL;
		PHash_Node pPrevious = NULL;
		BOOLEAN bFind = FALSE;
		ULONG index = CALC_HASH(ulKey);
		pEntry = m_pTable[index];

		while ( NULL != pEntry )
		{
			if ( pEntry->ulKey == ulKey)
			{
				if ( NULL != tValue)
				{
					*tValue = pEntry->tValue;
				}
				bFind = TRUE;
				break;
			}	
			pPrevious = pEntry;
			pEntry = pEntry->next;
		}

		if ( bFind)
		{
			//delete
			if ( NULL == pPrevious )//Find the first node?
			{
				m_pTable[index] = pEntry->next;
			}
			else
			{
				pPrevious->next = pEntry->next;
			}
			_FreePool(pEntry);
			//delete end;
			pEntry = NULL;
			m_size --;
		}
		return bFind;
	}

	BOOLEAN erase(KeyType ulKey, T* tValue)
	{
		KIRQL oldirq;
		BOOLEAN bFind = FALSE;

		if (LOCK)
		KeAcquireSpinLock(&m_spinLock , & oldirq);
		bFind = erase_nolock( ulKey, tValue);
		if (LOCK)
		KeReleaseSpinLock( &m_spinLock , oldirq);
		return bFind;
	}

	BOOLEAN find(KeyType ulKey ,  T* tValue)
	{
		PHash_Node pEntry = NULL;
		if ( NULL == tValue)
		{
			return FALSE;
		}
		pEntry = m_pTable[CALC_HASH(ulKey)];

		while ( NULL != pEntry )
		{
			if ( pEntry->ulKey == ulKey)
			{
				*tValue =pEntry->tValue;
				return TRUE;
			}
			pEntry = pEntry->next;
		}
		return FALSE;
	}

	BOOLEAN find_lock(KeyType ulKey ,  T* tValue)
	{
		KIRQL oldirq;
		BOOLEAN bFind = FALSE;
		KeAcquireSpinLock(&m_spinLock , & oldirq);
		bFind = find( ulKey, tValue);
		KeReleaseSpinLock( &m_spinLock , oldirq);
		return bFind;
	}

	//Iterator find(ULONG ulKey)
	//{
	//	for (Iterator i = begin() ; i != end() ; i = next(i))
	//	{
	//		if ( ulKey == get_key(i))
	//		{
	//			return i;
	//		}
	//	}
	//	return end();
	//}

	BOOLEAN clear( BOOLEAN bFreeValue = FALSE )
	{
		KIRQL oldirq;
		PHash_Node pEntry = NULL;
		PHash_Node pFree = NULL;

		if (LOCK)
		KeAcquireSpinLock( &m_spinLock , &oldirq );
		for ( int i = 0 ; i < HASH_SIZE ; i++ )
		{
			pEntry = m_pTable[i];
			while ( NULL != pEntry )
			{			
				pFree = pEntry;
				pEntry = pEntry->next;
				if( bFreeValue )
				{
					ExFreePoolWithTag( (PVOID)pFree->tValue, DefaultWDMPoolTag );
					pFree->tValue = NULL;
				}

				_FreePool( pFree);
			}
			m_pTable[i] = NULL;
		}
		m_size = 0;
		if (LOCK)
		KeReleaseSpinLock( &m_spinLock , oldirq);
		return TRUE;
	}

	inline ULONG size() 
	{ 
		return m_size ;
	}

	BOOLEAN pop_front(T* pValue)
	{
		BOOLEAN bResult = FALSE;
		KIRQL oldirq;
		if (LOCK)
			KeAcquireSpinLock( &m_spinLock , &oldirq );

		for(int i = 0 ; i < HASH_SIZE ; i ++)
		{
			if (NULL != m_pTable[i])
			{
				PHash_Node pEntry = m_pTable[i];
				m_pTable[i] = pEntry->next;
				*pValue = pEntry->tValue;
				bResult = TRUE;

				_FreePool( pEntry);
			}
		}

		if (LOCK)
			KeReleaseSpinLock( &m_spinLock , oldirq);
		return bResult;
	}

	ULONG	GetAllDataNoLock(T* pData, ULONG nCnt)
	{
		ULONG nCntResult = 0;

		for(int i = 0 ; (i < HASH_SIZE) && (nCntResult <= nCnt) ; i ++)
		{
			if (NULL != m_pTable[i])
			{
				PHash_Node pEntry = m_pTable[i];
				while (pEntry && nCntResult <= nCnt)
				{
					pData[nCntResult++] = pEntry->tValue;
					pEntry = pEntry->next;
				}
			}
		}

		return nCntResult;	
	}

	Iterator begin()
	{
		for(int i = 0 ; i < HASH_SIZE ; i ++)
		{
			if (NULL != m_pTable[i])
			{
				return Iterator( i, m_pTable[i] );
			}
		}
		return end();
	}

	inline Iterator end()
	{
		return Iterator( HASH_SIZE - 1, NULL );
	}

	Iterator next( Iterator pos )
	{
		PHash_Node pHash = pos.Node;
		if (NULL != pHash->next)
		{
			return Iterator( pos.index, pHash->next );
		}
		
		for ( int i = pos.index + 1; i < HASH_SIZE; i++ )
		{
			if ( NULL != m_pTable[i] )
				return Iterator( i, m_pTable[i] );
		}

		return end();
	}

	void lock( KIRQL *pOldIrq )
	{
		KeAcquireSpinLock(&m_spinLock , pOldIrq);
	}

	T get_data( Iterator item )
	{
		return item.Node->tValue;
	}

	ULONG get_key( Iterator item)
	{
		return ((PHash_Node)item)->ulKey ;
	}

	void unlock( KIRQL OldIrq )
	{
		KeReleaseSpinLock( &m_spinLock , OldIrq);
	}

public:	

	BOOLEAN _Init(ULONG ulPoolTag = DefaultWDMPoolTag)
	{			
		RtlZeroMemory(m_pTable , sizeof(Hash_Node*)*HASH_SIZE);
		KeInitializeSpinLock( &m_spinLock );
		ExInitializeNPagedLookasideList(
			&m_pageList ,
			NULL ,
			NULL ,
			0 ,
			sizeof(Hash_Node),
			ulPoolTag ,
			0
			);
		m_size = 0;
		return TRUE;
	}

	VOID  _Uninit()
	{
		clear();
		ExDeleteNPagedLookasideList(&m_pageList);
	} 

private:
	inline PVOID _AllocatePool()
	{
		return ExAllocateFromNPagedLookasideList(&m_pageList);
	}
	inline VOID _FreePool( PVOID pBuffer)
	{
		return ExFreeToNPagedLookasideList( &m_pageList , pBuffer);
	}
protected:
	PHash_Node m_pTable[HASH_SIZE];
	ULONG m_size ;
	KSPIN_LOCK m_spinLock;
	NPAGED_LOOKASIDE_LIST m_pageList;
};

#pragma warning( pop ) 
#endif
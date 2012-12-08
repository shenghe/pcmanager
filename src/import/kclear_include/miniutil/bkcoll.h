/**
* @file    bkcoll.h
* @brief   ...
* @author  zhangrui
* @date    2009-06-11  00:04
*/

#ifndef BKCOLL_H
#define BKCOLL_H

#include <new.h>
#include <atlcoll.h>

template< typename E, class ETraits = CElementTraits< E > >
class CBkDeque
{
public:
	typedef typename ETraits::INARGTYPE INARGTYPE;

    class CBlock
    {
    public:
        CBlock( UINT nBlockSize );
        ~CBlock();

        bool    Alloc();
        void    Free();
        void    ZeroBlock();

        bool    IsEmpty() const;
        size_t  GetCount() const;
        bool    SetCount(size_t nNewSize);
        size_t  SetCountToMax();
        size_t  GetCapacity() const;
        void    RemoveAll();

        void    AssertValid() const;

        const E& GetAt(size_t iElement) const;
        E&       GetAt(size_t iElement);
        void     SetAt(size_t iElement, INARGTYPE element);

        void    RemoveHeadNoReturn();

        void    AddTail();
        void    AddTail( INARGTYPE element );

    private:

        UINT        m_iBegin;
        UINT        m_iEnd;
        E*          m_pBlockData;
        const UINT  m_nBlockSize;
    };


    typedef CAtlArray<CBlock*>  CBlockArray;

public:
    CBkDeque( UINT nBlockSize = 64 );
    ~CBkDeque();

    size_t      GetCount() const;
    bool        IsEmpty() const;
    void        SetCount( size_t nNewSize );
    void        RemoveAll();

    const E&    GetAt( size_t iElement ) const;
    E&          GetAt( size_t iElement );
    void        SetAt( size_t iElement, INARGTYPE element );

    const E&    operator[]( size_t iElement ) const;
    E&          operator[]( size_t iElement );


    E&          GetHead();
    const E&    GetHead() const;
    E&          GetTail();
    const E&    GetTail() const;

    void        RemoveHeadNoReturn();
    void        RemoveTailNoReturn();

    void        AddTail();
    void        AddTail( INARGTYPE element );

// Implementation
private:

    CBlockArray     m_blockArray;
    const UINT      m_nBlockSize;

private:
    void CalcOffset(size_t iElement, size_t& iBlock, size_t& iElemInBlock);
    void CalcOffsetWithoutCheck(size_t iElement, size_t& iBlock, size_t& iElemInBlock);

    CBlock* CreateNewBlock();

private:
	// Private to prevent use
	CBkDeque( const CBkDeque& );
	CBkDeque& operator=( const CBkDeque& );
};




template< typename E, class ETraits >
inline bool CBkDeque< E, ETraits >::CBlock::Alloc()
{
    Free();
    m_pBlockData = (E*)calloc( m_nBlockSize, sizeof(E) );
    if ( !m_nBlockSize )
        return false;

    ZeroBlock();
    return true;
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::Free()
{
    if (m_pBlockData)
    {
        RemoveAll();
        free(m_pBlockData);
        m_pBlockData = NULL;
    }

    m_iBegin = 0;
    m_iEnd   = 0;
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::ZeroBlock()
{
    ATLASSERT( m_pBlockData );
    m_iBegin = 0;
    m_iEnd   = 0;
}



template< typename E, class ETraits >
inline CBkDeque< E, ETraits >::CBlock::CBlock( UINT nBlockSize = 64 ):
    m_iBegin(0),
    m_iEnd(0),
    m_pBlockData(NULL),
    m_nBlockSize(nBlockSize)
{
}

template< typename E, class ETraits >
inline CBkDeque< E, ETraits >::CBlock::~CBlock()
{
    Free();
}

template< typename E, class ETraits >
inline bool CBkDeque< E, ETraits >::CBlock::IsEmpty() const
{
    AssertValid();
    return GetCount() == 0;
}

template< typename E, class ETraits >
inline size_t CBkDeque< E, ETraits >::CBlock::GetCount() const
{
    AssertValid();
    size_t nCount = m_iEnd - m_iBegin;
    return nCount;
}




#pragma push_macro("new")
#pragma push_macro("delete")
#undef new

template< typename E, class ETraits >
inline bool CBkDeque< E, ETraits >::CBlock::SetCount(size_t nNewSize) 
{
    AssertValid();
    ATLASSERT( nNewSize <= GetCapacity() );

    if ( nNewSize == GetCount() )
    {
        return true;
    }
    else if ( nNewSize > GetCount() )
    {
        UINT iNewEnd = m_iBegin + nNewSize;
        for ( NULL; m_iEnd < iNewEnd; ++m_iEnd )
        {
            ::new (m_pBlockData + m_iEnd) E();
        }
    }
    else // if (nNewSize < GetCount() )
    {
        UINT iNewEnd = m_iBegin + nNewSize;
        for ( NULL; m_iEnd > iNewEnd; --m_iEnd )
        {
            m_pBlockData[m_iEnd - 1].~E();
        }
    }

    return true;
}

#pragma pop_macro("delete")
#pragma pop_macro("new")


template< typename E, class ETraits >
inline size_t CBkDeque< E, ETraits >::CBlock::SetCountToMax()
{
    SetCount( GetCapacity() );
    return GetCount();
}


template< typename E, class ETraits >
inline size_t CBkDeque< E, ETraits >::CBlock::GetCapacity() const
{
    AssertValid();
    size_t nCapacity = m_nBlockSize - (UINT)m_iBegin;
    return nCapacity;
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::RemoveAll()
{
    SetCount(0);
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::AssertValid() const
{
    ATLASSERT ( m_pBlockData );
    ATLASSERT ( m_iBegin <= m_iEnd );
    ATLASSERT ( m_iEnd <= m_nBlockSize );
}


template< typename E, class ETraits >
inline const E& CBkDeque< E, ETraits >::CBlock::GetAt(size_t iElement) const
{
    AssertValid();
    ATLASSERT( iElement < GetCount() );

    return m_pBlockData[m_iBegin + iElement];
}

template< typename E, class ETraits >
inline E& CBkDeque< E, ETraits >::CBlock::GetAt(size_t iElement)
{
    AssertValid();
    ATLASSERT( iElement < GetCount() );

    return m_pBlockData[m_iBegin + iElement];
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::SetAt(size_t iElement, INARGTYPE element)
{
    AssertValid();
    ATLASSERT( iElement < GetCount() );

    m_pBlockData[m_iBegin + iElement] = element;
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::RemoveHeadNoReturn()
{
    AssertValid();
    ATLASSERT( GetCount() > 0 );

    m_pBlockData[m_iBegin].~E();
    ++m_iBegin;
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::AddTail()
{
    AssertValid();
    ATLASSERT( GetCount() < m_nBlockSize );

    ::new (m_pBlockData + m_iEnd) E();
    ++m_iEnd;
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::CBlock::AddTail( typename CBkDeque< E, ETraits >::INARGTYPE element )
{
    AssertValid();
    ATLASSERT( GetCount() < m_nBlockSize );

    ::new (m_pBlockData + m_iEnd) E(element);
    ++m_iEnd;
}











template< typename E, class ETraits >
CBkDeque< E, ETraits >::CBkDeque(UINT nBlockSize) :
	m_nBlockSize( nBlockSize )
{
    ATLASSERT( m_nBlockSize > 0 );
}

template< typename E, class ETraits >
CBkDeque< E, ETraits >::~CBkDeque()
{
	RemoveAll();
}



template< typename E, class ETraits >
inline size_t CBkDeque< E, ETraits >::GetCount() const
{
    size_t nCount = 0;
    size_t nBlockCount = m_blockArray.GetCount();

	if ( nBlockCount >= 1 )
    {   // first block
        nCount += m_blockArray[0]->GetCount();
    }

    if ( nBlockCount >= 2)
    {   // last block
        nCount += m_blockArray[m_blockArray.GetCount() - 1]->GetCount();
    }

    if ( nBlockCount >= 3)
    {   // middle block
        nCount += ( nBlockCount - 2 ) * m_nBlockSize;
    }

    return nCount;
}

template< typename E, class ETraits >
inline bool CBkDeque< E, ETraits >::IsEmpty() const
{
	return( GetCount() == 0 );
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::RemoveAll()
{
	SetCount( 0 );
}

template< typename E, class ETraits >
void CBkDeque< E, ETraits >::SetCount( size_t nNewSize)
{
    size_t nOldCount = GetCount();


    if ( 0 == nNewSize )
    {
        for ( size_t i = 0; i < m_blockArray.GetCount(); ++i )
        {
            ATLASSERT(m_blockArray[i]);
            m_blockArray[i]->Free();
            m_blockArray[i] = NULL;
        }

        m_blockArray.RemoveAll();
    }
    else if ( nNewSize == nOldCount )
    {
        NULL;
    }
    else if ( nNewSize > nOldCount )
    {
        size_t iLastBlock       = 0;
        size_t iLastElemInBlock = 0;
        CalcOffsetWithoutCheck(nNewSize - 1, iLastBlock, iLastElemInBlock);
        if ( !m_blockArray.SetCount(m_blockArray.GetCount(), (int)nNewSize)) // reserved nNewSize
            AtlThrow( E_OUTOFMEMORY );;


        // before last block
        size_t nLeftSize = nNewSize;
        for ( size_t i = 0; i <= iLastBlock; ++i )
        {
            CBlock* pBlock = NULL;
            if (i >= m_blockArray.GetCount() || !m_blockArray[i])
            {   // allocate new block
                CBlock* pNewBlock = new CBlock(m_nBlockSize);
                if (!pNewBlock)
                    AtlThrow( E_OUTOFMEMORY );;

                if (!pNewBlock->Alloc())
                    AtlThrow( E_OUTOFMEMORY );;

                m_blockArray.SetAtGrow(i, pNewBlock);
            }

            ATLASSERT( i < m_blockArray.GetCount() );
            pBlock = m_blockArray[i];
            ATLASSERT( pBlock );

            // set count of block;
            size_t nGrow = min( nLeftSize, pBlock->GetCapacity() );
            pBlock->SetCount(nGrow);
            ATLASSERT(nLeftSize >= pBlock->GetCount());
            nLeftSize -= pBlock->GetCount();
        }
    }
    else // if ( nNewSize < nOldCount )
    {
        size_t iLastBlock       = 0;
        size_t iLastElemInBlock = 0;
        CalcOffsetWithoutCheck(nNewSize - 1, iLastBlock, iLastElemInBlock);

        for ( size_t i = iLastBlock + 1; i < m_blockArray.GetCount(); ++i )
        {   // after last block
            ATLASSERT( m_blockArray[i] );
            m_blockArray[i]->Free();
        }
        m_blockArray.SetCount(iLastBlock + 1);
        

        // last block;
        ATLASSERT( m_blockArray[iLastBlock] );
        m_blockArray[iLastBlock]->SetCount(iLastElemInBlock + 1);
    }
}






template< typename E, class ETraits >
inline const E& CBkDeque< E, ETraits >::GetAt( size_t iElement ) const
{
    size_t iBlock       = 0;
    size_t iElemInBlock = 0;
    CalcOffset( iElement, iBlock, iElemInBlock );

	return( m_blockArray[iBlock]->GetAt( iElemInBlock ) );
}


template< typename E, class ETraits >
inline E& CBkDeque< E, ETraits >::GetAt( size_t iElement )
{
    size_t iBlock       = 0;
    size_t iElemInBlock = 0;
    CalcOffset( iElement, iBlock, iElemInBlock );

    return( m_blockArray[iBlock]->GetAt( iElemInBlock ) );
}



template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::SetAt( size_t iElement, INARGTYPE element )
{
    GetAt( iElement ) = element;
}

template< typename E, class ETraits >
inline const E& CBkDeque< E, ETraits >::operator[]( size_t iElement ) const
{
	return GetAt(iElement);
}

template< typename E, class ETraits >
inline E& CBkDeque< E, ETraits >::operator[]( size_t iElement ) 
{
	return GetAt(iElement);
}





template< typename E, class ETraits >
inline E& CBkDeque< E, ETraits >::GetHead()
{
    ATLASSERT( !IsEmpty() );
    return GetAt( 0 );
}

template< typename E, class ETraits >
inline const E& CBkDeque< E, ETraits >::GetHead() const
{
    ATLASSERT( !IsEmpty() );
    return GetAt( 0 );
}

template< typename E, class ETraits >
inline E& CBkDeque< E, ETraits >::GetTail()
{
    ATLASSERT( !IsEmpty() );
    return GetAt( GetCount() - 1 );
}

template< typename E, class ETraits >
inline const E& CBkDeque< E, ETraits >::GetTail() const
{
    ATLASSERT( !IsEmpty() );
    return GetAt( GetCount() - 1 );
}



template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::RemoveHeadNoReturn()
{
    ATLASSERT( !IsEmpty() );

    ATLASSERT( m_blockArray[0] );
    ATLASSERT( !m_blockArray[0]->IsEmpty() );

    m_blockArray[0]->RemoveHeadNoReturn();
    if ( 0 == m_blockArray[0]->GetCapacity() )
    {
        m_blockArray.RemoveAt(0);
    }
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::RemoveTailNoReturn()
{
    ATLASSERT( !IsEmpty() );
    SetCount( GetCount() - 1 );
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::AddTail()
{
    if ( IsEmpty() || m_nBlockSize == m_blockArray[m_blockArray.GetCount() - 1]->GetCount())
    {
        CBlock* pNewBlock = new CBlock(m_nBlockSize);
        if (!pNewBlock)
            AtlThrow( E_OUTOFMEMORY );;

        if (!pNewBlock->Alloc())
            AtlThrow( E_OUTOFMEMORY );;

        m_blockArray.Add(pNewBlock);
    }

    CBlock* pLastBlock = m_blockArray[m_blockArray.GetCount() - 1];
    pLastBlock->AddTail();
}

template< typename E, class ETraits >
inline void CBkDeque< E, ETraits >::AddTail( INARGTYPE element )
{
    if ( IsEmpty() || m_nBlockSize == m_blockArray[m_blockArray.GetCount() - 1]->GetCount())
    {
        CBlock* pNewBlock = new CBlock(m_nBlockSize);
        if (!pNewBlock)
            AtlThrow( E_OUTOFMEMORY );;

        if (!pNewBlock->Alloc())
            AtlThrow( E_OUTOFMEMORY );;

        m_blockArray.Add(pNewBlock);
    }

    CBlock* pLastBlock = m_blockArray[m_blockArray.GetCount() - 1];
    pLastBlock->AddTail(element);
}











template< typename E, class ETraits >
void CBkDeque< E, ETraits >::CalcOffset(size_t iElement, size_t& iBlock, size_t& iElemInBlock)
{
    ATLASSERT( iElement < GetCount() );
    ATLASSERT( !m_blockArray.IsEmpty() );

    CalcOffsetWithoutCheck(iElement, iBlock, iElemInBlock);

    ATLASSERT( iBlock < m_blockArray.GetCount() );
    ATLASSERT( m_blockArray[iBlock] );
    ATLASSERT( iElemInBlock < m_blockArray[iBlock]->GetCount() );
}

template< typename E, class ETraits >
void CBkDeque< E, ETraits >::CalcOffsetWithoutCheck(size_t iElement, size_t& iBlock, size_t& iElemInBlock)
{
    if ( m_blockArray.IsEmpty() )
    {   // empty
        iBlock       = iElement / m_nBlockSize;
        iElemInBlock = iElemInBlock % m_nBlockSize;
        return;
    }

    ATLASSERT( m_blockArray[0] );
    if ( iElement < m_blockArray[0]->GetCount() )
    {   // in first block
        iBlock       = 0;
        iElemInBlock = iElement;
        return;
    }

    // in another block
    iElement    -= m_blockArray[0]->GetCount();
    iBlock       = iElement / m_nBlockSize + 1;
    iElemInBlock = iElement % m_nBlockSize;
}


#endif//BKCOLL_H
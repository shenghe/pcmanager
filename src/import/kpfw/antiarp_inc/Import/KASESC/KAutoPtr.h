/********************************************************************
* CreatedOn: 2007-8-24   17:59
* FileName:  KAutoPtr.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#ifndef __KAS_KAUTOPTR_H__
#define __KAS_KAUTOPTR_H__


template< typename Type >
class KAutoPtr
{
public:
    KAutoPtr() throw() :
        m_pPtr( NULL )
    {
    }

    template< typename TSrc >
    KAutoPtr( KAutoPtr< TSrc >& p ) throw()
    {
        m_pPtr = p.Detach();
    }

    KAutoPtr( KAutoPtr< Type >& p ) throw()
    {
        m_pPtr = p.Detach();
    }

    explicit KAutoPtr( Type* p ) throw() :
        m_pPtr( p )
    {
    }

    ~KAutoPtr() throw()
    {
        Free();
    }

    template< typename TSrc >
    KAutoPtr< Type >& operator=( KAutoPtr< TSrc >& p ) throw()
    {
        if(m_pPtr == p.m_pPtr)
        {
            assert(FALSE);
        }
        else
        {
            Free();
            Attach( p.Detach() );  
        }
        return( *this );
    }

    KAutoPtr<Type >& operator=( KAutoPtr< Type >& p ) throw()
    {
        if(*this == p )
        {
            if(this != &p)
            {
                assert(FALSE);
                p.Detach();
            }
            else
            {
            }
        }
        else
        {
            Free();
            Attach( p.Detach() );
        }

        return( *this );
    }

    bool operator!=(KAutoPtr<Type>& p) const
    {
        return !operator==(p);
    }

    bool operator==(KAutoPtr<Type>& p) const
    {
        return m_pPtr == p.m_pPtr;
    }

    operator Type*() const throw()
    {
        return( m_pPtr );
    }
    Type* operator->() const throw()
    {
        ATLASSUME( m_pPtr != NULL );
        return( m_pPtr );
    }

    void Attach( Type* p ) throw()
    {
        ATLASSUME( m_pPtr == NULL );
        m_pPtr = p;
    }

    Type* Detach() throw()
    {
        Type* p;

        p = m_pPtr;

        m_pPtr = NULL;

        return( p );
    }

    void Free() throw()
    {
        delete m_pPtr;

        m_pPtr = NULL;
    }

public:
    Type * m_pPtr;
};


#endif
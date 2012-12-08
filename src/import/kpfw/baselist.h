/*

Copyright (C) 2007-2008 Kingsoft Corporation

Creator: 
    
    Leo Jiang <JiangFengbing@kingsoft.com>

Module:

    baselist.h

Abstract:

    Privide a advanced encapsulation for LIST_ENTRY.

Environment:

    Kernel driver and application

*/

#ifndef BASELIST_INC_
#define BASELIST_INC_


//////////////////////////////////////////////////////////////////////////
template<typename T>
struct BaseListCompare
{
    virtual bool Equal(const T& a, const T&b) = 0;
};

//////////////////////////////////////////////////////////////////////////

template<typename T>
struct BaseFinder
{
    virtual bool operator()(const T& t) = 0;
};

template<typename T>
class BaseList
{
public:
    typedef struct _REAL_ENTRY {

        LIST_ENTRY  entry;
        T           obj;

    } REAL_ENTRY, *PREAL_ENTRY;

    BaseList() throw()
    {
        InitializeListHead(&listentry);
    }

    ~BaseList() throw()
    {
        Clean();
    }

    int Find(BaseFinder<T>* pFinder) throw()
    {
        int retval = false;
        PLIST_ENTRY next = NULL;
        PREAL_ENTRY realentry = NULL;

        if (!pFinder)
            goto clean0;

        next = listentry.Flink;
        while (next != &listentry)
        {
            realentry = (PREAL_ENTRY)next;
            next = next->Flink;

            if ((*pFinder)(realentry->obj))
            {
                retval = true;
                break;
            }
        }

clean0:
        return retval;
    }

    int InsertHead(const T& entry) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = new REAL_ENTRY;
        if (!realentry)
            goto clean0;

        RtlMoveMemory(&realentry->obj, &entry, sizeof(T));
        InsertHeadList(&listentry, (PLIST_ENTRY)realentry);

        retval = true;
clean0:
        return retval;
    }

    int InsertTail(const T& entry) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = new REAL_ENTRY;
        if (!realentry)
            goto clean0;

        RtlMoveMemory(&realentry->obj, &entry, sizeof(T));
        InsertTailList(&listentry, (PLIST_ENTRY)realentry);

        retval = true;
clean0:
        return retval;
    }

    void RemoveHead() throw()
    {
        PREAL_ENTRY realentry = NULL;

        if (IsListEmpty(listentry))
            return;

        realentry = (PREAL_ENTRY)RemoveHeadList(&listentry);
        if (realentry != (PREAL_ENTRY)&listentry)
        {
            delete realentry;
            realentry = NULL;
        }
    }

    int RemoveHead(T& entry) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = NULL;

        if (IsListEmpty(listentry))
            return;

        realentry = (PREAL_ENTRY)RemoveHeadList(&listentry);
        if (realentry != (PREAL_ENTRY)&listentry)
        {
            RtlMoveMemory(&entry, &realentry->obj, sizeof(T));
            delete realentry;
            realentry = NULL;

            retval = true;
        }

        return retval;
    }

    void RemoveTail() throw()
    {
        PREAL_ENTRY realentry = NULL;
        
        if (IsListEmpty(&listentry))
            return;

        realentry = (PREAL_ENTRY)RemoveTailList(&listentry);
        if (realentry != (PREAL_ENTRY)&listentry)
        {
            delete realentry;
            realentry = NULL;
        }
    }

    int RemoveTail(T& entry) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = NULL;

        if (IsListEmpty(&listentry))
            return false;

        realentry = (PREAL_ENTRY)RemoveTailList(&listentry);
        if (realentry != (PREAL_ENTRY)&listentry)
        {
            RtlMoveMemory(&entry, &realentry->obj, sizeof(T));

            delete realentry;
            realentry = NULL;

            retval = true;
        }

        return false;
    }

    int IsEmpty() throw()
    {
        return IsListEmpty(&listentry);
    }

    int Remove(const T& v) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = NULL;
        PLIST_ENTRY next = NULL;
        PLIST_ENTRY temp = NULL;

        next = listentry.Flink;
        while (next != &listentry)
        {
            realentry = (PREAL_ENTRY)next;
            next = next->Flink;

            if (v == realentry->obj)
            {
                temp = (PLIST_ENTRY)realentry;
                RemoveEntryList((PLIST_ENTRY)realentry);
                delete temp;

                retval = true;
                break;
            }
        }

        return retval;
    }

    int Find(const T& v) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = NULL;
        PLIST_ENTRY next = NULL;

        next = listentry.Flink;
        while (next != &listentry)
        {
            realentry = (PREAL_ENTRY)next;
            next = next->Flink;

            if (v == realentry->obj)
            {
                retval = true;
                break;
            }
        }

        return retval;
    }

    int Find(T& v, BaseListCompare<T>* cmp) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = NULL;
        PLIST_ENTRY next = NULL;

        next = listentry.Flink;
        while (next != &listentry)
        {
            realentry = (PREAL_ENTRY)next;
            next = next->Flink;

            if (cmp->Equal(v, realentry->obj))
            {
                v = realentry->obj;
                retval = true;
                break;
            }
        }

        return retval;
    }

    int Remove(T& v, BaseListCompare<T>* cmp) throw()
    {
        int retval = false;
        PREAL_ENTRY realentry = NULL;
        PLIST_ENTRY next = NULL;
        PLIST_ENTRY temp = NULL;

        next = listentry.Flink;
        while (next != &listentry)
        {
            realentry = (PREAL_ENTRY)next;
            next = next->Flink;

            if (cmp->Equal(v, realentry->obj))
            {
                v = realentry->obj;
                temp = (PLIST_ENTRY)realentry;
                RemoveEntryList((PLIST_ENTRY)realentry);
                delete temp;
                temp = NULL;

                retval = true;
                break;
            }
        }

        return retval;
    }

    void Clean() throw()
    {
        PREAL_ENTRY realentry = NULL;
        PLIST_ENTRY next = NULL;

        next = listentry.Flink;
        while (next != &listentry)
        {
            realentry = (PREAL_ENTRY)next;
            next = next->Flink;

            delete realentry;
            realentry = NULL;
        }

        InitializeListHead(&listentry);
    }


private:
    LIST_ENTRY listentry;
};

//////////////////////////////////////////////////////////////////////////

#endif  // !BASELIST_INC_


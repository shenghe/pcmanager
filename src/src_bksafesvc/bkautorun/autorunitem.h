/**
* @file    autorunitem.h
* @brief   ...
* @author  zhangrui
* @date    2009-04-26  17:31
*/

#ifndef AUTORUNITEM_H
#define AUTORUNITEM_H

#include <assert.h>
#include <atlcoll.h>
#include "skylark2\skylarkbase.h"
#include "skylark2\slcoll.h"
#include "skylark2\slatruns.h"
#include "autorunentry.h"



class CAutorunContext;
class CAutorunItem_RegKey_Run;

/// 启动项条目的基类
class CAutorunItem
{
public:

    typedef CAtlList<CAutorunItem*>     CItemList;
    typedef Skylark::CCopiableArray<CString>     CPathList;

    CAutorunItem();
    virtual ~CAutorunItem() {}

    static DWORD GetEntryTypeByID(DWORD dwEntryID);

    DWORD   GetEntryID() const;
    DWORD   GetEntryType() const;
    LPCWSTR GetInformation() const;


    virtual DWORD   GetFileCount() const = 0;
    virtual LPCWSTR GetFileAt(DWORD dwIndex) const = 0;
    virtual HRESULT Clone(CAutorunItem** ppItem) const = 0;

	virtual HRESULT Clean() const = 0;



    //virtual HRESULT Scan(CAutorunContext* pContext) = 0;
    //virtual HRESULT Disable(CAutorunContext* pContext) = 0;
    //virtual HRESULT Fix(CAutorunContext* pContext) = 0;


    const SLATR_ENTRY_DATA* m_pEntryData;
    CString                 m_strInformation;

public:


    template <class T>
    const T* CastTo() const;

    const CAutorunItem_RegKey_Run* CastTo_RegKey_Run() const;
};





inline CAutorunItem::CAutorunItem():
    m_pEntryData(NULL)
{
}

inline DWORD CAutorunItem::GetEntryTypeByID(DWORD dwEntryID)
{
    return SLATR_Group_Mask & dwEntryID;
}

inline DWORD CAutorunItem::GetEntryID() const
{
    assert(m_pEntryData);
    return m_pEntryData->m_dwEntryID;
}

inline DWORD CAutorunItem::GetEntryType() const
{
    assert(m_pEntryData);
    return GetEntryTypeByID(m_pEntryData->m_dwEntryID);
}

inline LPCWSTR CAutorunItem::GetInformation() const
{
    return m_strInformation;
}



#endif//AUTORUNITEM_H
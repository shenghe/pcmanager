/**
* @file    autorunitem_service.h
* @brief   ...
* @author  zhangrui
* @date    2009-06-09  15:23
*/

#ifndef AUTORUNITEM_SERVICE_H
#define AUTORUNITEM_SERVICE_H

#include "autorunitem.h"



/// ·þÎñ
class CAutorunItem_Service: public CAutorunItem
{
public:

    virtual DWORD GetFileCount() const
    {
        if (SERVICE_WIN32_SHARE_PROCESS == m_dwServiceType)
            return 2;

        return 1;
    }

    virtual LPCWSTR GetFileAt(DWORD dwIndex) const
    {
        if (0 == dwIndex)
            return m_strExpandedImagePath;

        assert(SERVICE_WIN32_SHARE_PROCESS == m_dwServiceType);
        if (1 == dwIndex)
            return m_strExpandedServiceDll;

        return NULL;
    }

    virtual HRESULT Clone(CAutorunItem** ppItem) const
    {
        assert(ppItem);
        CAutorunItem_Service* pNewItem = new CAutorunItem_Service();
        if (!pNewItem)
            return E_OUTOFMEMORY;

        *pNewItem = *this;
        *ppItem = pNewItem;
        return S_OK;
    }

	virtual HRESULT Clean() const
	{
		return  S_OK;
	}


    //virtual HRESULT Scan(CAutorunContext* pContext);
    //virtual HRESULT Disable(CAutorunContext* pContext);
    //virtual HRESULT Fix(CAutorunContext* pContext);


    CString m_strServiceName;
    DWORD   m_dwServiceType;
    DWORD   m_dwServiceStart;
    CString m_strImagePath;
    CString m_strServiceDll;
    CString m_strServiceDisplayName;
    CString m_strServiceDescription;

    CString m_strExpandedImagePath;
    CString m_strExpandedServiceDll;
};

template <>
inline const CAutorunItem_Service* CAutorunItem::CastTo() const
{
    if (SLATR_Group_Services != CAutorunItem::GetEntryType())
        return NULL;

    return reinterpret_cast<const CAutorunItem_Service*>(this);
}



#endif//AUTORUNITEM_SERVICE_H
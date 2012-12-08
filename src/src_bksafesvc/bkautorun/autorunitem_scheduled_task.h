/**
* @file    autorunitem_scheduled_task.h
* @brief   ...
* @author  bbcallen
* @date    2009-07-17 18:58
*/

#ifndef AUTORUNITEM_SCHEDULED_TASK_H
#define AUTORUNITEM_SCHEDULED_TASK_H

#include "autorunitem.h"



/// 计划任务
class CAutorunItem_Scheduled_Task: public CAutorunItem
{
public:

    virtual DWORD GetFileCount() const
    {
        return 1;
    }

    virtual LPCWSTR GetFileAt(DWORD dwIndex) const
    {
        UNREFERENCED_PARAMETER(dwIndex);
        return m_strApplicationName;
    }

    virtual HRESULT Clone(CAutorunItem** ppItem) const
    {
        assert(ppItem);
        CAutorunItem_Scheduled_Task* pNewItem = new CAutorunItem_Scheduled_Task();
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

    CString m_strTaskName;
    CString m_strApplicationName;
};

template <>
inline const CAutorunItem_Scheduled_Task* CAutorunItem::CastTo() const
{
    if (SLATR_Group_Scheduled_Tasks != CAutorunItem::GetEntryType())
        return NULL;

    return reinterpret_cast<const CAutorunItem_Scheduled_Task*>(this);
}



#endif//AUTORUNITEM_SCHEDULED_TASK_H
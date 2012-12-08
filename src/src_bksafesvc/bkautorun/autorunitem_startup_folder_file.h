/**
* @file    autorunitem_startup_folder_file.h
* @brief   ...
* @author  bbcallen
* @date    2009-07-17 22:12
*/

#ifndef AUTORUNITEM_STARTUP_FOLDER_FILE_H
#define AUTORUNITEM_STARTUP_FOLDER_FILE_H

#include "autorunitem.h"



/// ·þÎñ
class CAutorunItem_Startup_Folder_File: public CAutorunItem
{
public:

    virtual DWORD GetFileCount() const
    {
        return 1;
    }

    virtual LPCWSTR GetFileAt(DWORD dwIndex) const
    {
        UNREFERENCED_PARAMETER(dwIndex);
        return m_strFilePath;
    }

    virtual HRESULT Clone(CAutorunItem** ppItem) const
    {
        assert(ppItem);
        CAutorunItem_Startup_Folder_File* pNewItem = new CAutorunItem_Startup_Folder_File();
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


    CString m_strStartupFolder;
    CString m_strFilePath;
};

template <>
inline const CAutorunItem_Startup_Folder_File* CAutorunItem::CastTo() const
{
    if (SLATR_Group_Startup_Folder != CAutorunItem::GetEntryType())
        return NULL;

    return reinterpret_cast<const CAutorunItem_Startup_Folder_File*>(this);
}



#endif//AUTORUNITEM_STARTUP_FOLDER_FILE_H
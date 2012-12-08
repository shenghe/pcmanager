/**
* @file    autorunitem_startup_ini.h
* @brief   ...
* @author  bbcallen
* @date    2009-09-10 15:36
*/

#ifndef AUTORUNITEM_STARTUP_INI_H
#define AUTORUNITEM_STARTUP_INI_H

#include "autorunitem.h"



/// ·þÎñ
class CAutorunItem_Startup_Ini: public CAutorunItem
{
public:
    typedef Skylark::CCopiableArray<CString> CFileList;

    virtual DWORD GetFileCount() const
    {
        return (DWORD)m_fileList.GetCount();
    }

    virtual LPCWSTR GetFileAt(DWORD dwIndex) const
    {
        if (dwIndex >= m_fileList.GetCount())
            return L"";

        return m_fileList[dwIndex];
    }

    virtual HRESULT Clone(CAutorunItem** ppItem) const
    {
        assert(ppItem);
        CAutorunItem_Startup_Ini* pNewItem = new CAutorunItem_Startup_Ini();
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


    CString     m_strStartupIniFile;
    CString     m_strIniApp;
    CString     m_strIniKey;
    CFileList   m_fileList;
};

template <>
inline const CAutorunItem_Startup_Ini* CAutorunItem::CastTo() const
{
    if (SLATR_Group_Startup_Ini != CAutorunItem::GetEntryType())
        return NULL;

    return reinterpret_cast<const CAutorunItem_Startup_Ini*>(this);
}



#endif//AUTORUNITEM_STARTUP_INI_H
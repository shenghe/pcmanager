/**
* @file    autorunitem_regkey.h
* @brief   ...
* @author  zhangrui
* @date    2009-06-09  15:22
*/

#ifndef AUTORUNITEM_REGKEY_H
#define AUTORUNITEM_REGKEY_H

#include "skylark2\slcoll.h"
#include "autorunitem.h"



/// 注册表启动项
class CAutorunItem_RegKey_Run: public CAutorunItem
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
        CAutorunItem_RegKey_Run* pNewItem = new CAutorunItem_RegKey_Run();
        if (!pNewItem)
            return E_OUTOFMEMORY;

        *pNewItem = *this;
        *ppItem = pNewItem;
        return S_OK;
    }

	virtual HRESULT Clean() const
	{//暂时仅实现删除当前键。后再增加到删除当前键及其所有子键。最后是多个根键下同样键的删除。
		HKEY t_hCleanKey = NULL;
		HRESULT t_hRetCode = 0;
		TCHAR tt[] = _T("zhangjun");
		t_hRetCode = RegOpenKey(m_hKeyRoot, m_strRegKeyName, &t_hCleanKey);
		if (t_hRetCode != ERROR_SUCCESS) return t_hRetCode;

		t_hRetCode = RegDeleteValue(t_hCleanKey, m_strRegValueName);
		if (t_hRetCode == ERROR_SUCCESS) return S_OK;
		return t_hRetCode;

	}

    //virtual HRESULT Scan(CAutorunContext*) {return E_NOTIMPL;}
    //virtual HRESULT Disable(CAutorunContext*) {return E_NOTIMPL;}
    //virtual HRESULT Fix(CAutorunContext*) {return E_NOTIMPL;}

    HKEY        m_hKeyRoot;
    CString     m_strRegKeyName;
    CString     m_strRegValueName;
    CString     m_strRegValue;
    CFileList   m_fileList;
};

template <>
inline const CAutorunItem_RegKey_Run* CAutorunItem::CastTo() const
{
    if (SLATR_Group_RegKey_Startup != CAutorunItem::GetEntryType())
        return NULL;

    return reinterpret_cast<const CAutorunItem_RegKey_Run*>(this);
}



#endif//AUTORUNITEM_REGKEY_H
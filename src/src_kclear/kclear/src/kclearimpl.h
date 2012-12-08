#pragma once

//////////////////////////////////////////////////////////////////////////

#include "kclear/ikclear.h"
#include "kscmaindlg.h"

//////////////////////////////////////////////////////////////////////////

class CKClearImpl : public IKClear
{
public:
    CKClearImpl();
    virtual ~CKClearImpl();

    virtual void __cdecl Release();
    virtual HWND __cdecl CreateKClear(HWND hParent);
    virtual BOOL __cdecl Navigate(const wchar_t* szNavigate);
    virtual BOOL __cdecl PreNavigate(const wchar_t* szNavigate);

private:
    CKscMainDlg* m_pDlgMain;
};

//////////////////////////////////////////////////////////////////////////

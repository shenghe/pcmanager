#pragma once

class __declspec(uuid("00140123-5CCE-4279-BD9B-11D66677BB76"))
IUpdateHelper : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Combine( LPCWSTR lpwszDifflib ) = 0;
    virtual HRESULT STDMETHODCALLTYPE Notify( LPCWSTR lpwszFileName ) = 0;
};

#define FN_CreateUpdateHelper			"CreateUpdateHelper"
typedef HRESULT (WINAPI *PFN_CreateUpdateHelper)( REFIID riid, void** ppvObj );

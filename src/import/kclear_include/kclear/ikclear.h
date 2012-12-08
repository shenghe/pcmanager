/********************************************************************
	created:	2010/09/17
	created:	17:9:2010   14:36
	filename: 	ikclear.h
	author:		Jiang Fengbing
	
	purpose:	清理模块对外接口
*********************************************************************/

#pragma once

#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////

[
    uuid(35729581-fc15-402e-a6c4-0cdeaf6f2357)
]
interface IKClear
{
    virtual void __cdecl Release() = 0;
    virtual HWND __cdecl CreateKClear(HWND hParent) = 0;
    virtual BOOL __cdecl Navigate(const wchar_t* szNavigate) = 0;
    virtual BOOL __cdecl PreNavigate(const wchar_t* szNavigate) = 0;
};

//////////////////////////////////////////////////////////////////////////

typedef HRESULT (__cdecl *CreateKClearObject_t)(REFIID riid, void** ppObject);
HRESULT __cdecl CreateKClearObject(REFIID riid, void** ppObject);

//////////////////////////////////////////////////////////////////////////

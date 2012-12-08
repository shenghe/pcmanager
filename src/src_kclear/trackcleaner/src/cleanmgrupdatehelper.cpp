#include "CleanMgrUpdateHelper.h"

CCleanMgrUpdateHelper::CCleanMgrUpdateHelper(void)
{
}

CCleanMgrUpdateHelper::~CCleanMgrUpdateHelper(void)
{
}

HRESULT STDMETHODCALLTYPE CCleanMgrUpdateHelper::Combine( LPCWSTR lpwszDifflib )
{
	OutputDebugStringW(_T("CCleanMgrUpdateHelper::Combine"));
	return 0;
}

HRESULT STDMETHODCALLTYPE CCleanMgrUpdateHelper::Notify( LPCWSTR lpwszFileName )
{
	return 0;
}
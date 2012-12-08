#pragma once

interface IComponentMgr
{
	virtual HRESULT STDMETHODCALLTYPE Init() = 0;
	virtual HRESULT STDMETHODCALLTYPE Uninit() = 0;

};


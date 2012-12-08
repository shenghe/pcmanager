/** 
* @file     BKRescanSpMgr.h
* @author   jiaoyafei
* @date     2010-12-10
* @brief    管理rescansp的类
*/

#ifndef _BK_RESCAN_SP_MGR_H__
#define _BK_RESCAN_SP_MGR_H__

#include "ikrescanspmanager.h"
#include "irescancallback.h"

class CBKRescanSpMgr
{
public:
    CBKRescanSpMgr(): m_hCommSPModule(NULL), m_piReScanSP(NULL), m_bInit(false)
    {    }

public:
    HRESULT __stdcall Initialize(IReScanCallback* pIRescanCallback = NULL);

    HRESULT __stdcall Uninitialize();


    HRESULT __stdcall StopRescanSP();

    HRESULT __stdcall StartRescanSP();

    static CBKRescanSpMgr& Instance()
    {
        static CBKRescanSpMgr _ins;

        return _ins;
    }

private:
	HRESULT _GetFilePath(wchar_t* pwszFilePath, wchar_t* pwszFileName, int nLen);

private:
    HMODULE            m_hCommSPModule;
    IKRescanSPManager* m_piReScanSP;
    bool               m_bInit;
};

#endif //> _BK_RESCAN_SP_MGR_H__
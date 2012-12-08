////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : urlmonservice.cpp
//      Version   : 1.0
//      Comment   : 木马网络防火墙的的服务
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "urlmonservice.h"
#include "kws/urlmondef.h"
#include "kws/ipc.h"

#include "kdriverproxy.h"
#include "kprocessinfomgr.h"
#include "kmodulemgr.h"
#include "kurlcachemgr.h"
#include "netflux/knetflux.h"

#include "kpfw/msg_logger.h"

KUrlMonService* KUrlMonService::s_UrlMonService = NULL;

KUrlMonService::KUrlMonService(): m_pProxy(NULL),
								m_pProcessMgr(NULL),
								m_pModuleMgr(NULL),
								m_pUrlMgr(NULL),
								m_pIpcServer(NULL),
								m_pFluxMgr(NULL),
								m_UpdateThread(NULL),
								m_bUpdateThreadWorking(FALSE),
								m_hThreadExit(NULL)
{

}

KUrlMonService::~KUrlMonService()
{
	if (m_pProxy)
		Uninitialize();
}

void KUrlMonService::StopUpdate()
{
	m_hThreadExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bUpdateThreadWorking = FALSE;
	m_UpdateThread = ::CreateThread(NULL, 0, ThreadUpdate, this, NULL, NULL);
}

void KUrlMonService::StartUpdate()
{
	if (m_hThreadExit)
		::SetEvent(m_hThreadExit);

	if (m_UpdateThread)
	{
		if (::WaitForSingleObject(m_UpdateThread, 5000) == WAIT_TIMEOUT)
		{
			if (m_bUpdateThreadWorking)
			{
				//::DebugBreak();
				::TerminateThread(m_UpdateThread, 0);
			}
		}
		::CloseHandle(m_UpdateThread);
		m_UpdateThread = NULL;
	}
	if (m_hThreadExit)
	{
		::CloseHandle(m_hThreadExit);
		m_hThreadExit = NULL;
	}
}

DWORD KUrlMonService::ThreadUpdate(LPVOID lpParameter)
{
	KUrlMonService* pThis = (KUrlMonService*)lpParameter;
	if (pThis)
		pThis->DoUpdateCfg();
	return 0;
}

DWORD KUrlMonService::DoUpdateCfg()
{
	kws_log(TEXT("KNetFluxMgr::DoUpdateCfg start"));
	m_bUpdateThreadWorking = TRUE;
	try
	{
		while (1)
		{
			DWORD nWaitRes = ::WaitForSingleObject(m_hThreadExit, 1000 * 60 * 10);
			switch (nWaitRes)
			{
			case WAIT_TIMEOUT:
				UpdateCfg();
				break;
			case WAIT_OBJECT_0:
				goto exit0;
				break;
			default:
				goto exit0;
				break;
			}
		}
	}
	catch (...)
	{

	}

exit0:
	m_bUpdateThreadWorking = FALSE;
	return 0;
}

DWORD	KUrlMonService::UpdateCfg()
{
	if (m_pProxy)
	{
		m_pProxy->CheckMustUnLoadDriver();
	}
	else
	{
		KTdiDriverProxy* pProxy = new KTdiDriverProxy;
		pProxy->CheckMustUnLoadDriver();
		delete pProxy;
		pProxy = NULL;
	}
	return 0;
}

HRESULT KUrlMonService::Initialize()
{
	HANDLE hThreadInit = ::CreateThread(NULL, 0, ThreadInit, this, NULL, NULL);
	::CloseHandle(hThreadInit);
	return S_OK;
}

DWORD KUrlMonService::ThreadInit(LPVOID lpParameter)
{
	KUrlMonService* pThis = (KUrlMonService*)lpParameter;
	if (pThis)
		pThis->_Initialize();
	return 0;
}

HRESULT KUrlMonService::_Initialize()
{
	kws_log(TEXT("KUrlMonService::Initialize begin"));
	if (!m_pUrlMgr)
	{
		m_pUrlMgr = new KUrlCacheMgr;
		if (FAILED(m_pUrlMgr->Init()))
		{
			kws_log(TEXT("KUrlMonService::Initialize failed to init KUrlCacheMgr"));
			delete m_pUrlMgr;
			m_pUrlMgr = NULL;
		}
		if (!m_pUrlMgr->GetCfg())
		{
			kws_log(TEXT("KUrlMonService::Initialize failed to get cfg"));
			m_pUrlMgr->UnInit();
			delete m_pUrlMgr;
			m_pUrlMgr = NULL;
		}
	}

	if (!m_pModuleMgr && m_pUrlMgr)
	{
		m_pModuleMgr = new KModuleMgr;
		if (m_pModuleMgr && FAILED(m_pModuleMgr->Init(m_pUrlMgr->GetCfg())))
		{
			kws_log(TEXT("KUrlMonService::Initialize failed to init KModuleMgr"));
			delete m_pModuleMgr;
			m_pModuleMgr = NULL;
		}
	}
	
	if (!m_pProcessMgr && m_pModuleMgr && m_pUrlMgr)
	{
		m_pProcessMgr = new KProcessInfoMgr;
		if (m_pProcessMgr&& FAILED(m_pProcessMgr->Init(m_pModuleMgr, m_pUrlMgr)))
		{
			kws_log(TEXT("KUrlMonService::Initialize failed to init KProcessInfoMgr"));
			delete m_pProcessMgr;
			m_pProcessMgr = NULL;
		}
	}
	
	if (!m_pProxy && m_pProcessMgr)
	{
		m_pProxy = new KTdiDriverProxy;
		if (m_pProxy && FAILED(m_pProxy->Init(m_pProcessMgr, m_pModuleMgr, m_pUrlMgr->GetCfg())))
		{
			kws_log(TEXT("KUrlMonService::Initialize failed to init KTdiDriverProxy"));
			delete m_pProxy;
			m_pProxy = NULL;
		}
	}

	if (!m_pFluxMgr && m_pProxy && m_pProcessMgr && m_pModuleMgr)
	{
		m_pFluxMgr = new KNetFluxMgr;
		if (m_pFluxMgr && FAILED(m_pFluxMgr->Init(m_pProxy->GetDriverProxy(), m_pProcessMgr, m_pModuleMgr, m_pUrlMgr->GetCfg())))
		{
			kws_log(TEXT("KUrlMonService::Initialize failed to init fluxmgr"));
			delete m_pFluxMgr;
			m_pFluxMgr = NULL;
		}
	}

	if (!m_pIpcServer)
	{
		m_pIpcServer = new IPCServer;
		m_pIpcServer->AddCall(IPC_PROC_URLMON, KUrlMonService::OnUrlMonIpc);
		m_pIpcServer->Start();
	}

	s_UrlMonService = this;
	kws_log(TEXT("KUrlMonService::Initialize end"));
	return S_OK;
}

HRESULT KUrlMonService::Uninitialize()
{
	s_UrlMonService = NULL;
	if (m_pIpcServer)
	{
		m_pIpcServer->Uninit();
		delete m_pIpcServer;
		m_pIpcServer = NULL;
	}

	if (m_pFluxMgr)
	{
		m_pFluxMgr->Uninit();
		delete m_pFluxMgr;
		m_pFluxMgr = NULL;
	}
	if (m_pProxy)
	{
		m_pProxy->UnInit();
		delete m_pProxy;
		m_pProxy = NULL;
	}

	// 先停止url的Query，
	if (m_pUrlMgr)
	{
		m_pUrlMgr->UnInit();
		// 这里先不释放，等到其他的线程都结束后delete它，防止cfg数据释放引起崩溃
	}

	if (m_pProcessMgr)
	{
		m_pProcessMgr->UnInit();
		delete m_pProcessMgr;
		m_pProcessMgr = NULL;
	}

	if (m_pModuleMgr)
	{
		m_pModuleMgr->UnInit();
		delete m_pModuleMgr;
		m_pModuleMgr = NULL;
	}

	// 带配置的需要最后析构
	if (m_pUrlMgr)
	{
		delete m_pUrlMgr;
		m_pUrlMgr = NULL;
	}

	return S_OK;
}

DWORD KUrlMonService::OnUrlMonIpc(LPVOID pv)
{
	UrlMonIpcParam* pParam = NULL;

	IPC_CALL_STUB callStub = {0};
	memcpy(&callStub, (PIPC_CALL_STUB)pv, sizeof(IPC_CALL_STUB));
	pParam = (UrlMonIpcParam*)((char*)callStub.pParamBuffer + sizeof(unsigned));

	if (s_UrlMonService)
		s_UrlMonService->OnUrlMonIpc(pParam, &callStub);
	return 0;
}
void KUrlMonService::OnUrlMonIpc(UrlMonIpcParam* pParam, IPC_CALL_STUB* pStub)
{
	switch (pParam->m_nCallType)
	{
	case Ipc_NetFlux_ClearDumpStateAndStartUp:
		ClearDumpStateAndStartUp();
		break;
	case Ipc_NetFlux_SetProcessSpeed:
		SetProcessSpeed(&pParam->m_SetSpeed);
		break;
	case Ipc_NetFux_SetUnPop:
		SetProcUnPop(&pParam->m_UnPop);
		break;
	case Ipc_NetFux_GetPopInfo:
		if (m_pFluxMgr)
		{
			if (m_pFluxMgr->GetPopInfo(&pParam->m_PopInfo))
			{
				pStub->pServer->SetBuffer(pStub->pParamBuffer, 
					pParam, sizeof(UrlMonIpcParam));
			}
		}
		break;
	}
}

void KUrlMonService::SetProcessSpeed(FluxSetProcessSpeed* pParam)
{
	if (m_pFluxMgr)
		m_pFluxMgr->SetProcessSpeed(pParam);
}
void KUrlMonService::SetProcUnPop(FluxSetProcUnPop* pParam)
{
	if (m_pFluxMgr)
		m_pFluxMgr->SetProcUnPop(pParam);
}
void KUrlMonService::ClearDumpStateAndStartUp()
{
	if (!m_pProxy && m_pProcessMgr)
	{
		m_pProxy = new KTdiDriverProxy;

		m_pProxy->DoCheckForceDisableTDI();
		m_pProxy->ClearDumpState();

		if (FAILED(m_pProxy->Init(m_pProcessMgr, m_pModuleMgr, m_pUrlMgr->GetCfg())))
		{
			kws_log(TEXT("KUrlMonService::ClearDumpStateAndStartUp failed to init KTdiDriverProxy"));
			delete m_pProxy;
			m_pProxy = NULL;
		}
	}

	if (!m_pFluxMgr && m_pProxy && m_pProcessMgr && m_pModuleMgr)
	{
		m_pFluxMgr = new KNetFluxMgr;
		if (FAILED(m_pFluxMgr->Init(m_pProxy->GetDriverProxy(), m_pProcessMgr, m_pModuleMgr, m_pUrlMgr->GetCfg())))
		{
			kws_log(TEXT("KUrlMonService::ClearDumpStateAndStartUp failed to init fluxmgr"));
			delete m_pFluxMgr;
			m_pFluxMgr = NULL;
		}
	}
}
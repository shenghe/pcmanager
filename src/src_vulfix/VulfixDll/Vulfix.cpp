#include "stdafx.h"
#include "Vulfix\\Vulfix.h"
#include "..\\VulfixLib\\ImplVulfix.h"
#include "..\\VulfixLib\\ImplVulfixed.h"
#include "..\\VulfixLib\\ImplSoftVulfix.h"
#include "..\\VulfixLib\\ImplRepairVul.h"
#include "..\\VulfixLib\\ImplUpdateHelper.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef _DEBUG
#define DEBUG_TRACE _tcprintf
#else
#define DEBUG_TRACE
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  dwReason,
					  LPVOID lpReserved
					  )
{
	if (dwReason == DLL_THREAD_ATTACH)
	{
		//CoInitialize(NULL);
	}
	else if(dwReason == DLL_THREAD_DETACH)
	{
		//::CoUninitialize();
	}
	return TRUE;
}


template<typename TBase>
class CImplIUnkownT : public TBase
{
public:
	CImplIUnkownT() : m_nRefCount(0) 
	{}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject)
	{
		return E_NOTIMPL;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return InterlockedIncrement(&m_nRefCount);
	}

	virtual ULONG STDMETHODCALLTYPE Release( void)
	{
		LONG nCount = InterlockedDecrement(&m_nRefCount);
		if ( nCount <= 0 )
		{
			delete this;
		}
		return nCount;
	}
private:
	LONG m_nRefCount;
};

struct TOverviewItem 
{
	INT nLeakType;	// Leak type
	int nID;		// id 
	int nLeakLevel;	//漏洞等级 TLeakWarningLevel
	DWORD dwFlags;	// 补丁类型 
	CString strTitle, strDescription;	// 漏洞简述
	CString strPubDate;	// 发布日期 
};

class CVulCollection : public IVulCollection
{
public:
	CVulCollection()
	{
		m_statusValid = FALSE;
		m_nStatus = 0;
	}

	void Destroy()
	{
		leaks.RemoveAll();
		delete this;
	}
	
	HRESULT get_Status(LONG *ptrStatus)
	{
		if(!m_statusValid)
			return E_FAIL;
		if(ptrStatus)
			*ptrStatus = m_nStatus;
		return S_OK;
	}

	HRESULT get_Count(ULONG *ptrCount) 
	{
		ATLASSERT( ptrCount );
		*ptrCount = leaks.GetSize();
		return S_OK;
	}

	HRESULT get_Item(ULONG n, int *pKBID, int *pLevel, BSTR* pTitle, BSTR* pDescription)
	{
		return get_Item2(n, pKBID, pLevel, NULL, NULL, pTitle, pDescription);
	}
	
	HRESULT get_Item2(ULONG n, int *pKBID, int *pLevel, int *pNType, DWORD *pVFlags, BSTR* pTitle, BSTR* pDescription)
	{
		return get_Item3(n, pKBID, pLevel, pNType, pVFlags, pTitle, pDescription, NULL);
	}

	virtual HRESULT get_Item3(ULONG n, int *pKBID, int *pLevel, int *pNType, DWORD *pVFlags, BSTR* pTitle, BSTR* pDescription, BSTR *pPubDate)
	{
		if(n>=(ULONG)leaks.GetSize())
			return E_FAIL;
		
		USES_CONVERSION;
		TOverviewItem &item = leaks[n];

		if(pKBID)			*pKBID = item.nID;
		if(pLevel)			*pLevel = item.nLeakLevel;
		if(pNType)			*pNType = item.nLeakType;
		if(pVFlags)			*pVFlags = item.dwFlags;
		if(pTitle)			*pTitle = item.strTitle.AllocSysString();
		if(pDescription)	*pDescription = item.strDescription.AllocSysString();
		if(pPubDate)		*pPubDate = item.strPubDate.AllocSysString();
		return S_OK;
	}

	HRESULT get_VulCount(INT *pCntHigh, INT *pCntOption, INT *pCntSP)
	{
		INT cntHigh=0, cntOption=0, cntSP=0;
		for(int i=0; i<leaks.GetSize(); ++i)
		{
			TOverviewItem &t = leaks[i];
			if(t.dwFlags&VFLAG_SERVICE_PATCH)
				++cntSP;
			else if(t.nLeakLevel>0)
				++cntHigh;
			else if(t.nLeakLevel==0)
				++cntOption;
		}
		if(pCntHigh)
			*pCntHigh = cntHigh;
		if(pCntOption)
			*pCntOption = cntOption;
		if(pCntSP)
			*pCntSP = cntSP;
		return S_OK;
	}
public:
	CSimpleArray<TOverviewItem> leaks;
	BOOL m_statusValid;
	LONG m_nStatus;
};

class CVulEnvironment : public IVulEnvironment {

public:
	virtual void Destroy()
	{
		delete this;
	}
	
	virtual HRESULT IsUsingInterface(BOOL *pRet)
	{
		if(!pRet)
			return E_POINTER;
		*pRet = RequireUsingInterface();
		return S_OK;
	}

	virtual HRESULT IsSystemSupport(BOOL *pRet)
	{
		if(!pRet)
			return E_POINTER;
		CSysEnv env;
		env.Init();
		*pRet = env.IsOsSupported();
		return S_OK;
	}
};

HRESULT WINAPI ExpressScanLeak(IVulCollection **pVulCollection, DWORD dwFlags, BOOL bQuickScan)
{
	TIME_CHECK( _T("ExpressScanLeak ") );

	ATLASSERT(pVulCollection);
	CVulCollection *pCollection = new CImplIUnkownT<CVulCollection>;
	
	BOOL bFoundSysVuls = FALSE;
	IVulfix *pscan = new CImplVulfix;
	dwFlags |= VULSCAN_EXPRESS_SCAN;
	
	if( bQuickScan && RequireUsingInterface() )
		// Vista 进行快速扫描 
	{
		pCollection->m_nStatus = pscan->ExpressScanSystem();
		pCollection->m_statusValid = TRUE;
		bFoundSysVuls = pCollection->m_nStatus>0;
	}
	else
	{
		pCollection->m_statusValid = FALSE;
		if( SUCCEEDED( pscan->Scan(dwFlags) ))
		{
			const CSimpleArray<LPTUpdateItem> &arr = pscan->GetResults();
			for(int i=0; i<arr.GetSize(); ++i)
			{
				LPTUpdateItem pLeak = arr[i];

#ifdef _DEBUG
				_tcprintf(_T("-Quick.VUL %d %2d %s \n"), pLeak->nID, pLeak->nWarnLevel, pLeak->strName);
#endif
				if( pLeak->isIgnored || pLeak->nWarnLevel<=0 || pLeak->isExclusive )
					continue;
				
				TOverviewItem item;
				item.nLeakType = VTYPE_WINDOWS;
				item.nID = pLeak->nID;
				item.nLeakLevel = pLeak->nWarnLevel;
				item.dwFlags = pLeak->dwVFlags;
				item.strTitle = pLeak->strName;
				item.strDescription = pLeak->strDescription;
				item.strPubDate = pLeak->strPubdate;
				pCollection->leaks.Add( item );
			}
		}
		bFoundSysVuls = pCollection->leaks.GetSize()>0;

		// 如果没有找到系统漏洞, 则搜索软件漏洞 
		if(!bFoundSysVuls)
		{
			const CSimpleArray<LPTVulSoft> &arr = pscan->GetSoftVuls();
			for(int i=0; i<arr.GetSize(); ++i)
			{
				LPTVulSoft psoft = arr[i];
#ifdef _DEBUG
				_tcprintf(_T("-Quick.SOFT %d %2d %s \n"), psoft->nID, psoft->nLevel, psoft->strName);
#endif
				if(psoft->isIgnored || psoft->state.comState==COM_ALL_DISABLED)
					continue;

				TOverviewItem item;
				item.nLeakType = VTYPE_SOFTLEAK;
				item.nID = psoft->nID;
				item.nLeakLevel = psoft->nLevel;
				item.dwFlags = 0;
				item.strTitle = psoft->strName;
				item.strDescription = psoft->strDescription;
				item.strPubDate = psoft->strPubdate;
				pCollection->leaks.Add( item );
			}
		}
	}
	delete pscan;
	*pVulCollection = pCollection;
	return S_OK;
}

class CVulScanImpl : public IVulScan
{
public:
	virtual HRESULT ScanLeakEx(IVulCollection **pVulCollection, DWORD dwFlags)
	{
		TIME_CHECK( _T("ScanLeakEx ") );
		ATLASSERT(pVulCollection);

		if(pVulCollection==NULL)
			return E_POINTER;
		
		HRESULT hr = S_OK;
		CVulCollection *pCollection = new CImplIUnkownT<CVulCollection>;
		
		IVulfix *pscan = new CImplVulfix;
		if( (dwFlags&VULSCAN_EXPRESS_SCAN) && RequireUsingInterface())
		{
			// 对于需要使用接口的快速扫描, 只对比时间 
			pCollection->m_nStatus = pscan->ExpressScanSystem();
			pCollection->m_statusValid = TRUE;
		}
		else
		{
			pCollection->m_statusValid = FALSE;
			if( SUCCEEDED( hr=pscan->Scan(0) ))
			{
				pCollection->m_statusValid = FALSE;
				_FillCollection(pscan, pCollection);
			}
		}
		delete pscan;
		*pVulCollection = pCollection;
		return hr;
	}
	
	void _FillCollection( IVulfix * pscan, CVulCollection * pCollection )
	{
		// 系统&Office 漏洞 
		const CSimpleArray<LPTUpdateItem> &arr = pscan->GetResults();
		for(int i=0; i<arr.GetSize(); ++i)
		{
			LPTUpdateItem pLeak = arr[i];
			DEBUG_TRACE(_T("-Quick.VUL %d %2d %s \n"), pLeak->nID, pLeak->nWarnLevel, pLeak->strName);
			if( pLeak->isIgnored || pLeak->nWarnLevel<0 )
				continue;

			TOverviewItem item;
			item.nID = pLeak->nID;
			item.nLeakType = pLeak->m_nType;
			item.nLeakLevel = pLeak->nWarnLevel;
			item.dwFlags = pLeak->dwVFlags;
			item.strTitle = pLeak->strName;
			item.strDescription = pLeak->strDescription;
			item.strPubDate = pLeak->strPubdate;
			pCollection->leaks.Add( item );
		}

		// 软件 漏洞
		const CSimpleArray<LPTVulSoft> &arr2 = pscan->GetSoftVuls();
		for(int i=0; i<arr2.GetSize(); ++i)
		{
			LPTVulSoft psoft = arr2[i];
			DEBUG_TRACE(_T("-Quick.SOFT %d %2d %s \n"), psoft->nID, psoft->nLevel, psoft->strName);

			if(psoft->isIgnored || psoft->state.comState==COM_ALL_DISABLED)
				continue;

			TOverviewItem item;
			item.nID = psoft->nID;
			item.nLeakType = VTYPE_SOFTLEAK;
			item.nLeakLevel = psoft->nLevel;
			item.dwFlags = 0;
			item.strTitle = psoft->strName;
			item.strDescription = psoft->strDescription;
			item.strPubDate = psoft->strPubdate;
			pCollection->leaks.Add( item );
		}
	}
};


extern "C" IVulfix*       WINAPI CreateVulFix(int version/* = BEIKESAFE_VULSCAN_DLL_VERSION*/)
{
    return new CImplVulfix;
}

extern "C" IVulfixed*     WINAPI CreateVulFixed(int version/* = BEIKESAFE_VULSCAN_DLL_VERSION*/)
{
    return new CImplVulfixed;
}

extern "C" ISoftVulfix*   WINAPI CreateSoftVulFix(int version/* = BEIKESAFE_VULSCAN_DLL_VERSION*/)
{
    return new CImplSoftVulfix;
}

extern "C" IRepairVul*    WINAPI CreateRepairVul(int version/* = BEIKESAFE_VULSCAN_DLL_VERSION*/)
{
    return new CImplRepairVul;
}

extern "C"  HRESULT WINAPI CreateUpdateHelper( REFIID riid, void** ppvObj )
{
	if(!ppvObj)
		return E_POINTER;
	
	*ppvObj = (void*) new CImplUpdateHelper;
	return S_OK;
}

HRESULT WINAPI CreateObject( REFIID riid, void** ppvObj )
{
	ATLASSERT(ppvObj);
	if ( IsEqualIID(riid, __uuidof(IVulEnvironment)) )
	{
		IVulEnvironment *p = new CImplIUnkownT<CVulEnvironment>;
		p->AddRef();
		*ppvObj = p;
		return S_OK;
	}
	if(IsEqualIID(riid, __uuidof(IVulScan)))
	{
		IVulScan *p = new CImplIUnkownT<CVulScanImpl>;
		p->AddRef();
		*ppvObj = p;
		return S_OK;
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CreateObjectEx( LPCTSTR lpszVersion, REFIID riid, void** ppvObj )
{
	ATLASSERT(ppvObj);
	if ( IsEqualIID(riid, __uuidof(IVulEnvironment)) )
	{
		IVulEnvironment *p = new CImplIUnkownT<CVulEnvironment>;
		p->AddRef();
		*ppvObj = p;
		return S_OK;
	}
	if(IsEqualIID(riid, __uuidof(IVulScan)))
	{
		IVulScan *p = new CImplIUnkownT<CVulScanImpl>;
		p->AddRef();
		*ppvObj = p;
		return S_OK;
	}
	return E_NOINTERFACE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

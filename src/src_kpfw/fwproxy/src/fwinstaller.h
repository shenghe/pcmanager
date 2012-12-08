#ifndef FWINSTALLER_INC_
#define FWINSTALLER_INC_

#include "fwproxy_public.h"

//////////////////////////////////////////////////////////////////////////
#ifndef TDIFILT_FOR_MODULE_URL
#define TDI_ALEFILTER_MODULE       L"kaleflt.sys"
#define TDI_FILTER_MODULE       L"ktdifilt.sys"
#define TDI_FILTER_DRIVER       L"KTdiFilt"
#else
#define TDI_FILTER_MODULE       L"kmodurl.sys"
#define TDI_FILTER_MODULE64       L"kmodurl64.sys"
#define TDI_FILTER_DRIVER       L"kmodurl"
#define TDI_ALEFILTER_MODULE    L"kaleflt.sys"
#endif

//////////////////////////////////////////////////////////////////////////

class CFwInstaller : 
	public IFwInstaller,
	public KSComRoot<CFwInstaller> 
{
public:
    CFwInstaller();
    virtual ~CFwInstaller();

    STDMETHODIMP Install();
    STDMETHODIMP UnInstall();
	STDMETHODIMP SetConfig( DWORD dwEnableFlag, DWORD dwRequestFlag );

	KS_DEFINE_GETCLSID(CLSID_IFwInstallerImpl);
	DECLARE_INDEPENDENT_OBJECT();

	KSCOM_QUERY_BEGIN
	KSCOM_QUERY_ENTRY( IFwInstaller )
	KSCOM_QUERY_END

protected:
    int OpenSCManager();
    void CloseSCManager();
    int OpenService(
        const WCHAR cwszServiceName[], 
        HANDLE& hService, 
        BOOL& fExist
        );
    int CreateService(
        const WCHAR cwszServiceName[],
        const WCHAR cwszBinPath[],
        const WCHAR cwszLoadOrderGroup[],
        const WCHAR cwszDependencies[],
        HANDLE& hService,
		DWORD dwStartType
        );
    void CloseService(HANDLE hService);
    int DeleteService(HANDLE hService);
	
protected:
    int GetModuleDir();
    int IsVistaPlatform();
	void ChangeTdiStartOrder();
	void ChangeAleStartOrder();
	BOOL IsWow64();

private:
    SC_HANDLE   m_hSCManager;
    WCHAR       m_wszModuleDir[MAX_PATH];
};

//////////////////////////////////////////////////////////////////////////


#endif  // !FWINSTALLER_INC_


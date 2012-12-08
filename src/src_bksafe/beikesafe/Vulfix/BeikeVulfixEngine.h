#pragma once
#include "BeikeVulfix.h"
#include "BeikeVulfixFileLog.h"

class CVulEngine
{
public:
	CVulEngine();
	~CVulEngine();

	static DWORD WINAPI ThreadFunc_Scan( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_Repair( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_ScanFixed( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_ScanIngored( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_ScanSuperseded( LPVOID lpParam );
		
	void SetUseIUpdate(BOOL bUseIUpdate);
	
	void IgnoreVuls(int nKB, bool bIgnore);
	void IgnoreVuls(CSimpleArray<int> &arr, bool bIgnore);
	
	// É¨Ãè
	bool ScanVul(HWND hWnd);
	bool CancelScanVul();
	void _ScanVul(HWND hWnd);

	// ÐÞ¸´Â©¶´
	BOOL CheckRepairPrequisite(BOOL hasOfficeVul);
	bool RepairAll(HWND hWnd, const CSimpleArray<int> &arrVulIds, const CSimpleArray<int> &arrSoftVulIds);
	bool CancelRepair();
	void _RepairVul(HWND hWnd);
	
	// ÒÑ°²×°
	bool ScanFixed(HWND hWnd);
	void CancelScanFixed();
	void _ScanFixed(HWND hWnd);

	// ¹ýÆÚ²¹¶¡ 
	bool ScanSuperseded(HWND hWnd);
	bool CancelScanSuperseded();
	void _ScanSuperseded(HWND hWnd);
	
	// 
	// ÏµÍ³Â©¶´, ÒÑºöÂÔ²¹¶¡
	bool ScanIgnored(HWND hWnd);
	bool CancelScanIgnored();
	void _ScanIgnored(HWND hWnd);
	
	void _SafeTerminateThread( HANDLE &hThread, BOOL bForceKill=TRUE );
	
	// 
	void _RelayMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	DWORD _GetScanFlags();

	int ExpressScan();
	BOOL IsUsingInterface();
	BOOL IsSystemSupported();

protected:
	BOOL _CheckDiskFreeSpace();
	BOOL _CheckUpdateRelatedSvc();
	BOOL _CheckNeedCloseOffice();

public:	
	BOOL m_bUseInterface;
	
	IVulEnvironment *m_pVulEnv;
	IRepairVul *m_pRepairVul;
	
	IVulfix *m_pVulScan;
	//ISoftVulfix *m_pSoftVulScan;
	HANDLE m_hThreadVulScan;
	BOOL m_bVulScanCanceled;
	
	// repair 
	BOOL m_isRepairing;
	HANDLE m_hThreadVulRepair;
	CSimpleArray<int> m_arrRepairVulIds, m_arrRepairVulSoftIds;
	BOOL m_bRepairCanceled;	

	// installed 
	IVulfixed *m_pIVulFixed;
	ISoftVulfix *m_pIVulFixedSoft;
	HANDLE m_hThreadVulFixed;
	BOOL m_bVulFixedCanceled;

	// ÏµÍ³Â©¶´, ignored
	IVulfix *m_pVulScanIgnored;
	HANDLE m_hThreadVulIgnored;
	BOOL m_bVulIgnoredCanceled;

	// Superseded 
	IVulfix *m_pVulScanSuperseded;
	HANDLE m_hThreadSuperseded;
	BOOL m_bVulScanSuperseded;

	
	// Èí¼þÂ©¶´
	HANDLE m_hThreadSoftVul;
	BOOL m_bCanceledSoftVul;

	// for Express scan 
	IVulfix *m_pVulExpressScan;

    HMODULE m_hModuleVulFixEngine;

    IVulfix* CreateVulFix();
    IVulfixed* CreateVulFixed();
    ISoftVulfix* CreateSoftVulFix();
    IRepairVul* CreateRepairVul();

    FnCreateVulFix      m_pfnCreateVulFix;
    FnCreateVulFixed    m_pfnCreateVulFixed;
    FnCreateSoftVulFix  m_pfnCreateSoftVulFix;
    FnCreateRepairVul   m_pfnCreateRepairVul;
	FnCreateObject		m_pfnCreateObj;

    BOOL _InitFunctions();

	// Install Log 
	CBeikeVulfixFileLog m_fixLog;
};

extern CVulEngine *theEngine;

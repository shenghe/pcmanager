#pragma once
#include "BeikeVulfix.h"

class CDlgMain;
class CVulEngine
{
public:

	CVulEngine(CDlgMain &mainDlg);
	~CVulEngine();

#if 0 
	static unsigned int __stdcall ThreadFunc_Scan( void *lpParam  );
	static unsigned int __stdcall ThreadFunc_Repair( void *lpParam  );
	static unsigned int __stdcall ThreadFunc_ScanFixed( void *lpParam );
#else
	static DWORD WINAPI ThreadFunc_Scan( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_Repair( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_ScanFixed( LPVOID lpParam );
	static DWORD WINAPI ThreadFunc_ScanIngored( LPVOID lpParam );
#endif
	
	void IgnoreVuls(CSimpleArray<int> &arr, bool bIgnore);
	
	// É¨Ãè
	bool ScanVul(HWND hWnd);
	bool CancelScanVul();
	void _ScanVul(HWND hWnd);
	
	// ÐÞ¸´Â©¶´
	bool RepairAll(HWND hWnd, const CSimpleArray<int> &arrVulIds, const CSimpleArray<int> &arrSoftVulIds );
	bool CancelRepair();
	void _RepairVul(HWND hWnd);
	
	// ÒÑ°²×°
	bool ScanFixed(HWND hWnd);
	void CancelScanFixed();
	void _ScanFixed(HWND hWnd);
	
	// 
	// ÏµÍ³Â©¶´, ÒÑºöÂÔ²¹¶¡
	bool ScanIgnored(HWND hWnd);
	bool CancelScanIgnored();
	void _ScanIgnored(HWND hWnd);

	
	void _SafeTerminateThread( HANDLE &hThread );
	void WaitScanDone();
	
	// 
	void _RelayMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

public:
	CDlgMain &m_MainDlg;
	
	IRepairVul *m_pRepairVul;

	IVulfix *m_pVulScan;
	ISoftVulfix *m_pSoftVulScan;
	HANDLE m_hThreadVulScan;
	
	// repair 
	HANDLE m_hThreadVulRepair;
	CSimpleArray<int> m_arrRepairVulIds, m_arrRepairVulSoftIds;
	BOOL m_bRepairCanceled;
	

	// installed 
	IVulfixed *m_pIVulFixed;
	HANDLE m_hThreadVulFixed;
	BOOL m_bVulFixedCanceled;

	// ÏµÍ³Â©¶´, ignored
	IVulfix *m_pVulScanIgnored;
	HANDLE m_hThreadVulIgnored;
	BOOL m_bVulIgnoredCanceled;

	// Èí¼þÂ©¶´
	HANDLE m_hThreadSoftVul;
	BOOL m_bCanceledSoftVul;

};

extern CVulEngine *theEngine;

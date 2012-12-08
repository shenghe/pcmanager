#ifndef ACF_FILTER_INC_
#define ACF_FILTER_INC_

//////////////////////////////////////////////////////////////////////////

typedef struct tagContentFiltScanResult {

    long nFoundRisk;
    char szRiskName[64];

} CF_SCAN_RESULT, *PCF_SCAN_RESULT;

#define defFunc_GetFuncs		"KsGetFilterFuncs"

typedef int (__stdcall *PFNStart)(void* p);
typedef int (__stdcall *PFNStop)(void* p);
typedef int (__stdcall *PFNScanBuffer)(
	unsigned long dwBufferSize,
	void* pBuffer,
	bool Outbound,
	PCF_SCAN_RESULT pScanResult
	);

typedef int (__stdcall *TKsGetFuncs)(/*OUT*/void* funcs, /*IN*/void* p);

typedef struct tagKFiltFuncs
{
	PFNStart start;
	PFNStop stop;
	PFNScanBuffer scan;
}KFiltFuncs, *PKFiltFuncs;

//////////////////////////////////////////////////////////////////////////

#endif  // !ACF_FILTER_INC_



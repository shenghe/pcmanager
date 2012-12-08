////////////////////////////////////////////////////////////////////////////////
//
//      Comment   : 网镖流量监控对话框中list控件的行数据
//
//      Create at : 2010-08-25
//      Create by : 林杰
//
//////////////////////////////////////////////////////////////////////////////

//#include "treelistctrlex.h"
#pragma once

#include "perfmon/kperfmon.h"
#include "runoptimize/processmon.h"
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

struct KPFWFLUX;
struct KProcFluxItem;
enum enumProcessType;


//////////////////////////////////////////////////////////////////////////
// listbox colum index [12/8/2010 zhangbaoliang]
#define NETMON_COLUMN_NAME					0
#define NETMON_COLUMN_PROCESS_DESCRIPTION	1
#define NETMON_COLUMN_TRUST_MODE			2
#define NETMON_COLUMN_DOWNLOAD_SPEED		3
#define NETMON_COLUMN_UPLOAD_SPEED			4
#define NETMON_COLUMN_DOWNLOAD_SUM			5
#define NETMON_COLUMN_UPLOAD_SUM			6
#define NETMON_COLUMN_OPERATOR				7
class KModuleMgr;

class KPerfMonListItemData //: public ITreeListItemDataEx
{
protected:
    ATL::CString				m_strProcessName;
    ATL::CString				m_strTrust;
    ATL::CString				m_strSmallIcon;
    DWORD						m_nProcessID;
    BOOL						m_bIconExtracted;
    HICON						m_hSmallIcon;
    ATL::CString				m_strProcessPath;
    KProcessPerfData*			m_pProcInfoItem;
	ATL::CString				m_strPID;
	HICON						m_defaultIcon;
	CString						m_strProcessDescription;
	BOOL						m_bProcessCanKill;
	BOOL						m_bIsShowOpenFileLink;
	ULONGLONG					m_uMemUsage;
	double						m_fCpuUsage;
	ULONGLONG					m_uIOSpeed;

public:
	KPerfMonListItemData(KProcessPerfData* pProcInfoItem);
	KPerfMonListItemData();

    ~KPerfMonListItemData();

	BOOL						GetIsShowOpenFileLink(void) { return m_bIsShowOpenFileLink; }
    bool 						Match(KPerfMonListItemData* pItem);
    void 						SetProcessPath(LPCTSTR szPath);
    void 						SetTrust(LPCTSTR szTrust);
	ATL::CString 				GetTrust();
	ULONGLONG 					GetDownloadSpeed();
	ULONGLONG 					GetUploadSpeed();
	ULONGLONG 					GetTotalDownload();
	ULONGLONG 					GetTotalUpload();
    DWORD 						GetProcessId();
	ATL::CString 				GetProcessName();
	CString 					GetProcessDescription();
	CString 					GetProcessFullPath();
/*	CString						GetProcessFullPath_x64();*/
    int 						GetRowType();
	BOOL 						KillCurrentSelProcess();
	void 						OpenDirector( BOOL bSelectFile = TRUE);
	void 						OpenFileAttribute();
	COLORREF 					GetLevelColor();
	ATL::CString				GetProcessCpuUsage();
	ATL::CString				GetProcessMemUsage();
	ATL::CString				GetProcessIOSpeed();
	int							GetProcessCpuUsage_INT();
	ULONGLONG					GetProcessMemUsage_ULL();
	ULONGLONG					GetProcessIOSpeed_ULL();
public:

	BOOL						_KillProcess(DWORD processId);
	BOOL 						_IsKingsoftWeiShiProcess(LPCWSTR szProcessPath);
	BOOL 						_IsSystemProcess(LPCWSTR szProcessPath);
	BOOL 						_IsSystemProcess(DWORD pid);
	enumProcessType 			_GetProcessType();
	BOOL						_IsWow64();
	CString						FormatLongExeName(const CString& strIn);
};



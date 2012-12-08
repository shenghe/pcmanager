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

class KNetFlowMonListItemData //: public ITreeListItemDataEx
{
protected:
    ATL::CString				m_strProcessName;
    ATL::CString				m_strTrust;
    ATL::CString				m_strSmallIcon;
    DWORD						m_ProcessID;
    BOOL						m_bIconExtracted;
    HICON						m_hSmallIcon;
    ATL::CString				m_strProcessPath;
    KProcFluxItem				m_ProcessFlowItem;
    ATL::CString				m_strReceiveSpeedText;
    ATL::CString				m_strSendSpeedText;
    ATL::CString				m_strTotalReceiveText;
    ATL::CString				m_strTotalSendText;
	ATL::CString				m_strPID;
	HICON						m_defaultIcon;
	CString						m_strProcessDescription;
	BOOL						m_bProcessCanKill;
	DWORD						m_dwSendLimit;//上传限速
	DWORD						m_dwRecvLimit;//下载限速
	DWORD						m_dwDisable;//禁用网络
	BOOL						m_bIsShowOpenFileLink;
public:
    KNetFlowMonListItemData();
    KNetFlowMonListItemData(KProcFluxItem pFlowItem);

    ~KNetFlowMonListItemData();

	BOOL						GetIsShowOpenFileLink(void) { return m_bIsShowOpenFileLink; }
    bool 						Match(KNetFlowMonListItemData* pItem);
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
	CString 					GetProcessFullPath(){return m_strProcessPath;}
    int 						GetRowType();
	BOOL 						KillCurrentSelProcess();
	void 						OpenDirector( BOOL bSelectFile = TRUE);
	void 						OpenFileAttribute();
	COLORREF 					GetLevelColor();
	DWORD						AccessNetIsDisable(){return m_dwDisable;}
	DWORD						GetSendLimitVaule(){return m_dwSendLimit / 1024;}
	DWORD						GetRecvLimitValue(){return m_dwRecvLimit / 1024;}
public:

	BOOL						_KillProcess(DWORD processId);
	BOOL 						_IsKingsoftWeiShiProcess(LPCWSTR szProcessPath);
	BOOL 						_IsSystemProcess(LPCWSTR szProcessPath);
	BOOL 						_IsSystemProcess(DWORD pid);
	BOOL 						_IsNetMonEnabled();
	enumProcessType 			_GetProcessType(KProcFluxItem processFlowItem);
};



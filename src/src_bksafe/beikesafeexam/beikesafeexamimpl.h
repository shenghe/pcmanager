#pragma once

#include <safeexam/beikesafeexam.h>
#include <com_s/com/comobject.h>
#include <iefix/iefixeng.h>
using namespace IEScanEng;
#include "examcheckitemsex.h"
#include <bdx/bdx.h>
#include "iefix/iefixNewUI.h"// 新的IE修复接口 [8/20/2010 zhangbaoliang]
#include "runoptimize/interface.h"
#include "kws/KwsSettingOld.h"
#include <scan/KLogic.h>


BDX_DEF_BEGIN(BkSafeExamLib::LocalLib)
BDX_DEF_LAZY(nType)
BDX_DEF_LAZY(strValue)
BDX_DEF_END()

BDX_DEF_BEGIN(BkSafeExamLib::LocalLibInfo)
BDX_DEF_LAZY(strCaption)
BDX_DEF_LAZY(arrLib)
BDX_DEF_END()

class CBkSafeExaminer
    : public IBkSafeExaminer
	, public IKSWebIEFixEngCallbackNull
//	, public IEnumRunCallBackNull
{
public:
    CBkSafeExaminer()
        : m_hEventStop(NULL)
        , m_piCallback(NULL)
        , m_bSlowMode(FALSE)
		, m_bInitIEFix(FALSE)
		, m_pRunOptEng(NULL)
    {
		m_arrIgnoreItems.RemoveAll();
		m_bkLocalLib.arrLib.RemoveAll();
		_LoadLocalLib(m_bkLocalLib);
    }

	~CBkSafeExaminer();
    SCOM_BEGIN_COM_MAP(CBkSafeExaminer)
        SCOM_INTERFACE_ENTRY(IBkSafeExaminer)
    SCOM_END_COM_MAP()

protected:

    HANDLE						m_hEventStop;
    IBkSafeExamineCallback *	m_piCallback;
    CString						m_strAppPath;
    BOOL						m_bSlowMode;

    // IBkSafeExaminer Implement
    virtual HRESULT WINAPI Initialize(LPVOID pvParam);
    virtual HRESULT WINAPI Uninitialize();
    virtual HRESULT WINAPI Examine(IBkSafeExamineCallback *piCallback, HANDLE hStopEvent, BOOL bSlowMode = FALSE);
    virtual HRESULT WINAPI ExpressExamine(IBkSafeExamineCallback *piCallback, HANDLE hStopEvent);
    virtual HRESULT WINAPI Echo(IBkSafeExamineCallback *piCallback, HANDLE hStopEvent);
	virtual HRESULT WINAPI FixItem(int nItemID, void* pExData = NULL);
	virtual HRESULT WINAPI ResetIgnoreArray();
	virtual HRESULT WINAPI PushAnIgnoreItem(int nItemID);
	virtual HRESULT WINAPI GetItemCount(int nItemID, int& nCount);
	virtual HRESULT WINAPI EnumItemData(int nItemID, int nIndex, void** pData);
	virtual HRESULT WINAPI SetItemSafe(int nItemID, void** pData);
	virtual HRESULT WINAPI SetHostItemFix(int nIndex);
	virtual HRESULT WINAPI RemoveLocalLibItem(int nItemID, void* pValue/*LPCTSTR pszValue*/);
	virtual HRESULT WINAPI GetSafeLibItemCount(int& nCount);
	virtual HRESULT WINAPI EnumSafeLibItem(int nIndex, BkSafeExamLib::LocalLibExport& libItem);
	virtual HRESULT WINAPI LocalLibIsHaveItem(int nItemID, BOOL& bHave);
	virtual HRESULT WINAPI StartupAnUse(int nItemID);
	virtual HRESULT WINAPI ExamItem(int nItemID, BKSafeExamItemEx::BKSafeExamEx& ExamItemData);




	void				_ReportResult(int nItem, int nLevel, BKSafeExamItemEx::BKSafeExamEx* pExamItemEx = NULL);
    void				_ReportLocalResult(
											int     nItem, 
											int     nLevel, 
											LPCWSTR lpszItemNoticeText, 
											LPCWSTR lpszOperationText, 
											LPCWSTR lpszItemNavigateString, 
											LPCWSTR lpszItemDetailUrl);
    void				_Echo(HANDLE hEventStop);
    static BOOL			_IsStopping(HANDLE hStopEvent, DWORD dwWait = 0);
    static DWORD WINAPI _ExaminationThreadProc_1(LPVOID pvParam);//检测实时保护状态，官方版本
    static DWORD WINAPI _ExaminationThreadProc_2(LPVOID pvParam);//体检中木马扫描快速检测
    static DWORD WINAPI _ExaminationThreadProc_3(LPVOID pvParam);//体检模块中的漏洞扫描
    static DWORD WINAPI _ExaminationThreadProc_4(LPVOID pvParam);//体检功能，插件快速扫描
	static DWORD WINAPI _ExaminationThreadProc_5(LPVOID pvParam);//体检IE修复中的项目
	static DWORD WINAPI _ExaminationThreadProc_6(LPVOID pvParam);//体检扩展项目(共享资源，GUEST账户，远程桌面)
	static DWORD WINAPI _ExaminationThreadProc_7(LPVOID pvParam);//体检垃圾
	static DWORD WINAPI _ExaminationThreadProc_8(LPVOID pvParam);//启动项优化
	static DWORD WINAPI _ExaminationThreadProc_9(LPVOID pvParam);//检测软件升级
	static DWORD WINAPI _ExaminationThreadProc_10(LPVOID pvParam);//检测杀毒软件
	static int   WINAPI IEFixNotifyMessage(ULONG uMsg, WPARAM wParam, LPARAM lParam, void* lpConext);//IE扫描的回调接口

public:// new iefix interface [8/20/2010 zhangbaoliang]
	virtual VOID		KsIeFix_ScanStart();
	virtual	VOID		KsIeFix_ScanEnd(BOOL bStopped);
	virtual VOID		KsIeFix_ScanProgress(KSWebIEFixProgressData* pData);

	//启动项管理
/*
	virtual BOOL FindRunItem(CKsafeRunInfo* pInfo){return FALSE;};
	virtual void BeginScan(DWORD nType){return;}
	virtual void EndScan(){return;}
	virtual void RestoreEnd(){return;}
	virtual void PostRestoreRunItem(DWORD nType,BOOL bEnable,LPCTSTR lpName){return;}
*/

	
public:
	BOOL										m_bInitIEFix;
	CIEFixEng									m_IEFixEng;
	CExamCheckItemsEx							m_examEx;
	CSimpleArray<SCAN_RESULT_EXPORT>			m_arrScanResult;
	CSimpleArray<int>							m_arrIgnoreItems;
	CSimpleArray<BkSafeExamLib::HostFileItem>	m_arrHostFileItem;
	CAtlArray<KSWebIeScanData>					m_arrKwsScanResult;
	BkSafeExamLib::LocalLibInfo					m_bkLocalLib;
	int											m_nTrojanProgress;
	int											m_nKwsProgress;
	IRunOptimizeEng*							m_pRunOptEng;
	//int											m_nInitComErrorCode;

	int		GetExamItemIDFromGroupID(int nGroupID, int nSubItemID);
	void	ReportScanResult(CSimpleArray<SCAN_RESULT_EXPORT> arrScanResult);
	BOOL	ItemIsIgnored(int nItemID);
	int		ExamItemEx();
	int		FilterWithLocalLib(int nGroupID, CString strValue);
	int		SaveHostFile();
	int		HasTrash();//返回值<0调用失败，=0没有 >0有
	int		CheckSoftWareUpdate(int& nUpdateCount);
	void    _CheckKwsExamResult();
	int		_IsAllKwsOpen();
private:
	int		_HostsItemIsSafe(LPCTSTR pszHostsItem, LPCTSTR pszLibItem);
	int		_SetHostItemSafe(BkSafeExamLib::HostFileItem* pHostItem, int nLevel = 0);
	int		_SetHostItemUnSafe(BkSafeExamLib::HostFileItem* pHostItem);
	int		_LoadLocalLib(BkSafeExamLib::LocalLibInfo& libLocal);
	int		_SaveToLocalLib(BkSafeExamLib::LocalLibInfo& libLocal);
	void	_LoadLocalInfo();
	void	_ResetArray();
	int		_NotifyKwsScanResult(int nItem, KSWebIeScanData& scanData);
	BOOL	_IsVirusLevel(const EM_POINT_LEVEL& emLevel);
	
};

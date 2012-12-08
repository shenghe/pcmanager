#pragma once
#include <vulfix/Defines.h>
#include "UserPatcher.h"
#include <libdownload\libDownload.h>
#include "../../src_commonlib/libDownload2/LocalFileDownload.h"
#include "FileDownloader.h"
#include <winmod/wintrustverifier.h>

// #define _USE_SINGLE_THREAD_DOWNLOAD_

class CUserPatcher;
class CImplVulIgnore;
struct T_RepairItem
{
	T_RepairItem(LPTUpdateItem pItem)
	{
		type = pItem->m_nType;
		nID = pItem->nID;
		strURL = pItem->detail.strDownurl;
        strMirrorURL = pItem->detail.strMirrorurl;
		strReferer = pItem->strWebpage;
		
		if(!pItem->detail.strInstallParam.IsEmpty())
			strInstallParam = pItem->detail.strInstallParam;
		else
			strInstallParam = pItem->strInstallparam;
		stateDownload = stateInstalled = -1;
		//bUserPatcher = pItem->bUserPatcher;
		bUserPatcher = pItem->m_nType==VTYPE_OFFICE;
		strCondtion = pItem->strCondition;
        bCheckCert = pItem->detail.bCheckCert;
	}
	T_RepairItem(LPTVulSoft pItem)
	{
		type = VTYPE_SOFTLEAK;
		bDisableCOM = (pItem->matchedItem.nAction==SOFTACTION_DISABLECOM);
		
		nID = pItem->nID;
		strURL = pItem->matchedItem.strDownUrl;
		strReferer = pItem->matchedItem.strDownpage;
		stateDownload = stateInstalled = -1;
		strCondtion = pItem->matchedItem.conditon.m_strCondition;
		bUserPatcher = FALSE;
        bCheckCert = FALSE;
	}
	
	int nID;	//soft id, vul id 
	int type;	// soft, or 
	
	CString strURL, strMirrorURL, strReferer;
	CString strFilename, strInstallParam;
	CString strCondtion;

	int stateDownload;	// -1, 0, 1		未下载, 下载失败, 下载成功 
	int stateInstalled;	
	BOOL bUserPatcher;
	BOOL bDisableCOM;	// 是否禁用COM 解决 
    BOOL bCheckCert;
};

class CImplRepairVul : public IRepairVul, public IHttpAsyncObserver, public IFileDownloaderObserver
{
public:
	CImplRepairVul(void);
	~CImplRepairVul(void);
	
public:
	virtual HRESULT Reset();
	virtual HRESULT SetObserver(IVulfixObserver *pObserver);
	virtual HRESULT AddVul(IVulfix *pVulfix, const IntArray&arrayId);
	virtual HRESULT AddSoftVul(IVulfix *pSoftVulfix, const IntArray&arrayId);
	virtual HRESULT Repair(LPCTSTR sDownloadPath, LPCTSTR szImportPath, DWORD dwFlags);
	virtual HRESULT StopRepair();
	virtual HRESULT WaitForRepairDone();
	virtual HRESULT GetItemFilePath(INT nID, BSTR *bstr);

protected:
	void Notify(TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2);
	
public:
	virtual BOOL OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam );
	void OnFileDownloaderEvent( IFileDownloaderObserver::TFileDownloaderEventType evt, DWORD dwParam1, DWORD dwParam2, PVOID pUserData );

protected:
	static DWORD WINAPI _ThreadFuncDownload(LPVOID lpVoid);
	static DWORD WINAPI _ThreadFuncInstall(LPVOID lpVoid);
	DWORD _ThreadFuncDownloadImpl();
	DWORD _ThreadFuncInstallImpl();
	INT _UserPatch( T_RepairItem &item );

    HRESULT _VerifyWinTrustCert(LPCTSTR lpszPatchFullFileName);
	
	enum SRSP_FIX_DEFINE{
		SRSP_FIX_DEFINE_WINDOWS = 1,
		SRSP_FIX_DEFINE_SYSTEM32,
	};

	typedef	SRSP_FIX_DEFINE		SRSP_SUBMOD_FIX_DEFINE;

	void _FixSystemRootSpecialPath( T_RepairItem &item );
	void _IfFileExistDelete( LPCTSTR lpszFileName, SRSP_FIX_DEFINE nDirDef );
	void _FixPrefixFile( LPCTSTR lpszTargetDir, LPCTSTR lpszPrefix );
	void _FormatKBString( INT nKB, CString &str );

protected:
	BOOL m_bCanceled;
	HANDLE m_hHandleDownload, m_hHandleInstall;
    BOOL m_bDownloadHasReceivedData;
	CString m_strSystemRoot;
	
public:
	IVulfixObserver *m_Observer;	
	CString m_strDownloadPath;	// 下载的存放路径 
	CString m_strImportPath;	// 
	DWORD m_dwRepairFlags;		

	CSimpleArray<T_RepairItem> m_arrDownloadItem;
	CUserPatcher *m_pUserPatcher;
	IVulfix *m_pCurrentVulfix;

	CImplVulIgnore &m_objIgnore;
	
	// Install info  	
	struct TDownloadAndInstallInfo {
		
		HANDLE m_hInstallEvent;
        CFileDownloader *m_pFileDownloaderSingle;
#ifndef _USE_SINGLE_THREAD_DOWNLOAD_
		IDownload *m_pFileDownloader;
#endif
		CObjLock m_objLockFileDownloader;

		CLocalFileDownload m_fileDownloadLocal;
		
		int totalNum;
		int nInstallIndex;
		int nDownSuccess, nInstSuccess;
		BOOL isDownloadDone, isInstallDone;		

		TDownloadAndInstallInfo()
		{
			m_hInstallEvent = NULL;
			Reset();
		}
		
		void Reset()
		{
            m_pFileDownloaderSingle = NULL;
			m_pFileDownloader = NULL;
			totalNum = nInstallIndex = nDownSuccess = nInstSuccess = 0;
			isDownloadDone = isInstallDone = FALSE;
		}
	}m_thinfo;
};

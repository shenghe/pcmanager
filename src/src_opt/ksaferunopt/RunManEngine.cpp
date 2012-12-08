#include "StdAfx.h"
#include "RunManEngine.h"
#include "runhelper.h"
#include "servicehelper.h"
#include "taskjobhelper.h"
#include <communits/CmdLineParser.h>
#include <common/utility.h>
#include "disablerunhelper.h"
#include <runoptimize/restorerunitem.h>
#include <common/RegKeyConvertString.h>
#include <communits/Vista.h>
#include <iefix/cinifile.h>
using namespace IniFileOperate;

#pragma comment(lib,"Version.lib")

#define IGNORED_FILEPATH			L"\\Cfg\\ignoredlist.ini"
#define SEC_IGNOREDLIST_MAIN		L"Main"
#define KEY_IGNOREDLIST_COMMENT		L"Comment"

void _GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet)
{
	BOOL bRet = FALSE;
	UINT uCount = 0;
	UINT uRetSize = MAX_PATH - 1;
	LPWSTR lpszValue = NULL;

	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	bRet = ::VerQueryValue(pbyInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &uCount);
	if (!bRet || uCount == 0)
		return;

	CString strSubBlock;

	strSubBlock.Format(
		L"\\StringFileInfo\\%04x%04x\\%s",
		lpTranslate[0].wLanguage,
		lpTranslate[0].wCodePage, 
		lpszFileInfoKey
		);
	bRet = ::VerQueryValue(
		pbyInfo, (LPWSTR)(LPCWSTR)strSubBlock, 
		(LPVOID *)&lpszValue, &uRetSize
		);
	if (!bRet)
		return;

	wcsncpy(lpszFileInfoRet, lpszValue, MAX_PATH + 1);
}

BOOL _GetPEFileInfo(LPCWSTR lpszFileName,LPCWSTR lpSection, CString &strProductVersion)
{
	BOOL bResult = FALSE;
	BOOL bRet = FALSE;
	DWORD dwHandle          = 0;
	DWORD dwFileInfoSize    = 0;
	VS_FIXEDFILEINFO *pFixFileInfo = NULL;
	BYTE *pbyInfo           = NULL;

	dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
	if (0 == dwFileInfoSize)
		goto Exit0;

	pbyInfo = new BYTE[dwFileInfoSize];
	if (!pbyInfo)
		goto Exit0;

	bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
	if (!bRet)
		goto Exit0;

	_GetFileInfo(pbyInfo, lpSection, strProductVersion.GetBuffer(MAX_PATH + 1));
	strProductVersion.ReleaseBuffer();

	bResult = TRUE;

Exit0:

	if (pbyInfo)
	{
		delete[] pbyInfo;
		pbyInfo = NULL;
	}

	return bResult;
}

void LoadIgnoredID(CSimpleArray<int>& arrayIgnoredID)
{
	arrayIgnoredID.RemoveAll();
	CString strIgnoredIniPath;
	CAppPath::Instance().GetLeidianAppPath(strIgnoredIniPath);
	strIgnoredIniPath.Append(IGNORED_FILEPATH);
	CIniFile ini_IgnoredList(strIgnoredIniPath);
	CString strGetValue;
	ini_IgnoredList.GetStrValue(SEC_IGNOREDLIST_MAIN,KEY_IGNOREDLIST_COMMENT,strGetValue.GetBuffer(65536),65536);
	strGetValue.ReleaseBuffer(65536);

	WCHAR *szValue;
	WCHAR szTemp[10];
	szValue = strGetValue.GetBuffer();
	strGetValue.ReleaseBuffer();
	WCHAR* p = wcstok(szValue,L"|");
	while(p)
	{
		wcscpy_s(szTemp,p);
		arrayIgnoredID.Add(_wtoi(szTemp));
		p = wcstok(NULL,L"|");
	}
}

BOOL HasIgnored(int nID,CSimpleArray<int>& arrayIgnoredID)
{
	BOOL bFind = FALSE;
	for (int i = 0; i < arrayIgnoredID.GetSize();i++)
	{	
		if (nID == arrayIgnoredID[i])
		{
			bFind = TRUE;
			goto Exit0;
		}
	}
Exit0:
	return bFind;
}

void ConvertLib(CKsafeRunInfo& cinfo, KSRunInfo& libinfo)
{
	cinfo.nAdvice			= libinfo.nAdvice;
	cinfo.bSystem			= libinfo.bSystem;
	cinfo.bNoDealyShow		= libinfo.bNoDealyShow;
	cinfo.bUnknown			= FALSE;
	cinfo.nID				= libinfo.nItemId;
	cinfo.bShowRemain		= libinfo.bShowRemain;
	cinfo.nExtraFlag		= libinfo.nExtraFlag;
	cinfo.strOpenRate		= libinfo.strOpenRate;
	cinfo.nUserCount		= libinfo.nUserCount;

	if (!libinfo.strDesc.IsEmpty())
		cinfo.strDesc		= libinfo.strDesc;
	if (!libinfo.strDisplay.IsEmpty())
		cinfo.strDisplay	= libinfo.strDisplay;
	if (!libinfo.strWarning.IsEmpty())
		cinfo.strWarning	= libinfo.strWarning;
	if (cinfo.IsAdviseStop() || cinfo.IsCanDelay())
	{
		CSimpleArray<int> arrayIgnoredId;
		LoadIgnoredID(arrayIgnoredId);
		cinfo.bHasIgnored = HasIgnored(cinfo.nID,arrayIgnoredId);
	}
}

void PostGetItemDesc(CKsafeRunInfo& cinfo)
{
	if (!cinfo.strDesc.IsEmpty())
		return;

	if (cinfo.strExePath.IsEmpty())
		return;

	CString	strDesc;
	CString	strProduct;

	_GetPEFileInfo(cinfo.strExePath, L"ProductName", strProduct);
	_GetPEFileInfo(cinfo.strExePath, L"FileDescription", strDesc);

	if (!strProduct.IsEmpty())
	{
		cinfo.strDesc = strProduct;
	}
	if ( strProduct.CompareNoCase(strDesc) == 0 || strDesc.IsEmpty() )
	{
		return;
	}
	if (!cinfo.strDesc.IsEmpty())
		cinfo.strDesc.Append(_T(" - "));
	cinfo.strDesc.Append(strDesc);
}

CRunManEngine::CRunManEngine(void)
{
	m_wow64Switcher.Init();
	m_pLibLoader = NULL;
	m_hOpThread  = NULL;
	m_pLibLoader2 = NULL;
}

CRunManEngine::~CRunManEngine(void)
{
	if (m_hOpThread!=NULL)
	{
		::TerminateThread(m_hOpThread,0);
		m_hOpThread = NULL;
	}

	if (m_hOpThread_Exam!=NULL)
	{
		::TerminateThread(m_hOpThread_Exam,0);
		m_hOpThread_Exam = NULL;
	}

	if ( m_pLibLoader != NULL )
	{
		delete m_pLibLoader;
		m_pLibLoader = NULL;
	}

	if (m_pLibLoader2 != NULL)
	{
		delete m_pLibLoader2;
		m_pLibLoader2 = NULL;
	}
}

void CRunManEngine::EnumRegRunner(BOOL bEnable, CRunRegEnumer* pEnumer, IEnumRunCallBack* pCallback,CRunRegEnumer* pFilter)
{
	CRestoreRunner pItem;

	if (pEnumer->DoEnum())
	{
		for ( int i = 0; i < pEnumer->GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CRunRegEnumer::REG_RUN_INFO&	runreginfo = pEnumer->GetItem(i);

			if (runreginfo.strPath.IsEmpty())//如果路径为空，会被CommandLineToArgvW解析到当前路径,所以就跳过
				continue;
			if (pFilter != NULL)
			{
				BOOL	bExist = FALSE;
				for ( int k = 0; k< pFilter->GetCount(); k++)
				{
					CRunRegEnumer::REG_RUN_INFO&	cmpInfo = pFilter->GetItem(k);
					if ( IsSameRunType(cmpInfo.nRegType, runreginfo.nRegType) )
					{
						if (runreginfo.strName.CompareNoCase(cmpInfo.strName)==0)
						{
							bExist = TRUE;
							break;
						}
					}
				}
				if (bExist)
					continue;
			}

			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Close();
			}

			CRunoptCmdLineParser cmdLine(runreginfo.strPath);
			CString strAppName = cmdLine.GetCmd();
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Revert();
			}
			cinfo.nType		= KSRUN_TYPE_STARTUP;
			cinfo.strName	= runreginfo.strName;
			cinfo.strDisplay= runreginfo.strName;
			cinfo.strPath	= strAppName;
			cinfo.strParam	= runreginfo.strPath;
			cinfo.strExePath= strAppName;
			cinfo.nRegType	= runreginfo.nRegType;
			cinfo.bDisabledBy3xx = runreginfo.bDisabledBy3XX;
			cinfo.bFileIsExist = cmdLine.IsPEFileExist();

			RunLogItem* pSetting = NULL;

			pSetting = pItem.GetStartUpItem(runreginfo.strName);
			if (pSetting != NULL && pSetting->dwNewState == KSRUN_START_DELAY)
				cinfo.bEnable = pSetting->dwNewState;
			else
				cinfo.bEnable	= bEnable;
			if ((bEnable == FALSE) && (cinfo.bFileIsExist == FALSE))//如果是已禁用的启动项，而且目标文件不存在，那就不管他
				continue;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_STARTUP,runreginfo.strName,strAppName),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);
			PostGetItemDesc(cinfo);
			if (bEnable == KSRUN_START_DELAY && cinfo.bFileIsExist == FALSE)
			{
				CRestoreRunner* pRunner = new CRestoreRunner;
				pRunner->DeleteItemFromLog(cinfo.strName, KSRUN_TYPE_STARTUP, cinfo.nID);
			}
			if (pCallback)
			{
// 				if ((cinfo.bFileIsExist == FALSE) && (cinfo.bShowRemain == FALSE))
// 					continue;
// 				else

					pCallback->FindRunItem(&cinfo);
			}
		}
	}
}

void CRunManEngine::EnumDirRunner( BOOL bEnable, CRunDirEnumer* pEnumer, IEnumRunCallBack* pCallback,CRunDirEnumer* pFilter )
{
	CRestoreRunner pItem;
	if (pEnumer->DoEnum())
	{
		for ( int i = 0; i < pEnumer->GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CRunDirEnumer::RUN_DIR_INFO&	rundirinfo = pEnumer->GetItem(i);

			if (rundirinfo.strPath.IsEmpty())//如果路径为空，会被CommandLineToArgvW解析到当前路径,所以就跳过
				continue;

			if (pFilter != NULL)
			{
				BOOL	bExist = FALSE;
				for ( int k = 0; k< pFilter->GetCount(); k++)
				{
					CRunDirEnumer::RUN_DIR_INFO&	cmpInfo = pFilter->GetItem(k);
					if ( IsSameRunType(cmpInfo.nDirType, rundirinfo.nDirType) )
					{
						if (GetFileName(rundirinfo.strName).CompareNoCase(GetFileName(cmpInfo.strName))==0)
						{
							bExist = TRUE;
							break;
						}
					}
				}
				if (bExist)
					continue;
			}
			
			CString strParam;
			strParam.Format(_T("%s %s"),rundirinfo.strPath,rundirinfo.strParam);

			cinfo.nType			= KSRUN_TYPE_STARTUP;
			cinfo.strName		= GetFileName(rundirinfo.strName);
			cinfo.strPath		= rundirinfo.strName;	// 填写全路径
			
			RunLogItem* pSetting = NULL;

			pSetting = pItem.GetStartUpItem(cinfo.strName);
			if (pSetting != NULL && pSetting->dwNewState == KSRUN_START_DELAY)
				cinfo.bEnable = pSetting->dwNewState;
			else
				cinfo.bEnable	= bEnable;

			cinfo.strExePath	= rundirinfo.strPath;	// LNK指向的EXE路径
			cinfo.strParam		= strParam;	
			cinfo.strDisplay	= cinfo.strName;
			cinfo.nRegType		= rundirinfo.nDirType;
			if (!m_wow64Switcher.IsWin64_2())
				cinfo.bFileIsExist	= ::PathFileExists(rundirinfo.strPath);
			if (bEnable == FALSE && cinfo.bFileIsExist == FALSE)
				continue;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_STARTUP,rundirinfo.strName,rundirinfo.strPath),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);
			PostGetItemDesc(cinfo);
			if (pCallback)
			{
// 				if ((cinfo.bFileIsExist == FALSE) && (cinfo.bShowRemain == FALSE))
// 					continue;
// 				else
					pCallback->FindRunItem(&cinfo);
			}
		}
	}
}

void CRunManEngine::_InternEnumRunInfo( DWORD dwFlags,IEnumRunCallBackNull2* pCallback )
{
	if (m_pLibLoader==NULL)
	{
		m_pLibLoader = new CLibLoader;
		m_pLibLoader->LoadLib(NULL);
	}

	switch (dwFlags)
	{
	case 0:
		break;
	case KSRUN_TYPE_STARTUP:
		goto Sub_Run;
		break;
	case KSRUN_TYPE_SERVICE:
		goto Sub_Srv;
		break;
	case KSRUN_TYPE_TASK:
		goto Sub_Tasks;
		break;
	default:
		break;
	}

Sub_Run:
	if (TRUE)
	{
		CKSRunDirEnumer	ksdirEnumer;
		ksdirEnumer.SetCopyFilter(this);
		EnumDirRunner(FALSE,&ksdirEnumer,pCallback);

		CRunDirEnumer	dirEnumer;
		EnumDirRunner(TRUE,&dirEnumer,pCallback,&ksdirEnumer);

		CKSRunRegEnumer	ksregEnumer;
		ksregEnumer.SetCopyFilter(this);
		EnumRegRunner(FALSE,&ksregEnumer,pCallback);

		CRunRegEnumer	regEnumer;
		EnumRegRunner(TRUE,&regEnumer,pCallback, &ksregEnumer);
		
		if (dwFlags == 0)
		{
			if (pCallback)
				pCallback->EndScan(KSRUN_TYPE_STARTUP);
		}
		else if (dwFlags != 4)
		{
			goto Exit0;
		}
	}

Sub_Srv:
	
	if (TRUE)
	{
		CServiceEnumer srvEnumer;
		EnumServiceRunner(&srvEnumer, pCallback);

		if (dwFlags == 0)
		{
			if (pCallback)
				pCallback->EndScan(KSRUN_TYPE_SERVICE);
		}
		else if (dwFlags != 4)
		{
			goto Exit0;
		}
	}

Sub_Tasks:
	if (TRUE)
	{
		CTaskSchedularEnumer	taskEnumer;
		taskEnumer.SetCopyFilter(this);
		if (taskEnumer.DoEnum())
		{
			for ( int i = 0; i < taskEnumer.GetCount(); i++)
			{
				CKsafeRunInfo	cinfo;
				CTaskSchedularEnumer::TASK_JOB_INFO&	jobInfo = taskEnumer.GetItem(i);

				if (jobInfo.strExePath.IsEmpty())//如果路径为空，会被CommandLineToArgvW解析到当前路径,所以就跳过
					continue;

				if (m_wow64Switcher.IsWin64())
				{
					m_wow64Switcher.Close();
				}

				CRunoptCmdLineParser cmdLine(jobInfo.strExePath);
				CString strAppPath = cmdLine.GetCmd();

				if (m_wow64Switcher.IsWin64())
				{
					m_wow64Switcher.Revert();
				}

				cinfo.nType		= KSRUN_TYPE_TASK;
				cinfo.strName	= jobInfo.strJobPath;
				cinfo.strPath	= strAppPath;
				cinfo.strDisplay= jobInfo.strName;
				cinfo.strParam	= jobInfo.strParam;
				cinfo.bEnable	= jobInfo.bEnable;
				cinfo.strExePath= strAppPath;
				cinfo.nJobType	= jobInfo.nType;
				cinfo.strDesc	= jobInfo.strDesc;
				cinfo.bFileIsExist = cmdLine.IsPEFileExist();

				KSRunInfo*	pKSInfo = NULL;
				if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_TASK,jobInfo.strJobPath,strAppPath),&pKSInfo))
					ConvertLib(cinfo,*pKSInfo);

				PostGetItemDesc(cinfo);
				if (pCallback)
				{
// 					if ((cinfo.bFileIsExist == FALSE) && (cinfo.bShowRemain == FALSE))
// 						continue;
// 					else
						pCallback->FindRunItem(&cinfo);
				}
			}
		}

		if (dwFlags == 0)
		{
			if (pCallback)
				pCallback->EndScan(KSRUN_TYPE_TASK);
		}
		else if (dwFlags != 4)
		{
			goto Exit0;
		}
	}

	if (dwFlags == 4)
	{
		EnumSysConfigInfo(dwFlags, pCallback);
	}

Exit0:
	if (dwFlags != 0)
	{
		if (pCallback)
			pCallback->EndScan(dwFlags);
	}

	if (0 == dwFlags)
		pCallback->EndScan();
	m_hOpThread = NULL;
	return;
}

struct THREAD_PARAM
{
	DWORD				dwFlags;
	IEnumRunCallBack*	Callback;
	CRunManEngine*		pThis;
};

struct THREAD_PARAM2
{
	DWORD				dwFlags;
	IEnumRunCallBackNull2*	Callback;
	CRunManEngine*		pThis;
};

void CRunManEngine::EnumRunInfo( DWORD dwFlags,IEnumRunCallBack* pCallback )
{
	THREAD_PARAM* pt = new THREAD_PARAM;

	pt->Callback	= pCallback;
	pt->dwFlags		= dwFlags;
	pt->pThis		= this;
	m_hOpThread = (HANDLE)_beginthread(threadstub,0,(PVOID)pt);
	return;
}

void CRunManEngine::EnumRunInfoForExam(DWORD dwFlags,IEnumRunCallBack* pCallback )
{
	THREAD_PARAM* pt = new THREAD_PARAM;

	pt->Callback	= pCallback;
	pt->dwFlags		= dwFlags;
	pt->pThis		= this;
	m_hOpThread_Exam = (HANDLE)_beginthread(threadstub_Exam,0,(PVOID)pt);
	return;
}

void CRunManEngine::threadstub( PVOID lParam )
{
	THREAD_PARAM2* pt = (THREAD_PARAM2*)lParam;
	pt->pThis->_InternEnumRunInfo(pt->dwFlags,pt->Callback);
	delete pt;
	return;
}

void CRunManEngine::threadstub_Exam( PVOID lParam )
{
	THREAD_PARAM* pt = (THREAD_PARAM*)lParam;
	pt->pThis->_InternEnumRunInfo_Exam(pt->dwFlags,pt->Callback);
	delete pt;
	return;
}

struct _ENUM_RESTORE_PARAM
{
	DWORD				nType;
	IEnumRunCallBack*	pCB;
};

BOOL CRunManEngine::EnumLog( RunLogItem* pItem, PVOID lParam )
{
	_ENUM_RESTORE_PARAM*	ptrParam = (_ENUM_RESTORE_PARAM*)lParam;
	BOOL  bNoDel = TRUE;
	DWORD nType  = ptrParam->nType;
	if ( nType == pItem->nType)
	{
		CRestoreRunner* pLogRunner = new CRestoreRunner;
		if (pItem->nType==KSRUN_TYPE_SERVICE && !pItem->strName.IsEmpty() )
		{
			pLogRunner->DeleteItemFromLog(pItem->strName, pItem->nType, pItem->nID);
			delete pLogRunner;
			if ( CServiceOper().ControlRun(pItem->nInitState,pItem->strName) )
				bNoDel = FALSE;
		}
		if (pItem->nType==KSRUN_TYPE_TASK && !pItem->strName.IsEmpty() )
		{
			pLogRunner->DeleteItemFromLog(pItem->strName, pItem->nType, pItem->nID);
			delete pLogRunner;
			if ( CTaskSchedularOper().ControlRun(pItem->nInitState,pItem->strName,pItem->nExtType) )
				bNoDel = FALSE;
		}
		if (pItem->nType==KSRUN_TYPE_STARTUP && !pItem->strName.IsEmpty() )
		{
			pLogRunner->DeleteItemFromLog(pItem->strName, pItem->nType, pItem->nID);
			delete pLogRunner;
			DWORD	nControlType = pItem->nExtType;
			BOOL	bRes = FALSE;
			CString	strName = pItem->strName;

			if (nControlType<DIR_RUN_TYPE_BEGIN)
				bRes = CRegRunOper().ControlRun2(KSRUN_START_ENABLE, pItem->strName2, nControlType, strName, this);
			else
				bRes = CDirRunOper().ControlRun(KSRUN_START_ENABLE, nControlType, strName, this);

			if ( bRes )
				bNoDel = FALSE;

			if (bRes && ptrParam->pCB )
			{
				ptrParam->pCB->PostRestoreRunItem(pItem->nInitState, KSRUN_START_ENABLE, strName);
			}
		}
	}
	if (nType == KSRUN_TYPE_RESTORE_DELAY_RUN)
	{
		if (pItem->dwNewState == KSRUN_START_DELAY)
		{
			CRestoreRunner* pLogRunner = new CRestoreRunner;
			pLogRunner->DeleteItemFromLog(pItem->strName, pItem->nType, pItem->nID);
			delete pLogRunner;
			DWORD	nControlType = pItem->nExtType;
			BOOL	bRes = FALSE;
			CString	strName = pItem->strName;

			if (nControlType<DIR_RUN_TYPE_BEGIN)
				bRes = CRegRunOper().ControlRun2(KSRUN_START_ENABLE, pItem->strName2, nControlType, strName, this);
			else
				bRes = CDirRunOper().ControlRun(KSRUN_START_ENABLE, nControlType, strName, this);

			if ( bRes )
				bNoDel = FALSE;

			if (bRes && ptrParam->pCB )
			{
				ptrParam->pCB->PostRestoreRunItem(pItem->nInitState, KSRUN_START_ENABLE, strName);
			}
		}
	}
	return bNoDel;
}
void CRunManEngine::RestoreRun( DWORD nType,IEnumRunCallBack* pCallback )
{
	if (TRUE)
	{
		_ENUM_RESTORE_PARAM	xparam;

		CRestoreRunner* pRunner = new CRestoreRunner;
		if (nType == KSRUN_TYPE_RESTORE_ALL)
		{
			xparam.pCB	 = pCallback;
			xparam.nType = KSRUN_TYPE_STARTUP;
			pRunner->EnumItem(this,&xparam);
			xparam.nType = KSRUN_TYPE_SERVICE;
			pRunner->EnumItem(this,&xparam);
			xparam.nType = KSRUN_TYPE_TASK;
			pRunner->EnumItem(this,&xparam);
		}
		else
		{
			xparam.nType = nType;
			xparam.pCB	 = pCallback;
			pRunner->EnumItem(this,&xparam);
		}
	}	

	if (pCallback)
	{
		pCallback->RestoreEnd();
	}
}

BOOL CRunManEngine::ControlRun( BOOL bEnable, CKsafeRunInfo* pInfo )
{
	BOOL	bRes = FALSE;
	CRestoreRunner* pLogRunner = new CRestoreRunner;
	DWORD dwState = FALSE;

	if (pInfo->nType==KSRUN_TYPE_SERVICE)
	{
		if (bEnable == KSRUN_START_ENABLE)
			dwState = TRUE;

		bRes = CServiceOper().ControlRun(dwState, pInfo->strName);
		if (bRes)
		{
			pLogRunner->OperateService(pInfo->strName,pInfo->nID,pInfo->bEnable,bEnable, pInfo->bSystem);
			pInfo->bEnable = !pInfo->bEnable;
		}
	}
	else if ( pInfo->nType==KSRUN_TYPE_STARTUP )
	{
		DWORD nOldType = pInfo->nRegType;
		CString strOutNewPath = pInfo->strName;

		if (bEnable == KSRUN_START_DELAY_FAIL)
			dwState = KSRUN_START_ENABLE;
		else
			dwState = bEnable;

		if (pInfo->nRegType < DIR_RUN_TYPE_BEGIN )
		{
			if ((pInfo->bEnable == KSRUN_START_DISABELE && dwState == KSRUN_START_DELAY) ||
				(pInfo->bEnable == KSRUN_START_DELAY && dwState == KSRUN_START_DISABELE))
				bRes = TRUE;
			else
				bRes = CRegRunOper().ControlRun(dwState, pInfo->nRegType, strOutNewPath, bEnable?this:NULL);
		}
		else
		{
			if ((pInfo->bEnable == KSRUN_START_DISABELE && dwState == KSRUN_START_DELAY) ||
				(pInfo->bEnable == KSRUN_START_DELAY && dwState == KSRUN_START_DISABELE))
				bRes = TRUE;
			else
			{
				bRes = CDirRunOper().ControlRun(dwState, pInfo->nRegType, strOutNewPath, bEnable?this:NULL);
				pInfo->strPath = strOutNewPath;
			}
		}
		
		if (bRes)
		{
			pLogRunner->OperateRun(pInfo->strName, pInfo->nID, nOldType, pInfo->nRegType, bEnable, pInfo->bSystem);
			pInfo->bEnable = bEnable;
		}
	}
	else if ( pInfo->nType==KSRUN_TYPE_TASK )
	{
		if (bEnable == KSRUN_START_ENABLE)
			dwState = TRUE;

		bRes = CTaskSchedularOper().ControlRun(dwState, pInfo->strName, pInfo->nJobType);
		if (bRes)
		{
			pLogRunner->OperateTaskJob(pInfo->strName,pInfo->nID,pInfo->nJobType, pInfo->bEnable,bEnable, pInfo->bSystem);
			pInfo->bEnable = !pInfo->bEnable;
		}
	}
	delete pLogRunner;
	return bRes;
}

void CRunManEngine::RestoreRegRunner( CRunRegEnumer* pEnumer )
{
	if (pEnumer->DoEnum())
	{
		for ( int i=0; i<pEnumer->GetCount(); i++)
		{
			CRunRegEnumer::REG_RUN_INFO item = pEnumer->GetItem(i);
			DWORD	nRegType = item.nRegType;
			CRegRunOper().ControlRun(TRUE,nRegType,item.strName,this);
		}
	}
}

void CRunManEngine::RestoreDirRuner( CRunDirEnumer* pEnumer )
{
	if (pEnumer->DoEnum())
	{
		for ( int i=0; i<pEnumer->GetCount(); i++)
		{
			CRunDirEnumer::RUN_DIR_INFO item = pEnumer->GetItem(i);
			DWORD	nRegType = item.nDirType;
			CDirRunOper().ControlRun(TRUE,nRegType,item.strName,this);
		}
	}
}

BOOL CRunManEngine::WriteRunKey( HKEY hRoot, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData )
{
	CSafeMonitorTrayShell().AddTrustRegPath(CRegStringConvert(hRoot,lpKey,lpValue,lpData).GetStr());
	return TRUE;
}

BOOL CRunManEngine::WriteRunFile( LPCTSTR lpFile )
{
	CSafeMonitorTrayShell().AddTrustFilePath(lpFile);
	return TRUE;
}

BOOL CRunManEngine::DeleteRun( CKsafeRunInfo* pInfo )
{
	BOOL bRes = FALSE;
	CRestoreRunner* pLogRunner = new CRestoreRunner;
	if (pInfo->nType == KSRUN_TYPE_TASK )
	{
		bRes = CTaskSchedularOper().DeleteTask(pInfo->strName, pInfo->nJobType);
		if (bRes)
		{
			CDelHistory::GetPtr()->LogDelTask(pInfo->nJobType,pInfo->strName);
			pLogRunner->DeleteItemFromLog(pInfo->strName,pInfo->nType,pInfo->nID);
		}
	}
	if (pInfo->nType == KSRUN_TYPE_SERVICE )
	{
		bRes = CServiceOper().DeleteService(pInfo->strName);

		if (bRes)
		{
			CDelHistory::GetPtr()->LogDelService(pInfo->strName);
			pLogRunner->DeleteItemFromLog(
				pInfo->strName, 
				pInfo->nType, 
				pInfo->nID
				);
		}
	}
	if (pInfo->nType == KSRUN_TYPE_STARTUP )
	{
		if (pInfo->nRegType < DIR_RUN_TYPE_BEGIN )
			bRes = CRegRunOper().DeleteRun(pInfo->nRegType, pInfo->strName);
		else
			bRes = CDirRunOper().DeleteRun(pInfo->nRegType, pInfo->strPath);

		if (bRes)
		{
			CDelHistory::GetPtr()->LogDelRun(pInfo->nRegType, pInfo->strName);
			pLogRunner->DeleteItemFromLog(pInfo->strName,pInfo->nType,pInfo->nID);
		}
	}
	delete pLogRunner;
	return bRes;
}

BOOL CRunManEngine::PreCopyFile( DWORD nType, LPCTSTR lpFileName )
{
	return !CDelHistory::GetPtr()->FindDelRunItem(nType, lpFileName);
}

BOOL CRunManEngine::PreCopyValue( DWORD nType, LPCTSTR lpValue )
{
	return !CDelHistory::GetPtr()->FindDelRunItem(nType, lpValue);
}

BOOL CRunManEngine::PreCopyTaskFile( DWORD nType, LPCTSTR lpFileName )
{
	return !CDelHistory::GetPtr()->FindDelTaskItem(nType,lpFileName);

}

VOID CRunManEngine::SynEnumRunInfo( DWORD dwFlags, IEnumRunCallBackNull2* pCallBack )
{
	_InternEnumRunInfo(dwFlags,pCallBack);
}

DWORD CRunManEngine::GetRunCount( DWORD nFlags )
{
	DWORD	nCount = 0;
	if (nFlags == KSRUN_TYPE_STARTUP || nFlags == KSRUN_TYPE_ALL)
	{
		CRunDirEnumer*	pDirEnumer = new CRunDirEnumer;
		if (pDirEnumer != NULL)
		{
			pDirEnumer->DoEnum();
			for ( int i = 0; i < pDirEnumer->GetCount(); i++)
			{
				CRunDirEnumer::RUN_DIR_INFO&	rundirInfo = pDirEnumer->GetItem(i);
				CCmdLineParser cmdLine(rundirInfo.strPath);
				if (cmdLine.GetCmd()==NULL)
					continue;
				nCount++;
			}
		}
		CRunRegEnumer*	pRegEnumer = new CRunRegEnumer;
		if (pRegEnumer != NULL)
		{
			pRegEnumer->DoEnum();
			for ( int i = 0; i < pRegEnumer->GetCount(); i++)
			{
				CRunRegEnumer::REG_RUN_INFO&	runregInfo = pRegEnumer->GetItem(i);
				CCmdLineParser cmdLine(runregInfo.strPath);
				if (cmdLine.GetCmd()==NULL)
					continue;
				nCount++;
			}
		}
		if (nFlags != KSRUN_TYPE_ALL)
			goto Exit0;
	}
	if (nFlags == KSRUN_TYPE_SERVICE || nFlags == KSRUN_TYPE_ALL)
	{
		CServiceEnumer srvEnumer;
		nCount += srvEnumer.GetRunSrvCount();
		if (nFlags != KSRUN_TYPE_ALL)
			goto Exit0;
	}
	if (nFlags == KSRUN_TYPE_TASK || nFlags == KSRUN_TYPE_ALL)
	{
		CTaskSchedularEnumer* pTaskEnumer = new CTaskSchedularEnumer;

		if (pTaskEnumer != NULL)
		{
			pTaskEnumer->DoEnum();
			for ( int i = 0; i < pTaskEnumer->GetCount(); i++)
			{
				CTaskSchedularEnumer::TASK_JOB_INFO&	jobInfo = pTaskEnumer->GetItem(i);
				CString strAppName = GetFileName(jobInfo.strExePath);
				if (::PathFileExists(jobInfo.strExePath) && jobInfo.bEnable)
					nCount++;
			}
		}
		if (nFlags != KSRUN_TYPE_ALL)
			goto Exit0;
	}
Exit0:
	return nCount;
}

void CRunManEngine::_InternEnumRunInfo_Exam( DWORD dwFlags,IEnumRunCallBack* pCallback )
{
	if (m_pLibLoader==NULL)
	{
		m_pLibLoader = new CLibLoader;
		m_pLibLoader->LoadLib(NULL);
	}

	if (TRUE)
	{
		CKSRunDirEnumer	ksdirEnumer;
		ksdirEnumer.SetCopyFilter(this);
		EnumDirRunner(FALSE,&ksdirEnumer,pCallback);

		CRunDirEnumer	dirEnumer;		
		if (EnumDirRunner2(TRUE,&dirEnumer,pCallback,&ksdirEnumer) > 0)
			goto Exit0;
	}

	if (TRUE)
	{
		CKSRunRegEnumer	ksregEnumer;
		ksregEnumer.SetCopyFilter(this);
		EnumRegRunner(FALSE,&ksregEnumer,pCallback);

		CRunRegEnumer	regEnumer;
		if (EnumRegRunner2(TRUE,&regEnumer,pCallback,&ksregEnumer) > 0)
			goto Exit0;
	}

	if (TRUE)
	{
		CServiceEnumer	servEnumer;
		if (EnumServiceRunner2(&servEnumer,pCallback) > 0)
			goto Exit0;
	}

	if (TRUE)
	{
		CTaskSchedularEnumer	taskEnumer;
		if (EnumTaskRunner2(&taskEnumer,pCallback) > 0)
			goto Exit0;
	}

Exit0:
	if (pCallback)
		pCallback->EndScan();

	m_hOpThread_Exam = NULL;
	return;
}

int CRunManEngine::EnumRegRunner2(BOOL bEnable, CRunRegEnumer* pEnumer, IEnumRunCallBack* pCallback, CRunRegEnumer* pFilter)
{
	int nRet = 0;

	if (pEnumer->DoEnum())
	{
		for ( int i = 0; i < pEnumer->GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CRunRegEnumer::REG_RUN_INFO&	runreginfo = pEnumer->GetItem(i);

			if (pFilter != NULL)
			{
				BOOL	bExist = FALSE;
				for ( int k = 0; k< pFilter->GetCount(); k++)
				{
					CRunRegEnumer::REG_RUN_INFO&	cmpInfo = pFilter->GetItem(k);
					if ( IsSameRunType(cmpInfo.nRegType, runreginfo.nRegType) )
					{
						if (runreginfo.strName.CompareNoCase(cmpInfo.strName)==0)
						{
							bExist = TRUE;
							break;
						}
					}
				}
				if (bExist)
					continue;
			}
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Close();
			}
			CRunoptCmdLineParser cmdLine(runreginfo.strPath);
			CString strAppName = cmdLine.GetCmd();
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Revert();
			}
			cinfo.nType		= KSRUN_TYPE_STARTUP;
			cinfo.strName	= runreginfo.strName;
			cinfo.strDisplay= runreginfo.strName;
			cinfo.bEnable	= bEnable;
			cinfo.strPath	= strAppName;
			cinfo.strParam	= runreginfo.strPath;
			cinfo.strExePath= strAppName;
			cinfo.nRegType	= runreginfo.nRegType;
			cinfo.bDisabledBy3xx = runreginfo.bDisabledBy3XX;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_STARTUP,runreginfo.strName,strAppName),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);
			PostGetItemDesc(cinfo);
			if (pCallback)
			{
				if (pCallback->FindRunItem(&cinfo) == TRUE)
				{
					nRet++;
					goto Exit0;
				}
			}
		}
	}

Exit0:
	return nRet;
}

int CRunManEngine::EnumDirRunner2(BOOL bEnable, CRunDirEnumer* pEnumer, IEnumRunCallBack* pCallback, CRunDirEnumer* pFilter)
{
	int nRet = 0;
	if (pEnumer->DoEnum())
	{
		for ( int i = 0; i < pEnumer->GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CRunDirEnumer::RUN_DIR_INFO&	rundirinfo = pEnumer->GetItem(i);
			
			if (pFilter != NULL)
			{
				BOOL	bExist = FALSE;
				for ( int k = 0; k< pFilter->GetCount(); k++)
				{
					CRunDirEnumer::RUN_DIR_INFO&	cmpInfo = pFilter->GetItem(k);
					if ( IsSameRunType(cmpInfo.nDirType, rundirinfo.nDirType) )
					{
						if (GetFileName(rundirinfo.strName).CompareNoCase(GetFileName(cmpInfo.strName))==0)
						{
							bExist = TRUE;
							break;
						}
					}
				}
				if (bExist)
					continue;
			}

			CString strParam;
			strParam.Format(_T("%s %s"),rundirinfo.strPath,rundirinfo.strParam);

			cinfo.nType			= KSRUN_TYPE_STARTUP;
			cinfo.strName		= GetFileName(rundirinfo.strName);
			cinfo.strPath		= rundirinfo.strName;	// 填写全路径
			cinfo.bEnable		= bEnable;
			cinfo.strExePath	= rundirinfo.strPath;	// LNK指向的EXE路径
			cinfo.strParam		= strParam;	
			cinfo.strDisplay	= cinfo.strName;
			cinfo.nRegType		= rundirinfo.nDirType;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_STARTUP,rundirinfo.strName,rundirinfo.strPath),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);
			PostGetItemDesc(cinfo);
			if (pCallback)
			{
				if (pCallback->FindRunItem(&cinfo) == TRUE)
				{
					nRet++;
					goto Exit0;
				}
			}
		}
	}
Exit0:
	return nRet;
}


int CRunManEngine::EnumServiceRunner2(CServiceEnumer* pEnumer, IEnumRunCallBack* pCallback)
{
	int nRet = 0;
	if (pEnumer->DoEnum())
	{
		for ( int i = 0; i < pEnumer->GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CServiceEnumer::SERVICE_CONFIG&	servInfo = pEnumer->GetItem(i);
			CString strCmdLine = servInfo.strServDLL.IsEmpty()?servInfo.lpqscfg->lpBinaryPathName:servInfo.strServDLL;
			
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Close();
			}
			
			CRunoptCmdLineParser cmdLine(strCmdLine);
			CString strAppPath = cmdLine.GetCmd();
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Revert();
			}
			cinfo.nType		= KSRUN_TYPE_SERVICE;
			cinfo.strName	= servInfo.strName;
			cinfo.bEnable	= IsTypeStartEnable(servInfo.lpqscfg->dwStartType);
			if (cinfo.bEnable == FALSE)
				continue;
			cinfo.strDesc	= servInfo.strDesc;
			cinfo.strParam	= strCmdLine;
			cinfo.strDisplay= servInfo.lpqscfg->lpDisplayName;
			cinfo.strExePath= strAppPath;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_SERVICE,servInfo.strName,cinfo.strExePath),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);

			PostGetItemDesc(cinfo);
			if (pCallback)
			{
				if (pCallback->FindRunItem(&cinfo) == TRUE)
				{
					nRet++;
					goto Exit0;
				}
			}
		}
	}
Exit0:
	return nRet;
}

int CRunManEngine::EnumServiceRunner(CServiceEnumer* pEnumer, IEnumRunCallBack* pCallback)
{
	int nServiceCount = 0;
	CServiceEnumer	servEnumer;
	if (servEnumer.DoEnum())
	{
		for ( int i = 0; i < servEnumer.GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CServiceEnumer::SERVICE_CONFIG&	servInfo = servEnumer[i];
			CString strCmdLine = servInfo.strServDLL.IsEmpty()?servInfo.lpqscfg->lpBinaryPathName:servInfo.strServDLL;
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Close();
			}
			CRunoptCmdLineParser cmdLine(strCmdLine);
			CString strAppPath = cmdLine.GetCmd();
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Revert();
			}
			cinfo.nType		= KSRUN_TYPE_SERVICE;
			cinfo.strName	= servInfo.strName;
			cinfo.bEnable	= IsTypeStartEnable(servInfo.lpqscfg->dwStartType);
			cinfo.strDesc	= servInfo.strDesc;
			cinfo.strParam	= strCmdLine;
			cinfo.strDisplay= servInfo.lpqscfg->lpDisplayName;
			cinfo.strExePath= strAppPath;
			cinfo.strPath	= strAppPath;
			if ((cinfo.bEnable == FALSE) && (cmdLine.IsPEFileExist() == FALSE))
				continue;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_SERVICE,servInfo.strName,cinfo.strExePath),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);

			PostGetItemDesc(cinfo);
			if (pCallback)
			{
				pCallback->FindRunItem(&cinfo);
				nServiceCount++;
			}
		}
	}
	return nServiceCount;
}

int CRunManEngine::EnumTaskRunner2(CTaskSchedularEnumer* pEnumer, IEnumRunCallBack* pCallback)
{	
	int nRet = 0;
	pEnumer->SetCopyFilter(this);
	if (pEnumer->DoEnum())
	{
		for ( int i = 0; i < pEnumer->GetCount(); i++)
		{
			CKsafeRunInfo	cinfo;
			CTaskSchedularEnumer::TASK_JOB_INFO&	jobInfo = pEnumer->GetItem(i);
			if (jobInfo.bEnable == FALSE)
				continue;
			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Close();
			}

			CRunoptCmdLineParser cmdLine(jobInfo.strExePath);
			CString strAppPath = cmdLine.GetCmd();

			if (m_wow64Switcher.IsWin64())
			{
				m_wow64Switcher.Revert();
			}


			cinfo.nType		= KSRUN_TYPE_TASK;
			cinfo.strName	= jobInfo.strJobPath;
			cinfo.strPath	= strAppPath;
			cinfo.strDisplay= jobInfo.strName;
			cinfo.strParam	= jobInfo.strParam;
			cinfo.bEnable	= jobInfo.bEnable;
			if (cinfo.bEnable == FALSE)
				continue;
			cinfo.strExePath= strAppPath;
			cinfo.nJobType	= jobInfo.nType;
			cinfo.strDesc	= jobInfo.strDesc;

			KSRunInfo*	pKSInfo = NULL;
			if (m_pLibLoader->FindDatInfo(CFindData(KSRUN_TYPE_TASK,jobInfo.strJobPath,strAppPath),&pKSInfo))
				ConvertLib(cinfo,*pKSInfo);

			PostGetItemDesc(cinfo);
			if (pCallback)
			{
				if (pCallback->FindRunItem(&cinfo) == TRUE)
				{
					nRet++;
					goto Exit0;
				}
			}
		}
	}
Exit0:
	return nRet;
}

BOOL CRunManEngine::ControlSysConfig(DWORD dwFlags, CKSafeSysoptRegInfo* pSysCfgInfo)
{
	if (pSysCfgInfo == NULL)
	{
		return FALSE;
	}
	BOOL bRet = FALSE;
	
	for (int n = 0;n < m_pLibLoader2->GetOptInfoCount();n++)
	{
		CRestoreRunner* pLogRunner = new CRestoreRunner;
		CKSafeSysoptRegInfo& syscfgInfo = m_pLibLoader2->GetOptInfoByIndex(n);
		if (syscfgInfo.m_nID == pSysCfgInfo->m_nID)
		{
			HKEY hKeyRoot = syscfgInfo.m_hKeyRoot;
			CString strRegSubKey = syscfgInfo.m_strSubKey;
			CString strRegValueName = syscfgInfo.m_strValueName;
			DWORD dwRegValueType = syscfgInfo.m_nRegValueType;
			CString strDefaultValue = syscfgInfo.m_strDefalutValue;
			CString strOptimizeValue = syscfgInfo.m_strOptimizedValue;
			CString strInitValue = syscfgInfo.m_strInitValue;

			bRet = CSysConfigOper().OptimizeSysConfig(
				dwFlags, 
				hKeyRoot, 
				strRegSubKey, 
				strRegValueName, 
				strDefaultValue, 
				strOptimizeValue, 
				strInitValue,
				dwRegValueType
				);
			if (bRet)
			{
				pLogRunner->OperateSysCfg(
					pSysCfgInfo->m_nID, 
					KSRUN_TYPE_SYSCFG, 
					dwFlags, 
					strRegValueName, 
					strRegSubKey, 
					pSysCfgInfo->m_strInitValue
					);
			}
		}
		delete pLogRunner;
	}
	return bRet;
}

VOID CRunManEngine::EnumSysConfigInfo(DWORD dwFlags, IEnumRunCallBackNull2* pCallback)
{
	if (m_pLibLoader2 == NULL)
	{
		m_pLibLoader2 = new CLibLoader2;
		m_pLibLoader2->LoadLib(NULL);
		m_pLibLoader2->ConvertLibToOptData();
	}
	int nFindCount = 0;
	for (int nCount = 0; nCount < m_pLibLoader2->GetOptInfoCount(); nCount++)
	{
		CKSafeSysoptRegInfo& sysoptInfo = m_pLibLoader2->GetOptInfoByIndex(nCount) ;
		//对于nt6下独有的优化项，在非nt6的系统就不扫描
		if (!IsVistaLater() && sysoptInfo.m_bIsNt6OnlyHave == 1)
		{
			continue;
		}
		//对于nt5下独有的优化项，在nt6的系统就不扫描
		if (IsVistaLater() && sysoptInfo.m_bIsNt6OnlyHave == 2)
		{
			continue;
		}

		HKEY hKeyRoot = sysoptInfo.m_hKeyRoot;
		CString strRegSubKey = sysoptInfo.m_strSubKey;
		CString strRegValueName = sysoptInfo.m_strValueName;
		DWORD dwRegValueType = sysoptInfo.m_nRegValueType;
		DWORD dwType;
		CString strReadValue;
		DWORD dwSize;
		LONG lRet;
		if (dwRegValueType == REG_SZ)
		{
			TCHAR szValue[MAX_PATH] = {0};
			lRet = SHGetValue(hKeyRoot, strRegSubKey.GetString(), strRegValueName.GetString(), &dwType, (LPVOID)szValue, &dwSize);
			if (lRet == ERROR_SUCCESS)
			{
				strReadValue = szValue;
				if (sysoptInfo.IsCanOptimize(strReadValue, sysoptInfo.m_strOptimizedValue))
				{
					goto _FindItem;
				}
				else
				{
					continue;			
				}
			}
			else if (lRet == ERROR_FILE_NOT_FOUND)
			{
				strReadValue = KSAFE_REGSYSOPT_KEY_NULL;
				goto _FindItem;
			}
			else
			{
				continue;
			}
		}
		else if (dwRegValueType == REG_DWORD)
		{
			DWORD dwReadValue = 0;
			lRet = SHGetValue(hKeyRoot, strRegSubKey.GetString(), strRegValueName.GetString(), &dwType, (LPBYTE)(&dwReadValue), &dwSize);
			if (lRet == ERROR_SUCCESS)
			{
				_itow_s(dwReadValue, strReadValue.GetBuffer(20), 20, 10);
				if (sysoptInfo.IsCanOptimize(strReadValue, sysoptInfo.m_strOptimizedValue))
				{
					goto _FindItem;
				}
				else
				{
					continue;			
				}
			}
			else if (lRet == ERROR_FILE_NOT_FOUND)
			{
				strReadValue = KSAFE_REGSYSOPT_KEY_NULL;
				goto _FindItem;
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}

_FindItem:
		sysoptInfo.m_strInitValue = strReadValue;
		if (pCallback)
			pCallback->FindRunItem(&sysoptInfo);
		nFindCount++;
		continue;
	}
}

VOID CRunManEngine::RestoreSysCfg(DWORD dwFlags, IEnumRunCallBack* pCallback)
{
	if (m_pLibLoader2 == NULL)
	{
		m_pLibLoader2 = new CLibLoader2;
		m_pLibLoader2->LoadLib(NULL);
		m_pLibLoader2->ConvertLibToOptData();
	}
	int nCount = 0;
	if (dwFlags == KSAFE_SYSCONFIG_RESTORE_SYSDEF)
	{
		CRestoreRunner *pLogRunner = new CRestoreRunner;
		for (int n = 0;n < m_pLibLoader2->GetOptInfoCount();n++)
		{
			CKSafeSysoptRegInfo& syscfgInfo = m_pLibLoader2->GetOptInfoByIndex(n);	
			HKEY hKeyRoot = syscfgInfo.m_hKeyRoot;
			CString strRegSubKey = syscfgInfo.m_strSubKey;
			CString strRegValueName = syscfgInfo.m_strValueName;
			DWORD dwRegValueType = syscfgInfo.m_nRegValueType;
			CString strDefaultValue = syscfgInfo.m_strDefalutValue;
			CString strOptimizeValue = syscfgInfo.m_strOptimizedValue;
			CString strInitValue = syscfgInfo.m_strInitValue;
			CRestoreRunner* pLogRunner2 = new CRestoreRunner;
			BOOL bRet = FALSE;
			bRet = CSysConfigOper().OptimizeSysConfig(
				dwFlags, 
				hKeyRoot, 
				strRegSubKey, 
				strRegValueName, 
				strDefaultValue, 
				strOptimizeValue, 
				strInitValue,
				dwRegValueType
				);
			if (bRet)
			{
				nCount++;
				pLogRunner2->OperateSysCfg(
					syscfgInfo.m_nID, 
					KSRUN_TYPE_SYSCFG, 
					dwFlags, 
					strRegValueName, 
					strRegSubKey,
					strInitValue
					);
			}
			delete pLogRunner2;
		}
		delete pLogRunner;
	}
	if (dwFlags == KSAFE_SYSCONFIG_RESTORE_INIT)
	{
		CRestoreRunner *pLogRunner = new CRestoreRunner;
		for(int nLoop = 0; nLoop < pLogRunner->GetCount();nLoop++)
		{
			RunLogItem* pLog = new RunLogItem;
			pLog = pLogRunner->GetItem(nLoop);
			for (int nLoop2 = 0;nLoop2 < m_pLibLoader2->GetOptInfoCount();nLoop2++)
			{
				CKSafeSysoptRegInfo& syscfgInfo = m_pLibLoader2->GetOptInfoByIndex(nLoop2);
				
				if (pLog->nType == KSRUN_TYPE_SYSCFG
					&& syscfgInfo.m_nID == pLog->nID 
					&& syscfgInfo.m_strValueName.CompareNoCase(pLog->strName) == 0
					&& syscfgInfo.m_strSubKey.CompareNoCase(pLog->strSubKey) == 0)
				{
					CRestoreRunner* pLogRunner2 = new CRestoreRunner;
					HKEY hKeyRoot = syscfgInfo.m_hKeyRoot;
					CString strRegSubKey = syscfgInfo.m_strSubKey;
					CString strRegValueName = syscfgInfo.m_strValueName;
					DWORD dwRegValueType = syscfgInfo.m_nRegValueType;
					CString strDefaultValue = syscfgInfo.m_strDefalutValue;
					CString strOptimizeValue = syscfgInfo.m_strOptimizedValue;
					CString strInitValue = pLog->strName2;

					BOOL bRet = FALSE;
					bRet = CSysConfigOper().OptimizeSysConfig(
						dwFlags, 
						hKeyRoot, 
						strRegSubKey, 
						strRegValueName, 
						strDefaultValue, 
						strOptimizeValue, 
						strInitValue,
						dwRegValueType
						);
					if (bRet)
					{
						pLogRunner2->OperateSysCfg(
							syscfgInfo.m_nID, 
							KSRUN_TYPE_SYSCFG, 
							dwFlags, 
							strRegValueName, 
							strRegSubKey,
							strInitValue);
						nCount++;
					}
					delete pLogRunner2;
				}
			}
		}
	}
}
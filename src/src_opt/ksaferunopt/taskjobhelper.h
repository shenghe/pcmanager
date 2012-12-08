
#pragma once


#define _WIN32_DCOM

#include <initguid.h>
#include <ole2.h>
#include <MSTask.h>

#include <atlpath.h>
#include <plugin/enumfile.h>
#include "globalstate/GetGlobalState.h"
#include <common/autoinitcom.h>
#include "runhelper.h"
#include <comdef.h>
#include <taskschd.h>
# pragma comment(lib, "taskschd.lib")
# pragma comment(lib, "comsupp.lib")


#define JOB_TYPE_NT5		1
#define JOB_TYPE_NT6		2	

#define COMBINE_PATH_PARAM(s,e,p) {s=e;s.Append(_T(" "));s.Append(p);}

class CTaskSchedularOper
{
public:
	CTaskSchedularOper()
	{

	}
	virtual ~CTaskSchedularOper()
	{

	}
public:
	BOOL ControlRun(BOOL bEnable, LPCTSTR lpstrJobName, DWORD nJobType)
	{
		return _ControlRun(lpstrJobName,bEnable,nJobType);
	}

	BOOL DeleteTask(LPCTSTR lpstrJobName, DWORD nJobType)
	{
		return _ControlDelete(lpstrJobName,nJobType);
	}

protected:

	struct _JOB_THREAD_RUN
	{
		LPCTSTR				lpstrName;
		BOOL				bEnable;
		DWORD				nType;
		CTaskSchedularOper* pThis;	
		BOOL				bRet;
	};

	BOOL _ControlDelete(LPCTSTR lpstrJobName,DWORD nJobType)
	{
		_JOB_THREAD_RUN	xPa;

		xPa.bRet	= FALSE;
		xPa.lpstrName = lpstrJobName;
		xPa.pThis	= this;
		xPa.nType	= nJobType;

		HANDLE	hT = (HANDLE)_beginthread(_ControlDeletenStub,0,(PVOID)&xPa);
		::WaitForSingleObject(hT,INFINITE);

		return xPa.bRet;
	}

	static void _ControlDeletenStub(PVOID lParam)
	{
		_JOB_THREAD_RUN*	tP = (_JOB_THREAD_RUN*)lParam;
		tP->bRet = tP->pThis->_DeleteTask(tP->lpstrName,tP->nType);
		return;
	}

	BOOL _DeleteTask(LPCTSTR lpstrJobName, DWORD nJobType)
	{
		if (nJobType==JOB_TYPE_NT5)
			return _NT5Delete(lpstrJobName);
		else if (nJobType==JOB_TYPE_NT6)
			return _NT6Delete(lpstrJobName);
		return FALSE;
	}

	BOOL _NT6Delete(LPCTSTR lpstrJobName)
	{
		CPath		cPath(lpstrJobName);
		int			iIndex = cPath.FindFileName();

		if (iIndex==-1)
			return FALSE;

		CString		strName = cPath.m_strPath.Mid(iIndex);
		if (!cPath.RemoveFileSpec())
			return FALSE;

		CString		strFolder = cPath.m_strPath;
		if (strFolder.IsEmpty() || strName.IsEmpty() )
			return FALSE;

		CAutoCoInit	autoCO(NULL,COINIT_MULTITHREADED);
		if( !autoCO.IsSucceed() )
			return FALSE;

		//  Set general COM security levels.
		HRESULT hr = CoInitializeSecurity(
			NULL,
			-1,
			NULL,
			NULL,
			RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			0,
			NULL);

		CComPtr<ITaskService> pService = NULL;
		hr = CoCreateInstance( CLSID_TaskScheduler,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITaskService,
			(void**)&pService );  
		if (FAILED(hr))
			return FALSE;

		hr = pService->Connect(_variant_t(), _variant_t(),
			_variant_t(), _variant_t());
		if( FAILED(hr) )
			return FALSE;

		CComPtr<ITaskFolder> pSubFolder = NULL;
		hr = pService->GetFolder( CComBSTR(strFolder) , &pSubFolder );
		if( FAILED(hr) )
			return FALSE;

		hr = pSubFolder->DeleteTask(CComBSTR(strName),0);
		if( FAILED(hr) )
			return FALSE;

		return TRUE;
	}

	BOOL _NT5Delete(LPCTSTR lpName)
	{
		CString	strPath = GetGlobalState()->GetWindowsDir();

		strPath.Append(_T("\\Tasks\\"));
		strPath.Append(lpName);

		if (::PathFileExists(strPath))
		{
			return ::DeleteFile(strPath);
		}
		return FALSE;
	}

	BOOL _ControlRun(LPCTSTR lpstrJobName,BOOL bEnable, DWORD nJobType)
	{
		_JOB_THREAD_RUN	xPa;

		xPa.bEnable = bEnable;
		xPa.bRet	= FALSE;
		xPa.lpstrName = lpstrJobName;
		xPa.pThis = this;
		xPa.nType	= nJobType;

		HANDLE	hT = (HANDLE)_beginthread(_ControlRunStub,0,(PVOID)&xPa);
		::WaitForSingleObject(hT,INFINITE);

		return xPa.bRet;
	}

	static void _ControlRunStub(PVOID lParam)
	{
		_JOB_THREAD_RUN*	tP = (_JOB_THREAD_RUN*)lParam;
		tP->bRet = tP->pThis->_ThreadControlRun(tP->lpstrName,tP->bEnable,tP->nType);
		return;
	}

	BOOL _ThreadControlRun(LPCTSTR lpstrJobName,BOOL bEnable,DWORD nJobType)
	{
		OSVERSIONINFO	osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		if (osvi.dwMajorVersion==5)
			return _NT5Control(lpstrJobName,bEnable);
		else if (osvi.dwMajorVersion==6)
		{
			if (nJobType==JOB_TYPE_NT6)
				return _NT6Control(lpstrJobName,bEnable);
			else
				return _NT5Control(lpstrJobName,bEnable);
		}
		else
			return FALSE;
	}

	BOOL _NT5Control(LPCTSTR lpstrJobName,BOOL bEnable)
	{
		HRESULT hr = S_OK;
		CComPtr<ITaskScheduler> pITS;
		CAutoCoInit	autoInit;

		if (!autoInit.IsSucceed())
			return FALSE;

		hr = CoCreateInstance(CLSID_CTaskScheduler,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITaskScheduler,
			(void **) &pITS);
		if (FAILED(hr))
			return FALSE;

		CComPtr<ITask> pITask;
		hr = pITS->Activate(lpstrJobName,
			IID_ITask,
			(IUnknown**) &pITask);
		if (FAILED(hr))
			return FALSE;

		DWORD	nFlags;
		hr = pITask->GetFlags(&nFlags);
		if (FAILED(hr))
			return FALSE;

		if (bEnable)
			nFlags &= ~TASK_FLAG_DISABLED;
		else
			nFlags |= TASK_FLAG_DISABLED;

		hr = pITask->SetFlags(nFlags);
		if (FAILED(hr))
			return FALSE;

		CComPtr<IPersistFile> pIPersistFile;
		hr = pITask->QueryInterface(IID_IPersistFile,   (void**)&pIPersistFile);   
		if (FAILED(hr))   
			return FALSE;

		hr = pIPersistFile->Save(NULL,TRUE); 
		if (FAILED(hr))   
			return FALSE;

		return TRUE;
	}

	BOOL _NT6Control(LPCTSTR lpstrJobName,BOOL bEnable)
	{
		CPath		cPath(lpstrJobName);
		int			iIndex = cPath.FindFileName();

		if (iIndex==-1)
			return FALSE;

		CString		strName = cPath.m_strPath.Mid(iIndex);
		if (!cPath.RemoveFileSpec())
			return FALSE;

		CString		strFolder = cPath.m_strPath;
		if (strFolder.IsEmpty() || strName.IsEmpty() )
			return FALSE;

		CAutoCoInit	autoCO(NULL,COINIT_MULTITHREADED);
		if( !autoCO.IsSucceed() )
			return FALSE;

		//  Set general COM security levels.
		HRESULT hr = CoInitializeSecurity(
			NULL,
			-1,
			NULL,
			NULL,
			RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			0,
			NULL);

		CComPtr<ITaskService> pService = NULL;
		hr = CoCreateInstance( CLSID_TaskScheduler,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITaskService,
			(void**)&pService );  
		if (FAILED(hr))
			return FALSE;

		hr = pService->Connect(_variant_t(), _variant_t(),
			_variant_t(), _variant_t());
		if( FAILED(hr) )
			return FALSE;

		CComPtr<ITaskFolder> pSubFolder = NULL;
		hr = pService->GetFolder( CComBSTR(strFolder) , &pSubFolder );
		if( FAILED(hr) )
			return FALSE;

		CComPtr<IRegisteredTask>	pTask;
		hr = pSubFolder->GetTask(CComBSTR(strName),&pTask);
		if( FAILED(hr) )
			return FALSE;

		hr = pTask->put_Enabled(CComVariant(bEnable).boolVal);
		if( FAILED(hr) )
		{
			CDisableList cDisList;

			cDisList.OperateTaskJob(lpstrJobName, FALSE, FALSE, FALSE, FALSE, FALSE);
		}

		return TRUE;
	}
};

class IEnumTaskFilter
{
public:
	virtual BOOL PreCopyTaskFile(DWORD nType, LPCTSTR lpFileName) = 0;
};

class CTaskSchedularEnumer : public CCheck3XXInst
{
public:
	CTaskSchedularEnumer(){
		m_pFilter = NULL;
	}
	virtual ~CTaskSchedularEnumer(){}

	void SetCopyFilter(IEnumTaskFilter* p)
	{
		m_pFilter = p;
	}

	struct TASK_JOB_INFO
	{
		BOOL	bEnable;
		CString	strName;
		CString	strJobPath;
		CString	strExePath;
		CString	strParam;
		CString	strDesc;
		DWORD	nType;
	};

public:
	BOOL DoEnum()
	{
		HANDLE hT = (HANDLE)_beginthread(_threadStub,0,this);
		::WaitForSingleObject(hT,INFINITE);
		return TRUE;
	}

	int GetCount()
	{
		return m_jobArray.GetSize();
	}

	TASK_JOB_INFO& GetItem(int iIndex)
	{
		return m_jobArray[iIndex];
	}

protected:
	static void _threadStub(PVOID lpParam)
	{
		CTaskSchedularEnumer* p = (CTaskSchedularEnumer*)lpParam;

		p->_DoEnum();
	}

	BOOL _DoEnum()
	{
		OSVERSIONINFO	osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		if (osvi.dwMajorVersion==5)
			return _NT5Enum();
		else if (osvi.dwMajorVersion==6)
		{
			BOOL bRes = _NT6Enum();
			return _NT5Enum()||bRes;
		}
		else
			return FALSE;
	}

protected:

	void _PreEnumNt5()
	{
		TCHAR	szWin[MAX_PATH] = {0};
		GetWindowsDirectory(szWin,MAX_PATH);
		CString	strBuffer = szWin;
		if (strBuffer.IsEmpty())
			return;

		CString	strWinTask	= szWin;
		strWinTask+=_T("\\Tasks\\");

		strBuffer += _T("\\Tasks\\360Disabled");

		if (TRUE)
		{
			CEnumFile	enumer(strBuffer, _T("*.job"));
			for ( int i=0; i<enumer.GetFileCount(); i++)
			{
				CString	strFileName = enumer.GetFileName(i);
				CString	strFullPath = enumer.GetFileFullPath(i);

				CString	strMovejob = strWinTask;
				strMovejob+=strFileName;

				if (::PathFileExists(strMovejob))
					continue;

				if (!m_pFilter || m_pFilter->PreCopyTaskFile(JOB_TYPE_NT5,strFileName) )
				{
					if (CopyFile(strFullPath,strMovejob,TRUE))
					{
						CTaskSchedularOper().ControlRun(FALSE,strFileName,JOB_TYPE_NT5);
					}
				}
			}
		}
		return;
	}

	BOOL _NT5Enum()
	{
		if (Is3XXInstalled())
			_PreEnumNt5();

		HRESULT hr = S_OK;
		CComPtr<ITaskScheduler> pITS;
		CAutoCoInit	autoInit;

		if (!autoInit.IsSucceed())
			return FALSE;

		hr = CoCreateInstance(CLSID_CTaskScheduler,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITaskScheduler,
			(void **) &pITS);
		if (FAILED(hr))
			return FALSE;

		CComPtr<IEnumWorkItems> pIEnum;
		hr = pITS->Enum(&pIEnum);
		if (FAILED(hr))
			return FALSE;

		LPWSTR *lpwszNames;
		DWORD dwFetchedTasks = 0;
		while (SUCCEEDED(pIEnum->Next(5,
							&lpwszNames,
							&dwFetchedTasks))
							&& (dwFetchedTasks != 0)
				)
		{
			while (dwFetchedTasks)
			{
				CComPtr<ITask> pITask;
				LPCWSTR lpcwszTaskName;
				CString	strTaskName;
				lpcwszTaskName = lpwszNames[--dwFetchedTasks];
				hr = pITS->Activate(lpcwszTaskName,
					IID_ITask,
					(IUnknown**) &pITask);

				strTaskName = lpcwszTaskName;
				CoTaskMemFree(lpwszNames[dwFetchedTasks]);

				if (FAILED(hr))
				{
					continue;
				}

				LPWSTR	lpAppName = NULL;
				LPWSTR	lpParam	  = NULL;
				HRESULT	hRes	  = S_OK;
				DWORD	nFlags	  = 0;
				
				if (SUCCEEDED(pITask->GetApplicationName(&lpAppName)) &&
					SUCCEEDED(pITask->GetParameters(&lpParam)) &&
					SUCCEEDED(pITask->GetFlags(&nFlags) )
					)
				{
					TASK_JOB_INFO	jobinfo;
					jobinfo.strExePath	= lpAppName;
					jobinfo.strJobPath	= strTaskName;
					jobinfo.strName		= strTaskName;
					jobinfo.bEnable		= !(nFlags&TASK_FLAG_DISABLED);
					jobinfo.nType		= JOB_TYPE_NT5;

					LPWSTR	lpDesc = NULL;
					if ( SUCCEEDED(pITask->GetComment(&lpDesc)) && lpDesc != NULL)
					{
						jobinfo.strDesc	= lpDesc;
						CoTaskMemFree(lpDesc);
						lpDesc = NULL;
					}

					COMBINE_PATH_PARAM(jobinfo.strParam,lpAppName,lpParam);

					m_jobArray.Add(jobinfo);
				}

				if (lpAppName!=NULL)
				{
					CoTaskMemFree(lpAppName);
					lpAppName = NULL;
				}
				if (lpParam!=NULL)
				{
					CoTaskMemFree(lpParam);
					lpParam = NULL;
				}
			}
			CoTaskMemFree(lpwszNames);
		}

		return TRUE;
	}

	DWORD _RecurGetJobs(ITaskFolder* pRootFolder, CSimpleArray<TASK_JOB_INFO>& taskjobarray)
	{
		if (pRootFolder==NULL)
			return 0;

// 		CComBSTR	bstrFolderpath;
// 		if (SUCCEEDED(pRootFolder->get_Path(&bstrFolderpath)) && 
// 			bstrFolderpath==CComBSTR(L"\\Microsoft\\Windows") )
// 		{
// 			return 0;
// 		}
		
		//  -------------------------------------------------------
		//  Get the registered tasks in the folder.
		DWORD	iTaskCount = 0;

		CComPtr<ITaskFolderCollection>	pSubFolders;
		if (SUCCEEDED(pRootFolder->GetFolders(0,&pSubFolders)))
		{
			long iCount = 0;
			if (SUCCEEDED(pSubFolders->get_Count(&iCount)))
			{
				for ( int i=0; i<iCount;i++)
				{
					CComPtr<ITaskFolder>	pSubFolder;
					if ( SUCCEEDED(pSubFolders->get_Item(CComVariant(i+1),&pSubFolder)) )
					{
						iTaskCount += _RecurGetJobs(pSubFolder,taskjobarray);
					}
				}
			}
		}

		CComPtr<IRegisteredTaskCollection> pTaskCollection = NULL;
		HRESULT hr = pRootFolder->GetTasks( TASK_ENUM_HIDDEN, &pTaskCollection );

		if( FAILED(hr) )
			return iTaskCount;

		LONG numTasks = 0;
		hr = pTaskCollection->get_Count(&numTasks);

		if( numTasks == 0 )
			return iTaskCount;

		TASK_STATE taskState;
		for(LONG i=0; i < numTasks; i++)
		{
			CComPtr<IRegisteredTask> pRegisteredTask = NULL;
			hr = pTaskCollection->get_Item( CComVariant(i+1), &pRegisteredTask );

			if( SUCCEEDED(hr) )
			{
				CComBSTR taskName(NULL);
				hr = pRegisteredTask->get_Name(&taskName);
				if( SUCCEEDED(hr) )
				{
					CComBSTR		taskPath(NULL);
					CComVariant		stateEnable;
					TASK_JOB_INFO	jobinfo;
					CComPtr<ITaskDefinition>	pTaskDef;
					if (SUCCEEDED(pRegisteredTask->get_Path(&taskPath)) && 
						SUCCEEDED(pRegisteredTask->get_Enabled(&stateEnable.boolVal))&&
						SUCCEEDED(pRegisteredTask->get_State(&taskState)) &&
						SUCCEEDED(pRegisteredTask->get_Definition(&pTaskDef))
						)
					{
						//检查该计划任务是否能被修改，某些情况下，由于计划任务损坏，导致无法禁用，本逻辑将不显示这些计划任务
						//引起异常原因太多，计划任务的修复将在“系统修复”功能中实现。开机优化仅处理正常计划任务。

						CDisableList CDisList;

						if (CDisList.IsIgnore(taskPath))
							continue;
		
					
						//----------------------------------------------------------------------------------

						BOOL bFindExe = FALSE;
						jobinfo.bEnable		= ((VARIANT_TRUE == stateEnable.boolVal) ? TRUE : FALSE);
						jobinfo.strJobPath	= taskPath;
						jobinfo.strName		= taskName;
						jobinfo.nType		= JOB_TYPE_NT6;

						CComPtr<IRegistrationInfo>	pTaskInfo;
						if ( SUCCEEDED(pTaskDef->get_RegistrationInfo(&pTaskInfo)) )
						{
							CComBSTR	bstrDesc;
							if ( SUCCEEDED(pTaskInfo->get_Description(&bstrDesc)) )
							{
								jobinfo.strDesc	= bstrDesc;
							}
						}

						CComPtr<IActionCollection>	pActionColl;
						if (SUCCEEDED(pTaskDef->get_Actions(&pActionColl)))
						{
							long	nCount = 0;
							pActionColl->get_Count(&nCount);

							for (int i=0;i<nCount;i++)
							{
								CComPtr<IAction>	pAction;
								if (SUCCEEDED(pActionColl->get_Item(i+1,&pAction)))
								{
									CComPtr<IExecAction>	pExeAction;
									TASK_ACTION_TYPE		actType;
									if (SUCCEEDED(pAction->get_Type(&actType))&& actType==TASK_ACTION_EXEC )
									{
										if (SUCCEEDED(pAction->QueryInterface(IID_IExecAction,
											(void**)&pExeAction)))
										{
											CComBSTR	bstrPath;
											CComBSTR	bstrParam;
											pExeAction->get_Path(&bstrPath);
											pExeAction->get_Arguments(&bstrParam);

											if (bstrPath.Length()>0)
											{
												jobinfo.strExePath	= bstrPath;
												COMBINE_PATH_PARAM(jobinfo.strParam,bstrPath,bstrParam);

												bFindExe = TRUE;
												break;
											}
										}
									}
								}
							}
						}
						if (bFindExe)
						{
							taskjobarray.Add(jobinfo);
							iTaskCount++;
						}
					}
				}
			}
		}
		
		return iTaskCount;
	}

	BOOL _NT6Enum()
	{
		CAutoCoInit	autoCO(NULL,COINIT_MULTITHREADED);

		if( !autoCO.IsSucceed() )
			return FALSE;

		//  Set general COM security levels.
		HRESULT hr = CoInitializeSecurity(
					NULL,
					-1,
					NULL,
					NULL,
					RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
					RPC_C_IMP_LEVEL_IMPERSONATE,
					NULL,
					0,
					NULL);

// 		if( FAILED(hr) )
// 			return FALSE;

		//  ------------------------------------------------------
		//  Create an instance of the Task Service. 
		CComPtr<ITaskService> pService = NULL;
		hr = CoCreateInstance( CLSID_TaskScheduler,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITaskService,
			(void**)&pService );  
		if (FAILED(hr))
			return FALSE;

		//  Connect to the task service.
		hr = pService->Connect(_variant_t(), _variant_t(),
			_variant_t(), _variant_t());
		if( FAILED(hr) )
			return FALSE;

		CComPtr<ITaskFolder> pRootFolder = NULL;
		hr = pService->GetFolder( CComBSTR( L"\\") , &pRootFolder );

		if( FAILED(hr) )
			return FALSE;

		_RecurGetJobs(pRootFolder,m_jobArray);

		return TRUE;
	}

protected:
	CSimpleArray<TASK_JOB_INFO>	m_jobArray;
	IEnumTaskFilter*			m_pFilter;
};
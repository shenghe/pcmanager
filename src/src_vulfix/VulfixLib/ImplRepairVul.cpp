#include "StdAfx.h"
#include "ImplRepairVul.h"
#include "ImplVulIgnore.h"
#include "Utils.h"
#include "HotfixUtils.h"
#include "WUAUpdateInfo.h"
#include "EnvUtils.h"
#include "UserPatcher.h"
#include "LeakDB.h"
#include "../../src_commonlib/libDownload2/Utilfunction.h"
#include "winmod/winfilefind.h"

BOOL IsValidPatchFile(LPCTSTR szFilename)
{
	// MSCF
	const CHAR szMagicMsu[] = {'M', 'S', 'C', 'F'};
	const CHAR szMagicExe[] = {'M', 'Z'};
	
	const int TEST_BUFFER_SIZE = 4;
	CHAR buffer[TEST_BUFFER_SIZE] = {0};

	BOOL bMatched = FALSE;
	CAtlFile file;
	if(S_OK == file.Create(szFilename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING) )
	{		
		if(SUCCEEDED(file.Read(buffer, TEST_BUFFER_SIZE)))
		{
			LPCTSTR szDot = _tcsrchr(szFilename, _T('.'));
			if(szDot)
			{
				++szDot;
				if(_tcsicmp(szDot, _T("msu"))==0)
				{
					bMatched = memcmp(buffer, szMagicMsu, 4)==0;
				}
				else if(_tcsicmp(szDot, _T("exe"))==0)
				{
					bMatched = memcmp(buffer, szMagicExe, 2)==0;
				}
				else
					bMatched = TRUE;
			}
		}
		file.Close();
	}
	return bMatched;
}

BOOL CheckIfFirstRun(LPCTSTR szGlobalName)
{
	ATOM at = GlobalFindAtom(szGlobalName);
	if(at!=0)
	{
		return FALSE;
	}
	GlobalAddAtom(szGlobalName);
	return TRUE;
}

BOOL CheckIfFirstRunLeakRepair()
{
	return CheckIfFirstRun(_T("{03D3F7C4-3E8E-4fb9-8985-087F09A59866}"));
}

BOOL _FixUpdateGarbage(HKEY hKey, LPCTSTR lpSubKey, DWORD dwIndex, LPCTSTR lpszKey, LPVOID lpData)
{
	ATLASSERT(lpData);
	LPCTSTR szVolatile = _T("UpdateExeVolatile");
	if(_tcsnicmp(szVolatile, lpszKey, _tcslen(szVolatile))==0)
	{
		CSimpleArray<CString>& arr = *(CSimpleArray<CString>*)lpData;
		arr.Add(lpszKey);
	}
	return TRUE;
}

INT FixUpdateGarbage()
{
	CSimpleArray<CString> arrSubKeys;
	LPCTSTR szPathUpdates = _T("SOFTWARE\\Microsoft\\Updates");
	EnumRegKey(HKEY_LOCAL_MACHINE, szPathUpdates, _FixUpdateGarbage, (LPVOID)&arrSubKeys);	
	for(int i=0; i<arrSubKeys.GetSize(); ++i)
	{
		CString strSubKey;
		strSubKey.Format(_T("%s\\%s"), szPathUpdates, arrSubKeys[i]);
		RegDeleteKey(HKEY_LOCAL_MACHINE, strSubKey);
	}
	return arrSubKeys.GetSize();
}

CImplRepairVul::CImplRepairVul(void) : m_Observer(NULL), m_objIgnore( singleton<CImplVulIgnore>::Instance() )
{
	m_hHandleDownload = NULL;
	m_hHandleInstall = NULL;
	m_pUserPatcher = NULL;
	m_pCurrentVulfix = NULL;
    m_bDownloadHasReceivedData = FALSE;
}

CImplRepairVul::~CImplRepairVul(void)
{
	SAFE_DELETE(m_pUserPatcher);
}

HRESULT CImplRepairVul::Reset()
{
	m_pCurrentVulfix = NULL;
	m_arrDownloadItem.RemoveAll();
	m_thinfo.Reset();
	return S_OK;
}

HRESULT CImplRepairVul::SetObserver( IVulfixObserver *pObserver )
{
	m_Observer = pObserver;
	return S_OK;
}

template <typename T>
int FillDownloadsItem(const CSimpleArray<T> &arr, const IntArray&arrayId, CSimpleArray<T_RepairItem>&arrDownloadItem)
{
	int count = 0;
	for(int i=0; i<arrayId.GetSize(); ++i)
	{
		int nID = arrayId[i];
		T pItem = FindArrayItem(arr, nID);
		if(pItem)
		{
			arrDownloadItem.Add( T_RepairItem(pItem) );
			++ count;
		}
	}
	return count;
}

HRESULT CImplRepairVul::AddVul( IVulfix *pVulfix, const IntArray&arrayId )
{
	const CSimpleArray<LPTUpdateItem>& arr = pVulfix->GetResults();
	FillDownloadsItem(arr, arrayId, m_arrDownloadItem);
	return S_OK;
}

HRESULT CImplRepairVul::AddSoftVul( IVulfix *pVulfix, const IntArray&arrayId )
{
	const CSimpleArray<LPTVulSoft>& arr = pVulfix->GetSoftVuls();
	FillDownloadsItem(arr, arrayId, m_arrDownloadItem);
	m_pCurrentVulfix = pVulfix;
	return S_OK;
}

HRESULT CImplRepairVul::Repair(LPCTSTR sDownloadPath, LPCTSTR szImportPath, DWORD dwFlags)
{
	if(m_arrDownloadItem.GetSize()==0)
		return KERR_NONE;
	if( !sDownloadPath  || !CreateDirectoryNested(sDownloadPath) )
		return KERR_WRITE_FILE;
	
	HRESULT hr = S_OK;
	CSysEnv& sysEnv = singleton<CSysEnv>::Instance();
	sysEnv.Init();
	if( FAILED( hr=sysEnv.IsSupported(TRUE) ) )
		return hr;
	
	//if(CheckIfFirstRunLeakRepair())
	//	FixUpdateGarbage();

	m_strDownloadPath = sDownloadPath;
	m_strImportPath = szImportPath ? szImportPath : _T("");
	m_dwRepairFlags = dwFlags;
	m_bCanceled = FALSE;
	if(m_thinfo.m_hInstallEvent)
	{
		CloseHandle( m_thinfo.m_hInstallEvent );
		m_thinfo.m_hInstallEvent = NULL;
	}
	m_thinfo.m_hInstallEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	
	m_hHandleDownload = CreateThread(NULL, 0, _ThreadFuncDownload, (PVOID)this, 0, NULL);
	m_hHandleInstall = CreateThread(NULL, 0, _ThreadFuncInstall, (PVOID)this, 0, NULL);
	return S_OK;
}

HRESULT CImplRepairVul::StopRepair()
{
	m_bCanceled = TRUE;
	{
		CObjGuard __guard__(m_thinfo.m_objLockFileDownloader, TRUE);
        if(m_thinfo.m_pFileDownloaderSingle)
			m_thinfo.m_pFileDownloaderSingle->Stop();
        if(m_thinfo.m_pFileDownloader)
            m_thinfo.m_pFileDownloader->Stop();
	}
	m_thinfo.m_fileDownloadLocal.Stop();

	/*
	if(m_hHandleDownload)
	{
		SafeTerminateThread( m_hHandleDownload );
		m_hHandleDownload = NULL;
	}
	*/
	if(m_hHandleInstall)
	{
		SafeTerminateThread( m_hHandleInstall );
		m_hHandleInstall = NULL;
	}
	
	return WaitForRepairDone();
	return S_OK;
}

HRESULT CImplRepairVul::WaitForRepairDone()
{
	if(m_hHandleDownload)
	{
		WaitForSingleObject(m_hHandleDownload, INFINITE);
		CloseHandle( m_hHandleDownload );
		m_hHandleDownload = NULL;
	}
	if(m_hHandleInstall)
	{
		WaitForSingleObject(m_hHandleInstall, INFINITE);
		CloseHandle( m_hHandleInstall );
		m_hHandleInstall = NULL;
	}
	return S_OK;
}

HRESULT CImplRepairVul::GetItemFilePath( INT nID, BSTR *bstr )
{
	if(!bstr)
		return E_POINTER;
	
	int idx = FindArrayIndex( m_arrDownloadItem, nID );
	if(idx==-1)
		return E_FAIL;
	*bstr = m_arrDownloadItem[idx].strFilename.AllocSysString();
	return S_OK;
}

void CImplRepairVul::Notify( TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2 )
{
	if(m_Observer)
		m_Observer->OnVulfixEvent(evt, nKbId, dwParam1, dwParam2);
}

BOOL CImplRepairVul::OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam )
{
	TVulFixEventType t;
	switch(state)
	{
	case ProcessState_ReceiveData:
        if (FALSE == m_bDownloadHasReceivedData)
            m_bDownloadHasReceivedData = TRUE;
		t = EVulfix_DownloadProcess;
		break;
	
	case ProcessState_Finished:
	case ProcessState_UserCanceled:
	case ProcessState_Failed:
	default:
		return TRUE; 
	}
	Notify( t, int((INT_PTR)pDownload->GetUserData()), pDownload->GetRemoteFileInfo()->fileDownloaded, 0); 
	return TRUE;
}

void CImplRepairVul::OnFileDownloaderEvent( IFileDownloaderObserver::TFileDownloaderEventType evt, DWORD dwParam1, DWORD dwParam2, PVOID pUserData )
{
	TVulFixEventType t;
	switch(evt)
	{
	case IFileDownloaderObserver::DOWNLOAD_STATUS:
        if (FALSE == m_bDownloadHasReceivedData)
            m_bDownloadHasReceivedData = TRUE;
		t = EVulfix_DownloadProcess;
		break;
	case IFileDownloaderObserver::DOWNLOAD_COMPLETE:	// 这2个事件, 由调用者返回 
	case IFileDownloaderObserver::DOWNLOAD_ERROR:
		return; 
	}
	Notify( t, int((INT_PTR)pUserData), dwParam1, dwParam2); 
}

DWORD WINAPI CImplRepairVul::_ThreadFuncDownload( LPVOID lpVoid )
{
	CImplRepairVul *lpScan = (CImplRepairVul*) lpVoid;
	return lpScan->_ThreadFuncDownloadImpl();
}

DWORD WINAPI CImplRepairVul::_ThreadFuncInstall( LPVOID lpVoid )
{
	CImplRepairVul *lpScan = (CImplRepairVul*) lpVoid;
	return lpScan->_ThreadFuncInstallImpl();
}

DWORD CImplRepairVul::_ThreadFuncDownloadImpl()
{
	do 
	{
		m_thinfo.Reset();
		
		INT nTotalFixItem = m_arrDownloadItem.GetSize();
		if(nTotalFixItem==0)
			break;
		
		m_thinfo.totalNum = nTotalFixItem;
		
		//
		BOOL bImportLocal = m_dwRepairFlags&VULFLAG_REPAIR_IMPORT_LOCAL;
		BOOL bDownloadIfFailImport = m_dwRepairFlags&VULFLAG_REPAIR_DOWNLOAD_IF_FAIL_IMPORT;
		bImportLocal = bImportLocal && !m_strImportPath.IsEmpty();
		
		// 1. Download 

		CFileDownloader fdSingle;
		m_thinfo.m_pFileDownloaderSingle = &fdSingle;
		fdSingle.SetObserver( this );

		IDownload *fd = NULL;
		ATLVERIFY( SUCCEEDED(CreateDownloadObject(__uuidof(IDownload), (VOID**)&fd)) );
		m_thinfo.m_pFileDownloader = fd;
		fd->SetObserver( this );

		m_thinfo.m_fileDownloadLocal.SetObserver(this);
		for(int i=0; i<m_arrDownloadItem.GetSize() && !m_bCanceled; ++i)
		{
			T_RepairItem &item = m_arrDownloadItem[i];
            BOOL bNeedCheckCert = item.bCheckCert;
			DWORD dwDownfileStat = 0;
			
			Notify(EVulfix_DownloadBegin, item.nID, i, m_thinfo.totalNum);
			BOOL downSuccess = FALSE;
			if(item.type==VTYPE_SOFTLEAK && item.bDisableCOM)
			{
				downSuccess = TRUE;
			}
			else
			{
				downSuccess = FALSE;
				item.strFilename = m_strDownloadPath;
				CString strFilename;
				if(GetFileNameFromUrl(item.strURL, strFilename))
				{
					// 兼容正常文件名 
                    item.strFilename.Format(_T("%s\\%s"), m_strDownloadPath, strFilename);
					
					if(!IsFileExist(item.strFilename))
					{
                        item.strFilename.Format(_T("%s\\%s%s"), m_strDownloadPath, BK_FILE_PREFIX, strFilename);
						if(!IsFileExist(item.strFilename))
						{
							item.strFilename.Format(_T("%s\\%s"), m_strDownloadPath, strFilename);

							// download if file not exists 
							dwDownfileStat |= ERDOWN_LOCAL_NOT_EXISTS;
							BOOL toDownloadFromWeb = TRUE;
							if(bImportLocal)
							{
								m_thinfo.m_fileDownloadLocal.SetUserData((LPVOID)item.nID);

								CString strImportFilename;

								LPCTSTR lpszHttp = _T("http://");								
								if( _tcsnicmp(lpszHttp, m_strImportPath, _tcslen(lpszHttp))==0 )
								{
									// 支持本地http 导入 
									strImportFilename.Format(_T("%s/%s"), m_strImportPath, strFilename);
									fd->SetUserData((LPVOID)item.nID);
									fd->SetDownloadInfo(strImportFilename, item.strFilename);
									fd->Fetch();
								}
								else
								{
									// 本地路径导入 
									strImportFilename.Format(_T("%s\\%s"), m_strImportPath, strFilename);
									m_thinfo.m_fileDownloadLocal.SetDownloadInfo(strImportFilename, item.strFilename);
									if( !m_thinfo.m_fileDownloadLocal.Fetch() )
									{
										if(!m_bCanceled)
										{
											strImportFilename.Format(_T("%s\\%s%s"), m_strImportPath, BK_FILE_PREFIX, strFilename);
											m_thinfo.m_fileDownloadLocal.SetDownloadInfo(strImportFilename, item.strFilename);
											m_thinfo.m_fileDownloadLocal.Fetch();
										}
									}
								}

								if(PathFileExists(item.strFilename))
								{
									toDownloadFromWeb = FALSE;
									dwDownfileStat |= ERDOWN_IMPORT_LOCAL;
								}
								else
								{
									toDownloadFromWeb = bDownloadIfFailImport;
								}
							}
							
							if(toDownloadFromWeb && !m_bCanceled && !IsFileExist(item.strFilename) )
							{
								m_bDownloadHasReceivedData = FALSE;
                                for (int i = 0; i < 3 && !downSuccess && !m_bCanceled; i ++)
                                {
									// Download Fail, Wait for 3 seconds then to retry 
									if(i>0)
									{
										DWORD dwTimeWaitEnd = 3*1000 + GetTickCount();
										for(;!m_bCanceled && GetTickCount()<dwTimeWaitEnd;)
										{
											::Sleep(100);
										}
										if(m_bCanceled)
											break;
									}

                                    fd->SetUserData((LPVOID)item.nID);
                                    fd->SetDownloadInfo(item.strURL, item.strFilename);
                                    downSuccess = fd->Fetch();

									IDownStat *pdownstat = fd->GetDownloadStat();
									if(pdownstat && pdownstat->Downloaded()>0)
										m_bDownloadHasReceivedData = TRUE;

                                    if (downSuccess && bNeedCheckCert)
                                    {
                                        // 校验补丁包数字签名 
                                        HRESULT hVerifyRet = _VerifyWinTrustCert(item.strFilename);
                                        if(!IsValidPatchFile(item.strFilename) || FAILED(hVerifyRet))
                                        {
											dwDownfileStat |= ERDOWN_MULTIDOWN_ERRORSIGN;
                                            Notify(EVulfix_Download_Check_Error, item.nID, hVerifyRet, 0);
#ifdef _DEBUG
											MessageBox(NULL, _T("下载文件签名验证失败!!!"), NULL, MB_OK);
#else
                                            DeleteFile(item.strFilename);
											m_bDownloadHasReceivedData = TRUE;

											// 校验失败走单线程 
											downSuccess = fdSingle.Download(item.strURL, item.strFilename , NULL, (LPVOID)(INT_PTR)item.nID);
#endif                                      
											bNeedCheckCert = FALSE;
                                            break;
                                        }
                                        bNeedCheckCert = FALSE;
                                    }
                                }

								if(!m_bDownloadHasReceivedData)
									dwDownfileStat |= ERDOWN_CANNOT_REACH_OFFICIAL;

								// 如果未收到任何数据 
                                if (!downSuccess && !m_bCanceled && !m_bDownloadHasReceivedData && !item.strMirrorURL.IsEmpty())
                                {
									dwDownfileStat |= ERDOWN_MIRROR_USED;

									// 删除之前下载的所有文件?! , 因为如果一点数据都没有下载到的话, 就没必要删除临时文件了
                                    fd->SetUserData((LPVOID)item.nID);
                                    fd->SetDownloadInfo(item.strMirrorURL, item.strFilename);
                                    fd->Fetch(1);
                                }
							}
						}
					}
					
					downSuccess = IsFileExist(item.strFilename);
					if(downSuccess)
					{
                        if (bNeedCheckCert)
                        {
                            // 校验补丁包数字签名
                            HRESULT hVerifyRet = _VerifyWinTrustCert(item.strFilename);
                            if(FAILED(hVerifyRet))
                            {
                                Notify(EVulfix_Download_Check_Error, item.nID, hVerifyRet, 1);
                                DeleteFile(item.strFilename);
                                downSuccess = FALSE;
								dwDownfileStat |= ERDOWN_ERRORSIGN;
                            }
                        }
                        if(!IsValidPatchFile(item.strFilename))
						{
							DeleteFile(item.strFilename);
							downSuccess = FALSE;
						}
					}
				}
			}
			item.stateDownload = downSuccess;
			Notify(downSuccess ? EVulfix_DownloadEnd : EVulfix_DownloadError, item.nID, m_bCanceled, dwDownfileStat);
			
			if( downSuccess )
			{
				++m_thinfo.nDownSuccess;
				SetEvent(m_thinfo.m_hInstallEvent);
			}
		}
		{
			CObjGuard __guard__(m_thinfo.m_objLockFileDownloader, TRUE);
			m_thinfo.m_pFileDownloaderSingle = NULL;
            m_thinfo.m_pFileDownloader = NULL;
			m_thinfo.m_pFileDownloaderSingle = NULL;
			m_thinfo.m_fileDownloadLocal.SetObserver(NULL);
		}
		fd->SetObserver( NULL );
		fd->Release();
		fd = NULL;		
	} while (FALSE);
	
	m_thinfo.isDownloadDone = TRUE;
	SetEvent(m_thinfo.m_hInstallEvent);
	return 0;
}

DWORD CImplRepairVul::_ThreadFuncInstallImpl()
{
	T_ComInit _init_com_;
	while(!m_bCanceled)
	{
		WaitForSingleObject(m_thinfo.m_hInstallEvent,INFINITE);
		if(m_thinfo.nInstallIndex>=m_arrDownloadItem.GetSize())
		{
			if(m_thinfo.isDownloadDone) // 如果文件已经下载结束 
				break;
			else 
				continue;
		}
		
		BOOL bLastInstallSuccess = FALSE;
		for( int i=m_thinfo.nInstallIndex;i<m_arrDownloadItem.GetSize() && !m_bCanceled;i++)
		{
			T_RepairItem &item = m_arrDownloadItem[i];
			if(item.stateDownload==-1)
				break;
			if(item.stateInstalled!=-1) // 已经安装了 
				continue;
			
			if(!bLastInstallSuccess)
				FixUpdateGarbage();

			m_thinfo.nInstallIndex = i + 1;
			if(item.stateDownload)
			{
				CString strFileName = item.strFilename;
				CString strInstallParam = item.strInstallParam;
#ifdef _DEBUG
				strInstallParam = _T("");
#endif
				_FixSystemRootSpecialPath( item );
				
				BOOL bUserPatcher = FALSE;
				E_InstalledResult res_inst = EINST_FAIL;
				DWORD dwExitCode = 0;
				INT userPatchCode = -1;
				BOOL disableCOM = ( item.type==VTYPE_SOFTLEAK && item.bDisableCOM );		
				Notify(EVulfix_InstallBegin, item.nID, i, m_thinfo.totalNum);	
				if( disableCOM )
				{
					if( m_pCurrentVulfix )
					{
						m_pCurrentVulfix->EnableVulCOM( item.nID, false );
						res_inst = EINST_SUCCESS;
					}
				}
				else
				{
					BOOL bHijacked = FALSE;
					if( !ExecuteFile(strFileName, strInstallParam, dwExitCode, bHijacked) )
						res_inst = EINST_FAIL_EXECUTE;
					else
					{
						BOOL exactInstalled = ERROR_SUCCESS_REBOOT_REQUIRED==dwExitCode || ERROR_SUCCESS_REBOOT_INITIATED==dwExitCode;
						if(!exactInstalled && item.type==VTYPE_OFFICE)
							 exactInstalled = 17025==dwExitCode;

						if( exactInstalled )
							res_inst = EINST_SUCCESS;
						else if( 0x80240017==dwExitCode )
						{
							// Vista 的未找到产品
							// Office 的未找到产品错误码 
							res_inst = EINST_FAIL_NOT_APPLICABLE;
						}
						else
						{
							if(dwExitCode==0)
								res_inst = EINST_SUCCESS_CHECK_FAILED;

							BOOL bMeetCondition = TRUE, bFoundInRegistry = FALSE;

							// 检测条件是否还继续满足 
							if( !item.strCondtion.IsEmpty() )
								bMeetCondition = EvaluateCondition( item.strCondtion );
							CSysEnv& sysEnv = singleton<CSysEnv>::Instance();
							if( item.type==VTYPE_OFFICE||item.type==VTYPE_SOFTLEAK )
							{
								// Office, Software 的检查注册表, 确认安装成功 
								if (sysEnv.m_WinVer >= WINVISTA)
								{
									CWUAUpdateInfoVistaLaterReg vs;
									vs.Init(0);
									bFoundInRegistry = vs.IsUpdateInstalled( item.nID );
								}
								else
								{
									CWUAUpdateInfoXP xp;
									xp.Init(0);
									bFoundInRegistry = xp.IsUpdateInstalled( item.nID );
								}
							}

							BOOL bInstalledCombined = !bMeetCondition || bFoundInRegistry;							
							if(bInstalledCombined)
							{
								res_inst = EINST_SUCCESS_CHECKED;
							}
							else
							{
								if(item.type==VTYPE_OFFICE) // 对office进行自定义安装
								{
									bUserPatcher = TRUE;
									Notify( EVulfix_InstallBeginUser, item.nID, i, dwExitCode);
									userPatchCode = _UserPatch(item);
									if(KPATCHER_OK==userPatchCode)
										res_inst = EINST_SUCCESS;
									else
									{
										res_inst = EINST_FAIL;	// 可能被360 拦截?? 然后就只能忽略了?? 不进行智能忽略!!
										//if(dwExitCode==17031) // 智能忽略
										//	res_inst = EINST_FAIL_NOT_APPLICABLE;
									}				
								}
								else if(item.type==VTYPE_SOFTLEAK)
								{
									if(bMeetCondition)
										res_inst = EINST_FAIL;
								}
							}

							LPCTSTR szFilename = _tcsrchr(strFileName, _T('\\'));
							SHOWMSG(_T("Installed Result: VulType:%d ExitCode:%08x(%d) \
									   \r\nConditionCheck:%d InReg:%d UserPatch:%d(%d)\
									   \r\nInstRes:%d(%d)(Exact:%d)  InstRes2:%d\
									   \r\nFile: %s \r\n")
									   , item.type, dwExitCode, dwExitCode, bMeetCondition, bFoundInRegistry, item.bUserPatcher, userPatchCode, res_inst, res_inst<=EINST_SUCCESS_CHECK_FAILED, exactInstalled, bInstalledCombined, szFilename+1);

							if(res_inst!=EINST_SUCCESS_CHECKED && res_inst!=EINST_SUCCESS)
							{
								if(bHijacked)
									res_inst = EINST_FAIL_HIJACKED;
							}
						}
					}
				}
				if( res_inst==EINST_FAIL_NOT_APPLICABLE )
				{
					m_objIgnore.AutoIgnore(item.nID, TRUE);
					m_objIgnore.SaveIgnoreDB();
				}

				BOOL bInstalled = res_inst<=EINST_SUCCESS_CHECK_FAILED;
				item.stateInstalled = res_inst;
				if( bInstalled )
					++ m_thinfo.nInstSuccess;
				if(bUserPatcher && !bInstalled && userPatchCode==KPATCHER_ERR_FILEBUSY)
					res_inst = EINST_FAIL_FILEBUSY;
				Notify( bInstalled ? EVulfix_InstallEnd : EVulfix_InstallError, item.nID, res_inst, bUserPatcher ? userPatchCode : dwExitCode);

				// 通知WUA 已经被修改了, 使得 Vista 的话, 需要重新的去扫描IUpdate 接口
				if( bInstalled && !disableCOM )
				{
					CWUAUpdateInfoVistaLater &wuaInfo = singleton<CWUAUpdateInfoVistaLater>::Instance();
					wuaInfo.SetWUAModified();
				}
				bLastInstallSuccess = bInstalled;
			}
		}
		
		if( m_bCanceled )
			break; 
		
		if( m_thinfo.isDownloadDone 
			&& m_thinfo.nInstallIndex>=(m_arrDownloadItem.GetSize()-1) )
		{			
			Notify( m_thinfo.totalNum==m_thinfo.nDownSuccess && m_thinfo.nDownSuccess==m_thinfo.nInstSuccess ? EVulfix_Task_Complete : EVulfix_Task_Error, 0, 0, 0);
			break;
		}
	}
	m_thinfo.isInstallDone = TRUE;
	return 0;
}

INT CImplRepairVul::_UserPatch( T_RepairItem &item )
{
	INT err = KPATCHER_ERR_GENERAL;
	CString strFileName = item.strFilename;
	INT nPos = strFileName.ReverseFind(_T('\\'));
	if( nPos>=0 )
	{
		CString strName;
		strName = strFileName.Mid( nPos+1 );
		nPos = strName.ReverseFind(_T('.'));
		if(nPos>=0)
		{
			strName = strName.Left( nPos );
			CString strNamePrefix = strName.Left( _tcslen(BK_FILE_PREFIX) );
			if(strNamePrefix==BK_FILE_PREFIX)
				strName = strName.Mid( strNamePrefix.GetLength() );
			strName.Append( _T(".xml"));

			if(!m_pUserPatcher)
			{
				CString str7z, strMsi;
				CompletePathWithModulePath( str7z, _T("7z.dll") );
				CompletePathWithModulePath( strMsi, _T("msi.wcx") );
				m_pUserPatcher = new CUserPatcher;
				m_pUserPatcher->Initialize(str7z, strMsi);
			}
			
			CString strPatchInfoUrl;
			CString strNameLower = strName;
			strNameLower.MakeLower();
			strPatchInfoUrl.Format(VULFIX_OFFICE_XML_URL, strNameLower);

			CString strXmlFilePath, strXmlFileName;
			strXmlFilePath = m_strDownloadPath;
			strXmlFilePath.AppendFormat(_T("\\%s%s"), BK_FILE_PREFIX, strXmlFileName);
			
			if(GetFileNameFromUrl(strPatchInfoUrl, strXmlFileName))
			{
#ifdef _USE_SINGLE_THREAD_DOWNLOAD_
				CFileDownloader fd;
				fd.Download( strPatchInfoUrl, strXmlFilePath, NULL, NULL, FALSE );
#else				
				IDownload *fd = NULL;
				if( SUCCEEDED(CreateDownloadObject(__uuidof(IDownload), (VOID**)&fd)) && fd)
				{
					fd->SetDownloadInfo(strPatchInfoUrl, strXmlFilePath);
					fd->Fetch();
					fd->Release();
				}
#endif 
			}
			
			if( IsFileExist(strXmlFilePath) )
			{
				T_UserPatchInfo patch_info;
				patch_info.nKBID = item.nID;

				if( ParseUserPatcherXml(strXmlFilePath, patch_info) )
				{
					for(int m=0; m<patch_info.files.GetSize(); ++m)
					{
						ExpandFilePath( patch_info.files[m].strFilenameTo );
						SHOWMSG(_T("Patch To -> %s "), patch_info.files[m].strFilenameTo);
					}
					err = m_pUserPatcher->InstallPatch( strFileName, patch_info );
					SHOWMSG(_T("Installer UserPatcher %s -> %d\n"), strFileName, err);
				}
				else
				{
					err = KPATCHER_ERR_LOADXML;
					SHOWMSG(_T("UserPatcher LoadPathcerXml Error %s\n"), strXmlFilePath );
				}
				DeleteFile(strXmlFilePath);
			}
			else
			{
				err = KPATCHER_ERR_DOWNLOADFILE;
			}
		}
	}
	return err;
}

HRESULT CImplRepairVul::_VerifyWinTrustCert(LPCTSTR lpszPatchFullFileName)
{
    static WinMod::CWinTrustVerifier s_hWinTrustVerifier;

    s_hWinTrustVerifier.TryLoadDll();
    DWORD dwVerifyRet = s_hWinTrustVerifier.VerifyEmbeddedWinTrustFile(lpszPatchFullFileName, NULL, NULL);
    return (dwVerifyRet <= 0x0000FFFF) ? AtlHresultFromWin32(dwVerifyRet) : dwVerifyRet;
}

void CImplRepairVul::_FixSystemRootSpecialPath( T_RepairItem &item )
{
	if ( m_strSystemRoot.IsEmpty() )
	{
		WCHAR szBuf[MAX_PATH];
		// c:\\windows
		::GetWindowsDirectory(szBuf, MAX_PATH);
		m_strSystemRoot = szBuf;
		m_strSystemRoot.TrimRight( L'\\' );
		m_strSystemRoot += L"\\";
	}

	 _IfFileExistDelete( _T("$hf_mig$"),	SRSP_FIX_DEFINE_WINDOWS );
	 _IfFileExistDelete( _T("ie7updates"),	SRSP_FIX_DEFINE_WINDOWS );
	 _IfFileExistDelete( _T("ie8updates"),	SRSP_FIX_DEFINE_WINDOWS );
	 _IfFileExistDelete( _T("dllcache"),	SRSP_FIX_DEFINE_SYSTEM32 );
	 
	 CString strTargetDir = m_strSystemRoot + _T("$hf_mig$\\");
	 _FixPrefixFile( strTargetDir, _T("kb") );

	 CString strKBName, strKBStr;
	 _FormatKBString( item.nID, strKBStr );
	 strKBName.Format( L"$NtUninstall%s$", strKBStr );
	 _IfFileExistDelete( strKBName, SRSP_FIX_DEFINE_WINDOWS );
}

void CImplRepairVul::_FixPrefixFile( LPCTSTR lpszTargetDir, LPCTSTR lpszPrefix )
{
	if ( !lpszTargetDir || !*lpszTargetDir || !lpszPrefix || !*lpszPrefix )
	{
		return ;
	}

	WinMod::CWinFileFind hFileFinder;
	CString strTargetDir(lpszTargetDir), strFindPath;
	strFindPath = strTargetDir + L'*';
	BOOL bFind = hFileFinder.FindFirstFile( strFindPath );
	for (NULL; bFind; bFind = hFileFinder.FindNextFile())
	{
		if ( hFileFinder.IsDirectory() )
		{
			continue;
		}

		CString strFileName = hFileFinder.GetFileName();
		strFileName.MakeLower();
		if ( 0 == strFileName.Find( lpszPrefix ) )
		{
			strFindPath = strTargetDir + hFileFinder.GetFileName();

			SetFileAttributes( strFindPath, 0 );
			DeleteFile( strFindPath );
		}
	}
	hFileFinder.Close();
}

void CImplRepairVul::_IfFileExistDelete( LPCTSTR lpszFileName, SRSP_FIX_DEFINE nDirDef )
{
	CString strDetectPath;
	switch ( nDirDef )
	{
	case SRSP_FIX_DEFINE_WINDOWS:
		strDetectPath = m_strSystemRoot;
		break;
	case SRSP_FIX_DEFINE_SYSTEM32:
		strDetectPath = m_strSystemRoot + L"system32\\";
		break;
	}

	strDetectPath += lpszFileName;
	if ( IsFileExist( strDetectPath ) )
	{
		SetFileAttributes( strDetectPath, 0 );
		DeleteFile( strDetectPath );
	}
}

void CImplRepairVul::_FormatKBString( INT nKB, CString &str )
{
	if(nKB<KB_VER_BASE)
		str.Format(_T("KB%d"), nKB);
	else
	{
		INT nV = nKB/KB_VER_BASE;
		INT n = nKB-nV*KB_VER_BASE;
		str.Format(_T("KB%d-v%d"), n, nV);
	}	
}
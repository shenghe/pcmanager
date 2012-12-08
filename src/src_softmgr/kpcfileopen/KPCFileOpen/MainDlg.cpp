
#include "stdafx.h"
#include "app.h"
#include "maindlg.h"


UINT_PTR CMainDlg::DoModal(HWND hWndParent)
{
	return __super::DoModal(hWndParent);
}

BOOL CMainDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
	InitInterface();

	int nPos = 0;
	nPos = g_strCmd.ReverseFind('\\');
	if (nPos != -1)
		m_strFileName = g_strCmd.Right(g_strCmd.GetLength() - nPos - 1);

	if (m_strFileName.GetLength() > 23)
	{
		CString strTemp;
		strTemp = m_strFileName.Left(9) + L"..." + m_strFileName.Right(9);
		m_strFileName = strTemp;
	}
	if (g_strTip.GetLength() == 0)
		g_strTip = L"未知文件";
	this->FormatRichText(
		IDC_TXT_FILE_NAME, 
		BkString::Get(IDS_FILE_NAME_FORMAT), 
		m_strFileName,
		g_strTip);

	m_csPhoneNess.Init();
	m_IECtrl.Create(this->GetViewHWND(), IDC_CTL_IE, FALSE, RGB(0xFB, 0xFC, 0xFD));
	m_IECtrl.SetExternalDispatch(this);
	m_IECtrl.SetCallback(this);

	CString strErrorHtml;
	if ( SUCCEEDED(GetKSoftDirectory(strErrorHtml)) )
		strErrorHtml.Append(_T("\\html\\error.html"));
	else
		strErrorHtml.Empty();

	m_IECtrl.Show2(g_strUrl, strErrorHtml);
	SetTimer( TIMER_LOADINFO_BIGBTN, 100, NULL );

	return TRUE;
}

void CMainDlg::OnNavigateComplete(int id)
{
	if (id == IDC_CTL_IE)
	{
		KillTimer( TIMER_LOADINFO_BIGBTN );

		SetItemVisible(IDC_LOADING_TIP, FALSE);
		SetItemVisible(IDC_CTL_IE, TRUE);
	}
}

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_LOADINFO_BIGBTN)
	{
		RollTheTip();
	}
	else
		SetMsgHandled(FALSE);
}

void CMainDlg::RollTheTip()
{
	static int nPos = 0;

	SetItemIntAttribute(IDC_LOADING_TIP_ICO, "sub", (nPos)%8);
	nPos++;
}

void CMainDlg::OnDestroy()
{
	KillTimer( TIMER_LOADINFO_BIGBTN );

	m_csPhoneNess.Term();
}

void CMainDlg::OnBkBtnClose()
{
	EndDialog(IDCLOSE);
}

void CMainDlg::OnBkBtnOK()
{
	EndDialog(IDOK);
}

void CMainDlg::OnBkBtnDefault()
{
	TCHAR dstPath[MAX_PATH] = {0};
	::SHGetSpecialFolderPath(NULL, dstPath, CSIDL_WINDOWS, FALSE);
	::PathAddBackslash(dstPath);

	CString strParam;
	strParam.Format(L"%ssystem32\\shell32.dll,OpenAs_RunDLL %s", 
		dstPath,
		g_strCmd);
	CString strExe;
	strExe.Format(L"%ssystem32\\rundll32.exe", dstPath);
	::ShellExecute(NULL, L"open", strExe, strParam, NULL, SW_SHOWNORMAL);

	//EndDialog(IDCLOSE);
}

namespace
{
	const DISPID BASE_DIPSPID = 1000;
	const struct
	{
		LPCWSTR			_pFuncName;		// 名称
		DISPID			_dispId;		// 分发ID
		UINT			_argc;			// 函数参数个数（必须不BSTR类型）
		HRESULT (CMainDlg::*_pFuncProc)(DISPID, DISPPARAMS*, VARIANT*);
	} 
	sPhoneJsFunc[] = 
	{
		//
		//@Note
		// _dispId必须从BASE_DIPSPID递增
		//
		{L"SoftExists",		BASE_DIPSPID + 0,	1,	&CMainDlg::PCSoftExists},
		{L"StartDown",		BASE_DIPSPID + 1,	5,	&CMainDlg::PCSoftStartDown},
		{L"PauseDown",		BASE_DIPSPID + 2,	1,	&CMainDlg::PCSoftPauseDown},
		{L"ResumeDown",		BASE_DIPSPID + 3,	1,	&CMainDlg::PCSoftResumeDown},
		{L"StopDown",		BASE_DIPSPID + 4,	1,	&CMainDlg::PCSoftStopDown},
		{L"GetDownState",	BASE_DIPSPID + 5,	1,	&CMainDlg::PCSoftGetDownState},
		{L"OpenDownedSoft",	BASE_DIPSPID + 6,	1,	&CMainDlg::PCSoftOpenDownedSoft},
		{L"SoftRun",		BASE_DIPSPID + 7,	1,	&CMainDlg::PCSoftRun},
		{L"SoftRunDefault",	BASE_DIPSPID + 8,	1,	&CMainDlg::PCSoftRunDefault},
	};
}
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMainDlg::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = NULL;
	if(riid == __uuidof(IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	else if(riid ==  __uuidof(IDispatch))
	{
		*ppvObject = static_cast<IDispatch*>(this);
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP CMainDlg::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	if(cNames == 0 || rgszNames == NULL || rgszNames[0] == NULL || rgDispId == NULL)
	{
		return E_INVALIDARG;
	}

	for(int i = 0; i < STLSOFT_NUM_ELEMENTS(sPhoneJsFunc); ++i)
	{
		if(_wcsicmp(rgszNames[0], sPhoneJsFunc[i]._pFuncName) == 0)
		{
			*rgDispId = sPhoneJsFunc[i]._dispId;
			return S_OK;
		}
	}

	return E_INVALIDARG;
}

STDMETHODIMP CMainDlg::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	if(
		dispIdMember < BASE_DIPSPID || 
		dispIdMember > sPhoneJsFunc[STLSOFT_NUM_ELEMENTS(sPhoneJsFunc) - 1]._dispId
		)
	{
		return E_NOTIMPL;
	}

	// 检查参数个数
	if(pDispParams->cArgs != sPhoneJsFunc[dispIdMember - BASE_DIPSPID]._argc)
	{
		return E_INVALIDARG;
	}

	// 所有的参数都是字符串
	for(UINT i = 0; i < pDispParams->cArgs; ++i)
	{
		if((pDispParams->rgvarg[i].vt & VT_BSTR) == 0)
		{
			return E_INVALIDARG;
		}
	}

	return (this->*(sPhoneJsFunc[dispIdMember - BASE_DIPSPID]._pFuncProc))(dispIdMember, pDispParams, pVarResult);
}

void CMainDlg::PhoneSoftDownCallback(DTManager_Stat st,void* tk,void* para)
{
	CMainDlg *pDlg = reinterpret_cast<CMainDlg*>(para);
	pDlg->PhoneSoftDownProcess(st, tk);
}

void CMainDlg::PhoneSoftDownInfoCallback(__int64 cur,__int64 all,__int64 speed,DWORD time,void* para)
{
	PPhoneSoft pPhoneSoft = reinterpret_cast<PPhoneSoft>(para);

	pPhoneSoft->speed = static_cast<ULONG>(speed);
	pPhoneSoft->recved = static_cast<ULONG>(cur);
}

void CMainDlg::PhoneSoftDownProcess(DTManager_Stat st, void *tk)
{
	CriticalSectionScoped locker(m_csPhoneNess);

	Dt2IdIter itDt = m_dt2Id.Lookup(tk);
	if(itDt == NULL) return;

	Id2PhoneSoftIter itSoft = m_id2PhoneSoft.Lookup(itDt->m_value);
	ATLASSERT(itSoft != NULL);

	switch(st)
	{
	case TASK_DOWNING:
		{
			itSoft->m_value.state = PDS_DOWNING;
			m_pPhoneDTMgr->QueryTask(itSoft->m_value.idDown, PhoneSoftDownInfoCallback, &itSoft->m_value);
		}
		break;

	case TASK_DONE:
		itSoft->m_value.state = PDS_DONE;
		break;

	case TASK_ERROR:
		itSoft->m_value.state = PDS_ERROR;
		break;

	case TASK_ERROR_MD5:
		itSoft->m_value.state = PDS_ERROR_MD5;
		break;
	}
}

HRESULT CMainDlg::PCSoftExists(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	CString strStore;
	BKSafeConfig::GetStoreDir(strStore);
	strStore = _PathAddBackslash(strStore) + pDispParams->rgvarg[0].bstrVal;

	CComVariant retVal = (::PathFileExists(strStore) ? true : false);
	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}
	return S_OK;
}

HRESULT CMainDlg::PCSoftStartDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	CComVariant retVal = false;

	do
	{
		LONG id = safe_atol(pDispParams->rgvarg[4].bstrVal);
		if(id == 0)
		{
			break;
		}

		//
		// 若文件已存在，则直接返回
		//
		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		SHCreateDirectory(NULL, strStore);

		strStore = _PathAddBackslash(strStore) + pDispParams->rgvarg[1].bstrVal;
		if(::PathFileExists(strStore))
		{
			retVal = true;
			break;
		}

		CriticalSectionScoped locker(m_csPhoneNess);

		// 若任务已经存在，则直接返回
		{
			Id2PhoneSoftIter itSoft = m_id2PhoneSoft.Lookup(id);
			if(itSoft != NULL)
			{
				retVal = false;
				break;
			}
		}

		IDTManager *pDTMgr = GetDTMgrForPhone();
		if(pDTMgr != NULL)
		{
			int curPos = 0;
			CString strUrl = pDispParams->rgvarg[3].bstrVal;

			CString token = strUrl.Tokenize(L" ", curPos);
			if(!token.IsEmpty())
			{
				CAtlArray<CString> *pUrlArray = new CAtlArray<CString>();

				do
				{
					pUrlArray->Add(token);
					token = strUrl.Tokenize(L" ", curPos);
				}
				while(!token.IsEmpty());

				PhoneSoft ps;
				ps.speed = 0;
				ps.recved = 0;
				ps.state = PDS_INIT;
				ps.idDown = pDTMgr->NewTask(pUrlArray, pDispParams->rgvarg[2].bstrVal, strStore);

				m_id2PhoneSoft[id] = ps;
				m_dt2Id[ps.idDown] = id;

				retVal = true;
			}
		}
	}
	while(FALSE);

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}
	return S_OK;
}

HRESULT CMainDlg::PCSoftPauseDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	CComVariant retVal = false;

	do
	{
		LONG id = safe_atol(pDispParams->rgvarg[0].bstrVal);
		if(id == 0)
		{
			break;
		}

		CriticalSectionScoped locker(m_csPhoneNess);

		IDTManager *pDTMgr = GetDTMgrForPhone();
		if(pDTMgr != NULL)
		{
			Id2PhoneSoftIter itSoft = m_id2PhoneSoft.Lookup(id);
			if(itSoft != NULL)
			{
				pDTMgr->PauseTask(itSoft->m_value.idDown);
				retVal = true;
			}
		}
	}
	while(FALSE);

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}
	return S_OK;
}

HRESULT CMainDlg::PCSoftResumeDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	CComVariant retVal = false;

	do
	{
		LONG id = safe_atol(pDispParams->rgvarg[0].bstrVal);
		if(id == 0)
		{
			break;
		}

		CriticalSectionScoped locker(m_csPhoneNess);

		IDTManager *pDTMgr = GetDTMgrForPhone();
		if(pDTMgr != NULL)
		{
			Id2PhoneSoftIter itSoft = m_id2PhoneSoft.Lookup(id);
			if(itSoft != NULL)
			{
				pDTMgr->ResumeTask(itSoft->m_value.idDown);
				retVal = true;
			}
		}
	}
	while(FALSE);

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}

	return S_OK;
}

HRESULT CMainDlg::PCSoftStopDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	CComVariant retVal = false;

	do
	{
		LONG id = safe_atol(pDispParams->rgvarg[0].bstrVal);
		if(id == 0)
		{
			break;
		}

		CriticalSectionScoped locker(m_csPhoneNess);

		IDTManager *pDTMgr = GetDTMgrForPhone();
		if(pDTMgr != NULL)
		{
			Id2PhoneSoftIter itSoft = m_id2PhoneSoft.Lookup(id);
			if(itSoft != NULL)
			{
				pDTMgr->CancelTask(itSoft->m_value.idDown);

				m_dt2Id.RemoveKey(itSoft->m_value.idDown);
				m_id2PhoneSoft.RemoveKey(id);

				retVal = true;
			}
		}
	}
	while(FALSE);

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}

	return S_OK;
}

HRESULT CMainDlg::PCSoftGetDownState(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	if(pVarResult == NULL) return S_OK;

	::VariantInit(pVarResult);
	pVarResult->vt = VT_BSTR;
	pVarResult->bstrVal = NULL;

	LONG id = safe_atol(pDispParams->rgvarg[0].bstrVal);
	if(id == 0)
	{
		return S_OK;
	}

	CriticalSectionScoped locker(m_csPhoneNess);		

	Id2PhoneSoftIter itSoft = m_id2PhoneSoft.Lookup(id);
	if(itSoft != NULL)
	{
		CString strState;
		strState.Format(L"{state:%d,speed:%d,recved:%d}", 
			itSoft->m_value.state, itSoft->m_value.speed, itSoft->m_value.recved);

		pVarResult->bstrVal = ::SysAllocString(strState);
	}
	else
	{
		CString strState;
		strState.Format(L"{state:%d,speed:%d,recved:%d}", 
			3, 0, 0);

		pVarResult->bstrVal = ::SysAllocString(strState);
	}

	return S_OK;
}

HRESULT CMainDlg::PCSoftOpenDownedSoft(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{	
	CString strCmdLine;
	BKSafeConfig::GetStoreDir(strCmdLine);
	::SHCreateDirectory(NULL, strCmdLine);

	CComVariant retVal;
	if(pDispParams->rgvarg[0].bstrVal == NULL || pDispParams->rgvarg[0].bstrVal[0] == L'\0')
	{
		retVal = true;
		::ShellExecute(NULL, L"open", strCmdLine, NULL, NULL, SW_SHOWNORMAL);
	}
	else
	{
		strCmdLine = _PathAddBackslash(strCmdLine) + pDispParams->rgvarg[0].bstrVal;

		BOOL bExist = ::PathFileExists(strCmdLine);
		if(bExist)
		{
			strCmdLine = L"/select," + strCmdLine;
			::ShellExecute(NULL, NULL, L"explorer.exe", strCmdLine, NULL, SW_NORMAL);
		}

		retVal = (bExist ? true : false);
	}

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}
	return S_OK;
}

HRESULT CMainDlg::PCSoftRun(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{	
	CComVariant retVal;
	if(pDispParams->rgvarg[0].bstrVal == NULL || pDispParams->rgvarg[0].bstrVal[0] == L'\0')
	{
		retVal = false;
	}
	else
	{
		ShowWindow(FALSE);

		CString strOld;
		CString strNew;
		GetRegExtInfo(strOld);

		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		SHCreateDirectory(NULL, strStore);

		strStore = _PathAddBackslash(strStore) + pDispParams->rgvarg[0].bstrVal;
		CString strCmd;
		strCmd = strStore;
		
		CreateProcessSyn(strCmd);

		GetRegExtInfo(strNew);
		if (strOld != strNew)
		{
			CBkSafeMsgBox2	msgbox;
			msgbox.AddButton(L"是", IDOK);
			msgbox.AddButton(L"否", IDCANCEL);
			CString strMsg;
			strMsg.Format(L"软件安装完成，是否立即打开文件“%s”", m_strFileName);
			UINT nRet = msgbox.ShowMutlLineMsg(strMsg, 
				NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
			if(nRet == IDOK)
			{
				::ShellExecute(NULL, L"open", g_strCmd, L"", NULL, SW_SHOWNORMAL);
			}
		}
		else
		{
			CBkSafeMsgBox2 msgbox;
			msgbox.AddButton( TEXT("确定"), IDOK);
			CString strMsg;
			strMsg.Format(L"由于您已取消，将无法打开文件“%s”", m_strFileName);
			msgbox.ShowMutlLineMsg( strMsg, NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );
		}

		OnBkBtnClose();
		//::PostMessage(m_hWnd, WM_CLOSE, 0, 0);

		retVal = true;
	}

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}

	return S_OK;
}

HRESULT CMainDlg::PCSoftRunDefault(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{	
	CComVariant retVal;
	//if(pDispParams->rgvarg[0].bstrVal == NULL || pDispParams->rgvarg[0].bstrVal[0] == L'\0')
	//{
	//	retVal = false;
	//}
	//else
	{
		ShowWindow(FALSE);

		//CString strCmd;
		//strCmd = pDispParams->rgvarg[0].bstrVal;

		OnBkBtnDefault();

		OnBkBtnClose();
		//::PostMessage(this->m_hWnd, WM_CLOSE, 0, 0);

		retVal = true;
	}

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}

	return S_OK;
}

IDTManager* CMainDlg::GetDTMgrForPhone()
{
	if(m_pPhoneDTMgr == NULL && CreateObject != NULL)
	{
		CreateObject(__uuidof(IDTManager), (void**)&m_pPhoneDTMgr);

		if(m_pPhoneDTMgr != NULL)
		{
			CAtlMap<DtStateChangeNotifyCallBack,void*> *pCallback = new CAtlMap<DtStateChangeNotifyCallBack,void*>;
			pCallback->SetAt(PhoneSoftDownCallback, this);

			m_pPhoneDTMgr->Init(pCallback);
		}
	}

	return m_pPhoneDTMgr;
}

LONG CMainDlg::safe_atol(LPCWSTR pStr, LONG def /* = 0 */)
{
	if(pStr == NULL) return def;
	return _wtol(pStr);
}

CString CMainDlg::_PathAddBackslash(const CString &path)
{
	if(
		path.IsEmpty() ||
		(path[path.GetLength() - 1] != '\\' && path[path.GetLength() - 1] != '/')
		)
	{
		return path + L'\\';
	}

	return path;
}

BOOL CMainDlg::InitInterface()
{
	CString	strDll;

	if ( SUCCEEDED(GetKSoftDirectory(strDll)) )
		strDll.Append(_T("\\softmgr.dll"));

	hInstance = LoadLibrary(strDll);
	if (hInstance == NULL)
	{
		return FALSE;
	}
	CreateObject = (pCreateObject)GetProcAddress(hInstance, "CreateSoftMgrObject");
}

HRESULT CMainDlg::GetKSoftDirectory(CString &strDir)
{
	HRESULT hr = SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strDir));
	if ( hr )
	{
		strDir = strDir.Left(strDir.ReverseFind(_T('\\')) + 1);
		strDir.Append(_T("ksoft"));
	}

	return hr;
}

BOOL CMainDlg::CreateProcessSyn(CString strCmd)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line). 
		(LPTSTR)(LPCTSTR)strCmd,				// Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
	{
		return FALSE;
	}

	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess, INFINITE );

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	return TRUE;
}

BOOL CMainDlg::GetRegExtInfo(CString& strInfo)
{
	CRegKey key;
	CString strKeyName;
	strKeyName = L"." + g_strExt;
	LONG lRet = key.Open(HKEY_CLASSES_ROOT, strKeyName, KEY_READ);
	if (lRet != ERROR_SUCCESS)
		return FALSE;

	wchar_t* szPathOld = new wchar_t[MAX_PATH];
	ULONG nSize = MAX_PATH;
	lRet = key.QueryStringValue(L"", szPathOld, &nSize);
	if (nSize > MAX_PATH)
	{
		if (szPathOld != NULL)
		{
			delete[] szPathOld;
			szPathOld = NULL;
		}

		szPathOld = new wchar_t[nSize + 1];
		lRet = key.QueryStringValue(L"", szPathOld, &nSize);
		if (lRet != ERROR_SUCCESS)
			return FALSE;
	}

	strInfo = szPathOld;

	if (szPathOld != NULL)
	{
		delete[] szPathOld;
		szPathOld = NULL;
	}
	return TRUE;
}
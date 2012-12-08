
#include "stdafx.h"
#include <string>
typedef std::basic_string<TCHAR>	string;
#include <new>
#include <fstream>
#include <ios>
#include <atlcoll.h>
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafesoftmgrHeader.h"
#include "BkSoftMgrPowerSweepDlg.h"
#include "..\..\publish\libheader\libheader.h"
#include "..\..\publish\zlibcrc32\crc32.h"
#include "..\..\publish\safemon\safetrayshell.h"
#include "BkSoftMgrDownloadMgrDlg.h"
#include "BkSoftMgrMarkDlg.h"
#include "BkSoftMgrNewInfoDlg.h"
#include "BkSoftMgrIgnoreDlg.h"
#include "bksoftmgruninstall.h"
//#include "ldregistroperate.h"
#include "iefix/RegisterOperate.h"
#include "beikesafesoftmgrnecess.h"
#include "BkSoftMgrOneKeyDlg.h"
#include "BkSoftMgrPlugInfoDlg.h"

#include <string>
#include <fstream>
#include <algorithm>
using namespace std;
#include <stlsoft/memory/auto_buffer.hpp>
using namespace stlsoft;
#include <iefix/iefixeng.h>

#define STR_DEFAULT_SOFTMGR		BkString::Get(IDS_SOFTMGR_8016)
#define STR_UNINSTALL_INI		_T("http://stat.ijinshan.com/ini/uninsthelp.ini")

#define TIMER_DOWNLOAD_DETAIL_SOFTMGR	1200  //刷新下载详情定时器
#define TIMRE_DOWNLOAD_CHECK_SOFTMGR	1201  //检测下载是否失败定时器
#define TIMER_LOADTIP_SOFTMGR			1202  //提取软件信息定时器
#define TIMER_LOADINFO_SOFTMGR			1203  //提取软件信息的时候展示图片定时器
#define TIMER_LOADINFO_BIGBTN			1204  // 点击大按钮之后加载信息的tip
#define TIMER_RESTART_KSAFE				1205  //检测软件库更新

enum { UNI_BTN_ALL=0, UNI_BTN_DESK, UNI_BTN_STARTMENU, UNI_BTN_QUICKLAN, UNI_BTN_PROC, UNI_BTN_TRAY };



#define LOG_FILE_NAME _T("DownloadFailed.log")

namespace
{

	// 路径结尾加反斜线
	inline CString _PathAddBackslash(const CString &path);
	// 字符串转数字
	inline LONG safe_atol(LPCWSTR pStr, LONG def = 0);

	//////////////////////////////////////////////////////////////////////////
	// 大全项目排序
	//
	typedef CSoftListItemData				InsItem;
	typedef CSimpleArray<InsItem*>			InsItemArray;

	// 名称
	class InsItemNameSort
	{
	public:
		InsItemNameSort(BOOL asc) :_asc(asc) {}

		bool operator()(const InsItem* pLeft, const InsItem* pRight) const
		{
			ATLASSERT(pLeft != NULL && pRight != NULL);
			if(_asc)
				return (wcscoll(pLeft->m_strName, pRight->m_strName) < 0);//(pLeft->m_strName.CompareNoCase(pRight->m_strName) < 0);
			else
				return (wcscoll(pLeft->m_strName, pRight->m_strName) > 0);//(pLeft->m_strName.CompareNoCase(pRight->m_strName) > 0);
		}

	private:
		BOOL _asc;
	};

	// 大小
	class InsItemSizeSort
	{
	public:
		InsItemSizeSort(BOOL asc) : _asc(asc) {}

		bool operator()(const InsItem* pLeft, const InsItem* pRight) const
		{
			ATLASSERT(pLeft != NULL && pRight != NULL);
			if(_asc)
				return (pLeft->m_dwSize < pRight->m_dwSize);
			else
				return (pLeft->m_dwSize > pRight->m_dwSize);
		}

	private:
		BOOL _asc;
	};

	// 评分
	class InsItemMarkSort
	{
	public:
		InsItemMarkSort(BOOL asc) : _asc(asc) {}

		bool operator()(const InsItem* pLeft, const InsItem* pRight) const
		{
			ATLASSERT(pLeft != NULL && pRight != NULL);
			if(_asc)
				return (pLeft->m_fMark < pRight->m_fMark);
			else
				return (pLeft->m_fMark > pRight->m_fMark);
		}

	private:
		BOOL _asc;
	};

	//////////////////////////////////////////////////////////////////////////
	// 卸载项目排序
	//
	typedef CSoftMgrUniExpandListView::SOFT_UNI_ITEM	UniItem;
	typedef CSimpleArray<UniItem*>						UniItemArray;

	// 名称
	class UniItemNameSort
	{
	public:
		UniItemNameSort(BOOL asc) :_asc(asc) {}

		bool operator()(const UniItem* pLeft, const UniItem* pRight) const
		{
			ATLASSERT(pLeft != NULL && pRight != NULL);
			if(_asc)
				return (wcscoll(pLeft->pInfo->strCaption, pRight->pInfo->strCaption) < 0);
			else
				return (wcscoll(pLeft->pInfo->strCaption, pRight->pInfo->strCaption) > 0);
		}

	private:
		BOOL _asc;
	};

	// 大小
	class UniItemSizeSort
	{
	public:
		UniItemSizeSort(BOOL asc) : _asc(asc) {}

		bool operator()(const UniItem* pLeft, const UniItem* pRight) const
		{
			ATLASSERT(pLeft != NULL && pRight != NULL);
			if(_asc)
				return (pLeft->pInfo->nSize < pRight->pInfo->nSize);
			else
				return (pLeft->pInfo->nSize > pRight->pInfo->nSize);
		}

	private:
		BOOL _asc;
	};

	// 评分
	class UniItemMarkSort
	{
	public:
		UniItemMarkSort(BOOL asc) : _asc(asc) {}

		bool operator()(const UniItem* pLeft, const UniItem* pRight) const
		{
			ATLASSERT(pLeft != NULL && pRight != NULL);
			if(_asc)
				return (pLeft->pInfo->nRunCount < pRight->pInfo->nRunCount);
			else
				return (pLeft->pInfo->nRunCount > pRight->pInfo->nRunCount);
		}

	private:
		BOOL _asc;
	};

	//
	// 从软件信息中提示图标
	//
	BOOL ExtractIconFromSoftInfo(LPCWSTR pcIconPath, HICON *phiconLarge, HICON *phiconSmall);

	//
	// 预处理搜索关键字
	//
	void PreprocessKeyword(const CString &input, CString &output);

};
//////////////////////////////////////////////////////////////////////////
template<int LogLevel>
class Log
{
public:
	Log()
	{
		CString strLog;
		CompletePathWithModulePath(strLog, LOG_FILE_NAME);
		m_fs.open(strLog, std::ifstream::out | std::ifstream::binary | std::ifstream::app | std::ifstream::ate);
	}

	template<typename T>
	Log& operator<<(T _Val)
	{
		m_fs << _Val; 
		return *this;
	}
	~Log()
	{
		m_fs.close();
	}
	std::ofstream m_fs;
};
typedef CComAutoCriticalSection                     CObjLock;
typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;
BOOL __cdecl MYDOWNLOADLOG( LPCTSTR xxx, ... )
{
	static CObjLock lock;
	CObjGuard _guard_(lock);

	CString	str;
	va_list arg_ptr; 
	va_start(arg_ptr, xxx); 
	str.FormatV( xxx, arg_ptr );
	va_end( arg_ptr );

	CString strDate;
	SYSTEMTIME st;
	GetLocalTime(&st);
	strDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	USES_CONVERSION;
	CStringA szDate = CT2CA(strDate);
	CStringA szMsg = CT2CA(str);

	static Log<1> log;
	log<< szDate << " : " << szMsg;
	log.m_fs.flush();

	return TRUE;
}

const static int MAX_VALUE_LENGTH=16383;

struct PARAM 
{
	void *pTask;
	void *pDlg;
};

static DWORD CalcFileCrc(FILE* lpFile)
{
	const int nBuffLen = 500;
	BYTE cBuff[nBuffLen];
	DWORD dwCRC = 0;

	while ( TRUE )
	{
		size_t dwRead = fread(cBuff, 1, nBuffLen, lpFile);
		if ( dwRead == 0 )
		{
			break;
		}

		dwCRC = CRC32(dwCRC, cBuff, (DWORD)dwRead);
	}

	return dwCRC;
}

//将字节数直接转换为字符串
CString CalcStrFormatByteSize(LONGLONG qdw)
{
	CString retStr;
	StrFormatByteSize64(qdw, retStr.GetBuffer(64), 64);
	retStr.ReleaseBuffer();

	return retStr;
}

CString CalcStrFormatByCount(LONGLONG qdw)
{
	CString retStr;

	if (qdw > 20)
		retStr = L"经常";
	else if (qdw <= 20 && qdw > 5)
		retStr = L"偶尔";
	else if (qdw <= 5 && qdw > 0)
		retStr = L"很少";
	else if (qdw == 0)
		retStr = L"从不";
	else//else if (qdw == -1)
		retStr = L"未知";

	return retStr;
}

void GetRunInfoFromTime(CString& strRunFrequency, LONG& nRunCount, LONG nLastTime)
{
	if (nLastTime == 0)
	{
		strRunFrequency = L"未知";
		nRunCount = -1;
		return ;
	}

	CTime	tLastUse( nLastTime );
	CTime	tNow = CTime::GetCurrentTime();
	CTimeSpan  tdiff = tNow - tLastUse;

	int nLastUse = tdiff.GetDays();
	if (nLastUse < 3 && nLastUse >= 0)
	{
		strRunFrequency = L"经常";
		nRunCount = 21;
	}
	else if (nLastUse < 15 && nLastUse >= 3)
	{
		strRunFrequency = L"偶尔";
		nRunCount = 6;
	}
	else if (nLastUse >= 15)
	{
		strRunFrequency = L"很少";
		nRunCount = 1;
	}
	else
	{
		strRunFrequency = L"未知";
		nRunCount = -1;
	}
}

CString CalcStrFormatByteTime(LONGLONG qdw)
{
	CString retStr;

	CTime	tLastUse( qdw );
	CTime	tNow = CTime::GetCurrentTime();
	CTimeSpan  tdiff = tNow - tLastUse;

	int nLastUse = tdiff.GetDays();
	if ( nLastUse == 0 )
		retStr = BkString::Get(IDS_SOFTMGR_8028);
	else if ( nLastUse > 30 )
		retStr = BkString::Get(IDS_SOFTMGR_8029);
	else if ( nLastUse <= 30 && nLastUse > 0)
		retStr.Format( BkString::Get(IDS_SOFTMGR_8030), nLastUse );
	else
		retStr = L"未知";

	return retStr;
}

//判断软件是否安装的回调函数
int CheckSetupCallBack(CString id, CString name, CString ver, CString curver, CString published, CString filename, void* param)
{
	int _nRet = 0;

	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler *)param;

	if (pDlg != NULL)
	{
		CSoftListItemData *pData = pDlg->m_ItemData.GetDataByID(id);
		if ( !pData )
			return 0;

		pDlg->m_cs.Lock();
		pData->m_bSetup = TRUE;
		pData->m_strNewVersion = ver;
		pData->m_strVersion = curver;
		pData->m_strPublished = published;
		pData->m_strMainExe = filename;

		if ( pData->m_bDownloading && !pData->m_bWaitDownload && !pData->m_bDownLoad)
		{
			pData->m_bWaitDownload = FALSE;
			pData->m_bWaitInstall = FALSE;
		}
		else
		{
			pData->m_bDownloading = FALSE; //已经安装则不显示进度条，显示完整信息
			pData->m_bInstalling = FALSE;
			pData->m_bWaitInstall = FALSE;
		}

		pDlg->m_cs.Unlock();

		if ( -1 == pDlg->m_arrCheckSetup.Find( pData ))
		{
			pDlg->m_arrCheckSetup.Add( pData );
		}

		//加入到检测升级的队列中
		if ( pDlg->IsNeedUpdate(pData) )
		{
			pData->m_bUpdate = TRUE;
			if ( pDlg->m_arrCheckUpdate.Find(pData) == -1 )
			{
				pDlg->m_arrCheckUpdate.Add(pData);
			}

			_nRet = 1;
		}
		else
		{
			BOOL bUpdatePointOutInfo = FALSE;
			if (pData->m_bUpdate == TRUE)
			{
				bUpdatePointOutInfo = TRUE;
				int nTmp = _wtoi( pData->m_strSoftID );
				pDlg->m_softUpdatedList.Add(nTmp);
			}

			pData->m_bUpdate = FALSE;
			if ( pDlg->m_arrCheckUpdate.Find(pData) != -1 )
			{
				pDlg->m_arrCheckUpdate.Remove(pData);
			}

			if (bUpdatePointOutInfo)
				pDlg->SaveUpdatePointOutInfo();

			_nRet = 0;
		}

		pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

		if ( pData->m_bDaquan )
		{
			int nRet;
			nRet = BKSafeConfig::GetDelFile();
			if (nRet == 1)
			{
				CString strStore;
				BKSafeConfig::GetStoreDir(strStore);
				if(strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
				{
					strStore.Append(_T("\\")+ pData->m_strFileName);
				}
				else
				{
					strStore.Append(pData->m_strFileName);
				}
				int nRtn = 0;
				if (PathFileExists(strStore))
				{
					nRtn = DeleteFile(strStore);
				}
			}

			pData->m_bDaquan = FALSE;
		}

	}
	return _nRet;
}

//获得下载信息
void CBeikeSafeSoftmgrUIHandler::DownloadInfoCallBack(int64 cur,int64 all,int64 speed,DWORD time,void* para)
{
	PARAM *pParam = (PARAM*)para;
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler *)pParam->pDlg;

	CString strSoftId;
	if ( pDlg->m_arrTaskMap.Lookup(pParam->pTask) != NULL)
	{
		strSoftId = pDlg->m_arrTaskMap.Lookup(pParam->pTask)->m_value;
	}

	if (strSoftId != _T(""))
	{
		for (int i = 0; i < pDlg->m_arrData.GetSize(); i++)
		{
			CSoftListItemData *pData = pDlg->m_arrData[i];
			if ( (pData != NULL) && (pData->m_strSoftID == strSoftId))
			{
				if(speed < 0)
					speed=0;
				CString strSpeed = CalcStrFormatByteSize(speed);
				strSpeed.Remove(_T(' '));
				pData->m_strSpeed.Format(_T("%s/s"), strSpeed);
				pData->m_fSpeed = (float)speed;
				if (pData->m_bLinking)
				{
					pData->m_bLinking = FALSE;
				}

				pData->m_dwTime = time;
				if (all > 0 && cur <= all)
				{
					pData->m_dwProgress = (DWORD)((cur * 100) / all);
				}
				pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);
				//else if(all <= 0)
				//{
				//	pData->m_dwProgress = 0;
				//}
				break;
			}
		}
	}

	if (pParam != NULL)
	{
		delete pParam;
		pParam = NULL;
	}
}

//下载回调函数
void DownloadCallBack(DTManager_Stat st, void* tk,void* para)
{
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler*)para;
	PARAM *pParam = new PARAM;
	pParam->pDlg = para;
	pParam->pTask = tk;

	int nErr = -1;

	CString strSoftId;
	if (pDlg->m_arrTaskMap.Lookup(tk) != NULL)
	{
		strSoftId = pDlg->m_arrTaskMap.Lookup(tk)->m_value;
	}

	for (int i = 0; i < pDlg->m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = pDlg->m_arrData[i];
		if ( (pData != NULL) && (pData->m_strSoftID == strSoftId))
		{
			switch (st)
			{
			case TASK_DONE:
				{
					pDlg->SaveDownloadSoftInfo(pData->m_strFileName);

					pDlg->m_pDTManager->QueryTask(tk, pDlg->DownloadInfoCallBack, (void*)pParam);
					pData->m_bDownloading = TRUE;
					pData->m_bDownLoad = TRUE;
					pData->m_bContinue = FALSE;
					pData->m_bPause = FALSE;
					pData->m_bLinking = FALSE;
					pData->m_dwProgress = 100;
					if ((pData->m_attri&SA_Green) == SA_Green)
					{
						pData->m_bSetup = TRUE;
						//pData->m_bDownloading = FALSE; //已经安装则不显示进度条，显示完整信息
						//pData->m_bInstalling = FALSE;
						//pData->m_bWaitInstall = FALSE;
					}

					//下载任务完成，刷新进度	
					pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);


					pDlg->m_cs.Lock();
					pDlg->m_arrTaskMap.RemoveKey(tk);
					pDlg->m_arrTask.RemoveKey(strSoftId);
					pDlg->m_cs.Unlock();
					pDlg->m_pDTManager->CancelTask(tk);
					tk = NULL;


					if( pDlg->CheckSoftCanInstall(pData,TRUE) )
					{
						pDlg->_InstallSoftExistFile(pData);
					}

				}
				break;
			case TASK_DOWNING:
				pDlg->m_pDTManager->QueryTask(tk, pDlg->DownloadInfoCallBack, (void*)pParam);
				break;
			case TASK_ERROR:
				{
					if (!pData->m_bLinking)
					{
						pData->m_bPause = TRUE;
						pData->m_bContinue = FALSE;
						pDlg->m_pDTManager->PauseTask(tk);
					}


					pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

					if (pParam != NULL)
					{
						delete pParam;
						pParam = NULL;
					}


//#ifdef _DEBUG
					MYDOWNLOADLOG( _T("Download Success, name:%s,ID:%s,URL:%s, ERR:%d  %s\r\n\r\n"), 
									pData->m_strName, 
									pData->m_strSoftID, 
									pData->m_strDownURL,
									nErr, 
									_T("TASK_ERROR"));
//#endif
				}

				break;
			case TASK_ERROR_MD5:
				{
					if (!pData->m_bLinking)
					{
						pData->m_bPause = TRUE;
						pData->m_bContinue = FALSE;
						pDlg->m_pDTManager->PauseTask(tk);
					}
					

					if( tk != NULL )
					{
						IDownload* pDownLoad = (IDownload*)tk;
						nErr = (int)pDownLoad->LastError();
					}

					pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);
//#ifdef _DEBUG
					MYDOWNLOADLOG( _T("Download Success, name:%s,ID:%s,URL:%s, ERR:%d  %s\r\n\r\n"), 
									pData->m_strName, 
									pData->m_strSoftID, 
									pData->m_strDownURL,
									nErr, 
									_T("TASK_ERROR_MD5"));
//#endif
					if (pParam != NULL)
					{
						delete pParam;
						pParam = NULL;
					}

				}
				break;
			}
			break;					
		}
	}
}

CBeikeSafeSoftmgrUIHandler::~CBeikeSafeSoftmgrUIHandler()
{
	Uninit();
}

void CBeikeSafeSoftmgrUIHandler::InitLeftTypeList()
{
	int nCount = 0;
	CTypeListItemData * pListData = NULL;

	m_SoftTypeList.InsertString( 0, BkString::Get(IDS_SOFTMGR_8018) );
	pListData = new CTypeListItemData( 20, 0, COL_TYPE_NAME_NORMAL,BkString::Get(IDS_SOFTMGR_8018) , 50);
	m_SoftTypeList.SetItemDataX( 0, ( DWORD_PTR ) pListData );
	m_SoftTypeList.SetItemHeight( 0, TYPE_ITEM_HEIGHT );	

	nCount = m_pSoftMgr->GetSoftCount(_T(""));
	m_SoftTypeList.InsertString(1, BkString::Get(IDS_SOFTMGR_8019) );
	pListData = new CTypeListItemData( 20, 1, COL_TYPE_NAME_NORMAL, BkString::Get(IDS_SOFTMGR_8019), nCount );
	m_SoftTypeList.SetItemDataX( 1, ( DWORD_PTR ) pListData );
	m_SoftTypeList.SetItemHeight( 1, TYPE_ITEM_HEIGHT );	

	m_SoftTypeList.SetCallBack( this );
	m_SoftTypeList.SetListID(IDC_TYPES_LEFT_SOFTMGR);
	m_SoftTypeList.SetCurSel( 0 );

	CAtlArray<CString> arrType;
	int nTypeCount = m_pSoftMgr->GetCatalog(GetCatlogUseArr, &arrType);
	for(int i=2; i < (int)arrType.GetCount() + 2; i++)
	{
		if ( !m_SoftTypeList.CheckItemExist(arrType[i-2]) )
		{
			m_SoftTypeList.InsertString( i, arrType[i-2] );
			nCount = m_pSoftMgr->GetSoftCount( arrType[i-2]);
			pListData = new CTypeListItemData( 20, i, COL_TYPE_NAME_NORMAL, arrType[i-2], nCount);
			m_SoftTypeList.SetItemDataX( i, ( DWORD_PTR ) pListData );
			m_SoftTypeList.SetItemHeight( i, TYPE_ITEM_HEIGHT );		// 设置高度
		}
	}

	m_SoftTypeList.SetCallBack( this );
	m_SoftTypeList.SetListID(IDC_TYPES_LEFT_SOFTMGR);
	m_SoftTypeList.SetCurSel( 0 );

	m_SoftTypeList.SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
	m_SoftTypeList.Invalidate();
}

HRESULT CBeikeSafeSoftmgrUIHandler::OnInstSoftComplete(HRESULT hr)
{
	return S_OK;
}

HRESULT CBeikeSafeSoftmgrUIHandler::BeforeInstall(const S_KSAI_SOFT_INFO* pstruSoftInfo)
{
	return S_OK;
}

HRESULT CBeikeSafeSoftmgrUIHandler::OnProgress(const S_KSAI_STATUS* pstruStatus)
{
	if (m_necessUIHandler.m_pOnekeyDlg)
	{
		DWORD nCurInstProg = 0;
		if (pstruStatus && pstruStatus->dwTotalProgressCount)
		{
			nCurInstProg = pstruStatus->dwCurProgressCount*100 / pstruStatus->dwTotalProgressCount;
			::PostMessage(m_necessUIHandler.m_pOnekeyDlg->m_hWnd, WM_SOFT_OK_INST_PROGRESS,(WPARAM)nCurInstProg,NULL);
		}
	}

	return S_OK;
}

HRESULT CBeikeSafeSoftmgrUIHandler::AfterInstall(const S_KSAI_INST_RESULT* pstruInstResult)
{
	if (m_necessUIHandler.m_pOnekeyDlg)
	{
		CBkSoftOneKeyDlg::ONE_KEY_INST_RESULT*	pNewST = new CBkSoftOneKeyDlg::ONE_KEY_INST_RESULT;

		pNewST->bDownSuccess	= TRUE;
		pNewST->nDownError		= 0;

		if (pstruInstResult)
			pNewST->instST			= *pstruInstResult;

		::PostMessage(m_necessUIHandler.m_pOnekeyDlg->m_hWnd, WM_SOFT_OK_INST_DONE,(WPARAM)pNewST,NULL);
	}

	return S_OK;
}

void CBeikeSafeSoftmgrUIHandler::InitRightSoftList()
{
	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, 610);

	ShowItemsBySoftType(m_strCurType);

	CRect rcWnd;
	m_list.GetWindowRect(&rcWnd);
	OnListReSize(rcWnd);
}

IDTManager* CBeikeSafeSoftmgrUIHandler::GetDTManagerForOneKey()
{
	if ( m_pDTManagerOneKey != NULL )
		return m_pDTManagerOneKey;
	if (CreateObject == NULL)
		return NULL;

	CreateObject(__uuidof(IDTManager), (void**)&m_pDTManagerOneKey);
	return m_pDTManagerOneKey;
}

BOOL CBeikeSafeSoftmgrUIHandler::InitInterface()
{
	CString	strDll;
	CString strXML;
	CString strRank;
	CString	strNecess;

	if ( SUCCEEDED(GetKSoftDirectory(strDll)) )
	{
		strDll.Append(_T("\\softmgr.dll"));
	}

	if (SUCCEEDED(GetKSoftDataDir(strXML)))
	{
		strRank		= strXML;
		strNecess	= strXML;
		strXML.Append(_T("\\softmgr.dat"));
		strRank.Append(_T("\\rank.dat"));
		strNecess.Append(_T("\\softnecess.dat"));
	}

	if(!PathFileExists(strDll))
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8020), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		return FALSE;
	}

	hInstance = LoadLibrary(strDll);
	if (hInstance == NULL)
	{
		return FALSE;
	}
	CreateObject = (pCreateObject)GetProcAddress(hInstance, "CreateSoftMgrObject");
	CreateObject(__uuidof(ISoftManager), (void**) &m_pSoftMgr);
	CreateObject( __uuidof(ISoftCheckerEx), ( void ** ) &m_pSoftChecker );
	CreateObject(__uuidof(IDTManager), (void**)&m_pDTManager);
	CreateObject(__uuidof(IDownload), (void**)&m_pDownload);
	CreateObject(__uuidof(ksm::ISoftUninstall2), (void**)&m_pUninstall);
	CreateObject(__uuidof(ksm::ISoftInfoQuery), (void**)&m_pInfoQuery);

	if (m_bInitInfoQuery == FALSE && m_pInfoQuery)
	{
		m_bInitInfoQuery = TRUE;

		CString	strDir;
		GetKSafeDirectory(strDir);
		m_pInfoQuery->SetNotify(this);
		m_pInfoQuery->Initialize(strDir);
	}

	CAtlMap<DtStateChangeNotifyCallBack,void*>* calls=new CAtlMap<DtStateChangeNotifyCallBack,void*>;
	calls->SetAt(DownloadCallBack, this);
	m_pDTManager->Init(calls);

	CString kSafePath;
	GetKSafeDirectory(kSafePath);
	m_softUpdatedList.Startup(kSafePath);

	// 加载软件大全库、分级库, 装机必备库
	m_pSoftMgr->LoadSoftDB(strXML); 
	m_pSoftMgr->LoadRank(strRank);
	m_pSoftMgr->LoadNecessDat(strNecess);
	m_pSoftMgr->UpdateCache();
	m_pSoftMgr->GetCategroy(GetInfoUseMap, &m_arrTypeMap);

	m_pDlg->SetTimer(TIMER_RESTART_KSAFE, 5000,NULL);

	return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::InitSlow()
{
	InitSwitch();

	m_bInit = TRUE;

	m_pDlg->SetItemVisible(IDS_LINK_SUBMIT_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, FALSE);
	m_pDlg->SetItemVisible(IDC_TEXT_DETAIL_DOWNLAOD, FALSE);
	m_pDlg->SetItemVisible(IDC_UPDATE_MANAGER_TEXT, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, FALSE);
	m_pDlg->SetItemVisible(IDC_DOWNLOADMGR_SOFTMGR, FALSE);
	m_pDlg->SetFocus();

	m_bInitInterface = InitInterface();

	//因为读取数据比较慢，所以再开一个线程
	m_hInitData = (HANDLE)_beginthreadex(NULL, 0, ThreadInitDataProc, this, NULL, NULL);
	m_pDlg->SetTimer( TIMER_ID_PRE_LOAD_PHONE, 100, NULL );
}

void CBeikeSafeSoftmgrUIHandler::InitSwitch()
{
	LoadUpdateIgnoreList();
	//本地错误页URL
	if ( SUCCEEDED(GetKSoftDirectory(m_strErrorHtml)) )
		m_strErrorHtml.Append(_T("\\html\\error.html"));
	else
		m_strErrorHtml.Empty();


	m_pDlg->SetTimer(TIMER_DOWNLOAD_DETAIL_SOFTMGR, 1000,NULL);
	m_pDlg->SetTimer(TIMRE_DOWNLOAD_CHECK_SOFTMGR, 1000, NULL);

	m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

	m_pDlg->SetItemVisible(IDG_LOAD_TIP_SOFTMGR, FALSE);

	m_pDlg->SetItemVisible( IDC_LNK_DETAIL, FALSE );
	m_pDlg->SetItemVisible( IDC_IMG_GOBACK, FALSE );
	m_pDlg->SetItemVisible( IDC_LNK_GOBACK, FALSE );


	CString strIconPath;
	if ( SUCCEEDED(GetKSoftIconDir(strIconPath)) )
	{
		CString	strTmp;

		strTmp = strIconPath + _T("\\soft.png");
		if ( PathFileExists( strTmp ) )
		{
			m_pImge = Gdiplus::Image::FromFile(strTmp, TRUE);
		}
		else
		{
			m_pImge = NULL;
		}

		// 加载 残留项中使用的图标
		strTmp = strIconPath + _T("\\dir.png");
		m_pImageDir = Gdiplus::Image::FromFile(strTmp, TRUE);

		strTmp = strIconPath + _T("\\file.png");
		m_pImageFile = Gdiplus::Image::FromFile(strTmp, TRUE);

		strTmp = strIconPath + _T("\\reg.png");
		m_pImageReg = Gdiplus::Image::FromFile(strTmp, TRUE);
	}

	SetDownloadDetail();
	InitLinks();

	CString  strCachPath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
	{
		strCachPath.Append(_T("\\ksoft_setuped"));
	}
	BOOL bShow = PathFileExists(strCachPath);
	strCachPath = strCachPath.Left(strCachPath.ReverseFind(_T('\\')));
	strCachPath.Append(_T("\\ksoft_cache_3"));
	bShow &= PathFileExists(strCachPath);
	m_bCachExist = bShow;

	// 	m_pDlg->SetItemVisible(IDG_LOAD_TIP_SOFTMGR, bShow);
	m_pDlg->SetItemVisible(IDC_TYPES_LEFT_SOFTMGR, bShow);
	m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, bShow);
	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, bShow);
	m_pDlg->SetItemVisible(IDC_DLG_PROG1, TRUE /*!bShow*/);

	m_pDlg->SetItemVisible( IDC_UNINSTALL_SOFT_DLG, FALSE );

	strCachPath = strCachPath.Left(strCachPath.ReverseFind(_T('\\')));
	strCachPath.Append(_T("\\ksoft_ucache_5"));
	BOOL bExist = PathFileExists(strCachPath);
	m_pDlg->SetItemVisible( IDC_DLG_PROG2, !bExist );
	if (bExist)
	{
		m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, bExist );
		ShowLoadingTip();
	}

	m_pDlg->SetItemVisible( IDC_UNINSTALL_SOFT_DETAIL, FALSE );

	BOOL bCheck = (BOOL)BKSafeConfig::GetShowFreeSoftSet();
	m_pDlg->SetItemCheck( IDC_CHECKBOX_DAQUAN_FREESOFT, bCheck);
	bCheck = (BOOL)BKSafeConfig::GetShowNoPlugSet();
	m_pDlg->SetItemCheck( IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, bCheck);

}

void CBeikeSafeSoftmgrUIHandler::Init()
{
	m_edit.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, ES_NOHIDESEL | ES_AUTOHSCROLL, IDC_EDIT_SOFTMGR);
	m_edit.SetFont((HFONT)m_font);
	m_edit.SetDefaultString(STR_DEFAULT_SOFTMGR);
	m_edit.SetCallBack(this);
	m_edit.SetOnCharDelayTime();
	m_edit.LimitText(20);

	m_editUninstall.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, ES_NOHIDESEL | ES_AUTOHSCROLL, IDC_SOFT_UNISTALL_EDT_QEURY_EDIT);
	m_editUninstall.SetFont((HFONT)m_font);
	m_editUninstall.SetDefaultString(STR_DEFAULT_SOFTMGR);
	m_editUninstall.SetCallBack(this);
	m_editUninstall.SetOnCharDelayTime();
	m_editUninstall.LimitText(20);

	m_UpdateSoftList.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | LBS_OWNERDRAWVARIABLE, 0, IDC_UPDATE_LIST_SOFTMGR);
	m_UpdateSoftList.SetClickLinkCallback(this);
	m_UpdateSoftList.SetCheckNotifyHwnd(m_pDlg->m_hWnd);

	m_list.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED , 
		0, IDC_LIST_SOFTMGR);
	m_list.CreateTipCtrl();
	m_list.SetClickLinkCallback(this);
	m_list.ShowScrollBar(SB_HORZ, FALSE);
	m_list.ShowWindow(SW_SHOW);

	m_SoftTypeList.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | LBS_OWNERDRAWVARIABLE, 0, IDC_TYPES_LEFT_SOFTMGR);

	m_SoftListTip.Load( IDR_BK_POWERSWEEP_TIP_DLG );
	m_SoftListTip.Create( m_list.m_hWnd );
	m_SoftListTip.ShowWindow( SW_HIDE );

	InitUninstallSoftList();

	m_cs.Init();
	m_csUpdCacheLock.Init();
	m_csUpdCheckLock.Init();

	m_dlgTip.Load( IDR_BK_POWERSWEEP_TIP_DLG );
	m_dlgTip.Create( m_UniExpandSoftList.m_hWnd );
	m_dlgTip.ShowWindow( SW_HIDE );

	m_IconDlg.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, NULL, IDG_ICON_DETAIL_SOFTMGR);
	m_IconUninstallSoft.Create( m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, NULL, IDC_ICO_UNINSTALL_SOFT );
	m_IconUpdateSoft.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, NULL, IDG_UPDATE_ICON_DETAIL);

	m_IEDetail.Create(m_pDlg->GetViewHWND(), IDC_IE_DETAIL_SOFTMGR, FALSE, RGB(0xFB, 0xFC, 0xFD));
	m_IEUninstallDetail.Create(m_pDlg->GetViewHWND(), IDC_IE_UNINSTALL, FALSE, RGB(0xFB, 0xFC, 0xFD));
	m_IEUpdateDetail.Create(m_pDlg->GetViewHWND(), IDC_UPDATE_IE_DETAIL, FALSE, RGB(0xFB, 0xFC, 0xFD));
	m_IEGame.Create(m_pDlg->GetViewHWND(), IDC_CTL_IE_GAME, FALSE, RGB(0xFB, 0xFC, 0xFD));

	m_necessUIHandler.InitCtrl();

#if PHONE_NESS_SWITCH
	m_csPhoneNess.Init();
	m_IEPhoneNess.Create(m_pDlg->GetViewHWND(), IDC_PHONE_NESS_IE, FALSE, RGB(0xFB, 0xFC, 0xFD));
	m_IEPhoneNess.SetExternalDispatch(this);

	m_IESoftRecommend.Create(m_pDlg->GetViewHWND(), IDC_SOFT_RECOMMEND, FALSE, RGB(0xFB, 0xFC, 0xFD));
	m_IESoftRecommend.SetExternalDispatch(this);

	m_pDlg->SetTabCurSel(IDC_TAB_SOFTMGR, 1);
#endif
}

void CBeikeSafeSoftmgrUIHandler::InitLinks()
{
	// 从配置文件中获取 软件卸载页面 中底部的链接的信息
	/* uninsthelp.ini 
	[uninsthelp]
	Ver=1
	Text1=什么样的软件需要卸载？（最多12个汉字）
	Url1=www.baidu.com
	Text2=举报无法彻底卸载的软件（最多12个汉字）
	Url2=www.google.com
	*/

	CString		strDir;
	GetKSoftDirectory( strDir );
	strDir.Append( _T( "\\uninsthelp.ini" ) );

	CString	strText1;
	CString strUrl1;
	CString	strText2;
	CString	strUrl2;

	TCHAR buf[MAX_PATH] = {0};

	GetPrivateProfileString(_T( "uninsthelp" ), _T( "Text1" ), BkString::Get(IDS_SOFTMGR_8021), buf, MAX_PATH-1, strDir );
	strText1 = buf;

	GetPrivateProfileString(_T( "uninsthelp" ), _T( "Url1" ), _T( "" ), buf, MAX_PATH-1, strDir );
	strUrl1 = buf;

	if ( strUrl1.IsEmpty() )
	{
		strText1 = _T( "" );
	}

	GetPrivateProfileString(_T( "uninsthelp" ), _T( "Text2" ), BkString::Get(IDS_SOFTMGR_8022), buf, MAX_PATH-1, strDir );
	strText2 = buf;

	GetPrivateProfileString(_T( "uninsthelp" ), _T( "Url2" ), _T( "" ), buf, MAX_PATH-1, strDir );
	strUrl2 = buf;

	if ( strUrl2.IsEmpty() )
	{
		strText2 = _T( "" );
	}

	// 转换 url 为 utf-8 (ansi)
	char	url1[MAX_PATH] = { 0 };
	char	url2[MAX_PATH] = { 0 };

	::WideCharToMultiByte( CP_ACP, 0, strUrl1, strUrl1.GetLength(), url1, MAX_PATH-1, NULL, NULL );
	::WideCharToMultiByte( CP_ACP, 0, strUrl2, strUrl2.GetLength(), url2, MAX_PATH-1, NULL, NULL );

	m_pDlg->SetItemText( IDC_LINK_WHAT_TO_UNINSTALL, strText1 );
	m_pDlg->SetItemAttribute( IDC_LINK_WHAT_TO_UNINSTALL, "href", url1 );
	m_pDlg->SetItemVisible( IDC_LINK_WHAT_TO_UNINSTALL, FALSE);
	m_pDlg->SetItemText( IDC_LINK_RPT_CANT_UNINSTALL, strText2 );
	m_pDlg->SetItemAttribute( IDC_LINK_RPT_CANT_UNINSTALL, "href", url2 );
	m_pDlg->SetItemVisible( IDC_LINK_RPT_CANT_UNINSTALL, FALSE);
}

int __stdcall UpdateCheckCallBack(void* mp,void* param)
{
	CBeikeSafeSoftmgrUIHandler* _pcThis = ( CBeikeSafeSoftmgrUIHandler* ) param;
	if ( !_pcThis ) return 1;

	CAtlMap<CString,CString>& _soft = *(CAtlMap<CString,CString>*) mp;
	CSoftListItemData* _pData = new CSoftListItemData;
	if ( !_pData ) return 1;

	// 修改软件的状态为升级
	if ( _pData != NULL )
	{
		_pData->m_bUpdate = TRUE;
		_pData->m_strSoftID = _soft[_T( "id" )];
		_pData->m_strName = _soft[L"name"];
		_pData->m_strVersion = _soft[L"cver"];
		_pData->m_strNewVersion = _soft[L"nver"];
		_pData->m_strPublished = _soft[L"lastupdate"];

		CString strIconPath;
		CString strTmpPath;
		if ( SUCCEEDED(_pcThis->GetKSoftIconDir(strIconPath)) )
		{
			strTmpPath = strIconPath;
			strIconPath.Append(_T("\\") + _pData->m_strSoftID + _T(".png"));
			if ( ::PathFileExists(strIconPath) )
			{
				_pData->m_pImage = Gdiplus::Image::FromFile(strIconPath);
				_pData->m_bIcon = TRUE;
			}
		}

		// 添加到安装缓存中
		if ( -1 == _pcThis->m_arrUpdCache.Find( _pData ) )
		{
			_pcThis->m_arrUpdCache.Add( _pData );
		}
	}

	return 0;
}

void CBeikeSafeSoftmgrUIHandler::GetUninstallDataFromMap( CUninstallSoftInfo * pData, void * mp )
{
	CAtlMap<CString,CString>& soft = *(CAtlMap<CString,CString>*) mp;

	pData->key			= soft[_T( "k" )];	
	pData->name			= soft[_T( "name" )];
	pData->m_strName	= pData->name;
	pData->disp_icon	= soft[_T( "ico" )];
	pData->loc			= soft[_T( "loc" )];
	pData->uni_cmd		= soft[_T( "uni" )];
	pData->parent		= soft[_T( "pr" )];


	// 大小  
	CString  strTmpSize;
	strTmpSize = soft[_T( "si" )];
	if ( strTmpSize.IsEmpty() )
	{
		pData->m_nSize = -1;
		pData->m_strSize = _T( "" );
	}
	else
	{
		StrToInt64Ex( strTmpSize, STIF_DEFAULT, &(pData->m_nSize) );
		pData->m_strSize = CalcStrFormatByteSize( pData->m_nSize );
	}

	// 上次使用时间
	CString	strTmpLastUse;
	strTmpLastUse = soft[_T( "lastuse" )];
	if ( !strTmpLastUse.IsEmpty() )
	{
		LONGLONG	nLastUse;
		StrToInt64Ex( strTmpLastUse, STIF_DEFAULT, &nLastUse );
		CTime	tLastUse( nLastUse );
		CTime	tNow = CTime::GetCurrentTime();
		CTimeSpan  tdiff = tNow - tLastUse;

		pData->m_nLastUse = tdiff.GetDays();
		if ( pData->m_nLastUse == 0 )
		{
			pData->m_strFreq = BkString::Get(IDS_SOFTMGR_8028);
		}
		else if ( pData->m_nLastUse > 30 )
		{
			pData->m_strFreq = BkString::Get(IDS_SOFTMGR_8029);
		}
		else
		{
			pData->m_strFreq.Format( BkString::Get(IDS_SOFTMGR_8030), pData->m_nLastUse );
		}

	}
	else
	{
		pData->m_nLastUse = -1;
		pData->m_strFreq = _T( "" );

		if ( pData->loc.IsEmpty() || pData->loc.GetLength() < 4 )
		{
			pData->m_bCalcFreq = FALSE;
		}
		else
		{
			pData->m_bCalcFreq = TRUE;
		}
	}

	pData->m_strTypeName	= m_arrTypeMap[ soft[_T( "typeid" )] ];

	pData->m_strSoftID		= soft[_T( "softid" )];  
	pData->m_strDescription = soft[_T( "brief" )];		 
	pData->m_strURL			= soft[_T( "infourl" )];	 
	pData->m_strIconURL		= soft[_T( "softlogo" )];	

	if ( pData->m_strDescription.IsEmpty() )
	{
		pData->m_strDescription = BkString::Get(IDS_SOFTMGR_8031);
	}

	soft.RemoveAll();

}

void CBeikeSafeSoftmgrUIHandler::LoadUninstallImage( CUninstallSoftInfo * pData )
{
	pData->m_pImage = m_pImge;
	pData->m_bIcon = FALSE;

	CString		strIconPath;
	BOOL		bIcon = TRUE;
	if ( SUCCEEDED( GetKSoftIconDir( strIconPath ) ) )
	{
		strIconPath.Append(_T("\\") + pData->m_strSoftID + _T(".png"));
	}

	if ( PathFileExists( strIconPath ) && ( m_arrDataMap.Lookup( pData->m_strSoftID ) != NULL ) )
	{
		CSoftListItemData *pSoftData = m_arrDataMap.Lookup(pData->m_strSoftID)->m_value;
		if (pSoftData != NULL)
		{
			pData->m_pImage = pSoftData->m_pImage;
			pData->m_bIcon = pSoftData->m_bIcon;
		}
	}
}

int UninstallCheckCallBack2( void * mp, void* param )
{
	return	0;	// continue
}
void CBeikeSafeSoftmgrUIHandler::InitUninstallSoftList()
{
	m_TypeUni.Create(m_pDlg->GetViewHWND(), NULL, NULL, WS_CHILD | LBS_OWNERDRAWVARIABLE, 0, IDC_XIEZAI_TYPES_LIST);

	m_UniExpandSoftList.SetOpCallback(this);
	m_UniExpandSoftList.Create( 
		m_pDlg->GetViewHWND(), NULL, NULL, 
		WS_CLIPCHILDREN | WS_CHILD |WS_VSCROLL | LBS_OWNERDRAWVARIABLE| LBS_HASSTRINGS , 
		0, IDC_UNINSTALL_SOFTLIST, NULL);
	m_UniExpandSoftList.CreateCtrl();

	m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);

	m_comBoBoxUni.Create( m_pDlg->GetViewHWND(), NULL, _T(""), 
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 0, IDS_CLASS_COMBO_UNI_SOFTMGR);
	m_comBoBoxUni.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	m_comBoBoxUni.AddString(L"全部软件");
	m_comBoBoxUni.AddString(L"桌面图标");
	m_comBoBoxUni.AddString(L"开始菜单");
	m_comBoBoxUni.AddString(L"快速启动");
	m_comBoBoxUni.AddString(L"当前进程");
	m_comBoBoxUni.AddString(L"托盘图标");
	m_comBoBoxUni.SetCurSel(0);
}

void CBeikeSafeSoftmgrUIHandler::ClickDaquanType(CString strNewType)
{
	m_bQuery = FALSE;
	m_bViewDetail = FALSE;

	if (!m_strDetailSoftID.IsEmpty())
	{
		_QuerySoftMark(m_strDetailSoftID);
		m_strDetailSoftID.Empty();

		if ( ::GetFocus() != m_edit.m_hWnd )
			m_pDlg->SetFocus();
	}
	m_pDlg->SetItemVisible(IDR_FIND_QUERY_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_TYPE, TRUE);
	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_COUNT, TRUE);

	m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );
	m_pDlg->SetItemAttribute(IDC_DAQUAN_SOFTLIST_HEADER, "pos", "0,0,-0,24");
	m_pDlg->SetItemVisible(IDC_DAQUAN_SOFTLIST_HEADER, TRUE);
	m_pDlg->SetItemAttribute(IDC_LIST_SOFTMGR, "pos", "0,24,-0,-0");
	m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);

	m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);
	m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);
	//		m_pDlg->SetItemVisible(IDC_BATCHDOWN_SOFTMGR, TRUE);

	m_list.ClearAllItemData();
	m_list.SetLoadShow(TRUE);

	if (m_SoftListTip.IsWindow())
	{
		CRect	rc;
		m_list.GetWindowRect(&rc);
		m_SoftListTip.MoveWindow( 
			(rc.Width()-XX_MSG_DLG_WIDTH)/2, 
			(rc.Height()-XX_MSG_DLG_HEIGHT)/2,
			XX_MSG_DLG_WIDTH,
			XX_MSG_DLG_HEIGHT);
	}

	m_SoftListTip.ShowWindow( SW_SHOW );

	ShowItemsBySoftType(strNewType);

	m_strCurType = strNewType;

	m_SoftListTip.ShowWindow( SW_HIDE );

	m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);


	CRect rcWnd;
	m_list.GetWindowRect(&rcWnd);
	OnListReSize(rcWnd);

	m_list.SetLoadShow(FALSE);

}

void CBeikeSafeSoftmgrUIHandler::OnClick( int nListId, CTypeListItemData * pData)
{
	if (!m_bInitData)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8033), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		if(nListId == IDC_TYPES_LEFT_SOFTMGR)
		{
			m_SoftTypeList.SetCurSelTypeName(m_strCurType);
		}

		return;
	}

	if(nListId == IDC_TYPES_LEFT_SOFTMGR)
	{
		CString strNewType;
		strNewType = pData->strTypeName;

		if (m_strCurType == strNewType && !m_bQuery && !m_bViewDetail) //之前显示的不是搜索页面
		{
			return;
		}

		m_bQuery = FALSE;
		m_bViewDetail = FALSE;

		if (!m_strDetailSoftID.IsEmpty())
		{
			_QuerySoftMark(m_strDetailSoftID);
			m_strDetailSoftID.Empty();

			if ( ::GetFocus() != m_edit.m_hWnd )
				m_pDlg->SetFocus();
		}
		m_pDlg->SetItemVisible(IDR_FIND_QUERY_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_TYPE, TRUE);
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_COUNT, TRUE);

		m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );
		m_pDlg->SetItemAttribute(IDC_DAQUAN_SOFTLIST_HEADER, "pos", "0,0,-0,24");
		m_pDlg->SetItemVisible(IDC_DAQUAN_SOFTLIST_HEADER, TRUE);
		m_pDlg->SetItemAttribute(IDC_LIST_SOFTMGR, "pos", "0,24,-0,-0");
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);

		m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
		m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

		m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);
		//		m_pDlg->SetItemVisible(IDC_BATCHDOWN_SOFTMGR, TRUE);

		m_list.ClearAllItemData();
		m_list.SetLoadShow(TRUE);

		if (m_SoftListTip.IsWindow())
		{
			CRect	rc;
			m_list.GetWindowRect(&rc);
			m_SoftListTip.MoveWindow( 
				(rc.Width()-XX_MSG_DLG_WIDTH)/2, 
				(rc.Height()-XX_MSG_DLG_HEIGHT)/2,
				XX_MSG_DLG_WIDTH,
				XX_MSG_DLG_HEIGHT);
		}

		m_SoftListTip.ShowWindow( SW_SHOW );

		ShowItemsBySoftType(strNewType);

		m_strCurType = strNewType;

		m_SoftListTip.ShowWindow( SW_HIDE );

		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);


		CRect rcWnd;
		m_list.GetWindowRect(&rcWnd);
		OnListReSize(rcWnd);

		m_list.SetLoadShow(FALSE);

	}
	else if (nListId == IDC_XIEZAI_TYPES_LIST)
	{
		CString strCurTypeUniOld;
		strCurTypeUniOld = m_strCurTypeUni;
		m_strCurTypeUni = pData->strTypeName;

		OnUninstallDetailBack();	
		BOOL bQuery = m_bQueryUni;
		OnUninstallQueryBack();

		if (bQuery || strCurTypeUniOld.CompareNoCase(m_strCurTypeUni) != 0)
		{
			_ReloadListUni();
			OnBtnSmrUniSortByName(); 
		}
	}

	//	m_pDlg->SetFocus();
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSearchBack()
{
	OnUninstallBack(TRUE);
}

void CBeikeSafeSoftmgrUIHandler::OnUninstallBack(BOOL bClearEditText /*= FALSE*/)
{
	OnUninstallDetailBack();	
	OnUninstallQueryBack(bClearEditText);

	_ReloadListUni();
	OnBtnSmrUniSortByName(); 
}

void CBeikeSafeSoftmgrUIHandler::OnUninstallQueryBack(BOOL bClearEditText /*= FALSE*/)
{
	if (m_bQueryUni)
	{
		m_pDlg->SetItemVisible(IDR_FIND_QUERY_UNI_SOFTMGR,FALSE);
		m_pDlg->SetItemVisible(IDC_SMR_UNI_HEADER_TYPE,TRUE);
		m_pDlg->SetItemVisible(IDC_SMR_UNI_HEADER_COUNT,TRUE);

		m_pDlg->SetItemAttribute(IDC_UNINSTALL_SOFTLIST_HEADER, "pos", "0,0,-0,44");
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST_HEADER,TRUE);
		m_pDlg->SetItemAttribute(IDC_UNINSTALL_SOFTLIST, "pos", "0,44,-0,-0");
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST, TRUE);

		m_bQueryUni = FALSE;
	}

	if (bClearEditText)
		m_editUninstall.SetWindowText(L"");
	m_editUninstall.SetFocus();
}

void CBeikeSafeSoftmgrUIHandler::SetDownloadDetail()
{
	LONGLONG lDownloadSpeed = 0;
	float fDownloadValue = 0.0;
	float fDownloadAll = 0.0;
	int nCount = m_arrData.GetSize();
	int nSize = 0;
	int nDownCount = 0;
	int nPause = 0;
	float fPauseProgress = 0;
	for (int i = 0; i < nCount; i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if(pData != NULL)
		{
			if(pData->m_bDownloading && pData->m_bContinue)
			{
				if (!pData->m_bLinking)
				{
					nSize++;
					fDownloadValue += pData->m_dwProgress;
					lDownloadSpeed += (LONGLONG)pData->m_fSpeed;
					fDownloadAll += pData->m_dwSize;
				}

				for (int j =0; j < m_list.GetItemCount(); j++)
				{
					CSoftListItemData *pItemData = m_list.GetItemDataEx(j);
					if (pItemData != NULL)
					{
						if (pData->m_strSoftID == pItemData->m_strSoftID)
						{
							m_list.RefreshProgress(j);
						}
					}
				}
			}
			else if ( pData->m_bDownloading && pData->m_bPause)
			{
				fPauseProgress += pData->m_dwProgress;
				nPause++;
			}
			else if ( pData->m_bDownLoad)
			{
				nDownCount++;
			}
		}
	}

	CRect rcWnd;
	m_pDlg->GetWindowRect(&rcWnd);

	m_bShowDownloaded = FALSE;

	if(nSize > 0)
	{
		CString str;
		CString strSpeed = CalcStrFormatByteSize(lDownloadSpeed);
		strSpeed.Remove(_T(' '));
		str.Format(BkString::Get(IDS_SOFTMGR_8034), nSize, fDownloadValue/nSize, strSpeed);
		CRect rcCalc;
		CDC dc = GetDC(m_pDlg->m_hWnd);
		dc.DrawText(str, -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		int nWidth = rcCalc.Width() - 15;
		ReleaseDC(m_pDlg->m_hWnd, dc);

		CRect rcText;
		rcText.top = -26;
		rcText.bottom = -1;
		rcText.right = -80;
		rcText.left = rcText.right - nWidth;
		CString strPos;
		strPos.Format(_T("%d,%d,%d,%d"),rcText.left, rcText.top, rcText.right, rcText.bottom);
		m_pDlg->SetItemStringAttribute(IDC_TEXT_DETAIL_DOWNLAOD, "pos", (LPCTSTR)strPos);
		m_pDlg->SetItemText(IDC_TEXT_DETAIL_DOWNLAOD, str);
		m_pDlg->SetItemVisible(IDC_TEXT_DETAIL_DOWNLAOD, TRUE);

		m_pDlg->SetItemStringAttribute(IDC_UPDATE_MANAGER_TEXT, "pos", (LPCTSTR)strPos);
		m_pDlg->SetItemText(IDC_UPDATE_MANAGER_TEXT, str);
		m_pDlg->SetItemVisible(IDC_UPDATE_MANAGER_TEXT, TRUE);

		m_pDlg->SetItemStringAttribute(IDC_SOFT_NECESS_TXT_STATE_DOWN, "pos", (LPCTSTR)strPos);
		m_pDlg->SetItemText(IDC_SOFT_NECESS_TXT_STATE_DOWN, str);
		m_pDlg->SetItemVisible(IDC_SOFT_NECESS_TXT_STATE_DOWN, TRUE);
		m_pDlg->SetItemText( IDC_SOFT_NECESS_LBL_VIEW_DOWN, BkString::Get(IDS_SOFTMGR_8038));
		m_pDlg->SetItemText( IDC_DOWNLOADMGR_SOFTMGR, BkString::Get(IDS_SOFTMGR_8038));
	}
	else if(nSize == 0)
	{
		if ( nPause > 0)//有暂停下载的文件
		{
			CString str;
			str.Format(BkString::Get(IDS_SOFTMGR_8036), nPause, fPauseProgress / nPause);
			CRect rcCalc;
			CDC dc = GetDC(m_pDlg->m_hWnd);
			dc.DrawText(str, -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			int nWidth = rcCalc.Width();
			ReleaseDC(m_pDlg->m_hWnd, dc);

			CRect rcText;
			rcText.top = -26;
			rcText.bottom = -1;
			rcText.right = -80;
			rcText.left = rcText.right - nWidth;
			CString strPos;
			strPos.Format(_T("%d,%d,%d,%d"),rcText.left, rcText.top, rcText.right, rcText.bottom);
			m_pDlg->SetItemStringAttribute(IDC_TEXT_DETAIL_DOWNLAOD, "pos", (LPCTSTR)strPos);

			m_pDlg->SetItemVisible(IDC_TEXT_DETAIL_DOWNLAOD, TRUE);
			m_pDlg->SetItemText(IDC_TEXT_DETAIL_DOWNLAOD, str);

			m_pDlg->SetItemStringAttribute(IDC_UPDATE_MANAGER_TEXT, "pos", (LPCTSTR)strPos);
			m_pDlg->SetItemVisible(IDC_UPDATE_MANAGER_TEXT, TRUE);
			m_pDlg->SetItemText(IDC_UPDATE_MANAGER_TEXT, str);

			m_pDlg->SetItemText( IDC_DOWNLOADMGR_SOFTMGR, BkString::Get(IDS_SOFTMGR_8038));
			m_pDlg->SetItemStringAttribute(IDC_SOFT_NECESS_TXT_STATE_DOWN, "pos", (LPCTSTR)strPos);
			m_pDlg->SetItemText(IDC_SOFT_NECESS_TXT_STATE_DOWN, str);
			m_pDlg->SetItemVisible(IDC_SOFT_NECESS_TXT_STATE_DOWN, TRUE);
			m_pDlg->SetItemText( IDC_SOFT_NECESS_LBL_VIEW_DOWN, BkString::Get(IDS_SOFTMGR_8038));
		}
		else if (nDownCount != 0) //文件夹下面有文件
		{
			CString str;
			str.Format(BkString::Get(IDS_SOFTMGR_8037), nDownCount);
			CRect rcCalc;
			CDC dc = GetDC(m_pDlg->m_hWnd);
			dc.DrawText(str, -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			int nWidth = rcCalc.Width() + 10;
			ReleaseDC(m_pDlg->m_hWnd, dc);

			CRect rcText;
			rcText.top = -26;
			rcText.bottom = -1;
			rcText.right = -80;
			rcText.left = rcText.right - nWidth;
			CString strPos;
			strPos.Format(_T("%d,%d,%d,%d"),rcText.left, rcText.top, rcText.right, rcText.bottom);
			m_pDlg->SetItemStringAttribute(IDC_TEXT_DETAIL_DOWNLAOD, "pos", (LPCTSTR)strPos);

			m_pDlg->SetItemVisible(IDC_TEXT_DETAIL_DOWNLAOD, TRUE);
			m_pDlg->SetItemText(IDC_TEXT_DETAIL_DOWNLAOD, str);

			m_pDlg->SetItemStringAttribute(IDC_UPDATE_MANAGER_TEXT, "pos", (LPCTSTR)strPos);
			m_pDlg->SetItemVisible(IDC_UPDATE_MANAGER_TEXT, TRUE);
			m_pDlg->SetItemText(IDC_UPDATE_MANAGER_TEXT, str);

			m_pDlg->SetItemText( IDC_DOWNLOADMGR_SOFTMGR, BkString::Get(IDS_SOFTMGR_8038));

			m_pDlg->SetItemStringAttribute(IDC_SOFT_NECESS_TXT_STATE_DOWN, "pos", (LPCTSTR)strPos);
			m_pDlg->SetItemText(IDC_SOFT_NECESS_TXT_STATE_DOWN, str);
			m_pDlg->SetItemVisible(IDC_SOFT_NECESS_TXT_STATE_DOWN, TRUE);
			m_pDlg->SetItemText( IDC_SOFT_NECESS_LBL_VIEW_DOWN, BkString::Get(IDS_SOFTMGR_8038));
			m_bShowDownloaded = TRUE;
		}
		else
		{
			m_pDlg->SetItemVisible(IDC_TEXT_DETAIL_DOWNLAOD, FALSE);
			m_pDlg->SetItemVisible(IDC_UPDATE_MANAGER_TEXT, FALSE);
			m_pDlg->SetItemText(IDC_DOWNLOADMGR_SOFTMGR, BkString::Get(IDS_SOFTMGR_8038));
			m_pDlg->SetItemVisible(IDC_SOFT_NECESS_TXT_STATE_DOWN, FALSE);
			m_pDlg->SetItemText( IDC_SOFT_NECESS_LBL_VIEW_DOWN, BkString::Get(IDS_SOFTMGR_8038));
		}

	}
}

void CBeikeSafeSoftmgrUIHandler::OnViewDetail(DWORD dwIndex)
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	if (pData == NULL)
	{
		return;
	}

	m_bViewDetail = TRUE;
	m_strDetailSoftID = pData->m_strSoftID;
	m_nDown = dwIndex;

	//	m_pDlg->SetItemVisible(IDC_BATCHDOWN_SOFTMGR, FALSE);

	//软件信息
	CString strVersion;
	if (pData->m_strNewVersion.IsEmpty() == FALSE && 
		pData->m_strNewVersion.CompareNoCase(L"0.0.0.0") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"1.0.0.0") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"0.0.0.1") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"1.0.0.1") != 0)
	{
		strVersion = pData->m_strNewVersion;
	}
	else if (pData->m_strVersion.IsEmpty() == FALSE && 
		pData->m_strVersion.CompareNoCase(L"0.0.0.0") != 0 &&
		pData->m_strVersion.CompareNoCase(L"1.0.0.0") != 0 &&
		pData->m_strVersion.CompareNoCase(L"0.0.0.1") != 0 &&
		pData->m_strVersion.CompareNoCase(L"1.0.0.1") != 0)
	{
		strVersion = pData->m_strVersion;
	}

	if (strVersion.IsEmpty())
	{
		m_pDlg->FormatRichText(
			IDS_INFO_SOFTMGR, 
			BkString::Get(IDS_SOFTMGR_8128), 
			pData->m_strName, pData->m_strSize);
	}
	else
	{
		m_pDlg->FormatRichText(
			IDS_INFO_SOFTMGR, 
			BkString::Get(IDS_INFO_FORMAT_SOFTMGR), 
			pData->m_strName, pData->m_strSize,strVersion);
	}

	//收费信息
	if (pData->m_bCharge)
	{
		m_pDlg->SetItemAttribute(IDC_ICON_FREE_DETAIL_SOFTMGR, "sub", "2");
		m_pDlg->SetItemText(IDS_FREE_DETAIL_SOFTMGR, BkString::Get(IDS_SOFTMGR_8039));
	}
	else
	{
		m_pDlg->SetItemAttribute(IDC_ICON_FREE_DETAIL_SOFTMGR, "sub", "1");
		m_pDlg->SetItemText(IDS_FREE_DETAIL_SOFTMGR, BkString::Get(IDS_SOFTMGR_8040));
	}
	//插件信息
	if (pData->m_bPlug)
	{
		m_pDlg->SetItemAttribute(IDC_ICON_PLUG_DETAIL_SOFTMGR, "sub", "2");
		m_pDlg->SetItemText(IDS_PLUG_DETAIL_SOFTMGR, BkString::Get(IDS_SOFTMGR_8041));
	}
	else
	{
		m_pDlg->SetItemAttribute(IDC_ICON_PLUG_DETAIL_SOFTMGR, "sub", "1");
		m_pDlg->SetItemText(IDS_PLUG_DETAIL_SOFTMGR, BkString::Get(IDS_SOFTMGR_8042));
	}


	//系统信息
	CString strCurOS = GetCurrentSystemVersion();
	int nPos = pData->m_strOSName.Find(strCurOS);
	if (nPos >= 0 && nPos < pData->m_strOSName.GetLength())
	{
		strVersion = BkString::Get(IDS_SOFTMGR_8043);
		m_pDlg->SetItemAttribute(IDC_ICON_FITAYATEM_DETAIL_SOFTMGR, "sub", "1");
	}
	else
	{
		strVersion = BkString::Get(IDS_SOFTMGR_8044);
		m_pDlg->SetItemAttribute(IDC_ICON_FITAYATEM_DETAIL_SOFTMGR, "sub", "2");
	}
	strVersion.Append(strCurOS + _T(")"));
	m_pDlg->SetItemText(IDS_FITAYATEM_DETAIL_SOFTMGR, strVersion);

	//详情页右侧按钮显示
	if (pData->m_bSetup)
	{
		if (pData->m_bUpdate)//需要升级
			m_pDlg->SetItemAttribute(IDC_DOWN_BTN_DETAIL_SOFTMGR, "skin", "btndetailupdate");
		else
		{
			if ((pData->m_attri&SA_Green) == SA_Green)
				m_pDlg->SetItemAttribute(IDC_DOWN_BTN_DETAIL_SOFTMGR, "skin", "btndetaildownloaded");
			else
				m_pDlg->SetItemAttribute(IDC_DOWN_BTN_DETAIL_SOFTMGR, "skin", "btndetailsetup");
		}
	}
	else if (pData->m_bDownLoad)
	{
		m_pDlg->SetItemAttribute(IDC_DOWN_BTN_DETAIL_SOFTMGR, "skin", "btndetaildown");
	}
	else
	{
		m_pDlg->SetItemAttribute(IDC_DOWN_BTN_DETAIL_SOFTMGR, "skin", "btndetailundown");
	}

	//详情IE
	if(pData->GetInfoUrl() == _T(""))	//没有详情信息
	{
		m_IEDetail.Show2(m_strErrorHtml,NULL);
	}
	else	//有详情页URL
	{
		m_IEDetail.Show2(pData->GetInfoUrl(), m_strErrorHtml);
	}


	m_IconDlg.SetDataRes(pData->m_pImage);

	m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, FALSE);

	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, FALSE);
}

void CBeikeSafeSoftmgrUIHandler::OnViewNew(DWORD dwIndex)
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	if (pData == NULL)
		return;

	OnViewNewInfo(pData);
}

void CBeikeSafeSoftmgrUIHandler::OnBack()
{
	//	m_IEDetail.Show(_T("about:blank"));
	//if (m_bQuery)
	//	m_bQuery = FALSE;

	m_bViewDetail = FALSE;

	if (!m_strDetailSoftID.IsEmpty())
	{
		_QuerySoftMark(m_strDetailSoftID);
		m_strDetailSoftID.Empty();
		if ( ::GetFocus() != m_edit.m_hWnd )
			m_pDlg->SetFocus();
	}

	//	m_pDlg->SetItemVisible(IDC_BATCHDOWN_SOFTMGR, TRUE);
	m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

	m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);
	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);
}

void CBeikeSafeSoftmgrUIHandler::OnUninstallDetailBack()
{
	if (m_bViewUninstall)
	{
		m_IEUninstallDetail.Show2( _T( "about:blank" ), NULL );

		m_pDlg->SetItemVisible( IDC_UNINSTALL_SOFT_DETAIL, FALSE );

		m_pDlg->SetItemVisible( IDC_LNK_DETAIL, FALSE );
		m_pDlg->SetItemVisible( IDC_IMG_GOBACK, FALSE );
		m_pDlg->SetItemVisible( IDC_LNK_GOBACK, FALSE );

		m_pDlg->SetItemVisible( IDC_UNINSTALL_SOFT_DLG, TRUE );
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST, TRUE);
		m_bViewUninstall = FALSE;
	}

	if ( ::GetFocus() != m_editUninstall.m_hWnd )
		m_pDlg->SetFocus();
}

void CBeikeSafeSoftmgrUIHandler::OnUninstallOpenDir()
{
	if (m_pSelSoftUniData)
	{
		CString strPath;
		strPath = m_pSelSoftUniData->strMainPath;

		if ( strPath.GetLength() > 0 )
		{
			if( PathIsDirectory( strPath ) )
				ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
			else
			{
				CString strMessage;
				strMessage.Format(BkString::Get(IDS_SOFTMGR_8045), strPath);

				CBkSafeMsgBox2 msgbox;
				msgbox.AddButton( TEXT("确定"), IDOK);
				msgbox.ShowMutlLineMsg( strMessage, BkString::Get(IDS_SOFTMGR_8046), MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );
			}
		}
	}
}

BOOL CBeikeSafeSoftmgrUIHandler::OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam )
{
	CSoftListItemData *pData = (CSoftListItemData*)pDownload->GetUserData();
	if (state == ProcessState_Finished)
	{
		CString strIconPath;
		CString strTmpPath;
		if ( SUCCEEDED(GetKSoftIconDir(strIconPath)) )
		{
			strTmpPath = strIconPath;
			strIconPath.Append(_T("\\") + pData->m_strSoftID + _T(".png"));
		}
		if (PathFileExists(strIconPath))
		{
			if ( pData->m_bChangeIcon && pData->m_pImage != NULL)
			{
				delete pData->m_pImage;
				pData->m_pImage = NULL;
				pData->m_bChangeIcon = FALSE;
			}
			pData->m_pImage = Gdiplus::Image::FromFile(strIconPath,TRUE);
			pData->m_bIcon = TRUE;

			if ( pData->m_nCount <= 3 && !pData->m_strCrc.IsEmpty() )
			{
				FILE *pFile = NULL;
				char szFilePath[MAX_PATH] = {0};
				::WideCharToMultiByte( CP_ACP, 0, strIconPath, strIconPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
				pFile = fopen(szFilePath, "rb");
				if ( pFile )
				{
					DWORD dwCrc = 0;
					dwCrc = CalcFileCrc( pFile );
					fclose( pFile );
					CString strTmpCrc;
					strTmpCrc.Format(_T("%08X"), dwCrc);
					if ( pData->m_strCrc != strTmpCrc )
					{

						if ( pData->m_pImage )
						{
							delete pData->m_pImage;
							pData->m_pImage = NULL;
						}

						if ( pData->m_nCount == 3)
						{
							pData->m_pImage = m_pImge;
							pData->m_bIcon = TRUE;
						}
						else
						{
							strTmpPath.Append(_T("\\") + pData->m_strSoftID + _T("_1.png"));
							if (PathFileExists( strTmpPath ))
							{
								WCHAR SrcFolder[MAX_PATH];
								WCHAR DestFolder[MAX_PATH];
								wcscpy_s(SrcFolder, strIconPath);
								wcscpy_s(DestFolder,strTmpPath);
								SrcFolder[wcslen(SrcFolder) + 1] = 0;
								DestFolder[wcslen(DestFolder) + 1] = 0;
								SHFILEOPSTRUCT op = {0};
								op.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_NOERRORUI;
								op.pFrom = DestFolder;
								op.wFunc = FO_DELETE;
								SHFileOperation( &op );
							}

							MoveFile( strIconPath, strTmpPath);
							pData->m_pImage = Gdiplus::Image::FromFile( strTmpPath, FALSE );
							pData->m_bIcon = FALSE;
							pData->m_bChangeIcon = TRUE;
							m_arrDownIconData.AddTail( pData );
						}
					}
				}
				else
				{
					fclose( pFile );
				}
			}


			//刷新大全列表
			_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);
			for (int j = 0; j < m_UniExpandSoftList.GetAllCount(); j++)
			{
				SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniData[j];
				if (pInfo->strSoftID == pData->m_strSoftID)
				{
					pInfo->bDefaultImg = FALSE;
					pInfo->pImg = pData->m_pImage;
					m_UniExpandSoftList.InvalidateRect(NULL);
					break;
				}
			}

			//刷新卸载列表

			//刷新升级列表
		}
	}
	if (state == ProcessState_Failed)
	{
		return FALSE;
	}

	return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::OnDownLoadIcon(CString strID)
{
	CSoftListItemData *pData = NULL;
	if (m_arrDataMap.Lookup(strID) != NULL)
	{
		pData = m_arrDataMap.Lookup(strID)->m_value;
	}
	if(pData != NULL)
	{
		CString strIconPath;
		CString strTmpPath;
		if ( SUCCEEDED(GetKSoftIconDir(strIconPath)) )
		{
			strTmpPath = strIconPath;
			strIconPath.Append(_T("\\") + pData->m_strSoftID + _T(".png"));
		}
		if (PathFileExists(strIconPath))
		{
			if ( pData->m_bChangeIcon && pData->m_pImage)
			{
				delete pData->m_pImage;
				pData->m_pImage = NULL;
				pData->m_bChangeIcon = FALSE;
			}
			pData->m_pImage = Gdiplus::Image::FromFile(strIconPath,TRUE);
			pData->m_bIcon = TRUE;
			pData->m_bChangeIcon = TRUE;

			if( !pData->m_strCrc.IsEmpty() )
			{
				FILE *pFile = NULL;
				char szIconPath[MAX_PATH] = {0};
				::WideCharToMultiByte( CP_ACP, 0, strIconPath, strIconPath.GetLength(), szIconPath, MAX_PATH-1, NULL, NULL );
				pFile = fopen( szIconPath, "rb");
				if ( pFile )
				{
					DWORD dwCrc = 0;
					dwCrc = CalcFileCrc( pFile );
					fclose( pFile );
					CString strTmpCrc;
					strTmpCrc.Format(_T("%08X"), dwCrc);
					if ( pData->m_strCrc == strTmpCrc)
					{
						//刷新大全列表
						_RefershItemBySoftIDMainThread(pData->m_strSoftID);
						for (int j = 0; j < m_UniExpandSoftList.GetAllCount(); j++)
						{
							SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniData[j];
							if (pInfo->strSoftID == pData->m_strSoftID)
							{
								pInfo->bDefaultImg = FALSE;
								pInfo->pImg = pData->m_pImage;
								m_UniExpandSoftList.InvalidateRect(NULL);
								break;
							}
						}

						return;
					}
					else
					{
						strTmpPath.Append(_T("\\") + pData->m_strSoftID + _T("_1.png"));
						if( pData->m_pImage && pData->m_bChangeIcon)
						{
							delete pData->m_pImage;
							pData->m_pImage = NULL;
							pData->m_bChangeIcon = FALSE;
						}

						int nReturn;
						if (PathFileExists( strTmpPath ))
						{
							WCHAR SrcFolder[MAX_PATH];
							WCHAR DestFolder[MAX_PATH];
							wcscpy_s(SrcFolder, strIconPath);
							wcscpy_s(DestFolder,strTmpPath);
							SrcFolder[wcslen(SrcFolder) + 1] = 0;
							DestFolder[wcslen(DestFolder) + 1] = 0;
							SHFILEOPSTRUCT op = {0};
							op.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_NOERRORUI;
							op.pFrom = DestFolder;
							op.wFunc = FO_DELETE;
							nReturn = SHFileOperation( &op );
						}

						BOOL bRtn = MoveFile( strIconPath, strTmpPath);
						pData->m_pImage = Gdiplus::Image::FromFile( strTmpPath, FALSE );
						pData->m_bIcon = FALSE;
						pData->m_bChangeIcon = TRUE;
					}
				}
				else
				{
					fclose( pFile );
				}
			}
		}

		m_lockIconDown.Lock();
		if (m_arrDownIconData.Find(pData) == NULL) //如果已经在下载队列中就不再下载
		{
			if ( pData->m_nCount > 3) 
			{
				if ( pData->m_pImage != NULL && pData->m_bChangeIcon)
				{
					delete pData->m_pImage;
					pData->m_pImage = NULL;
					pData->m_bChangeIcon = FALSE;
				}

				pData->m_pImage = m_pImge;
				pData->m_bIcon = TRUE;

				m_lockIconDown.Release();
				return;
			}

			if(m_hDownIcon == (HANDLE)-1)
			{
				m_arrDownIconData.RemoveAll();
				m_arrDownIconData.AddTail(pData);
				m_hDownIcon = (HANDLE)_beginthreadex(NULL, 0, ThreadIconProc, this, 0, NULL);
			}
			else
			{
				m_arrDownIconData.AddTail(pData);
			}
		}
		m_lockIconDown.Release();
	}
}

BOOL CBeikeSafeSoftmgrUIHandler::SelDir(CString& strPath)
{
	if( SelectFolderEx(strPath, m_pDlg->m_hWnd) )
	{
		// 检查路径是否过长
		if ( strPath.GetLength() > 200 )
		{
			CBkSafeMsgBox::Show( BkString::Get(IDS_SOFTMGR_8078), NULL, MB_OK | MB_ICONWARNING);
		}
		else
		{
			if (PathFileExists(strPath))
			{
				BKSafeConfig::SetStortDir(strPath);
				return TRUE;
			}
		}	
	}
	return FALSE;
}

void CBeikeSafeSoftmgrUIHandler::_downLoadSoft( CSoftListItemData* pData, SoftActionFrom nFrom, BOOL bUpdate )
{
	if (pData != NULL)
	{
		if (pData->m_bDownloading && !pData->m_bDownLoad)
			return;

		if(pData->m_bInstalling)
			return;

		if (pData->m_strFileName.GetLength() == 0)
			return;

		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		::SHCreateDirectory(NULL, strStore);

		if (!PathFileExists(strStore))
		{
			CBkSafeMsgBox2	msgbox;
			msgbox.AddButton(L"确定", IDOK);
			msgbox.AddButton(L"取消", IDCANCEL);
			UINT nRet = msgbox.ShowMutlLineMsg(_T("检测到安装包存储目录不存在，请重新选择，\r\n取消则恢复默认目录。"), 
				NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
			if(nRet==IDOK)
			{
				BOOL bRetCode = FALSE;
				bRetCode = SelDir(strStore);
				if (!bRetCode)
					return;
			}
			else if (nRet == IDCANCEL)
			{
				WCHAR szSystemPath[MAX_PATH] = {0};
				GetSystemDirectory(szSystemPath, MAX_PATH-1);

				CString strTemp;
				strTemp.Format(_T("%s"), szSystemPath);
				strTemp = strTemp.Left(3);
				strTemp.Append( _T( "ksDownloads" ) );
				BKSafeConfig::SetStortDir(strTemp);

				strStore = strTemp;
			}
		}

		if(strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
			strStore.Append(_T("\\")+ pData->m_strFileName);
		else
			strStore.Append(pData->m_strFileName);

		if( !PathFileExists(strStore))
		{
			BKSafeConfig::GetStoreDir(strStore);
			if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
			{
				strStore.Append(_T("\\")+ pData->m_strFileName);
			}
			else
			{
				strStore.Append(pData->m_strFileName);
			}
		}

		if (PathFileExists(strStore))
		{
			CFileTime timeright;
			WIN32_FILE_ATTRIBUTE_DATA	data;
			GetFileAttributesEx( strStore, GetFileExInfoStandard, &data );
			timeright.dwHighDateTime = data.ftCreationTime.dwHighDateTime;
			timeright.dwLowDateTime  = data.ftCreationTime.dwLowDateTime;
			pData->m_llDownloadTime = ( LONGLONG )(timeright.GetTime());

			pData->m_bDownloading = TRUE;
			pData->m_bDownLoad = TRUE;
			pData->m_dwProgress = 100;
			if ((pData->m_attri&SA_Green) == SA_Green)
			{
				pData->m_bSetup = TRUE;
			}

			if ( CheckSoftCanInstall(pData,FALSE) )
				_InstallSoftExistFile(pData);

			_RefershItemBySoftIDMainThread(pData->m_strSoftID);

			return;
		}
		else
		{
			strStore.Append(L"_kt");
			if (PathFileExists(strStore))
			{
				CFileTime timeright;
				WIN32_FILE_ATTRIBUTE_DATA	data;
				GetFileAttributesEx( strStore, GetFileExInfoStandard, &data );
				timeright.dwHighDateTime = data.ftCreationTime.dwHighDateTime;
				timeright.dwLowDateTime  = data.ftCreationTime.dwLowDateTime;
				pData->m_llDownloadTime = ( LONGLONG )(timeright.GetTime());
			}
			else
			{
				CFileTime timeleft;
				SYSTEMTIME sysTimeLeft;
				GetLocalTime(&sysTimeLeft);
				SystemTimeToFileTime( &sysTimeLeft, &timeleft );
				pData->m_llDownloadTime = ( LONGLONG )(timeleft.GetTime());
			}
		}

		int		nRet;
		nRet = BKSafeConfig::GetHintPlugin();
		if(nRet == 1)
		{
			if (pData->m_bCharge || pData->m_bPlug)
			{
				CBkPlugInfoDlg	dlg(pData);
				UINT_PTR uRet = dlg.DoModal(GetActiveWindow());
				if(uRet!=IDOK)
				{
					pData->m_bDownloading = FALSE;
					_RefershItemBySoftIDMainThread(pData->m_strSoftID);
					return;
				}
			}
		}

		if(pData->m_strDownURL != _T(""))
		{
			if (m_arrDownload.Find(pData) != NULL)
			{
				return;
			}
			m_cs.Lock();
			m_arrDownload.AddTail(pData);
			m_cs.Unlock();
			//			if (pData->m_bBatchDown)
			{
				pData->m_bWaitDownload = TRUE;
				pData->m_bCancel = FALSE;
			}
			_RefershItemBySoftIDMainThread(pData->m_strSoftID);

			if(m_hDownload == (HANDLE)-1)
			{
				m_hDownload = (HANDLE)_beginthreadex(NULL, 0, ThreadDownloadProc, this, 0, NULL);
			}

		}
	}
}

void CBeikeSafeSoftmgrUIHandler::OnDownLoad(DWORD dwIndex)
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	if( pData )
	{
		if( !pData->m_bSetup || !pData->m_bUpdate )
			AddToNewInst( pData->m_strSoftID );

		_downLoadSoft(pData, ACT_SOFT_FROM_DAQUAN, pData->m_bSetup && pData->m_bUpdate );
	}
}

void CBeikeSafeSoftmgrUIHandler::_CancelDownLoad( CSoftListItemData* pData, SoftActionFrom nFrom )
{
	if ( m_arrTask.Lookup( pData->m_strSoftID ) != NULL )
	{
		threadParam *param = new threadParam;
		param->pData = pData;
		param->pDlg = this;
		param->flag = 2;

		//重新开一个线程处理，防止死锁
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, param, 0, NULL);
		if (hThread)
		{
			CloseHandle(hThread);
		}

	}
	else
	{
		pData->m_bCancel = TRUE;
		pData->m_bDownLoad = FALSE;
		pData->m_bDownloading = FALSE;
		pData->m_bInstalling = FALSE;
		pData->m_bLinking = FALSE;

		//刷新
		_RefershItemBySoftIDMainThread(pData->m_strSoftID);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnCancel(DWORD dwIndex)
//删除临时文件 ——_kt、_kti
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	_CancelDownLoad(pData);
}

void CBeikeSafeSoftmgrUIHandler::_ContinueDownLoad( CSoftListItemData* pData, SoftActionFrom nFrom )
{

	if (m_arrTask.Lookup( pData->m_strSoftID ) == NULL)
	{
		m_cs.Lock();
		m_arrDownload.AddTail(pData);
		m_cs.Unlock();
		{
			pData->m_bWaitDownload = TRUE;
		}

		_RefershItemBySoftIDMainThread(pData->m_strSoftID);

		if(m_hDownload == (HANDLE)-1)
		{
			m_hDownload = (HANDLE)_beginthreadex(NULL, 0, ThreadDownloadProc, this, 0, NULL);
		}

	}
	else	
	{
		threadParam *param = new threadParam;
		param->pData = pData;
		param->pDlg = this;
		param->flag = 0;

		//重新开一个线程处理，防止死锁
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, param, 0, NULL);
		if (hThread)
		{
			CloseHandle(hThread);
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::OnContinue(DWORD dwIndex)
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	_ContinueDownLoad(pData);
}
void CBeikeSafeSoftmgrUIHandler::_PauseDownLoad( CSoftListItemData* pData , SoftActionFrom nFrom)
{
	threadParam *param = new threadParam;
	param->pData = pData;
	param->pDlg = this;
	param->flag = 1;

	//重新开一个线程处理，防止死锁
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, param, 0, NULL);
	if (hThread)
	{
		CloseHandle(hThread);
	}
}
void CBeikeSafeSoftmgrUIHandler::OnPause(DWORD dwIndex)
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	_PauseDownLoad(pData);
}

void CBeikeSafeSoftmgrUIHandler::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_DOWNLOAD_DETAIL_SOFTMGR)
	{
		SetDownloadDetail();
	}
	else if (nIDEvent == TIMRE_DOWNLOAD_CHECK_SOFTMGR)
	{
		CheckDownloadState();
	}
	else if (nIDEvent == TIMER_LOADTIP_SOFTMGR)
	{
		ShowLoadInstallInfoTip();
	}
	else if (nIDEvent == TIMER_LOADINFO_SOFTMGR)
	{
		ShowPicSoftMgr();
	}
	else if ( nIDEvent == TIMER_LOADINFO_BIGBTN )
	{
		RollTheTip();
	}
	else if( nIDEvent == TIMER_ID_ALLSOFE_KEYWORD )
	{
		CString	strSearch;
		if( m_edit.IsWindow() )
			m_edit.GetWindowText(strSearch);


		m_pDlg->KillTimer( TIMER_ID_ALLSOFE_KEYWORD );
	}
	else if ( nIDEvent == TIMER_ID_UNINST_KEYWORD )
	{
		CString	strSearch;
		if( m_editUninstall.IsWindow() )
			m_editUninstall.GetWindowText(strSearch);


		m_pDlg->KillTimer( TIMER_ID_UNINST_KEYWORD );
	}
	else if( nIDEvent == TIMER_ID_PRE_LOAD_PHONE )
	{
		if(!m_phoneLoaded)
		{
			m_phoneLoaded = TRUE;
			CString strUrl;
			if( IsTestSoftPhone( strUrl ) )
				m_IEPhoneNess.Show2( strUrl, m_strErrorHtml );
			else
				m_IEPhoneNess.Show2(L"http://baike.ijinshan.com/ksafew/site/sj/index.html", m_strErrorHtml);
		}

		m_pDlg->KillTimer( TIMER_ID_PRE_LOAD_PHONE );
	}
	else if (nIDEvent == TIMER_RESTART_KSAFE)
	{
		if (m_pSoftMgr->IsLibUpdate())
		{
			m_pDlg->SetItemVisible(IDC_SOFT_NECESS_LEFT_VER_TIP, TRUE);
			m_pDlg->SetItemAttribute(IDC_SOFT_NECESS_LEFT_VIEW, "pos", "118,28,-0,-0");

			m_pDlg->SetItemVisible(IDC_SOFT_DAQUAN_LEFT_VER_TIP, TRUE);
			m_pDlg->SetItemAttribute(IDC_SOFT_DAQUAN_LEFT_VIEW, "pos", "118,28,-0,-31");

			m_pDlg->SetItemVisible(IDC_SOFT_UPDATE_LEFT_VER_TIP, TRUE);
			m_pDlg->SetItemAttribute(IDC_SOFT_UPDATE_LEFT_VIEW, "pos", "0,59,-0,-31");

			m_pDlg->KillTimer( TIMER_RESTART_KSAFE );
		}
	}
	else
		SetMsgHandled(FALSE);
}

void CBeikeSafeSoftmgrUIHandler::RollTheTip()
{
	static int nPos = 0;

	m_pDlg->SetItemIntAttribute(IDC_IMG_LOADING_PIC, "sub", (nPos)%8);
	nPos++;
}

void CBeikeSafeSoftmgrUIHandler::OnSetting()
{
	//m_SoftMgrSettingCaller.DoModal();
	_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_SOFTMGR));//hub

}

void CBeikeSafeSoftmgrUIHandler::OnRefresh()
{
	if (!m_bInitData)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8033), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		return;
	}

	if (m_bLoadFail)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8020), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		return;
	}

	if (m_nTab == TAB_INDEX_TUIJIAN)
	{
		m_IESoftRecommend.Show2(L"http://baike.ijinshan.com/ksafew/site/hot/index.html", m_strErrorHtml);
	}
	else if (m_nTab == TAB_INDEX_DAQUAN)
	{
		OnBack();

		m_pDlg->SetItemVisible(IDR_FIND_QUERY_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_TYPE, TRUE);
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_COUNT, TRUE);

		m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );
		m_pDlg->SetItemAttribute(IDC_DAQUAN_SOFTLIST_HEADER, "pos", "0,0,-0,24");
		m_pDlg->SetItemVisible(IDC_DAQUAN_SOFTLIST_HEADER, TRUE);
		m_pDlg->SetItemAttribute(IDC_LIST_SOFTMGR, "pos", "0,24,-0,-0");
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);

		m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
		m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

		m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);

		m_list.ClearAllItemData();

		ShowItemsBySoftType(m_strCurType);

		CRect rcWnd;
		m_list.GetWindowRect(&rcWnd);
		OnListReSize(rcWnd);

		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);
	}
	else if (m_nTab == TAB_INDEX_UNINSTALL)
	{
		OnUninstallBack();

		ShowUninstallInfoBar( TRUE );

		m_pDlg->SetItemVisible(IDR_FIND_QUERY_UNI_SOFTMGR,FALSE);
		m_pDlg->SetItemVisible(IDC_SMR_UNI_HEADER_TYPE,TRUE);
		m_pDlg->SetItemVisible(IDC_SMR_UNI_HEADER_COUNT,TRUE);

		m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_UNI_SOFTMGR, FALSE);
		m_pDlg->SetItemAttribute(IDC_UNINSTALL_SOFTLIST_HEADER, "pos", "0,0,-0,44");
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST_HEADER, TRUE);
		m_pDlg->SetItemAttribute(IDC_UNINSTALL_SOFTLIST, "pos", "0,44,-0,-0");
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFT_DETAIL, FALSE);
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFT_DLG, TRUE);
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST, TRUE);

		if (m_pUninstall)
		{
			m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
			ShowLoadingTip();

			m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

			m_UniExpandSoftList.ClearAll();
			m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, FALSE );

			switch(m_nUniType)
			{
			case UNI_TYPE_ALL:
				{
					m_bUniRefresh = TRUE;
					m_pUninstall->DataRefresh(TRUE);
				}
				break;

			case UNI_TYPE_START:
				{
					m_pUninstall->LinkRefresh(ksm::SIA_Start, TRUE);
				}
				break;

			case UNI_TYPE_QUICK:
				{
					m_pUninstall->LinkRefresh(ksm::SIA_Quick, TRUE);
				}
				break;

			case UNI_TYPE_DESKTOP:
				{
					m_pUninstall->LinkRefresh(ksm::SIA_Desktop, TRUE);
				}
				break;

			case UNI_TYPE_PROCESS:
				{
					m_pUninstall->LinkRefresh(ksm::SIA_Process, TRUE);
				}
				break;

			case UNI_TYPE_TRAY:
				{
					m_pUninstall->LinkRefresh(ksm::SIA_Tray, TRUE);
				}
				break;
			}
		}
	}
	else if (m_nTab == TAB_INDEX_UPDATE)
	{
		if (m_bInitUpdate)
			_LoadUpdate();
	}
	else if (m_nTab == TAB_INDEX_PHONE)
	{
		CString strUrl;
		if( IsTestSoftPhone( strUrl ) )
			m_IEPhoneNess.Show2( strUrl, m_strErrorHtml );
		else
			m_IEPhoneNess.Show2(L"http://baike.ijinshan.com/ksafew/site/sj/index.html", m_strErrorHtml);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnQueryUninstall()
{
	if (m_bLoadFail)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8020), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		return;
	}

	CString strText;
	m_editUninstall.GetWindowText(strText);

	if((strText != STR_DEFAULT_SOFTMGR) && (strText != _T("")))
	{
		if (m_nTab == TAB_INDEX_UNINSTALL)
		{
			// 卸载部分的查询
			OnUninstallBack();

			m_bQueryUni = TRUE;

			m_pDlg->SetItemVisible(IDR_FIND_QUERY_UNI_SOFTMGR,TRUE);
			m_pDlg->SetItemVisible(IDC_SMR_UNI_HEADER_TYPE,FALSE);
			m_pDlg->SetItemVisible(IDC_SMR_UNI_HEADER_COUNT,FALSE);

			m_pDlg->SetItemAttribute(IDC_UNINSTALL_SOFTLIST_HEADER, "pos", "0,0,-0,44");
			m_pDlg->SetItemAttribute(IDC_UNINSTALL_SOFTLIST, "pos", "0,44,-0,-0");

			//显示详情页按钮
			ShowUninstallInfoBar( FALSE );

			// 填充列表
			CSimpleArray<SOFT_UNI_INFO*> arrSoftUniDataTemp;

			CString keyword;
			PreprocessKeyword(strText, keyword);
			CWildcard wildcard(keyword, FALSE);

			for ( int i=0; i<m_arrSoftUniData.GetSize(); i++ )
			{
				SOFT_UNI_INFO * pInfo = m_arrSoftUniData[i];
				BOOL bFind = FALSE;
				bFind = IsSearchInfo(pInfo, wildcard);
				if (bFind)
					arrSoftUniDataTemp.Add(pInfo);
			}
			m_UniExpandSoftList.ClearAll();

			BOOL bQueryMark = FALSE;
			for ( int i=0; i<arrSoftUniDataTemp.GetSize(); i++ )
			{
				SOFT_UNI_INFO * pInfo = arrSoftUniDataTemp[i];

				if (pInfo->fMark == 0 && m_pInfoQuery)
				{
					int nTmp = _wtoi( pInfo->strSoftID );
					m_pInfoQuery->AddSoft(nTmp);
					bQueryMark = TRUE;
				}

				m_UniExpandSoftList.InsertItemX(pInfo);
			}
			m_UniExpandSoftList.UpdateAll();
			if (bQueryMark && m_pInfoQuery)
				m_pInfoQuery->Query();

			if (m_UniExpandSoftList.GetAllCount() == 0)
			{
				m_pDlg->FormatRichText(
					IDS_TIP_QUERY_UNI_SOFTMGR, 
					BkString::Get(IDS_NO_QUERY_SOFTMGR), 
					strText);

				m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_UNI_SOFTMGR, TRUE);
				m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST_HEADER, FALSE);
				m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST, FALSE);

			}
			else
			{
				m_pDlg->FormatRichText(
					IDS_TIP_QUERY_UNI_SOFTMGR, 
					BkString::Get(IDS_RESULT_QUERY_SOFTMGR),
					strText, m_UniExpandSoftList.GetAllCount());

				m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST, TRUE);
			}
		}

		m_strUninstKeyWord = strText;
		m_pDlg->SetTimer( TIMER_ID_UNINST_KEYWORD, 5000, NULL );
	}
	else if (strText == _T(""))
	{
		OnClearUninstallQeuryEdit();
	}

	//m_editUninstall.SetFocus();	
}

void CBeikeSafeSoftmgrUIHandler::OnQuerySoft()
{
	if (!m_bInitData)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8033), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		return;
	}

	if (m_bLoadFail)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg( BkString::Get(IDS_SOFTMGR_8020), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );

		return;
	}

	CString strText;
	m_edit.GetWindowText(strText);

	if((strText != STR_DEFAULT_SOFTMGR) && (strText != _T("")))
	{
		if (m_nTab == TAB_INDEX_DAQUAN)
		{
			OnBack();

			m_bQuery = TRUE;

			m_pDlg->SetItemVisible( IDC_LIST_SOFTMGR, FALSE );

			//读取
			CSimpleArray<CSoftListItemData*> arrQuery;
			if (m_pSoftMgr != NULL)
			{
				BOOL bFreeChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_FREESOFT);
				BOOL bPlugChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT);

				void *pos = m_pSoftMgr->SearchSoft(strText, L"ar");
				CAtlMap<CString,CString> soft;
				while(0 == m_pSoftMgr->GetNextSoft(pos, GetInfoUseMap,&soft))
				{	
					CString strTmp = soft[_T("softid")];	//软件ID

					CSoftListItemData *pData = NULL;
					pData = GetDataBySoftID(strTmp);
					if (pData && !IsDataFilterByCheck(pData, bFreeChecked, bPlugChecked))
						arrQuery.Add(pData);
				}
				m_pSoftMgr->FinalizeGet(pos);
			}

			//排序
			CAtlList<CSoftListItemData*> arrOrder;
			_SortArrayByOrder(arrQuery, arrOrder);

			//显示
			m_list.ClearAllItemData();
			ShowDataToList(arrOrder, TRUE);

			m_pDlg->SetItemVisible(IDR_FIND_QUERY_SOFTMGR, TRUE);
			m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_TYPE, FALSE);
			m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_COUNT, FALSE);

			m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );
			m_pDlg->SetItemAttribute(IDC_DAQUAN_SOFTLIST_HEADER, "pos", "0,0,-0,24");
			m_pDlg->SetItemAttribute(IDC_LIST_SOFTMGR, "pos", "0,24,-0,-0");

			if (m_list.GetItemCount() == 0)
			{
				m_pDlg->FormatRichText(
					IDS_TIP_QUERY_SOFTMGR, 
					BkString::Get(IDS_NO_QUERY_SOFTMGR), 
					strText);
				CString str;
				str.Format(	BkString::Get(IDS_SOFTMGR_8051), strText);

				m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, TRUE);
				m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);

			}
			else
			{
				m_pDlg->FormatRichText(
					IDS_TIP_QUERY_SOFTMGR, 
					BkString::Get(IDS_RESULT_QUERY_SOFTMGR), 
					strText, m_list.GetItemCount());
				m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, FALSE);
				m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);
			}

			CRect rcWnd;
			m_list.GetWindowRect(&rcWnd);
			OnListReSize(rcWnd);
		}

		m_strAllSoftKeyWord = strText;
		m_pDlg->SetTimer( TIMER_ID_ALLSOFE_KEYWORD, 5000, NULL );
	}
	else if (strText == _T(""))
	{
		if (m_nTab == TAB_INDEX_DAQUAN)
		{
			m_SoftTypeList.SetCurSelTypeName(m_strCurType, TRUE);
		}
	}

	//	m_edit.SetFocus();	
}

BOOL CBeikeSafeSoftmgrUIHandler::IsSearchInfo(SOFT_UNI_INFO * pInfo, CWildcard &wildcard)
{
	if (!pInfo)
		return FALSE;

	//	if (pInfo->bShowInList == FALSE)
	//		return FALSE;

	if (pInfo->bEnable == TRUE)
		return FALSE;

	if(
		wildcard.IsMatch(pInfo->strCaption) ||
		wildcard.IsMatch(pInfo->strSpellWhole) ||
		wildcard.IsMatch(pInfo->strSpellAcronym) ||
		wildcard.IsMatch(pInfo->strDesc)
		)
		return TRUE;

	return FALSE;
}

void CBeikeSafeSoftmgrUIHandler::OnDownloadMgr()
{
	CBkDownloadMgrDlg	dlg( this );

	// 检查当前是否有正在下载
	if ( m_bShowDownloaded )
	{
		dlg.DoModal( 1, GetActiveWindow() );
	}
	else
	{
		dlg.DoModal( 0, GetActiveWindow() );
	}
}

void CBeikeSafeSoftmgrUIHandler::OnSetIEMainhome()
{
	static const LPCTSTR STR_IE_HOMPAGE_REGNAME = _T("Start Page");
	static const LPCTSTR STR_IE_HOMPAGE_REGPATH = _T("Software\\Microsoft\\Internet Explorer\\Main");
	static const TCHAR strValue[] = _T("http://wan.duba.net/?utm_source=ws&utm_medium=client&utm_content=index&utm_campaign=wan_ie_index");

	CBkSafeMsgBox2	msgbox;
	msgbox.AddButton(L"确定", IDOK);
	msgbox.AddButton(L"取消", IDCANCEL);
	UINT nRet = msgbox.ShowMutlLineMsg(_T("将要设置“游戏大厅”为 IE 首页。确定吗？"), 
		NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
	if(nRet!=IDOK)
		return;

	//修改首页之前先要通知DUMMY
	CSafeMonitorTrayShell safeTray;
	safeTray.SetDefaultHomepage(strValue);

	int ret = RegisterOperate::CRegisterOperate::GetRegOperatePtr()->SetRegValue(
		HKEY_CURRENT_USER,
		STR_IE_HOMPAGE_REGPATH, 
		STR_IE_HOMPAGE_REGNAME, 
		strValue, 
		sizeof(strValue) - sizeof(TCHAR));

	if(ret == ERROR_SUCCESS)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg(_T("设置“游戏大厅”为 IE 首页成功"), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONINFORMATION );
	}
	else
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg(_T("设置“游戏大厅”为 IE 首页失败"), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );
	}
}

void CBeikeSafeSoftmgrUIHandler::OnSetDesktopLink()
{
	static const LPCTSTR LINK_FILE_NAME = _T("金山游戏大厅");

	CBkSafeMsgBox2	msgbox;
	msgbox.AddButton(L"确定", IDOK);
	msgbox.AddButton(L"取消", IDCANCEL);
	UINT nRet = msgbox.ShowMutlLineMsg(_T("将要在桌面上添加“游戏大厅”图标。确定吗？"), 
		NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
	if(nRet!=IDOK)
		return;

	BOOL succeeded = FALSE;

	do
	{
		CComPtr<IShellLinkW> pShellLink;
		HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
		if(!SUCCEEDED(hr)) break;

		CComPtr<IPersistFile> pPersistFile;
		hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
		if(!SUCCEEDED(hr)) break;

		CString iconPath;
		GetKSoftIconDir(iconPath);

		pShellLink->SetPath(L"http://wan.duba.net/?utm_source=ws&utm_medium=client&utm_content=desk&utm_campaign=wan_desk_icon");
		pShellLink->SetIconLocation(iconPath + L"\\金山游戏大厅.ico", 0);

		TCHAR dstPath[MAX_PATH] = {0};
		{
			::SHGetSpecialFolderPath(NULL, dstPath, CSIDL_DESKTOPDIRECTORY, FALSE);
			::PathAddBackslash(dstPath);
			wcscat_s(dstPath, MAX_PATH, LINK_FILE_NAME);
			wcscat_s(dstPath, MAX_PATH, L".lnk");
		}

		hr = pPersistFile->Save(dstPath, FALSE);
		succeeded = SUCCEEDED(hr);
	}
	while(FALSE);

	if(succeeded)
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg(_T("在桌面上添加“游戏大厅”图标成功"), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONINFORMATION );
	}
	else
	{
		CBkSafeMsgBox2 msgbox;
		msgbox.AddButton( TEXT("确定"), IDOK);
		msgbox.ShowMutlLineMsg(_T("在桌面上添加“游戏大厅”图标失败"), NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING );
	}
}

BOOL CBeikeSafeSoftmgrUIHandler::InsertSoftInfo(CSoftListItemData *pData, BOOL bPlugCheck, BOOL bFreeCheck, BOOL& bQueryMark)
{
	BOOL bResult = FALSE;

	if( pData != NULL )
	{
		if ( m_list.FindItem(pData))
		{
			bResult = FALSE;
			goto Exit0;
		}

		if (pData->m_fMark == 0 && m_pInfoQuery)
		{
			int nTmp = _wtoi( pData->m_strSoftID );
			m_pInfoQuery->AddSoft(nTmp);
			bQueryMark = TRUE;
		}

		if( bPlugCheck && bFreeCheck )
		{
			if ( !pData->m_bCharge && !pData->m_bPlug )
			{
				pData->m_dwID = m_list.GetItemCount();
				m_list.InsertItem(m_list.GetItemCount(), _T(""));
				m_list.SetItemDataEx(0, (DWORD_PTR)pData);
				m_list.SetPaintItem(TRUE);

				bResult = TRUE;
			}
		}
		else if ( bPlugCheck && !bFreeCheck )
		{
			if ( !pData->m_bPlug )
			{
				pData->m_dwID = m_list.GetItemCount();
				m_list.InsertItem(m_list.GetItemCount(), _T(""));
				m_list.SetItemDataEx(0, (DWORD_PTR)pData);
				m_list.SetPaintItem(TRUE);

				bResult = TRUE;
			}
		}
		else if ( !bPlugCheck && bFreeCheck )
		{
			if ( !pData->m_bCharge )
			{
				pData->m_dwID = m_list.GetItemCount();
				m_list.InsertItem(m_list.GetItemCount(), _T(""));
				m_list.SetItemDataEx(0, (DWORD_PTR)pData);
				m_list.SetPaintItem(TRUE);

				bResult = TRUE;
			}
		}
		else
		{
			pData->m_dwID = m_list.GetItemCount();
			m_list.InsertItem(m_list.GetItemCount(), _T(""));
			m_list.SetItemDataEx(0, (DWORD_PTR)pData);
			m_list.SetPaintItem(TRUE);

			bResult = TRUE;
		}
	}

Exit0:
	if (bResult && pData)
	{
		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
			strStore.Append(_T("\\")+ pData->m_strFileName);
		else
			strStore.Append(pData->m_strFileName);
		if (PathFileExists(strStore) && !pData->m_bSetup)
		{
			pData->m_strPath = strStore;
			pData->m_dwProgress = 100;
			pData->m_bDownloading = TRUE;
			pData->m_bDownLoad = TRUE;
		}
	}

	return bResult;
}

void CBeikeSafeSoftmgrUIHandler::GetSoftInfoByCondition()
{
	BOOL bFreeChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_FREESOFT);
	BOOL bPlugChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT);
	if(bFreeChecked || bPlugChecked)
	{
		m_list.ClearAllItemData();
		m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );
		if (m_bQuery)
		{
			OnQuerySoft();
		}
		else
		{
			ShowItemsBySoftType(m_strCurType);

			CRect rcWnd;
			m_list.GetWindowRect(&rcWnd);
			OnListReSize(rcWnd);
		}
	}
	else
	{
		if (m_bQuery)
		{
			OnQuerySoft();
		}
		else
		{
			OnRefresh();
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::OnShowNoPlugSoft()
{
	BOOL bCheck = m_pDlg->GetItemCheck( IDC_CHECKBOX_DAQUAN_NOPLUGSOFT );
	BKSafeConfig::SetShowNoPlugSet( bCheck );

	GetSoftInfoByCondition();
}

void CBeikeSafeSoftmgrUIHandler::OnShowFreeSoft()
{
	BOOL bCheck = m_pDlg->GetItemCheck( IDC_CHECKBOX_DAQUAN_FREESOFT );
	BKSafeConfig::SetShowFreeSoftSet( bCheck );

	GetSoftInfoByCondition();	
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateCheckAll()
{
	BOOL bAllChecked = m_pDlg->GetItemCheck(IDC_UPDATE_CHECKBOX_ALL);
	if (bAllChecked)
	{
		if (m_UpdateSoftList.IsWindow())
			::SendMessage(m_UpdateSoftList.m_pHostWnd->m_hWnd, WM_USER_SET_CHECK_LIST, USCT_ALL, TRUE);
	}
	else
	{
		if (m_UpdateSoftList.IsWindow())
			::SendMessage(m_UpdateSoftList.m_pHostWnd->m_hWnd, WM_USER_SET_CHECK_LIST, USCT_ALL, FALSE);
	}

	int nCheck = m_UpdateSoftList.GetCheck();
	if (nCheck == BST_CHECKED || nCheck == BST_INDETERMINATE)
		m_pDlg->EnableItem( IDC_UPDATE_SELECT_SOFT, TRUE);
	else
		m_pDlg->EnableItem( IDC_UPDATE_SELECT_SOFT, FALSE);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateBtnIgnore()
{
	if (!m_bInitUpdate)
		return;

	CBkIgnoreDlg	dlg( this );

	for (int i = 0; i < m_arrCheckUpdate.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrCheckUpdate[i];
		if (pData && pData->m_bIgnore == TRUE)
		{
			dlg.m_arrData.Add(pData);
		}
	}

	int nIgnoreCountOld = dlg.m_arrData.GetSize();
	if (nIgnoreCountOld)
	{
		UINT_PTR uRet = dlg.DoModal(GetActiveWindow());

		int nIgnoreCountNew = 0;
		for (int i = 0; i < dlg.m_arrData.GetSize(); i++)
		{
			CSoftListItemData *pData = dlg.m_arrData[i];
			if (pData)
			{
				if (pData->m_bIgnore == TRUE)
					nIgnoreCountNew++;
			}
		}

		if (nIgnoreCountNew != nIgnoreCountOld)
		{
			_LoadUpdate();
			OnUpdateCheckAll();

			if (nIgnoreCountNew)
				SaveUpdateIgnoreList();
			else
			{
				CString  strCachPath;
				if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
					strCachPath.Append(_T("\\ksoft_ignore"));
				DeleteFile(strCachPath);
			}
		}

		if (IDOK == uRet)//恢复某些软件的升级
		{
			if (dlg.m_datalistUpdate)//立即升级某软件
			{
				_downLoadSoft(dlg.m_datalistUpdate, ACT_SOFT_FROM_UPDATE);
			}

			return;
		}
	}
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSetCheckListNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == USCT_ALL)
	{
		m_pDlg->SetItemCheck( IDC_UPDATE_CHECKBOX_ALL, lParam);
	}

	int nCheck = m_UpdateSoftList.GetCheck();
	if (nCheck == BST_CHECKED || nCheck == BST_INDETERMINATE)
		m_pDlg->EnableItem( IDC_UPDATE_SELECT_SOFT, TRUE);
	else
		m_pDlg->EnableItem( IDC_UPDATE_SELECT_SOFT, FALSE);

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnLoadUpdateCache(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_UpdateSoftList.DeleteAllItems();
	m_pDlg->EnableItem(IDC_UPDATE_SELECT_SOFT, FALSE);
	m_pDlg->SetItemCheck( IDC_UPDATE_CHECKBOX_ALL, FALSE);
	//m_pDlg->SetItemCheck( IDC_UPDATE_CHECKBOX_IGNORE, FALSE);

	//
	if (m_arrIgnore.GetSize())
	{
		for ( int _nIndex = 0; _nIndex < m_arrUpdCache.GetSize(); ++_nIndex )
		{
			CSoftListItemData *pData = m_arrUpdCache[_nIndex];
			if (pData && pData->m_bIgnore == FALSE)
			{
				int nFind = m_arrIgnore.Find(pData->m_strSoftID);
				if (nFind != -1)
				{
					pData->m_bIgnore = TRUE;
				}
			}
		}
	}

	//排序
	CAtlList<CSoftListItemData*> arrTemp;
	//_SortArrayByOrder(m_arrUpdCache, arrTemp);
	_SortArrayByLastUpdate(m_arrUpdCache, arrTemp);

	CSoftListItemData *pData = NULL;
	int nCount = 0;
	for ( int _nIndex = 0; _nIndex < m_arrUpdCache.GetSize(); ++_nIndex )
	{
		if ( m_arrUpdCache[_nIndex]->m_bIgnore == FALSE )
			nCount++;
	}
	if (nCount == 0)
	{
		m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE, FALSE);
		m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, FALSE);
		m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);
	}
	else
	{
		m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE, FALSE);
		m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, TRUE);
		m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);

		BOOL bQueryMark = FALSE;
		CSoftListItemData *pDataTemp = NULL;
		POSITION posTemp =  arrTemp.GetHeadPosition();
		while (posTemp)
		{
			pDataTemp = arrTemp.GetAt(posTemp);
			if (pDataTemp)
			{
				if (pDataTemp->m_pImage == NULL)
					pDataTemp->m_pImage = m_pImge;

				int nSoftID = _wtoi( pDataTemp->m_strSoftID );
				if (!m_softUpdatedList.InList( nSoftID ) && 
					pDataTemp->m_bIgnore == FALSE)
				{
					m_UpdateSoftList.AddItem((DWORD_PTR)pDataTemp);

					if (pDataTemp->m_fMark == 0 && m_pInfoQuery)
					{
						m_pInfoQuery->AddSoft(nSoftID);
						bQueryMark = TRUE;
					}
				}
			}

			arrTemp.GetNext(posTemp);
		}
		if (bQueryMark && m_pInfoQuery)
			m_pInfoQuery->Query();

		m_UpdateSoftList.UpdateCtrlWindow();
	}

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnUpdateUniType(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CString strType;
	strType = GetTypeUI(m_strCurTypeUni);
	m_pDlg->SetItemText(IDC_SMR_UNI_HEADER_TYPE, strType);
	CString strCount;
	strCount.Format(L"共%d款", m_UniExpandSoftList.GetAllCount());
	m_pDlg->SetItemText(IDC_SMR_UNI_HEADER_COUNT, strCount);

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnLoadUpdateReal(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//一键装机可能已经打开了，这时再通知其刷新
	if (m_necessUIHandler.m_pOnekeyDlg)
		m_necessUIHandler.m_pOnekeyDlg->RefreshCheckNumberSize();

	m_pDlg->SetItemVisible( IDC_DLG_PROG3, FALSE );

	m_pDlg->EnableItem(IDC_UPDATE_SELECT_SOFT, FALSE);
	m_pDlg->SetItemCheck( IDC_UPDATE_CHECKBOX_ALL, FALSE);
	//m_pDlg->SetItemCheck( IDC_UPDATE_CHECKBOX_IGNORE, FALSE);

	//
	if (m_arrIgnore.GetSize())
	{
		for ( int _nIndex = 0; _nIndex < m_arrCheckUpdate.GetSize(); ++_nIndex )
		{
			CSoftListItemData *pData = m_arrCheckUpdate[_nIndex];
			if (pData && pData->m_bIgnore == FALSE)
			{
				int nFind = m_arrIgnore.Find(pData->m_strSoftID);
				if (nFind != -1)
				{
					pData->m_bIgnore = TRUE;
				}
			}
		}
	}

	_LoadUpdate();
	m_bInitUpdate = TRUE;
	if (m_nSoftIdByCmd != 0)
	{
		CString strSoftId;
		strSoftId.Format(L"%d", m_nSoftIdByCmd);
		m_UpdateSoftList.UpdateSoft(strSoftId);
	}
	return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::_LoadUpdate()
{
	m_UpdateSoftList.DeleteAllItems();

	BOOL bIgnoreChecked = FALSE;//m_pDlg->GetItemCheck(IDC_UPDATE_CHECKBOX_IGNORE);

	//排序
	CAtlList<CSoftListItemData*> arrTemp;
	//_SortArrayByOrder(m_arrCheckUpdate, arrTemp);
	_SortArrayByLastUpdate(m_arrCheckUpdate, arrTemp);
	_MoveMaijorFirst(arrTemp);

	CSoftListItemData *pData = NULL;
	int nCount = 0;
	m_nCountUpdateIgnore = 0;
	if (bIgnoreChecked == FALSE)
	{
		for ( int _nIndex = 0; _nIndex < m_arrCheckUpdate.GetSize(); ++_nIndex )
		{
			int nSoftID = _wtoi( m_arrCheckUpdate[_nIndex]->m_strSoftID );
			if ( !m_softUpdatedList.InList( nSoftID ) && 
				m_arrCheckUpdate[_nIndex]->m_bUpdate == TRUE &&
				m_arrCheckUpdate[_nIndex]->m_bIgnore == FALSE )
			{
				nCount++;
			}
			if ( !m_softUpdatedList.InList( nSoftID ) && 
				m_arrCheckUpdate[_nIndex]->m_bUpdate == TRUE &&
				m_arrCheckUpdate[_nIndex]->m_bIgnore == TRUE )
			{
				m_nCountUpdateIgnore++;
			}
		}
	}
	else
	{
		for ( int _nIndex = 0; _nIndex < m_arrCheckUpdate.GetSize(); ++_nIndex )
		{
			int nSoftID = _wtoi( m_arrCheckUpdate[_nIndex]->m_strSoftID );
			if ( !m_softUpdatedList.InList( nSoftID ) &&
				m_arrCheckUpdate[_nIndex]->m_bUpdate == TRUE )
			{
				nCount++;
			}
			if ( !m_softUpdatedList.InList( nSoftID ) && 
				m_arrCheckUpdate[_nIndex]->m_bUpdate == TRUE &&
				m_arrCheckUpdate[_nIndex]->m_bIgnore == TRUE )
			{
				m_nCountUpdateIgnore++;
			}
		}
	}
	m_pDlg->FormatRichText(
		IDC_UPDATE_IGNORE_NUM, 
		BkString::Get(IDS_DLG_UPDATE_IGNORE_NUM_FORMAT), 
		m_nCountUpdateIgnore );
	if (m_nCountUpdateIgnore == 0)
	{
		m_pDlg->SetItemVisible(IDC_UPDATE_IGNORE_BTN, FALSE);
		//m_pDlg->SetItemVisible(IDC_UPDATE_CHECKBOX_IGNORE, FALSE);
	}
	else
	{
		m_pDlg->SetItemVisible(IDC_UPDATE_IGNORE_BTN, TRUE);
		//m_pDlg->SetItemVisible(IDC_UPDATE_CHECKBOX_IGNORE, TRUE);
	}
	if (nCount == 0)
	{
		if (m_nCountUpdateIgnore == 0)
		{
			m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE, TRUE);
			m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, FALSE);
			m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);
		}
		else
		{
			m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE, FALSE);
			m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, TRUE);
			m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);

			m_pDlg->SetItemVisible(IDC_UPDATE_NEED_PAGE, FALSE);
			m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE_BUT_IGNORE, TRUE);
			m_pDlg->FormatRichText(
				IDC_UPDATE_CHECK_IGNORE_COUNT, 
				BkString::Get(IDS_DLG_UPDATE_CHECK_IGNORE_COUNT_FORMAT), 
				m_nCountUpdateIgnore );
		}
	}
	else
	{
		m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE, FALSE);
		m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, TRUE);
		m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);

		m_pDlg->SetItemVisible(IDC_UPDATE_NEED_PAGE, TRUE);
		m_pDlg->SetItemVisible(IDC_UPDATE_NONE_PAGE_BUT_IGNORE, FALSE);

		BOOL bQueryMark = FALSE;
		CSoftListItemData *pDataTemp = NULL;
		POSITION posTemp =  arrTemp.GetHeadPosition();
		while (posTemp)
		{
			pDataTemp = arrTemp.GetAt(posTemp);
			if (pDataTemp)
			{
				int nSoftID = _wtoi( pDataTemp->m_strSoftID );
				BOOL bCanInsert = FALSE;
				if (bIgnoreChecked == FALSE)
				{
					if (!m_softUpdatedList.InList( nSoftID ) && 
						pDataTemp->m_bIgnore == FALSE &&
						pDataTemp->m_bUpdate == TRUE)
					{
						bCanInsert = TRUE;
					}
				}
				else
				{
					if (!m_softUpdatedList.InList( nSoftID ) &&
						pDataTemp->m_bUpdate == TRUE)
					{
						bCanInsert = TRUE;
					}
				}

				if (bCanInsert)
				{
					m_UpdateSoftList.AddItem((DWORD_PTR)pDataTemp);

					if (pDataTemp->m_fMark == 0 && m_pInfoQuery)
					{
						m_pInfoQuery->AddSoft(nSoftID);
						bQueryMark = TRUE;
					}
				}
			}

			arrTemp.GetNext(posTemp);
		}
		if (bQueryMark && m_pInfoQuery)
			m_pInfoQuery->Query();

		m_UpdateSoftList.UpdateCtrlWindow();
	}

	::PostMessage( m_pDlg->m_hWnd, WM_UPDATE_SOFT_COUNT, 0, 0 );
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnChangeUninstallEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CString strText;
	m_editUninstall.GetWindowText(strText);
	if (strText.GetLength() != 0 && strText == BkString::Get(IDS_SOFTMGR_8016))
		m_pDlg->SetItemVisible(IDC_SOFT_UNISTALL_BTN_QEURY_EDIT, FALSE);
	else if (strText.GetLength() == 0)
		m_pDlg->SetItemVisible(IDC_SOFT_UNISTALL_BTN_QEURY_EDIT, FALSE);
	else
		m_pDlg->SetItemVisible(IDC_SOFT_UNISTALL_BTN_QEURY_EDIT, TRUE);

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnChangeDaquanEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CString strText;
	m_edit.GetWindowText(strText);
	if (strText.GetLength() != 0 && strText == BkString::Get(IDS_SOFTMGR_8016))
		m_pDlg->SetItemVisible(IDC_SOFT_DAQUAN_BTN_QEURY_EDIT, FALSE);
	else if (strText.GetLength() == 0)
		m_pDlg->SetItemVisible(IDC_SOFT_DAQUAN_BTN_QEURY_EDIT, FALSE);
	else
		m_pDlg->SetItemVisible(IDC_SOFT_DAQUAN_BTN_QEURY_EDIT, TRUE);

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnUniComboBoxSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int nCurSel = m_comBoBoxUni.GetCurSel();
	switch(nCurSel)
	{
	case 0:	OnBtnUniAll();break;
	case 1:	OnBtnUniDesk();break;
	case 2:	OnBtnUniStartMenu();break;
	case 3:	OnBtnUniQuickLan();break;
	case 4:	OnBtnUniProcess();break;
	case 5:	OnBtnUniTray();break;
	}

	return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::OnClearDaquanQeuryEdit()
{
	if (m_bViewDetail)
		OnBack();
	if (m_bQuery)
		OnBackFromQuery();
}

void CBeikeSafeSoftmgrUIHandler::OnClearUninstallQeuryEdit()
{
	OnUninstallBack(TRUE);
}

void CBeikeSafeSoftmgrUIHandler::OnListReSize(CRect rcWnd)
{
	if(m_list)
	{
		if (rcWnd.Height() > m_list.GetItemCount() * 54)
		{
			m_list.SetColumnWidth(0, rcWnd.Width());
		}
		else
		{
			m_list.SetColumnWidth(0, rcWnd.Width() - 18);
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::OnDownLoadFromeDetail()
{
	m_IEDetail.Show2(_T("about:blank"), NULL);

	m_pDlg->SetItemVisible(IDC_DLG_DETAIL_SOFTMGR, FALSE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);

	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);

	//	m_pDlg->SetItemVisible(IDC_BATCHDOWN_SOFTMGR, TRUE);
	m_bViewDetail = FALSE;
	if (!m_strDetailSoftID.IsEmpty())
	{
		_QuerySoftMark(m_strDetailSoftID);
		m_strDetailSoftID.Empty();
		if ( ::GetFocus() != m_edit.m_hWnd )
			m_pDlg->SetFocus();
	}

	//	m_pDlg->SetFocus();

	if (m_nDown >= 0)
	{
		OnDownLoad(m_nDown);
	}
}

CString CBeikeSafeSoftmgrUIHandler::GetCurrentSystemVersion()
{
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;
	CString strVersion = _T("UnKnowVersion");

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return strVersion;
	}

	pGNSI = (PGNSI) GetProcAddress(	GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if(NULL != pGNSI)
	{
		pGNSI(&si);
	}
	else
	{
		GetSystemInfo(&si);
	}

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				strVersion = _T("Win7");
			else 
				strVersion = _T("WinS2008" );
		}

		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				strVersion = _T("Vista");
			else 
				strVersion = _T("WinS2008" );
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
		{
			if( GetSystemMetrics(SM_SERVERR2) )
				strVersion = _T( "WinS2003");
			else if( osvi.wProductType == VER_NT_WORKSTATION &&
				si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			{
				strVersion = _T( "WinXP");
			}
			else 
				strVersion = _T("WinS2003");
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			strVersion = _T("WinXP");

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			strVersion = _T("Win2000 ");

		if ( osvi.dwMajorVersion <= 4 )
			strVersion = _T("WinNT ");

		break;

	case VER_PLATFORM_WIN32_WINDOWS:
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			strVersion = _T("Win9x");
		} 

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			strVersion = _T("Win9x");
		} 

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			strVersion = _T("WinME");
		} 
		break;

	case VER_PLATFORM_WIN32s:

		strVersion = _T("Win32s");
		break;
	}

	return strVersion; 
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrInitFinish(UINT , WPARAM wParam, LPARAM lParam)
{
	if (m_bInitData)
	{
		if (m_strBibeiCmd.GetLength())
			DoBibeiCmd(m_strBibeiCmd);

		if (m_strDaquanCmd.GetLength())
			DoDaquanCmd(m_strDaquanCmd);
	}

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnOnSoftmgrRefreshLoadTip(UINT , WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = (BOOL)wParam;

	m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);

	if (m_bInitData)
	{
		if (m_pSoftMgr==NULL)
			return FALSE;
		m_necessUIHandler.OnFirstShow();
		m_pDlg->SetItemVisible(IDC_DLG_PROG1,FALSE);
		m_pDlg->SetItemVisible(IDC_SOFT_NECESS_DIV_ALL,TRUE);

		// 初始化上面提示软件个数
		m_pDlg->FormatRichText(
			IDC_SOFT_TXT_NUMBER_TOP_TIP, 
			BkString::Get(IDS_SOFTMGR_8122), 
			m_pSoftMgr->GetSoftCount( CString(L"")));
	}

	return 0;
}

void CBeikeSafeSoftmgrUIHandler::OnDetailUninstall()
{
	if (m_pSelSoftUniData)
	{
		if (m_pUninstall && m_pSelSoftUniData && m_pSelSoftUniData->bEnable == FALSE)
		{

			ATLASSERT(m_dlgSoftMgrUninstall==NULL);
			m_dlgSoftMgrUninstall = new CBkSafeSoftMgrUnstDlg(this, m_pUninstall, m_pSelSoftUniData->strCaption,m_pSelSoftUniData->strKey );
			m_dlgSoftMgrUninstall->DoModal();
			m_pSelSoftUniData->bEnable = m_dlgSoftMgrUninstall->IsSoftUnistComplete();
			delete m_dlgSoftMgrUninstall;
			m_dlgSoftMgrUninstall = NULL;

			FreshTotal();

			if (m_pSelSoftUniData->bEnable)
			{
				m_UniExpandSoftList.RefreshRight(m_pSelSoftUniData->strKey);
				m_pDlg->SetItemAttribute(IDC_BTN_UNINSTALL_DO, "skin", "uninstalled_btn");

				if ( !m_pSelSoftUniData->strSoftID.IsEmpty() && m_arrDataMap.Lookup(m_pSelSoftUniData->strSoftID))
				{
					CSoftListItemData *pData = NULL;
					if (m_arrDataMap.Lookup(m_pSelSoftUniData->strSoftID, pData))
					{
						pData->m_bSetup		= FALSE;
						pData->m_bUpdate	= FALSE;
						_RefershItemBySoftIDMainThread(pData->m_strSoftID);
					}
				}

			}
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::ShowLoadingTip()
{
	m_pDlg->SetItemVisible( IDC_DLG_LOADING_TIP, TRUE );

	m_pDlg->SetTimer( TIMER_LOADINFO_BIGBTN, 100, NULL );
}

void CBeikeSafeSoftmgrUIHandler::HideLoadingTip()
{
	m_pDlg->SetItemVisible( IDC_DLG_LOADING_TIP, FALSE );

	m_pDlg->KillTimer( TIMER_LOADINFO_BIGBTN );
}

void CBeikeSafeSoftmgrUIHandler::ShowItemsBySoftType(CString strSoftType)
{
	BOOL bFreeChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_FREESOFT);
	BOOL bPlugChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT);

	if (strSoftType.CompareNoCase(BkString::Get(IDS_SOFTMGR_8018)) == 0)//最新更新
	{
		CSimpleArray<CSoftListItemData*> arrFilter;
		CAtlList<CSoftListItemData*> lstOrder;

		//生成
		if (m_pSoftMgr != NULL && m_arrDataNew.GetSize() == 0)
		{
			int nCount = 0;
			void *pos = m_pSoftMgr->GetLast50Soft();
			CAtlMap<CString,CString> soft;
			while(0 == m_pSoftMgr->GetNextSoft(pos, GetInfoUseMap,&soft))
			{
				CString strTmp = soft[_T("softid")];
				CSoftListItemData *pData = NULL;
				pData = GetDataBySoftID(strTmp);
				if (pData)
					m_arrDataNew.Add(pData);
				if (m_arrDataNew.GetSize() == 50)
					break;
			}	
			m_pSoftMgr->FinalizeGet(pos);
		}

		//过滤
		for (int i = 0; i < m_arrDataNew.GetSize(); i++)
		{
			CSoftListItemData *pData = NULL;
			pData = m_arrDataNew[i];
			if (pData && !IsDataFilterByCheck(pData, bFreeChecked, bPlugChecked))
				arrFilter.Add(pData);
		}

		//不排序，转列表
		_ChangeArrayToList(arrFilter, lstOrder);

		//显示
		m_list.ClearAllItemData();
		ShowDataToList(lstOrder);
	}
	else if (strSoftType.CompareNoCase(BkString::Get(IDS_SOFTMGR_8019)) == 0)//全部
	{
		CSimpleArray<CSoftListItemData*> arrFilter;
		CAtlList<CSoftListItemData*> lstOrder;

		int nPageIndex = 1;
		int nCanInsert = -1;
		int nPageCount = PAGE_ITEM_MAX_COUNT;
		int nMin = nPageCount*(nPageIndex-1);
		int nMax = nPageCount*nPageIndex;

		m_arrDataSub = m_arrData;

		//过滤
		for (int i = 0; i < m_arrData.GetSize(); i++)
		{
			BOOL bFilter = IsDataFilterByCheck(m_arrData[i], bFreeChecked, bPlugChecked);
			if (!bFilter)
				nCanInsert++;

			if (nCanInsert != -1 && nCanInsert >= nMin && nCanInsert < nMax)
			{
				if (!bFilter)
				{
					CSoftListItemData *pData = NULL;
					pData = m_arrData[i];
					if (pData)
						arrFilter.Add(pData);
				}
			}
		}

		//排序
		_SortArrayByOrder(arrFilter, lstOrder);

		//显示
		m_list.ClearAllItemData();
		ShowDataToList(lstOrder);

		//显示分页控制
		if (nCanInsert > PAGE_ITEM_MAX_COUNT)
		{
			m_list.AppendPageItem(nPageIndex, nCanInsert);
			m_list.UpdateBtnRect();
		}
	}
	else
	{
		CSimpleArray<CSoftListItemData*> arrDataTemp;
		CSimpleArray<CSoftListItemData*> arrFilter;
		CAtlList<CSoftListItemData*> lstOrder;

		int nPageIndex = 1;
		int nCanInsert = -1;
		int nPageCount = PAGE_ITEM_MAX_COUNT;
		int nMin = nPageCount*(nPageIndex-1);
		int nMax = nPageCount*nPageIndex;

		//生成
		if (m_pSoftMgr != NULL)
		{
			void *pos = m_pSoftMgr->GetAllSoft(strSoftType, L"df");
			CAtlMap<CString,CString> soft;
			while(0 == m_pSoftMgr->GetNextSoft(pos, GetInfoUseMap,&soft))
			{
				CString strTmp = soft[_T("softid")];	//软件ID
				CSoftListItemData *pData = NULL;
				pData = GetDataBySoftID(strTmp);
				if (pData)
					arrDataTemp.Add(pData);
			}	
			m_pSoftMgr->FinalizeGet(pos);
		}
		m_arrDataSub = arrDataTemp;

		//过滤
		for (int i = 0; i < arrDataTemp.GetSize(); i++)
		{
			BOOL bFilter = IsDataFilterByCheck(arrDataTemp[i], bFreeChecked, bPlugChecked);
			if (!bFilter)
				nCanInsert++;

			if (nCanInsert != -1 && nCanInsert >= nMin && nCanInsert < nMax)
			{
				if (!bFilter)
				{
					CSoftListItemData *pData = NULL;
					pData = arrDataTemp[i];
					if (pData)
						arrFilter.Add(pData);
				}
			}
		}

		//排序
		_SortArrayByOrder(arrFilter, lstOrder);

		//显示
		m_list.ClearAllItemData();
		ShowDataToList(lstOrder);

		//显示分页控制
		if (nCanInsert > PAGE_ITEM_MAX_COUNT)
		{
			m_list.AppendPageItem(nPageIndex, nCanInsert);
			m_list.UpdateBtnRect();
		}
	}
}

unsigned CBeikeSafeSoftmgrUIHandler::ThreadInitDataProc(LPVOID lpVoid)
{
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler*)lpVoid;
	BOOL bChecked = FALSE;  //是否已经取得软件的信息
	pDlg->m_bViewDetail = FALSE;

	pDlg->GetDownloadCachInfo();

	// 另外的线程中的事情是不安全的  yykingking todo 

	pDlg->m_pDlg->SetTimer(TIMER_LOADINFO_SOFTMGR, 2000, NULL);
	pDlg->m_pDlg->SetTimer(TIMER_LOADTIP_SOFTMGR, 100, NULL);
	pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT1, "value", 10 );

	// 加载softmgr.dll, 创建对象, 加载softmgr.dat, rank.dat
	BOOL bOK = pDlg->m_bInitInterface;
	if(bOK)
	{
		// 		pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT1, "value", 50 );
		// 
		// 初始化左侧列表
		pDlg->InitLeftTypeList();
		// 		pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT1, "value", 60 );
		// 
		// 		pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT1, "value", 70 );
		// 


		//发送消息，显示加载信息的提示
		::SendMessage( pDlg->m_pDlg->m_hWnd, WM_LOADTIP_REFRESH, (WPARAM)pDlg->m_bCachExist, 0);

		pDlg->_CheckSoftUpdate();

		// 先加载装机必备的数据
		pDlg->LoadNecessData();

		pDlg->m_bInitData = TRUE;
		//发送消息，显示加载信息的提示
		::SendMessage( pDlg->m_pDlg->m_hWnd, WM_LOADTIP_REFRESH, (WPARAM)pDlg->m_bCachExist, 0);

		// 加载软件大全中所有的软件数据, 添加到m_arrData中, **
		pDlg->LoadAllSoftData();

		// 初始化右侧列表
		pDlg->InitRightSoftList();

		//pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT1, "value", 80 );

		CRect rcWnd;
		pDlg->m_list.GetWindowRect(&rcWnd);
		pDlg->OnListReSize(rcWnd);

		pDlg->GetConfig();//下载卸载需要的ini文件

		// 加载所有软件的图片信息，是否下载完成等信息
		pDlg->_UpdateSoftInfo();
		pDlg->_CheckSoftInstall();

		// 初始化结束，设置结束状态
		pDlg->m_bInitData = TRUE;

		::SendMessage( pDlg->m_pDlg->m_hWnd, WM_LOADTIP_REFRESH, (WPARAM)FALSE, 0);//发送消息，隐藏加载信息的提示
		::SendMessage( pDlg->m_pDlg->m_hWnd, WM_SOFT_INIT_FINISH, 0, 0);
	}
	else
	{
		pDlg->m_bLoadFail = TRUE;
		pDlg->m_bInitData = TRUE;
		pDlg->m_pDlg->SetItemText(IDC_LOAD_TIP1, BkString::Get(IDS_SOFTMGR_8020));
		pDlg->m_pDlg->SetItemText(IDC_LOAD_TIP2, BkString::Get(IDS_SOFTMGR_8020));
		pDlg->m_pDlg->SetItemText(IDC_LOAD_TIP3, BkString::Get(IDS_SOFTMGR_8020));
		pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT1, "value", 100 );
		pDlg->m_pDlg->SetItemIntAttribute(IDC_PROG_INIT2, "value", 100 );
		::SendMessage( pDlg->m_pDlg->m_hWnd, WM_BOTTOM_REFRESH, 0, 0 );
		::SendMessage( pDlg->m_pDlg->m_hWnd, WM_LOADTIP_REFRESH, (WPARAM)FALSE, 0);//发送消息，隐藏加载信息的提示
		pDlg->m_pDlg->KillTimer(TIMER_LOADTIP_SOFTMGR);
		pDlg->m_pDlg->KillTimer(TIMER_LOADINFO_SOFTMGR);
	}

	// 设置一周后删除下载的安装包时，删除安装包

	pDlg->m_hInitData = (HANDLE)-1;
	return 0;
}

void GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace )
{
	CString strDisk;
	strDisk = szCurrentPath;
	if ( ! strDisk.IsEmpty() )
	{
		strDisk = strDisk.Left( 3 );

		ULARGE_INTEGER   lpuse = {0}; 
		ULARGE_INTEGER   lptotal = {0};
		ULARGE_INTEGER   lpfree = {0};
		GetDiskFreeSpaceEx(strDisk,&lpuse,&lptotal,&lpfree);

		if ( lpfree.QuadPart > ( 1024 * 1024 * 1024 ) )
		{
			double	freeGB = lpfree.QuadPart / ( 1024 * 1024 * 1024.0 );
			strFreeSpace.Format( _T( "%.2fGB" ), freeGB );
		}
		else
		{
			ULONGLONG		freeMB = lpfree.QuadPart / ( 1024 * 1024 );
			strFreeSpace.Format( _T( "%dMB" ), freeMB );
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::OnBtnUniDesk()
{
	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

	OnUninstallBack();

	m_nUniType = UNI_TYPE_DESKTOP;

	if (m_pUninstall)
		m_pUninstall->LinkRefresh(ksm::SIA_Desktop, FALSE);

	m_pDlg->SetItemText( IDC_LNK_CUR_PAGE, BkString::Get(IDS_SOFTMGR_8064) );
	m_pDlg->SetItemVisible(IDC_UNINSTALL_BTM_DIV_ADVICE,FALSE);

	m_editUninstall.SetFocus();

}

void CBeikeSafeSoftmgrUIHandler::OnBtnUniProcess()
{
	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

	OnUninstallBack();

	m_nUniType = UNI_TYPE_PROCESS;

	if (m_pUninstall)
		m_pUninstall->LinkRefresh(ksm::SIA_Process, FALSE);

	m_pDlg->SetItemText( IDC_LNK_CUR_PAGE, BkString::Get(IDS_SOFTMGR_8065) );
	m_pDlg->SetItemVisible(IDC_UNINSTALL_BTM_DIV_ADVICE,FALSE);

	m_editUninstall.SetFocus();

}

void CBeikeSafeSoftmgrUIHandler::OnBtnUniQuickLan()
{
	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

	OnUninstallBack();

	m_nUniType = UNI_TYPE_QUICK;

	if (m_pUninstall)
		m_pUninstall->LinkRefresh(ksm::SIA_Quick, FALSE);

	m_pDlg->SetItemText( IDC_LNK_CUR_PAGE, BkString::Get(IDS_SOFTMGR_8066) );
	m_pDlg->SetItemVisible(IDC_UNINSTALL_BTM_DIV_ADVICE,FALSE);

	m_editUninstall.SetFocus();

}

void CBeikeSafeSoftmgrUIHandler::ShowUniDataPage()
{
	ShowQueryAndRefreshBtn( TRUE );

	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, FALSE);
	HideLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, TRUE );
	m_pDlg->SetItemVisible( IDC_UNINSTALL_SOFT_DLG, TRUE );
	m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFTLIST, TRUE);
}

void CBeikeSafeSoftmgrUIHandler::OnBtnUniStartMenu()
{
	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

	OnUninstallBack();

	m_nUniType = UNI_TYPE_START;

	if (m_pUninstall)
		m_pUninstall->LinkRefresh(ksm::SIA_Start, FALSE);

	m_pDlg->SetItemText( IDC_LNK_CUR_PAGE, BkString::Get(IDS_SOFTMGR_8067) );
	m_pDlg->SetItemVisible(IDC_UNINSTALL_BTM_DIV_ADVICE,FALSE);

	m_editUninstall.SetFocus();

}

void CBeikeSafeSoftmgrUIHandler::OnBtnUniTray()
{
	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

	OnUninstallBack();

	m_nUniType = UNI_TYPE_TRAY;

	if (m_pUninstall)
		m_pUninstall->LinkRefresh(ksm::SIA_Tray, FALSE);

	m_pDlg->SetItemText( IDC_LNK_CUR_PAGE, BkString::Get(IDS_SOFTMGR_8068) );
	m_pDlg->SetItemVisible(IDC_UNINSTALL_BTM_DIV_ADVICE,FALSE);

	m_editUninstall.SetFocus();

}


void CBeikeSafeSoftmgrUIHandler::OnBtnUniAll()
{
	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	m_pDlg->SetItemVisible( IDC_DLG_UNI_CONTENT, FALSE );

	OnUninstallBack();

	m_nUniType = UNI_TYPE_ALL;

	// 设置当前页为“全部软件”
	m_pDlg->SetItemText( IDC_LNK_CUR_PAGE, _T( "全部软件" ) );

	m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, FALSE );

	m_arrSoftUniDataSub.RemoveAll();
	for (int i=0; i < m_arrSoftUniData.GetSize(); i++)
	{
		SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniData[i];
		if (pInfo->bEnable == FALSE)
			m_arrSoftUniDataSub.Add(pInfo);
	}
	_ReloadTypeUni();
	_ReloadListUni();

	OnBtnSmrUniSortByName(); 
	ShowUniDataPage();

	m_pDlg->SetItemVisible(IDC_UNINSTALL_BTM_DIV_ADVICE,FALSE);

	m_editUninstall.SetFocus();

}

void CBeikeSafeSoftmgrUIHandler::FreshTotal()
{
	int nSize = m_arrSoftUniData.GetSize(); ;
	int	nCount = 0;
	for ( int i=0; i<nSize; i++ )
	{
		SOFT_UNI_INFO * pInfo = m_arrSoftUniData[i];
		if (pInfo && pInfo->bEnable == FALSE)
			nCount++;
	}

	WCHAR szSystemPath[MAX_PATH] = {0};
	GetSystemDirectory(szSystemPath, MAX_PATH - 1);
	CString strSysPath;
	strSysPath.Format(_T("%s"), szSystemPath);
	strSysPath = strSysPath.Left(3);
	CString	strFree;
	GetCurDiskFreeSpace( strSysPath, strFree );

	CString strT = strSysPath.GetAt(0);
	m_pDlg->FormatRichText(
		IDC_LBL_UNINSTALL_TOTAL, 
		BkString::Get(IDS_LBL_UNINSTALL_TOTAL_FMT), 
		nCount, strT, strFree );
}

unsigned CBeikeSafeSoftmgrUIHandler::ThreadProc(LPVOID lpVoid)
{
	threadParam *pParam = (threadParam*)lpVoid;
	CBeikeSafeSoftmgrUIHandler *pThis = (CBeikeSafeSoftmgrUIHandler*)pParam->pDlg;
	CSoftListItemData *pData = (CSoftListItemData *)pParam->pData;
	if(pData != NULL)
	{
		void *pTask = NULL;
		if (pThis->m_arrTask.Lookup(pData->m_strSoftID))
		{
			pTask = pThis->m_arrTask.Lookup(pData->m_strSoftID)->m_value;
		}

		if (pTask != NULL)
		{
			if (pParam->flag == 0)//继续
			{
				pData->m_bDownloading = TRUE;
				pData->m_bContinue = TRUE;
				pData->m_bPause = FALSE;
				pThis->m_pDTManager->ResumeTask(pTask);

				//刷新

				pThis->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);
			}
			else if (pParam->flag == 1)//暂停
			{
				pData->m_bDownloading = TRUE;
				pData->m_bPause = TRUE;
				pData->m_bContinue = FALSE;
				pThis->m_pDTManager->PauseTask(pTask);

				//刷新

				pThis->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);
			}
			else if (pParam->flag == 2)//取消
			{
				pData->m_bCancel = TRUE;
				pData->m_bDownLoad = FALSE;
				pData->m_bDownloading = FALSE;
				pData->m_bInstalling = FALSE;
				pData->m_bLinking = FALSE;
				/*pData->m_fSpeed = 0.0;
				pData->m_dwTime = 0;
				pData->m_dwStartTime = 0;
				pData->m_dwProgress = 0;*/

				pThis->m_pDTManager->PauseTask(pTask);

				//刷新

				pThis->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

			}
			else //删除
			{
				pThis->m_cs.Lock();
				pThis->m_arrTaskMap.RemoveKey(pTask);
				pThis->m_cs.Unlock();
				pThis->m_pDTManager->CancelTask(pTask);
				pTask = NULL;

				pThis->m_cs.Lock();
				pThis->m_arrTask.RemoveKey(pData->m_strSoftID);
				pThis->m_cs.Unlock();

				pData->m_bCancel = FALSE;
				pData->m_bDownLoad = FALSE;
				pData->m_bDownloading = FALSE;
				pData->m_bInstalling = FALSE;
				pData->m_bLinking = FALSE;
				pData->m_fSpeed = 0.0;
				pData->m_dwTime = 0;
				pData->m_dwStartTime = 0;
				pData->m_dwProgress = 0;

				//刷新

				pThis->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

				Sleep(1000);

				//删除临时文件 ——_kt、_kti
				CString strStore;
				BKSafeConfig::GetStoreDir(strStore);
				if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
				{
					strStore.Append(_T("\\")+ pData->m_strFileName + _T("_kt"));
				}
				else
				{
					strStore.Append(pData->m_strFileName + DOWNLOAD_TEMP_FILE_SUFFIX);
				}

				if (PathFileExists(strStore))
				{
					BOOL bFlag = DeleteFile(strStore);
				}
				strStore = strStore.Left(strStore.ReverseFind(_T('\\')) + 1);
				strStore.Append(pData->m_strFileName + DOWNLOAD_TEMP_FILE_INFO_SUFFIX);
				if (PathFileExists(strStore))
				{
					BOOL bFlag = DeleteFile(strStore);
				}
				strStore = strStore.Left(strStore.ReverseFind(_T('\\')) + 1);
				strStore.Append(pData->m_strFileName + _T("c"));
				if (PathFileExists(strStore))
				{
					BOOL bFlag = DeleteFile(strStore);
				}

			}
		}
	}

	if (pParam != NULL)
	{
		delete pParam;
		pParam = NULL;
	}
	return 0;
}

BOOL CBeikeSafeSoftmgrUIHandler::OnTabSoftMgrSelChange(int nTabItemIDOld, int nTabItemIDNew)
{
	//二级tab统计
	CString		strFormat;

	if (nTabItemIDNew == TAB_INDEX_TUIJIAN)
	{
		m_pDlg->SetItemVisible( IDC_REFRESH_PNG_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible( IDC_REFRESH_SOFTMGR, TRUE);

		if(!m_recommendLoaded)
		{
			m_recommendLoaded = TRUE;
			m_IESoftRecommend.Show2(L"http://baike.ijinshan.com/ksafew/site/hot/index.html", m_strErrorHtml);
		}

	}
	if (nTabItemIDNew == TAB_INDEX_BIBEI)//装机必备
	{
		m_necessUIHandler.m_editQuery.SetDefaultString(STR_DEFAULT_SOFTMGR); //设置缺省显示文字

		ShowQueryAndRefreshBtn(FALSE);


		m_necessUIHandler.m_editQuery.SetFocus();
	}
	else if (nTabItemIDNew == TAB_INDEX_DAQUAN)//软件大全
	{
		m_pDlg->SetItemVisible( IDG_QUERY_BAR_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible( IDC_REFRESH_PNG_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible( IDC_REFRESH_SOFTMGR, FALSE);

		m_edit.SetDefaultString(STR_DEFAULT_SOFTMGR); //设置缺省显示文字


		//m_list.SetFocus();
		m_edit.SetFocus();
	}
	else if (nTabItemIDNew == TAB_INDEX_UNINSTALL)//软件卸载
	{
		if ( m_pDlg->IsItemVisible(IDC_DLG_UNI_CONTENT) == TRUE )
		{
			ShowQueryAndRefreshBtn( TRUE );
		}
		else
		{
			ShowQueryAndRefreshBtn( FALSE );
		}

		m_editUninstall.SetDefaultString(STR_DEFAULT_SOFTMGR);   //设置缺省显示文字


		if (m_bInitUni == FALSE && m_pUninstall)
		{
			m_pUninstall->SetNotify(this);

			m_bInitUni = TRUE;

			CString	strDir;
			GetKSafeDirectory(strDir);
			m_pUninstall->Initialize(strDir);
		}

		//m_list.SetFocus();
		m_editUninstall.SetFocus();
	}
	else if (nTabItemIDNew == TAB_INDEX_UPDATE)//软件更新
	{
		m_pDlg->SetItemVisible( IDC_REFRESH_PNG_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible( IDC_REFRESH_SOFTMGR, TRUE);


		m_UpdateSoftList.SetFocus();
	}
	else if (nTabItemIDNew == TAB_INDEX_GAME)//热门游戏
	{
		ShowQueryAndRefreshBtn(FALSE);

		m_IEGame.Show2(L"http://wan.duba.net/db2010_games.shtml?utm_source=ws&utm_medium=client&utm_campaign=wan_tab", m_strErrorHtml);
		
	}
	else if (nTabItemIDNew == TAB_INDEX_PHONE) // 手机必备
	{
		m_pDlg->SetItemVisible( IDC_REFRESH_PNG_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible( IDC_REFRESH_SOFTMGR, TRUE);

		if(!m_phoneLoaded)
		{
			m_phoneLoaded = TRUE;
			CString strUrl;
			if( IsTestSoftPhone( strUrl ) )
				m_IEPhoneNess.Show2( strUrl, m_strErrorHtml );
			else
				m_IEPhoneNess.Show2(L"http://baike.ijinshan.com/ksafew/site/sj/index.html", m_strErrorHtml);
		}

	}


	m_nTab = nTabItemIDNew;


	return TRUE;
}

BOOL CBeikeSafeSoftmgrUIHandler::IsTestSoftPhone( CString& strUrl )
{
	BOOL bRet = FALSE;

	CString strFileName;
	HRESULT hRet = CAppPath::Instance().GetLeidianCfgPath(strFileName);

	if (FAILED(hRet))
		return FALSE;

	strFileName += L'\\';
	strFileName += TEXT("test_url.ini");

	::SetFileAttributes(strFileName, FILE_ATTRIBUTE_NORMAL);

	if( ::PathFileExists( strFileName) )
	{
		TCHAR buf[MAX_PATH] = {0};
		DWORD  dwRet = GetPrivateProfileString( TEXT("test"), TEXT("url"), TEXT(""), buf, MAX_PATH-1, strFileName );
		strUrl = buf;
		bRet = TRUE;
	}

	return bRet;
}

struct PARAM_CHECK_INST_SOFT
{
	CBeikeSafeSoftmgrUIHandler*	pThis;
	CSimpleArray<CString>		strIDs;
};
void CBeikeSafeSoftmgrUIHandler::CheckSoftInstallState(const CSimpleArray<CString>& strSoftIDS)
{


	PARAM_CHECK_INST_SOFT* pParam = new PARAM_CHECK_INST_SOFT;
	pParam->pThis	= this;
	pParam->strIDs	= strSoftIDS;

	_beginthread(_CheckSoftInstallStateProc,0,(LPVOID)pParam);
}

void CBeikeSafeSoftmgrUIHandler::_CheckSoftInstallStateProc( LPVOID lpParam )
{
	PARAM_CHECK_INST_SOFT*	pParam = (PARAM_CHECK_INST_SOFT*)lpParam;

	for ( int i=0; i < pParam->strIDs.GetSize(); i++)
	{
		CString&		strID = pParam->strIDs[i];
		pParam->pThis->m_pSoftChecker->CheckOneInstalled(strID, CheckSetupCallBack, pParam->pThis);
	}

	delete pParam;
}

unsigned CBeikeSafeSoftmgrUIHandler::InstallProc(LPVOID lpVoid)
{
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler*)lpVoid;

	installParam *pParam = NULL;
	pParam = pDlg->m_arrInstall.GetHead();
	while(pParam != NULL)
	{
		if (pParam->pData != NULL)
		{
			pParam->pData->m_bWaitInstall = FALSE;
			pParam->pData->m_bInstalling = TRUE;
			pParam->pData->m_bDaquan = TRUE;

			pDlg->_RefershItemBySoftIDWorkerThread(pParam->pData->m_strSoftID);

			if (pParam->pData->m_strPath != _T(""))
			{
				pDlg->m_pSoftChecker->Install( pParam->pData->m_strPath);
			}
			else
			{
				pDlg->m_pSoftChecker->Install( pParam->strPath);
			}

			Sleep(1000);
			int nRet = pDlg->m_pSoftChecker->CheckOneInstalled(pParam->pData->m_strSoftID, CheckSetupCallBack, pDlg);
			pDlg->m_cs.Lock();
			pParam->pData->m_bInstalling = FALSE;
			pParam->pData->m_bUpdate = pDlg->IsNeedUpdate(pParam->pData);
			pDlg->m_cs.Unlock();

			::SendMessage( pDlg->m_pDlg->m_hWnd,WM_SOFT_INSTALL_NOTIFY, (WPARAM)pParam->pData, nRet);
			::PostMessage( pDlg->m_pDlg->m_hWnd, WM_UPDATE_SOFT_COUNT, 0, 0 );


			pDlg->_RefershItemBySoftIDWorkerThread(pParam->pData->m_strSoftID);
		}
		pDlg->m_arrInstall.RemoveHead();
		if (pParam != NULL)
		{
			delete pParam ;
			pParam = NULL;
		}

		if (pDlg->m_arrInstall.GetCount() != 0)
		{
			pParam = pDlg->m_arrInstall.GetHead();
		}
		else
		{
			break;
		}
	}

	pDlg->UpdateCach();
	pDlg->m_hInstall = (HANDLE)-1;
	return 0;
}

void CBeikeSafeSoftmgrUIHandler::AddToNewInst(LPCTSTR pszSoftId )
{
	if( pszSoftId )
		m_arrNewDown.Add( pszSoftId );
}

unsigned CBeikeSafeSoftmgrUIHandler::ThreadIconProc(LPVOID lpVoid)
{
	// 加入对m_arrDownIconData访问的锁
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler *)lpVoid;

	pDlg->m_lockIconDown.Lock();
	CSoftListItemData *pData = pDlg->m_arrDownIconData.GetHead();
	while(pData != NULL)
	{
		pDlg->m_arrDownIconData.RemoveHead();
		pDlg->m_lockIconDown.Release();

		CString strIconPath;
		CString strTmpPath;
		if ( SUCCEEDED(pDlg->GetKSoftIconDir(strIconPath)) )
		{
			strTmpPath = strIconPath;
			strIconPath.Append(_T("\\") + pData->m_strSoftID + _T(".png"));
		}
		pData->m_nCount++;
		pDlg->m_pDownload->SetObserver(pDlg);
		pDlg->m_pDownload->SetUserData(pData);
		pDlg->m_pDownload->SetDownloadInfo(pData->m_strIconURL, strIconPath);
		pDlg->m_pDownload->Fetch();


		pDlg->m_lockIconDown.Lock();
		if (pDlg->m_arrDownIconData.GetCount() > 0)
		{
			pData = pDlg->m_arrDownIconData.GetHead();
		}
		else
		{
			break;
		}
	}

	pDlg->m_hDownIcon = (HANDLE)-1;
	pDlg->m_lockIconDown.Release();

	return 0; 
}

unsigned CBeikeSafeSoftmgrUIHandler::ThreadDownloadProc(LPVOID lpVoid)
{
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler *)lpVoid;

	CSoftListItemData *pData = pDlg->m_arrDownload.GetHead();
	while (pData != NULL)
	{
		pData->m_bWaitDownload = FALSE;

		pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
		{
			strStore.Append(_T("\\")+ pData->m_strFileName);
		}
		else
		{
			strStore.Append(pData->m_strFileName);
		}

		pData->m_strPath = strStore;
		CAtlArray<CString> *urls = new CAtlArray<CString>;
		CString strURL = pData->m_strDownURL;
		pData->m_nURLCount = 0;
		int n = strURL.Find(_T(';'));			
		while (n != -1)
		{
			pData->m_nURLCount++;
			urls->Add(strURL.Left(n));
			strURL = strURL.Right(strURL.GetLength() - n - 1);
			n = strURL.Find(_T(';'));
		}
		if (n == -1 && strURL != _T(""))
		{
			pData->m_nURLCount++;
			urls->Add(strURL);
		}

		pDlg->InitDownloadDir();

		void *tk = pDlg->m_pDTManager->NewTask(urls, pData->m_strMD5, strStore);
		pDlg->m_cs.Lock();
		pDlg->m_arrTask.SetAt(pData->m_strSoftID, tk);
		pDlg->m_arrTaskMap.SetAt(tk, pData->m_strSoftID);
		pDlg->m_cs.Unlock();

		pData->m_dwStartTime = GetTickCount();
		pData->m_bDownLoad = FALSE;
		pData->m_bPause = FALSE;
		pData->m_bContinue = TRUE;
		pData->m_bDownloading = TRUE;
		pData->m_bLinking = TRUE;


		pDlg->_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

		pDlg->m_cs.Lock();
		pDlg->m_arrDownload.RemoveHead();
		pDlg->m_cs.Unlock();

		while(TRUE)
		{
			int nCount = 0;
			for (int k = 0; k < pDlg->m_arrData.GetSize(); k++)
			{
				CSoftListItemData *pTaskData = pDlg->m_arrData[k];
				if (pTaskData != NULL &&  pTaskData->m_bLinking && !pTaskData->m_bLinkFailed )
				{
					nCount ++;
				}
			}

			if (nCount < 2)
			{
				break;
			}
		}

		pData->m_bBatchDown = FALSE;

		if (pDlg->m_arrDownload.GetCount() > 0)
		{
			pData = pDlg->m_arrDownload.GetHead();
		}
		else
		{
			break;
		}
	}

	pDlg->m_hDownload = (HANDLE)-1;

	return 0;	
}

unsigned __stdcall 
CBeikeSafeSoftmgrUIHandler::ThreadCheckInstallProc(LPVOID lpVoid) //检测软件安装
{
	CBeikeSafeSoftmgrUIHandler* pDlg = (CBeikeSafeSoftmgrUIHandler*)lpVoid;
	if ( !pDlg ) return -1;

	BOOL bChecked = FALSE;
	// 加载update库和卸载补充信息库
	CString	strSoftDataDir;
	CString strSetupXML;
	if (SUCCEEDED(pDlg->GetKSoftDataDir(strSoftDataDir)))
	{
		strSetupXML = strSoftDataDir + _T("\\softmgrup.dat");
	}

	// 加载update库***
	pDlg->m_pSoftChecker->Load(strSetupXML);

	::SetEvent(pDlg->m_hSignLoadFinish);

	// 加载安装软件缓存，缓存中为软件的id
	CString  strCach;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCach)) )
	{
		strCach.Append(_T("\\ksoft_setuped"));
	}

	// 若安装缓存存在，根据缓存文件中的id设置对应软件的状态并刷新右侧列表
	if(PathFileExists(strCach)) 
	{
		pDlg->GetSoftStateFromCach(strCach);
	}
	else
	{
		// 检测软件是否已经安装
		pDlg->m_pSoftChecker->CheckAllInstalled(CheckSetupCallBack,pDlg); 
		bChecked = TRUE;
	}

	if (!bChecked)
	{
		pDlg->m_pSoftChecker->CheckAllInstalled(CheckSetupCallBack,pDlg); // 检测软件是否已经安装
	}

	::PostMessage(pDlg->m_pDlg->m_hWnd, MSG_APP_LOAD_UPDATE_REAL, 0, 0);

	//更新安装缓存信息(m_arrCachSetup):  刷新列表中错误的已安装软件的信息，有些软件已经卸载，缓存中标记为已安装
	for ( int k = 0; k < pDlg->m_arrCachSetup.GetSize(); k++)
	{
		CSoftListItemData *pItemData = NULL;

		pItemData = pDlg->m_arrCachSetup[k];
		if ( pItemData != NULL)
		{
			if ( -1 == pDlg->m_arrCheckSetup.Find( pItemData ))
			{
				pItemData->m_bSetup = FALSE;

				CString strStore;
				BKSafeConfig::GetStoreDir(strStore);
				if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
				{
					strStore.Append(_T("\\")+ pItemData->m_strFileName);
				}
				else
				{
					strStore.Append(pItemData->m_strFileName);
				}

				if (PathFileExists(strStore))
				{
					pItemData->m_strPath = strStore;
					pItemData->m_dwProgress = 100;
					pItemData->m_bDownloading = TRUE;
					pItemData->m_bDownLoad = TRUE;

					if ((pItemData->m_attri&SA_Green) == SA_Green)
					{
						pItemData->m_bSetup = TRUE;
					}
				}
			}
		}
	}

	return 0;
}

unsigned __stdcall 
CBeikeSafeSoftmgrUIHandler::ThreadUpdateSoftInfoProc(LPVOID lpVoid) //更新所有软件信息
{
	CBeikeSafeSoftmgrUIHandler* _pcThis = (CBeikeSafeSoftmgrUIHandler*)lpVoid;
	if ( !_pcThis ) return -1;

	CString strIconPath;
	_pcThis->GetKSoftIconDir(strIconPath);

	CString strStore;
	BKSafeConfig::GetStoreDir(strStore);

	CString strTmpPath;
	CString _strStore;

	for ( unsigned int _nIndex = 0; _nIndex < _pcThis->m_arrData.GetSize(); ++_nIndex )
	{
		CSoftListItemData* pData = _pcThis->m_arrData[_nIndex];
		if ( !pData ) continue;

		strTmpPath = strIconPath;
		strTmpPath.Append(_T("\\") + pData->m_strSoftID + _T(".png"));

		if (PathFileExists(strTmpPath))
		{
			pData->m_pImage = Gdiplus::Image::FromFile(strTmpPath,TRUE);
			pData->m_bIcon = TRUE;
			pData->m_bChangeIcon = FALSE;

			if( !pData->m_strCrc.IsEmpty() )
			{
				char szFilePath[MAX_PATH] = {0};
				::WideCharToMultiByte( CP_ACP, 0, strTmpPath, strTmpPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
				FILE *pFile = NULL;

				pFile = fopen( szFilePath, "rb");
				if ( pFile )
				{
					DWORD dwCrc = 0;
					dwCrc = CalcFileCrc( pFile );
					CString strTmpCrc;
					strTmpCrc.Format(_T("%08X"), dwCrc);
					if ( pData->m_strCrc != strTmpCrc )
					{
						pData->m_bIcon = FALSE;
						pData->m_bChangeIcon = TRUE;
					}
				}
				fclose( pFile );
			}
		}
		else
		{
			pData->m_bIcon = FALSE;
			pData->m_pImage = _pcThis->m_pImge;
		}


		_strStore = strStore;
		if (_strStore.ReverseFind(_T('\\')) < (_strStore.GetLength() - 1))
		{
			_strStore.Append(_T("\\")+ pData->m_strFileName);
		}
		else
		{
			_strStore.Append(pData->m_strFileName);
		}

		if (PathFileExists(_strStore) && pData->m_strFileName.GetLength() != 0)
		{
			pData->m_strPath = _strStore;
			pData->m_dwProgress = 100;
			pData->m_bDownloading = TRUE;
			pData->m_bDownLoad = TRUE;

			CFileTime timeright;
			WIN32_FILE_ATTRIBUTE_DATA	data;
			GetFileAttributesEx( _strStore, GetFileExInfoStandard, &data );
			timeright.dwHighDateTime = data.ftCreationTime.dwHighDateTime;
			timeright.dwLowDateTime  = data.ftCreationTime.dwLowDateTime;
			pData->m_llDownloadTime = ( LONGLONG )(timeright.GetTime());
		}
		else
		{
			_strStore.Append(L"_kt");
			if (PathFileExists(_strStore))
			{
				CFileTime timeright;
				WIN32_FILE_ATTRIBUTE_DATA	data;
				GetFileAttributesEx( _strStore, GetFileExInfoStandard, &data );
				timeright.dwHighDateTime = data.ftCreationTime.dwHighDateTime;
				timeright.dwLowDateTime  = data.ftCreationTime.dwLowDateTime;
				pData->m_llDownloadTime = ( LONGLONG )(timeright.GetTime());
			}
		}

		// 注释掉，此时不更新是否安装
		//		pData->m_bSetup = FALSE;

		if (_pcThis->m_arrCachInfo.Lookup(pData->m_strSoftID) != NULL)
		{
			softInfo *pSoftInfo = NULL;
			pSoftInfo = _pcThis->m_arrCachInfo.Lookup( pData->m_strSoftID )->m_value;
			if ( pSoftInfo != NULL )
			{
				if (pSoftInfo->nState == 1)
				{
					pData->m_bDownLoad = FALSE;
					pData->m_bDownloading = TRUE;
					pData->m_bPause = TRUE;
					pData->m_bContinue = FALSE;
					pData->m_bCancel = FALSE;
					pData->m_dwProgress = pSoftInfo->nProgress;
				}
				else if ( pSoftInfo->nState == 2 )
				{
					pData->m_bDownloading = FALSE;
					pData->m_bDownLoad = FALSE;
					pData->m_bContinue = FALSE;
					pData->m_bPause = FALSE;
					pData->m_bCancel = TRUE;
					pData->m_dwProgress = pSoftInfo->nProgress;
				}
			}
		}
	}

	_pcThis->DeleteFileAfterTime();
	::SendMessage( _pcThis->m_pDlg->m_hWnd, WM_BOTTOM_REFRESH, 0, 0 );

	return 0;
}

unsigned __stdcall 
CBeikeSafeSoftmgrUIHandler::ThreadCheckUpateProc(LPVOID lpVoid)	//检测升级
{
	CBeikeSafeSoftmgrUIHandler* _pcThis = (CBeikeSafeSoftmgrUIHandler*)lpVoid;
	if ( !_pcThis ) return -1;

	//先读缓存
	CString  _strCache;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(_strCache)) )
	{
		_strCache.Append(_T("\\ksoft_upcache"));
		BOOL bExist = PathFileExists(_strCache);
		if(bExist) 
		{
			_pcThis->GetUpdateStateFromCache(_strCache);

			//升级列表缓存信息展示
			::SendMessage( _pcThis->m_pDlg->m_hWnd, MSG_APP_LOAD_UPDATE_CACHE, 0, 0 );
		}
	}

	return 0;
}

void CBeikeSafeSoftmgrUIHandler::_UpdateSoftInfo()
{
	if ( m_hUpdSoftInfoThread ) return;
	m_hUpdSoftInfoThread = (HANDLE)::_beginthreadex(NULL, 0, ThreadUpdateSoftInfoProc, this, 0, NULL);

}

void CBeikeSafeSoftmgrUIHandler::_CheckSoftInstall()
{
	m_hSignLoadFinish = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if ( m_hCheckInstallThread ) return;
	m_hCheckInstallThread = (HANDLE)::_beginthreadex(NULL, 0, ThreadCheckInstallProc, this, 0, NULL);
}

void CBeikeSafeSoftmgrUIHandler::_CheckSoftUpdate()
{
	if ( m_hCheckUpdateThread ) return;

	m_hCheckUpdateThread = (HANDLE)::_beginthreadex(NULL, 0, ThreadCheckUpateProc, this, 0, NULL);
}

static inline void ver4ws(const TCHAR* ver,DWORD& dwMS,DWORD& dwLS)
{
	static const size_t size = 20+1+20+1+20+1+10+1;
	TCHAR buff[size]={0};

	wcscpy_s(buff,size,ver);

	TCHAR* next_tok = NULL;
	TCHAR* tok=wcstok_s( buff, _T("."), &next_tok );

	WORD wVer[4] = {0};
	for ( int i = 0 ; tok ; i++ )
	{
		wVer[i]=_wtoi(tok);
		tok=wcstok_s( NULL, _T("."), &next_tok );
	}
	dwMS=(DWORD)MAKELONG( wVer[1], wVer[0] );
	dwLS=(DWORD)MAKELONG( wVer[3], wVer[2] );
}

BOOL CBeikeSafeSoftmgrUIHandler::IsNeedUpdate(CSoftListItemData* pData)
{
	BOOL _bNeedUpdate = FALSE;

	do 
	{
		if ( pData->m_strNewVersion.IsEmpty() || pData->m_strVersion.IsEmpty())
			break;

		DWORD dwNewMS = 0, dwNewLS = 0, dwCurMS = 0, dwCurLS = 0;
		ver4ws(pData->m_strNewVersion, dwNewMS, dwNewLS);
		ver4ws(pData->m_strVersion, dwCurMS, dwCurLS);

		if((dwNewMS > dwCurMS) || ((dwNewMS == dwCurMS) && (dwNewLS > dwCurLS)))
		{
			_bNeedUpdate = TRUE;
		}
	} while (FALSE);

	return _bNeedUpdate;
}

void CBeikeSafeSoftmgrUIHandler::OnEditEnterMsg(DWORD nEditId)
{
	if (nEditId==IDC_SOFT_UNISTALL_EDT_QEURY_EDIT)
		OnQueryUninstall();
	else if (nEditId==IDC_EDIT_SOFTMGR)
		OnQuerySoft();
}

void CBeikeSafeSoftmgrUIHandler::ConvertMapToSoftItemData( CSoftListItemData* pData, CAtlMap<CString,CString>& soft )
{
	CString strTmp;
	int nTmp;

	pData->m_strSoftID		= soft[_T("softid")];	//软件ID
	pData->m_strName		= soft[L"softname"];		//软件名称
	pData->m_strFileName	= soft[_T("filename")];	//软件下载文件名
	pData->m_strDescription = soft[_T("brief")];	//描述
	pData->m_strOSName		= soft[_T("osname")];		//支持的操作系统
	pData->m_strVersion		= soft[_T("version")];		//版本
	pData->m_strURL			= soft[_T("infourl")];			//详情页URL
	pData->m_strMD5			= soft[_T("filehash")];			//MD5
	strTmp					= soft[_T("filesize")]; //文件大小
	nTmp					= _wtoi( strTmp );
	pData->m_dwSize			= nTmp;
	pData->m_strSize		= CalcStrFormatByteSize( nTmp );
	strTmp					= soft[_T("plugin")];	//插件
	nTmp					= _wtoi(strTmp);
	pData->m_bPlug			= (nTmp==1)?TRUE:FALSE;
	strTmp					= soft[_T("free")];	//收费
	nTmp					= _wtoi(strTmp);
	pData->m_bCharge		= (nTmp==1)?TRUE:FALSE;
	strTmp					= soft[_T("attr")];	// 属性
	pData->m_attri			= static_cast<ULONG>(_wtol(strTmp));
	pData->m_strDownURL		= soft[_T("downurl")];	//下载地址
	pData->m_strType		= soft[_T("typename")];	//软件类型
	pData->m_strTypeID		= soft[_T("typeid")];    //软件类型ID
	pData->m_strTypeShort	= GetTypeUI(pData->m_strType);
	pData->m_strIconURL		= soft[_T("softlogo")];	//软件图标URL
	pData->m_strCrc			= soft[_T("logocrc")];//图片crc
	pData->m_strOrder		= soft[_T("ar")];
}

void CBeikeSafeSoftmgrUIHandler::LoadNecessData()
{
	ATLASSERT(m_pSoftMgr != NULL);

	if (m_pSoftMgr != NULL)
	{
		CAtlMap<CString,CString> soft;			
		void* pos = m_pSoftMgr->GetAllNecessSoft();
		while(0 == m_pSoftMgr->GetNextSoft(pos, GetInfoUseMap,&soft))
		{
			CSoftListItemData *pData = new CSoftListItemData;

			pData->m_strSoftID = soft[_T("softid")];	//软件ID
			//查询软件是否已经load
			if (m_arrDataMap.Lookup(pData->m_strSoftID) != NULL)
			{
				delete pData;
				pData = NULL;

				continue;
			}

			ConvertMapToSoftItemData(pData,soft);

			//添加软件类型ID到类型名称的映射
			if(m_arrTypeMap.Lookup(pData->m_strTypeID) == NULL)
			{
				m_arrTypeMap.SetAt(pData->m_strTypeID, pData->m_strType);
			}

			// 添加到软件大全软件列表中
			m_arrData.Add(pData);
			m_ItemData.AddData(pData);
			m_arrDataMap.SetAt(pData->m_strSoftID, pData);
		}
		m_pSoftMgr->FinalizeGet(pos);
	}
}


void CBeikeSafeSoftmgrUIHandler::LoadAllSoftData()
{
	ATLASSERT(m_pSoftMgr != NULL);

	if (m_pSoftMgr != NULL)
	{
		void *pos;

		pos = m_pSoftMgr->GetAllSoft(_T(""), L"ar");

		int nCount = 0;
		CAtlMap<CString,CString> soft;

		while(0 == m_pSoftMgr->GetNextSoft(pos, GetInfoUseMap,&soft))
		{
			CSoftListItemData *pData = new CSoftListItemData;

			pData->m_strSoftID = soft[_T("softid")];	//软件ID
			//查询软件是否已经load
			if (m_arrDataMap.Lookup(pData->m_strSoftID) != NULL)
			{
				delete pData;
				pData = NULL;

				pData = m_arrDataMap.Lookup(soft[_T("softid")])->m_value;
				if (pData)
					pData->m_strOrder = soft[_T("ar")];

				continue;
			}

			ConvertMapToSoftItemData(pData,soft);

			//添加软件类型ID到类型名称的映射
			if(m_arrTypeMap.Lookup(pData->m_strTypeID) == NULL)
			{
				m_arrTypeMap.SetAt(pData->m_strTypeID, pData->m_strType);
			}

			// 添加到软件大全软件列表中
			m_arrData.Add(pData);
			m_ItemData.AddData(pData);
			m_arrDataMap.SetAt(pData->m_strSoftID, pData);
		}

		m_pSoftMgr->FinalizeGet(pos);

		//排序
		m_ItemData.SortData(CSoftItemData::SORT_BY_ID);
	}
}

CSoftListItemData* CBeikeSafeSoftmgrUIHandler::GetDataBySoftID(CString strID)
{
	CSoftListItemData *pData = NULL;

	if(m_arrDataMap.Lookup(strID))
		pData = m_arrDataMap.Lookup(strID)->m_value;
	else
		return NULL;

	if(pData != NULL)
	{
		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
			strStore.Append(_T("\\")+ pData->m_strFileName);
		else
			strStore.Append(pData->m_strFileName);
		if (PathFileExists(strStore))
		{
			pData->m_strPath = strStore;
			if ((pData->m_attri&SA_Green) == SA_Green)
				pData->m_bSetup = TRUE;
			if (!pData->m_bSetup)
			{
				pData->m_dwProgress = 100;
				pData->m_bDownloading = TRUE;
				pData->m_bDownLoad = TRUE;
			}
		}
	}

	return pData;
}

BOOL CBeikeSafeSoftmgrUIHandler::IsDataFilterByCheck(CSoftListItemData* pData, BOOL bFree, BOOL bPlug)
{
	BOOL bFilter = TRUE;

	BOOL bCanShow = FALSE;
	BOOL bFreeChecked = bFree;
	BOOL bPlugChecked = bPlug;
	if( bPlugChecked && bFreeChecked )
	{
		if ( !pData->m_bCharge && !pData->m_bPlug )
			bFilter = FALSE;
	}
	else if ( bPlugChecked && !bFreeChecked )
	{
		if ( !pData->m_bPlug )
			bFilter = FALSE;
	}
	else if ( !bPlugChecked && bFreeChecked )
	{
		if ( !pData->m_bCharge )
			bFilter = FALSE;
	}
	else
	{
		bFilter = FALSE;
	}

	return bFilter;
}

BOOL CBeikeSafeSoftmgrUIHandler::ShowDataToList(CAtlList<CSoftListItemData*>& arr, BOOL bShowTypeInfo /*= FALSE*/)
{
	BOOL bQueryMark = FALSE;
	CSoftListItemData *pDataTemp = NULL;

	m_list.SetShowType(bShowTypeInfo);
	POSITION posTemp =  arr.GetHeadPosition();
	while (posTemp)
	{
		pDataTemp = arr.GetAt(posTemp);
		if (pDataTemp)
		{
			pDataTemp->m_dwID = m_list.GetItemCount();
			m_list.InsertItem(m_list.GetItemCount(), _T(""));
			m_list.SetItemDataEx(m_list.GetItemCount(), (DWORD_PTR)pDataTemp);
			m_list.SetPaintItem(TRUE);

			if (pDataTemp->m_fMark == 0 && m_pInfoQuery)
			{
				int nTmp = _wtoi( pDataTemp->m_strSoftID );
				m_pInfoQuery->AddSoft(nTmp);
				bQueryMark = TRUE;
			}
		}

		arr.GetNext(posTemp);
	}
	if (bQueryMark && m_pInfoQuery)
		m_pInfoQuery->Query();

	return TRUE;
}

//我要完善
void CBeikeSafeSoftmgrUIHandler::OnFreeBack()
{
	CString strURL;
	if (m_nTab == TAB_INDEX_DAQUAN)
	{
		CSoftListItemData *pData = m_list.GetItemDataEx(m_nDown);
		if (pData != NULL)
		{
			strURL.Format(_T("http://baike.ijinshan.com/ksafe/baike/customer_supplementsoft.html?name=%s"),
				url_encode.URLEncode(pData->m_strName));
		}
	}
	else
	{
		SOFT_UNI_INFO *pUninstallInfo = m_pSelSoftUniData;
		if (pUninstallInfo != NULL)
		{
			strURL.Format(_T("http://baike.ijinshan.com/ksafe/baike/customer_supplementsoft.html?name=%s"),
				url_encode.URLEncode(pUninstallInfo->strCaption));
		}
	}
	ShellExecute( NULL, _T("open"), strURL, NULL, NULL , SW_SHOW);
}

void CBeikeSafeSoftmgrUIHandler::OnReStart()
{
	if (m_pDlg)
	{
		m_pDlg->OnBkBtnClose();

		HWND hWnd = NULL;
		hWnd = FindWindow(L"{5469F950-888A-4bc1-B0B4-72F0159D7ACD}", NULL);
		if (hWnd)
			::PostMessage(hWnd, (WM_USER + 111), 0, 0);
	}
}
void CBeikeSafeSoftmgrUIHandler::OnIgnoreReStartTip()
{
	OnCloseReStartTip();
}
void CBeikeSafeSoftmgrUIHandler::OnCloseReStartTip()
{
	if ( m_pDlg->IsItemVisible(IDC_SOFT_NECESS_LEFT_VER_TIP) == TRUE )
	{
		m_pDlg->SetItemVisible(IDC_SOFT_NECESS_LEFT_VER_TIP, FALSE);
		m_pDlg->SetItemAttribute(IDC_SOFT_NECESS_LEFT_VIEW, "pos", "118,0,-0,-0");
	}

	if ( m_pDlg->IsItemVisible(IDC_SOFT_DAQUAN_LEFT_VER_TIP) == TRUE )
	{
		m_pDlg->SetItemVisible(IDC_SOFT_DAQUAN_LEFT_VER_TIP, FALSE);
		m_pDlg->SetItemAttribute(IDC_SOFT_DAQUAN_LEFT_VIEW, "pos", "118,0,-0,-31");
	}

	if ( m_pDlg->IsItemVisible(IDC_SOFT_UPDATE_LEFT_VER_TIP) == TRUE )
	{
		m_pDlg->SetItemVisible(IDC_SOFT_UPDATE_LEFT_VER_TIP, FALSE);
		m_pDlg->SetItemAttribute(IDC_SOFT_UPDATE_LEFT_VIEW, "pos", "0,31,-0,-31");
	}
}

void CBeikeSafeSoftmgrUIHandler::GetConfig()
{
	//	_beginthreadex(NULL,0, ThreadIniProc, this, NULL, NULL);
}

unsigned CBeikeSafeSoftmgrUIHandler::ThreadIniProc(LPVOID lpVoid)
{
	CBeikeSafeSoftmgrUIHandler *pDlg = (CBeikeSafeSoftmgrUIHandler *)lpVoid;
	CString strPath;
	if ( SUCCEEDED(pDlg->GetKSoftDirectory(strPath)) )
	{
		strPath.Append(_T("\\uninsthelp.ini"));
	}
	pDlg->m_pDownload->SetDownloadInfo(STR_UNINSTALL_INI, strPath);
	pDlg->m_pDownload->Fetch();
	return 0;
}

void CBeikeSafeSoftmgrUIHandler::CheckDownloadState()
{
	CString strSoftId;
	for(int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = NULL;
		pData = m_arrData[i];

		if(pData != NULL)
		{
			if(pData->m_bLinking)
			{
				DWORD time = GetTickCount() - pData->m_dwStartTime;
				DWORD nCount = (DWORD)pData->m_nURLCount;
				if (time > 60*1000*(nCount*2-1) || time == 60*1000*(nCount*2-1))
				{
					if (!pData->m_bLinkFailed)
					{
						pData->m_bLinkFailed = TRUE;
						pData->m_dwProgress = 100;

						_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);

						//连接服务器失败清理工作
						void *tk = NULL;
						if (m_arrTask.Lookup(pData->m_strSoftID) != NULL)
						{
							tk = m_arrTask.Lookup(pData->m_strSoftID)->m_value;
							if( tk != NULL)
							{
								m_cs.Lock();
								m_arrTaskMap.RemoveKey(tk);
								m_arrTask.RemoveKey(pData->m_strSoftID);
								m_cs.Unlock();

								m_pDTManager->CancelTask(tk);
								tk = NULL;
							}
						}

					}					
				}
				else
				{
					pData->m_dwProgress = (DWORD)(time / (600 * (nCount*2-1)));
				}
			}
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::_FreebackFromList( CSoftListItemData* pData,SoftActionFrom nFrom)
{
	CString strFreeback = L"http://bbs.ijinshan.com/forum-18-1.html";
	::ShellExecute( NULL, _T("open"), strFreeback, NULL, NULL , SW_SHOW);
}
void CBeikeSafeSoftmgrUIHandler::OnFreebackFromList(DWORD dwIndex)
{
	CSoftListItemData *pData = m_list.GetItemDataEx(dwIndex);
	if (pData != NULL)
	{
		_FreebackFromList(pData);
	}
}

void CBeikeSafeSoftmgrUIHandler::_TautilogyFromList( CSoftListItemData* pData,SoftActionFrom nFrom)
{
	pData->m_bLinkFailed = FALSE;
	pData->m_bLinking = TRUE;

	pData->m_bDownloading = FALSE;
	pData->m_bDownLoad = FALSE;
	_downLoadSoft(pData,nFrom);
}

void CBeikeSafeSoftmgrUIHandler::OnSwitchPage(DWORD dwPage)
{
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_NAME, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_SIZE, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_TIME, "class","linkblue" );

	BOOL bFreeChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_FREESOFT);
	BOOL bPlugChecked = m_pDlg->GetItemCheck(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT);
	m_list.ClearAllItemData();
	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );

	int nPageIndex = dwPage;
	int nCanInsert = -1;
	int nPageCount = PAGE_ITEM_MAX_COUNT;
	int nMin = nPageCount*(nPageIndex-1);
	int nMax = nPageCount*nPageIndex;

	CSimpleArray<CSoftListItemData*> arrFilter;
	CAtlList<CSoftListItemData*> lstOrder;

	for (int i = 0; i < m_arrDataSub.GetSize(); i++)
	{
		BOOL bFilter = IsDataFilterByCheck(m_arrDataSub[i], bFreeChecked, bPlugChecked);
		if (!bFilter)
			nCanInsert++;

		if (nCanInsert != -1 && nCanInsert >= nMin && nCanInsert < nMax)
		{
			if (!bFilter)
			{
				CSoftListItemData *pData = NULL;
				pData = m_arrDataSub[i];
				if (pData)
					arrFilter.Add(pData);
			}
		}
	}

	//排序
	_SortArrayByOrder(arrFilter, lstOrder);

	//显示
	ShowDataToList(lstOrder);

	if (nCanInsert > PAGE_ITEM_MAX_COUNT)
	{
		m_list.AppendPageItem(nPageIndex, nCanInsert);
		m_list.UpdateBtnRect();
	}

	CRect rcWnd;
	m_list.GetWindowRect(&rcWnd);
	OnListReSize(rcWnd);
}

void CBeikeSafeSoftmgrUIHandler::OnDaquanSoftMark(DWORD dwIndex)
{
	CSoftListItemData *pData = (CSoftListItemData *)m_list.GetItemDataEx(dwIndex);
	if (pData)
	{
		CBkMarkDlg	dlg( this, pData->m_strSoftID, pData->m_strName );
		dlg.DoModal(GetActiveWindow());

		_QuerySoftMark(pData->m_strSoftID);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnViewNewInfo(CSoftListItemData* pData)
{
	if (pData)
	{
		CBkNewInfoDlg	dlg( this, pData->m_strSoftID, pData->m_strName );
		UINT_PTR uRet = dlg.DoModal(GetActiveWindow());
		if (IDOK == uRet)
			_downLoadSoft(pData,ACT_SOFT_FROM_UPDATE);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnBiBeiSoftMark(CSoftListItemData* pData)
{
	if (pData)
	{
		CBkMarkDlg	dlg( this, pData->m_strSoftID, pData->m_strName );
		dlg.DoModal(GetActiveWindow());

		_QuerySoftMark(pData->m_strSoftID);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnTautilogyFromList(DWORD dwIndex)
{
	CSoftListItemData *pData = (CSoftListItemData *)m_list.GetItemDataEx(dwIndex);
	if (pData)
	{
		_TautilogyFromList(pData);
	}
}

HRESULT CBeikeSafeSoftmgrUIHandler::GetKSafeDirectory(CString &strDir)
{
	HRESULT hr = SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strDir));
	if ( hr )
	{
		strDir = strDir.Left(strDir.ReverseFind(_T('\\')));
	}
	return hr;
}

HRESULT CBeikeSafeSoftmgrUIHandler::GetKSoftDirectory(CString &strDir)
{
	HRESULT hr = SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strDir));
	if ( hr )
	{
		strDir = strDir.Left(strDir.ReverseFind(_T('\\')) + 1);
		strDir.Append(_T("ksoft"));
	}
	return hr;
}

HRESULT CBeikeSafeSoftmgrUIHandler::GetKSoftDataDir(CString &strDir)
{
	HRESULT hr = GetKSoftDirectory(strDir);
	if ( hr )
	{
		strDir.Append(_T("\\data"));
	}
	return hr;
}

HRESULT CBeikeSafeSoftmgrUIHandler::GetKSoftIconDir(CString &strDir)
{
	HRESULT hr = GetKSoftDirectory(strDir);
	if ( hr )
	{
		strDir.Append(_T("\\icon"));

		if (!PathFileExists(strDir))
		{
			hr = CAppPath::Instance().CreateFullPath( strDir );
		}

	}
	return hr;
}

CString CBeikeSafeSoftmgrUIHandler::GetSoftLibraryVersion()
{
	return GetLibVersion(_T("\\softmgr.dat"));
}

CString CBeikeSafeSoftmgrUIHandler::GetUninstLibVersion()
{
	return GetLibVersion(_T("\\softuninst2.dat"));
}

CString CBeikeSafeSoftmgrUIHandler::GetLibVersion(LPCWSTR pLibPath)
{
	CString strVersion;
	CDataFileLoader	loader;
	BkDatLibHeader new_h;

	CString strPath;
	if (SUCCEEDED(GetKSoftDataDir(strPath)))
	{
		strPath.Append(pLibPath);
		if( loader.GetLibDatHeader(strPath,new_h) )
		{
			strVersion.Format(_T("%d.%d.%d.%d"), HIWORD(new_h.llVersion.HighPart),LOWORD(new_h.llVersion.HighPart),
				HIWORD(new_h.llVersion.LowPart), LOWORD(new_h.llVersion.LowPart));
		}
	}

	return strVersion;
}

void CBeikeSafeSoftmgrUIHandler::InitDownloadDir()
{
	CString strPath;
	BKSafeConfig::GetStoreDir(strPath);

	if (!PathFileExists(strPath))
	{
		CAppPath::Instance().CreateFullPath( strPath );
	}
}

void CBeikeSafeSoftmgrUIHandler::OnBackFromQuery()
{
	OnRefresh();

	m_edit.SetWindowText(L"");
	m_edit.SetFocus();

	m_bQuery = FALSE;
}

void CBeikeSafeSoftmgrUIHandler::OnBackFromInsQuery()
{
	OnRefresh();

	if (m_nTab == TAB_INDEX_DAQUAN)
		m_edit.SetFocus();
	else if (m_nTab == TAB_INDEX_UNINSTALL)
		m_editUninstall.SetFocus();

	m_bQuery = FALSE;
}


void CBeikeSafeSoftmgrUIHandler::OnSize(UINT nType, CSize size)
{
	if (m_pDlg->IsItemVisible(IDC_SMR_UNI_HEADER_ARROW))
	{
		CStringA strPos;
		switch(m_nUniLastClickHeaderIndex)
		{
		case 0:strPos = "-182,7,-0,-0";break;
		case 1:strPos = "-112,7,-0,-0";break;
		case 2:strPos = "-15,7,-0,-0";break;
		}
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "pos", strPos);
	}

	if (m_pDlg->IsItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW))
	{
		CStringA strPos;
		switch(m_nDaquanLastClickHeaderIndex)
		{
		case 0:strPos = "-158,7,-0,-0";break;
		case 1:strPos = "-88,7,-0,-0";break;
		case 2:strPos = "-20,7,-0,-0";break;
		}
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "pos", strPos);
	}

	SetDownloadDetail();
}

void CBeikeSafeSoftmgrUIHandler::OnTransferFromQuery()
{
	CTypeListItemData *pTypeData = m_SoftTypeList.GetItemDataX(1);
	m_SoftTypeList.SetCurSel(1);
	OnClick(IDC_TYPES_LEFT_SOFTMGR, pTypeData);
}

void CBeikeSafeSoftmgrUIHandler::OnTransferUninstall()
{
	OnBackFromInsQuery();
}

void CBeikeSafeSoftmgrUIHandler::Uninit()
{	
	if (m_bInitInstProxy == TRUE)
	{
		m_softInstEng.Uninitialize();
		m_bInitInstProxy = FALSE;
	}

	if (m_pSoftChecker)
	{
		m_pSoftChecker->MakeUpdateCache(); //建立升级缓存
	}

	SetDownloadCachInfo();
	UpdateCach();
	SaveUpdateIgnoreList();
	SaveUpdatePointOutInfo();

	if (m_arrTask.GetCount() != 0)
	{
		POSITION pos;
		pos = m_arrTask.GetStartPosition();

		while (pos != NULL)
		{
			void *tk = m_arrTask.GetValueAt(pos);
			if (tk != NULL)
			{
				if (m_arrTaskMap.Lookup(tk) != NULL)
				{
					CString strID = m_arrTaskMap.Lookup(tk)->m_value;
					CSoftListItemData *pData = NULL;
					if (m_arrDataMap.Lookup(strID) != NULL)
					{
						pData = m_arrDataMap.Lookup(strID)->m_value;
						if (pData != NULL && !pData->m_bPause)
						{
							pData->m_bDownloading = TRUE;
							pData->m_bCancel = FALSE;
							pData->m_bPause = TRUE;
							m_pDTManager->StopTask(tk);
							//							m_pDTManager->CancelTask(tk);
							tk = NULL;
						}
					}
				}
			}
			m_arrTask.GetNext(pos);
		}
		m_arrTask.RemoveAll();
	}

#if PHONE_NESS_SWITCH
	{
		CriticalSectionScoped locker(m_csPhoneNess);

		//POSITION pos = m_id2PhoneSoft.GetStartPosition();
		//while(pos != NULL)
		//{
		//	const PhoneSoft &ps = m_id2PhoneSoft.GetNextValue(pos);
		//	
		//	// 直接使用
		//	m_pPhoneDTMgr->StopTask(ps.idDown);
		//}

		m_csPhoneNess.Term();
	}
#endif

	for ( int _nIndex = 0; _nIndex < m_arrUpdCache.GetSize(); ++_nIndex )
	{
		if ( m_arrUpdCache[_nIndex] )
		{
			delete m_arrUpdCache[_nIndex];
		}
	}
	m_arrUpdCache.RemoveAll();

	if (m_hDownIcon != (HANDLE)-1)
	{
		TerminateThread(m_hDownIcon, NULL);
	}

	if (m_hInstall != (HANDLE)-1)
	{
		TerminateThread(m_hInstall, NULL);
	}

	if (m_hInitData != (HANDLE)-1)
	{
		TerminateThread(m_hInitData, NULL);
	}

	if (m_hInstall != (HANDLE)-1)
	{
		TerminateThread(m_hInstall, NULL);
	}

	if (m_hDownload != (HANDLE)-1)
	{
		TerminateThread(m_hDownload, NULL);
	}

	if ( m_hSignLoadFinish )
	{
		::CloseHandle(m_hSignLoadFinish);
		m_hSignLoadFinish = NULL;
	}

	if (m_hUpdSoftInfoThread != (HANDLE)-1)
	{
		TerminateThread(m_hUpdSoftInfoThread, NULL);
	}

	if (m_hCheckInstallThread != (HANDLE)-1)
	{
		TerminateThread(m_hCheckInstallThread, NULL);
	}

	m_arrTaskMap.RemoveAll();

	if (m_arrDownload.GetCount() > 0)
	{
		m_arrDownload.RemoveAll();
	}

	if (m_pImge != NULL)
	{
		delete m_pImge;
		m_pImge = NULL;
	}

	m_font.Detach();

	if (m_pUninstall)
	{
		m_pUninstall->Uninitialize();
		m_pUninstall = NULL;
	}

	if (m_pInfoQuery)
	{
		m_pInfoQuery->Uninitialize();
		m_pInfoQuery = NULL;
	}

	m_softUpdatedList.Cleanup();
}

void CBeikeSafeSoftmgrUIHandler::DeleteFileAfterTime()
{
	int		nRet;
	nRet = BKSafeConfig::GetDelFile();
	if (nRet != 2)
	{
		return;
	}

	// 首先确定存贮安装包的路径
	CString strStore;
	BKSafeConfig::GetStoreDir(strStore);
	if(strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
	{
		strStore.Append(_T("\\"));
	}

	// 遍历保存软件信息的数组，发现bDownload为真时判断是否需要删除
	for ( int i=0; i<m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_arrData[i];

		if ( pData->m_bDownLoad )
		{
			CString strName;
			CFileTime timeleft, timeright;
			LONGLONG  diff;

			strName = strStore + pData->m_strFileName;

			SYSTEMTIME sysTimeLeft;
			GetLocalTime(&sysTimeLeft);

			SystemTimeToFileTime( &sysTimeLeft, &timeleft );

			WIN32_FILE_ATTRIBUTE_DATA	data;
			GetFileAttributesEx( strName, GetFileExInfoStandard, &data );
			timeright.dwHighDateTime = data.ftCreationTime.dwHighDateTime;
			timeright.dwLowDateTime  = data.ftCreationTime.dwLowDateTime;

			diff = ( LONGLONG )( timeleft.GetTime() - timeright.GetTime() );
			if ( diff > CFileTime::Day * 7 )
			{
				DeleteFile(strName);
				pData->m_bDownLoad = FALSE;
				pData->m_bDownloading = FALSE;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	CSimpleArray<CString> arrItemsRead;
	CSimpleArray<CString> arrItemsWrite;

	CString strFilePath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strFilePath)) )
		strFilePath.Append(_T("\\ksoft_download"));

	BOOL bChange = FALSE;
	if(PathFileExists(strFilePath)) 
	{
		LoadSoftInfoLog(strFilePath, arrItemsRead);

		CString strStore;
		BKSafeConfig::GetStoreDir(strStore);
		if(strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
			strStore.Append(_T("\\"));

		for (int i=0; i < arrItemsRead.GetSize(); i++)
		{
			CString strName;
			strName = strStore + arrItemsRead[i];

			CFileTime timeleft, timeright;
			LONGLONG  diff;

			SYSTEMTIME sysTimeLeft;
			GetLocalTime(&sysTimeLeft);

			SystemTimeToFileTime( &sysTimeLeft, &timeleft );

			WIN32_FILE_ATTRIBUTE_DATA	data;
			GetFileAttributesEx( strName, GetFileExInfoStandard, &data );
			timeright.dwHighDateTime = data.ftCreationTime.dwHighDateTime;
			timeright.dwLowDateTime  = data.ftCreationTime.dwLowDateTime;

			diff = ( LONGLONG )( timeleft.GetTime() - timeright.GetTime() );
			if ( diff > CFileTime::Day * 7 )
			{
				bChange = TRUE;
				DeleteFile(strName);
			}
			else
			{
				if(!PathFileExists(strName)) 
					bChange = TRUE;
				else
					arrItemsWrite.Add(arrItemsRead[i]);
			}
		}
	}

	if (bChange)
		SaveSoftInfoLog(strFilePath, arrItemsWrite);
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrRefreshBottom(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	if (!m_bViewDetail)
	{
		m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, TRUE);

		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);
	}
	else
	{
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_DLG_INSTALL_SOFTMGR, FALSE);
	}

	m_pDlg->SetItemVisible(IDC_TYPES_LEFT_SOFTMGR, TRUE);

	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_FREESOFT, TRUE);
	m_pDlg->SetItemVisible(IDC_DOWNLOADMGR_SOFTMGR, TRUE);
	m_pDlg->SetItemVisible(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, TRUE);
	m_pDlg->SetItemVisible(IDS_LINK_SUBMIT_SOFTMGR, TRUE);

	return 0;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnRefreshNetPage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

STATE_CLOSE_CHECK CBeikeSafeSoftmgrUIHandler::CloseCheck()
{
	CString strInfo;
	int nCount = 0;
	for (int i = 0; i < m_arrData.GetSize(); i ++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData != NULL && pData->m_bDownloading && !pData->m_bDownLoad && !pData->m_bLinkFailed && !pData->m_bPause)
		{
			nCount ++;
		}
	}
	if (nCount > 0)
	{
		CBkSafeMsgBox2	msgbox;
		msgbox.AddButton(L"是", IDOK);
		msgbox.AddButton(L"否", IDCANCEL);
		strInfo.Format(BkString::Get(IDS_SOFTMGR_8070), nCount);
		UINT nRet = msgbox.ShowMutlLineMsg(strInfo, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
		if(nRet==IDOK)
			return CLOSE_CHECK_FORCE_CLOSE;
		else
			return CLOSE_CHECK_CANCEL_CLOSE;
	}

	int nCheck = m_necessUIHandler.CloseCheck();

	if(nCheck==1)
		return CLOSE_CHECK_FORCE_CLOSE;
	else if (nCheck==0)
		return CLOSE_CHECK_CANCEL_CLOSE;

	return CLOSE_CHECK_CONTINUE;
}

void CBeikeSafeSoftmgrUIHandler::OnEditMouseWheelMsg(WPARAM wParam, LPARAM lParam)
{
	CPoint pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

	OnMouseWheel(LOWORD(wParam), HIWORD(wParam), pt);
}

BOOL CBeikeSafeSoftmgrUIHandler::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rcList;

	if(m_nTab == TAB_INDEX_DAQUAN)
	{
		if (m_bViewDetail)
		{
			m_IEDetail.GetWindowRect(&rcList);
			if(rcList.PtInRect(pt))
			{
				SendMessage(m_IEDetail.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
			}
		}
		else
		{
			m_list.GetWindowRect(&rcList);
			if (rcList.PtInRect(pt))
			{
				SendMessage(m_list.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
			}
		}

		m_SoftTypeList.GetWindowRect(&rcList);
		if (rcList.PtInRect(pt))
		{
			SendMessage(m_SoftTypeList.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		}
	}
	else if(m_nTab == TAB_INDEX_UNINSTALL)
	{
		if(m_bViewUninstall)
		{
			m_IEUninstallDetail.GetWindowRect(&rcList);
			if (rcList.PtInRect(pt))
			{
				SendMessage(m_IEUninstallDetail.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
			}
		}
		else
		{
			m_UniExpandSoftList.GetWindowRect(&rcList);
			if (rcList.PtInRect(pt))
			{
				SendMessage(m_UniExpandSoftList.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
			}
		}

		//m_TypeUni.GetWindowRect(&rcList);
		//if (rcList.PtInRect(pt))
		//{
		//	SendMessage(m_TypeUni.m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
		//}
	}

	SetMsgHandled(FALSE);
	return FALSE;
}

void CBeikeSafeSoftmgrUIHandler::LoadUpdateIgnoreList()
{
	if (m_arrIgnore.GetSize() != 0)
		return ;

	CString strFilePath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strFilePath)) )
	{
		strFilePath.Append(_T("\\ksoft_ignore"));
	}

	// 若安装缓存存在，根据缓存文件中的id设置对应软件的状态并刷新右侧列表
	if(!PathFileExists(strFilePath)) 
		return ;

	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char szSoftID[MAX_PATH] = {0};
	CAtlMap<CString, CSoftListItemData*>::CPair*	lpPair = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strFilePath, strFilePath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
	if (!(pf = fopen(szFilePath, "r"))) 
	{
		return ;
	}

	// 读取文件的内容
	while( !feof(pf) )
	{
		fgets(szSoftID, MAX_PATH, pf);
		DWORD dwLength = ::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, NULL, 0);
		wchar_t *szID = new wchar_t[dwLength];

		::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, szID, dwLength);
		strID.Empty();
		strID.Format(_T("%s"), szID);
		if(szID != NULL)
		{
			delete []szID;
			szID = NULL;
		}

		if (strID.GetLength() && m_arrIgnore.Find(strID) == -1)
			m_arrIgnore.Add(strID);
	}

	fclose(pf);
}

void CBeikeSafeSoftmgrUIHandler::GetSoftStateFromCach(CString strFilePath)
{
	m_cs.Lock();
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char szSoftID[MAX_PATH] = {0};
	CAtlMap<CString, CSoftListItemData*>::CPair*	lpPair = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strFilePath, strFilePath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
	if (!(pf = fopen(szFilePath, "r"))) 
	{
		return ;
	}

	// 读取文件的内容
	while( !feof(pf) )
	{
		fgets(szSoftID, MAX_PATH, pf);
		DWORD dwLength = ::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, NULL, 0);
		wchar_t *szID = new wchar_t[dwLength];

		::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, szID, dwLength);
		strID.Empty();
		strID.Format(_T("%s"), szID);
		if(szID != NULL)
		{
			delete []szID;
			szID = NULL;
		}

		// 查询对应id的软件
		CSoftListItemData *pData = NULL;
		lpPair = m_arrDataMap.Lookup(strID);
		if ( lpPair != NULL )
		{
			pData = lpPair->m_value;
		}

		// 修改软件的状态为已安装
		if ( pData != NULL )
		{
			pData->m_bSetup = TRUE;

			if ( pData->m_bDownloading && !pData->m_bWaitDownload && !pData->m_bDownLoad)
			{
				pData->m_bDownloading = TRUE;
				pData->m_bInstalling = FALSE;
				pData->m_bWaitInstall = FALSE;
			}
			else
			{
				pData->m_bDownloading = FALSE; //已经安装则不显示进度条，显示完整信息
				pData->m_bInstalling = FALSE;
				pData->m_bWaitInstall = FALSE;
			}

			// 添加到安装缓存中
			if ( -1 == m_arrCachSetup.Find( pData ) )
			{
				m_arrCachSetup.Add( pData );
			}

			// 更新列表中对应软件的状态, ?
			_RefershItemBySoftIDWorkerThread(pData->m_strSoftID);
		}
	}

	fclose(pf);
	m_cs.Unlock();

}

void CBeikeSafeSoftmgrUIHandler::GetUpdateStateFromCache(CString strFilePath)
{
	m_csUpdCacheLock.Lock();

	CAtlMap<CString,CString> _mapUpdSoft;
	if ( m_pSoftChecker )
	{
		m_pSoftChecker->GetUpdateFromCache(UpdateCheckCallBack, GetInfoUseMap, &_mapUpdSoft, this);
	}

	m_csUpdCacheLock.Unlock();

	return;
}

void CBeikeSafeSoftmgrUIHandler::UpdateCach()
{
	CString  strCachPath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
	{
		strCachPath.Append(_T("\\ksoft_setuped"));
	}

	m_cs.Lock();
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char *szSoftID = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strCachPath, strCachPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );

	if (!(pf = fopen(szFilePath, "w"))) 
	{
		return ;
	}

	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData != NULL && pData->m_bSetup)
		{
			DWORD dwLength = ::WideCharToMultiByte(CP_ACP, 0, pData->m_strSoftID, 
				-1, NULL, 0, NULL, FALSE);
			szSoftID = new char[dwLength];
			::WideCharToMultiByte(CP_ACP, 0, pData->m_strSoftID, -1, szSoftID,
				dwLength, NULL, FALSE);
			fwrite(szSoftID, dwLength, 1, pf);
			fwrite("\r\n", strlen("\r\n"), 1, pf);

			if(szSoftID != NULL)
			{
				delete []szSoftID;
				szSoftID = NULL;
			}
		}
	}

	fclose(pf);
	m_cs.Unlock();
}

void CBeikeSafeSoftmgrUIHandler::SaveUpdateIgnoreList()
{
	CString  strCachPath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
		strCachPath.Append(_T("\\ksoft_ignore"));

	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char *szSoftID = NULL;

	if (m_bInitUpdate && m_arrCheckUpdate.GetSize())
	{
		m_arrIgnore.RemoveAll();

		for (int i = 0; i < m_arrCheckUpdate.GetSize(); i++)
		{
			CSoftListItemData *pData = m_arrCheckUpdate[i];
			if (pData != NULL && pData->m_bUpdate && pData->m_bIgnore)
			{
				CString strSoftId;
				strSoftId = pData->m_strSoftID;
				m_arrIgnore.Add(strSoftId);
			}
		}
	}

	if (m_arrIgnore.GetSize())
	{
		::WideCharToMultiByte( CP_ACP, 0, strCachPath, strCachPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
		if (!(pf = fopen(szFilePath, "w"))) 
			return ;

		for (int i=0; i < m_arrIgnore.GetSize(); i++)
		{
			CString strSoftId;
			strSoftId = m_arrIgnore[i];

			DWORD dwLength = ::WideCharToMultiByte(CP_ACP, 0, strSoftId, 
				-1, NULL, 0, NULL, FALSE);
			szSoftID = new char[dwLength];
			::WideCharToMultiByte(CP_ACP, 0, strSoftId, -1, szSoftID,
				dwLength, NULL, FALSE);
			fwrite(szSoftID, dwLength, 1, pf);
			fwrite("\r\n", strlen("\r\n"), 1, pf);

			if(szSoftID != NULL)
			{
				delete []szSoftID;
				szSoftID = NULL;
			}
		}

		fclose(pf);
	}
}

BOOL CBeikeSafeSoftmgrUIHandler::IsRecommend(CSoftListItemData *pData)
{
	int nType = ST_INVALID;
	if (pData->m_bCharge == FALSE)
	{
		nType = ST_FREE;
		if ((pData->m_attri&SA_Beta) == SA_Beta)
			nType = ST_BETA;
	}
	else
		nType = ST_PAY;

	if ((nType == ST_BETA || nType == ST_PAY) && (pData->m_attri&SA_Major) != SA_Major)
		return FALSE;
	else
		return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::SaveSoftInfoLog(CString strFilePath, CSimpleArray<CString>& arr)
{
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char *szSoftID = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strFilePath, strFilePath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
	if (!(pf = fopen(szFilePath, "w"))) 
		return ;

	for (int i=0; i < arr.GetSize(); i++)
	{
		CString strSoftId;
		strSoftId = arr[i];

		DWORD dwLength = ::WideCharToMultiByte(CP_ACP, 0, strSoftId, 
			-1, NULL, 0, NULL, FALSE);
		szSoftID = new char[dwLength];
		::WideCharToMultiByte(CP_ACP, 0, strSoftId, -1, szSoftID,
			dwLength, NULL, FALSE);
		fwrite(szSoftID, dwLength, 1, pf);
		fwrite("\r\n", strlen("\r\n"), 1, pf);

		if(szSoftID != NULL)
		{
			delete []szSoftID;
			szSoftID = NULL;
		}
	}

	fclose(pf);
}

void CBeikeSafeSoftmgrUIHandler::LoadSoftInfoLog(CString strFilePath, CSimpleArray<CString>& arr)
{
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char szSoftID[MAX_PATH] = {0};
	CAtlMap<CString, CSoftListItemData*>::CPair*	lpPair = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strFilePath, strFilePath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );
	if (!(pf = fopen(szFilePath, "r"))) 
	{
		return ;
	}

	// 读取文件的内容
	while( !feof(pf) )
	{
		fgets(szSoftID, MAX_PATH, pf);
		DWORD dwLength = ::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, NULL, 0);
		wchar_t *szID = new wchar_t[dwLength];

		::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, szID, dwLength);
		strID.Empty();
		strID.Format(_T("%s"), szID);
		if(szID != NULL)
		{
			delete []szID;
			szID = NULL;
		}

		if (strID.GetLength() && arr.Find(strID) == -1)
			arr.Add(strID);
	}

	fclose(pf);
}

void CBeikeSafeSoftmgrUIHandler::SaveDownloadSoftInfo(CString strSoftInfo)
{
	CSimpleArray<CString> arrItems;

	CString strFilePath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strFilePath)) )
		strFilePath.Append(_T("\\ksoft_download"));

	if(!PathFileExists(strFilePath)) 
	{
		arrItems.Add(strSoftInfo);

		SaveSoftInfoLog(strFilePath, arrItems);
	}
	else
	{
		LoadSoftInfoLog(strFilePath, arrItems);

		if (strSoftInfo.GetLength() && arrItems.Find(strSoftInfo) == -1)
		{
			arrItems.Add(strSoftInfo);

			SaveSoftInfoLog(strFilePath, arrItems);
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::SaveUpdatePointOutInfo()
{
	if (m_bInitUpdate)
	{
		ksm::CUpdateInfoMgr mgr;

		for (int i = 0; i < m_arrCheckUpdate.GetSize(); i++)
		{
			CSoftListItemData *pData = m_arrCheckUpdate[i];
			BOOL bIsRecommend = FALSE;
			bIsRecommend = IsRecommend(pData);

			if (pData != NULL && pData->m_bUpdate && pData->m_bIgnore == FALSE && bIsRecommend)
			{
				int nId = _wtoi( pData->m_strSoftID );
				int nOrder = _wtoi( pData->m_strOrder );
				mgr.AddUpdateInfo(nId, nOrder, pData->m_strName, pData->m_strPublished, 
					pData->m_strVersion, pData->m_strNewVersion, 
					pData->m_dwSize, pData->m_attri, pData->m_strMainExe);
			}
		}

		CString  strPath;
		if( SUCCEEDED(CAppPath::Instance().GetLeidianAppPath(strPath)) )
		{
			mgr.Save(strPath);
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::ShowLoadInstallInfoTip()
{
	static int nPos = 0;
	++nPos;

	m_pDlg->SetItemVisible(IDC_LOAD_PIC_SOFTMGR, FALSE);
	m_pDlg->SetItemIntAttribute(IDC_LOAD_PIC_SOFTMGR, "sub", (nPos)%8);
	m_pDlg->SetItemIntAttribute(IDC_LOAD_PIC1_SOFTMGR, "sub", (nPos)%8);
	m_pDlg->SetItemIntAttribute(IDC_LOAD_PIC2_SOFTMGR, "sub", (nPos)%8);
	m_pDlg->SetItemIntAttribute(IDC_LOAD_TIPPIC_SOFTMGR, "sub", (nPos)%8);
	m_pDlg->SetItemVisible(IDC_LOAD_PIC_SOFTMGR, TRUE);
}

CString CBeikeSafeSoftmgrUIHandler::GetTypeUI(CString strTypeData)
{
	CString strResult;

	if (strTypeData == L"未知")
		strResult = strTypeData;
	else if (strTypeData == L"全部")
		strResult = strTypeData;
	else if (strTypeData == L"聊天工具")
		strResult = L"聊天";
	else if (strTypeData == L"下载工具")
		strResult = L"下载";
	else if (strTypeData == L"浏览器")
		strResult = strTypeData;
	else if (strTypeData == L"输入法")
		strResult = strTypeData;
	else if (strTypeData == L"音乐播放")
		strResult = L"音乐";
	else if (strTypeData == L"视频播放")
		strResult = L"视频";
	else if (strTypeData == L"图形图像")
		strResult = L"图像";
	else if (strTypeData == L"游戏对战")
		strResult = L"游戏";
	else if (strTypeData == L"杀毒防护")
		strResult = L"杀毒";
	else if (strTypeData == L"系统工具")
		strResult = L"系统";
	else if (strTypeData == L"股票网银")
		strResult = L"股票";
	else if (strTypeData == L"翻译词典")
		strResult = L"翻译";
	else if (strTypeData == L"其他")
		strResult = L"生活";
	else if (strTypeData == L"学习考试")
		strResult = L"学习";
	else if (strTypeData == L"文字处理")
		strResult = L"文字";
	else if (strTypeData == L"编程开发")
		strResult = L"编程";
	else if (strTypeData == L"邮件")
		strResult = L"邮件";
	else if (strTypeData == L"压缩刻录")
		strResult = L"解压";
	else if (strTypeData == L"数码编辑")
		strResult = L"数码";
	else if (strTypeData == L"网络应用")
		strResult = L"网络";
	else if (strTypeData == L"驱动")
		strResult = L"驱动";
	else if (strTypeData == L"桌面主题")
		strResult = L"主题";

	return strResult;
}

CString CBeikeSafeSoftmgrUIHandler::GetTypeData(CString strTypeUI)
{
	CString strResult;

	if (strTypeUI == L"未知")
		strResult = strTypeUI;
	else if (strTypeUI == L"全部")
		strResult = strTypeUI;
	else if (strTypeUI == L"聊天")
		strResult = L"聊天工具";
	else if (strTypeUI == L"下载")
		strResult = L"下载工具";
	else if (strTypeUI == L"浏览器")
		strResult = strTypeUI;
	else if (strTypeUI == L"输入法")
		strResult = strTypeUI;
	else if (strTypeUI == L"音乐")
		strResult = L"音乐播放";
	else if (strTypeUI == L"视频")
		strResult = L"视频播放";
	else if (strTypeUI == L"图像")
		strResult = L"图形图像";
	else if (strTypeUI == L"游戏")
		strResult = L"游戏对战";
	else if (strTypeUI == L"杀毒")
		strResult = L"杀毒防护";
	else if (strTypeUI == L"系统")
		strResult = L"系统工具";
	else if (strTypeUI == L"股票")
		strResult = L"股票网银";
	else if (strTypeUI == L"翻译")
		strResult = L"翻译词典";
	else if (strTypeUI == L"生活")
		strResult = L"其他";
	else if (strTypeUI == L"学习")
		strResult = L"学习考试";
	else if (strTypeUI == L"文字")
		strResult = L"文字处理";
	else if (strTypeUI == L"编程")
		strResult = L"编程开发";
	else if (strTypeUI == L"邮件")
		strResult = L"邮件";
	else if (strTypeUI == L"解压")
		strResult = L"压缩刻录";
	else if (strTypeUI == L"数码")
		strResult = L"数码编辑";
	else if (strTypeUI == L"网络")
		strResult = L"网络应用";
	else if (strTypeUI == L"驱动")
		strResult = L"驱动";
	else if (strTypeUI == L"主题")
		strResult = L"桌面主题";

	return strResult;
}

int CBeikeSafeSoftmgrUIHandler::GetDaquanType(CString strType)
{
	int nResult = 0;

	if (strType == L"最新更新")
		nResult = 0;
	else if (strType == L"全部")
		nResult = 1;
	else if (strType == L"聊天工具")
		nResult = 2;
	else if (strType == L"下载工具")
		nResult = 3;
	else if (strType == L"浏览器")
		nResult = 4;
	else if (strType == L"输入法")
		nResult = 5;
	else if (strType == L"音乐播放")
		nResult = 6;
	else if (strType == L"视频播放")
		nResult = 7;
	else if (strType == L"图形图像")
		nResult = 8;
	else if (strType == L"游戏对战")
		nResult = 9;
	else if (strType == L"杀毒防护")
		nResult = 10;
	else if (strType == L"系统工具")
		nResult = 11;
	else if (strType == L"股票网银")
		nResult = 12;
	else if (strType == L"翻译词典")
		nResult = 13;
	else if (strType == L"其他")
		nResult = 14;
	else if (strType == L"学习考试")
		nResult = 15;
	else if (strType == L"文字处理")
		nResult = 16;
	else if (strType == L"编程开发")
		nResult = 17;
	else if (strType == L"邮件")
		nResult = 18;
	else if (strType == L"压缩刻录")
		nResult = 19;
	else if (strType == L"数码编辑")
		nResult = 20;
	else if (strType == L"网络应用")
		nResult = 21;
	else if (strType == L"驱动")
		nResult = 22;
	else if (strType == L"桌面主题")
		nResult = 23;

	return nResult + IDC_SOFTMGR_DAQUAN_TYPE_FIRST;
}

void CBeikeSafeSoftmgrUIHandler::OnClickDaquanType(UINT uCmdId)
{
	CString strNewType;
	DWORD	nIndex = uCmdId-IDC_SOFTMGR_DAQUAN_TYPE_FIRST;
	switch(nIndex)
	{
	case 0:strNewType = L"最新更新";	break;
	case 1:strNewType = L"全部";		break;
	case 2:strNewType = L"聊天工具";	break;
	case 3:strNewType = L"下载工具";	break;
	case 4:strNewType = L"浏览器";		break;
	case 5:strNewType = L"输入法";	break;
	case 6:strNewType = L"音乐播放";	break;
	case 7:strNewType = L"视频播放";	break;
	case 8:strNewType = L"图形图像";	break;
	case 9:strNewType = L"游戏对战";	break;
	case 10:strNewType = L"杀毒防护";		break;
	case 11:strNewType = L"系统工具";	break;
	case 12:strNewType = L"股票网银";	break;
	case 13:strNewType = L"翻译词典";	break;
	case 14:strNewType = L"其他";	break;
	case 15:strNewType = L"学习考试";	break;
	case 16:strNewType = L"文字处理";	break;
	case 17:strNewType = L"编程开发";	break;
	case 18:strNewType = L"邮件";		break;
	case 19:strNewType = L"压缩刻录";	break;
	case 20:strNewType = L"数码编辑";	break;
	case 21:strNewType = L"网络应用";		break;
	case 22:strNewType = L"驱动";		break;
	case 23:strNewType = L"桌面主题";	break;
	}

	if (m_strCurType == strNewType && !m_bQuery && !m_bViewDetail) //之前显示的不是搜索页面
	{
		return;
	}

	m_pDlg->SetItemVisible( m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_BK_FIRST, FALSE);
	m_pDlg->SetItemAttribute( m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_FIRST, "class","typenormal" );
	m_nCurType = nIndex;
	m_pDlg->SetItemVisible( m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_BK_FIRST, TRUE);
	m_pDlg->SetItemAttribute( m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_FIRST, "class","typesel" );

	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_NAME, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_SIZE, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_TIME, "class","linkblue" );

	CString strType;
	strType = m_pDlg->GetItemText(m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_FIRST);
	m_pDlg->SetItemText(IDC_SMR_DAQUAN_HEADER_TYPE, strType);
	CString strCount;
	int nCount = 0;
	if (strNewType == L"最新更新")
		nCount = 50;
	else if (strNewType == L"全部")
		nCount = m_arrData.GetSize();
	else
		nCount = m_pSoftMgr->GetSoftCount(strNewType);
	strCount.Format(L"共%d款", nCount);
	m_pDlg->SetItemText(IDC_SMR_DAQUAN_HEADER_COUNT, strCount);

	ClickDaquanType(strNewType);
}

void CBeikeSafeSoftmgrUIHandler::OnClickXiezaiType(UINT uCmdId)
{
	DWORD	nIndex = uCmdId-IDC_SOFTMGR_XIEZAI_TYPE_FIRST;
	m_pDlg->SetItemVisible( m_nCurTypeUni + IDC_SOFTMGR_XIEZAI_TYPE_BK_FIRST, FALSE);
	m_pDlg->SetItemAttribute( m_nCurTypeUni + IDC_SOFTMGR_XIEZAI_TYPE_FIRST, "class","typenormal" );
	m_nCurTypeUni = nIndex;
	m_pDlg->SetItemVisible( m_nCurTypeUni + IDC_SOFTMGR_XIEZAI_TYPE_BK_FIRST, TRUE);
	m_pDlg->SetItemAttribute( m_nCurTypeUni + IDC_SOFTMGR_XIEZAI_TYPE_FIRST, "class","typesel" );

	CString strTypeUINew;
	strTypeUINew = m_pDlg->GetItemText( uCmdId );
	CString strTypeDataNew;
	strTypeDataNew = GetTypeData(strTypeUINew);

	CString strCurTypeUniOld;
	strCurTypeUniOld = m_strCurTypeUni;
	m_strCurTypeUni = strTypeDataNew;


	OnUninstallDetailBack();	
	BOOL bQuery = m_bQueryUni;
	OnUninstallQueryBack();

	if (bQuery || strCurTypeUniOld.CompareNoCase(m_strCurTypeUni) != 0)
	{
		_ReloadListUni();
		OnBtnSmrUniSortByName(); 
	}
}

void CBeikeSafeSoftmgrUIHandler::OnSubmitSoft()
{
	CString strFreeBack = L"http://baike.ijinshan.com/ksafe/baike/customer_newsoft.html";
	ShellExecute( NULL, _T("open"), strFreeBack, NULL, NULL , SW_SHOW);
}

void CBeikeSafeSoftmgrUIHandler::GetDownloadCachInfo()
{
	CString strCachPath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
	{
		strCachPath.Append(_T("\\ksoft_info"));
	}

	//	m_cs.Lock();
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char szSoftID[MAX_PATH] = {0};
	::WideCharToMultiByte( CP_ACP, 0, strCachPath, strCachPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );

	if (!(pf = fopen(szFilePath, "r"))) 
	{
		return ;
	}

	while( !feof(pf) )
	{
		fgets(szSoftID, MAX_PATH, pf);
		DWORD dwLength = ::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, NULL, 0);
		wchar_t *szID = new wchar_t[dwLength];

		::MultiByteToWideChar (CP_ACP, 0, szSoftID, -1, szID, dwLength);
		strID.Empty();
		strID.Format(_T("%s"), szID);
		CString strSoftId;
		strSoftId = strID.Left(strID.Find(_T(" ")));
		strID = strID.Right(strID.GetLength() - strID.Find(_T(" ")) - 1);
		softInfo *pSoftInfo = new softInfo;
		pSoftInfo->nState = _wtoi(strID.Left(strID.Find(_T(" "))));
		strID = strID.Right(strID.GetLength() - strID.Find(_T(" ")) - 1);
		pSoftInfo->nProgress = _wtoi(strID);

		if(szID != NULL)
		{
			delete []szID;
			szID = NULL;
		}

		if (m_arrCachInfo.Lookup( strSoftId) != NULL)
		{
			continue;
		}
		m_arrCachInfo.SetAt(strSoftId, pSoftInfo);

	}
	fclose(pf);
	//	m_cs.Unlock();
}

void CBeikeSafeSoftmgrUIHandler::SetDownloadCachInfo()
{
	CString strCachPath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
	{
		strCachPath.Append(_T("\\ksoft_info"));
	}

	//	m_cs.Lock();
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char *szSoftID = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strCachPath, strCachPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );

	if (!(pf = fopen(szFilePath, "w"))) 
	{
		return ;
	}

	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData != NULL )
		{
			if ( pData->m_bCancel )
			{
				CString strTmp;
				strTmp.Format(_T("%s %d %d"), pData->m_strSoftID, 2, pData->m_dwProgress);
				DWORD dwLength = ::WideCharToMultiByte(CP_ACP, 0, strTmp, 
					-1, NULL, 0, NULL, FALSE);
				szSoftID = new char[dwLength];
				::WideCharToMultiByte(CP_ACP, 0, strTmp, -1, szSoftID,
					dwLength, NULL, FALSE);
				fwrite(szSoftID, dwLength, 1, pf);
				fwrite("\r\n", strlen("\r\n"), 1, pf);

				if(szSoftID != NULL)
				{
					delete []szSoftID;
					szSoftID = NULL;
				}
			}
			else if ( (pData->m_bPause || pData->m_bContinue) && pData->m_bDownloading )
			{
				DWORD dwProgress = pData->m_dwProgress;
				if ( pData->m_bLinking )
				{
					dwProgress = 0;
				}

				CString strTmp;
				strTmp.Format(_T("%s %d %d"), pData->m_strSoftID, 1, dwProgress);
				DWORD dwLength = ::WideCharToMultiByte(CP_ACP, 0, strTmp, 
					-1, NULL, 0, NULL, FALSE);
				szSoftID = new char[dwLength];
				::WideCharToMultiByte(CP_ACP, 0, strTmp, -1, szSoftID,
					dwLength, NULL, FALSE);
				fwrite(szSoftID, dwLength, 1, pf);
				fwrite("\r\n", strlen("\r\n"), 1, pf);

				if(szSoftID != NULL)
				{
					delete []szSoftID;
					szSoftID = NULL;
				}
			}
		}
	}

	fclose(pf);
	//	m_cs.Unlock();
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateViewDetail(DWORD dwIndex, HWND hWndList) //查看软件详情
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData == NULL)
		return;

	if (pData->m_bUpdate)
		m_pDlg->SetItemAttribute(IDC_UPDATE_BTN_DETAIL, "skin", "btndetailupdate");
	else
		m_pDlg->SetItemAttribute(IDC_UPDATE_BTN_DETAIL, "skin", "btndetailsetup");

	m_nIndexUpdate = dwIndex;
	m_hWndListUpdate = hWndList;
	m_strUpdateDetailSoftID = pData->m_strSoftID;

	//软件信息
	CString strVersion;
	if (pData->m_strNewVersion.IsEmpty() == FALSE && 
		pData->m_strNewVersion.CompareNoCase(L"0.0.0.0") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"1.0.0.0") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"0.0.0.1") != 0 &&
		pData->m_strNewVersion.CompareNoCase(L"1.0.0.1") != 0)
	{
		strVersion = pData->m_strNewVersion;
	}
	else if (pData->m_strVersion.IsEmpty() == FALSE  && 
		pData->m_strVersion.CompareNoCase(L"0.0.0.0") != 0 &&
		pData->m_strVersion.CompareNoCase(L"1.0.0.0") != 0 &&
		pData->m_strVersion.CompareNoCase(L"0.0.0.1") != 0 &&
		pData->m_strVersion.CompareNoCase(L"1.0.0.1") != 0)
	{
		strVersion = pData->m_strVersion;
	}
	if (strVersion.IsEmpty())
	{
		m_pDlg->FormatRichText(
			IDS_UPDATE_INFO_DETAIL, 
			BkString::Get(IDS_SOFTMGR_8128), 
			pData->m_strName, pData->m_strSize);
	}
	else
	{
		m_pDlg->FormatRichText(
			IDS_UPDATE_INFO_DETAIL, 
			BkString::Get(IDS_INFO_FORMAT_SOFTMGR), 
			pData->m_strName, pData->m_strSize,strVersion);
	}

	//收费信息
	if (pData->m_bCharge)
	{
		m_pDlg->SetItemAttribute(IDG_UPDATE_ICON_FREE_DETAIL, "sub", "2");
		m_pDlg->SetItemText(IDS_UPDATE_TEXT_FREE_DETAIL, _T("收费"));
	}
	else
	{
		m_pDlg->SetItemAttribute(IDG_UPDATE_ICON_FREE_DETAIL, "sub", "1");
		m_pDlg->SetItemText(IDS_UPDATE_TEXT_FREE_DETAIL, _T("免费"));
	}
	//插件信息
	if (pData->m_bPlug)
	{
		m_pDlg->SetItemAttribute(IDG_UPDATE_ICON_PLUG_DETAIL, "sub", "2");
		m_pDlg->SetItemText(IDS_UPDATE_TEXT_PLUG_DETAIL, _T("有插件"));
	}
	else
	{
		m_pDlg->SetItemAttribute(IDG_UPDATE_ICON_PLUG_DETAIL, "sub", "1");
		m_pDlg->SetItemText(IDS_UPDATE_TEXT_PLUG_DETAIL, _T("无插件"));
	}

	//系统信息
	CString strCurOS = GetCurrentSystemVersion();
	int nPos = pData->m_strOSName.Find(strCurOS);
	if (nPos >= 0 && nPos < pData->m_strOSName.GetLength())
	{
		strVersion = _T("适合您当前系统(");
		m_pDlg->SetItemAttribute(IDG_UPDATE_ICON_FITAYATEM_DETAIL, "sub", "1");
	}
	else
	{
		strVersion = _T("不适合您当前系统(");
		m_pDlg->SetItemAttribute(IDG_UPDATE_ICON_FITAYATEM_DETAIL, "sub", "2");
	}
	strVersion.Append(strCurOS + _T(")"));
	m_pDlg->SetItemText(IDS_UPDATE_TEXT_FITAYATEM_DETAIL, strVersion);

	//详情IE

	if(pData->GetInfoUrl() == _T(""))	//没有详情信息
	{
		m_IEUpdateDetail.Show2(m_strErrorHtml,NULL);
	}
	else	//有详情页URL
	{
		m_IEUpdateDetail.Show2(pData->GetInfoUrl(), m_strErrorHtml);
	}


	m_IconUpdateSoft.SetDataRes(pData->m_pImage);

	m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, TRUE);
	m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, FALSE);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateDetailBack()
{
	m_UpdateSoftList.SetFocus();
	//m_bViewDetail = FALSE;

	if (!m_strUpdateDetailSoftID.IsEmpty())
	{
		_QuerySoftMark(m_strUpdateDetailSoftID);
		m_strUpdateDetailSoftID.Empty();
	}

	m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);
	m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, TRUE);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateSelectSoft()
{

	m_UpdateSoftList.UpdateSelectSoft();
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateFromeDetail()
{
	m_IEUpdateDetail.Show2(_T("about:blank"), NULL);
	m_pDlg->SetItemVisible(IDC_UPDATE_DETAIL_PAGE, FALSE);
	m_pDlg->SetItemVisible(IDC_UPDATE_LIST_PAGE, TRUE);

	m_UpdateSoftList.SetFocus();

	if (!m_strUpdateDetailSoftID.IsEmpty())
	{
		_QuerySoftMark(m_strUpdateDetailSoftID);
		m_strUpdateDetailSoftID.Empty();
	}

	OnUpdateDownLoad(m_nIndexUpdate, m_hWndListUpdate);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateViewNew(DWORD dwIndex, HWND hWndList) //查看软件更新
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData == NULL)
		return;

	CBkNewInfoDlg	dlg( this, pData->m_strSoftID, pData->m_strName );
	UINT_PTR uRet = dlg.DoModal(GetActiveWindow());
	if (IDOK == uRet)
		_downLoadSoft(pData,ACT_SOFT_FROM_UPDATE);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateDownLoad(DWORD dwIndex, HWND hWndList)	//下载
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData == NULL)
		return;


	_downLoadSoft(pData,ACT_SOFT_FROM_UPDATE);
}

void CBeikeSafeSoftmgrUIHandler::OnBtnClick( DWORD dwIndex, int nType )
{
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateContinue(DWORD dwIndex, HWND hWndList)	//继续
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData == NULL)
		return;

	_ContinueDownLoad(pData,ACT_SOFT_FROM_UPDATE);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdatePause(DWORD dwIndex, HWND hWndList)	//暂停
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData == NULL)
		return;

	_PauseDownLoad(pData,ACT_SOFT_FROM_UPDATE);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateCancel(DWORD dwIndex, HWND hWndList)	//取消
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData == NULL)
		return;

	_CancelDownLoad(pData,ACT_SOFT_FROM_UPDATE);
}

void CBeikeSafeSoftmgrUIHandler::OnUniExpandDownLoadIcon(CString strID)
{
	if (strID == _T(""))
		return;

	OnDownLoadIcon(strID);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateDownLoadIcon(CString strID)
{
	if (strID == _T(""))
		return;

	OnDownLoadIcon(strID);
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateFreebackFromList(DWORD dwIndex, HWND hWndList)
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData)
	{
		CString strFreeback;
		strFreeback = L"http://bbs.ijinshan.com/forum-18-1.html";

		ShellExecute( NULL, _T("open"), strFreeback, NULL, NULL , SW_SHOW);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateTautilogyFromList(DWORD dwIndex, HWND hWndList)
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData)
	{
		pData->m_bLinkFailed = FALSE;
		pData->m_bLinking = TRUE;

		pData->m_bDownloading = FALSE;
		pData->m_bDownLoad = FALSE;
		OnUpdateDownLoad(dwIndex, hWndList);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateSoftMark(DWORD dwIndex, HWND hWndList)
{
	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData)
	{
		CBkMarkDlg	dlg( this, pData->m_strSoftID, pData->m_strName );
		dlg.DoModal(GetActiveWindow());

		_QuerySoftMark(pData->m_strSoftID);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnUpdateIgnore(DWORD dwIndex, HWND hWndList)
{
	if (!m_bInitUpdate)
		return ;

	CSoftListItemData *pData = m_UpdateSoftList.GetItemDataEx(dwIndex, hWndList);
	if (pData)
	{
		BOOL bIgnoreChecked = FALSE;//m_pDlg->GetItemCheck(IDC_UPDATE_CHECKBOX_IGNORE);
		if (bIgnoreChecked)
		{
			if (pData->m_bIgnore)
				pData->m_bIgnore = FALSE;
			else
				pData->m_bIgnore = TRUE;

			_LoadUpdate();
		}
		else
		{
			if (pData->m_bIgnore == FALSE)
			{
				pData->m_bIgnore = TRUE;
				_LoadUpdate();
				SaveUpdateIgnoreList();
			}
		}
	}
}

BOOL CBeikeSafeSoftmgrUIHandler::OnClickSoftUniExpand(DWORD nIndex, SOFT_UNI_INFO* pInfo)
{
	if (nIndex == CLICK_EXPAND_ENABLE)
	{
		if (pInfo->bEnable)
			return FALSE;

		//创建卸载步骤框等
		if (m_pUninstall)
		{
			ATLASSERT(m_dlgSoftMgrUninstall==NULL);
			m_dlgSoftMgrUninstall = new CBkSafeSoftMgrUnstDlg(this, m_pUninstall, pInfo->strCaption, pInfo->strKey );
			m_dlgSoftMgrUninstall->DoModal();
			pInfo->bEnable = m_dlgSoftMgrUninstall->IsSoftUnistComplete();
			delete m_dlgSoftMgrUninstall;
			m_dlgSoftMgrUninstall = NULL;

			FreshTotal();

			if (pInfo->bEnable)
			{
				m_UniExpandSoftList.RefreshRight(pInfo->strKey);

				if ( !pInfo->strSoftID.IsEmpty() && m_arrDataMap.Lookup(pInfo->strSoftID) )
				{
					CSoftListItemData *pData = NULL;
					if (m_arrDataMap.Lookup(pInfo->strSoftID, pData))
					{
						pData->m_bSetup		= FALSE;
						pData->m_bUpdate	= FALSE;
						_RefershItemBySoftIDMainThread(pData->m_strSoftID);
					}
				}


				////屏蔽卸载自动刷新
				//m_arrSoftUniDataSub.RemoveAll();
				//for (int i=0; i < m_arrSoftUniData.GetSize(); i++)
				//{
				//	SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniData[i];
				//	if (pInfo->bEnable == FALSE)
				//		m_arrSoftUniDataSub.Add(pInfo);
				//}
				//_ReloadTypeUni(FALSE);
				//_ReloadListUni();
				//OnBtnSmrUniSortByName(); 
			}
		}
	}
	else if (nIndex == CLICK_EXPAND_ICON || nIndex == CLICK_EXPAND_CAPTION)
	{
		m_bViewUninstall = TRUE;
		m_pSelSoftUniData = pInfo;

		m_pDlg->SetItemVisible( IDC_LNK_DETAIL, TRUE );
		m_pDlg->SetItemVisible( IDC_IMG_GOBACK, TRUE );

		ShowUninstallInfoBar( FALSE );

		if ( !pInfo->strMainPath.IsEmpty() && PathIsDirectory( pInfo->strMainPath ) )
		{
			m_pDlg->SetItemText( IDC_LINK_UNINSTALL_OPENDIR, BkString::Get(IDS_SOFTMGR_8059) );
			m_pDlg->SetItemAttribute(IDC_LINK_UNINSTALL_OPENDIR, "class", "linktext");
		}
		else
		{
			m_pDlg->SetItemText( IDC_LINK_UNINSTALL_OPENDIR, _T( "没有独立的安装目录，无法查看" ) );
			m_pDlg->SetItemAttribute(IDC_LINK_UNINSTALL_OPENDIR, "class", "graytext");
		}

		CString str;
		str.Format(BkString::Get(IDS_LBL_UNINSTALL_SOFT_N_FMT), pInfo->strCaption);
		CRect rcCalc;
		CDC dc = GetDC(m_pDlg->m_hWnd);
		dc.DrawText(str, -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		int nWidth = rcCalc.Width() + 100;
		ReleaseDC(m_pDlg->m_hWnd, dc);

		m_pDlg->FormatRichText(
			IDC_LBL_UNINSTALL_SOFT_NAME, 
			BkString::Get(IDS_LBL_UNINSTALL_SOFT_N_FMT), 
			pInfo->strCaption );

		CRect rcWnd;
		m_pDlg->GetWindowRect(&rcWnd);
		CString strText = pInfo->strCaption;

		CRect rcText;
		rcText.top = 9;
		rcText.bottom = 22;
		rcText.left = 150;
		rcText.right = rcText.left + nWidth;

		if (pInfo->bEnable == FALSE)
			m_pDlg->SetItemAttribute(IDC_BTN_UNINSTALL_DO, "skin", "uninstall_btn");
		else
			m_pDlg->SetItemAttribute(IDC_BTN_UNINSTALL_DO, "skin", "uninstalled_btn");

		if(pInfo->GetInfoUrl() == _T(""))
		{		
			CString		strFormat;
			strFormat.Format(_T("http://stat.ijinshan.com/uninfo.php?pid=%s&ver=%s&dat=%s&act=alldown&name=%s&cont=%s"),
				_Module.GetPID(),_Module.GetProductVersion(),GetSoftLibraryVersion(),
				url_encode.URLEncode(pInfo->strCaption),url_encode.URLEncode(pInfo->strMainPath));

			m_IEUninstallDetail.Show2(strFormat, m_strErrorHtml);
		}
		else
		{
			m_IEUninstallDetail.Show2(pInfo->GetInfoUrl(), m_strErrorHtml);
		}


		m_IconUninstallSoft.SetDataRes( NULL );
		m_IconUninstallSoft.SetDataICON(NULL);

		if (pInfo->pImg && !pInfo->bDefaultImg )
			m_IconUninstallSoft.SetDataRes( pInfo->pImg );
		else if (pInfo->hIcon)
			m_IconUninstallSoft.SetDataICON(pInfo->hIcon);
		else if (pInfo->pImg )
			m_IconUninstallSoft.SetDataRes( pInfo->pImg );

		m_pDlg->SetItemVisible( IDC_UNINSTALL_SOFT_DLG, FALSE );
		m_pDlg->SetItemVisible(IDC_UNINSTALL_SOFT_DETAIL, TRUE);
	}
	else if (nIndex == CLICK_EXPAND_MARK)
	{
		if (pInfo)
		{
			CBkMarkDlg	dlg( this, pInfo->strSoftID, pInfo->strCaption );
			dlg.DoModal(GetActiveWindow());

			_QuerySoftMark(pInfo->strSoftID);
		}
	}

	return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::ShowPicSoftMgr()
{	
	static int nNum = 0;
	GetTheShowPic( ++nNum );	
}

void CBeikeSafeSoftmgrUIHandler::GetTheShowPic(int nIndex)
{
	switch( nIndex % 3)
	{
	case 0:
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC_SOFTMGR, "skin", "softmgrinstall1");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC1_SOFTMGR, "skin", "softmgruninstall1");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC2_SOFTMGR, "skin", "softmgrinstall1");
		break;
	case 1:
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC_SOFTMGR, "skin", "softmgrinstall2");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC1_SOFTMGR, "skin", "softmgruninstall2");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC2_SOFTMGR, "skin", "softmgrinstall2");
		break;
	case 2:
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC_SOFTMGR, "skin", "softmgrinstall3");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC1_SOFTMGR, "skin", "softmgruninstall3");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC2_SOFTMGR, "skin", "softmgrinstall3");
		break;
	default:
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC_SOFTMGR, "skin", "softmgrinstall1");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC1_SOFTMGR, "skin", "softmgruninstall1");
		m_pDlg->SetItemAttribute(IDC_LOADINFO_PIC2_SOFTMGR, "skin", "softmgrinstall1");
		break;
	}
}

void CBeikeSafeSoftmgrUIHandler::ShowUninstallInfoBar(BOOL bShow)
{
	m_pDlg->SetItemVisible( IDC_UNINSTALL_QUERYBACK_LNK, !bShow);
}

void CBeikeSafeSoftmgrUIHandler::RefreshUniPage()
{
	switch(m_nUniType)
	{
	case UNI_TYPE_ALL:
		{
			m_bUniRefresh = TRUE;
			m_pUninstall->DataRefresh(FALSE);
		}
		break;

	case UNI_TYPE_START:
		m_pUninstall->LinkRefresh(ksm::SIA_Start, FALSE);
		break;

	case UNI_TYPE_QUICK:
		m_pUninstall->LinkRefresh(ksm::SIA_Quick, FALSE);
		break;

	case UNI_TYPE_DESKTOP:
		m_pUninstall->LinkRefresh(ksm::SIA_Desktop, FALSE);
		break;

	case UNI_TYPE_PROCESS:
		m_pUninstall->LinkRefresh(ksm::SIA_Process, FALSE);
		break;

	case UNI_TYPE_TRAY:
		m_pUninstall->LinkRefresh(ksm::SIA_Tray, FALSE);
		break;
	}
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSwitchWatch()
{
	CBkSafeMsgBox dlg;
	dlg.AddButton(BkString::Get(IDS_SOFTMGR_8071), IDOK);
	dlg.AddButton(BkString::Get(IDS_SOFTMGR_8072), IDCANCEL);
	CString strMsg;

	strMsg.Format(BkString::Get(IDS_SOFTMGR_8073));

	int nValue = dlg.ShowMsg( strMsg, NULL, MB_BK_CUSTOM_BUTTON | MB_ICONINFORMATION,
		NULL, m_pDlg->GetViewHWND());
	if ( nValue != IDOK )
	{
		return;
	}

	CSafeMonitorTrayShell safeMonitor;
	safeMonitor.SetMonitorRun( SM_ID_PROCESS, TRUE );
	m_pDlg->ModifyMonitorState( 0 );

}

void CBeikeSafeSoftmgrUIHandler::OnBtnResidualClear()
{
	m_pPowerSweepDlg = new CBeikeSoftMgrPowerSweepDlg(this);
	m_pSoftRubbishSweep = m_pUninstall->RubbishSweep();
	m_pPowerSweepDlg->CopySysRubInfo(m_arrRubData,m_pSoftRubbishSweep);
	m_pPowerSweepDlg->DoModal();
	BOOL	bClear = m_pPowerSweepDlg->IsCleared();
	delete m_pPowerSweepDlg;
	m_pPowerSweepDlg = NULL;

	if (bClear)
	{
		// 如果有清理，更新数据
		for (int i=0; i < m_arrRubData.GetSize(); )
		{
			SOFT_RUB_INFO* pRubInfo = (SOFT_RUB_INFO*)m_arrRubData[i];
			if (pRubInfo && pRubInfo->bCheck == TRUE)
			{
				delete pRubInfo;
				m_arrRubData.RemoveAt(i);
			}
			else
				i++;
		}

		if (m_arrRubData.GetSize())
		{
			m_pDlg->FormatRichText(
				IDC_DLG_RESIDUAL_COUNT, 
				BkString::Get(IDS_DLG_RESIDUAL_COUNT), 
				m_arrRubData.GetSize() );
			m_pDlg->SetItemVisible( IDC_DLG_RESIDUAL, TRUE );
		}
		else
			m_pDlg->SetItemVisible( IDC_DLG_RESIDUAL, FALSE );
	}
	else
	{
		// 如果没有清理，通知退出
		m_pSoftRubbishSweep->RubbishCancelSweep();
	}

	m_pSoftRubbishSweep->Uninitialize();
	m_pSoftRubbishSweep = NULL;
}

void CBeikeSafeSoftmgrUIHandler::OnBtnRubbishClear()
{
	SAFE_CALL_FUN(m_pDlg, Navigate(BKSFNS_MAKE(BKSFNS_PAGE_SYSTEM_CLEAR, BKSFNS_SYSOPT_CLR_RUBBISH)));
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSystemSlim()
{
	SAFE_CALL_FUN(m_pDlg, Navigate(BKSFNS_MAKE(BKSFNS_PAGE_SYSTEM_CLEAR, BKSFNS_SYSOPT_CLR_SHOUSHEN)));
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSmrDaquanSortByName()
{
	InsItemArray &array = m_list.GetItemArray();
	if(array.GetSize() == 0)
	{
		ATLASSERT(FALSE);
		return;
	}

	int nSize = array.GetSize();
	InsItem*	pItem = NULL;
	pItem = array[nSize - 1];
	if ((pItem->m_dwFlags & SOFTMGR_LISTITEM_TITLE) == SOFTMGR_LISTITEM_TITLE)
		nSize -= 1;

	m_nDaquanLastClickHeaderIndex = 0;
	BOOL bUp = m_list.GetSoftType(m_nDaquanLastClickHeaderIndex);
	_wsetlocale(LC_ALL, L"");
	std::sort(array.GetData(), array.GetData() + nSize, InsItemNameSort(bUp));

	m_list.UpdateAll();

	CStringA strPos;
	switch(m_nDaquanLastClickHeaderIndex)
	{
	case 0:strPos = "-158,7,-0,-0";break;
	case 1:strPos = "-88,7,-0,-0";break;
	case 2:strPos = "-20,7,-0,-0";break;
	}
	m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "pos", strPos);
	if (bUp)
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "skin", "listheaderarrowup");
	else
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "skin", "listheaderarrowdown");
	m_pDlg->SetItemVisible( IDC_SMR_DAQUAN_HEADER_ARROW, TRUE );

	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_NAME, "class","linkblue2" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_SIZE, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_TIME, "class","linkblue" );
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSmrDaquanSortBySize()
{
	InsItemArray &array = m_list.GetItemArray();
	if(array.GetSize() == 0)
	{
		ATLASSERT(FALSE);
		return;
	}

	int nSize = array.GetSize();
	InsItem*	pItem = NULL;
	pItem = array[nSize - 1];
	if ((pItem->m_dwFlags & SOFTMGR_LISTITEM_TITLE) == SOFTMGR_LISTITEM_TITLE)
		nSize -= 1;

	m_nDaquanLastClickHeaderIndex = 1;
	BOOL bUp = m_list.GetSoftType(m_nDaquanLastClickHeaderIndex);
	std::sort(array.GetData(), array.GetData() + nSize, InsItemSizeSort(bUp));

	m_list.UpdateAll();

	CStringA strPos;
	switch(m_nDaquanLastClickHeaderIndex)
	{
	case 0:strPos = "-158,7,-0,-0";break;
	case 1:strPos = "-88,7,-0,-0";break;
	case 2:strPos = "-20,7,-0,-0";break;
	}
	m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "pos", strPos);
	if (bUp)
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "skin", "listheaderarrowup");
	else
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "skin", "listheaderarrowdown");
	m_pDlg->SetItemVisible( IDC_SMR_DAQUAN_HEADER_ARROW, TRUE );

	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_NAME, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_SIZE, "class","linkblue2" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_TIME, "class","linkblue" );
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSmrDaquanSortByTime()
{
	InsItemArray &array = m_list.GetItemArray();
	if(array.GetSize() == 0)
	{
		ATLASSERT(FALSE);
		return;
	}

	int nSize = array.GetSize();
	InsItem*	pItem = NULL;
	pItem = array[nSize - 1];
	if ((pItem->m_dwFlags & SOFTMGR_LISTITEM_TITLE) == SOFTMGR_LISTITEM_TITLE)
		nSize -= 1;

	m_nDaquanLastClickHeaderIndex = 2;
	BOOL bUp = m_list.GetSoftType(m_nDaquanLastClickHeaderIndex);
	std::sort(array.GetData(), array.GetData() + nSize, InsItemMarkSort(bUp));

	m_list.UpdateAll();

	CStringA strPos;
	switch(m_nDaquanLastClickHeaderIndex)
	{
	case 0:strPos = "-158,7,-0,-0";break;
	case 1:strPos = "-88,7,-0,-0";break;
	case 2:strPos = "-20,7,-0,-0";break;
	}
	m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "pos", strPos);
	if (bUp)
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "skin", "listheaderarrowup");
	else
		m_pDlg->SetItemAttribute(IDC_SMR_DAQUAN_HEADER_ARROW, "skin", "listheaderarrowdown");
	m_pDlg->SetItemVisible( IDC_SMR_DAQUAN_HEADER_ARROW, TRUE );

	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_NAME, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_SIZE, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_DAQUAN_HEADER_TIME, "class","linkblue2" );
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSmrUniSortByName()
{
	UniItemArray &array = m_UniExpandSoftList.GetItemArray();
	if(array.GetSize() == 0)
	{
		//ATLASSERT(FALSE);
		return;
	}

	m_nUniLastClickHeaderIndex = 0;
	BOOL bUp = m_UniExpandSoftList.GetSoftType(m_nUniLastClickHeaderIndex);
	_wsetlocale(LC_ALL, L"");

	std::sort(array.GetData(), array.GetData() + array.GetSize(), UniItemNameSort(!bUp));

	m_UniExpandSoftList.UpdateAll();

	CStringA strPos;
	switch(m_nUniLastClickHeaderIndex)
	{
	case 0:strPos = "-182,7,-0,-0";break;
	case 1:strPos = "-112,7,-0,-0";break;
	case 2:strPos = "-15,7,-0,-0";break;
	}
	m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "pos", strPos);
	if (bUp)
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "skin", "listheaderarrowdown");
	else
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "skin", "listheaderarrowup");
	m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, TRUE );

	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_NAME, "class","linkblue2" );
	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_SIZE, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_TIME, "class","linkblue" );
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSmrUniSortBySize()
{
	UniItemArray &array = m_UniExpandSoftList.GetItemArray();
	if(array.GetSize() == 0)
	{
		ATLASSERT(FALSE);
		return;
	}

	m_nUniLastClickHeaderIndex = 1;
	BOOL bUp = m_UniExpandSoftList.GetSoftType(m_nUniLastClickHeaderIndex);
	std::sort(array.GetData(), array.GetData() + array.GetSize(), UniItemSizeSort(!bUp));

	m_UniExpandSoftList.UpdateAll();

	CStringA strPos;
	switch(m_nUniLastClickHeaderIndex)
	{
	case 0:strPos = "-182,7,-0,-0";break;
	case 1:strPos = "-112,7,-0,-0";break;
	case 2:strPos = "-15,7,-0,-0";break;
	}
	m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "pos", strPos);
	if (bUp)
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "skin", "listheaderarrowdown");
	else
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "skin", "listheaderarrowup");
	m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, TRUE );

	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_NAME, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_SIZE, "class","linkblue2" );
	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_TIME, "class","linkblue" );
}

void CBeikeSafeSoftmgrUIHandler::OnBtnSmrUniSortByTime()
{
	UniItemArray &array = m_UniExpandSoftList.GetItemArray();
	if(array.GetSize() == 0)
	{
		ATLASSERT(FALSE);
		return;
	}

	m_nUniLastClickHeaderIndex = 2;
	BOOL bUp = m_UniExpandSoftList.GetSoftType(m_nUniLastClickHeaderIndex);
	std::sort(array.GetData(), array.GetData() + array.GetSize(), UniItemMarkSort(!bUp));

	m_UniExpandSoftList.UpdateAll();

	CStringA strPos;
	switch(m_nUniLastClickHeaderIndex)
	{
	case 0:strPos = "-182,7,-0,-0";break;
	case 1:strPos = "-112,7,-0,-0";break;
	case 2:strPos = "-15,7,-0,-0";break;
	}
	m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "pos", strPos);
	if (bUp)
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "skin", "listheaderarrowup");
	else
		m_pDlg->SetItemAttribute(IDC_SMR_UNI_HEADER_ARROW, "skin", "listheaderarrowdown");
	m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, TRUE );

	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_NAME, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_SIZE, "class","linkblue" );
	m_pDlg->SetItemAttribute( IDC_SMR_UNI_HEADER_TIME, "class","linkblue2" );
}

void CBeikeSafeSoftmgrUIHandler::ShowQueryAndRefreshBtn( BOOL bShow )
{
	m_pDlg->SetItemVisible( IDG_QUERY_BAR_SOFTMGR, bShow);
	m_pDlg->SetItemVisible( IDC_REFRESH_PNG_SOFTMGR, bShow);
	m_pDlg->SetItemVisible( IDC_REFRESH_SOFTMGR, bShow);
}

void CBeikeSafeSoftmgrUIHandler::OnLnkCurrentBack()
{
	OnUninstallBack();

	RefreshUniPage();
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnUpdateSoftCount(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{	
	int nRecommend = 0;
	int nChoice = 0;
	m_UpdateSoftList.GetUpdateCount(nRecommend, nChoice);
	NotifySoftUpdateCount(nRecommend);

	UpdateDesktopIcon(nRecommend);

	if (nRecommend == 0 && nChoice == 0)
	{
		m_pDlg->SetItemVisible(IDC_UPDATE_NUM, FALSE);

		//m_pDlg->SetItemText(IDC_SOFT_UPDATE_TXT_NUMBER_TOP, L"0");
		m_pDlg->FormatRichText(
			IDC_SOFT_UPDATE_TXT_NUMBER_TOP, 
			BkString::Get(IDS_DLG_UPDATE_COUNT_FORMAT), 
			nRecommend, nChoice );

		if (m_nCountUpdateIgnore == 0)
		{
			m_pDlg->SetItemVisible(IDC_UPDATE_TAB_TOP_BAR, FALSE);
		}
		else
		{
			m_pDlg->SetItemVisible(IDC_UPDATE_TAB_TOP_BAR, TRUE);
		}
	}
	else
	{
		if (nRecommend != 0)
		{
			CString strUpdateCount;
			strUpdateCount.Format(L"%d", nRecommend);
			m_pDlg->SetItemText(IDS_UPDATE_NUM_TEXT, strUpdateCount);
			m_pDlg->SetItemVisible(IDC_UPDATE_NUM, TRUE);
		}
		else
			m_pDlg->SetItemVisible(IDC_UPDATE_NUM, FALSE);

		//m_pDlg->SetItemText(IDC_SOFT_UPDATE_TXT_NUMBER_TOP, strUpdateCount);
		m_pDlg->FormatRichText(
			IDC_SOFT_UPDATE_TXT_NUMBER_TOP, 
			BkString::Get(IDS_DLG_UPDATE_COUNT_FORMAT), 
			nRecommend, nChoice );
		m_pDlg->SetItemVisible(IDC_UPDATE_TAB_TOP_BAR, TRUE);
	}
	BKSafeConfig::SetSoftUpdateCount( (DWORD)nRecommend, (DWORD)time(NULL) );

	return TRUE;
}

void CBeikeSafeSoftmgrUIHandler::UpdateDesktopIcon(int nNum)
{
	TCHAR dstPath[MAX_PATH] = {0};
	::SHGetSpecialFolderPath(NULL, dstPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
	::PathAddBackslash(dstPath);
	wcscat_s(dstPath, MAX_PATH, L"金山卫士软件管理");
	wcscat_s(dstPath, MAX_PATH, L".lnk");

	BOOL bDesktopIcon = FALSE;
	BOOL bDesktopIconNum = FALSE;
	BKSafeConfig::GetSoftUpdateDesktopIcon( bDesktopIcon, bDesktopIconNum );
	BKSafeConfig::SetDesktopIconNum(nNum);

	if (!bDesktopIcon)
		return ;

	if (!bDesktopIconNum)
		nNum = 0;

	if (!PathFileExists(dstPath))
	{
		bDesktopIcon = FALSE;
		BKSafeConfig::SetSoftUpdateDesktopIcon( bDesktopIcon, bDesktopIconNum );
		return;
	}

	CString  strPath;
	CAppPath::Instance().GetLeidianAppPath(strPath);
	strPath.Append(L"\\");

	CString strSoft;
	strSoft = strPath + L"KSafe.exe";
	CString strIcon;
	strIcon = strPath;

	if (nNum > 0)
	{
		CString strFile;
		if (nNum > 10)
			strFile = L"soft_10+.ico";
		else
			strFile.Format(L"soft_%d.ico", nNum);
		strIcon.Append(L"KSoft\\icon\\");
		strIcon.Append(strFile);

		if (!::PathFileExists(strIcon))
		{
			strIcon = strPath;
			strIcon.Append(L"KSoft\\softmgr.ico");
		}
	}
	else if (nNum == 0)
	{
		strIcon.Append(L"KSoft\\softmgr.ico");
	}

	WinMod::CWinPathApi::CreateLnkFile(strSoft, 
		L"-do:UI_SoftMgr_SoftUpdate",
		L"",
		dstPath,
		strIcon);
}

void CBeikeSafeSoftmgrUIHandler::_ChangeArrayToList(CSimpleArray<CSoftListItemData*>& arrIn, 
													CAtlList<CSoftListItemData*>& arrOut)
{
	CSoftListItemData *pData = NULL;
	int nCountIn = 0;

	nCountIn = (int)arrIn.GetSize();
	for (int i = 0; i<nCountIn; i++)
	{
		pData = arrIn[i];
		if ( !pData )
			continue;

		if (arrOut.GetCount() == 0)
			arrOut.AddHead(pData);
		else
			arrOut.AddTail(pData);
	}
}

void CBeikeSafeSoftmgrUIHandler::_SortArrayByOrder(CSimpleArray<CSoftListItemData*>& arrIn, 
												   CAtlList<CSoftListItemData*>& arrOut)
{
	int nOrder = 0;
	CSoftListItemData *pData = NULL;
	int nCountIn = 0;

	nCountIn = (int)arrIn.GetSize();
	for (int i = 0; i<nCountIn; i++)
	{
		pData = arrIn[i];
		if ( !pData )
			continue;

		nOrder = _wtoi(pData->m_strOrder);

		if (arrOut.GetCount() == 0)
			arrOut.AddHead(pData);
		else
		{
			CSoftListItemData *pDataTemp = NULL;
			int nOrderTemp = 0;
			POSITION posTemp =  arrOut.GetHeadPosition();
			BOOL bFind = FALSE;
			while (posTemp)
			{
				pDataTemp = arrOut.GetAt(posTemp);
				nOrderTemp = _wtoi(pDataTemp->m_strOrder);
				if (nOrderTemp < nOrder)
				{
					bFind = TRUE;
					break;
				}

				arrOut.GetNext(posTemp);
			}

			if (bFind)
				arrOut.InsertBefore(posTemp, pData);
			else
				arrOut.AddTail(pData);
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::_SortArrayByLastUpdate(CSimpleArray<CSoftListItemData*>& arrIn, 
														CAtlList<CSoftListItemData*>& arrOut)
{
	CString strPublish;
	CSoftListItemData *pData = NULL;
	int nCountIn = 0;

	nCountIn = (int)arrIn.GetSize();
	for (int i = 0; i<nCountIn; i++)
	{
		pData = arrIn[i];
		if ( !pData )
			continue;

		strPublish = pData->m_strPublished;

		if (arrOut.GetCount() == 0)
			arrOut.AddHead(pData);
		else
		{
			CSoftListItemData *pDataTemp = NULL;
			CString strPublishTemp;
			POSITION posTemp =  arrOut.GetHeadPosition();
			BOOL bFind = FALSE;
			while (posTemp)
			{
				pDataTemp = arrOut.GetAt(posTemp);
				strPublishTemp = pDataTemp->m_strPublished;
				if (strPublishTemp < strPublish)
				{
					bFind = TRUE;
					break;
				}

				arrOut.GetNext(posTemp);
			}

			if (bFind)
				arrOut.InsertBefore(posTemp, pData);
			else
				arrOut.AddTail(pData);
		}
	}
}

void CBeikeSafeSoftmgrUIHandler::_MoveMaijorFirst(CAtlList<CSoftListItemData*>& arrOut)
{
	CSoftListItemData *pDataMajor = NULL;
	POSITION posMajor =  arrOut.GetHeadPosition();
	while (posMajor)
	{
		pDataMajor = arrOut.GetAt(posMajor);
		if ((pDataMajor->m_attri&SA_Major) == SA_Major)
		{
			//
			CSoftListItemData *pDataNormal = NULL;
			POSITION posNormal =  arrOut.GetHeadPosition();
			while (posNormal)
			{
				pDataNormal = arrOut.GetAt(posNormal);
				if (pDataNormal == pDataMajor)
					break;

				if ((pDataNormal->m_attri&SA_Major) != SA_Major)
				{
					arrOut.SetAt(posNormal, pDataMajor);
					arrOut.SetAt(posMajor, pDataNormal);
					break;
				}

				arrOut.GetNext(posNormal);
			}
		}

		arrOut.GetNext(posMajor);
	}
}

void __stdcall CBeikeSafeSoftmgrUIHandler::SoftDataEvent(ksm::UninstEvent event, ksm::ISoftDataEnum *pEnum)
{
	switch(event)
	{
	case ksm::UE_Data_Failed:
		break;
	case ksm::UE_Data_Completed:
		{
			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_DATA_COMPLETED,NULL,NULL);

			ksm::SoftData sd = {0};
			BOOL bRetCode = pEnum->EnumFirst(&sd);

			while (bRetCode)
			{
				UniAddOrUpdateItemToList(sd);

				bRetCode = pEnum->EnumNext(&sd);
			}

			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_FRESH_TOTAL,NULL,NULL);
			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UNI_ALL_REFRESH_FINISH,NULL,NULL);
		}
		break;

	case ksm::UE_Data_Loaded:
		break;

	case ksm::UE_Modify:
	case ksm::UE_Update:
		{
			ksm::SoftData sd = {0};
			BOOL bRetCode = pEnum->EnumFirst(&sd);

			while (bRetCode)
			{
				SOFT_UNI_INFO *pInfo = NULL;
				pInfo = UniAddOrUpdateItemToList(sd);
				if (pInfo)
					m_UniExpandSoftList.RefreshRight(pInfo->strKey);

				bRetCode = pEnum->EnumNext(&sd);
			}
		}
		break;

	case ksm::UE_Add:
		{
			ksm::SoftData sd = {0};
			BOOL bRetCode = pEnum->EnumFirst(&sd);
			while (bRetCode)
			{
				UniAddOrUpdateItemToList(sd);

				if ( sd._id != 0 )
				{
					CString		strId;
					strId.Format(L"%d", sd._id);

					CSoftListItemData *pData = NULL;
					if (m_arrDataMap.Lookup(strId, pData))
					{

						if (pData)
						{
							pData->m_bSetup		= TRUE;
							m_pSoftChecker->CheckOneInstalled(pData->m_strSoftID, CheckSetupCallBack, this);
							_RefershItemBySoftIDWorkerThread( pData->m_strSoftID );
						}
					}
				}

				bRetCode = pEnum->EnumNext(&sd);
			}

			m_bUniRefresh = TRUE;
			m_pUninstall->DataRefresh(FALSE);
			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_FRESH_TOTAL,NULL,NULL);
		}
		break;

	case ksm::UE_Delete:
		{
			ksm::SoftData sd = {0};
			BOOL bRetCode = pEnum->EnumFirst(&sd);
			while (bRetCode)
			{
				SOFT_UNI_INFO *pInfo = NULL;
				if (m_arrSoftUniDataMap.Lookup(sd._pcKey) != NULL)
					pInfo = m_arrSoftUniDataMap.Lookup(sd._pcKey)->m_value;

				if (pInfo && pInfo->bEnable == FALSE)
				{
					pInfo->bEnable = TRUE;

					m_UniExpandSoftList.RefreshRight(pInfo->strKey);

					if ( !pInfo->strSoftID.IsEmpty() )
					{
						CSoftListItemData *pData = NULL;
						if (m_arrDataMap.Lookup(pInfo->strSoftID, pData))
						{

							if (pData)
							{
								pData->m_bSetup		= FALSE;
								pData->m_bUpdate	= FALSE;
								_RefershItemBySoftIDWorkerThread( pData->m_strSoftID );
							}
						}
					}
				}

				bRetCode = pEnum->EnumNext(&sd);
			}

			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_FRESH_TOTAL,NULL,NULL);
		}
		break;

	case ksm::UE_Refresh_Begin:
		break;
	case ksm::UE_Refresh_End:
		{
			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UNI_ALL_REFRESH_FINISH,NULL,NULL);
		}
		break;
	}
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnRefershUniAllSoftFinish( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (m_nUniType == UNI_TYPE_ALL && m_bUniRefresh == TRUE)
	{
		m_UniExpandSoftList.SetRedraw(FALSE);

		m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, FALSE );

		m_arrSoftUniDataSub.RemoveAll();
		for (int i=0; i < m_arrSoftUniData.GetSize(); i++)
		{
			SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniData[i];
			if (pInfo->bEnable == FALSE)
				m_arrSoftUniDataSub.Add(pInfo);
		}
		_ReloadTypeUni();
		_ReloadListUni();

		OnBtnSmrUniSortByName(); 
		ShowUniDataPage();

		m_UniExpandSoftList.SetRedraw(TRUE);

		m_bUniRefresh = FALSE;

		if ( m_pDlg->IsItemVisible(IDC_DLG_UNI_CONTENT,TRUE) )
			m_pDlg->SetFocus();
	}

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrUEDataCompleted( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_pDlg->SetItemVisible( IDC_DLG_PROG2, FALSE );

	m_pDlg->SetItemVisible( IDC_DLG_UNI_STARTPAGE, TRUE);
	ShowLoadingTip();

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrFreshTotal( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	FreshTotal();
	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrUESortByName( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	OnBtnSmrUniSortByName(); 

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrRefreshUniType( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	_ReloadTypeUni();

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrUEShowUniDataPage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ShowUniDataPage();

	return TRUE;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnSoftmgrUEHideHeaderArrow( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_pDlg->SetItemVisible( IDC_SMR_UNI_HEADER_ARROW, FALSE );
	return TRUE;
}

SOFT_UNI_INFO * CBeikeSafeSoftmgrUIHandler::UniAddOrUpdateItemToList(ksm::SoftData& sd)
{
	SOFT_UNI_INFO *pInfo = NULL;
	if (m_arrSoftUniDataMap.Lookup(sd._pcKey) != NULL)
		pInfo = m_arrSoftUniDataMap.Lookup(sd._pcKey)->m_value;
	else
	{
		pInfo = new SOFT_UNI_INFO;
		pInfo->strCaption = sd._pcDisplayName;

		m_arrSoftUniData.Add(pInfo);
		m_arrSoftUniDataMap.SetAt(sd._pcKey, pInfo);
	}

	if (pInfo)
	{
		pInfo->strKey = sd._pcKey;
		if (pInfo->strSoftID.IsEmpty() && sd._id != 0 )
			pInfo->strSoftID.Format(L"%u", sd._id);
		if ((sd._mask & ksm::SDM_PinYin)== ksm::SDM_PinYin)
		{
			pInfo->strSpellWhole = sd._pcSpellWhole;
			pInfo->strSpellAcronym = sd._pcSpellAcronym;
		}
		if ((sd._mask & ksm::SDM_Description)== ksm::SDM_Description)
		{
			pInfo->strDesc = sd._pcDescript;
			pInfo->strDesc.Trim();
		}
		if ((sd._mask & ksm::SDM_Size) == ksm::SDM_Size)
		{
			pInfo->strSize = sd._size > 0 ? CalcStrFormatByteSize( sd._size ) : L"未知";
			pInfo->nSize = sd._size;
		}

		if ((sd._mask & ksm::SDM_Info_Url) == ksm::SDM_Info_Url)
			pInfo->strInfoUrl = sd._pcInfoUrl;
		if ((sd._mask & ksm::SDM_Main_Path) == ksm::SDM_Main_Path)
			pInfo->strMainPath = sd._pcMainPath;
		if ((sd._mask & ksm::SDM_Count) == ksm::SDM_Count)
		{
			pInfo->strRunFrequency = CalcStrFormatByCount(sd._count);
			pInfo->nRunCount = sd._count;

			if (sd._count == -1)// 监控使用次数失败，使用LastTime转算
			{
				if ((sd._mask & ksm::SDM_LastUse) == ksm::SDM_LastUse)
				{
					//pInfo->strLastTime = sd._lastUse > 0 ? CalcStrFormatByteTime(sd._lastUse) : L"未知";
					//pInfo->nLastTime = sd._lastUse;
					
					GetRunInfoFromTime(pInfo->strRunFrequency, pInfo->nRunCount, sd._lastUse);
				}
			}
		}
		if ((sd._mask & ksm::SDM_Type) == ksm::SDM_Type)
			pInfo->strType = GetSoftType(sd._type);

		if (pInfo->pImg == NULL || pInfo->bDefaultImg)
		{
			// 使用网络Logo
			if(sd._id != 0)
			{
				CString strTemp;
				CString strIconPath;
				GetKSoftIconDir(strTemp);
				strIconPath.Format(L"%s\\%d.png", strTemp, sd._id);
				if ( ::PathFileExists(strIconPath) )
				{
					pInfo->bDefaultImg = FALSE;
					pInfo->pImg = Gdiplus::Image::FromFile(strIconPath, TRUE);
				}
			}

			// 抽取本地软件Logo
			if (pInfo->pImg == NULL && pInfo->hIcon == NULL && sd._pcIconLocation != NULL)
			{
				HICON	hLargeIcon = NULL;
				if ( ExtractIconFromSoftInfo(sd._pcIconLocation, &hLargeIcon, NULL) && hLargeIcon!=NULL)
				{
					pInfo->bDefaultImg = FALSE;
					pInfo->hIcon = hLargeIcon;
				}
			}

			// 使用默认Logo
			if (pInfo->pImg==NULL && pInfo->hIcon==NULL )
			{
				pInfo->bDefaultImg = TRUE;
				pInfo->pImg = m_pImge;
			}
		}

		pInfo->bEnable = FALSE;
	}

	return pInfo;
}

void __stdcall CBeikeSafeSoftmgrUIHandler::SoftDataSweepEvent(ksm::UninstEvent event, ksm::ISoftDataSweep *pSweep)
{
	//直接跳到卸载对话框中处理

	if ( m_dlgSoftMgrUninstall )
		m_dlgSoftMgrUninstall->SoftDataSweepEvent(event,pSweep);

	return;
}

void __stdcall CBeikeSafeSoftmgrUIHandler::SoftLinkEvent(ksm::UninstEvent event, ksm::SoftItemAttri type, ksm::ISoftLinkEnum *pData)
{
	switch(event)
	{
	case ksm::UE_Refresh_Begin:
		break;
	case ksm::UE_Refresh_End://扫描桌面、快速启动栏、开始菜单、进程以及托盘
		{
			//::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UNI_LINK_REFRESH_FINISH,(WPARAM)pData,NULL);

			if (m_nUniType == UNI_TYPE_START || 
				m_nUniType == UNI_TYPE_QUICK || 
				m_nUniType == UNI_TYPE_DESKTOP || 
				m_nUniType == UNI_TYPE_PROCESS || 
				m_nUniType == UNI_TYPE_TRAY)
			{
				if (pData == NULL)
				{
					m_UniExpandSoftList.SetRedraw(FALSE);

					m_UniExpandSoftList.ClearAll();
					::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_HIDE_HEADER_ARROW,NULL,NULL);

					m_arrSoftUniDataSub.RemoveAll();
					//_ReloadTypeUni();
					::PostMessage(m_pDlg->m_hWnd,WM_SOFT_REFRESH_UNI_TYPE,NULL,NULL);

					::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_SORT_BY_NAME,NULL,NULL);
					::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_SHOW_UNIDATAPAGE,NULL,NULL);

					m_UniExpandSoftList.SetRedraw(TRUE);
				}
				else
				{
					LPCWSTR pKey = NULL;
					BOOL bRetCode = pData->EnumFirst(&pKey);
					if (bRetCode)
					{
						m_UniExpandSoftList.SetRedraw(FALSE);
						::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_HIDE_HEADER_ARROW,NULL,NULL);

						m_arrSoftUniDataSub.RemoveAll();
						while (bRetCode)
						{
							SOFT_UNI_INFO *pInfo = NULL;
							if (m_arrSoftUniDataMap.Lookup(pKey) != NULL)
								pInfo = m_arrSoftUniDataMap.Lookup(pKey)->m_value;

							if (pInfo && pInfo->bEnable == FALSE)
								m_arrSoftUniDataSub.Add(pInfo);

							bRetCode = pData->EnumNext(&pKey);
						}
						//_ReloadTypeUni();
						::PostMessage(m_pDlg->m_hWnd,WM_SOFT_REFRESH_UNI_TYPE,NULL,NULL);
						_ReloadListUni();

						::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_SORT_BY_NAME,NULL,NULL);
						::PostMessage(m_pDlg->m_hWnd,WM_SOFT_UE_SHOW_UNIDATAPAGE,NULL,NULL);

						m_UniExpandSoftList.SetRedraw(TRUE);
					}
				}
			}
		}
		break;
	}
}

void CBeikeSafeSoftmgrUIHandler::_ReloadTypeUni(BOOL bChangeType /*= TRUE*/)
{
	CAtlMap<CString, int> arrTypeMap; 
	for(int i=0; i < (int)m_arrSoftUniDataSub.GetSize(); i++)
	{
		SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniDataSub[i];
		if(arrTypeMap.Lookup(pInfo->strType) == NULL)
			arrTypeMap.SetAt(pInfo->strType, 1);
		else
		{
			int nNum = arrTypeMap.Lookup(pInfo->strType)->m_value;
			nNum++;
			arrTypeMap.SetAt(pInfo->strType, nNum);
		}
	}

	CString strType;
	strType = m_strCurTypeUni;
	if (bChangeType)
		m_strCurTypeUni.Empty();
	else
	{
		if (strType.GetLength() != 0 && arrTypeMap.Lookup(strType))
			m_strCurTypeUni = strType;
		else
			m_strCurTypeUni.Empty();
	}

	CSimpleArray<CString> arrTypeMapUI;
	arrTypeMapUI.RemoveAll();
	//m_TypeUni.DeleteAllItems();

	int nIndex = 0;
	int nNum = 0;

	{
		strType = L"全部";
		if (m_strCurTypeUni.IsEmpty())
			m_strCurTypeUni = strType;
		nNum = m_arrSoftUniDataSub.GetSize();

		CString strTypeUI;
		strTypeUI = GetTypeUI(strType);
		arrTypeMapUI.Add(strTypeUI);
		//////////////////////////////////////////////////////////////////////////
		//m_TypeUni.InsertString(nIndex, strType );
		//CTypeListItemData * pListData = NULL;
		//pListData = new CTypeListItemData( 20, nIndex, COL_TYPE_NAME_NORMAL, strType, nNum);
		//m_TypeUni.SetItemDataX( nIndex, ( DWORD_PTR ) pListData );
		//m_TypeUni.SetItemHeight( nIndex, TYPE_ITEM_HEIGHT );

		nIndex++;
	}

	for (int i = 1; i < 23; i++)
	{
		strType = GetSoftType(i);
		if (arrTypeMap.Lookup(strType))
		{
			if (m_strCurTypeUni.IsEmpty())
				m_strCurTypeUni = strType;
			nNum = arrTypeMap.Lookup(strType)->m_value;

			CString strTypeUI;
			strTypeUI = GetTypeUI(strType);
			arrTypeMapUI.Add(strTypeUI);
			//////////////////////////////////////////////////////////////////////////
			//m_TypeUni.InsertString(nIndex, strType );
			//CTypeListItemData * pListData = NULL;
			//pListData = new CTypeListItemData( 20, nIndex, COL_TYPE_NAME_NORMAL, strType, nNum);
			//m_TypeUni.SetItemDataX( nIndex, ( DWORD_PTR ) pListData );
			//m_TypeUni.SetItemHeight( nIndex, TYPE_ITEM_HEIGHT );

			nIndex++;
		}
	}

	{
		strType = L"未知";
		if (arrTypeMap.Lookup(strType) && arrTypeMap.GetCount() > 1)
		{
			if (m_strCurTypeUni.IsEmpty())
				m_strCurTypeUni = strType;
			nNum = arrTypeMap.Lookup(strType)->m_value;

			CString strTypeUI;
			strTypeUI = GetTypeUI(strType);
			arrTypeMapUI.Add(strTypeUI);
			//////////////////////////////////////////////////////////////////////////
			//m_TypeUni.InsertString(nIndex, strType );
			//CTypeListItemData * pListData = NULL;
			//pListData = new CTypeListItemData( 20, nIndex, COL_TYPE_NAME_NORMAL, strType, nNum);
			//m_TypeUni.SetItemDataX( nIndex, ( DWORD_PTR ) pListData );
			//m_TypeUni.SetItemHeight( nIndex, TYPE_ITEM_HEIGHT );

			nIndex++;
		}
	}

	LoadUniType(arrTypeMapUI);
	//////////////////////////////////////////////////////////////////////////
	//m_TypeUni.SetCallBack( this );
	//m_TypeUni.SetListID(IDC_XIEZAI_TYPES_LIST);
	//m_TypeUni.SetCurSel( 0 );

	//m_TypeUni.SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
	//m_TypeUni.Invalidate();
}

void CBeikeSafeSoftmgrUIHandler::LoadUniType(CSimpleArray<CString>& soft)
{
	for (int i = IDC_SOFTMGR_XIEZAI_TYPE_LINE_FIRST; i <= IDC_SOFTMGR_XIEZAI_TYPE_LINE_LAST; i++)
	{
		m_pDlg->SetItemVisible(i, FALSE);
	}

	for (int i = IDC_SOFTMGR_XIEZAI_TYPE_FIRST; i <= IDC_SOFTMGR_XIEZAI_TYPE_LAST; i++)
	{
		m_pDlg->SetItemVisible( i + 1000, FALSE);
		m_pDlg->SetItemAttribute( i, "class","typenormal" );
		m_pDlg->SetItemVisible(i, FALSE);
	}

	for (int i=0; i < soft.GetSize(); i++)
	{
		CString strType;
		strType = soft[i];

		m_pDlg->SetItemText(i + IDC_SOFTMGR_XIEZAI_TYPE_FIRST, strType);
		m_pDlg->SetItemVisible(i + IDC_SOFTMGR_XIEZAI_TYPE_FIRST, TRUE);

		if (i % 2 == 0)
			m_pDlg->SetItemVisible(i / 2 + IDC_SOFTMGR_XIEZAI_TYPE_LINE_FIRST, TRUE);
	}

	m_nCurTypeUni = 0;
	m_pDlg->SetItemAttribute( m_nCurTypeUni + IDC_SOFTMGR_XIEZAI_TYPE_FIRST, "class","typesel" );
	m_pDlg->SetItemVisible( m_nCurTypeUni + IDC_SOFTMGR_XIEZAI_TYPE_BK_FIRST, TRUE);
}

void CBeikeSafeSoftmgrUIHandler::_ReloadListUni()
{
	if (m_strCurTypeUni.IsEmpty() == FALSE)
	{
		//m_TypeUni.SetCurSelTypeName(m_strCurTypeUni);

		BOOL bQueryMark = FALSE;
		m_UniExpandSoftList.ClearAll();

		for(int i=0; i < (int)m_arrSoftUniDataSub.GetSize(); i++)
		{
			SOFT_UNI_INFO* pInfo = (SOFT_UNI_INFO*)m_arrSoftUniDataSub[i];
			if (m_strCurTypeUni.CompareNoCase(L"全部") == 0 ||
				(m_strCurTypeUni.CompareNoCase(L"全部") != 0 && pInfo->strType.CompareNoCase(m_strCurTypeUni) == 0))
			{
				if (pInfo->fMark == 0 && m_pInfoQuery)
				{
					int nTmp = _wtoi( pInfo->strSoftID );
					m_pInfoQuery->AddSoft(nTmp);
					bQueryMark = TRUE;
				}

				m_UniExpandSoftList.InsertItemX(pInfo);
			}
		}

		if (bQueryMark && m_pInfoQuery)
			m_pInfoQuery->Query();

		::PostMessage(m_pDlg->m_hWnd, MSG_APP_UPDATE_UNI_TYPE, 0, 0);
	}
}

void CBeikeSafeSoftmgrUIHandler::DoBibeiCmd(LPCTSTR lpstrCmd)
{
	CString strCmd;
	strCmd = lpstrCmd;

	CString strTemp;
	CString strKey;
	CString strValue;
	int nSpliter = strCmd.Find(L';');
	if (-1 == nSpliter)
		nSpliter = strCmd.GetLength();

	strTemp = strCmd.Left(nSpliter);
	int nEqual = strTemp.Find(L'=');
	if (-1 != nEqual)
	{
		strKey = strTemp.Left(nEqual);
		strValue = strTemp.Mid(nEqual + 1);
	}

	if (strKey.CompareNoCase(L"type") == 0 )
	{
		m_necessUIHandler.BibeiChangeType(strValue);
	}
}

void CBeikeSafeSoftmgrUIHandler::DoDaquanCmd(LPCTSTR lpstrCmd)
{
	CString strCmd;
	strCmd = lpstrCmd;

	CString strTemp;
	CString strKey;
	CString strValue;
	int nSpliter = strCmd.Find(L';');
	if (-1 == nSpliter)
		nSpliter = strCmd.GetLength();

	strTemp = strCmd.Left(nSpliter);
	int nEqual = strTemp.Find(L'=');
	if (-1 != nEqual)
	{
		strKey = strTemp.Left(nEqual);
		strValue = strTemp.Mid(nEqual + 1);
	}

	if (strKey.CompareNoCase(L"type") == 0 )
	{
		//m_SoftTypeList.SetCurSelTypeName(strValue, TRUE);
		int nType = GetDaquanType(strValue);

		m_pDlg->SetItemAttribute( m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_FIRST, "class","typenormal" );
		m_nCurType = nType;
		m_pDlg->SetItemAttribute( m_nCurType + IDC_SOFTMGR_DAQUAN_TYPE_FIRST, "class","typesel" );

		OnClickDaquanType(nType);
	}
	else if (strKey.CompareNoCase(L"detail") == 0 )
	{
		OnQuerySoftByID(strValue);

		if (m_list.GetItemCount() == 1)
			OnViewDetail(0);
	}
}

void CBeikeSafeSoftmgrUIHandler::OnQuerySoftByID(LPCTSTR lpstrSoftId)
{
	OnBack();

	m_bQuery = TRUE;

	m_pDlg->SetItemVisible( IDC_LIST_SOFTMGR, FALSE );

	//读取
	CSimpleArray<CSoftListItemData*> arrQuery;
	CString strText = lpstrSoftId;
	CSoftListItemData *pData = NULL;
	pData = GetDataBySoftID(strText);
	if (pData)
		arrQuery.Add(pData);

	//排序
	CAtlList<CSoftListItemData*> arrOrder;
	_SortArrayByOrder(arrQuery, arrOrder);

	//显示
	m_list.ClearAllItemData();
	ShowDataToList(arrOrder);

	m_pDlg->SetItemVisible(IDR_FIND_QUERY_SOFTMGR, TRUE);
	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_TYPE, FALSE);
	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_COUNT, FALSE);

	m_pDlg->SetItemVisible(IDC_SMR_DAQUAN_HEADER_ARROW, FALSE );
	m_pDlg->SetItemAttribute(IDC_DAQUAN_SOFTLIST_HEADER, "pos", "0,0,-0,24");
	m_pDlg->SetItemAttribute(IDC_LIST_SOFTMGR, "pos", "0,24,-0,-0");

	if (m_list.GetItemCount() == 0)
	{
		m_pDlg->FormatRichText(
			IDS_TIP_QUERY_SOFTMGR, 
			BkString::Get(IDS_NO_QUERY_SOFTMGR), 
			strText);
		CString str;
		str.Format(	BkString::Get(IDS_SOFTMGR_8051), strText);

		m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, TRUE);
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, FALSE);

	}
	else
	{
		m_pDlg->FormatRichText(
			IDS_TIP_QUERY_SOFTMGR, 
			L"以下是为您找到的相关软件，共 %d 款", 
			m_list.GetItemCount());
		m_pDlg->SetItemVisible(IDS_NOFIND_QUERY_SOFTMGR, FALSE);
		m_pDlg->SetItemVisible(IDC_LIST_SOFTMGR, TRUE);
	}

	CRect rcWnd;
	m_list.GetWindowRect(&rcWnd);
	OnListReSize(rcWnd);
}

void __stdcall CBeikeSafeSoftmgrUIHandler::SoftRubbishEvent(ksm::UninstEvent event, ksm::ISoftRubbishEnum *pEnum)
{
	switch(event)
	{
	case ksm::UE_Update:
		{
			ksm::SoftRubbish sr;
			BOOL bRetCode = FALSE;
			bRetCode = pEnum->EnumFirst(&sr);
			if (bRetCode)
			{
				int	nType = sr._type;
				for (int i=0; i < m_arrRubData.GetSize(); )
				{
					SOFT_RUB_INFO* pRubInfo = (SOFT_RUB_INFO*)m_arrRubData[i];
					if (pRubInfo && pRubInfo->nType == nType)
					{
						delete pRubInfo;
						m_arrRubData.RemoveAt(i);
					}
					else
						i++;
				}
			}
			bRetCode = pEnum->EnumFirst(&sr);
			while (bRetCode)
			{
				SOFT_RUB_INFO *pRubInfo = new SOFT_RUB_INFO;
				pRubInfo->nType = sr._type;
				pRubInfo->strData = sr._pData;

				m_arrRubData.Add(pRubInfo);

				bRetCode = pEnum->EnumNext(&sr);
			}
			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_REFRESH_CANLIU,NULL,NULL);
		}
		break;
	case ksm::UE_Refresh_Begin:
		break;
	case ksm::UE_Refresh_End:
		{
			for (int i=0; i < m_arrRubData.GetSize(); i++)
			{
				SOFT_RUB_INFO* pRubInfo = (SOFT_RUB_INFO*)m_arrRubData[i];
				if (pRubInfo)
				{
					delete pRubInfo;
					pRubInfo = NULL;
				}
			}
			m_arrRubData.RemoveAll();


			if (pEnum)
			{
				ksm::SoftRubbish sr;
				BOOL bRetCode = pEnum->EnumFirst(&sr);
				while (bRetCode)
				{
					SOFT_RUB_INFO *pRubInfo = new SOFT_RUB_INFO;
					pRubInfo->nType = sr._type;
					pRubInfo->strData = sr._pData;

					m_arrRubData.Add(pRubInfo);

					bRetCode = pEnum->EnumNext(&sr);
				}
			}	

			::PostMessage(m_pDlg->m_hWnd,WM_SOFT_REFRESH_CANLIU,NULL,NULL);
		}
		break;
	}
}

void __stdcall CBeikeSafeSoftmgrUIHandler::SoftRubbishSweepEvent(ksm::UninstEvent event, ksm::ISoftRubbishSweep *pSweep)
{
	if (m_pPowerSweepDlg != NULL)
	{
		m_pPowerSweepDlg->SoftRubbishSweepEvent(event,pSweep);
		return;
	}

	switch(event)
	{
	case ksm::UE_Sweep_Begin:
		break;
	case ksm::UE_Sweeping:
		break;
	case ksm::UE_Sweep_End:
		//		m_pSoftRubbishSweep = NULL;
		break;
	}
}

void __stdcall CBeikeSafeSoftmgrUIHandler::SoftQueryNotify(ksm::ISoftInfoEnum *pEnum, void *pParams)
{
	if (pEnum)
	{
		ksm::SoftInfo sr;
		BOOL bRetCode = pEnum->EnumFirst(&sr);
		while (bRetCode)
		{
			float fMark = _wtof(sr._pcGrade);
			CString strSoftId;
			strSoftId.Format(L"%d", sr._id);

			BOOL bRefresh = FALSE;
			CSoftListItemData *pData = NULL;
			if(m_arrDataMap.Lookup(strSoftId, pData))
			{
				pData->m_fMark = fMark;
				bRefresh = TRUE;
			}
			for ( int i=0; i<m_arrSoftUniData.GetSize(); i++ )
			{
				SOFT_UNI_INFO * pInfo = m_arrSoftUniData[i];
				if (pInfo->strSoftID.CompareNoCase(strSoftId) == 0 && pInfo->fMark != fMark)
				{
					pInfo->fMark = fMark;
					bRefresh = TRUE;
				}
			}
			if (bRefresh)
				_RefershItemBySoftIDWorkerThread(strSoftId);

			bRetCode = pEnum->EnumNext(&sr);
		}
	}
}


VOID CBeikeSafeSoftmgrUIHandler::_RefershItemBySoftIDMainThread( CString strSoftID )
{
	if (m_list.IsWindowVisible())
	{
		for (int i = 0; i < m_list.GetItemCount(); i++)
		{
			CSoftListItemData *pItemData = m_list.GetItemDataEx(i);
			if (pItemData != NULL && pItemData->m_strSoftID == strSoftID)
			{
				m_list.RefreshItem(i);
				break;
			}
		}
	}
	if (m_UpdateSoftList.IsWindowVisible())
	{
		m_UpdateSoftList.RefreshRight(strSoftID);
		m_UpdateSoftList.RefreshIcon(strSoftID);
	}
	m_necessUIHandler.RefreshItemBySoftID(strSoftID);
	if (m_UniExpandSoftList.IsWindowVisible())
	{
		for ( int i=0; i<m_arrSoftUniData.GetSize(); i++ )
		{
			SOFT_UNI_INFO * pInfo = m_arrSoftUniData[i];
			if (pInfo->strSoftID.CompareNoCase(strSoftID) == 0)
				m_UniExpandSoftList.RefreshRight(pInfo->strKey);
		}
	}
}
VOID CBeikeSafeSoftmgrUIHandler::_RefershItemBySoftIDWorkerThread( CString strSoftID )
{
	CString* pString = new CString(strSoftID);
	::PostMessage(m_pDlg->m_hWnd, WM_SOFT_REFRESH_ITEM,(WPARAM)pString,NULL);
}
LRESULT CBeikeSafeSoftmgrUIHandler::OnRefershItemBySoftID( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CString* pString = (CString*)wParam;
	_RefershItemBySoftIDMainThread(*pString);
	delete pString;
	return S_OK;
}
LRESULT CBeikeSafeSoftmgrUIHandler::OnRefershRubbishInfo( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (m_arrRubData.GetSize())
	{
		m_pDlg->FormatRichText(
			IDC_DLG_RESIDUAL_COUNT, 
			BkString::Get(IDS_DLG_RESIDUAL_COUNT), 
			m_arrRubData.GetSize() );
		m_pDlg->SetItemVisible( IDC_DLG_RESIDUAL, TRUE );
	}
	else
		m_pDlg->SetItemVisible( IDC_DLG_RESIDUAL, FALSE );

	return S_OK;
}


CBkNavigator* CBeikeSafeSoftmgrUIHandler::OnNavigate( CString &strChildName )
{
	if ( strChildName.Find(BKSFNS_SOFTMGR_PAGE_ZJBB) != -1 )
	{
		m_pDlg->SetTabCurSel(IDC_TAB_SOFTMGR, TAB_INDEX_BIBEI);

		int nSpliter = strChildName.Find(L':');
		if (-1 != nSpliter)
		{ 
			m_strBibeiCmd = strChildName.Mid(nSpliter + 1);
			if (m_bInitData)
				DoBibeiCmd(m_strBibeiCmd);
		}
	}else if (strChildName.Find(BKSFNS_SOFTMGR_PAGE_DAQUAN) != -1)
	{
		m_pDlg->SetTabCurSel(IDC_TAB_SOFTMGR, TAB_INDEX_DAQUAN);

		int nSpliter = strChildName.Find(L':');
		if (-1 != nSpliter)
		{ 
			m_strDaquanCmd = strChildName.Mid(nSpliter + 1);
			if (m_bInitData)
				DoDaquanCmd(m_strDaquanCmd);
		}
	}else if ( 0 == strChildName.CompareNoCase(BKSFNS_SOFTMGR_PAGE_UNINSTALL) )
	{
		m_pDlg->SetTabCurSel(IDC_TAB_SOFTMGR, TAB_INDEX_UNINSTALL);
	}else if (strChildName.Find(BKSFNS_SOFTMGR_PAGE_UPDATE) != -1)
	{
		m_pDlg->SetTabCurSel(IDC_TAB_SOFTMGR, TAB_INDEX_UPDATE);

		int nSpliter = strChildName.Find(L':');
		if (-1 != nSpliter)
		{ 
			CString strSoftId;
			strSoftId = strChildName.Mid(nSpliter + 1);
			m_nSoftIdByCmd = _wtoi( strSoftId );
			if (m_bInitUpdate == TRUE)
			{
				CString strSoftId;
				strSoftId.Format(L"%d", m_nSoftIdByCmd);
				m_UpdateSoftList.UpdateSoft(strSoftId);
			}
		}
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SOFTMGR_PAGE_HOTGAME))
	{
		m_pDlg->SetTabCurSel(IDC_TAB_SOFTMGR, TAB_INDEX_GAME);
	}

	return NULL;
}

CString CBeikeSafeSoftmgrUIHandler::GetSoftType(LONG nType)
{
	CString strResult;

	switch(nType)
	{
	case 1:	strResult = L"聊天工具";	break;
	case 2:	strResult = L"视频播放";	break;
	case 3:	strResult = L"浏览器";	break;
	case 4:	strResult = L"音乐播放";	break;
	case 5:	strResult = L"下载工具";	break;
	case 6:	strResult = L"游戏对战";	break;
	case 7:	strResult = L"图形图像";	break;
	case 8:	strResult = L"杀毒防护";	break;
	case 9:	strResult = L"输入法";	break;
	case 10:	strResult = L"股票网银";	break;
	case 11:	strResult = L"文字处理";	break;
	case 12:	strResult = L"翻译词典";	break;
	case 13:	strResult = L"压缩刻录";	break;
	case 14:	strResult = L"系统工具";	break;
	case 15:	strResult = L"编程开发";	break;
	case 16:	strResult = L"数码编辑";	break;
	case 17:	strResult = L"邮件";	break;
	case 18:	strResult = L"学习考试";	break;
	case 19:	strResult = L"网络应用";	break;
	case 20:	strResult = L"其他";	break;
	case 21:	strResult = L"驱动";	break;
	case 22:	strResult = L"桌面主题";	break;
	default: strResult = L"未知";
	}
	return strResult;
}

void CBeikeSafeSoftmgrUIHandler::_QuerySoftMark( LPCTSTR lpstrSoftID )
{
	if (m_pInfoQuery)
	{
		m_pInfoQuery->AddSoft( _ttoi(lpstrSoftID) );
		m_pInfoQuery->Query();
	}
}

BOOL CBeikeSafeSoftmgrUIHandler::_InstallSoftExistFile( CSoftListItemData* pData )
{
	CString strStore;
	BKSafeConfig::GetStoreDir(strStore);
	if(strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
	{
		strStore.Append(_T("\\")+ pData->m_strFileName);
	}
	else
	{
		strStore.Append(pData->m_strFileName);
	}

	if ( PathFileExists(strStore) )
	{
		installParam *pParam = new installParam;
		pParam->pData = pData;
		pParam->strPath = strStore;
		m_cs.Lock();
		m_arrInstall.AddTail(pParam);
		m_cs.Unlock();

		if (m_hInstall == (HANDLE)-1)
		{
			m_hInstall = (HANDLE)_beginthreadex(NULL, 0, InstallProc, this, 0, NULL);
		}
		pData->m_bInstalling = FALSE;
		pData->m_bWaitInstall = TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL CBeikeSafeSoftmgrUIHandler::CheckSoftCanInstall( CSoftListItemData* pData, BOOL bFromDownOK )
{
	return (BOOL)::SendMessage(m_pDlg->m_hWnd, WM_SOFT_INSTALL_CHECK, (WPARAM)pData, (LPARAM)bFromDownOK);
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnCheckSoftCanInstall( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CSoftListItemData*	pData		= (CSoftListItemData*)wParam;
	BOOL				bFromDownOK	= (BOOL)lParam;
	BOOL				bRet		= TRUE;

	if (bFromDownOK)
	{
		if (m_necessUIHandler.m_pOnekeyDlg != NULL)
		{
			if ( !m_necessUIHandler.m_pOnekeyDlg->CheckSoftCanInstall(pData,bFromDownOK) )
				bRet = FALSE;
			else
				bRet = BKSafeConfig::GetAutoInstall();
		}
		else
			bRet = BKSafeConfig::GetAutoInstall();
	}
	else
	{
		bRet = TRUE;
	}

	return (LRESULT)bRet;
}

LRESULT CBeikeSafeSoftmgrUIHandler::OnNotifySoftInstall( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CSoftListItemData*	pData = (CSoftListItemData*)wParam;


	if ( m_necessUIHandler.m_pOnekeyDlg != NULL)
	{
		m_necessUIHandler.m_pOnekeyDlg->NotifySoftInstallComplete(pData);
	}

	return S_OK;
}

void CBeikeSafeSoftmgrUIHandler::OnKeyDownReturn(MSG* pMsg)
{
	if (IsWindowVisible(m_IEPhoneNess.m_hWnd))
		_IsChildCtrlMessage(pMsg, m_IEPhoneNess.m_hWnd);
}

BOOL CBeikeSafeSoftmgrUIHandler::_IsChildCtrlMessage(MSG* pMsg, HWND hParent)
{
	if (!m_pDlg)
		return FALSE;

	HWND hWndCtl = ::GetFocus();
	if(::IsChild(m_pDlg->m_hWnd, hWndCtl))
	{
		while(::GetParent(hWndCtl) != hParent)
			hWndCtl = ::GetParent(hWndCtl);

		if(::SendMessage(hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg) != 0)
			return TRUE;
	}

	return FALSE;
}

VOID CBeikeSafeSoftmgrUIHandler::QuerySoftMark( LPCTSTR lpstrSoftID )
{
	_QuerySoftMark(lpstrSoftID);
}

void CBeikeSafeSoftmgrUIHandler::NotifySoftUpdateCount(int cnt)
{
	static const wchar_t sIniFile[]		= L"\\Cfg\\bksoftmgr.ini";
	static const wchar_t sTipSec[]		= L"update_tip";
	static const wchar_t sTipCount[]	= L"tip_count";
	static const wchar_t sTipCountTime[]= L"tip_count_time";

	// 保存数量
	CString iniPath;
	GetKSafeDirectory(iniPath); iniPath += sIniFile;

	CString strTipCount; strTipCount.Format(L"%d", cnt);
	::WritePrivateProfileString(sTipSec, sTipCount, strTipCount, iniPath);

	CString strTipCountTime; strTipCountTime.Format(L"%d", static_cast<int>(_time32(NULL)));
	::WritePrivateProfileString(sTipSec, sTipCountTime, strTipCountTime, iniPath);

	// 通知体检
	//::SendMessage(m_pDlg->m_hWnd, MSG_APP_EXAM_SCORE_CHANGE, 0, 0);
	m_pDlg->RemoveFromTodoList(BkSafeExamItem::EXAM_CHECK_UPDATE);
}

namespace
{

	BOOL ExtractIconFromSoftInfo(LPCWSTR pcIconPath, HICON *phiconLarge, HICON *phiconSmall)
	{
		if(pcIconPath == NULL) return FALSE;

		wstring path;

		//
		// 扩展环境变量
		//
		if(wcschr(const_cast<LPWSTR>(pcIconPath), L'%') != NULL)
		{
			// 形如：%ProgramFiles%\Synaptics\SynTP\InstNT.exe
			DWORD size = ::ExpandEnvironmentStringsW(pcIconPath, NULL, 0);
			if(size == 0) return FALSE;

			auto_buffer<wchar_t> buffer(size + 1);
			DWORD ret = ::ExpandEnvironmentStringsW(pcIconPath, buffer.data(), size + 1);
			if(ret == 0 || ret >= size + 1) return FALSE;

			path = buffer.data();
		}
		else
		{
			path = pcIconPath;
		}

		//
		// 提取图标
		//
		if(0xFFFFFFFF != ::GetFileAttributesW(path.c_str()))
		{
			// 形如：C:\Program Files\Youdao\Dict\RunDict.exe
			return (::ExtractIconExW(path.c_str(), 0, phiconLarge, phiconSmall, 1) == 1);
		}
		else
		{
			// 形如：C:\Program Files\Youdao\Dict\RunDict.exe,0
			// c:\Program Files\Microsoft Visual Studio 8\Setup\setup.ico
			wstring::size_type pos = path.find_last_of(L',');
			if(pos == wstring::npos) return 0;

			int index = _wtoi(&path[pos+1]);

			path.resize(pos);
			return (::ExtractIconExW(path.c_str(), index, phiconLarge, phiconSmall, 1) == 1);
		}
	}

	void PreprocessKeyword(const CString &input, CString &output)
	{
		//
		// 对搜索关键字进行预处理
		//
		// 1）两端加*
		// 2）替换空格（包括中英文）
		// 3）合并重复*
		//

		output = L"*";
		int state = 1; // 初始化为：*与空格状态，2为其它字符状态

		LPCWSTR p = input;
		while(TRUE)
		{
			if(state == 1)
			{
				if(*p == L'\0')
				{
					break;
				}
				else if(*p != L'*' && *p != L' ' && *p != L'　')
				{
					output += *p;
					state = 2;
				}
			}
			else
			{
				if(*p == L'\0')
				{
					output += L'*';
					break;
				}
				if(*p == L'*' || *p == L' ' || *p == L'　')
				{
					output += L'*';
					state = 1;
				}
				else
				{
					output += *p;
				}
			}

			++p;
		}
	}

	inline CString _PathAddBackslash(const CString &path)
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

	inline LONG safe_atol(LPCWSTR pStr, LONG def /* = 0 */)
	{
		if(pStr == NULL) return def;
		return _wtol(pStr);
	}

}

void CSoftUpdatedList::Startup(const CString &kSafePath)
{
	_csList.Init();
	_filePath = kSafePath + _T("\\AppData\\ksoft_updated_list");

	wifstream is(_filePath);
	if(!is) return;

	__time32_t now = _time32(NULL);

	while(true)
	{
		SoftInfo softInfo;
		is>>softInfo._id>>softInfo._time;

		if(softInfo._id == 0) break;
		__time32_t interval = abs(now - softInfo._time);
		if(interval > 24*60*60) continue;

		_softInfoList.Add(softInfo);
	}
}

void CSoftUpdatedList::Add(int softId)
{
	_csList.Lock();
	_softInfoList.Add(SoftInfo(softId, _time32(NULL)));
	_csList.Unlock();
}

BOOL CSoftUpdatedList::InList(int softId)
{
	BOOL exist = FALSE;

	_csList.Lock();
	for(size_t i = 0; i < _softInfoList.GetCount(); ++i)
	{
		if(_softInfoList[i]._id == softId)
		{
			exist = TRUE;
			break;
		}
	}
	_csList.Unlock();

	return exist;
}

void CSoftUpdatedList::Cleanup()
{

	wofstream os(_filePath);
	if(os)
	{
		for(size_t i = 0; i < _softInfoList.GetCount(); ++i)
		{
			os<<_softInfoList[i]._id<<L'\t'<<_softInfoList[i]._time<<L'\t';
		}
	}

	_csList.Term();
}

#if PHONE_NESS_SWITCH
//
// 手机必备
//
//////////////////////////////////////////////////////////////////////////
namespace
{

	const DISPID BASE_DIPSPID = 1000;
	const struct
	{
		LPCWSTR			_pFuncName;		// 名称
		DISPID			_dispId;		// 分发ID
		UINT			_argc;			// 函数参数个数（必须不BSTR类型）
		HRESULT (CBeikeSafeSoftmgrUIHandler::*_pFuncProc)(DISPID, DISPPARAMS*, VARIANT*);
	} 
	sPhoneJsFunc[] = 
	{
		//
		//@Note
		// _dispId必须从BASE_DIPSPID递增
		//
		{L"SoftExists",		BASE_DIPSPID + 0,	1,	&CBeikeSafeSoftmgrUIHandler::PhoneSoftExists},
		{L"StartDown",		BASE_DIPSPID + 1,	5,	&CBeikeSafeSoftmgrUIHandler::PhoneStartDown},
		{L"PauseDown",		BASE_DIPSPID + 2,	1,	&CBeikeSafeSoftmgrUIHandler::PhonePauseDown},
		{L"ResumeDown",		BASE_DIPSPID + 3,	1,	&CBeikeSafeSoftmgrUIHandler::PhoneResumeDown},
		{L"StopDown",		BASE_DIPSPID + 4,	1,	&CBeikeSafeSoftmgrUIHandler::PhoneStopDown},
		{L"GetDownState",	BASE_DIPSPID + 5,	1,	&CBeikeSafeSoftmgrUIHandler::PhoneGetDownState},
		{L"OpenDownedSoft",	BASE_DIPSPID + 6,	1,	&CBeikeSafeSoftmgrUIHandler::PhoneOpenDownedSoft},
		{L"NavigatePage",	BASE_DIPSPID + 7,	1,	&CBeikeSafeSoftmgrUIHandler::PhoneNavigatePage},
		{L"ReportInfo",		BASE_DIPSPID + 8,	1,	&CBeikeSafeSoftmgrUIHandler::ReportInfo},
	};

}
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CBeikeSafeSoftmgrUIHandler::QueryInterface(REFIID riid, void **ppvObject)
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

STDMETHODIMP CBeikeSafeSoftmgrUIHandler::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
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

STDMETHODIMP CBeikeSafeSoftmgrUIHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
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

void CBeikeSafeSoftmgrUIHandler::PhoneSoftDownCallback(DTManager_Stat st,void* tk,void* para)
{
	CBeikeSafeSoftmgrUIHandler *pDlg = reinterpret_cast<CBeikeSafeSoftmgrUIHandler*>(para);
	pDlg->PhoneSoftDownProcess(st, tk);
}

void CBeikeSafeSoftmgrUIHandler::PhoneSoftDownInfoCallback(__int64 cur,__int64 all,__int64 speed,DWORD time,void* para)
{
	PPhoneSoft pPhoneSoft = reinterpret_cast<PPhoneSoft>(para);

	pPhoneSoft->speed = static_cast<ULONG>(speed);
	pPhoneSoft->recved = static_cast<ULONG>(cur);
}

void CBeikeSafeSoftmgrUIHandler::PhoneSoftDownProcess(DTManager_Stat st, void *tk)
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

HRESULT CBeikeSafeSoftmgrUIHandler::PhoneSoftExists(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

HRESULT CBeikeSafeSoftmgrUIHandler::PhoneStartDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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
		::SHCreateDirectory(NULL, strStore);

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

HRESULT CBeikeSafeSoftmgrUIHandler::PhonePauseDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

HRESULT CBeikeSafeSoftmgrUIHandler::PhoneResumeDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

HRESULT CBeikeSafeSoftmgrUIHandler::PhoneStopDown(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

HRESULT CBeikeSafeSoftmgrUIHandler::PhoneGetDownState(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

	return S_OK;
}

HRESULT CBeikeSafeSoftmgrUIHandler::PhoneOpenDownedSoft(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

HRESULT CBeikeSafeSoftmgrUIHandler::ReportInfo(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	CComVariant retVal;
	if(pDispParams->rgvarg[0].bstrVal == NULL || pDispParams->rgvarg[0].bstrVal[0] == L'\0')
	{
		retVal = false;
	}
	else
	{
		retVal = true;
	}

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}

	return S_OK;
}


HRESULT CBeikeSafeSoftmgrUIHandler::PhoneNavigatePage(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{	
	CComVariant retVal;
	if(pDispParams->rgvarg[0].bstrVal == NULL || pDispParams->rgvarg[0].bstrVal[0] == L'\0')
	{
		retVal = false;
	}
	else
	{
		CString strCmd;
		strCmd = pDispParams->rgvarg[0].bstrVal;
		_Module.Navigate(strCmd);

		retVal = true;
	}

	if(pVarResult != NULL)
	{
		retVal.Detach(pVarResult);
	}

	return S_OK;
}

IDTManager* CBeikeSafeSoftmgrUIHandler::GetDTMgrForPhone()
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

#endif

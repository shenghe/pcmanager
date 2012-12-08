#pragma once
#include <safeexam/beikesafeexam.h>



class CBkSafeExamCaller
    : public IBkSafeExamineCallback
{
public:
	CBkSafeExamCaller();
	~CBkSafeExamCaller();

	IBkSafeExaminer *m_piExaminer;
	HMODULE m_hExamModule;
    void	Start(BOOL bExpress, HWND hWndNotify, UINT uMsgNotifyItem, UINT uMsgNotifyProgress);
	void	FixItem(int nItemID, void* pData = NULL);	//修复
//	void	SetIgnoreItems(CSimpleArray<int> arrItems);//增加忽略项目
//	void	ResetIgnoreArray();
	int		GetItemCount(int nItemID, int& nCount);
	int		EnumItemData(int nItemID, int nIndex, void** pData);
	int		SetItemSafe(int nItemID, void** pData);
	void	SetHostItemFix(int nIndex);
	int		GetUnSafeMonitorCount();
	BOOL	IsWin64();
	void	SetAllMonitorRun(BOOL bRun= TRUE);
    void	_SetAllMonitorRun(BOOL bRun= TRUE);
	int		IsSafeMonitorAllRun();
    int     _IsSafeMonitorAllRun();
	int		RemoveLocalLibItem(int nItemID, void* pValue);
	int		GetSafeLibItemCount(int& nCount);
	int		EnumSafeLibItem(int nIndex, BkSafeExamLib::LocalLibExport& libItem);
	int		LocalLibIsHaveItem(int nItemID, BOOL& bHave);
	int		StartupAnUse(int nItemID);
    void	Echo(HWND hWndNotify, UINT uMsgNotify);
	BOOL	GetResultItem(int nOrder, BKSAFE_EXAM_RESULT_ITEM &item);
	BOOL	RemoveItem(int nItemID);
	BOOL	RemoveItem2(REMOVE_EXAM_RESULT_ITEM RemoveExamItem);
	int		GetItemLevel(int nItemID);
	int		LoadExamModule();
	void	UnLoadExamModule();
	int		ConvertKWSIDToExamID(int nIndex);
	int     SetNotifyHwndToTrash();
	int		ExamItem(int nItemID, BKSafeExamItemEx::BKSafeExamEx& ExamItemData);

    void	Stop()
    {
        m_bExamCanceled = TRUE;

        if (m_hEventStop)
            ::SetEvent(m_hEventStop);
    }
	BOOL	Canceled()
	{
		return m_bExamCanceled;
	}


#if 0
    BOOL HasRisk()
    {
        return m_bHasRisk;
    }
#endif


#if 0
    BOOL HasCriticalRisk()
    {
        return m_bHasCriticalRisk;
    
	}
#endif

protected:
//	CSimpleArray<int> m_arrIgnoreItems;
    HWND	m_hWndNotify;
    UINT	m_uMsgNotifyItem;
    UINT	m_uMsgNotifyProgress;
    HANDLE  m_hEventStop;
    BOOL	m_bExpressExam;
    BOOL	m_bExamCanceled;
//     BOOL	m_bHasRisk;
//     BOOL	m_bHasCriticalRisk;
	int		m_nUnSafeMonitorCount;

    CRBMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapExamResult;
    CRITICAL_SECTION					 m_lockExamResult;
	CRITICAL_SECTION					 m_lockExamResult2;

    struct _ECHO_THREAD_PARAM 
    {
        HWND hWndNotify;
        UINT uMsgNotify;
        CBkSafeExamCaller *pThis;
    };

    virtual HRESULT WINAPI OnItemExamFinish(
        int     nItem, 
        int     nLevel, 
        LPCWSTR lpszItemNoticeText, 
        LPCWSTR lpszOperationText, 
        LPCWSTR lpszItemNavigateString, 
        LPCWSTR lpszItemDetailUrl, 
        LPCWSTR lpszDetail);

    virtual HRESULT WINAPI OnFinish()
    {
        return S_OK;
    }

    virtual HRESULT WINAPI OnProgress(DWORD dwProgress, int nExamItem = -1)
    {
        if (::IsWindow(m_hWndNotify))
            ::SendMessage(m_hWndNotify, m_uMsgNotifyProgress, (WPARAM)dwProgress, nExamItem);

        return S_OK;
    }

    static DWORD WINAPI _SafeExaminationThreadProc(LPVOID pvParam);

    class _EchoCallback
        : public IBkSafeExamineCallback
    {
    public:
        _EchoCallback(_ECHO_THREAD_PARAM *pParam)
            : m_pParam(pParam)
        {
        }

        _ECHO_THREAD_PARAM *m_pParam;

        virtual HRESULT WINAPI OnItemExamFinish(
            int     nItem, 
            int     nLevel, 
            LPCWSTR lpszItemNoticeText, 
            LPCWSTR lpszOperationText, 
            LPCWSTR lpszItemNavigateString, 
            LPCWSTR lpszItemDetailUrl, 
            LPCWSTR lpszDetail)
        {
            BKSAFE_EXAM_RESULT_ITEM item = { nLevel, lpszItemNoticeText, lpszOperationText, lpszItemNavigateString};
			item.SafeExamItemEx  = *((BKSafeExamItemEx::BKSafeExamEx*)(lpszDetail));

			if(BkSafeExamItem::EXAM_CONNECT_SRV != nItem)
				return S_OK;

            if (::IsWindow(m_pParam->hWndNotify))
                ::SendMessage(m_pParam->hWndNotify, m_pParam->uMsgNotify, (WPARAM)nItem, (LPARAM)&item);

            return S_OK;
        }

        virtual HRESULT WINAPI OnFinish()
        {
            return S_OK;
        }

		virtual HRESULT WINAPI OnProgress(DWORD dwProgress, int nExamItem = -1)
		{
			return S_OK;
		}

//         virtual HRESULT WINAPI OnProgress(DWORD dwProgress, int nExamItem = -1)
//         {
//             return S_OK;
//         }
    };

    static DWORD WINAPI _EchoThreadProc(LPVOID pvParam)
    {
        _ECHO_THREAD_PARAM *pParam = (_ECHO_THREAD_PARAM *)pvParam;
        IBkSafeExaminer *piExaminer = NULL;

        if (NULL == pParam)
            return 0;

        _EchoCallback _callback(pParam);

        HMODULE hModExam = ::LoadLibrary(L"ksafeexam.dll");
        if (NULL == hModExam)
            goto Exit0;

        PFN_BkSafeCreateObject BkSafeCreateObject = (PFN_BkSafeCreateObject)::GetProcAddress(hModExam, FN_BkSafeCreateObject);
        if (NULL == BkSafeCreateObject)
            goto Exit0;

        HRESULT hRet = BkSafeCreateObject(BK_SAFEEXAM_DLL_VERSION, __uuidof(IBkSafeExaminer), (LPVOID *)&piExaminer);
        if (NULL == piExaminer)
            goto Exit0;

        hRet = piExaminer->Initialize(NULL);
        if (FAILED(hRet))
            goto Exit0;

        piExaminer->Echo(&_callback, NULL);

        if (::IsWindow(pParam->hWndNotify))
            ::SendMessage(pParam->hWndNotify, pParam->uMsgNotify, BkSafeExamItem::Invalid, NULL);

        piExaminer->Uninitialize();

Exit0:

        if (piExaminer)
        {
            piExaminer->Release();
            piExaminer = NULL;
        }

        if (hModExam)
        {
            ::FreeLibrary(hModExam);
            hModExam = NULL;
        }

        delete pParam;

        return 0;
    }
};
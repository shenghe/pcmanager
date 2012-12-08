#pragma once

#include "ksaferunstrdefine.h"

#define VERSION_KSAFE_RUNOPT_INTERFACE		3
#define STR_KSAFE_RUN_OPTIMIZE_DAT			_T("\\runopt.dat")


#define KSRUN_TYPE_STARTUP	1
#define KSRUN_TYPE_SERVICE	2
#define KSRUN_TYPE_TASK		3
#define KSRUN_TYPE_ALL		-1

#define REG_RUN_TYPE_WINDOW_HKLM		1
#define REG_RUN_TYPE_WINDOW_HKCU		2
#define REG_RUN_TYPE_KSAFE_HKLM			3
#define REG_RUN_TYPE_KSAFE_HKCU			4
#define REG_RUN_TYPE_3XX_HKLM			5
#define REG_RUN_TYPE_3XX_HKCU			6

#define DIR_RUN_TYPE_BEGIN				100
#define DIR_RUN_TYPE_WINDOW_COMM		101
#define DIR_RUN_TYPE_WINDOW_USER		102
#define DIR_RUN_TYPE_KSAFE_COMM			103
#define DIR_RUN_TYPE_KSAFE_USER			104
#define DIR_RUN_TYPE_3XX_COMM			105
#define DIR_RUN_TYPE_3XX_USER			106

inline BOOL IsTypeRunEnable(DWORD nType)
{
	return (nType==REG_RUN_TYPE_WINDOW_HKLM||
		nType==REG_RUN_TYPE_WINDOW_HKCU||
		nType==DIR_RUN_TYPE_WINDOW_COMM||
		nType==DIR_RUN_TYPE_WINDOW_USER);
}

inline BOOL IsTypeRunReg(DWORD nType)
{
	return (nType<DIR_RUN_TYPE_BEGIN);
}

inline BOOL IsTypeRunDir(DWORD nType)
{
	return (nType>DIR_RUN_TYPE_BEGIN);
}

inline BOOL IsHKLMRunType(DWORD nType)
{
	return (nType==REG_RUN_TYPE_WINDOW_HKLM || nType==REG_RUN_TYPE_KSAFE_HKLM);
}

inline BOOL IsSameRunType(DWORD nType, DWORD nTypeX)
{
	if (nTypeX==nType)
		return TRUE;

	if (nType==REG_RUN_TYPE_WINDOW_HKLM && nTypeX==REG_RUN_TYPE_KSAFE_HKLM )
		return TRUE;

	if (nType==REG_RUN_TYPE_WINDOW_HKCU && nTypeX==REG_RUN_TYPE_KSAFE_HKCU )
		return TRUE;

	if (nTypeX==REG_RUN_TYPE_WINDOW_HKLM && nType==REG_RUN_TYPE_KSAFE_HKLM )
		return TRUE;

	if (nTypeX==REG_RUN_TYPE_WINDOW_HKCU && nType==REG_RUN_TYPE_KSAFE_HKCU )
		return TRUE;

	//////////////////////////////////////////////////////////////////////////
	if (nType==DIR_RUN_TYPE_WINDOW_COMM && nTypeX==DIR_RUN_TYPE_KSAFE_COMM )
		return TRUE;

	if (nType==DIR_RUN_TYPE_WINDOW_USER && nTypeX==DIR_RUN_TYPE_KSAFE_USER )
		return TRUE;

	if (nTypeX==DIR_RUN_TYPE_WINDOW_COMM && nType==DIR_RUN_TYPE_KSAFE_COMM )
		return TRUE;

	if (nTypeX==DIR_RUN_TYPE_WINDOW_USER && nType==DIR_RUN_TYPE_KSAFE_USER )
		return TRUE;

	return FALSE;
}


inline CString GetFileName(LPCTSTR lpFullPath)
{
	LPCTSTR lpstr = _tcsrchr(lpFullPath,_T('\\'));

	if (lpstr==NULL)
		return CString(lpFullPath);
	else
		return CString(lpstr+1);
}

//NoAutoDisable
//StopIt

class CKsafeRunInfo
{
public:
	CKsafeRunInfo()
	{	
		nID		= 0;
		bEnable = FALSE;
		nType	= 0;
		nAdvice	= 4;
		bSystem = FALSE;
		bNoDealyShow = FALSE;
		nRegType= 0;
		bUnknown= TRUE;
	}
	~CKsafeRunInfo(){}

	BOOL IsAdviseStop()
	{
		return (nAdvice==0);
	}

	LPCTSTR GetAdviceStr()
	{
		LPCTSTR	lpstr = _T("暂无建议");
		switch (nAdvice)
		{
		case 0:
			lpstr = _T("可以禁止启动");
			break;
		case 1:
			lpstr = _T("建议开机启动");
			break;
		case 2:
			lpstr = _T("必须开机启动");
			break;
		case 3:
			lpstr = _T("建议开机启动");
			break;
		case 4:
			lpstr = _T("建议维持现状");
			break;
		default:
			break;
		}
		return lpstr;
	}

public:
	DWORD		nID;
	DWORD		nType;			
	DWORD		nAdvice;	
	BOOL		bEnable;
	BOOL		bNoDealyShow;	
	BOOL		bUnknown;
	BOOL		bSystem;
	DWORD		nRegType;
	DWORD		nJobType;
	CString		strName;			
	CString		strDesc;		
	CString		strPath;		//启动项的全路径
	CString		strExePath;		//启动项lnk指向文件的全路径
	CString		strValue;		//run项的注册表值
	CString		strParam;		
	CString		strDisplay;		//显示的名字
	CString		strWarning;		//禁止的警告
};

class IEnumRunCallBack
{
public:
	virtual BOOL FindRunItem(CKsafeRunInfo* pInfo) = 0;
	virtual void BeginScan(DWORD nType) = 0;
	virtual void EndScan() = 0;
	virtual void RestoreEnd() = 0;
	virtual void PostRestoreRunItem(DWORD nType,BOOL bEnable,LPCTSTR lpName) = 0;
};

class IEnumRunCallBackNull : public IEnumRunCallBack
{
	virtual BOOL FindRunItem(CKsafeRunInfo* pInfo)=0;
	virtual void BeginScan(DWORD nType){return;}
	virtual void EndScan(){return;}
	virtual void RestoreEnd(){return;}
	virtual void PostRestoreRunItem(DWORD nType,BOOL bEnable,LPCTSTR lpName){return;}
};

class IRunDatQueryEng
{
public:
	virtual BOOL GetInfoByPathValue(DWORD nType, LPCTSTR lpPath, LPCTSTR lpFullPath, KSRunInfo** ppInfo) = 0;
	virtual VOID DeleteInfo(KSRunInfo* pInfo) = 0;
};

class IRunOptimizeEng
{
public:
	virtual VOID EnumRunInfo(DWORD dwFlags,IEnumRunCallBack* pCallback )=0;
	virtual BOOL ControlRun(BOOL bEnable, CKsafeRunInfo* pInfo) = 0;
	virtual VOID RestoreRun(DWORD nType,IEnumRunCallBack* pCallback) = 0;
	virtual BOOL DeleteRun(CKsafeRunInfo* pInfo) = 0;
};

typedef IRunOptimizeEng* (WINAPI *PFNGetRunOptEng)(DWORD);
typedef VOID (WINAPI *PFNFreeRunOptEng)(IRunOptimizeEng* );

typedef IRunDatQueryEng* (WINAPI *PFNGetQueryEng)(DWORD);
typedef VOID (WINAPI *PFNFreeQueryEng)(IRunDatQueryEng* );

#define STR_RUNOPT_DLL_NAME _T("krunopt.dll")
#define STR_RUNOPT_GET_ENG	("a")
#define STR_RUNOPT_DEL_ENG	("b")
#define STR_RUNOPT_GET_QENG	("c")
#define STR_RUNOPT_DEL_QENG	("d")




class CRunOptLoader
{
public:
	CRunOptLoader()
	{
		m_hModDLL	= NULL;
		pfnGet		= NULL;
		pfnFree		= NULL;
		pfnGetQEng	= NULL;
		pfnFreeQEng = NULL;

		m_hModDLL	= ::LoadLibrary(STR_RUNOPT_DLL_NAME);
		if ( m_hModDLL != NULL )
		{
			pfnGet  = (PFNGetRunOptEng)::GetProcAddress( m_hModDLL, STR_RUNOPT_GET_ENG);
			pfnFree = (PFNFreeRunOptEng)::GetProcAddress( m_hModDLL, STR_RUNOPT_DEL_ENG);
			pfnGetQEng  = (PFNGetQueryEng)::GetProcAddress( m_hModDLL, STR_RUNOPT_GET_QENG);
			pfnFreeQEng = (PFNFreeQueryEng)::GetProcAddress( m_hModDLL, STR_RUNOPT_DEL_QENG);
		}
	}
	virtual ~CRunOptLoader()
	{
		if ( m_hModDLL != NULL )
		{
			::FreeLibrary(m_hModDLL);
			m_hModDLL = NULL;
		}
	}

	static CRunOptLoader* GetGlobalPtr()
	{
		static CRunOptLoader loader;
		return &loader;
	}

	IRunOptimizeEng* GetOptEng()
	{
		if ( pfnGet != NULL )
			return pfnGet(VERSION_KSAFE_RUNOPT_INTERFACE);

		return NULL;	
	}

	VOID FreeOptEng(IRunOptimizeEng* pEng)
	{
		if ( pfnFree != NULL )
			pfnFree(pEng);
	}


	IRunDatQueryEng* GetQueryEng()
	{
		if ( pfnGetQEng != NULL )
			return pfnGetQEng(VERSION_KSAFE_RUNOPT_INTERFACE);

		return NULL;	
	}

	VOID FreeQueryEng(IRunDatQueryEng* pEng)
	{
		if ( pfnFreeQEng != NULL )
			pfnFreeQEng(pEng);
	}


protected:
	HMODULE				m_hModDLL;
	PFNGetRunOptEng		pfnGet;
	PFNFreeRunOptEng	pfnFree;

	PFNGetQueryEng		pfnGetQEng;
	PFNFreeQueryEng		pfnFreeQEng;
};


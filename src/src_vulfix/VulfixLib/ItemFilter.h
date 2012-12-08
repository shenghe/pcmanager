#pragma once
#include "vulfix/Defines.h"
#include "WUAUpdateInfo.h"
#include "EnvUtils.h"

class DBUpdatableHotfix;

class DBFilterBase
{
public:
	DBFilterBase(IVulIgnore *piIgnore=NULL)
		: m_piIgnore(piIgnore)
	{
	}
	virtual ~DBFilterBase() {}
	
	virtual void Init(DWORD dwFlags);
	// check conditon if meets, and return dwId 
	virtual BOOL IsInstallable(TOptionCondition &condition, int nKBID, int &nType, DWORD &dwLang) = 0;
	virtual BOOL IsInstalled(TOptionCondition &condition, int nKBID)
	{
		return IsKBInstalled(nKBID);
	}
	virtual M_KBInfo &GetInstalledInfo() = 0;

	virtual BOOL IsKBInstalled(int nKBID) = 0;

	virtual void Cancel()
	{
		
	}
	
	virtual BOOL WaitComplete()
	{
		return TRUE;
	}
	

	virtual void SetIIgnore(IVulIgnore *piIgnore)
	{
		m_piIgnore = piIgnore;
	}
	virtual BOOL IsIgnored(INT nKBID)
	{
		return m_piIgnore && m_piIgnore->IsIgnored( nKBID );
	}
	
protected:
	IVulIgnore *m_piIgnore;
};

class DBFilterXp : public DBFilterBase
{
public:
	virtual void Init(DWORD dwFlags);
	virtual BOOL IsInstallable(TOptionCondition & condition, int nKBID, int &nType, DWORD &dwLang);
	virtual BOOL IsInstalled(TOptionCondition &condition, int nKBID);
	virtual BOOL IsKBInstalled(int nKBID);
	virtual M_KBInfo &GetInstalledInfo();
protected:
	CWUAUpdateInfoXP m_updateInfo;
};

class DBFilterVista : public DBFilterBase
{
public:
	DBFilterVista()
	{
		m_bUseInterface = TRUE;
		m_bCanceled = FALSE;
	}
	virtual void Init(DWORD dwFlags);
	virtual BOOL IsInstallable(TOptionCondition & condition, int nKBID, int &nType, DWORD &dwLang);
	virtual BOOL IsInstalled(TOptionCondition &condition, int nKBID);
	virtual BOOL IsKBInstalled(int nKBID);
	virtual M_KBInfo &GetInstalledInfo();

	virtual BOOL WaitComplete();
	virtual void Cancel();
protected:
	CThreadGuard m_guard;
	BOOL m_bUseInterface;
	BOOL m_bExpressScan;	// øÏÀŸ…®√Ë
	BOOL m_bCanceled;
};

#pragma once
#include <map>
#import "wuapi.dll" no_auto_exclude
#include "vulfix/Defines.h"

typedef std::map<int, TItemFixed> M_KBInfo;

INT ReadInstalledOffice(M_KBInfo &arr);

class IWUAUpdateInfo {
public:
	IWUAUpdateInfo() : m_dwLastResultTime(0), m_dwLastModifiedTime(0)
	{ }
	virtual void SetWUAModified()
	{
		m_dwLastModifiedTime = GetTickCount();
	}
	
	virtual void Init(DWORD dwFlags) = 0;
	virtual BOOL IsUpdateAvailable(int nKBID) = 0;
	virtual BOOL IsUpdateInstalled(int nKBID) = 0;
	virtual M_KBInfo & GetInstalledInfo() = 0;

protected:
	DWORD m_dwLastResultTime, m_dwLastModifiedTime;
};

class CWUAUpdateInfoXP : public IWUAUpdateInfo {
public:
	virtual void Init(DWORD dwFlags);
	virtual BOOL IsUpdateAvailable(int nKBID);
	virtual BOOL IsUpdateInstalled(int nKBID);
	virtual M_KBInfo & GetInstalledInfo();

protected:
	void _GetKBIDsByReg(LPCTSTR szKeyPath, M_KBInfo &arrKBID);

protected:
	M_KBInfo m_arrKBID;
};

class CWUAUpdateInfoVistaLater: public IWUAUpdateInfo {
public:
	static DWORD WINAPI ThreadFunc_Scan( LPVOID lpParam );

	CWUAUpdateInfoVistaLater();
	~CWUAUpdateInfoVistaLater();
	virtual void Init(DWORD dwFlags);
	virtual BOOL IsUpdateAvailable(int nKBID);
	virtual BOOL IsUpdateInstalled(int nKBID);
	virtual M_KBInfo & GetInstalledInfo();
	
	BOOL IsIUpdateResultAvailable();	
	void SetCompleted(BOOL bCompleted);
	BOOL IsCompleted();
	
	FILETIME m_LastestUpdateTime;
	
protected:
	void _Scan();
	BOOL _GetKBIDsByIUpdate(BOOL getInstalled, M_KBInfo &arrKBID);
	BOOL _ReadUpdateInfo( WUApiLib::IUpdateSearcher *spUpdateSearch, BOOL installed, M_KBInfo &arrKBID );
	
protected:
	BOOL m_bDataAvailable;
	M_KBInfo m_arrInstalledKB, m_arrInstallableKB;

	CObjLock m_objLock;
	HANDLE m_hThread;
	BOOL m_bCompleted;
};

class CWUAUpdateInfoVistaLaterReg: public IWUAUpdateInfo {
public:
	CWUAUpdateInfoVistaLaterReg();
	virtual void Init(DWORD dwFlags);
	virtual BOOL IsUpdateAvailable(int nKBID);
	virtual BOOL IsUpdateInstalled(int nKBID);
	virtual M_KBInfo & GetInstalledInfo();

	FILETIME m_LastestUpdateTime;
	
protected:
	M_KBInfo m_arrInstalledKB;
};
#pragma once

class KSProcessInfo;
class KProcessLibLoader;
class IProcessLibQuery;

class CProcessLibQueryImp:
		public IProcessLibQuery
{
public:
	CProcessLibQueryImp(void);
	~CProcessLibQueryImp(void);

	virtual BOOL GetInfoByPath( LPCTSTR szFullPath, KSProcessInfo** pInfo );

	virtual void DeleteInfo(KSProcessInfo* pInfo);

	virtual BOOL Init( LPCTSTR szLibPath );
protected:

	
protected:
	KProcessLibLoader*	m_pLibLoader;
	BOOL		m_bInit;
};

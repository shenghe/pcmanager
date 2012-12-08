#pragma once
#include "avmgr\avmgrinterface.h"
#include "AVLibParse.h"
#include "com_s\com\comobject.h"
#include "vulfix\BeikeUtils.h"
using namespace AVProductMgr;

class CAVProductMgr :
	public IAVProductMgr
{
public:
	CAVProductMgr(void);
	~CAVProductMgr(void);

	BOOL LoadSoftGuid();

	SCOM_BEGIN_COM_MAP(CAVProductMgr)
		SCOM_INTERFACE_ENTRY(IAVProductMgr)
	SCOM_END_COM_MAP()

protected:
	CAVLibParse m_libParser;
	CAtlList<INSTALLED_SOFT_ITEM>	m_GuidList;
	CAtlList<COMPANY_INFO>			m_CompanyList;
	POSITION						m_iCompantPos;
	POSITION						m_iProductPos;
	CString							m_strAVNames;

	void ProcessGUID(const CString &strGuids, CString &strExpression);

public:
	virtual UINT32 STDMETHODCALLTYPE LoadAVLib(LPCWSTR lpszPath);
	virtual LPCWSTR STDMETHODCALLTYPE Check();
	virtual UINT32 STDMETHODCALLTYPE IsCanUninstall();
	virtual UINT32 STDMETHODCALLTYPE UnInstall();
	virtual LPCWSTR STDMETHODCALLTYPE GetInstallAVs();
};


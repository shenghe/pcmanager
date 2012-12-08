#pragma once
#include "Utils.h"

class CImplVulIgnore : public IVulIgnore
{
public:
	CImplVulIgnore();
	virtual HRESULT LoadIgnoreDB();
	virtual HRESULT SaveIgnoreDB();	
	virtual HRESULT Ignore(int nKBID, BOOL bIgnore);
	virtual HRESULT AutoIgnore(int nKBID, BOOL bIgnore) ;
	virtual int IsIgnored(int nKBID);

protected:
	CSimpleArray<TPairIdString> m_arrIgnoredId, m_arrAutoIgnoredId;
};
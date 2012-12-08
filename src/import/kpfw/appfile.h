////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : appfile.h
//      Version   : 1.0
//      Comment   : 网镖应用程序规则文件读写
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "kis/kpfw/filecomm.h"
#include "framework/kis_file.h"
#include "kis/kpfw/errorcode.h"

// appr
#define APP_FILE_MAGIC						( ((int('R')) << 24) | ((int('P')) << 16) | ((int('P')) << 8) | ((int('A'))) )

// 应用程序规则文件头
struct AppFileHeader
{
	DWORD				cbSize;				// 文件头大小
	DWORD				nMagic;				// 标志
	INT					nVer;				// 文件版本
	DWORD				nAppRuleOffset;		// 程序规则开始位置
};
class KAppRuleVec;
class KAppRule;

class KAppFile
{
private:
	KAppRuleVec*		m_pAppMgr;

public:
	KAppFile(KAppRuleVec* pMgr):m_pAppMgr(pMgr)		{}

	HRESULT	Load( LPCWSTR strFile ) ;
	HRESULT Save( LPCWSTR strFile );

private:
	HRESULT	ReadAppRules(KRecord* pRecord);
	HRESULT	ReadRuleList(KRecord* pRecord);
	HRESULT	ReadRule(KRecord* pRecord);
	HRESULT ReadDenyPortList(KRecord* pRecord, KAppRule* pRule);

	HRESULT	SaveRuleList(KRecordWriter* pWriter);
	HRESULT SaveRule(KRecordWriter* pWriter, KAppRule* pRule);
};
////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : areafile.h
//      Version   : 1.0
//      Comment   : 网镖区域规则文件
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "kis/kpfw/filecomm.h"
#include "framework/kis_file.h"
#include "kis/kpfw/errorcode.h"

#define AREA_FILE_MAGIC						( ((int('A')) << 24) | ((int('E')) << 16) | ((int('R')) << 8) | ((int('A'))) )

// ip规则文件头
struct AreaFileHeader
{
	DWORD				cbSize;				// 文件头大小
	DWORD				nMagic;				// 标志
	INT					nVer;				// 文件版本
	DWORD				DataOffset;			// ip开始位置
};

class KAreaVec;
class KArea;

class KAreaFile
{
private:
	KAreaVec*				m_pAreaMgr;

public:
	KAreaFile(KAreaVec* pMgr):m_pAreaMgr(pMgr)		{}

	HRESULT Load(LPCWSTR strFile);
	HRESULT Save(LPCWSTR strFile);

private:
	HRESULT ReadAreaRoot(KRecord* pRecord);
	HRESULT	ReadAreaList(KRecord* pRecord);
	HRESULT	ReadArea(KRecord* pRecord);

	HRESULT SaveAreaList(KRecordWriter* pWriter);
	HRESULT SaveArea(KRecordWriter* pWriter, KArea* pRule);
};
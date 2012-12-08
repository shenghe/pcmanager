////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : advflt.h
//      Version   : 1.0
//      Comment   : 网镖高级包过滤配置文件
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "kis/kpfw/filecomm.h"
#include "framework/kis_file.h"
#include "kis/kpfw/errorcode.h"
#include "framework/kis_crypt.h"

#define ADVIP_FILE_MAGIC						( ((int('L')) << 24) | ((int('R')) << 16) | ((int('D')) << 8) | ((int('A'))) )

// ip规则文件头
struct ADVFLTFileHeader
{
	DWORD				cbSize;				// 文件头大小
	DWORD				nMagic;				// 标志
	INT					nVer;				// 文件版本
	DWORD				nAdvFltCnt;			// 高级过滤配置个数
	BYTE				CheckSum[16];		// 高级包过滤数据的md5
	DWORD				nIpRuleOffset;		// ip开始位置
};

interface IKAdvTemplateList
{
	virtual INT		GetTemplateCnt()		PURE;

	virtual	INT		GetBufferSize()			PURE;

	virtual	void	WriteBuffer(BYTE* pBuff)	PURE;
};

class KAdvFltFile
{
private:
	kis::KFileRead*		m_pFile;
public:
	KAdvFltFile(): m_pFile(NULL)
	{

	}

	~KAdvFltFile()
	{
		if (m_pFile)
		{
			delete m_pFile;
			m_pFile = NULL;
		}
	}

	inline HRESULT Load(LPCWSTR strFile);

	inline HRESULT Save(LPCWSTR strFile, KPackTemplate* pTemplate, INT nBufSize, INT nCnt);

	inline HRESULT Save(LPCWSTR strFile, IKAdvTemplateList* pTempList);

	inline INT	   GetTemplateCnt();

	inline KPackTemplate* GetFirstTemplate();
};


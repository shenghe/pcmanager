// ----------------------------------------------------------------------------- 
//	FileName	: oeminforw.cpp
//	Author		: Zqy
//	Create On	: 2007-11-10 18:17:45
//	Description	: 
//
// ----------------------------------------------------------------------------- 
#include "stdafx.h"
#include "oeminforw.h"
#include "aplib.h"
#include "kis_diag.h"
#include "KOemKsfe.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

#include <assert.h>
#ifndef ASSERT
#define ASSERT assert
#endif
// -----------------------------------------------------------------------------
static DWORD dwTotalSize = 0;
DWORD _Pack_Proc(unsigned char* pSourceData, DWORD dwSrcSize, unsigned char* &pDestData)
{
	KS_CHECK_BOOL(pSourceData != NULL);

	BOOL bRetCode = FALSE;
	DWORD dwPackedSize = -1;

	void* workmem = new char[aP_workmem_size(dwSrcSize)];
	KS_CHECK_BOOL(workmem != NULL);

	pDestData =(unsigned char*) new char[dwSrcSize * 2];
	KS_CHECK_BOOL(pDestData != NULL);
	memset(pDestData, 0, dwSrcSize * 2);


	dwPackedSize = aP_pack(pSourceData, pDestData, dwSrcSize, (unsigned char *)workmem, NULL);
	KS_CHECK_BOOL(dwPackedSize != -1);


KS_EXIT:
	if ( workmem )
	{
		delete []workmem;
		workmem = NULL;
	}
	return dwPackedSize;
}

DWORD _DePack_Proc(unsigned char* pSrcData, unsigned char* &pDstData)
{
	DWORD  nDepackLength = -1;

	nDepackLength = aP_depack_asm_fast((unsigned char*)pSrcData, (unsigned char*)pDstData);
	KS_CHECK_BOOL(nDepackLength != -1);

KS_EXIT:
	return nDepackLength;
}

// -----------------------------------------------------------------------------

inline int LocGroup(int id)
{
	return ((id & FDGROUPID_MASK) >> 16);
}

FDRecHeader* KFeatureReader::Find(int id)
{
	FDGroup* grp = FindGroup(id);
	if (grp)
	{
		int idx = id - grp->groupid - 1;
		if (idx >= 0 && idx < grp->fdcnt)
			return (FDRecHeader*)(m_buf + grp->offset[idx]);
	}
	return NULL;
}

FDGroup* KFeatureReader::FindGroup(int id)
{
	int idx = LocGroup(id);
	if (m_FDStart && idx >= 0 && idx < m_FDStart->groups)
		return m_groups[idx];
	
//	ASSERT_ONCE(0);
	return NULL;
}

BOOL KFeatureReader::ValidRange(void* p, int size)
{
	return (p >= m_buf) && (((BYTE*)p + size) <= m_end);
}

KFeatureReader::KFeatureReader()
{
	m_buf     = NULL;
	m_end     = NULL;
	m_FDStart = NULL;

	memset(m_groups, 0, sizeof(m_groups));
}
KFeatureReader::~KFeatureReader()
{
	Clear();
}

HRESULT KFeatureReader::Clear()
{
	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}

	m_end = NULL;
	m_FDStart = NULL;
	memset(m_groups, 0, sizeof(m_groups));

	return S_OK;
}

int KFeatureReader::QueryInt(int id)
{
	FDRecHeader* p = Find(id);
	FDIntData* d = (FDIntData*)(p + 1);

	if (ValidRange(p, sizeof(FDRecHeader) + sizeof(FDIntData)) &&
		p->type == rtIntData &&
		d->id == id)
	{
		return d->val;
	}
	
	return -1;
}

LPCWSTR KFeatureReader::QueryStr(int id)
{
	FDRecHeader* p = Find(id);
	FDStrData* d = (FDStrData*)(p + 1);

	if (ValidRange(p, sizeof(FDRecHeader)) &&
		ValidRange(d, p->len) &&
		p->type == rtStrData &&
		d->id == id)
	{
		return d->val;
	}
	
//	ASSERT(0);
	return 0;
}

HRESULT KFeatureReader::Load(LPCWSTR lpcwFile)
{
	ASSERT(!m_buf);

	DWORD dwSize = 0,
		  dwRead = 0;
	DWORD dwDatas = 0;

	FDStart* start = NULL;
	FDRecHeader* header = NULL;
	BYTE* buffer = NULL;
	DWORD dwOriSize = 0;

	HRESULT hr = E_FAIL;
	HANDLE hFile = CreateFileW(lpcwFile,
							   GENERIC_READ,
							   FILE_SHARE_READ,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);
	
	if (INVALID_HANDLE_VALUE == hFile) // 可以允许找不到，比如oem.dat
	{ 
		goto KS_EXIT; 
	} 
	
	KS_CHECK_BOOL(INVALID_FILE_SIZE != (dwSize = GetFileSize(hFile, NULL)));
	KS_CHECK_BOOL(NULL != (buffer = (BYTE*)malloc(dwSize)));
	KS_CHECK_BOOL(ReadFile(hFile, buffer, dwSize, &dwRead, NULL));
	KS_CHECK_BOOL(dwRead == dwSize);

	FDPackInfoHeader *packinfo = (FDPackInfoHeader*)buffer;
	KS_CHECK_BOOL(packinfo->wType == rtStart);
	KS_CHECK_BOOL((dwOriSize = packinfo->wTotalSize) != 0);

	KS_CHECK_BOOL(NULL != (m_buf = (BYTE*)malloc(dwOriSize)));
	memset(m_buf, 0, dwOriSize);

	{//解密
		DWORD dwDepackSize = 0;
		dwDepackSize = _DePack_Proc(buffer + sizeof(FDPackInfoHeader), m_buf);
		KS_CHECK_BOOL(dwDepackSize != -1 && dwDepackSize == dwOriSize);

		
	}
	m_end = m_buf + dwOriSize - sizeof(FDRecHeader) - sizeof(FDEnd);
	dwTotalSize = 0;

	//生成组
	{
		header    = (FDRecHeader*)m_buf;
		start     = (FDStart*)(header + 1);
		m_FDStart = start;
		
		KS_CHECK_BOOL(rtStart == header->type);
		KS_CHECK_BOOL(start->groups <= MAX_GROUP && header->len == sizeof(FDStart) + sizeof(int) * start->groups)
		KS_CHECK_BOOL(start->filever == 1);

		for (int i = 0; i < start->groups; ++i)
		{
			header = (FDRecHeader*)(m_buf + start->offset[i]);
			FDGroup* grp = (FDGroup*)(header + 1);

			KS_CHECK_BOOL(grp <= (FDGroup*)(m_end - sizeof(FDGroup)));
			KS_CHECK_BOOL(header->type == rtGroup && header->len == sizeof(FDGroup) + sizeof(int) * grp->fdcnt);

			{
				int idx = LocGroup(grp->groupid);
				KS_CHECK_BOOL(idx >= 0 && idx < m_FDStart->groups);
				m_groups[idx] = grp;
			}

			dwDatas += grp->fdcnt;
		}
	}

	//检查End记录
	{
		//最后一个记录是 rtEnd;	
		header = (FDRecHeader*)m_end;
		KS_CHECK_BOOL(header->type == rtEnd && header->len == sizeof(FDEnd));
	}

	if (dwDatas == start->datas)
		hr = S_OK;
KS_EXIT:
	if (INVALID_HANDLE_VALUE != hFile) // 可以允许找不到，比如oem.dat
		CloseHandle(hFile);
	
	if ( buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	//ASSERT(SUCCEEDED(hr));
	return hr;
}

FDStart* KFeatureReader::GetFDStart()
{
	return m_FDStart;
}

BOOL KFeatureWriter::CheckID(int id)
{
	BOOL bFailed = FALSE;
	int idx = (id & (~FDGROUPID_MASK)) - 1;

	if (idx != m_grp->size())
	{
		bFailed = TRUE;
		REPORT("id不连续");
	}
	else
	{
		//必须是按顺序而且连续的号
		if (m_grp->size())
		{
			FDRecHeader* h = m_grp->at(m_grp->size() - 1);

			if (bFailed =  (*(int*)(h + 1) + 1 != id))
				REPORT("id不连续");
		}
		else//第一个
		{
			if (bFailed = id != ((id & 0x0F000000) + 1))
				REPORT("非法的第一个id");
		}
	}

	if (bFailed)
		m_failed = bFailed;

	return !bFailed;
}

KFeatureWriter::KFeatureWriter() : m_grp(NULL), m_start((FDStart*)m_bufStart), m_failed(FALSE)
{
	ZeroMemory(m_bufStart, sizeof(m_bufStart));
	ZeroMemory(m_groups, sizeof(m_groups));
}

KFeatureWriter::~KFeatureWriter()
{
	Over();
}

void KFeatureWriter::Over()
{
	m_failed = false;
	if ( m_grp != NULL)
	{
		GROUP::iterator iter;
		for ( iter = m_grp->begin(); iter != m_grp->end(); iter++)
		{
			free(*iter);
			*iter = NULL;

		}
		m_grp->clear();
		m_grp = NULL;
	}
	
	
	for (int i = 0; i < m_start->groups; ++i)
	{
		GROUP::iterator it = m_groups[i].begin();
		for (; it != m_groups[i].end(); ++it)
			free(*it);
		m_groups[i].clear();
		m_groups[i].datasize = 0;
	}

	ZeroMemory(m_bufStart, sizeof(m_bufStart));
	ZeroMemory(m_groups, sizeof(m_groups));
}

KFeatureWriter& KFeatureWriter::Start()
{
	Over();
	m_start->filever = 1;
	m_start->datas = 0;
	return *this;
}

KFeatureWriter& KFeatureWriter::StartGroup(int grpid)
{
	int idx = LocGroup(grpid);
	if (idx >= 0 && idx < MAX_GROUP)
	{
		m_grp = m_groups + idx;

		if (!m_grp->used)
		{
			++m_start->groups;
			m_grp->used = TRUE;
		}
	}
	else
	{
		m_grp = NULL;
		ASSERT(0);
	}

	return *this;
}

KFeatureWriter& KFeatureWriter::AddFeature(int id, int Val)
{
	CheckID(id);

	FDRecHeader* h = (FDRecHeader*)malloc(sizeof(FDRecHeader) + sizeof(FDIntData));
	h->len = sizeof(FDIntData);
	h->type = rtIntData;
	
	FDIntData* d = (FDIntData*)(h + 1);
	d->id = id;
	d->val = Val;

	m_grp->push_back(h);
	m_grp->datasize += sizeof(FDRecHeader) + h->len;

	return *this;
}

KFeatureWriter& KFeatureWriter::AddFeature(int id, LPCWSTR lpcwVal)
{
	ASSERT(lpcwVal);

	CheckID(id);

	int len = (int)wcslen(lpcwVal) + 1;
	
	FDRecHeader* h = (FDRecHeader*)malloc(sizeof(FDRecHeader) + sizeof(FDStrData) + sizeof(WCHAR) * len);
	h->len = (WORD)(sizeof(FDStrData) + sizeof(WCHAR) * len);
	h->type = rtStrData;
	
	FDStrData* d = (FDStrData*)(h + 1);
	d->id = id;
	d->len = len;
	d->val[len - 1] = 0;
	
	if (len > 1)
		wcscpy_s(d->val, len, lpcwVal);

	m_grp->push_back(h);
	m_grp->datasize += sizeof(FDRecHeader) + h->len;

	return *this;
}

KFeatureWriter& KFeatureWriter::EndGroup(int grpid)
{
	m_start->datas += (int)m_grp->size();

	ASSERT(m_grp->size());

	FDRecHeader* h = m_grp->at(m_grp->size() - 1);
	int nLastID  = *(int*)(h + 1) + 1;
	BOOL failed = nLastID != KIS::KOemKsfe::oemc_end;


	if (failed)
	{
		REPORT("这个分组中的ID不完整，意思是说后面还有FeatureData，而你没有把它加上");
		m_failed = failed;
	}

	return *this;
}

HRESULT KFeatureWriter::End()
{
	return m_failed ? E_FAIL : S_OK;
}

HRESULT KFeatureWriter::Write(LPCWSTR lpcwPath)
{
	if (m_failed)
		return E_FAIL;
	
	HANDLE hFile = CreateFileW(lpcwPath,
							   GENERIC_WRITE,
							   0,
							   NULL,
							   CREATE_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL,
							   NULL);
	
	FDRecHeader h;
	//DWORD dwWrite;
	
	BYTE pWriteBuffer[1024 * 16] = { 0 };
	DWORD dwOffset = 0;
	//memset(pWriteBuffer, '\x0', 512 * 5);

	

	//写Start记录
	{
		h.type = rtStart;
		h.len = (WORD)(sizeof(FDStart) + sizeof(int) * m_start->groups);

		m_start->offset[0] = sizeof(FDRecHeader) + h.len;
		for (int i = 1; i < m_start->groups; ++i)
			m_start->offset[i] = (int)(m_start->offset[i - 1] + sizeof(FDRecHeader) + sizeof(FDGroup) + sizeof(int) * m_groups[i - 1].size() + m_groups[i - 1].datasize);
	
		memcpy(pWriteBuffer, &h, sizeof(h));
		dwOffset += sizeof(h);
		
		memcpy(pWriteBuffer + dwOffset, m_start, h.len);
		dwOffset += h.len;
		
		
		/*WriteFile(hFile, &h, sizeof(h), &dwWrite, NULL);
		WriteFile(hFile, m_start, h.len, &dwWrite, NULL);*/
	}

	//写group
	{
		for (int i = 0; i < m_start->groups; ++i)
		{
			GROUP& grp = m_groups[i];

			h.type = rtGroup;
			h.len  =(WORD)(sizeof(FDGroup) + sizeof(int) * grp.size());

			FDGroup* fdgrp = (FDGroup*)malloc(h.len);
			fdgrp->fdcnt = (int)(grp.size());
			fdgrp->groupid = i << 16;

			//偏移到第一个FeatureData记录
			int offset = m_start->offset[i] + sizeof(FDRecHeader) + h.len;

			//确定每一个记录的偏移量
			for (size_t j = 0; j < grp.size(); ++j)
			{
				FDRecHeader* p = grp[j];
				
				fdgrp->offset[j] = offset;

				offset += sizeof(FDRecHeader) + p->len;
			}

			//写FDGroup

			memcpy(pWriteBuffer + dwOffset, &h, sizeof(FDRecHeader));
			dwOffset += sizeof(FDRecHeader);

			memcpy(pWriteBuffer + dwOffset, fdgrp, h.len);
			dwOffset += h.len;

			/*WriteFile(hFile, &h, sizeof(FDRecHeader), &dwWrite, NULL);
			WriteFile(hFile, fdgrp, h.len, &dwWrite, NULL);*/

			//写FeatureDatas

			for (size_t j = 0; j < grp.size(); ++j)
			{
				FDRecHeader* p = grp[j];
				
				memcpy(pWriteBuffer + dwOffset, p, sizeof(FDRecHeader));
				dwOffset += sizeof(FDRecHeader);

				memcpy(pWriteBuffer + dwOffset, p + 1, p->len);
				dwOffset += p->len;

				/*WriteFile(hFile, p, sizeof(FDRecHeader), &dwWrite, NULL);
				WriteFile(hFile, p + 1, p->len, &dwWrite, NULL);*/
			}

			free(fdgrp);
		}
	}

	//写End
	{
		h.type = rtEnd;
		h.len = sizeof(FDEnd);

		FDEnd end = {0};

		memcpy(pWriteBuffer + dwOffset, &h, sizeof(FDRecHeader));
		dwOffset += sizeof(FDRecHeader);

		memcpy(pWriteBuffer + dwOffset, &end, sizeof(FDEnd));
		dwOffset += sizeof(FDEnd);

		/*WriteFile(hFile, &h, sizeof(FDRecHeader), &dwWrite, NULL);
		WriteFile(hFile, &end, sizeof(FDEnd), &dwWrite, NULL);*/
	}

	{//加密数据
		DWORD dwWrite = -1;
		DWORD dwPackedSize = -1;
		BYTE* pBuffer = new BYTE[dwOffset];
		BYTE* pPackedBuffer = NULL;
		

		memset(pBuffer, 0, dwOffset);
		memcpy(pBuffer, pWriteBuffer, dwOffset);

		FDPackInfoHeader info;
		info.wType = rtStart;
		info.wTotalSize = (WORD)dwOffset;

		dwPackedSize = _Pack_Proc(pBuffer, dwOffset, pPackedBuffer);
		WriteFile(hFile, &info, sizeof(FDPackInfoHeader), &dwWrite, NULL);
		WriteFile(hFile, pPackedBuffer, dwPackedSize, &dwWrite, NULL);
		
		dwTotalSize = dwOffset;
		/*if ( pWriteBuffer)
		{
			delete pWriteBuffer;
			pWriteBuffer = NULL;
		}*/

		if ( pBuffer )
		{
			delete []pBuffer;
			pBuffer = NULL;
		}
		
		if ( pPackedBuffer )
		{
			delete []pPackedBuffer;
			pPackedBuffer = NULL;
		}
	}
	if (INVALID_HANDLE_VALUE != hFile)
		CloseHandle(hFile);

	return S_OK;
}

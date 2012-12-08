//////////////////////////////////////////////////////////////////////////////
//
//	Module:		detours.lib
//  File:		image.cpp
//  Summary:	Image manipulation functions (for payloads, byways, and imports).
//
//	Detours for binary functions.  Version 1.5 (Build 46)
//
//	Copyright 1995-2001, Microsoft Corporation
//

#include <ole2.h>
#include <imagehlp.h>

#define DETOURS_INTERNAL
#include "detours.h"

namespace Detour
{
///////////////////////////////////////////////////////////////////////////////
//
class CImageData
{
	friend class CImage;
	
public:
	CImageData(PBYTE pbData, DWORD cbData);
	~CImageData();

	PBYTE 					Enumerate(GUID *pGuid, DWORD *pcbData, DWORD *pnIterator);
	PBYTE					Find(REFGUID rguid, DWORD *pcbData);
	PBYTE					Set(REFGUID rguid, PBYTE pbData, DWORD cbData);
	
	BOOL					Delete(REFGUID rguid);
	BOOL					Purge();

	BOOL					IsEmpty()			{ return m_cbData == 0; }
	BOOL					IsValid();
	
protected:
	BOOL					SizeTo(DWORD cbData);

protected:
	PBYTE					m_pbData;
	DWORD					m_cbData;
	DWORD					m_cbAlloc;
};

class CImageImportFile
{
	friend class CImage;
	friend class CImageImportName;

public:
	CImageImportFile();
	~CImageImportFile();
	
public:
	CImageImportFile *		m_pNextFile;
	BOOL					m_bByway;

	CImageImportName *		m_pImportNames;
	DWORD					m_nImportNames;

	PIMAGE_THUNK_DATA		m_rvaOriginalFirstThunk;
	PIMAGE_THUNK_DATA		m_rvaFirstThunk;
	
	DWORD					m_nForwarderChain;
	PCHAR					m_pszOrig;
	PCHAR					m_pszName;
};

class CImageImportName
{
	friend class CImage;
	friend class CImageImportFile;
	
public:
	CImageImportName();
	~CImageImportName();
	
public:
	WORD		m_nHint;
	DWORD		m_nOrdinal;
	PCHAR		m_pszOrig;
	PCHAR		m_pszName;
};

class CImage
{
	friend class CImageThunks;
	friend class CImageChars;
	friend class CImageImportFile;
	friend class CImageImportName;

public:
	CImage();
	~CImage();

	static CImage *			IsValid(PDETOUR_BINARY pBinary);
	
public:													// File Functions
	BOOL					Read(HANDLE hFile);
	BOOL					Write(HANDLE hFile);
	BOOL 					Close();

public:													// Manipulation Functions
	PBYTE 					DataEnum(GUID *pGuid, DWORD *pcbData, DWORD *pnIterator);
	PBYTE					DataFind(REFGUID rguid, DWORD *pcbData);
	PBYTE					DataSet(REFGUID rguid, PBYTE pbData, DWORD cbData);
	BOOL					DataDelete(REFGUID rguid);
	BOOL					DataPurge();

	BOOL					EditImports(PVOID pContext,
										PF_DETOUR_BINARY_BYWAY_CALLBACK pfBywayCallback,
										PF_DETOUR_BINARY_FILE_CALLBACK pfFileCallback,
										PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbolCallback,
										PF_DETOUR_BINARY_FINAL_CALLBACK pfFinalCallback);
	
protected:
	BOOL 					CopyFileData(HANDLE hFile, DWORD nOldPos, DWORD cbData);
	BOOL 					ZeroFileData(HANDLE hFile, DWORD cbData);
	BOOL					AlignFileData(HANDLE hFile);

	BOOL					SizeOutputBuffer(DWORD cbData);
	PBYTE					AllocateOutput(DWORD cbData, DWORD *pnVirtAddr);
	
	PVOID 					RvaToVa(DWORD nRva);
	DWORD					RvaToFileOffset(DWORD nRva);
	
	DWORD					FileAlign(DWORD nAddr);
	DWORD					SectionAlign(DWORD nAddr);
		
	BOOL					CheckImportsNeeded(DWORD *pnTables,
											   DWORD *pnThunks,
											   DWORD *pnChars);

	CImageImportFile *		NewByway(PCHAR pszName);

private:
	DWORD					m_dwValidSignature;
	CImageData *			m_pImageData;				// Read & Write
	
	HANDLE					m_hMap;						// Read & Write
	PBYTE					m_pMap;						// Read & Write

	DWORD					m_nNextFileAddr;			// Write
	DWORD					m_nNextVirtAddr;			// Write

	IMAGE_NT_HEADERS		m_NtHeader;					// Read & Write
	IMAGE_SECTION_HEADER	m_SectionHeaders[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
	
	DWORD					m_nPeOffset;
	DWORD					m_nSectionsOffset;
	DWORD					m_nExtraOffset;
	DWORD					m_nFileSize;

	DWORD					m_nOutputVirtAddr;
	DWORD					m_nOutputVirtSize;
	DWORD					m_nOutputFileAddr;
	
	PBYTE					m_pbOutputBuffer;
	DWORD					m_cbOutputBuffer;

	CImageImportFile *		m_pImportFiles;
	DWORD					m_nImportFiles;

private:
	enum {
		DETOUR_IMAGE_VALID_SIGNATURE = 0xfedcba01,		// "Dtr\0"
	};
};

//////////////////////////////////////////////////////////////////////////////
//
static inline DWORD Max(DWORD a, DWORD b) 
{
	return a > b ? a : b;
}

static inline DWORD Align(DWORD a, DWORD size)
{
	size--;
	return (a + size) & ~size;
}

static inline DWORD QuadAlign(DWORD a)
{
	return Align(a, 8);
}

static PCHAR DuplicateString(PCHAR pszIn)
{
	if (pszIn) {
		UINT nIn = strlen(pszIn);
		PCHAR pszOut = new CHAR [nIn + 1];
		if (pszOut == NULL) {
			SetLastError(ERROR_OUTOFMEMORY);
		}
		else {
			CopyMemory(pszOut, pszIn, nIn + 1);
		}
		return pszOut;
	}
	return NULL;
}

static PCHAR ReplaceString(PCHAR *ppsz, PCHAR pszIn)
{
	if (ppsz == NULL) {
		return NULL;
	}

	if (strcmp(*ppsz, pszIn) != 0) {
		UINT nIn = strlen(pszIn);
		
		if (strlen(*ppsz) == nIn) {
			CopyMemory(*ppsz, pszIn, nIn + 1);
			return *ppsz;
		}
		else {
			delete[] *ppsz;	
			*ppsz = new CHAR [nIn + 1];
			if (*ppsz == NULL) {
				SetLastError(ERROR_OUTOFMEMORY);
			}
			else {
				CopyMemory(*ppsz, pszIn, nIn + 1);
			}
			return *ppsz;
		}
	}
	return *ppsz;

}

//////////////////////////////////////////////////////////////////////////////
//
CImageImportFile::CImageImportFile()
{
	m_pNextFile = NULL;
	m_bByway = FALSE;
		
	m_pImportNames = NULL;
	m_nImportNames = 0;

	m_rvaOriginalFirstThunk = 0;
	m_rvaFirstThunk = 0;

	m_nForwarderChain = (UINT)-1;
	m_pszName = NULL;
	m_pszOrig = NULL;
}

CImageImportFile::~CImageImportFile()
{
	if (m_pNextFile) {
		delete m_pNextFile;
		m_pNextFile = NULL;
	}
	if (m_pImportNames) {
		delete[] m_pImportNames;
		m_pImportNames = NULL;
		m_nImportNames = 0;
	}
	if (m_pszName) {
		delete[] m_pszName;
		m_pszName = NULL;
	}
	if (m_pszOrig) {
		delete[] m_pszOrig;
		m_pszOrig = NULL;
	}
}

CImageImportName::CImageImportName()
{
	m_nOrdinal = 0;
	m_nHint = 0;
	m_pszName = NULL;
	m_pszOrig = NULL;
}
	
CImageImportName::~CImageImportName()
{
	if (m_pszName) {
		delete[] m_pszName;
		m_pszName = NULL;
	}
	if (m_pszOrig) {
		delete[] m_pszOrig;
		m_pszOrig = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
CImageData::CImageData(PBYTE pbData, DWORD cbData)
{
	m_pbData = pbData;
	m_cbData = cbData;
	m_cbAlloc = 0;
}

CImageData::~CImageData()
{
	IsValid();
	
	if (m_cbAlloc == 0) {
		m_pbData = NULL;
	}
	if (m_pbData) {
		delete[] m_pbData;
		m_pbData = NULL;
	}
	m_cbData = 0;
	m_cbAlloc = 0;
}

BOOL CImageData::SizeTo(DWORD cbData)
{
	IsValid();
	
	if (cbData <= m_cbAlloc) {
		return TRUE;
	}
	
	PBYTE pbNew = new BYTE [cbData];
	if (pbNew == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}
	
	if (m_pbData) {
		CopyMemory(pbNew, m_pbData, m_cbData);
		if (m_cbAlloc > 0) {
			delete[] m_pbData;
		}
		m_pbData = NULL;
	}
	m_pbData = pbNew;
	m_cbAlloc = cbData;

	IsValid();
	
	return TRUE;
}

BOOL CImageData::Purge()
{
	m_cbData = 0;
	
	IsValid();
	
	return TRUE;
}

BOOL CImageData::IsValid()
{
	if (m_pbData == NULL) {
		return TRUE;
	}

	PBYTE pbBeg = m_pbData;
	PBYTE pbEnd = m_pbData + m_cbData;
	
	for (PBYTE pbIter = pbBeg; pbIter < pbEnd;) {
		PDETOUR_SECTION_RECORD pRecord = (PDETOUR_SECTION_RECORD)pbIter;

		if (pRecord->cbBytes < sizeof(DETOUR_SECTION_RECORD)) {
			__asm int 3;
		}
		if (pRecord->nReserved != 0) {
			__asm int 3;
		}
		
		pbIter += pRecord->cbBytes;
	}
	return TRUE;
}

PBYTE CImageData::Enumerate(GUID *pGuid, DWORD *pcbData, DWORD *pnIterator)
{
	IsValid();

	if (pnIterator == NULL ||
		m_cbData < *pnIterator + sizeof(DETOUR_SECTION_RECORD)) {
		
		if (pcbData) {
			*pcbData = 0;
		}
		if (pGuid) {
			ZeroMemory(pGuid, sizeof(*pGuid));
		}
		return NULL;
	}
	
	PDETOUR_SECTION_RECORD pRecord = (PDETOUR_SECTION_RECORD)(m_pbData + *pnIterator);

	if (pGuid) {
		*pGuid = pRecord->guid;
	}
	if (pcbData) {
		*pcbData = pRecord->cbBytes - sizeof(DETOUR_SECTION_RECORD);
	}
	*pnIterator = ((PBYTE)pRecord - m_pbData) + pRecord->cbBytes;
	
	return (PBYTE)(pRecord + 1);
}

PBYTE CImageData::Find(REFGUID rguid, DWORD *pcbData)
{
	IsValid();
	
	DWORD cbBytes = sizeof(DETOUR_SECTION_RECORD);
	for (DWORD nOffset = 0; nOffset < m_cbData; nOffset += cbBytes) {
		PDETOUR_SECTION_RECORD pRecord = (PDETOUR_SECTION_RECORD)(m_pbData + nOffset);

		cbBytes = pRecord->cbBytes;
		if (cbBytes > m_cbData)
			break;
		if (cbBytes < sizeof(DETOUR_SECTION_RECORD))
			continue;

		if (IsEqualGUID(pRecord->guid, rguid)) {
			*pcbData = cbBytes - sizeof(DETOUR_SECTION_RECORD);
			return (PBYTE)(pRecord + 1);
		}
	}
	
	if (pcbData) {
		*pcbData = 0;
	}
	return NULL;
}

BOOL CImageData::Delete(REFGUID rguid)
{
	IsValid();

	PBYTE pbFound = NULL;
	DWORD cbFound = 0;

	pbFound = Find(rguid, &cbFound);
	if (pbFound == NULL) {
		SetLastError(ERROR_MOD_NOT_FOUND);
		return FALSE;
	}

	pbFound -= sizeof(DETOUR_SECTION_RECORD);
	cbFound += sizeof(DETOUR_SECTION_RECORD);
	
	PBYTE pbRestData = pbFound + cbFound;
	DWORD cbRestData = m_cbData - (pbRestData - m_pbData);

	if (cbRestData)
		MoveMemory(pbFound, pbRestData, cbRestData);
	m_cbData -= cbFound;

	IsValid();
	return TRUE;
}

PBYTE CImageData::Set(REFGUID rguid, PBYTE pbData, DWORD cbData)
{
	IsValid();
	Delete(rguid);

	DWORD cbAlloc = QuadAlign(cbData);
	
	if (!SizeTo(m_cbData + cbAlloc + sizeof(DETOUR_SECTION_RECORD))) {
		return NULL;
	}
		
	PDETOUR_SECTION_RECORD pRecord = (PDETOUR_SECTION_RECORD)(m_pbData + m_cbData);
	pRecord->cbBytes = cbAlloc + sizeof(DETOUR_SECTION_RECORD);
	pRecord->nReserved = 0;
	pRecord->guid = rguid;
	
	PBYTE pbDest = (PBYTE)(pRecord + 1);
	if (pbData) {
		CopyMemory(pbDest, pbData, cbData);
		if (cbData < cbAlloc) {
			ZeroMemory(pbDest + cbData, cbAlloc - cbData);
		}
	}
	else {
		if (cbAlloc > 0) {
			ZeroMemory(pbDest, cbAlloc);
		}
	}
	
	m_cbData += cbAlloc + sizeof(DETOUR_SECTION_RECORD);

	IsValid();
	return pbDest;
}

//////////////////////////////////////////////////////////////////////////////
//
class CImageThunks 
{
private:
	CImage *			m_pImage;
	PIMAGE_THUNK_DATA	m_pThunks;
	DWORD				m_nThunks;
	DWORD				m_nThunksMax;
	DWORD				m_nThunkVirtAddr;
		
public:
	CImageThunks(CImage *pImage, DWORD nThunksMax, DWORD *pnAddr) 
	{
		m_pImage = pImage;
		m_nThunks = 0;
		m_nThunksMax = nThunksMax;
		m_pThunks = (PIMAGE_THUNK_DATA)
			m_pImage->AllocateOutput(sizeof(IMAGE_THUNK_DATA) * nThunksMax,
									 &m_nThunkVirtAddr);
		*pnAddr = m_nThunkVirtAddr;
	}
	
	PIMAGE_THUNK_DATA Current(DWORD *pnVirtAddr)
	{
		if (m_nThunksMax > 1) {
			*pnVirtAddr = m_nThunkVirtAddr;
			return m_pThunks;
		}
		*pnVirtAddr = 0;
		return NULL;
	}

	PIMAGE_THUNK_DATA Allocate(DWORD nData, DWORD *pnVirtAddr) 
	{
		if (m_nThunks < m_nThunksMax) {
			*pnVirtAddr = m_nThunkVirtAddr;
			
			m_nThunks++;
			m_nThunkVirtAddr += sizeof(IMAGE_THUNK_DATA);
			m_pThunks->u1.Ordinal = nData;
			return m_pThunks++;
		}
		*pnVirtAddr = 0;
		return NULL;
	}

	DWORD	Size()
	{
		return m_nThunksMax * sizeof(IMAGE_THUNK_DATA);
	}
};
	
//////////////////////////////////////////////////////////////////////////////
//
class CImageChars
{
private:
	CImage *		m_pImage;
	PCHAR			m_pChars;
	DWORD			m_nChars;
	DWORD			m_nCharsMax;
	DWORD			m_nCharVirtAddr;
		
public:
	CImageChars(CImage *pImage, DWORD nCharsMax, DWORD *pnAddr) 
	{
		m_pImage = pImage;
		m_nChars = 0;
		m_nCharsMax = nCharsMax;
		m_pChars = (PCHAR)m_pImage->AllocateOutput(nCharsMax, &m_nCharVirtAddr);
		*pnAddr = m_nCharVirtAddr;
	}

	PCHAR Allocate(PCHAR pszString, DWORD *pnVirtAddr) 
	{
		DWORD nLen = strlen(pszString) + 1;
		nLen += (nLen & 1);

		if (m_nChars + nLen > m_nCharsMax) {
			*pnVirtAddr = 0;
			return NULL;
		}

		*pnVirtAddr = m_nCharVirtAddr;
		strcpy(m_pChars, pszString);

		pszString = m_pChars;
			
		m_pChars += nLen;
		m_nChars += nLen;
		m_nCharVirtAddr += nLen;
			
		return pszString;
	}
		
	PCHAR Allocate(PCHAR pszString, DWORD nHint, DWORD *pnVirtAddr) 
	{
		DWORD nLen = strlen(pszString) + 1 + sizeof(USHORT);
		nLen += (nLen & 1);
			
		if (m_nChars + nLen > m_nCharsMax) {
			*pnVirtAddr = 0;
			return NULL;
		}

		*pnVirtAddr = m_nCharVirtAddr;
		*(USHORT *)m_pChars = (USHORT)nHint;
		strcpy(m_pChars + sizeof(USHORT), pszString);

		pszString = m_pChars + sizeof(USHORT);
		
		m_pChars += nLen;
		m_nChars += nLen;
		m_nCharVirtAddr += nLen;
			
		return pszString;
	}

	DWORD Size()
	{
		return m_nChars;
	}
};

//////////////////////////////////////////////////////////////////////////////
//
CImage * CImage::IsValid(PDETOUR_BINARY pBinary)
{
	if (pBinary) {
		CImage *pImage = (CImage *)pBinary;

		if (pImage->m_dwValidSignature == DETOUR_IMAGE_VALID_SIGNATURE) {
			return pImage;
		}
	}
	SetLastError(ERROR_INVALID_HANDLE);
	return NULL;
}

CImage::CImage()
{
	m_dwValidSignature = (DWORD)DETOUR_IMAGE_VALID_SIGNATURE;

	m_hMap = NULL;
	m_pMap = NULL;
	
	m_nPeOffset = 0;
	m_nSectionsOffset = 0;

	m_pbOutputBuffer = NULL;
	m_cbOutputBuffer = 0;

	m_pImageData = NULL;

	m_pImportFiles = NULL;
	m_nImportFiles = 0;
}

CImage::~CImage()
{
	Close();
	m_dwValidSignature = 0;
}

BOOL CImage::Close()
{
	if (m_pImportFiles) {
		delete m_pImportFiles;
		m_pImportFiles = NULL;
		m_nImportFiles = 0;
	}
	
	if (m_pImageData) {
		delete m_pImageData;
		m_pImageData = NULL;
	}

	if (m_pMap != NULL) {
		UnmapViewOfFile(m_pMap);
		m_pMap = NULL;
	}
	
	if (m_hMap) {
		CloseHandle(m_hMap);
		m_hMap = NULL;
	}
	
	if (m_pbOutputBuffer) {
		delete[] m_pbOutputBuffer;
		m_pbOutputBuffer = NULL;
		m_cbOutputBuffer = 0;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
PBYTE CImage::DataEnum(GUID *pGuid, DWORD *pcbData, DWORD *pnIterator)
{
	if (m_pImageData == NULL) {
		return NULL;
	}
	return m_pImageData->Enumerate(pGuid, pcbData, pnIterator);
}

PBYTE CImage::DataFind(REFGUID rguid, DWORD *pcbData)
{
	if (m_pImageData == NULL) {
		return NULL;
	}
	return m_pImageData->Find(rguid, pcbData);
}

PBYTE CImage::DataSet(REFGUID rguid, PBYTE pbData, DWORD cbData)
{
	if (m_pImageData == NULL) {
		return NULL;
	}
	return m_pImageData->Set(rguid, pbData, cbData);
}

BOOL CImage::DataDelete(REFGUID rguid)
{
	if (m_pImageData == NULL) {
		return FALSE;
	}
	return m_pImageData->Delete(rguid);
}

BOOL CImage::DataPurge()
{
	if (m_pImageData == NULL) {
		return TRUE;
	}
	return m_pImageData->Purge();
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL CImage::SizeOutputBuffer(DWORD cbData)
{
	if (m_cbOutputBuffer < cbData) {
		if (cbData < 1024)	//65536
			cbData = 1024;
		cbData = FileAlign(cbData);

		PBYTE pOutput = new BYTE [cbData];
		if (pOutput == NULL) {
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}

		if (m_pbOutputBuffer) {
			CopyMemory(pOutput, m_pbOutputBuffer, m_cbOutputBuffer);
			
			delete[] m_pbOutputBuffer;
			m_pbOutputBuffer = NULL;
		}

		ZeroMemory(pOutput + m_cbOutputBuffer, cbData - m_cbOutputBuffer), 
		
		m_pbOutputBuffer = pOutput;
		m_cbOutputBuffer = cbData;
	}
	return TRUE;
}

PBYTE CImage::AllocateOutput(DWORD cbData, DWORD *pnVirtAddr)
{
	cbData = QuadAlign(cbData);

	PBYTE pbData = m_pbOutputBuffer + m_nOutputVirtSize;

	*pnVirtAddr = m_nOutputVirtAddr + m_nOutputVirtSize;
	m_nOutputVirtSize += cbData;
	
	if (m_nOutputVirtSize > m_cbOutputBuffer) {
		SetLastError(ERROR_OUTOFMEMORY);
		return NULL;
	}

	ZeroMemory(pbData, cbData);
	
	return pbData;
}

//////////////////////////////////////////////////////////////////////////////
//
DWORD CImage::FileAlign(DWORD nAddr)
{
	return Align(nAddr, m_NtHeader.OptionalHeader.FileAlignment);
}

DWORD CImage::SectionAlign(DWORD nAddr)
{
	return Align(nAddr, m_NtHeader.OptionalHeader.SectionAlignment);
}

//////////////////////////////////////////////////////////////////////////////
//
PVOID CImage::RvaToVa(DWORD nRva)
{
	if (nRva == 0) {
		return NULL;
	}
	
	for (DWORD n = 0; n < m_NtHeader.FileHeader.NumberOfSections; n++) {
		DWORD vaStart = m_SectionHeaders[n].VirtualAddress;
		DWORD vaEnd = vaStart + m_SectionHeaders[n].SizeOfRawData;

		if (nRva >= vaStart && nRva < vaEnd) {
			return (PBYTE)m_pMap
				+ m_SectionHeaders[n].PointerToRawData 
				+ nRva - m_SectionHeaders[n].VirtualAddress;
		}
	}
	return NULL;
}

DWORD CImage::RvaToFileOffset(DWORD nRva)
{
	DWORD n;
	for (n = 0; n < m_NtHeader.FileHeader.NumberOfSections; n++) {
		DWORD vaStart = m_SectionHeaders[n].VirtualAddress;
		DWORD vaEnd = vaStart + m_SectionHeaders[n].SizeOfRawData;
		
		if (nRva >= vaStart && nRva < vaEnd) {
			return m_SectionHeaders[n].PointerToRawData 
				+ nRva - m_SectionHeaders[n].VirtualAddress;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL CImage::CopyFileData(HANDLE hFile, DWORD nOldPos, DWORD cbData)
{
	DWORD cbDone = 0;
	return WriteFile(hFile, m_pMap + nOldPos, cbData, &cbDone, NULL);
}

BOOL CImage::ZeroFileData(HANDLE hFile, DWORD cbData)
{
	if (!SizeOutputBuffer(4096)) {
		return FALSE;
	}
	
	ZeroMemory(m_pbOutputBuffer, 4096);
	
	for (DWORD cbLeft = cbData; cbLeft > 0;) {
		DWORD cbStep = cbLeft > sizeof(m_pbOutputBuffer)
			? sizeof(m_pbOutputBuffer) : cbLeft;
		DWORD cbDone = 0;
		
		if (!WriteFile(hFile, m_pbOutputBuffer, cbStep, &cbDone, NULL)) {
			return FALSE;
		}
		if (cbDone == 0)
			break;

		cbLeft -= cbDone;
	}
	return TRUE;
}

BOOL CImage::AlignFileData(HANDLE hFile)
{
	DWORD nLastFileAddr = m_nNextFileAddr;
	
	m_nNextFileAddr = FileAlign(m_nNextFileAddr);
	m_nNextVirtAddr = SectionAlign(m_nNextVirtAddr);

	if (hFile != INVALID_HANDLE_VALUE) {
		if (m_nNextFileAddr > nLastFileAddr) {
			if (SetFilePointer(hFile, nLastFileAddr, NULL, FILE_BEGIN) == ~0u) {
				return FALSE;
			}
			return ZeroFileData(hFile, m_nNextFileAddr - nLastFileAddr);
		}
	}
	return TRUE;
}

BOOL CImage::Read(HANDLE hFile)
{

	DWORD n;
	PBYTE pbData = NULL;
	DWORD cbData = 0;
	
	if (hFile == INVALID_HANDLE_VALUE) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	///////////////////////////////////////////////////////// Create mapping.
	//
	m_nFileSize = GetFileSize(hFile, NULL);
	if (m_nFileSize == (DWORD)-1) {
		return FALSE;
	}

	m_hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_hMap == NULL) {
		return FALSE;
	}

	m_pMap = (PBYTE)MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0);
	if (m_pMap == NULL) {
		return FALSE;
	}

	////////////////////////////////////////////////////// Process DOS Header.
	//
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_pMap;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		SetLastError(ERROR_BAD_EXE_FORMAT);
		return FALSE;
	}
	m_nPeOffset = pDosHeader->e_lfanew;

	/////////////////////////////////////////////////////// Process PE Header.
	//
	CopyMemory(&m_NtHeader, m_pMap + m_nPeOffset, sizeof(m_NtHeader));
	if (m_NtHeader.Signature != IMAGE_NT_SIGNATURE) {
		SetLastError(ERROR_INVALID_EXE_SIGNATURE);
		return FALSE;
	}
	if (m_NtHeader.FileHeader.SizeOfOptionalHeader == 0) {
		SetLastError(ERROR_EXE_MARKED_INVALID);
		return FALSE;
	}
	m_nSectionsOffset = m_nPeOffset
		+ sizeof(m_NtHeader.Signature)
		+ sizeof(m_NtHeader.FileHeader)
		+ m_NtHeader.FileHeader.SizeOfOptionalHeader;

	///////////////////////////////////////////////// Process Section Headers.
	//
	if (m_NtHeader.FileHeader.NumberOfSections > (sizeof(m_SectionHeaders) /
												  sizeof(m_SectionHeaders[0]))) {
		SetLastError(ERROR_EXE_MARKED_INVALID);
		return FALSE;
	}
	CopyMemory(&m_SectionHeaders,
			   m_pMap + m_nSectionsOffset, 
			   sizeof(m_SectionHeaders[0]) * m_NtHeader.FileHeader.NumberOfSections);

	////////////////////////////////////////////////////////// Parse Sections.
	//
	DWORD rvaOriginalImageDirectory = 0;
	for (n = 0; n < m_NtHeader.FileHeader.NumberOfSections; n++) {
		if (strcmp((PCHAR)m_SectionHeaders[n].Name, ".detour") == 0) {
			DETOUR_SECTION_HEADER dh;
			CopyMemory(&dh,
					   m_pMap + m_SectionHeaders[n].PointerToRawData,
					   sizeof(dh));
			
			rvaOriginalImageDirectory = dh.nOriginalImportVirtualAddress;	
		}
	}

	//////////////////////////////////////////////////////// Get Import Table.
	//
	DWORD rvaImageDirectory = m_NtHeader.OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	PIMAGE_IMPORT_DESCRIPTOR iidp
		= (PIMAGE_IMPORT_DESCRIPTOR)RvaToVa(rvaImageDirectory);
	PIMAGE_IMPORT_DESCRIPTOR oidp
		= (PIMAGE_IMPORT_DESCRIPTOR)RvaToVa(rvaOriginalImageDirectory);
	if (oidp == NULL) {
		oidp = iidp;
	}
	if (iidp == NULL || oidp == NULL) {
		SetLastError(ERROR_EXE_MARKED_INVALID);
		return FALSE;
	}

	DWORD rvaIatBeg = m_NtHeader.OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
	DWORD rvaIatEnd = rvaIatBeg + m_NtHeader.OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size;

	if (iidp) {
		for (DWORD nFiles = 0; iidp[nFiles].Characteristics != 0; nFiles++) {
		}
		
		CImageImportFile **ppLastFile = &m_pImportFiles;
		m_pImportFiles = NULL;

		for (DWORD n = 0; n < nFiles; n++, iidp++) {
			DWORD rvaName = iidp->Name;
			PCHAR pszName = (PCHAR)RvaToVa(rvaName);
			if (pszName == NULL) {
				SetLastError(ERROR_EXE_MARKED_INVALID);
				goto fail;
			}

			CImageImportFile *pImportFile = new CImageImportFile;
			if (pImportFile == NULL) {
				SetLastError(ERROR_OUTOFMEMORY);
				goto fail;
			}

			*ppLastFile = pImportFile;
			ppLastFile = &pImportFile->m_pNextFile;
			m_nImportFiles++;
			
			pImportFile->m_pszName = DuplicateString(pszName);
			if (pImportFile->m_pszName == NULL) {
				goto fail;
			}
			
			pImportFile->m_rvaOriginalFirstThunk
				= (PIMAGE_THUNK_DATA)iidp->OriginalFirstThunk;
			pImportFile->m_rvaFirstThunk = (PIMAGE_THUNK_DATA)iidp->FirstThunk;
			pImportFile->m_nForwarderChain = iidp->ForwarderChain;
			pImportFile->m_pImportNames = NULL;
			pImportFile->m_nImportNames = 0;
			pImportFile->m_bByway = FALSE;

			if ((ULONG)iidp->FirstThunk < rvaIatBeg ||
				(ULONG)iidp->FirstThunk >= rvaIatEnd) {
				
				pImportFile->m_pszOrig = NULL;
				pImportFile->m_bByway = TRUE;
				continue;
			}

			rvaName = oidp->Name;
			pszName = (PCHAR)RvaToVa(rvaName);
			if (pszName == NULL) {
				SetLastError(ERROR_EXE_MARKED_INVALID);
				goto fail;
			}
			pImportFile->m_pszOrig = DuplicateString(pszName);
			if (pImportFile->m_pszOrig == NULL) {
				goto fail;
			}
			
			DWORD rvaThunk = (DWORD)iidp->OriginalFirstThunk;
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)RvaToVa(rvaThunk);
			rvaThunk = (DWORD)oidp->OriginalFirstThunk;
			PIMAGE_THUNK_DATA pOrigThunk = (PIMAGE_THUNK_DATA)RvaToVa(rvaThunk);

			DWORD nNames = 0;
			if (pThunk) {
				for (; pThunk[nNames].u1.Ordinal; nNames++) {
				}
			}

			if (pThunk && nNames) {
				pImportFile->m_nImportNames = nNames;
				pImportFile->m_pImportNames = new CImageImportName [nNames];
				if (pImportFile->m_pImportNames == NULL) {
					SetLastError(ERROR_OUTOFMEMORY);
					goto fail;
				}

				CImageImportName *pImportName = &pImportFile->m_pImportNames[0];
				
				for (DWORD f = 0; f < nNames; f++, pImportName++) {
					pImportName->m_nOrdinal = 0;
					pImportName->m_nHint = 0;
					pImportName->m_pszName = NULL;
					pImportName->m_pszOrig = NULL;
					
					DWORD rvaName = pThunk[f].u1.Ordinal;
					if (rvaName & IMAGE_ORDINAL_FLAG) {
						pImportName->m_nOrdinal = IMAGE_ORDINAL(rvaName);
					}
					else {
						PIMAGE_IMPORT_BY_NAME pName
							= (PIMAGE_IMPORT_BY_NAME)RvaToVa(rvaName);
						if (pName) {
							pImportName->m_nHint = pName->Hint;
							pImportName->m_pszName = DuplicateString((PCHAR)pName->Name);
							if (pImportName->m_pszName == NULL) {
								goto fail;
							}
						}
						
						rvaName = pOrigThunk[f].u1.Ordinal;
						if (rvaName & IMAGE_ORDINAL_FLAG) {
							pImportName->m_nOrdinal = IMAGE_ORDINAL(rvaName);
						}
						else {
							pName = (PIMAGE_IMPORT_BY_NAME)RvaToVa(rvaName);
							if (pName) {
								pImportName->m_pszOrig
									= DuplicateString((PCHAR)pName->Name);
								if (pImportName->m_pszOrig == NULL) {
									goto fail;
								}
							}
						}
					}
				}
			}
			oidp++;
		}
	}
	
	////////////////////////////////////////////////////////// Parse Sections.
	//
	m_nExtraOffset = 0;
	for (n = 0; n < m_NtHeader.FileHeader.NumberOfSections; n++) {
		m_nExtraOffset = Max(m_SectionHeaders[n].PointerToRawData +
							 m_SectionHeaders[n].SizeOfRawData,
							 m_nExtraOffset);
		
		if (strcmp((PCHAR)m_SectionHeaders[n].Name, ".detour") == 0) {
			DETOUR_SECTION_HEADER dh;
			CopyMemory(&dh,
					   m_pMap + m_SectionHeaders[n].PointerToRawData,
					   sizeof(dh));

			if (dh.nDataOffset == 0) {
				dh.nDataOffset = dh.cbHeaderSize;
			}

			cbData = dh.cbDataSize - dh.nDataOffset;
			pbData = (m_pMap +
					  m_SectionHeaders[n].PointerToRawData +
					  dh.nDataOffset);
			
			m_nExtraOffset = Max(m_SectionHeaders[n].PointerToRawData +
								 m_SectionHeaders[n].SizeOfRawData,
								 m_nExtraOffset);
			
			m_NtHeader.FileHeader.NumberOfSections--;

			m_NtHeader.OptionalHeader
				.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
				= dh.nOriginalImportVirtualAddress;
			m_NtHeader.OptionalHeader
				.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size
				= dh.nOriginalImportSize;
			
			m_NtHeader.OptionalHeader
				.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress
				= dh.nOriginalBoundImportVirtualAddress;
			m_NtHeader.OptionalHeader
				.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size
				= dh.nOriginalBoundImportSize;
	
			m_NtHeader.OptionalHeader
				.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress
				= dh.nOriginalIatVirtualAddress;
			m_NtHeader.OptionalHeader
				.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size
				= dh.nOriginalIatSize;

			m_NtHeader.OptionalHeader.CheckSum = 0;
			m_NtHeader.OptionalHeader.SizeOfImage
				= dh.nOriginalSizeOfImage;
		}
	}

	m_pImageData = new CImageData(pbData, cbData);
	if (m_pImageData == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
	}
	return TRUE;

fail:
	return FALSE;
}

static inline BOOL strneq(PCHAR pszOne, PCHAR pszTwo)
{
	if (pszOne == pszTwo) {
		return FALSE;
	}
	if (!pszOne || !pszTwo) {
		return TRUE;
	}
	return (strcmp(pszOne, pszTwo) != 0);
}

BOOL CImage::CheckImportsNeeded(DWORD *pnTables, DWORD *pnThunks, DWORD *pnChars)
{
	DWORD nTables = 0;
	DWORD nThunks = 0;
	DWORD nChars = 0;
	BOOL bNeedDetourSection = FALSE;

	for (CImageImportFile *pImportFile = m_pImportFiles;
		 pImportFile != NULL; pImportFile = pImportFile->m_pNextFile) {
		
		nChars += strlen(pImportFile->m_pszName) + 1;
		nChars += nChars & 1;

		if (pImportFile->m_bByway) {
			bNeedDetourSection = TRUE;
			nThunks++;
		}
		else {
			if (!bNeedDetourSection &&
				strneq(pImportFile->m_pszName, pImportFile->m_pszOrig)) {
				
				bNeedDetourSection = TRUE;
			}
			for (DWORD n = 0; n < pImportFile->m_nImportNames; n++) {
				CImageImportName *pImportName = &pImportFile->m_pImportNames[n];

				if (!bNeedDetourSection &&
					strneq(pImportName->m_pszName, pImportName->m_pszOrig)) {
					
					bNeedDetourSection = TRUE;
				}
				
				if (pImportName->m_pszName) {
					nChars += sizeof(WORD);				// Hint
					nChars += strlen(pImportName->m_pszName) + 1;
					nChars += nChars & 1;
				}
				nThunks++;
			}
		}
		nThunks++;
		nTables++;
	}
	nTables++;

	*pnTables = nTables;
	*pnThunks = nThunks;
	*pnChars = nChars;

	return bNeedDetourSection;
}

//////////////////////////////////////////////////////////////////////////////
//
CImageImportFile * CImage::NewByway(PCHAR pszName)
{
	CImageImportFile *pImportFile = new CImageImportFile;
	if (pImportFile == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		goto fail;
	}

	if (pImportFile) {
		pImportFile->m_pNextFile = NULL;
		pImportFile->m_bByway = TRUE;
		
		pImportFile->m_pszName = DuplicateString(pszName);
		if (pImportFile->m_pszName == NULL) {
			goto fail;
		}
		
		pImportFile->m_rvaOriginalFirstThunk = 0;
		pImportFile->m_rvaFirstThunk = 0;
		pImportFile->m_nForwarderChain = (UINT)-1;
		pImportFile->m_pImportNames = NULL;
		pImportFile->m_nImportNames = 0;

		m_nImportFiles++;
	}
	return pImportFile;

fail:
	if (pImportFile) {
		delete pImportFile;
		pImportFile = NULL;
	}
	return NULL;
}

BOOL CImage::EditImports(PVOID pContext,
						 PF_DETOUR_BINARY_BYWAY_CALLBACK pfBywayCallback,
						 PF_DETOUR_BINARY_FILE_CALLBACK pfFileCallback,
						 PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbolCallback,
						 PF_DETOUR_BINARY_FINAL_CALLBACK pfFinalCallback)
{
	CImageImportFile *pImportFile = NULL;
	CImageImportFile **ppLastFile = &m_pImportFiles;

	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	
	while ((pImportFile = *ppLastFile) != NULL) {
		
		if (pfBywayCallback) {
			PCHAR pszFile = NULL;
			if (!(*pfBywayCallback)(pContext, pszFile, &pszFile)) {
				goto fail;
			}
			
			if (pszFile) {
				// Insert a new Byway.
				CImageImportFile *pByway = NewByway(pszFile);
				if (pByway == NULL)
					return FALSE;
				
				pByway->m_pNextFile = pImportFile;
				*ppLastFile = pByway;
				ppLastFile = &pByway->m_pNextFile;
				continue;								// Retry after Byway.
			}
		}

		if (pImportFile->m_bByway) {
			if (pfBywayCallback) {
				PCHAR pszFile = pImportFile->m_pszName;

				if (!(*pfBywayCallback)(pContext, pszFile, &pszFile)) {
					goto fail;
				}
				
				if (pszFile) {					// Replace? Byway
					if (ReplaceString(&pImportFile->m_pszName, pszFile) == NULL) {
						goto fail;
					}
				}
				else {								// Delete Byway
					*ppLastFile = pImportFile->m_pNextFile;
					pImportFile->m_pNextFile = NULL;
					delete pImportFile;
					pImportFile = *ppLastFile;
					m_nImportFiles--;
					continue;						// Retry after delete.
				}
			}
		}
		else {
			if (pfFileCallback) {
				PCHAR pszFile = pImportFile->m_pszName;
				
				if (!(*pfFileCallback)(pContext, pImportFile->m_pszOrig,
									   pszFile, &pszFile)) {
					goto fail;
				}
				
				if (pszFile != NULL) {
					if (ReplaceString(&pImportFile->m_pszName, pszFile) == NULL) {
						goto fail;
					}
				}
			}

			if (pfSymbolCallback) {
				for (DWORD n = 0; n < pImportFile->m_nImportNames; n++) {
					CImageImportName *pImportName = &pImportFile->m_pImportNames[n];
					
					PCHAR pszName = pImportName->m_pszName;
					if (!(*pfSymbolCallback)(pContext,
											pImportName->m_nOrdinal,
											pImportName->m_pszOrig,
											pszName,
											&pszName)) {
						goto fail;
					}
					
					if (pszName != NULL) {
						if (ReplaceString(&pImportName->m_pszName, pszName) == NULL) {
							goto fail;
						}
					}
				}
			}
		}
		
		ppLastFile = &pImportFile->m_pNextFile;
		pImportFile = pImportFile->m_pNextFile;
	}

	for (;;) {
		if (pfBywayCallback) {
			PCHAR pszFile = NULL;
			if (!(*pfBywayCallback)(pContext, NULL, &pszFile)) {
				goto fail;
			}
			if (pszFile) {
				// Insert a new Byway.
				CImageImportFile *pByway = NewByway(pszFile);
				if (pByway == NULL)
					return FALSE;
				
				pByway->m_pNextFile = pImportFile;
				*ppLastFile = pByway;
				ppLastFile = &pByway->m_pNextFile;
				continue;								// Retry after Byway.
			}
		}
		break;
	}

	if (pfFinalCallback) {
		if (!(*pfFinalCallback)(pContext)) {
			goto fail;
		}
	}
	
	SetLastError(NO_ERROR);
	return TRUE;

fail:
	return FALSE;
}

BOOL CImage::Write(HANDLE hFile)
{
	if (hFile == INVALID_HANDLE_VALUE) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	m_nNextFileAddr = 0;
	m_nNextVirtAddr = 0;
	
	//////////////////////////////////////////////////////////// Copy Headers.
	//
	if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == ~0u) {
		return FALSE;
	}
	if (!CopyFileData(hFile, 0, m_NtHeader.OptionalHeader.SizeOfHeaders)) {
		return FALSE;
	}

	m_nNextFileAddr = m_NtHeader.OptionalHeader.SizeOfHeaders;
	m_nNextVirtAddr = 0;
	if (!AlignFileData(hFile)) {
		return FALSE;
	}
	
	/////////////////////////////////////////////////////////// Copy Sections.
	//
	for (DWORD n = 0; n < m_NtHeader.FileHeader.NumberOfSections; n++) {
		if (m_SectionHeaders[n].SizeOfRawData) {
			if (SetFilePointer(hFile,
							   m_SectionHeaders[n].PointerToRawData,
							   NULL, FILE_BEGIN) == ~0u) {
				return FALSE;
			}
			if (!CopyFileData(hFile,
							  m_SectionHeaders[n].PointerToRawData,
							  m_SectionHeaders[n].SizeOfRawData)) {
				return FALSE;
			}
		}
		m_nNextFileAddr = Max(m_SectionHeaders[n].PointerToRawData +
							  m_SectionHeaders[n].SizeOfRawData,
							  m_nNextFileAddr);
		m_nNextVirtAddr = Max(m_SectionHeaders[n].VirtualAddress +
							  m_SectionHeaders[n].Misc.VirtualSize,
							  m_nNextVirtAddr);
		m_nExtraOffset = Max(m_nNextFileAddr, m_nExtraOffset);
		
		if (!AlignFileData(hFile)) {
			return FALSE;
		}
	}

	/////////////////////////////////////////////////////////////// Old WriteSection
	BOOL bNeedDetourSection;
	DWORD cbDone;

	DWORD nTables = 0;
	DWORD nThunks = 0;
	DWORD nChars = 0;
	bNeedDetourSection = CheckImportsNeeded(&nTables, &nThunks, &nChars);
	
	if (bNeedDetourSection || !m_pImageData->IsEmpty()) {
		/////////////////////////////////////////////////// Insert .detour Section.
		//
		DWORD nSection = m_NtHeader.FileHeader.NumberOfSections++;
		DETOUR_SECTION_HEADER dh;

		ZeroMemory(&dh, sizeof(dh));
		ZeroMemory(&m_SectionHeaders[nSection], sizeof(m_SectionHeaders[nSection]));

		dh.cbHeaderSize = sizeof(DETOUR_SECTION_HEADER);
		dh.nSignature = DETOUR_SECTION_HEADER_SIGNATURE;
	
		dh.nOriginalImportVirtualAddress = m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		dh.nOriginalImportSize = m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		
		dh.nOriginalBoundImportVirtualAddress
			= m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;
		dh.nOriginalBoundImportSize = m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size;
	
		dh.nOriginalIatVirtualAddress = m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
		dh.nOriginalIatSize = m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size;
	
		dh.nOriginalSizeOfImage = m_NtHeader.OptionalHeader.SizeOfImage;
	
		strcpy((PCHAR)m_SectionHeaders[nSection].Name, ".detour");
		m_SectionHeaders[nSection].Characteristics
			= IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

		m_nOutputVirtAddr = m_nNextVirtAddr;
		m_nOutputVirtSize = 0;
		m_nOutputFileAddr = m_nNextFileAddr;

		dh.nDataOffset = 0;						// pbData
		dh.cbDataSize = m_pImageData->m_cbData;

		//////////////////////////////////////////////////////////////////////////
		//
		
		DWORD rvaImportTable = 0;
		DWORD rvaLookupTable = 0;
		DWORD rvaBoundTable = 0;
		DWORD rvaNameTable = 0;
		DWORD nImportTableSize = nTables * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	
		if (!SizeOutputBuffer(QuadAlign(sizeof(dh))
							 + QuadAlign(m_pImageData->m_cbData)
							 + QuadAlign(sizeof(IMAGE_THUNK_DATA) * nThunks)
							 + QuadAlign(sizeof(IMAGE_THUNK_DATA) * nThunks)
							 + QuadAlign(nChars)
							 + QuadAlign(nImportTableSize))) {
			return FALSE;
		}

		DWORD vaHead = 0;
		PBYTE pbHead = NULL;
		DWORD vaData = 0;
		PBYTE pbData = NULL;

		if ((pbHead = AllocateOutput(sizeof(dh), &vaHead)) == NULL) {
			return FALSE;
		}
		
		CImageThunks lookupTable(this, nThunks, &rvaLookupTable);
		CImageThunks boundTable(this, nThunks, &rvaBoundTable);
		CImageChars nameTable(this, nChars, &rvaNameTable);
			
		if ((pbData = AllocateOutput(m_pImageData->m_cbData, &vaData)) == NULL) {
			return FALSE;
		}
		
		dh.nDataOffset = vaData - vaHead;
		dh.cbDataSize = dh.nDataOffset + m_pImageData->m_cbData;
		CopyMemory(pbHead, &dh, sizeof(dh));
		CopyMemory(pbData, m_pImageData->m_pbData, m_pImageData->m_cbData);
	
		PIMAGE_IMPORT_DESCRIPTOR piidDst = (PIMAGE_IMPORT_DESCRIPTOR)
			AllocateOutput(nImportTableSize, &rvaImportTable);
		if (piidDst == NULL) {
			return FALSE;
		}

		//////////////////////////////////////////////// Step Through Imports.
		//
		for (CImageImportFile *pImportFile = m_pImportFiles;
			 pImportFile != NULL; pImportFile = pImportFile->m_pNextFile) {

			ZeroMemory(piidDst, sizeof(piidDst));
			nameTable.Allocate(pImportFile->m_pszName, (DWORD *)&piidDst->Name);
			piidDst->TimeDateStamp = 0;
			piidDst->ForwarderChain = pImportFile->m_nForwarderChain;
			
			if (pImportFile->m_bByway) {
				ULONG rvaIgnored;
				
				lookupTable.Allocate(IMAGE_ORDINAL_FLAG+1,
									 (DWORD *)&piidDst->OriginalFirstThunk);
				boundTable.Allocate(IMAGE_ORDINAL_FLAG+1,
									(DWORD *)&piidDst->FirstThunk);
				
				lookupTable.Allocate(0, &rvaIgnored);
				boundTable.Allocate(0, &rvaIgnored);
			}
			else {
				ULONG rvaIgnored;

				piidDst->FirstThunk = (ULONG)pImportFile->m_rvaFirstThunk;
				lookupTable.Current((DWORD *)&piidDst->OriginalFirstThunk);
				
				for (DWORD n = 0; n < pImportFile->m_nImportNames; n++) {
					CImageImportName *pImportName = &pImportFile->m_pImportNames[n];

					if (pImportName->m_pszName) {
						ULONG nDstName = 0;
						
						nameTable.Allocate(pImportName->m_pszName,
										   pImportName->m_nHint,
										   &nDstName);
						lookupTable.Allocate(nDstName, &rvaIgnored);
					}
					else {
						lookupTable.Allocate(IMAGE_ORDINAL_FLAG+pImportName->m_nOrdinal,
											 &rvaIgnored);
					}
				}
				lookupTable.Allocate(0, &rvaIgnored);
			}
			piidDst++;
		}
		ZeroMemory(piidDst, sizeof(piidDst));
		
		//////////////////////////////////////////////////////////////////////////
		//
		m_nNextVirtAddr += m_nOutputVirtSize;
		m_nNextFileAddr += FileAlign(m_nOutputVirtSize);
	
		if (!AlignFileData(hFile)) {
			return FALSE;
		}
	
		//////////////////////////////////////////////////////////////////////////
		//
		m_SectionHeaders[nSection].VirtualAddress = m_nOutputVirtAddr;
		m_SectionHeaders[nSection].Misc.VirtualSize = m_nOutputVirtSize;
		m_SectionHeaders[nSection].PointerToRawData = m_nOutputFileAddr;
		m_SectionHeaders[nSection].SizeOfRawData = FileAlign(m_nOutputVirtSize);
	
		m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
			= rvaImportTable;
		m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size
			= nImportTableSize;

		m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
		m_NtHeader.OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;

		//////////////////////////////////////////////////////////////////////////
		//
		if (SetFilePointer(hFile, m_SectionHeaders[nSection].PointerToRawData,
						   NULL, FILE_BEGIN) == ~0u) {
			return FALSE;
		}
		if (!WriteFile(hFile, m_pbOutputBuffer, m_SectionHeaders[nSection].SizeOfRawData,
					   &cbDone, NULL)) {
			return FALSE;
		}
	}
	
	///////////////////////////////////////////////////// Adjust Extra Data.
	//
	LONG nExtraAdjust = m_nNextFileAddr - m_nExtraOffset;
	for (n = 0; n < m_NtHeader.FileHeader.NumberOfSections; n++) {
		if (m_SectionHeaders[n].PointerToRawData > m_nExtraOffset)
			m_SectionHeaders[n].PointerToRawData += nExtraAdjust;
		if (m_SectionHeaders[n].PointerToRelocations > m_nExtraOffset)
			m_SectionHeaders[n].PointerToRelocations += nExtraAdjust;
		if (m_SectionHeaders[n].PointerToLinenumbers > m_nExtraOffset)
			m_SectionHeaders[n].PointerToLinenumbers += nExtraAdjust;
	}
	if (m_NtHeader.FileHeader.PointerToSymbolTable > m_nExtraOffset)
		m_NtHeader.FileHeader.PointerToSymbolTable += nExtraAdjust;
		
	m_NtHeader.OptionalHeader.CheckSum = 0;
	m_NtHeader.OptionalHeader.SizeOfImage = m_nNextVirtAddr;
	
	////////////////////////////////////////////////// Adjust Debug Directory.
	//
	DWORD debugAddr = m_NtHeader.OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
	DWORD debugSize = m_NtHeader.OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
	if (debugAddr && debugSize) {
		DWORD nFileOffset = RvaToFileOffset(debugAddr);
		if (SetFilePointer(hFile, nFileOffset, NULL, FILE_BEGIN) == ~0u) {
			return FALSE; 
		}
        
		PIMAGE_DEBUG_DIRECTORY pDir = (PIMAGE_DEBUG_DIRECTORY)RvaToVa(debugAddr);
		if (pDir == NULL) {
			return FALSE;
		}
        
		DWORD nEntries = debugSize / sizeof(*pDir);
		for (DWORD n = 0; n < nEntries; n++) {
            IMAGE_DEBUG_DIRECTORY dir = pDir[n];
            
			dir.PointerToRawData += nExtraAdjust;
            if (!WriteFile(hFile, &dir, sizeof(dir), &cbDone, NULL)) {
                return FALSE;
            }
		}
	}
	
	///////////////////////////////////////////////// Copy Left-over Data.
	//
	if (m_nFileSize > m_nExtraOffset) {
		if (SetFilePointer(hFile, m_nNextFileAddr, NULL, FILE_BEGIN) == ~0u) {
			return FALSE;
		}
		if (!CopyFileData(hFile, m_nExtraOffset, m_nFileSize - m_nExtraOffset)) {
			return FALSE;
		}
	}

	//////////////////////////////////////////////////// Finalize Headers.
	//

	if (SetFilePointer(hFile, m_nPeOffset, NULL, FILE_BEGIN) == ~0u) {
		return FALSE;
	}
	if (!WriteFile(hFile, &m_NtHeader, sizeof(m_NtHeader), &cbDone, NULL)) {
		return FALSE;
	}
	
	if (SetFilePointer(hFile, m_nSectionsOffset, NULL, FILE_BEGIN) == ~0u) {
		return FALSE;
	}
	if (!WriteFile(hFile, &m_SectionHeaders,
				   sizeof(m_SectionHeaders[0])
				   * m_NtHeader.FileHeader.NumberOfSections,
				   &cbDone, NULL)) {
		return FALSE;
	}
	return TRUE;
}

};														// namespace Detour


//////////////////////////////////////////////////////////////////////////////
//
static BOOL CALLBACK ResetBywayCallback(PVOID pContext,
										PCHAR pszFile,
										PCHAR *ppszOutFile)
{
    (void)pContext;
    (void)pszFile;
    
	*ppszOutFile = NULL;
	return TRUE;
}

static BOOL CALLBACK ResetFileCallback(PVOID pContext,
									   PCHAR pszOrigFile,
									   PCHAR pszFile,
									   PCHAR *ppszOutFile)
{
    (void)pContext;
    (void)pszFile;
    
	*ppszOutFile = pszOrigFile;
	return TRUE;
}

static BOOL CALLBACK ResetSymbolCallback(PVOID pContext,
										 DWORD nOrdinal,
										 PCHAR pszOrigSymbol,
										 PCHAR pszSymbol,
										 PCHAR *ppszOutSymbol)
{
    (void)pContext;
    (void)nOrdinal;
    (void)pszSymbol;
    
	*ppszOutSymbol = pszOrigSymbol;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
PDETOUR_BINARY WINAPI DetourBinaryOpen(HANDLE hFile)
{
	Detour::CImage *pImage = new Detour::CImage;
	if (pImage == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}

	if (!pImage->Read(hFile)) {
		delete pImage;
		return FALSE;
	}

	return (PDETOUR_BINARY)pImage;
}

BOOL WINAPI DetourBinaryWrite(PDETOUR_BINARY pdi, HANDLE hFile)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->Write(hFile);
}

PBYTE WINAPI DetourBinaryEnumeratePayloads(PDETOUR_BINARY pdi,
										   GUID *pGuid,
										   DWORD *pcbData,
										   DWORD *pnIterator)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->DataEnum(pGuid, pcbData, pnIterator);
}

PBYTE WINAPI DetourBinaryFindPayload(PDETOUR_BINARY pdi,
									 REFGUID rguid,
									 DWORD *pcbData)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->DataFind(rguid, pcbData);
}

PBYTE WINAPI DetourBinarySetPayload(PDETOUR_BINARY pdi,
									REFGUID rguid,
									PBYTE pbData,
									DWORD cbData)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->DataSet(rguid, pbData, cbData);
}

BOOL WINAPI DetourBinaryDeletePayload(PDETOUR_BINARY pdi,
									 REFGUID rguid)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->DataDelete(rguid);
}

BOOL WINAPI DetourBinaryPurgePayloads(PDETOUR_BINARY pdi)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->DataPurge();
}

BOOL WINAPI DetourBinaryResetImports(PDETOUR_BINARY pdi)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->EditImports(NULL,
							   ResetBywayCallback,
							   ResetFileCallback,
							   ResetSymbolCallback,
							   NULL);
}

BOOL WINAPI DetourBinaryEditImports(PDETOUR_BINARY pdi,
									PVOID pContext,
									PF_DETOUR_BINARY_BYWAY_CALLBACK pfBywayCallback,
									PF_DETOUR_BINARY_FILE_CALLBACK pfFileCallback,
									PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbolCallback,
									PF_DETOUR_BINARY_FINAL_CALLBACK pfFinalCallback)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	return pImage->EditImports(pContext,
							   pfBywayCallback,
							   pfFileCallback,
							   pfSymbolCallback,
							   pfFinalCallback);
}

BOOL WINAPI DetourBinaryClose(PDETOUR_BINARY pdi)
{
	Detour::CImage *pImage = Detour::CImage::IsValid(pdi);
	if (pImage == NULL) {
		return FALSE;
	}

	BOOL bSuccess = pImage->Close();
	delete pImage;
	pImage = NULL;

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL WINAPI DetourBinaryBindA(PCHAR pszFile, PCHAR pszDll, PCHAR pszPath)
{
    PDETOUR_SYM_INFO pSymInfo = DetourLoadImageHlp();
    if (pSymInfo == NULL) {
        SetLastError(ERROR_MOD_NOT_FOUND);
        return FALSE;
    }
    if (pSymInfo->pfBindImage) {
        return (*pSymInfo->pfBindImage)(pszFile, pszDll ? pszDll : ".", pszPath ? pszPath : ".");
    }
    SetLastError(ERROR_INVALID_FUNCTION);
    return FALSE;
}

static void UnicodeToOem(PWCHAR pwzIn, PCHAR pszOut, INT cbOut)
{
    cbOut = WideCharToMultiByte(CP_OEMCP, 0,
                                pwzIn, wcslen(pwzIn),
                                pszOut, cbOut-1,
                                NULL, NULL);
    pszOut[cbOut] = '\0';
}

BOOL WINAPI DetourBinaryBindW(PWCHAR pwzFile, PWCHAR pwzDll, PWCHAR pwzPath)
{
    PDETOUR_SYM_INFO pSymInfo = DetourLoadImageHlp();
    if (pSymInfo == NULL) {
        SetLastError(ERROR_MOD_NOT_FOUND);
        return FALSE;
    }
    
    CHAR szFile[MAX_PATH];
    CHAR szDll[MAX_PATH];
    CHAR szPath[MAX_PATH];

    UnicodeToOem(pwzFile, szFile, sizeof(szFile));
    UnicodeToOem(pwzDll, szDll, sizeof(szDll));
    UnicodeToOem(pwzPath, szPath, sizeof(szPath));

    if (pSymInfo->pfBindImage) {
        return (pSymInfo->pfBindImage)(szFile, szDll, szPath);
    }
    SetLastError(ERROR_INVALID_FUNCTION);
    return FALSE;
}
//
///////////////////////////////////////////////////////////////// End of File.

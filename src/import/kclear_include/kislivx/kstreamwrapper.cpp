#include "StdAfx.h"
#include "KStreamWrapper.h"

CFileOutStream::CFileOutStream(LPCWSTR lpszFile)
	:hFile_(NULL)
{
	hFile_ = ::CreateFileW(lpszFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

CFileOutStream::~CFileOutStream()
{
	if (hFile_ && INVALID_HANDLE_VALUE != hFile_)
	{
		::CloseHandle(hFile_);
		hFile_ = NULL;
	}
}

HRESULT CFileOutStream::Write(IN const byte * pbyBuf, IN u_int uSize, OUT u_int * pWrittenSize)
{
	HRESULT hResult = E_FAIL;

	if (hFile_ && INVALID_HANDLE_VALUE != hFile_)
	{
		DWORD dwWrittenSize = 0;
		::WriteFile(hFile_, pbyBuf, uSize, &dwWrittenSize, NULL);

		if (pWrittenSize)
			*pWrittenSize = dwWrittenSize;
		
		if (dwWrittenSize == uSize)
			hResult = S_OK;
		else
			hResult = ::GetLastError();
	}

	return hResult;
}

CMemOutStream::CMemOutStream(std::string& _byBuffer)
	:byBuffer_(_byBuffer)
{
}

CMemOutStream::~CMemOutStream()
{
}

HRESULT CMemOutStream::Write(IN const byte * pbyBuf, IN u_int uSize, OUT u_int * pWrittenSize)
{
	u_int uWrittenSize = 0;

	if (pbyBuf && uSize > 0)
	{
		this->byBuffer_.append((char*)pbyBuf, uSize);
		
		uWrittenSize = uSize;
	}

	if (pWrittenSize)
		*pWrittenSize = uWrittenSize;
	
	return S_OK;
}

// KStreamWrapper.h
// 各种流的操作包装。

#pragma once
#ifndef __UPLIVE_KSTREAM_WRAPPER_H__
#define __UPLIVE_KSTREAM_WRAPPER_H__
#include "uplive_common.h"
#include "iseqstream.h"

class CFileOutStream
	: public IOutStream
{
public:
	CFileOutStream(LPCWSTR lpszFile);
	~CFileOutStream();
public:
	virtual HRESULT Write(IN const byte * pbyBuf, IN u_int uSize, OUT u_int * pWrittenSize);
protected:
	HANDLE hFile_;
};

class CMemOutStream
	: public IOutStream
{
public:
	CMemOutStream(std::string& _byBuffer);
	~CMemOutStream();
public:
	virtual HRESULT Write(IN const byte * pbyBuf, IN u_int uSize, OUT u_int * pWrittenSize);
protected:
	std::string& byBuffer_;
};

#endif //__UPLIVE_KSTREAM_WRAPPER_H__
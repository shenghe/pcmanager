#pragma once
#include "Defines.h"

class CFileStream
{
public:
	CFileStream(LPCTSTR szFilename);
	~CFileStream();
	BOOL SetLength(uint64 iFileSize);
	BOOL Create();
	BOOL Write(uint64 iPosition, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);
	BOOL Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);
	void CloseFile();
protected:
	HANDLE m_hFile;
	uint64 m_iFileLength;
	CString m_strFilePath;
};

class CFileInStream
{
public:
	CFileInStream(LPCTSTR szFilename);
	~CFileInStream();
	BOOL Create();
	BOOL Read(LPVOID lpBuffer,DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);
	void CloseFile();
	int64 GetFileSize();

protected:
	HANDLE m_hFile;
	uint64 m_iFileLength;
	CString m_strFilePath;
};

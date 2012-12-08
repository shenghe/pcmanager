

#pragma once

#include <md5/md5.h>

inline void MD5ToString(BYTE* lpMD5, LPTSTR lpStr)
{
	for ( int i = 0; i < 16; i++ )
	{
		BYTE v = *lpMD5;

		*lpStr++ = _T("0123456789abcdef")[v >> 4];
		*lpStr++ = _T("0123456789abcdef")[v & 0xf];

		lpMD5++;
	}

	*lpStr = 0;
}

inline LPCTSTR GetBufferMD5(PVOID pBuffer, DWORD nSzie, LPTSTR lpMD5Str)
{
	MD5_CTX md5Ctx;
	BYTE	Buffer[512];

	MD5Init(&md5Ctx);
	MD5Update(&md5Ctx,(unsigned char*)pBuffer,nSzie);
	MD5Final(Buffer, &md5Ctx);
	MD5ToString(Buffer, lpMD5Str);

	return lpMD5Str;
}

inline LPCTSTR GetFileMD5(LPCTSTR lpPath, LPTSTR lpMD5Str)
{
	MD5_CTX md5Ctx;
	BYTE	Buffer[512];

	MD5Init(&md5Ctx);

	HANDLE hFile = CreateFile(lpPath, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, INVALID_HANDLE_VALUE);
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD	dwLength;

		while ( ReadFile(hFile, Buffer, 512, &dwLength, NULL) &&
			dwLength != 0
			)
		{
			MD5Update(&md5Ctx, Buffer, dwLength);
		}

		CloseHandle(hFile);
	}

	MD5Final(Buffer, &md5Ctx);
	MD5ToString(Buffer, lpMD5Str);

	return lpMD5Str;
}

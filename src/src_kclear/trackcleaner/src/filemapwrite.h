
#ifndef _MY_FILEMAPWRITE_H
#define _MY_FILEMAPWRITE_H
#ifdef _UNIX
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif
#ifdef _WIN_32
#include <windows.h>
#endif

class CFileMapWrite
{
public:

	DWORD GetSize()
	{
		return m_dwSize;
	}
	
	void Close()
	{
		#ifdef WIN32
		if (m_pBase) 
		{
			UnmapViewOfFile(m_pBase);
			m_pBase = NULL;
		}
		if (m_hMapping != INVALID_HANDLE_VALUE) 
		{
			CloseHandle(m_hMapping);
			m_hMapping = INVALID_HANDLE_VALUE;
		}
		if (m_hFile != INVALID_HANDLE_VALUE) 
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
		#endif

		#ifdef _UNIX
		if(m_pBase)
		{
			munmap(m_pBase, m_dwSize);
			m_pBase = NULL;
		}
		if(m_nFd!=-1)
		{
			close(m_nFd);
		}
		#endif
	}
	bool GetFileName(LPCTSTR *pFileName)
	{
		if (!pFileName) return false;
		*pFileName = m_szFileName;

		return true;
	}
public:
	BOOL GetData(LPBYTE * ppBuffer)
	{
		if (m_pBase)
		{
			*ppBuffer = m_pBase;
			return true;
		}
		return false;
	}

	CFileMapWrite(LPCTSTR lpFileName)
	{
		m_pBase = NULL;
		m_dwSize = 0 ;
		//wcsncpy(m_szFileName, lpFileName, sizeof(m_szFileName) );
		
		wmemset(m_szFileName,0,MAX_PATH);
		memcpy(m_szFileName,lpFileName,2*wcslen(lpFileName));

		#ifdef WIN32
		m_hMapping = m_hFile = INVALID_HANDLE_VALUE;
		static DWORD dwShareMode = 0;
		if (0 == dwShareMode)
		{
			dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
			if (GetVersion() < 0x80000000)	// Windows NT/2000)
				dwShareMode |= FILE_SHARE_DELETE ;
		}
		m_hFile = CreateFile(lpFileName , GENERIC_READ | GENERIC_WRITE, dwShareMode, NULL , 
			OPEN_EXISTING , 0 , NULL);

		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			// 取数据长度
			m_dwSize = GetFileSize(m_hFile , NULL);
			m_hMapping = CreateFileMapping(m_hFile ,  NULL , PAGE_READWRITE , 0 , 0 , NULL);
			if (m_hMapping !=NULL)
			{
				m_pBase = (LPBYTE)MapViewOfFile(m_hMapping , FILE_MAP_READ | FILE_MAP_WRITE, 0 , 0 , 0);
			}
		}
		#endif
		
		#ifdef _UNIX
		m_nFd = open(lpFileName,O_RDONLY);
		struct stat status;
		if( m_nFd != -1 )
		{
			lstat(lpFileName,&status);
			m_dwSize = status.st_size;
			m_pBase = (LPBYTE)mmap(NULL,m_dwSize,PROT_READ,MAP_SHARED,m_nFd,0);	
		}
		#endif
	}
	virtual ~CFileMapWrite()
	{
		Close();
	}
private:
	#ifdef WIN32
	HANDLE  m_hMapping, m_hFile;
	#endif//WIN32

	#ifdef _UNIX
	int m_nFd;	
	#endif//_UNIX
	TCHAR 	m_szFileName[MAX_PATH];
	LPBYTE  m_pBase;
	DWORD   m_dwSize;
};
#endif//_MY_FILEMAPWRITE_H

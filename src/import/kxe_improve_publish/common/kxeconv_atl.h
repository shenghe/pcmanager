#pragma once

#include <atlbase.h>
#include <atlconv.h>

#if _MSC_VER == 1200

// #ifndef ATLENSURE_THROW
// #define ATLENSURE_THROW(expr, hr)          \
// 	do {                                       \
// 	int __atl_condVal=!!(expr);            \
// 	ATLASSERT(__atl_condVal);              \
// 	if(!(__atl_condVal)) AtlThrow(hr);     \
// 	} while (0)
// #endif // ATLENSURE
// 
// 
inline void __declspec(noreturn) _AtlRaiseException( DWORD dwExceptionCode, DWORD dwExceptionFlags = EXCEPTION_NONCONTINUABLE )
{
	RaiseException( dwExceptionCode, dwExceptionFlags, 0, NULL );
}


inline void WINAPI AtlThrow( HRESULT hr )
{		
	DWORD dwExceptionCode;
	switch(hr)
	{
	case E_OUTOFMEMORY:
		dwExceptionCode = STATUS_NO_MEMORY;
		break;
	default:
		dwExceptionCode = EXCEPTION_ILLEGAL_INSTRUCTION;
	}
	_AtlRaiseException((DWORD)dwExceptionCode);
}
 

#define ATLCONV_DEADLAND_FILL 0xFD

// Throw a CAtlException corresponding to the result of ::GetLastError
inline void WINAPI AtlThrowLastWin32()
{
	DWORD dwError = ::GetLastError();
	AtlThrow ( HRESULT_FROM_WIN32( dwError ) );
}

template <class _CharType>
inline  void AtlConvAllocMemory(_CharType** ppBuff, int nLength, _CharType* pszFixedBuffer, int nFixedBufferLength)
{
	//ATLENSURE_THROW(ppBuff != NULL, E_INVALIDARG);
	//ATLENSURE_THROW(nLength >= 0, E_INVALIDARG);
	//ATLENSURE_THROW(pszFixedBuffer != NULL, E_INVALIDARG);
	
	//if buffer malloced, try to realloc.
	if (*ppBuff != pszFixedBuffer)
	{
		if( nLength > nFixedBufferLength )
		{
			_CharType* ppReallocBuf = static_cast< _CharType* >( realloc(*ppBuff, nLength * sizeof( _CharType ) ) );
			if (ppReallocBuf == NULL) 
			{
				AtlThrow( E_OUTOFMEMORY );
			}
			*ppBuff = ppReallocBuf;
		} else
		{
			free(*ppBuff);
			*ppBuff=pszFixedBuffer;
		}
		
	} else //Buffer is not currently malloced.
	{
		if( nLength > nFixedBufferLength )
		{
			*ppBuff = static_cast< _CharType* >( calloc(nLength,sizeof( _CharType ) ) );
		} else
		{			
			*ppBuff=pszFixedBuffer;
		}
	}
	
	if (*ppBuff == NULL)
	{
		AtlThrow( E_OUTOFMEMORY );
	}
	
}

template <class _CharType>
inline void AtlConvFreeMemory(_CharType* pBuff,_CharType* pszFixedBuffer,int nFixedBufferLength)
{	
	if( pBuff != pszFixedBuffer )
	{
		free( pBuff );
	} 	
#ifdef _DEBUG
	else
	{		
		memset(pszFixedBuffer,ATLCONV_DEADLAND_FILL,nFixedBufferLength*sizeof(_CharType));
	}
#endif
	
}

template< int t_nBufferLength = 128 >
class CA2WEX
{
public:
	CA2WEX( LPCSTR psz ) :
	  m_psz( m_szBuffer )
	  {
		  Init( psz, CP_ACP );
	  }
	  CA2WEX( LPCSTR psz, UINT nCodePage ):
	  m_psz( m_szBuffer )
	  {
		  Init( psz, nCodePage );
	  }
	  ~CA2WEX() throw()
	  {
		  AtlConvFreeMemory(m_psz,m_szBuffer,t_nBufferLength);
	  }
	  
	  operator LPWSTR() const throw()
	  {
		  return( m_psz );
	  }
	  
private:
	void Init( LPCSTR psz, UINT nCodePage ) 
	{
		if (psz == NULL)
		{
			m_psz = NULL;
			return;
		}
		int nLengthA = lstrlenA( psz )+1;
		int nLengthW = nLengthA;
		
		AtlConvAllocMemory(&m_psz,nLengthW,m_szBuffer,t_nBufferLength);
		
		BOOL bFailed=(0 == ::MultiByteToWideChar( nCodePage, 0, psz, nLengthA, m_psz, nLengthW ) );
		if (bFailed)
		{
			if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				nLengthW = ::MultiByteToWideChar( nCodePage, 0, psz, nLengthA, NULL, 0);
				AtlConvAllocMemory(&m_psz,nLengthW,m_szBuffer,t_nBufferLength);
				bFailed=(0 == ::MultiByteToWideChar( nCodePage, 0, psz, nLengthA, m_psz, nLengthW ) );
			}			
		}
		if (bFailed)
		{
			AtlThrowLastWin32();
		}		
	}
	
public:
	LPWSTR m_psz;
	wchar_t m_szBuffer[t_nBufferLength];
	
private:
	CA2WEX( const CA2WEX& ) throw();
	CA2WEX& operator=( const CA2WEX& ) throw();
};
typedef CA2WEX<> CA2W;

template< int t_nBufferLength = 128 >
class CW2AEX
{
public:
	CW2AEX( LPCWSTR psz ) :
	  m_psz( m_szBuffer )
	  {
		  Init( psz, CP_ACP );
	  }
	  CW2AEX( LPCWSTR psz, UINT nCodePage ) :
	  m_psz( m_szBuffer )
	  {
		  Init( psz, nCodePage );
	  }
	  ~CW2AEX() throw()
	  {		
		  AtlConvFreeMemory(m_psz,m_szBuffer,t_nBufferLength);
	  }
	  
	  operator LPSTR() const throw()
	  {
		  return( m_psz );
	  }
	  
private:
	void Init( LPCWSTR psz, UINT nConvertCodePage ) 
	{
		if (psz == NULL)
		{
			m_psz = NULL;
			return;
		}
		int nLengthW = lstrlenW( psz )+1;		 
		int nLengthA = nLengthW*4;
		
		AtlConvAllocMemory(&m_psz,nLengthA,m_szBuffer,t_nBufferLength);
		
		BOOL bFailed=(0 == ::WideCharToMultiByte( nConvertCodePage, 0, psz, nLengthW, m_psz, nLengthA, NULL, NULL ));
		if (bFailed)
		{
			if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				nLengthA = ::WideCharToMultiByte( nConvertCodePage, 0, psz, nLengthW, NULL, 0, NULL, NULL );
				AtlConvAllocMemory(&m_psz,nLengthA,m_szBuffer,t_nBufferLength);
				bFailed=(0 == ::WideCharToMultiByte( nConvertCodePage, 0, psz, nLengthW, m_psz, nLengthA, NULL, NULL ));
			}			
		}
		if (bFailed)
		{
			AtlThrowLastWin32();
		}
	}
	
public:
	LPSTR m_psz;
	char m_szBuffer[t_nBufferLength];
	
private:
	CW2AEX( const CW2AEX& ) throw();
	CW2AEX& operator=( const CW2AEX& ) throw();
};
typedef CW2AEX<> CW2A;

#endif


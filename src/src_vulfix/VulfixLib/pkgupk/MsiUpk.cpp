#include "StdAfx.h"
#include "MsiUpk.h"
#include <MsiQuery.h>
#include <fstream>
//#pragma comment(lib, "msi.lib")

typedef UINT (WINAPI *FnMsiOpenDatabaseW)( LPCWSTR szDatabasePath, LPCWSTR szPersist, MSIHANDLE* phDatabase);
typedef UINT (WINAPI *FnMsiCloseHandle)(MSIHANDLE hAny);
typedef UINT (WINAPI *FnMsiDatabaseOpenViewW)(MSIHANDLE hDatabase, LPCWSTR szQuery, MSIHANDLE*  phView); 
typedef UINT (WINAPI *FnMsiViewExecute)(MSIHANDLE hView, MSIHANDLE hRecord);
typedef UINT (WINAPI *FnMsiViewFetch)(MSIHANDLE hView, MSIHANDLE *phRecord);
typedef UINT (WINAPI *FnMsiRecordGetStringW)(MSIHANDLE hRecord,
											 UINT iField,
											 __out_ecount_opt(*pcchValueBuf) LPWSTR  szValueBuf,      // buffer for returned value
											 __inout_opt                     LPDWORD  pcchValueBuf);   // in/out buffer character count
typedef UINT (WINAPI *FnMsiRecordReadStream)(MSIHANDLE hRecord, UINT iField, __out_bcount_opt(*pcbDataBuf) char *szDataBuf, __inout LPDWORD pcbDataBuf);

struct CMsiCaller
{
protected:
	HMODULE					m_hModule;

public:
	FnMsiOpenDatabaseW		_MsiOpenDatabaseW;
	FnMsiCloseHandle		_MsiCloseHandle;
	FnMsiDatabaseOpenViewW	_MsiDatabaseOpenViewW;
	FnMsiViewExecute		_MsiViewExecute;
	FnMsiViewFetch			_MsiViewFetch;
	FnMsiRecordGetStringW	_MsiRecordGetStringW;
	FnMsiRecordReadStream	_MsiRecordReadStream;	

public:
	CMsiCaller()
		: m_hModule(NULL)
		, _MsiOpenDatabaseW(NULL)
		, _MsiCloseHandle(NULL)
		, _MsiDatabaseOpenViewW(NULL)
		, _MsiViewExecute(NULL)
		, _MsiViewFetch(NULL)
		, _MsiRecordGetStringW(NULL)
		, _MsiRecordReadStream(NULL)
	{
		Init();
	}
	~CMsiCaller()
	{
		if(m_hModule)
			FreeLibrary(m_hModule);
	}
	
	BOOL Init()
	{
		m_hModule = ::LoadLibrary(_T("msi.dll"));
		if(!m_hModule) return FALSE;
		
#define _FillFunc(x) _##x = (Fn##x)::GetProcAddress(m_hModule,#x)
		_FillFunc(MsiOpenDatabaseW);
		_FillFunc(MsiCloseHandle);
		_FillFunc(MsiDatabaseOpenViewW);
		_FillFunc(MsiViewExecute);
		_FillFunc(MsiViewFetch);
		_FillFunc(MsiRecordGetStringW);
		_FillFunc(MsiRecordReadStream);
#undef _FillFunc
		
		return IsValid();
	}
	
	BOOL IsValid()
	{
		return _MsiOpenDatabaseW && _MsiCloseHandle && _MsiDatabaseOpenViewW 
			&& _MsiViewExecute && _MsiViewFetch && _MsiRecordGetStringW && _MsiRecordReadStream;
	}
};

static CMsiCaller _msicaller;


BOOL					IsLibPkgUpkValid()
{
	return _msicaller.IsValid();
}

class PMSIHANDLE2
{
	MSIHANDLE m_h;
public:
	PMSIHANDLE2():m_h(0){}
	PMSIHANDLE2(MSIHANDLE h):m_h(h){}
	~PMSIHANDLE2(){if (m_h!=0) _msicaller._MsiCloseHandle(m_h);}
	void operator =(MSIHANDLE h) {if (m_h) _msicaller._MsiCloseHandle(m_h); m_h=h;}
	operator MSIHANDLE() {return m_h;}
	MSIHANDLE* operator &() {if (m_h) _msicaller._MsiCloseHandle(m_h); m_h = 0; return &m_h;}
};

BOOL ExtractMSP(LPCTSTR szDatabasePath, LPCTSTR pszDir, Files &files)
{
	DWORD dwError = NOERROR;
	LPCTSTR pszPersist = (LPTSTR)MSIDBOPEN_READONLY;
	PMSIHANDLE2 hDatabase = NULL;
	PMSIHANDLE2 hView = NULL;
	PMSIHANDLE2 hRecord = NULL;
	
	if(!_msicaller.IsValid())
		return FALSE;
	
	pszPersist = MSIDBOPEN_READONLY + MSIDBOPEN_PATCHFILE;
	// Now open the database using MSI APIs. Patches cannot be opened simultaneously
	// since exclusive access is required and no MSI APIs are exported that accept
	// an IStorage pointer.
	dwError = _msicaller._MsiOpenDatabaseW(szDatabasePath, pszPersist, &hDatabase);
	if (ERROR_SUCCESS == dwError)
	{
		dwError = _msicaller._MsiDatabaseOpenViewW(hDatabase,
			TEXT("SELECT `Name`, `Data` FROM `_Streams`"), &hView);
		if (ERROR_SUCCESS == dwError)
		{
			dwError = _msicaller._MsiViewExecute(hView, NULL);
			if (ERROR_SUCCESS == dwError)
			{
				while (ERROR_SUCCESS == (dwError = _msicaller._MsiViewFetch(hView, &hRecord)))
				{
					CString strFilename;
					dwError = SaveStream(hRecord, pszDir, strFilename);
					if(dwError==ERROR_SUCCESS||dwError==E_ABORT)
					{
						if(dwError==ERROR_SUCCESS)
							files.push_back( strFilename );
					}
					else
						break;
				}
				
				// If there are no more records indicate success.
				if (ERROR_NO_MORE_ITEMS == dwError)
				{
					dwError = ERROR_SUCCESS;
				}
			}
		}
	}
	_msicaller._MsiCloseHandle(hDatabase);
	return ERROR_SUCCESS == dwError;
}

// Wrapper around FormatMessage for getting error text.
// Calls error() to print the error to the console.
void win32_error(DWORD dwError)
{
	LPTSTR pszError;
	
	// Format the error. Error ends with new line.
	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &pszError,
		0,
		NULL))
	{
		DBG_TRACE(_T("Error 0x%1$08x (%1$d): %2$s"), dwError, pszError);
		LocalFree(pszError);
	}
}

// Wrapper around allocating and filling a buffer using MsiRecordGetString().
UINT GetString(MSIHANDLE hRecord, UINT iField, LPTSTR* ppszProperty, DWORD* pcchProperty)
{
	_ASSERTE(hRecord);
	_ASSERTE(iField > 0);
	_ASSERTE(ppszProperty);
	_ASSERTE(pcchProperty);

	UINT iErr = NOERROR;
	DWORD cchProperty = 0;

	iErr = _msicaller._MsiRecordGetStringW(hRecord, iField, TEXT(""), &cchProperty);
	if (ERROR_MORE_DATA == iErr)
	{
		*ppszProperty = new TCHAR[++cchProperty];
		*pcchProperty = cchProperty;

		iErr = _msicaller._MsiRecordGetStringW(hRecord, iField, *ppszProperty, &cchProperty);
		if (ERROR_SUCCESS != iErr)
		{
			delete [] *ppszProperty;
			*ppszProperty = NULL;
			*pcchProperty = 0;
		}
	}
	return iErr;
}

// Saves the stream from the given record to a file with or without
// an extension based on whether or not fIncludeExt is set to TRUE.
UINT SaveStream(MSIHANDLE hRecord, LPCTSTR pszDir, CString &strFilename)
{
	UINT uiError = NOERROR;
	TCHAR szPath[MAX_PATH];
	LPTSTR pszName = NULL;
	DWORD cchName = 0;
	CHAR szBuffer[256];
	DWORD cbBuffer = sizeof(szBuffer);
	std::ofstream file;
	
	try
	{
		// Get the name of the stream but skip if \005SummaryInformation stream.
		if (ERROR_SUCCESS == GetString(hRecord, 1, &pszName, &cchName))
		{
			if( 0 == _tcsncmp(pszName, TEXT("\005"), 1) )
			{
				uiError = E_ABORT;
			}
			else
			{
				// Create the local file with the simple CFile write-only class.
				do
				{
					uiError = _msicaller._MsiRecordReadStream(hRecord, 2, szBuffer, &cbBuffer);
					if (ERROR_SUCCESS == uiError)
					{
						if (!file.is_open())
						{
							// Create the file path if the file is not created and assume the extension
							// if requested by fIncludeExt.
							LPCTSTR pszExt = MakePathForData(szBuffer, cbBuffer);
							if (!MakePath(szPath, MAX_PATH, pszDir, pszName, pszExt))
								break;

							strFilename = szPath;
							// Create the local file in which data is written.
							DBG_TRACE(_T("%s\n"), szPath);						
							if(!pszExt || _tcsicmp(pszExt, _T(".cab"))!=0)
							{
								uiError = E_ABORT;
								break;
							}						
							file.open(szPath, std::ios_base::binary);
						}
						file.write(szBuffer, cbBuffer );
					}
					else
					{
						throw std::exception("Could not read from stream.");
					}
				} while (cbBuffer);
			}			
		}
	}
	catch (std::exception& ex)
	{
		_tcprintf(TEXT("Error: %s\n"), CA2T(ex.what()));
		uiError = ERROR_CANNOT_MAKE;
	}
	
	file.close();
	if (pszName)
	{
		delete [] pszName;
		pszName = NULL;
	}
	return uiError;
}

// Creates a patch for the given file using MakePath, but uses what of the
// buffer it can to guess the file type and infer a common file extension.
LPTSTR MakePathForData(LPCVOID pBuffer, size_t cbBuffer)
{
	LPTSTR pszExt = NULL;
	// Cabinet (*.cab) files.
	if (0 == memcmp(pBuffer, "MSCF", 4))
	{
		pszExt = TEXT(".cab");
	}
	
	// Executable files. Assumed to be .dll (more common).
	else if (0 == memcmp(pBuffer, "MZ", 2))
	{
		pszExt = TEXT(".dll");
	}

	// Icon (*.ico) files. Only assumed because they're common.
	else if (0 == memcmp(pBuffer, "\0\0\1\0", 4))
	{
		pszExt = TEXT(".ico");
	}

	// Bitmap (*.bmp) files.
	else if (0 == memcmp(pBuffer, "BM", 2))
	{
		pszExt = TEXT(".bmp");
	}

	// GIF (*.gif) files.
	else if (0 == memcmp(pBuffer, "GIF", 3))
	{
		pszExt = TEXT(".gif");
	}

	// PING (*.png) files.
	else if (0 == memcmp(pBuffer, "\x89PNG", 4))
	{
		pszExt = TEXT(".png");
	}

	// TIFF (*.tif) files.
	else if (0 == memcmp(pBuffer, "II", 2))
	{
		pszExt = TEXT(".tif");
	}

	return pszExt;
}


// Creates a patch from components, using the current working
// directory if pszDir is NULL.
// pszExt should be either NULL or start with a ".".
LPTSTR MakePath(LPTSTR pszDest, size_t cchDest, LPCTSTR pszDir, LPCTSTR pszName, LPCTSTR pszExt)
{
	size_t len = 0;

	_ASSERTE(pszDest);
	_ASSERTE(cchDest);
	_ASSERTE(pszName);

	// Make sure pszDest is NULL-terminated.
	pszDest[0] = TEXT('\0');

	if (pszDir)
	{
		// Get the length of pszDir.
		len = _tcslen(pszDir);

		if (len && 0 != _tcsncpy_s(pszDest, cchDest, pszDir, len))
		{
			return NULL;
		}

		if (len && TEXT('\\') != pszDest[len - 1])
		{
			// Make sure the path ends with a "\".
			if (0 != _tcsncat_s(pszDest, cchDest, TEXT("\\"), _TRUNCATE))
			{
				return NULL;
			}
		}
	}

	// Append the file name.
	if (0 != _tcsncat_s(pszDest, cchDest, pszName, _TRUNCATE))
	{
		return NULL;
	}

	// Append the extension.
	if (pszExt)
	{
		if (0 != _tcsncat_s(pszDest, cchDest, pszExt, _TRUNCATE))
		{
			return NULL;
		}
	}

	return pszDest;
}

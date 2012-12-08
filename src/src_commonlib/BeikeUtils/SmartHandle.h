// SmartHandle.h

#pragma once


// Class CSmartHandle works like Jeffrey Richter's EnsureCleanup class
// (Windows HANDLE wrapper which releases handle in destructor).
// The change: instead of release function pointer it uses template 
// class (using the same technique as described in 
// Modern C++ Design by Andrei Alexandrescu.
//
// Classes CCloseHandle, CCloseRegKey... are releasing policies.
// These classes are templates. Class CSmartHandle has second parameter
// ReleaseAlgorithm which is release policy template.
// This means, CSmartHandle template declaration contains
// nesting template. This code is compiled in Visual Stidio .NET 2003
// but not compiled in previous versions.
//
// Protected destructors prevent client code to release CSmartHandle
// using release pointer code (see Modern C++ Design, 
// 1.7  Destructors of Policy Classes).



// Release algorithms (release policies)

template <typename T>
struct CCloseHandle
{
    void Close(T handle)
    {
        cout << "Handle is released" << endl;         // for testing
        CloseHandle(handle);
    }

protected:
    ~CCloseHandle()
    {
    }
};

template <typename T>
struct CCloseRegKey
{
    void Close(T handle)
    {
        RegCloseKey(handle);
    }

protected:
    ~CCloseRegKey()
    {
    }
};


template <typename T>
struct SCloseLibrary
{
    void Close(T handle)
    {
        cout << "Library is released" << endl;        // for testing
        FreeLibrary(handle);
    }

protected:
    ~SCloseLibrary()
    {
    }
};


template <typename T>
struct CCloseViewOfFile
{
    void Close(T handle)
    {
        UnmapViewOfFile(handle);
    }

protected:
    ~CCloseViewOfFile()
    {
    }
};

template <typename T>
struct CCloseServiceHandle
{
	void Close(T handle)
	{
		CloseServiceHandle(handle);
	}

protected:
	~CCloseServiceHandle()
	{
	}
};


// Class CSmartHandle which implements release policy.
// Second template parameter is ReleaseAlgorithm which is template itself.

template <typename HandleType, 
          template <typename> class ReleaseAlgorithm, 
          HandleType NULL_VALUE = NULL>
class CSmartHandle : public ReleaseAlgorithm<HandleType>
{
public:
    CSmartHandle()
    {
        m_Handle = NULL_VALUE;
    }

    CSmartHandle(HandleType h)
    {
        m_Handle = h;
    }

    HandleType operator=(HandleType h) 
    { 
        CleanUp(); 
        m_Handle = h;
        return(*this);  
    }

    operator HandleType()
    {
        return m_Handle;
    }

    HandleType operator->()                 // for using as smart pointer
    {
        return m_Handle;
    }

    BOOL IsValid()
    {
        return m_Handle != NULL_VALUE;
    }

    ~CSmartHandle()
    {
        CleanUp();
    }

protected:
    void CleanUp()
    {
        if ( m_Handle != NULL_VALUE )
        {
            Close(m_Handle);
            m_Handle = NULL_VALUE;
        }
    }
    HandleType m_Handle;
};


// For using in the client code (definitions of standard Windows handles):
typedef CSmartHandle<HANDLE,  CCloseHandle>                         CAutoGeneralHandle;
typedef CSmartHandle<HKEY,    CCloseRegKey>                         CAutoRegKey;
typedef CSmartHandle<PVOID,   CCloseViewOfFile>                     CAutoViewOfFile;
typedef CSmartHandle<HMODULE, SCloseLibrary>                        CAutoLibrary;
typedef CSmartHandle<HANDLE,  CCloseHandle, INVALID_HANDLE_VALUE>   CAutoFile;
typedef CSmartHandle<SC_HANDLE, CCloseServiceHandle>                CAutoServiceHandle;



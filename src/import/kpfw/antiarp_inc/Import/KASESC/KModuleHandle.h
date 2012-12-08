/********************************************************************
* CreatedOn: 2007-8-24   17:59
* FileName:  KModuleHandle.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#ifndef __KAS_KMODULEHANDLE_H__
#define __KAS_KMODULEHANDLE_H__


template<bool t_bManaged>
class KModuleHandleT
{
public:

    KModuleHandleT( HMODULE hModule = NULL ) :
        m_hModule(hModule)
    {
    }

    KModuleHandleT( LPCTSTR pszPathFile )
    {
        if (pszPathFile != NULL)
            LoadLibrary(pszPathFile);
    }

    ~KModuleHandleT()
    {
        if (m_hModule != NULL && t_bManaged)
            FreeLibrary();
    }

public:

    HMODULE LoadLibrary(LPCTSTR pszPathFile)
    {
        m_hModule = ::LoadLibrary(pszPathFile);
        
        return m_hModule;
    }

    BOOL FreeLibrary()
    {
        BOOL bResult = FALSE;

        if (m_hModule != NULL)
            bResult = ::FreeLibrary(m_hModule);

        if (bResult)
            m_hModule = NULL;

        return bResult;
    }
    
    FARPROC GetProcAddress(LPCSTR pszProcName )
    {
        if (m_hModule == NULL)
            return NULL;

        return ::GetProcAddress(m_hModule, pszProcName);
    }

public:

    HMODULE Handle( void )           { return m_hModule   ; }
    void    Handle( HMODULE hModule) { m_hModule = hModule; };

protected:

    KModuleHandleT<t_bManaged>& operator = (const KModuleHandleT<t_bManaged>&);
    
    HMODULE m_hModule;
};

//////////////////////////////////////////////////////////////////////////

typedef KModuleHandleT<true>  KModule;
typedef KModuleHandleT<false> KModuleHandle;

//////////////////////////////////////////////////////////////////////////

#endif
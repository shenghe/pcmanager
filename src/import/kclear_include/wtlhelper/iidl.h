#pragma once

#include <objbase.h>

class CIidl
{
public:
    CIidl()
        : m_pidl(NULL)
    {
    }

    CIidl(LPITEMIDLIST pidl)
        : m_pidl(pidl)
    {
    }

    CIidl(CIidl& src)
        : m_pidl(src.Clone())
    {
    }

    CIidl(int nSpecialFolder)
    {
        ::SHGetSpecialFolderLocation(NULL, nSpecialFolder, &m_pidl);
    }

    virtual ~CIidl()
    {
        Free();
    }

    void Free()
    {
        if (m_pidl)
        {
            ::CoTaskMemFree(m_pidl);
            m_pidl = NULL;
        }
    }

    void Attach(LPITEMIDLIST pidl)
    {
        Free();

        m_pidl = pidl;
    }

    LPITEMIDLIST Detach()
    {
        LPITEMIDLIST pidl = m_pidl;

        m_pidl = NULL;

        return pidl;
    }

    LPITEMIDLIST Clone()
    {
        int nSize = GetSize();

        if (0 == nSize)
            return NULL;

        LPITEMIDLIST pidlNew = (LPITEMIDLIST)::CoTaskMemAlloc(nSize + sizeof(USHORT));
        if (pidlNew == NULL)
            return NULL;

        CopyMemory(pidlNew, m_pidl, nSize);

        *((USHORT *)(((LPBYTE)pidlNew) + nSize)) = 0;

        return pidlNew;
    }

    int GetSize()
    {
        if(NULL == m_pidl)
            return 0;

        int nSize = 0;
        LPCITEMIDLIST pidl = m_pidl;

        while (pidl->mkid.cb)
        {
            nSize += pidl->mkid.cb;
            pidl = (LPCITEMIDLIST)(((LPBYTE)pidl) + pidl->mkid.cb);
        }

        return nSize;
    }

    operator LPITEMIDLIST()
    {
        return m_pidl;
    }

    operator LPCITEMIDLIST()
    {
        return m_pidl;
    }

    operator LPITEMIDLIST*()
    {
        return &m_pidl;
    }

    operator LPCITEMIDLIST*()
    {
        return (LPCITEMIDLIST*)&m_pidl;
    }

    CIidl& operator=(LPITEMIDLIST pidl)
    {
        Attach(pidl);

        return *this;
    }

    CIidl& operator=(CIidl& src)
    {
        Attach(src.Clone());

        return *this;
    }

    BOOL IsValid()
    {
        return NULL != m_pidl;
    }

    IShellFolder* GetShellFolder()
    {
        if (!IsValid())
            return NULL;

        IShellFolder *piFolder = NULL;
        IShellFolder *piDesktop = NULL;

        HRESULT hRet = ::SHGetDesktopFolder(&piDesktop);
        if (FAILED(hRet))
            return NULL;

        piDesktop->BindToObject(
            m_pidl,
            NULL,
            IID_IShellFolder,
            (LPVOID *)&piFolder
            );

        piDesktop->Release();
        piDesktop = NULL;

        return piFolder;
    }

    LPITEMIDLIST Append(LPITEMIDLIST pidl)
    {
        if (!IsValid())
            return NULL;

        CIidl iidl;

        iidl.Attach(pidl);

        int nThisSize = GetSize();
        int nSize = nThisSize + iidl.GetSize();

        LPITEMIDLIST pidlNew = (LPITEMIDLIST)::CoTaskMemAlloc(nSize + sizeof(USHORT));
        if (pidlNew == NULL)
            return NULL;

        CopyMemory(((LPBYTE)pidlNew), m_pidl, nThisSize);
        CopyMemory(((LPBYTE)pidlNew) + nThisSize, pidl, iidl.GetSize());

        *((USHORT *)(((LPBYTE) pidlNew) + nSize)) = 0;

        iidl.Detach();

        return pidlNew;
    }

/*
    LPITEMIDLIST MergeItemID(LPCITEMIDLIST pidl,...)
    {
        va_list marker;
        int nSize = GetItemIDSize(pidl) + sizeof(pidl->mkid.cb);
        LPITEMIDLIST p;

        //	count size of pidl
        va_start(marker,pidl);
        while( p = va_arg(marker, LPITEMIDLIST) )
            nSize += GetItemIDSize(p);
        va_end(marker);
        //	allocate and merge pidls
        LPITEMIDLIST pidlNew = (LPITEMIDLIST)GetMalloc()->Alloc(nSize);
        if(pidlNew == NULL) return NULL;

        va_start(marker,pidl);
        CopyMemory(((LPBYTE)pidlNew), pidl, nSize = GetItemIDSize(pidl));
        while( p = va_arg(marker, LPITEMIDLIST) ) {
            CopyMemory(((LPBYTE)pidlNew) + nSize, p, GetItemIDSize(p));
            nSize += p->mkid.cb;
        }
        va_end(marker);
        *((USHORT *)(((LPBYTE) pidlNew) + nSize)) = 0;
        return pidlNew;
    }
*/

protected:

    LPITEMIDLIST m_pidl;
};

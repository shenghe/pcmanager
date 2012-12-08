// Add Duplicate for ATL CHandle

#pragma once

class CBkHandle
    : public CHandle
{
public:
    inline CBkHandle& Clone(CHandle& h) throw()
    {
        return Clone(h.m_h);
    }

    inline CBkHandle& Clone(HANDLE h) throw()
    {
        if(m_h)
            Close();

        BOOL bRet = ::DuplicateHandle(
            GetCurrentProcess(), 
            h, 
            GetCurrentProcess(),
            &m_h, 
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

        return *this;
    }

    inline CBkHandle& operator=(CHandle& h) throw()
    {
        return Clone(h);
    }
};
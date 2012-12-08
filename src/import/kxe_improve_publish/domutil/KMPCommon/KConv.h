/********************************************************************
* CreatedOn: 2006-2-21   9:27
* FileName:  KConv.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KConv_H_
#define __KConv_H_


//#include "../../KSCE/SCOMCommon/SCOMMallocObj.h"
//#include "../../KSCE/KSCECoreCommon/KException.h"


#ifdef WIN32
//
#include <atlbase.h>
#include "KChar.h"

#endif // WIN32




#ifndef KCONVAssert
#define KCONVAssert assert
#endif//KCONVAssert



#ifndef _KSCE_BEGIN
#define _KSCE_BEGIN namespace KSCE {
#define _KSCE_END   };
#endif // _KSCE_BEGIN








_KSCE_BEGIN








// 窄字符转换到宽字符
template<int t_nBufferLength = 128>
class KMByteToWCharEx
{
public:
    KMByteToWCharEx(const char* psz, int nCodePage = CP_ACP):
        m_psz(m_szBuffer)
    {
        Init(psz, nCodePage);
    }

    ~KMByteToWCharEx() throw()
    {
        if( m_psz != m_szBuffer )
        {
            delete[] m_psz;
        }
    }

    operator const wchar_t*() const
    {
        return( m_psz );
    }

    // 包含终结符'\0'在内的字符数目
    size_t CharCount()
    {
        if (m_psz == NULL)
            return 0;

        return m_nCharCount;
    }

private:
    void Init(const char* psz, int nCodePage)
    {
        if (psz == NULL)
        {
            m_psz = NULL;
            return;
        }

        // 检查字符串长度(包括结束符在内的字符数)
        // 如果超出缓冲区长度，则需要分配堆内存
        m_nCharCount = MultiByteToWideChar(
            nCodePage, 0,
            psz, -1,        // null terminator
            NULL, 0         // require buffer size including the null terminator
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);

        if(m_nCharCount > t_nBufferLength)
        {
            m_psz = new wchar_t[m_nCharCount];
            if( m_psz == NULL )
            {
                throw E_OUTOFMEMORY;
            }
        }

        m_nCharCount = MultiByteToWideChar(
            nCodePage, 0,
            psz, -1,
            m_psz, static_cast<int>(m_nCharCount)
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);
    }

public:
    wchar_t*   m_psz;
    wchar_t    m_szBuffer[t_nBufferLength];
    size_t     m_nCharCount;

private:
    KMByteToWCharEx( const KMByteToWCharEx& ) throw();
    KMByteToWCharEx& operator=( const KMByteToWCharEx& ) throw();
};


typedef KMByteToWCharEx<> KMByteToWChar;









// UTF16转换到UTF8
template<int t_nBufferLength = 128>
class KWCharToMByteEx
{
public:
    KWCharToMByteEx(const wchar_t* psz, int nCodePage = CP_ACP):
        m_psz(m_szBuffer)
    {
        Init(psz, nCodePage);
    }

    ~KWCharToMByteEx() throw()
    {
        if( m_psz != m_szBuffer )
        {
            delete[] m_psz;
        }
    }

    operator const char*() const
    {
        return( m_psz );
    }

    // 包含终结符'\0'在内的字符数目
    // 对于UTF8,是指的字节长度，即ASCII长度
    size_t CharCount()
    {
        if (m_psz == NULL)
            return 0;

        return m_nCharCount;
    }

private:
    void Init(const wchar_t* psz, int nCodePage)
    {
        if (psz == NULL)
        {
            m_psz = NULL;
            return;
        }

        // 检查字符串长度(包括结束符在内的字符数)
        // 如果超出缓冲区长度，则需要分配堆内存
        m_nCharCount = WideCharToMultiByte(
            nCodePage, 0,
            psz, -1,        // null terminator
            NULL, 0,        // require buffer size including the null terminator
            NULL, NULL
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);

        if(m_nCharCount > t_nBufferLength)
        {
            m_psz = new char[m_nCharCount];
            if( m_psz == NULL )
            {
                throw E_OUTOFMEMORY;
            }
        }

        m_nCharCount = WideCharToMultiByte(
            nCodePage, 0,
            psz, -1,
            m_psz, static_cast<int>(m_nCharCount),
            NULL, NULL
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);
    }

public:
    char*   m_psz;
    char    m_szBuffer[t_nBufferLength];
    size_t  m_nCharCount;

private:
    KWCharToMByteEx( const KWCharToMByteEx& ) throw();
    KWCharToMByteEx& operator=( const KWCharToMByteEx& ) throw();
};


typedef KWCharToMByteEx<> KWCharToMByte;






// UTF8转换到UTF16
template<int t_nBufferLength = 128>
class KUTF8ToUTF16Ex
{
public:
    KUTF8ToUTF16Ex(const char* psz):
        m_psz(m_szBuffer)
    {
        Init(psz);
    }

    ~KUTF8ToUTF16Ex() throw()
    {
        if( m_psz != m_szBuffer )
        {
            delete[] m_psz;
        }
    }

    operator const wchar_t*() const
    {
        return( m_psz );
    }

    // 包含终结符'\0'在内的字符数目
    size_t CharCount()
    {
        if (m_psz == NULL)
            return 0;

        return m_nCharCount;
    }

private:
    void Init(const char* psz)
    {
        if (psz == NULL)
        {
            m_psz = NULL;
            return;
        }

        // 检查字符串长度(包括结束符在内的字符数)
        // 如果超出缓冲区长度，则需要分配堆内存
        m_nCharCount = MultiByteToWideChar(
            CP_UTF8, 0,
            psz, -1,        // null terminator
            NULL, 0         // require buffer size including the null terminator
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);

        if(m_nCharCount > t_nBufferLength)
        {
            m_psz = new wchar_t[m_nCharCount];
            if( m_psz == NULL )
            {
                throw E_OUTOFMEMORY;
            }
        }

        m_nCharCount = MultiByteToWideChar(
            CP_UTF8, 0,
            psz, -1,
            m_psz, static_cast<int>(m_nCharCount)
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);
    }

public:
    wchar_t*   m_psz;
    wchar_t    m_szBuffer[t_nBufferLength];
    size_t     m_nCharCount;

private:
    KUTF8ToUTF16Ex( const KUTF8ToUTF16Ex& ) throw();
    KUTF8ToUTF16Ex& operator=( const KUTF8ToUTF16Ex& ) throw();
};


typedef KUTF8ToUTF16Ex<> KU8ToU16;









// UTF16转换到UTF8
template<int t_nBufferLength = 128>
class KUTF16ToUTF8Ex
{
public:
    KUTF16ToUTF8Ex(const wchar_t* psz):
        m_psz(m_szBuffer)
    {
        Init(psz);
    }

    ~KUTF16ToUTF8Ex() throw()
    {
        if( m_psz != m_szBuffer )
        {
            delete[] m_psz;
        }
    }

    operator const char*() const
    {
        return( m_psz );
    }

    // 包含终结符'\0'在内的字符数目
    // 对于UTF8,是指的字节长度，即ASCII长度
    size_t CharCount()
    {
        if (m_psz == NULL)
            return 0;

        return m_nCharCount;
    }

private:
    void Init(const wchar_t* psz)
    {
        if (psz == NULL)
        {
            m_psz = NULL;
            return;
        }

        // 检查字符串长度(包括结束符在内的字符数)
        // 如果超出缓冲区长度，则需要分配堆内存
        m_nCharCount = WideCharToMultiByte(
            CP_UTF8, 0,
            psz, -1,        // null terminator
            NULL, 0,        // require buffer size including the null terminator
            NULL, NULL
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);

        if(m_nCharCount > t_nBufferLength)
        {
            m_psz = new char[m_nCharCount];
            if( m_psz == NULL )
            {
                throw E_OUTOFMEMORY;
            }
        }

        m_nCharCount = WideCharToMultiByte(
            CP_UTF8, 0,
            psz, -1,
            m_psz, static_cast<int>(m_nCharCount),
            NULL, NULL
        );
        assert(m_nCharCount > 0);
        KCONVAssert(m_nCharCount > 0);
    }

public:
    char*   m_psz;
    char    m_szBuffer[t_nBufferLength];
    size_t  m_nCharCount;

private:
    KUTF16ToUTF8Ex( const KUTF16ToUTF8Ex& ) throw();
    KUTF16ToUTF8Ex& operator=( const KUTF16ToUTF8Ex& ) throw();
};


typedef KUTF16ToUTF8Ex<> KU16ToU8;



_KSCE_END







#endif//__KConv_H_

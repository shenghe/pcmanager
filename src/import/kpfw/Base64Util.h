/********************************************************************
* CreatedOn: 2006-6-18   10:57
* FileName:  Base64Util.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __Base64Util_H_
#define __Base64Util_H_

#include <atlenc.h>

// Base64编码
template<int t_nBufferLength = 128>
class KBase64EncodeEx
{
public:
    KBase64EncodeEx(const char* psz):
        m_psz(m_szBuffer)
    {
        Init(psz);
    }

    ~KBase64EncodeEx() throw()
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
    size_t BufSize()
    {
        if (m_psz == NULL)
            return 0;

        return m_nBufSize;
    }

private:
    void Init(const char* psz)
    {
        int bRetCode = false;
        int nInLen   = 0;
        int nRetSize = 0;

        if (psz == NULL)
        {
            m_psz = NULL;
            return;
        }

        // 检查字符串长度(包括结束符在内的字符数)
        // 如果超出缓冲区长度，则需要分配堆内存
        nInLen = static_cast<int>(strlen(psz));
        m_nBufSize = Base64EncodeGetRequiredLength(nInLen, ATL_BASE64_FLAG_NOCRLF);
        ++m_nBufSize; // for '\0'
        nRetSize   = (int)m_nBufSize;
        assert(m_nBufSize > 0);

        if(m_nBufSize > t_nBufferLength)
        {
            m_psz = new char[m_nBufSize];
            if( m_psz == NULL )
            {
                throw E_OUTOFMEMORY;
            }
        }
        else
        {
            m_psz = m_szBuffer;
        }

        memset(m_psz, 0, nRetSize);
        bRetCode = Base64Encode((const BYTE*)psz, nInLen, m_psz, &nRetSize, ATL_BASE64_FLAG_NOCRLF);
        assert(bRetCode != false);
    }

public:
    char*   m_psz;
    char    m_szBuffer[t_nBufferLength];
    size_t  m_nBufSize;

private:
    KBase64EncodeEx( const KBase64EncodeEx& ) throw();
    KBase64EncodeEx& operator=( const KBase64EncodeEx& ) throw();
};


typedef KBase64EncodeEx<> KBase64Encode;









// Base64解码
template<int t_nBufferLength = 128>
class KBase64DecodeEx
{
public:
    KBase64DecodeEx(const char* psz):
        m_psz(m_szBuffer)
    {
        Init(psz);
    }

    ~KBase64DecodeEx() throw()
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
    size_t BufSize()
    {
        if (m_psz == NULL)
            return 0;

        return m_nBufSize;
    }

private:
    void Init(const char* psz)
    {
        int bRetcode = false;
        int nInLen   = 0;
        int nRetSize = 0;

        if (psz == NULL)
        {
            m_psz = NULL;
            return;
        }

        // 检查字符串长度(包括结束符在内的字符数)
        // 如果超出缓冲区长度，则需要分配堆内存
        nInLen = static_cast<int>(strlen(psz));
        m_nBufSize = Base64DecodeGetRequiredLength(nInLen);
        ++m_nBufSize; // for '\0'
        nRetSize   = (int)m_nBufSize;
        assert(m_nBufSize > 0);

        if(m_nBufSize > t_nBufferLength)
        {
            m_psz = new char[m_nBufSize];
            if( m_psz == NULL )
            {
                throw E_OUTOFMEMORY;
            }
        }
        else
        {
            m_psz = m_szBuffer;
        }

        memset(m_psz, 0, nRetSize);
        bRetcode = Base64Decode(psz, nInLen, (BYTE*)m_psz, &nRetSize);
        assert(bRetcode > 0);
    }

public:
    char*   m_psz;
    char    m_szBuffer[t_nBufferLength];
    size_t  m_nBufSize;

private:
    KBase64DecodeEx( const KBase64DecodeEx& ) throw();
    KBase64DecodeEx& operator=( const KBase64DecodeEx& ) throw();
};


typedef KBase64DecodeEx<> KBase64Decode;




#endif//__Base64Util_H_
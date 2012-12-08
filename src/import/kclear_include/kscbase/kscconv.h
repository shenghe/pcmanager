/********************************************************************
	created:	2010/03/08
	created:	8:3:2010   9:28
	filename: 	kscconv.h
	author:		Jiang Fengbing
	
	purpose:	±àÂë×ª»»·â×°
*********************************************************************/

#ifndef KSCCONV_INC_
#define KSCCONV_INC_

//////////////////////////////////////////////////////////////////////////

#include "kscbase.h"

//////////////////////////////////////////////////////////////////////////

// UTF8 To UTF16
template<int t_nBufferLength = 260>
class KUtf8ToUtf16 
{
public:
    KUtf8ToUtf16(const char* psz) : m_psz(m_szBuffer) 
	{
        Initialize(psz);
    }

    ~KUtf8ToUtf16() throw() 
	{
        if (m_psz != m_szBuffer) 
		{
            delete[] m_psz;
        }
    }

    operator const wchar_t*() const 
	{
        return m_psz;
    }

    size_t CharCount() 
	{
        if (NULL == m_psz)
            return 0;

        return m_nCharCount;
    }

protected:
    void Initialize(const char* psz) 
	{
        if (NULL == psz) 
		{
            m_psz = NULL;
            return;
        }

        m_nCharCount = MultiByteToWideChar(CP_UTF8, 0, psz, -1, NULL, 0);
        if (m_nCharCount > t_nBufferLength) 
		{
            m_psz = new wchar_t[m_nCharCount];
            if (NULL == m_psz) 
			{
                throw E_OUTOFMEMORY;
            }
        }

        m_nCharCount = MultiByteToWideChar(CP_UTF8, 0, psz, -1, m_psz, 
				static_cast<int>(m_nCharCount));
    }

    wchar_t*	m_psz;
    wchar_t		m_szBuffer[t_nBufferLength];
    size_t		m_nCharCount;
};

// UTF16 To UTF8
template<int t_nBufferLength = 260>
class KUtf16ToUtf8 
{
public:
    KUtf16ToUtf8(const wchar_t* psz) : m_psz(m_szBuffer) 
	{
        Initialize(psz);
    }

    ~KUtf16ToUtf8() throw() 
	{
        if (m_psz != m_szBuffer) 
		{
            delete[] m_psz;
        }
    }

    operator const char*() const 
	{
        return m_psz;
    }

    size_t CharCount() 
	{
        if (NULL == m_psz)
            return 0;

        return m_nCharCount;
    }

protected:
    void Initialize(const wchar_t* psz) 
	{
        if (NULL == psz) 
		{
            m_psz = NULL;
            return;
        }

        m_nCharCount = WideCharToMultiByte(CP_UTF8, 0, psz, -1, NULL, 0, NULL, NULL);
        if (m_nCharCount > t_nBufferLength) 
		{
            m_psz = new char[m_nCharCount];
            if (NULL == m_psz) 
			{
                throw E_OUTOFMEMORY;
            }
        }

        m_nCharCount = WideCharToMultiByte(CP_UTF8, 0, psz, -1, m_psz, 
				static_cast<int>(m_nCharCount), NULL, NULL);
    }

    char*	m_psz;
    char	m_szBuffer[t_nBufferLength];
    size_t	m_nCharCount;
};

// UTF16 To ANSI
template<int t_nBufferLength = 260>
class KUtf16ToAnsi
{
public:
	KUtf16ToAnsi(const wchar_t* psz) : m_psz(m_szBuffer) 
	{
		Initialize(psz);
	}

	~KUtf16ToAnsi() throw() 
	{
		if (m_psz != m_szBuffer) 
		{
			delete[] m_psz;
		}
	}

	operator const char*() const 
	{
		return m_psz;
	}

	size_t CharCount() 
	{
		if (NULL == m_psz)
			return 0;

		return m_nCharCount;
	}

protected:
	void Initialize(const wchar_t* psz) 
	{
		if (NULL == psz) 
		{
			m_psz = NULL;
			return;
		}

		m_nCharCount = WideCharToMultiByte(CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL);
		if (m_nCharCount > t_nBufferLength) 
		{
			m_psz = new char[m_nCharCount];
			if (NULL == m_psz) 
			{
				throw E_OUTOFMEMORY;
			}
		}

		m_nCharCount = WideCharToMultiByte(CP_ACP, 0, psz, -1, m_psz, 
				static_cast<int>(m_nCharCount), NULL, NULL);
	}

	char*	m_psz;
	char	m_szBuffer[t_nBufferLength];
	size_t	m_nCharCount;
};

// ANSI To UTF16
template<int t_nBufferLength = 260>
class KAnsiToUtf16 
{
public:
	KAnsiToUtf16(const char* psz) : m_psz(m_szBuffer) 
	{
		Initialize(psz);
	}

	~KAnsiToUtf16() throw() 
	{
		if (m_psz != m_szBuffer) 
		{
			delete[] m_psz;
		}
	}

	operator const wchar_t*() const 
	{
		return m_psz;
	}

	size_t CharCount() 
	{
		if (NULL == m_psz)
			return 0;

		return m_nCharCount;
	}

protected:
	void Initialize(const char* psz) 
	{
		if (NULL == psz) 
		{
			m_psz = NULL;
			return;
		}

		m_nCharCount = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
		if (m_nCharCount > t_nBufferLength) 
		{
			m_psz = new wchar_t[m_nCharCount];
			if (NULL == m_psz) 
			{
				throw E_OUTOFMEMORY;
			}
		}

		m_nCharCount = MultiByteToWideChar(CP_ACP, 0, psz, -1, m_psz, static_cast<int>(m_nCharCount));
	}

	wchar_t*	m_psz;
	wchar_t		m_szBuffer[t_nBufferLength];
	size_t		m_nCharCount;
};

//////////////////////////////////////////////////////////////////////////

#define KUTF8_To_UTF16(a) (LPCWSTR)KUtf8ToUtf16<>(a)
#define KUTF16_To_UTF8(a) (LPCSTR)KUtf16ToUtf8<>(a)

#define KUTF16_To_ANSI(a) (LPCSTR)KUtf16ToAnsi<>(a)
#define KANSI_TO_UTF16(a)	(LPCWSTR)KAnsiToUtf16<>(a)

//////////////////////////////////////////////////////////////////////////

std::wstring AnsiToUnicode(const std::string& strAnsi);
std::string UnicodeToAnsi(const std::wstring& strUnicode);
std::string UnicodeToUtf8(const std::wstring& strUnicode);
std::wstring Utf8ToUnicode(const std::string& strUtf8);
std::string Utf8ToAnsi(const std::string& strUtf8);
std::string AnsiToUtf8(const std::string& strAnsi);

//////////////////////////////////////////////////////////////////////////

#endif	// KSCCONV_INC_

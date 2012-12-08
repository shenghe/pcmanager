#pragma once

#include <atlstr.h>

inline int _szttoi(LPCTSTR _Str)
{
    return ::StrToInt(_Str);
}

inline  int _sztoupper(int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    else
        return c;
}

inline  int _sztolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    else
        return c;
}

inline  int __cdecl _vsnprintf(char *string, size_t count, const char *format, va_list ap)
{
    int nRet = -1;
    CStringA strTemp;

    strTemp.FormatV(format, ap);

    if (strTemp.GetLength() >= (int)count)
        strTemp.Truncate((int)count - 1);
    else
        nRet = strTemp.GetLength();

    memcpy(string, (LPCSTR)strTemp, nRet);
}

inline  int __cdecl _vsnwprintf(wchar_t *string, size_t count, const wchar_t *format, va_list ap)
{
    int nRet = -1;
    CStringW strTemp;

    strTemp.FormatV(format, ap);

    if (strTemp.GetLength() >= (int)count)
        strTemp.Truncate((int)count - 1);
    else
        nRet = strTemp.GetLength();

    memcpy(string, (LPCWSTR)strTemp, nRet * sizeof(wchar_t));

    return nRet;
}

#ifndef _DEBUG

inline  int __cdecl _putts(const _TCHAR *string)
{
    return 0;
}

#endif

template < typename T >
class CSZCharTraits
{
};

template <>
class CSZCharTraits<char>
{
public:
    static char CharToUpper(char x)
    {
        return (char)_sztoupper(x);
    }

    static char CharToLower(char x)
    {
        return (char)_sztolower(x);
    }
};

template <>
class CSZCharTraits<wchar_t>
{
public:
    static wchar_t CharToUpper(wchar_t x)
    {
        return (wchar_t)_sztoupper(x);
    }

    static wchar_t CharToLower(wchar_t x)
    {
        return (wchar_t)_sztolower(x);
    }
};

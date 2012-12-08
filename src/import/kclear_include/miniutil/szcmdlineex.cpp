/**
* @file    szcmdlineex.cpp
* @brief   ...
* @author  zhangrui
* @date    2009-03-09  14:39
*/

#include "stdafx.h"
#include "szcmdlineex.h"

#include <assert.h>

BOOL CSZCommandLineEx::VerifyCommandLine(CString* pstrErrorMessage)
{
    if (pstrErrorMessage)
        pstrErrorMessage->Empty();

    POSITION pos = m_usageMap.GetHeadPosition();
    while (pos)
    {
        CParamUsageMap::CPair* pPair = m_usageMap.GetNext(pos);
        if (!pPair)
            continue;

        if (!VerifyParam(pPair->m_key, pPair->m_value, pstrErrorMessage))
            return FALSE;
    }

    return TRUE;
}

void CSZCommandLineEx::SetSubCommandUsage(LPCWSTR lpszSubCommandName, LPCWSTR lpszUsage)
{
    assert(lpszSubCommandName && *lpszSubCommandName);
    m_subCommandMap.SetAt(lpszSubCommandName, lpszUsage);
}

void CSZCommandLineEx::SetParamUsage(LPCTSTR lpszParamName, LPCTSTR lpszParamValueName, LPCTSTR lpszUsage)
{
    assert(lpszParamName && *lpszParamName);
    CParamUsage& usage          = GetParamUsage(lpszParamName);
    usage.m_strParamValueName   = lpszParamValueName;
    usage.m_strUsage            = lpszUsage;
}

void CSZCommandLineEx::SetParamMustBeExist(LPCTSTR lpszParamName)
{
    assert(lpszParamName && *lpszParamName);
    CParamUsage& usage = GetParamUsage(lpszParamName);
    usage.m_bMustExist = TRUE;
}

void CSZCommandLineEx::SetParamValueMustBeExist(LPCTSTR lpszParamName)
{
    assert(lpszParamName && *lpszParamName);
    CParamUsage& usage      = GetParamUsage(lpszParamName);
    usage.m_bValueMustExist = TRUE;
}

void CSZCommandLineEx::SetParamValueMustBeInt(LPCTSTR lpszParamName)
{
    assert(lpszParamName && *lpszParamName);
    CParamUsage& usage      = GetParamUsage(lpszParamName);
    usage.m_bValueMustBeInt = TRUE;
}

void CSZCommandLineEx::RemoveAllParamUsages()
{
    m_usageMap.RemoveAll();
    m_subCommandMap.RemoveAll();
}


CString CSZCommandLineEx::FormatEchoUsage(LPCWSTR lpCommandName, LPCWSTR lpSummary)
{
    CString strEchoUsage  = lpSummary;

    CString strSubCommand           = FormatSubCommand(lpCommandName);
    CString strCommandPresentation  = FormatCommandPresentation(lpCommandName);
    CString strCommandUsage         = FormatCommandUsage();

    strEchoUsage.Append(_T("\n\n"));
    
    if (!strSubCommand.IsEmpty())
    {
        strEchoUsage.Append(strSubCommand);
        if (!strCommandPresentation.IsEmpty())
        {
            strEchoUsage.Append(_T("\n\n"));
            strEchoUsage.Append(_T("    = Or =\n"));
        }
        strEchoUsage.Append(_T("\n"));
    }

    if (!strCommandPresentation.IsEmpty())
    {
        strEchoUsage.Append(strCommandPresentation);
        strEchoUsage.Append(_T("\n\n"));
        strEchoUsage.Append(strCommandUsage);
        strEchoUsage.Append(_T("\n\n"));
    }

    return strEchoUsage;
}

CString CSZCommandLineEx::FormatSubCommand(LPCWSTR lpCommandName)
{
    POSITION pos = m_subCommandMap.GetHeadPosition();
    if (!pos)
        return _T("");

    CSubCommandMap::CPair* pPair = m_subCommandMap.GetNext(pos);
    if (!pPair)
        return _T("");

    CString strSubCommand = lpCommandName;
    strSubCommand.Append(_T(" "));
    strSubCommand.Append(pPair->m_key);

    while(pos)
    {
        CSubCommandMap::CPair* pPair = m_subCommandMap.GetNext(pos);
        if (!pPair)
            continue;

        assert(!pPair->m_key.IsEmpty());

        strSubCommand.Append(_T(" | "));
        strSubCommand.Append(pPair->m_key);
    }

    return strSubCommand;
}

CString CSZCommandLineEx::FormatCommandPresentation(LPCWSTR lpCommandName)
{
    POSITION pos = m_usageMap.GetHeadPosition();
    if (!pos)
        return _T("");

    CParamUsageMap::CPair* pPair = m_usageMap.GetAt(pos);
    if (!pPair)
        return _T("");

    CString strCommandPresentation = lpCommandName;
    while (pos)
    {
        pPair = m_usageMap.GetNext(pos);
        if (!pPair)
            continue;

        CString strParamPresentation = FormatParamPresentation(pPair->m_key, pPair->m_value);
        strCommandPresentation.Append(_T(" "));
        strCommandPresentation.Append(strParamPresentation);
    }

    return strCommandPresentation;
}


CString CSZCommandLineEx::FormatCommandUsage(int nParamNameWidth, int nValueNameWidth)
{
    CString strCommandUsage;

    int nMaxParamNameLen = 0;
    int nMaxValueNameLen = 0;
    GetMaxLengthOfParamNameAndValue(&nMaxParamNameLen, &nMaxValueNameLen);

    if (-1 == nParamNameWidth)
        nParamNameWidth = nMaxParamNameLen;

    if (-1 == nValueNameWidth)
        nValueNameWidth = nMaxValueNameLen;


    nParamNameWidth = max(1,  nParamNameWidth);
    nParamNameWidth = min(20, nParamNameWidth);

    nValueNameWidth = max(1,  nValueNameWidth);
    nValueNameWidth = min(20, nValueNameWidth);



    POSITION pos = m_usageMap.GetHeadPosition();
    while (pos)
    {
        CParamUsageMap::CPair* pPair = m_usageMap.GetNext(pos);
        if (!pPair)
            continue;

        CString strParamPresentation = FormatParamUsage(
            pPair->m_key,
            pPair->m_value,
            nParamNameWidth,
            nValueNameWidth);
        strCommandUsage.Append(strParamPresentation);
        strCommandUsage.Append(_T("\n"));
    }

    return strCommandUsage;
}




CSZCommandLineEx::CParamUsage& CSZCommandLineEx::GetParamUsage(LPCTSTR lpszParamName)
{
    assert(lpszParamName && *lpszParamName);
    CParamUsageMap::CPair* pPair = m_usageMap.Lookup(lpszParamName);
    if (pPair)
        return pPair->m_value;

    CParamUsage newUsage;
    POSITION pos = m_usageMap.SetAt(lpszParamName, newUsage);
    assert(pos);

    pPair = m_usageMap.GetAt(pos);
    assert(pPair);
    return pPair->m_value;
}

void CSZCommandLineEx::GetMaxLengthOfParamNameAndValue(int* pnParamNameWidth, int* pnValueNameWidth)
{
    int nMaxParamNameLen = 0;
    int nMaxValueNameLen = 0;

    POSITION pos = m_usageMap.GetHeadPosition();
    while (pos)
    {
        CParamUsageMap::CPair* pPair = m_usageMap.GetNext(pos);
        if (!pPair)
            continue;

        nMaxParamNameLen = max(nMaxParamNameLen, pPair->m_key.GetLength());
        nMaxValueNameLen = max(nMaxValueNameLen, pPair->m_value.m_strParamValueName.GetLength());
    }

    if (pnParamNameWidth)
        *pnParamNameWidth = nMaxParamNameLen;

    if (pnValueNameWidth)
        *pnValueNameWidth = nMaxValueNameLen;
}

BOOL CSZCommandLineEx::VerifyParam(
    LPCTSTR         lpszParamName,
    CParamUsage&    usage,
    CString*        pstrErrorMessage)
{
    assert(lpszParamName && *lpszParamName);


    if (!usage.m_bMustExist         &&
        !usage.m_bValueMustExist    &&
        !usage.m_bValueMustBeInt)
        return TRUE;

    
    CString strParamValue;
    if (!m_mapParams.Lookup(lpszParamName, strParamValue))
    {
        if (usage.m_bMustExist)
        {
            if (pstrErrorMessage)
            {
                pstrErrorMessage->Format(_T("Parameter \"%s\" must be set!"), lpszParamName);
            }
            return FALSE;
        }
    }


    if (usage.m_bValueMustExist)
    {
        if (strParamValue.IsEmpty())
        {
            if (pstrErrorMessage)
            {
                pstrErrorMessage->Format(_T("Parameter \"%s\" must not be empty!"), lpszParamName);
            }
            return FALSE;
        }
    }


    if (usage.m_bValueMustBeInt)
    {
        int nValue = 0;
        if (!StrToIntEx(strParamValue, STIF_DEFAULT, &nValue))
        {
            if (pstrErrorMessage)
            {
                pstrErrorMessage->Format(_T("Parameter \"%s\" must be an integer!"), lpszParamName);
            }
            return FALSE;
        }
    }


    return TRUE;
}

CString CSZCommandLineEx::FormatParamPresentation(LPCTSTR lpszParamName, CParamUsage& usage)
{
    CString strParamPresentation;
    CString strParamName = lpszParamName;
    CString strValue;
    CString strWrappedValue;



    // value
    if (!usage.m_strParamValueName.IsEmpty())
    {
        strValue.Format(_T(":<%s>"), usage.m_strParamValueName);   // named strparam
    }
    else if (usage.m_bValueMustBeInt)
    {
        strValue.Format(_T(":<n>"));   // interger param
    }
    else if (usage.m_bValueMustExist)
    {
        strValue.Format(_T(":<s>"));   // string param
    }


    // check if need wrap value with []
    if (strValue.IsEmpty())
    {
        strWrappedValue.Empty();
    }
    else if (usage.m_bValueMustExist)
    {
        strWrappedValue.Format(_T("%s"), strValue);
    }
    else
    {
        strWrappedValue.Format(_T("[%s]"), strValue);
    }


    // param name + value
    if (usage.m_bMustExist)
    {
        strParamPresentation.Format(_T("-%s%s"), strParamName, strWrappedValue);
    }
    else
    {
        strParamPresentation.Format(_T("[-%s%s]"), strParamName, strWrappedValue);
    }


    return strParamPresentation;
}

CString CSZCommandLineEx::FormatParamUsage(
    LPCTSTR         lpszParamName,
    CParamUsage&    usage, 
    int             nParamNameWidth,
    int             nValueNameWidth)
{
    assert(0 <= nParamNameWidth && nParamNameWidth <= 80);
    assert(0 <= nValueNameWidth && nValueNameWidth <= 80);


    CString strParamUsage;

    if (nValueNameWidth)
        nValueNameWidth += 3;   // for :<>

    // indent
    int nTruncateLength = 4;
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T('-'));

    // param name
    nTruncateLength += nParamNameWidth;
    strParamUsage.Append(lpszParamName);
    for (int i = strParamUsage.GetLength(); i <= nTruncateLength; ++i)
    {
        strParamUsage.AppendChar(_T(' '));
    }
    strParamUsage.Truncate(nTruncateLength);


    // space
    nTruncateLength += 2;
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T(' '));


    // param value
    nTruncateLength += nValueNameWidth;
    if (!usage.m_strParamValueName.IsEmpty())
        strParamUsage.AppendFormat(_T(":<%s>"), usage.m_strParamValueName);
    for (int i = strParamUsage.GetLength(); i <= nTruncateLength; ++i)
    {
        strParamUsage.AppendChar(_T(' '));
    }
    strParamUsage.Truncate(nTruncateLength);


    // space
    nTruncateLength += 4;
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T(' '));
    strParamUsage.AppendChar(_T(' '));


    // param usage
    strParamUsage.Append(usage.m_strUsage);
    return strParamUsage;
}
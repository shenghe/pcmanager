/**
* @file    szcmdlineex.h
* @brief   ...
* @author  zhangrui
* @date    2009-03-09  14:39
*/

#ifndef SZCMDLINEEX_H
#define SZCMDLINEEX_H

#include <atlcoll.h>
#include "szcmdline.h"

class CSZCommandLineEx: public CSZCommandLine
{
public:

    BOOL    VerifyCommandLine(CString* pstrErrorMessage = NULL);


    void    SetSubCommandUsage(LPCWSTR lpszSubCommandName, LPCWSTR lpszUsage = NULL);

    void    SetParamUsage(LPCTSTR lpszParamName, LPCTSTR lpszParamValueName, LPCTSTR lpszUsage);
    void    SetParamMustBeExist(LPCTSTR lpszParamName);
    void    SetParamValueMustBeExist(LPCTSTR lpszParamName);
    void    SetParamValueMustBeInt(LPCTSTR lpszParamName);

    void    RemoveAllParamUsages();


    CString FormatEchoUsage(LPCWSTR lpCommandName, LPCWSTR lpSummary);
    CString FormatSubCommand(LPCWSTR lpCommandName);
    CString FormatCommandPresentation(LPCWSTR lpCommandName);
    CString FormatCommandUsage(int nParamNameWidth = -1, int nValueNameWidth = -1);

protected:

    class CParamUsage
    {
    public:
        CParamUsage():
            m_bMustExist(FALSE),
            m_bValueMustExist(FALSE),
            m_bValueMustBeInt(FALSE)
        {
        }

        BOOL    m_bMustExist;
        BOOL    m_bValueMustExist;
        BOOL    m_bValueMustBeInt;
        CString m_strParamValueName;
        CString m_strUsage;
    };

    typedef CRBMap<
        CString,
        CParamUsage,
        CStringElementTraitsI<
            CString,
            CSZCharTraits<TCHAR>
        >
    > CParamUsageMap;

    typedef CRBMap<
        CString,
        CString,
        CStringElementTraitsI<
        CString,
        CSZCharTraits<TCHAR>
        >
    > CSubCommandMap;



    CParamUsage& GetParamUsage(LPCTSTR lpszParamName);

    void    GetMaxLengthOfParamNameAndValue(int* pnParamNameWidth, int* pnValueNameWidth);

    BOOL    VerifyParam(LPCTSTR lpszParamName, CParamUsage& usage, CString* pstrErrorMessage = NULL);

    CString FormatParamPresentation(LPCTSTR lpszParamName, CParamUsage& usage);

    CString FormatParamUsage(LPCTSTR lpszParamName, CParamUsage& usage, int nParamNameWidth, int nValueNameWidth);




    CSubCommandMap  m_subCommandMap;
    CParamUsageMap  m_usageMap;
};

#endif//SZCMDLINEEX_H
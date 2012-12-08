#include "stdafx.h"
#include "szcmdline.h"
#include "szstr.h"
#include <shellapi.h>

CSZCommandLine::CSZCommandLine(LPCTSTR lpszCmdLine)
{
    Analyze(lpszCmdLine);
}

CSZCommandLine::~CSZCommandLine()
{

}

CString CSZCommandLine::FindSubCommand(LPCTSTR lpszCmdLine, LPCWSTR* lppszSubCmdLine)
{
    if (lppszSubCmdLine)
        *lppszSubCmdLine = lpszCmdLine;

    if (!lpszCmdLine)
        return _T("");

    // trim white space
    while (*lpszCmdLine)
    {
        if (!IsWhiteSpace(*lpszCmdLine))
            break;

        ++lpszCmdLine;
    }


    // find first word
    LPCWSTR lpNextWord = lpszCmdLine;
    while (*lpNextWord)
    {
        if (!IsAlpha(*lpNextWord))
            break;

        ++lpNextWord;
    }


    // no word
    if (lpNextWord == lpszCmdLine)
        return _T("");


    // get subcommand
    CString strSubCommand;
    strSubCommand.SetString(lpszCmdLine, int(lpNextWord - lpszCmdLine));



    // trim white space
    while (*lpNextWord)
    {
        if (!IsWhiteSpace(*lpNextWord))
            break;

        ++lpNextWord;
    }


    // return command line for sub command
    if (lppszSubCmdLine)
        *lppszSubCmdLine = lpNextWord;


    return strSubCommand;
}

CString CSZCommandLine::FindSubCommand(int argc, TCHAR* argv[])
{
    if (argc < 2)
        return _T("");

    LPCWSTR lpSubCommandEnd = NULL;    
    CString strSubCommand = FindSubCommand(argv[1], &lpSubCommandEnd);


    // confirm no more content except white space
    if (*lpSubCommandEnd)
        return _T("");


    return strSubCommand;
}

BOOL CSZCommandLine::Analyze(LPCTSTR lpszCmdLine)
{
    BOOL    bResult = FALSE;
    int     nParamNamePos  = 0;
    int     nParamValuePos = 0;
    int     nSubCommandPos = 0;
    CString strParamName;
    CString strSubCommand;

    BOOL    bInQuotation   = FALSE;

    EM_CMDLINE_STATUS nStatus = em_Cmd_New_Arg;

    m_mapParams.RemoveAll();

    if (!lpszCmdLine)
        goto Exit0;

    for (int nPos = 0; 0 == nPos || lpszCmdLine[nPos - 1]; ++nPos)
    {
        TCHAR ch = lpszCmdLine[nPos];
        switch (nStatus)
        {
        case em_Cmd_New_Arg:
            bInQuotation = FALSE;
            // no break;

        case em_Cmd_White_Space:

            if (IsWhiteSpace(ch))
            {
                nStatus = em_Cmd_White_Space;
            }
            else if (IsArgNamePrefix(ch))
            {
                nStatus = em_Cmd_Arg_Name_Prefix;
            }
            else if (IsAlpha(ch))
            {   // skip sub command
                nSubCommandPos  = nPos;
                nStatus         = em_Cmd_Sub_Command;
            }
            else if (IsQuotation(ch))
            {
                bInQuotation = TRUE;
                nStatus = em_Cmd_White_Space;
            }
            else
            {
                goto Exit0;
            }

            break;

        case em_Cmd_Sub_Command:

            if (IsWhiteSpace(ch))
            {
                strSubCommand.SetString(lpszCmdLine + nSubCommandPos, nPos - nSubCommandPos);
                AppendSubCommand(strSubCommand);
                nStatus = em_Cmd_New_Arg;
            }
            else if (IsAlpha(ch))
            {   // skip sub command
                nStatus = em_Cmd_Sub_Command;
            }
            else if (IsQuotation(ch))
            {
                strSubCommand.SetString(lpszCmdLine + nSubCommandPos, nPos - nSubCommandPos);
                AppendSubCommand(strSubCommand);
                nStatus = em_Cmd_New_Arg;
            }
            else
            {
                goto Exit0;
            }

            break;

        case em_Cmd_Arg_Name_Prefix:

            if (IsWhiteSpace(ch))
            {
                goto Exit0;
            }
            else if (IsArgNamePrefix(ch))
            {   // 连续的前缀
                nStatus = em_Cmd_Arg_Name_Prefix;
            }
            else
            {
                nParamNamePos = nPos;
                nStatus = em_Cmd_Arg_Name;
            }

            break;

        case em_Cmd_Arg_Name:

            if (IsWhiteSpace(ch))
            {
                strParamName.SetString(lpszCmdLine + nParamNamePos, nPos - nParamNamePos);
                SetParam(strParamName, _T(""));
                nStatus = em_Cmd_New_Arg;
            }
            else if (IsArgValuePrefix(ch))
            {
                strParamName.SetString(lpszCmdLine + nParamNamePos, nPos - nParamNamePos);
                nStatus = em_Cmd_Arg_Value_Prefix;
            }
            else if (IsQuotation(ch))
            {
                strParamName.SetString(lpszCmdLine + nParamNamePos, nPos - nParamNamePos);
                SetParam(strParamName, _T(""));
                nStatus = em_Cmd_New_Arg;
            }
            else
            {
                nStatus = em_Cmd_Arg_Name;
            }

            break;

        case em_Cmd_Arg_Value_Prefix:

            if (IsWhiteSpace(ch))
            {
                if (bInQuotation)
                {   // treat quoted white space as arg-value
                    nParamValuePos  = nPos;
                    nStatus         = em_Cmd_Arg_Value;
                }
                else
                {
                    SetParam(strParamName, _T(""));
                    nStatus         = em_Cmd_New_Arg;
                }

            }
            else if (IsQuotation(ch))
            {
                nParamValuePos  = nPos + 1;
                bInQuotation    = TRUE;
                nStatus         = em_Cmd_Arg_Value_Prefix;
            }
            else
            {
                nParamValuePos  = nPos;
                nStatus         = em_Cmd_Arg_Value;
            }

            break;

        case em_Cmd_Arg_Value:

            if (IsWhiteSpace(ch) && !bInQuotation)
            {
                SetParam(strParamName, lpszCmdLine + nParamValuePos, nPos - nParamValuePos);
                nStatus = em_Cmd_New_Arg;
            }
            else if (IsQuotation(ch))
            {
                SetParam(strParamName, lpszCmdLine + nParamValuePos, nPos - nParamValuePos);
                nStatus = em_Cmd_New_Arg;
            }
            else
            {
                nStatus = em_Cmd_Arg_Value;
            }

            break;
        }
     }

     bResult = TRUE;

Exit0:

     return bResult;
}

BOOL CSZCommandLine::Analyze(int argc, TCHAR* argv[])
{
    if (argc < 2)
        return TRUE;

    CString strCmdLine;
    strCmdLine.AppendFormat(L"\"%s\"", argv[1]);
    for (int i = 2; i < argc; ++i)
    {
        strCmdLine.Append(L" ");
        strCmdLine.AppendFormat(L"\"%s\"", argv[i]);
    }

    return Analyze(strCmdLine);
}

BOOL CSZCommandLine::HasParam(LPCTSTR lpszParamName)
{
    if (m_mapParams.Lookup(lpszParamName))
        return TRUE;

    return FALSE;
}

BOOL CSZCommandLine::SetParam(LPCTSTR lpszParamName, LPCTSTR lpszParamValue)
{
    m_mapParams[lpszParamName] = lpszParamValue;

    return TRUE;
}

BOOL CSZCommandLine::SetParam(LPCTSTR lpszParamName, LPCTSTR lpszParamValue, int nLen)
{
    m_mapParams[lpszParamName].SetString(lpszParamValue, nLen);

    return TRUE;
}

BOOL CSZCommandLine::SetParam(LPCTSTR lpszParamName, int nValue)
{
    m_mapParams[lpszParamName].Format(_T("%d"), nValue);

    return TRUE;
}

CSZCommandLine::CParam CSZCommandLine::GetParam(LPCTSTR lpszParamName)
{
    CString strValue;

    if (!m_mapParams.Lookup(lpszParamName, strValue))
        return CParam();

    return CParam(strValue);
}

void CSZCommandLine::AppendSubCommand(LPCTSTR lpszSubCommand)
{
    if (!lpszSubCommand || !*lpszSubCommand)
        return;

    m_subCmdList.AddTail(lpszSubCommand);
}

void CSZCommandLine::RemoveParam(LPCTSTR lpszParamName)
{
    m_mapParams.RemoveKey(lpszParamName);
}

void CSZCommandLine::RemoveAllParams()
{
    m_subCmdList.RemoveAll();
    m_mapParams.RemoveAll();
}

void CSZCommandLine::GetCmdLine(CString& strCmdLine, BOOL bAutoQuoteSpace)
{
    strCmdLine = _T("");

    POSITION pos = m_subCmdList.GetHeadPosition();
    while (pos)
    {
        CString strSubCommand = m_subCmdList.GetNext(pos);
        if (strSubCommand.IsEmpty())
            continue;

        strCmdLine.Append(L" ");
        strCmdLine.Append(strSubCommand);
    }


    pos = m_mapParams.GetStartPosition();

    while (pos)
    {
        _CmdParamMap::CPair *pair = m_mapParams.GetNext(pos);
        if (!pair)
            continue;


        // param key
        CString strParam;
        strParam.Format(_T(" -%s"), pair->m_key);


        // param value
        if (pair->m_value.IsEmpty())
        {
            NULL;
        }
        else if (bAutoQuoteSpace)
        {
            if (-1 != pair->m_value.Find(_T(' ')))
            {
                strParam.AppendFormat(_T(":\"%s\""), pair->m_value);
            }
            else
            {
                strParam.AppendFormat(_T(":%s"), pair->m_value);
            }
        }
        else
        {
            strParam.AppendFormat(_T(":%s"), pair->m_value);
        }


        strCmdLine += strParam;
    }
}

inline BOOL CSZCommandLine::IsWhiteSpace(TCHAR ch)
{
    switch (ch)
    {
    case _T(' '):
    case _T('\t'):
    case _T('\0'):
        return TRUE;
    }

    return FALSE;
}

inline BOOL CSZCommandLine::IsArgNamePrefix(TCHAR ch)
{
    switch (ch)
    {
    case _T('-'):
    case _T('/'):
        return TRUE;
    }

    return FALSE;
}

inline BOOL CSZCommandLine::IsArgValuePrefix(TCHAR ch)
{
    switch (ch)
    {
    case _T(':'):
    case _T('='):
        return TRUE;
    }

    return FALSE;
}

inline BOOL CSZCommandLine::IsQuotation(TCHAR ch)
{
    switch (ch)
    {
    case _T('\"'):
        return TRUE;
    }

    return FALSE;
}

inline BOOL CSZCommandLine::IsArgNameChar(TCHAR ch)
{
    if (IsAlpha(ch))
        return TRUE;

    if (IsDecNumber(ch))
        return TRUE;

    switch (ch)
    {
    case _T('_'):
    case _T('-'):
        return TRUE;
    }

    return FALSE;
}

inline BOOL CSZCommandLine::IsAlpha(TCHAR ch)
{
    if (_T('a') <= ch && ch <= _T('z'))
        return TRUE;

    if (_T('A') <= ch && ch <= _T('Z'))
        return TRUE;

    return FALSE;
}

inline BOOL CSZCommandLine::IsDecNumber(TCHAR ch)
{
    if (_T('0') <= ch && ch <= _T('9'))
        return TRUE;

    return FALSE;
}




CSZCommandLine::CParam CSZCommandLine::operator[](LPCTSTR lpszParamName)
{
    return GetParam(lpszParamName);
}









CSZCommandLine::CParam::CParam():
    m_bValid(FALSE)
{
}

CSZCommandLine::CParam::CParam(LPCTSTR lpszValue):
    m_bValid(TRUE)
{
    m_strValue = lpszValue;
}

CSZCommandLine::CParam::~CParam()
{

}

CString CSZCommandLine::CParam::String()
{
    return m_strValue;
}

int CSZCommandLine::CParam::Int()
{
    return _szttoi(m_strValue);
}

BOOL CSZCommandLine::CParam::Valid()
{
    return m_bValid;
}

BOOL CSZCommandLine::Execute(LPCTSTR lpszProcessName, BOOL bForceAdmin, BOOL bWaitProcess)
{
    BOOL bRet = FALSE;
    PROCESS_INFORMATION processInfo;
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    CString strCmdLine;
    HANDLE hProcess = NULL;
    
    GetCmdLine(strCmdLine);

    if (bForceAdmin)
    {
        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };

        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = NULL;
        sei.lpVerb = _T("runas");
        sei.lpFile = lpszProcessName;
        sei.lpParameters = (LPWSTR)(LPCWSTR)strCmdLine;
        sei.nShow = SW_SHOWNORMAL;

        bRet = ::ShellExecuteEx(&sei);

        hProcess = sei.hProcess;
    }
    else
    {
        bRet = ::CreateProcess(
            lpszProcessName, 
            (LPWSTR)(LPCWSTR)strCmdLine, 
            NULL, NULL, FALSE, 0, NULL, NULL, &si, &processInfo
            );

        if (bRet)
        {
            ::CloseHandle(processInfo.hThread);
            hProcess = processInfo.hProcess;
        }
    }

    if (bRet)
    {
        if (bWaitProcess)
        {
            ::WaitForSingleObject(hProcess, INFINITE);
        }
        ::CloseHandle(hProcess);
    }

    return bRet;
}

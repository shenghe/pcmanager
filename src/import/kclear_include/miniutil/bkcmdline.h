#pragma once

#include <shellapi.h>
#include <atlcoll.h>

/***********************************************************************

    Command Line Format:

    {-|/}$ParamName1[:$ParamValue1][ {-|/}$ParamName2[:$ParamValue2] ...]

    NOTICE: $ParamValue must be in double commas when 
            it contains space character

    Example:

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    CBkCmdLine cmdLine;

    // Setup Default Params
    cmdLine.SetParam(_T("ParamName"), FALSE);
    cmdLine.SetParam(_T("ParamName2"), _T("ABCDEF"));

    // Analyze Command Line
    cmdLine.Analyze(lpCmdLine);

    // Get Int Value
    BOOL bParam = cmdLine[_T("IntParam")];

    // Get String Value
    CString strParam = cmdLine[_T("StringParam")];

    return 0;
}

***********************************************************************/

class CBkCmdLine
{
public:
    CBkCmdLine(LPCTSTR lpszCmdLine = NULL)
    {
        Analyze(lpszCmdLine);
    }

    ~CBkCmdLine()
    {
    }

    class CParam
    {
    public:
        CParam(LPCTSTR lpszValue)
        {
            m_strValue = lpszValue;
        }

        ~CParam()
        {
        }

        CString String()
        {
            return m_strValue;
        }

        operator CString()
        {
            return m_strValue;
        }

        operator int()
        {
            return ::StrToInt(m_strValue);
        }

    private:
        CString m_strValue;
    };

    BOOL Analyze(LPCTSTR lpszCmdLine)
    {
        BOOL bResult = FALSE;
        int nPos = 0, nParamNamePos = 0, nParamValuePos = 0;
        int nStatus = 0;
        CString strParamName;
        BOOL bInComma = FALSE;
        TCHAR chPos = _T('\0');

        if (!lpszCmdLine)
            goto Exit0;

        m_strCmdLine = lpszCmdLine;

        for (int nPos = 0; 0 == nPos || _T('\0') != lpszCmdLine[nPos - 1]; nPos ++)
        {
            chPos = lpszCmdLine[nPos];

            switch (nStatus)
            {
            case 0:

                if (bInComma)
                {
                    if (_T('\"') == chPos)
                        bInComma = FALSE;
                }
                else if (_T('-') == chPos || _T('/') == chPos)
                    nStatus = 1;
                else if (_T('\"') == chPos)
                    bInComma = TRUE;

                break;

            case 1:
                nParamNamePos = nPos;

                nStatus = 2;
                break;

            case 2:
                if (_T(' ') == chPos || _T('\0') == chPos)
                {
                    strParamName = m_strCmdLine.Mid(nParamNamePos, nPos - nParamNamePos);

                    SetParam(strParamName, TRUE);

                    nStatus = 0;
                }
                else if (_T(':') == chPos)
                {
                    strParamName = m_strCmdLine.Mid(nParamNamePos, nPos - nParamNamePos);
                    nStatus = 3;
                }

                break;

            case 3:
                if (_T('\"') == chPos)
                {
                    nPos ++;
                    bInComma = TRUE;
                }
                else if (_T(' ') == chPos || _T('\0') == chPos)
                {
                    SetParam(strParamName, _T(""));
                    nStatus = 0;

                    break;
                }

                nParamValuePos = nPos;
                nStatus = 4;

                break;

            case 4:
                if ((_T(' ') == chPos && !bInComma) || _T('\0') == chPos || (_T('\"') == chPos && bInComma))
                {
                    SetParam(strParamName, m_strCmdLine.Mid(nParamValuePos, nPos - nParamValuePos));
                    nStatus = 0;
                    bInComma = FALSE;
                }

                break;
            }
        }

        bResult = TRUE;

    Exit0:

        return bResult;
    }

    BOOL HasParam(LPCTSTR lpszParamName)
    {
        if (m_mapParams.Lookup(lpszParamName))
            return TRUE;

        return FALSE;
    }

    BOOL SetParam(LPCTSTR lpszParamName, LPCTSTR lpszParamValue)
    {
        m_mapParams[lpszParamName] = lpszParamValue;

        return TRUE;
    }

    BOOL SetParam(LPCTSTR lpszParamName, int nValue)
    {
        m_mapParams[lpszParamName].Format(_T("%d"), nValue);

        return TRUE;
    }

    void GetCmdLine(CString& strCmdLine)
    {
        POSITION pos = m_mapParams.GetStartPosition();
        CString strParam;

        strCmdLine = _T("");

        while (pos)
        {
            _CmdParamMap::CPair *pair = m_mapParams.GetNext(pos);

            if (pair->m_value == _T("1"))
                strParam.Format(_T(" -%s"), pair->m_key);
            else
                strParam.Format(_T(" -%s:%s"), pair->m_key, pair->m_value);

            strCmdLine += strParam;
        }
    }

    BOOL Execute(LPCTSTR lpszProcessName, BOOL bForceAdmin, BOOL bWaitProcess)
    {
        BOOL bRet = FALSE;
        PROCESS_INFORMATION processInfo;
        STARTUPINFO si = {sizeof(STARTUPINFO)};
        CString strCmdLine;
        DWORD dwErrCode = 0;
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

    CParam operator[](LPCTSTR lpszParamName)
    {
        CString strValue;

        m_mapParams.Lookup(lpszParamName, strValue);

        return CParam(strValue);
    }


protected:

    CString m_strCmdLine;

    template < typename T >
    class CBkCharTraits
    {
    };

    template <>
    class CBkCharTraits<char>
    {
    public:
        static char CharToUpper(char c)
        {
            if (c >= 'a' && c <= 'z')
                return c - 'a' + 'A';
            else
                return c;
        }

        static char CharToLower(char c)
        {
            if (c >= 'A' && c <= 'Z')
                return c - 'A' + 'a';
            else
                return c;
        }
    };

    template <>
    class CBkCharTraits<wchar_t>
    {
    public:
        static wchar_t CharToUpper(wchar_t c)
        {
            if (c >= 'a' && c <= 'z')
                return c - 'a' + 'A';
            else
                return c;
        }

        static wchar_t CharToLower(wchar_t c)
        {
            if (c >= 'A' && c <= 'Z')
                return c - 'A' + 'a';
            else
                return c;
        }
    };

    typedef CAtlMap<CString, CString, CStringElementTraitsI<CString, CBkCharTraits<TCHAR>>> _CmdParamMap;

    _CmdParamMap m_mapParams;

private:
};
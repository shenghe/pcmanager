#pragma once

#include <atlcoll.h>
#include <atlstr.h>
#include "szstr.h"

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
    CSZCommandLine cmdLine;

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

class CSZCommandLine
{
public:
    CSZCommandLine(LPCTSTR lpszCmdLine = NULL);
    ~CSZCommandLine();

    class CParam
    {
    public:
        CParam();
        CParam(LPCTSTR lpszValue);
        ~CParam();

        CString String();
        int     Int();
        BOOL    Valid();
    private:
        BOOL    m_bValid;
        CString m_strValue;
    };

    // SubCommand 是紧跟在Command之后的第一个单词,该单词全部由英文字母组成
    static CString FindSubCommand(LPCTSTR lpszCmdLine, LPCWSTR* lppszSubCmdLine);
    static CString FindSubCommand(int argc, TCHAR* argv[]);

    BOOL    Analyze(LPCTSTR lpszCmdLine);

    // 以main为入口的可执行程序有可能将命令行参数中的引号去除
    BOOL    Analyze(int argc, TCHAR* argv[]);

    BOOL    HasParam(LPCTSTR lpszParamName);
    BOOL    SetParam(LPCTSTR lpszParamName, LPCTSTR lpszParamValue);
    BOOL    SetParam(LPCTSTR lpszParamName, LPCTSTR lpszParamValue, int nLen);
    BOOL    SetParam(LPCTSTR lpszParamName, int nValue);
    CParam  GetParam(LPCTSTR lpszParamName);

    void    AppendSubCommand(LPCTSTR lpszSubCommand);

    void    RemoveParam(LPCTSTR lpszParamName);
    void    RemoveAllParams();

    void    GetCmdLine(CString& strCmdLine, BOOL bAutoQuoteSpace = TRUE);

    BOOL    Execute(LPCTSTR lpszProcessName, BOOL bForceAdmin, BOOL bWaitProcess);


    CParam operator[](LPCTSTR lpszParamName);

protected:

    enum EM_CMDLINE_STATUS
    {
        em_Cmd_New_Arg,
        em_Cmd_White_Space,
        em_Cmd_Sub_Command,
        em_Cmd_Arg_Name_Prefix,
        em_Cmd_Arg_Name,
        em_Cmd_Arg_Value_Prefix,
        em_Cmd_Arg_Value,
    };



    static BOOL IsWhiteSpace(TCHAR ch);
    static BOOL IsArgNamePrefix(TCHAR ch);
    static BOOL IsArgValuePrefix(TCHAR ch);
    static BOOL IsQuotation(TCHAR ch);
    static BOOL IsArgNameChar(TCHAR ch);

    static BOOL IsAlpha(TCHAR ch);
    static BOOL IsDecNumber(TCHAR ch);


    typedef CAtlMap<CString, CString, CStringElementTraitsI<CString, CSZCharTraits<TCHAR>>> _CmdParamMap;
    typedef CAtlList<CString> _CmdSubCmdList;

    _CmdSubCmdList  m_subCmdList;
    _CmdParamMap    m_mapParams;
};
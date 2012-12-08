/**
* @file    avpperf.cpp
* @brief   ...
* @author  bbcallen
* @date    2010-06-30 11:51
*/

#include "stdafx.h"
#include "avpperf.h"

#include <atltime.h>
#include "winmod\winlogfile.h"
#include "winmod\winpath.h"
#include "skylark2\skylarkpath.h"

NS_SKYLARK_USING
using namespace WinMod;

static CWinLogFile g_hLogFile;
inline void AVP_FORMAT_PERF(LPCWSTR lpszFormat, ...)
{
    va_list argList;
    va_start(argList, lpszFormat);

    g_hLogFile.LogFormatV(WINMOD_LLVL_ALWAYS, lpszFormat, argList);

    va_end( argList );
}


#define AVP_ECHO_PERF(x)    AVP_FORMAT_PERF(L"%s", x)


void CAvpEchoPerf::InitAvpEchoPerf()
{
    CWinPath PathExeName;
    PathExeName.GetModuleFileName(NULL);
    PathExeName.StripPath();

    CWinPath PathLogFile;
    CSkylarkPath::GetLogPath(PathLogFile.m_strPath, TRUE);
    PathLogFile.Append(PathExeName);

    ::MoveFileEx(PathLogFile.m_strPath + L".spf4.log", PathLogFile.m_strPath + L".spf5.log", MOVEFILE_REPLACE_EXISTING);
    ::MoveFileEx(PathLogFile.m_strPath + L".spf3.log", PathLogFile.m_strPath + L".spf4.log", MOVEFILE_REPLACE_EXISTING);
    ::MoveFileEx(PathLogFile.m_strPath + L".spf2.log", PathLogFile.m_strPath + L".spf3.log", MOVEFILE_REPLACE_EXISTING);
    ::MoveFileEx(PathLogFile.m_strPath + L".spf1.log", PathLogFile.m_strPath + L".spf2.log", MOVEFILE_REPLACE_EXISTING);
    ::MoveFileEx(PathLogFile.m_strPath + L".spf0.log", PathLogFile.m_strPath + L".spf1.log", MOVEFILE_REPLACE_EXISTING);

    g_hLogFile.CreateAsLogFile(PathLogFile.m_strPath + L".spf0.log");
}

CString CAvpEchoPerf::FormatPerfNumber(ULONGLONG uPerfNumber)
{
    CString strPerfString;
    if (uPerfNumber >= CFileTime::Second)
    {
        strPerfString.AppendFormat(L"%lu,",     DWORD(uPerfNumber / CFileTime::Second));
        strPerfString.AppendFormat(L"%03lu,",   DWORD(uPerfNumber / CFileTime::Millisecond % 1000));
        strPerfString.AppendFormat(L"%04lu",    DWORD(uPerfNumber % CFileTime::Millisecond));
    }
    else if (uPerfNumber >= CFileTime::Millisecond)
    {
        strPerfString.AppendFormat(L"%lu,",     DWORD(uPerfNumber / CFileTime::Millisecond));
        strPerfString.AppendFormat(L"%04lu",    DWORD(uPerfNumber % CFileTime::Millisecond));
    }
    else
    {
        strPerfString.AppendFormat(L"%lu",      DWORD(uPerfNumber));
    }

    return strPerfString;
}

CString CAvpEchoPerf::FormatPerfLine(IBKEngPerformanceView* piPerfView, LPCWSTR lpszTopic, const BKENG_PERF_DATA& perfData)
{
    ULONGLONG uPerfTotal = piPerfView->TickToSystemTime(perfData.m_uPerfTotal);
    ULONG     uPerfCount = perfData.m_uPerfCount;
    ULONGLONG uAverage   = 0;
    if (0 == uPerfCount)
    {
        uAverage = 0;
    }
    else
    {
        uAverage = uPerfTotal / uPerfCount;
    }


    CString strPerfTotal    = FormatPerfNumber(uPerfTotal);
    CString strPerfAverage  = FormatPerfNumber(uAverage);
    CString strPerfMax1     = FormatPerfNumber(perfData.m_uTopPerf[0]);
    CString strPerfMax2     = FormatPerfNumber(perfData.m_uTopPerf[1]);
    CString strPerfLine;
    strPerfLine.Format(
        L"   %-5s | %15s | %8lu | %12s | %12s | %12s\r\n",
        lpszTopic,
        strPerfTotal,
        uPerfCount,
        strPerfAverage,
        strPerfMax1,
        strPerfMax2);

    return strPerfLine;
}

HRESULT CAvpEchoPerf::EchoEnginePerf(IBKEngine* piEngine)
{
    return EchoPerf((IUnknown*)piEngine);
}

HRESULT CAvpEchoPerf::EchoPerf(IUnknown* piEngine)
{
    if (!piEngine)
        return E_INVALIDARG;

    CComPtr<IBKEngPerformanceView> spiPerfView;
    HRESULT hr = piEngine->QueryInterface(__uuidof(IBKEngPerformanceView), (void**)&spiPerfView);
    if (FAILED(hr))
    {
        AVP_FORMAT_PERF(L"\rfailed to QueryInterface IBKEngPerformanceCounter : 0x%lx\r\n", hr);
        return hr;
    }



    BKENG_PERF_DATA perfData[BKENG_PERF_MAX_ITEM];
    for (DWORD i = 0; i < _countof(perfData); ++i)
    {
        spiPerfView->GetPerformanceData(i, &perfData[i]);
    }




    AVP_FORMAT_PERF(L"\r\n\r\n========================================\r\n");
    AVP_FORMAT_PERF(
        L"   %-5s | %15s | %8s | %12s | %12s | %12s\r\n",
        L"Tp",
        L"Tt",
        L"Ct",
        L"Av",
        L"Mx1",
        L"Mx2");
    AVP_FORMAT_PERF(L"\r\n");

    //EchoPerf(spiPerfView,   L"debug1",                  perfData[BKENG_PERF_DEBUG1]);
    //EchoPerf(spiPerfView,   L"debug2",                  perfData[BKENG_PERF_DEBUG2]);
    //EchoPerf(spiPerfView,   L"debug3",                  perfData[BKENG_PERF_DEBUG3]);
    //EchoPerf(spiPerfView,   L"debug4",                  perfData[BKENG_PERF_DEBUG4]);
    //AVP_ECHO_PERF(L"\r\n");

    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"ls",      perfData[BKENG_PERF_LOCAL_SCAN]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"rs",      perfData[BKENG_PERF_REVIEW_SCAN]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"q",       perfData[BKENG_PERF_QUERY]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"rq",      perfData[BKENG_PERF_REQUERY]));
    AVP_ECHO_PERF(L"\r\n");

    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"nm",      perfData[BKENG_PERF_NORMALIZE_PATH]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"tg",      perfData[BKENG_PERF_CALC_FILE_TAG]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"cf",      perfData[BKENG_PERF_CREATE_FILE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"cp",      perfData[BKENG_PERF_CHECK_PE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"hs",      perfData[BKENG_PERF_HASH_FILE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"wt",      perfData[BKENG_PERF_WINTRUST]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"en",      perfData[BKENG_PERF_ENCODE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"de",      perfData[BKENG_PERF_DECODE]));
    AVP_ECHO_PERF(L"\r\n");

    //AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"lh",      perfData[BKENG_PERF_LOCAL_HEURIST]));
    //AVP_ECHO_PERF(L"\r\n");

    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"as",      perfData[BKENG_PERF_AVENG_SCAN]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"ac",      perfData[BKENG_PERF_AVENG_CLEAN]));

    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"lpn",     perfData[BKENG_PERF_LOOKUP_PATH_TO_NONPE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"lpw",     perfData[BKENG_PERF_LOOKUP_PATH_TO_WINTRUST]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"lph",     perfData[BKENG_PERF_LOOKUP_PATH_TO_HASH]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"lhl",     perfData[BKENG_PERF_LOOKUP_HASH_TO_LEVEL]));
    AVP_ECHO_PERF(L"\r\n");

    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"upn",     perfData[BKENG_PERF_UPDATE_PATH_TO_NONPE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"upw",     perfData[BKENG_PERF_UPDATE_PATH_TO_WINTRUST]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"uph",     perfData[BKENG_PERF_UPDATE_PATH_TO_HASH]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"upl",     perfData[BKENG_PERF_UPDATE_HASH_TO_LEVEL]));
    AVP_ECHO_PERF(L"\r\n");

    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"fpn",     perfData[BKENG_PERF_FLUSH_PATH_TO_NONPE]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"fpw",     perfData[BKENG_PERF_FLUSH_PATH_TO_WINTRUST]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"fph",     perfData[BKENG_PERF_FLUSH_PATH_TO_HASH]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"fhl",     perfData[BKENG_PERF_FLUSH_HASH_TO_LEVEL]));
    AVP_ECHO_PERF(L"\r\n");

    //AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"kld1",    perfData[BKENG_PERF_KCLOUD_LOCAL_DETECT_1ST]));
    //AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"kld2",    perfData[BKENG_PERF_KCLOUD_LOCAL_DETECT_2ND]));
    AVP_ECHO_PERF(FormatPerfLine(spiPerfView,   L"knd",     perfData[BKENG_PERF_KCLOUD_NET_DETECT]));
    AVP_ECHO_PERF(L"\r\n");

    return S_OK;
}
/**
* @file    avpperf.h
* @brief   ...
* @author  bbcallen
* @date    2010-06-29 19:10
*/

#ifndef AVPPERF_H
#define AVPPERF_H

#include "skylark2\skylarkbase.h"
#include "skylark2\bkeng.h"
#include "skylark2\bkperf.h"

NS_SKYLARK_BEGIN

class CAvpEchoPerf
{
public:
    static void     InitAvpEchoPerf();
    static CString  FormatPerfNumber(ULONGLONG uPerfNumber);
    static CString  FormatPerfLine(IBKEngPerformanceView* piPerfView, LPCWSTR lpszTopic, const BKENG_PERF_DATA& perfData);
    static HRESULT  EchoEnginePerf(IBKEngine* piEngine);
    static HRESULT  EchoPerf(IUnknown* piEngine);
};


NS_SKYLARK_END

#endif//AVPPERF_H
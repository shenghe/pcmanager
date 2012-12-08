#include "stdafx.h"
#include "kcompare.h"

// str1和str2 不要用引用，因为里面会改变str1和str2的值
int KCompare::StringNoCase(ATL::CString str1, ATL::CString str2)
{
	str1 = str1.MakeLower();
	str2 = str2.MakeLower();

    int nCmpResult = KPFW_COMPARE_RESULT_ERROR;

    if(str1 == str2)
    {
        nCmpResult = KPFW_COMPARE_RESULT_EQUAL;
    }
    else if(str1 < str2)
    {
        nCmpResult = KPFW_COMPARE_RESULT_LESS;
    }
    else
    {
        nCmpResult = KPFW_COMPARE_RESULT_GREAT;
    }

    return nCmpResult;
}
#pragma once

//////////////////////////////////////////////////////////////////////////
#include "miniutil/bkjson.h"
#include <atlenc.h>

//////////////////////////////////////////////////////////////////////////


class CTrashCleanerFeedback
{
public:
    BOOL Report(LPCTSTR lpDir, LPCTSTR lpVer,int nItemId)
    {
        CString strEncodedStr = KUTF8_To_UTF16(GetString(lpDir));
        if (!strEncodedStr.GetLength())
            return FALSE;

        return TRUE;
    }

protected:
    CStringA GetString(const CString& strDir)
    {
        int nSize;
        CStringA strCleanDir = KUTF16_To_UTF8(strDir);
        CAtlArray<CHAR> bufEncode;

        nSize = Base64EncodeGetRequiredLength(strCleanDir.GetLength(), ATL_BASE64_FLAG_NOCRLF);
        bufEncode.SetCount(nSize + 1);
        RtlZeroMemory(bufEncode.GetData(), nSize + 1);
        Base64Encode(
            (const BYTE*)(LPCSTR)strCleanDir, 
            strCleanDir.GetLength(), 
            bufEncode.GetData(),
            &nSize,
            ATL_BASE64_FLAG_NOCRLF
            );

        return bufEncode.GetData();
    }
};

//////////////////////////////////////////////////////////////////////////

#pragma once

class CExamCheckAV
{
public:
    CExamCheckAV()
    {
    }

    BOOL Check(BOOL &bUpToDate);
private:

    BOOL _CheckFromSecurityCenter(BOOL &bUpToDate);
    BOOL _CheckFromSecurityCenterByWMIServerName(LPCSTR lpszServerName, BOOL &bUpToDate);
    BOOL _CheckByRegisterValue();
};
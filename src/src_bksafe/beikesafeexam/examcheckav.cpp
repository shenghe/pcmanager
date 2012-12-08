#include "stdafx.h"
#include "examcheckav.h"
#include <Wbemidl.h>
#include <miniutil/bkfileutil.h>

#pragma comment(lib, "Wbemuuid.lib")

BOOL CExamCheckAV::Check(BOOL &bUpToDate)
{
    BOOL bInstalled = FALSE;

    bInstalled = _CheckFromSecurityCenter(bUpToDate);
    if (bInstalled)
        return TRUE;

    bInstalled = _CheckByRegisterValue();
    if (bInstalled)
        return TRUE;

    return FALSE;
}

BOOL CExamCheckAV::_CheckFromSecurityCenter(BOOL &bUpToDate)
{
    BOOL bRet = FALSE;

    bUpToDate = TRUE;

    bRet = _CheckFromSecurityCenterByWMIServerName("root\\SecurityCenter", bUpToDate);
    if (!bRet)
        bRet = _CheckFromSecurityCenterByWMIServerName("root\\SecurityCenter2", bUpToDate);

    return bRet;
}

BOOL CExamCheckAV::_CheckFromSecurityCenterByWMIServerName(LPCSTR lpszServerName, BOOL &bUpToDate)
{
    BOOL bRet = FALSE;

    CComPtr<IWbemLocator> spiLoc;
    CComPtr<IWbemServices> spiSvc;
    CComPtr<IEnumWbemClassObject> spiEnumerator;
    CComPtr<IWbemClassObject> spiClsObj;
    CComVariant varProductUpToDate;
    ULONG uReturn = 0;

    HRESULT hr = ::CoCreateInstance(
        CLSID_WbemLocator, 
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (void**) &spiLoc);
    if (FAILED(hr))
        goto Exit0;

    hr = spiLoc->ConnectServer(
        CComBSTR(lpszServerName),
        NULL, 
        NULL,
        0,
        NULL,
        0, 
        0,
        &spiSvc);
    if (FAILED(hr))
        goto Exit0;

    hr = ::CoSetProxyBlanket(
        spiSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);
    if (FAILED(hr))
        goto Exit0;

    hr = spiSvc->ExecQuery(
        CComBSTR("WQL"), 
        CComBSTR("SELECT * FROM AntiVirusProduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &spiEnumerator);
    if (FAILED(hr))
        goto Exit0;

    hr = spiEnumerator->Next(WBEM_INFINITE, 1, &spiClsObj, &uReturn);
    if(FAILED(hr) || 0 == uReturn)
        goto Exit0;

    hr = spiClsObj->Get(
        CComBSTR("productUptoDate"),
        0, 
        &varProductUpToDate,
        NULL, NULL);
    if (SUCCEEDED(hr) && (VT_BOOL == varProductUpToDate.vt))
    {
        bUpToDate = varProductUpToDate.boolVal;
    }

    bRet = TRUE;

Exit0:

    return bRet;
}

typedef struct _AVKEYINFO
{
    HKEY    hRootKey;
    LPCWSTR szKeyName;
    LPCWSTR szValueName;
}AVKEYINFO , *PAVKEYINFO;

#define VALUE_NAME_DISPLAY_NAME     L"DisplayName"
#define VALUE_NAME_IMAGE_PATH       L"ImagePath"
#define VALUE_NAME_PRODUCTROOT      L"ProductRoot"
#define VALUE_NAME_PROGRAM_PATH     L"ProgramPath"

AVKEYINFO g_AVKey[] = {
    // 卡巴斯基
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\protected\\avp9\\environment", VALUE_NAME_PRODUCTROOT },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\avp7\\environment", VALUE_NAME_PRODUCTROOT },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\avp6\\environment",VALUE_NAME_PRODUCTROOT },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\protected\\avp7\\environment", VALUE_NAME_PRODUCTROOT },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\protected\\avp80\\environment", VALUE_NAME_PRODUCTROOT },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\protected\\avp8\\environment", VALUE_NAME_PRODUCTROOT },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\installedproducts\\kaspersky anti-virus for workstation", L"folder" },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\installedproducts\\kaspersky anti-virus personal", L"folder" },
    { HKEY_LOCAL_MACHINE, L"software\\kasperskylab\\avp8\\environment", VALUE_NAME_PRODUCTROOT },
    // 金山毒霸
    { HKEY_LOCAL_MACHINE, L"software\\kingsoft\\antivirus\\v6.0", VALUE_NAME_PROGRAM_PATH },
    { HKEY_LOCAL_MACHINE, L"software\\kingsoft\\antivirus", VALUE_NAME_PROGRAM_PATH },
    { HKEY_LOCAL_MACHINE, L"software\\kingsoft\\antivirus", L"kis2008" },
    { HKEY_LOCAL_MACHINE, L"software\\kingsoft\\antivirus", L"kis2009" },
    { HKEY_LOCAL_MACHINE, L"software\\kingsoft\\antivirus\\kanclient", L"path" },
    { HKEY_CURRENT_USER, L"software\\kingsoft\\antivirus\\v6.0", VALUE_NAME_PROGRAM_PATH },
    { HKEY_CURRENT_USER, L"software\\kingsoft\\antivirus", VALUE_NAME_PROGRAM_PATH },
    // F-PROT
    // avast! 
    { HKEY_LOCAL_MACHINE, L"software\\alwil software\\avast\\4.0", L"avast4programfolder" },
    { HKEY_LOCAL_MACHINE, L"software\\alwil software\\avast\\5.0", L"programfolder" },
    // 瑞星
    { HKEY_LOCAL_MACHINE, L"software\\rising\\rav", L"installpath" },
    { HKEY_LOCAL_MACHINE, L"software\\rising\\ris", L"installpath" },
    // McAfee
    { HKEY_LOCAL_MACHINE, L"software\\network associates\\epolicy orchestrator\\application plugins\\viruscan8000", L"install path" },
    { HKEY_LOCAL_MACHINE, L"software\\mcafee.com\\agent", L"install dir" },
    { HKEY_LOCAL_MACHINE, L"software\\network associates\\epolicy orchestrator\\application plugins\\viruscan7100", L"install path" },
    { HKEY_LOCAL_MACHINE, L"software\\mcafee\\epolicy orchestrator\\application plugins\\viruscan8700", L"install path" },
    { HKEY_LOCAL_MACHINE, L"software\\mcafee\\epolicy orchestrator\\application plugins\\viruscan8600", L"install path" },
    { HKEY_LOCAL_MACHINE, L"software\\mcafee\\msc", L"install dir" },
    { HKEY_LOCAL_MACHINE, L"software\\mcafee.com\\virusscan online", L"install dir" },
    // 诺顿
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"norton 360" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"norton internet security" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"ui" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"nav" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"sav install directory" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"scs install directory" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"ui2" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"navnt" },
    { HKEY_LOCAL_MACHINE, L"software\\symantec\\installedapps", L"n360" },
    // KILL
    { HKEY_LOCAL_MACHINE, L"software\\computerassociates\\kill\\currentversion\\path", L"home" },
    { HKEY_LOCAL_MACHINE, L"software\\computerassociates\\etrustitm\\currentversion\\path", L"home" },
    // 安天防线
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\安天防线_is1", L"app path" },
    // ZoneAlarm Antivirus
    { HKEY_LOCAL_MACHINE, L"software\\zone labs\\zonealarm", L"installdirectory" },
    // Grisoft AVG
    { HKEY_LOCAL_MACHINE, L"software\\avg\\avg8", L"avgdir" },
    { HKEY_LOCAL_MACHINE, L"software\\avg\\avg9", L"avgdir" },
    { HKEY_LOCAL_MACHINE, L"software\\grisoft\\avgantispyware", L"installdir" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\avg7uninstall\\directories", L"dir_avgdir" },
    // ClamWin
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\clamwin free antivirus_is1", L"installlocation" },
    // 趋势科技
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\serverprotect for nt", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\trendmicro\\pc-cillinntcorp\\currentversion", L"application path" },
    { HKEY_LOCAL_MACHINE, L"software\\trendmicro\\pc-cillin", L"application path" },
    // 微点
    { HKEY_LOCAL_MACHINE, L"software\\mpav", L"mpav1000" },
    { HKEY_LOCAL_MACHINE, L"software\\micropoint\\anti-attack", L"mp100000" },
    // Virus 驱逐舰
    // 光华反病毒
    { HKEY_LOCAL_MACHINE, L"software\\rygh\\vc", L"path" },
    // BitDefender
    { HKEY_LOCAL_MACHINE, L"software\\bitdefender\\bitdefender desktop\\maintenance\\install", L"installdir" },
    { HKEY_LOCAL_MACHINE, L"software\\softwin\\bitdefender desktop\\maintenance\\install", L"installdir" },
    // ESET NOD32
    { HKEY_LOCAL_MACHINE, L"software\\eset\\eset security\\currentversion\\info", L"installdir" },
    { HKEY_LOCAL_MACHINE, L"software\\eset\\nod\\currentversion\\info", L"installdir" },
    // 360杀毒
    { HKEY_LOCAL_MACHINE, L"software\\bitdefender\\bitdefender desktop\\maintenance\\install", L"installdir" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\app paths\\360sd.exe", L"path" },
    // Quick Heal
    { HKEY_LOCAL_MACHINE, L"software\\quick heal\\quick heal total security", L"datapath" },
    { HKEY_LOCAL_MACHINE, L"software\\cat computer services\\quick heal", L"datapath" },
    { HKEY_LOCAL_MACHINE, L"software\\quick heal\\quick heal antivirus plus", L"datapath" },
    // Microsoft Security Essentials
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\microsoft antimalware", L"installlocation" },
    // 安铁诺
    { HKEY_LOCAL_MACHINE, L"software\\sanlen\\antiunknown", L"installpath" },
    // nProtect Personal
    { HKEY_LOCAL_MACHINE, L"software\\incainternet\\npp5", L"installdir" },
    // 北信源
    // f-secure
    { HKEY_LOCAL_MACHINE, L"software\\data fellows\\f-secure\\f-secure gui", L"installationdirectory" },
    // norman
    { HKEY_LOCAL_MACHINE, L"software\\norman data defense systems", L"rootpath" },
    // 费尔托斯特安全
    { HKEY_LOCAL_MACHINE, L"software\\filseclab\\twister", L"mainpath" },
    // 江民杀毒软件
    { HKEY_LOCAL_MACHINE, L"software\\jiangmin", L"installpath" },
    // G DATA杀毒软件
    { HKEY_LOCAL_MACHINE, L"software\\g data\\antivirenkit", L"pgmpath" },
    // Avira
    { HKEY_LOCAL_MACHINE, L"software\\avira\\premium security suite", L"path" },
    { HKEY_LOCAL_MACHINE, L"software\\avira\\antivir personaledition classic", L"path" },
    { HKEY_LOCAL_MACHINE, L"software\\avira\\antivir workstation", L"path" },
    { HKEY_LOCAL_MACHINE, L"software\\avira\\antivir desktop", L"path" },
    { HKEY_LOCAL_MACHINE, L"software\\avira\\antivir personaledition premium", L"path" },
    // 安博士
    { HKEY_LOCAL_MACHINE, L"software\\ahnlab\\v3pro", L"installpath" },
    { HKEY_LOCAL_MACHINE, L"software\\ahnlab\\v3is2007", L"installpath" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\{af8267c6_8886_4cfd_aac7_48bcb879743f}", L"installlocation" },
    // eTrust EZ Antivirus
    { HKEY_LOCAL_MACHINE, L"software\\computerassociates\\anti-virus\\install", L"programpath" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\vetwin32vp5", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\computerassociates\\etrustitm\\currentversion\\path", L"home" },
    // comodo
    { HKEY_LOCAL_MACHINE, L"software\\comodogroup\\cdi\\1", L"installproductpath" },
    // Microsoft Forefront Client Security
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\microsoft operations manager\\2.0\\setup", L"installdirectory" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\microsoft forefront\\client security\\1.0\\am", L"installlocation" },
    // sophos
    { HKEY_LOCAL_MACHINE, L"software\\sophos\\savservice\\application", L"path" },
    { HKEY_LOCAL_MACHINE, L"software\\sophos\\sweepnt", L"path" },
    // pc tools
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\pc tools internet security", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\pctools\\pctav", L"applicationpath" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\spyware doctor", L"installlocation" },
    // 熊猫卫士
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\panda cloud antivirus", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\panda software\\panda antivirus platinum", L"path" },
    { HKEY_LOCAL_MACHINE, L"software\\panda software\\panda antivirus lite", L"dir" },
    { HKEY_LOCAL_MACHINE, L"software\\panda software\\setup", L"path" },
    // Windows Live OneCare
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\winss", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\onecare protection", L"installlocation" },
    // Dr.Web大蜘蛛反病毒
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\{bbe2f69c-4338-11d7-8f0c-00a0244f4e2d}", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\{01b91c25-272b-415f-b9d1-f2780448c343}", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\{2bd3661d-1384-4ef4-9e5c-dfdb8ee6e3ea}", L"installlocation" },
    { HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows\\currentversion\\uninstall\\{e96b3169-86f4-415e-8e87-f0b6251167fb}", L"installlocation" },
    // 结束标记
    { NULL, NULL, NULL }
};

BOOL CExamCheckAV::_CheckByRegisterValue() 
{
    BOOL        bInstalled  = FALSE;
    LONG        lRet        = ERROR_SUCCESS;
    PAVKEYINFO  pAVKey      = g_AVKey;
    CRegKey     reg;
    LPTSTR      szFileName  = new TCHAR[MAX_PATH * 2 + 1];
    DWORD       dwLen = 0;

    // 如果内存都申请失败了，报没装杀软也没啥用
    if (NULL == szFileName)
        return TRUE;

    while (NULL != pAVKey->hRootKey && !bInstalled)
    {
        lRet = reg.Open(pAVKey->hRootKey, pAVKey->szKeyName, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS == lRet)
        {
            dwLen = MAX_PATH * 2;
            szFileName[0] = '\0';

            lRet = reg.QueryStringValue(pAVKey->szValueName, szFileName, &dwLen);
            if (ERROR_FILE_NOT_FOUND != lRet)
            {
                szFileName[max(dwLen, MAX_PATH * 2)] = '\0';
                bInstalled = (S_OK == CBkFileUtil::FileExist(szFileName));
            }

            reg.Close();
        }

        pAVKey ++;
    }

    if (szFileName)
    {
        delete[] szFileName;
        szFileName = NULL;
    }

    return bInstalled;
}
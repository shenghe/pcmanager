/********************************************************************
//	File Name	:	KAEAutoRunsReg.cpp
//	Version		:	1.0
//	Datetime	:	2006-6-27	16:59	--	Created.
//	Author		:	Chenghu
//	Comment		:	
*********************************************************************/

#include "stdafx.h"
#include <AclAPI.h>
#include <tchar.h>

#include "KAEAutorunsReg.h"

unsigned int KAERegOpenKey(
                           /* [In] */ HKEY hKey,
                           /* [In] */ LPWSTR lpSubKey,
                           /* [Out] */ PHKEY phkResult,
                           /* [In] */ DWORD AceessType
                           )
{
    if(ERROR_SUCCESS == ::RegOpenKeyExW(hKey, lpSubKey, 0, AceessType, phkResult))
        return TRUE;

    else if (ERROR_ACCESS_DENIED == ::RegOpenKeyExW(hKey, lpSubKey, 0, AceessType, phkResult))
    {
        KAGRegUpKeySecurity(hKey, lpSubKey);
        if (ERROR_SUCCESS == ::RegOpenKeyExW(hKey, lpSubKey, 0, AceessType, phkResult))
            return TRUE;
        else
            return FALSE;
    }
    return FALSE;
}

unsigned int KAERegCreateKey(
                             /* [In] */ HKEY hKey,
                             /* [In] */ LPWSTR lpSubKey,
                             /* [Out] */ PHKEY phkResult
                             )
{
    unsigned long uDisposition = REG_CREATED_NEW_KEY;
    if(ERROR_SUCCESS == ::RegCreateKeyExW(hKey, lpSubKey, 0, 
        NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, phkResult, &uDisposition))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegCloseKey(
                            /* [In] */ HKEY hKey
                            )
{
    if(ERROR_SUCCESS == ::RegCloseKey(hKey))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegQueryValue(
                              /* [In] */ HKEY hKey,
                              /* [In] */ LPWSTR lpSubKey,
                              /* [Out] */ LPWSTR lpValue,
                              /* [In, Out] */ PLONG lpcbValue
                              )
{
    if(ERROR_SUCCESS == ::RegQueryValueW(hKey, lpSubKey, lpValue, lpcbValue))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegQueryValueEx(
                                /* [In] */ HKEY hKey,
                                /* [In] */ LPWSTR lpValueName,
                                /* [NULL] */ LPDWORD lpReserved,
                                /* [Out] */ LPDWORD lpType,
                                /* [Out] */ LPBYTE lpData,
                                /* [In, Out] */ LPDWORD lpcbData
                                )
{
    if(ERROR_SUCCESS == ::RegQueryValueExW(hKey, lpValueName, lpReserved,
        lpType, lpData, lpcbData))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegEnumKey(
                           /* [In] */ HKEY hKey,
                           /* [In] */ DWORD dwIndex,
                           /* [Out] */ LPWSTR lpName,
                           /* [In] */ DWORD cchName
                           )
{
    if(ERROR_SUCCESS == ::RegEnumKeyW(hKey, dwIndex, lpName, cchName))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegEnumValue(
                             /* [In] */ HKEY hKey,
                             /* [In] */ DWORD dwIndex,
                             /* [Out] */ LPWSTR lpValueName,
                             /* [In, Out] */ LPDWORD lpcValueName,
                             /* [NULL] */ LPDWORD lpReserved,
                             /* [Out] */ LPDWORD lpType,
                             /* [Out] */ LPBYTE lpData,
                             /* [In, Out] */ LPDWORD lpcbData
                             )
{
    if(ERROR_SUCCESS == ::RegEnumValueW(hKey, dwIndex, lpValueName, lpcValueName,
        lpReserved, lpType, lpData, lpcbData))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegSetValue(
                            /* [In] */ HKEY hKey,
                            /* [In] */ LPWSTR lpSubKey,
                            /* [In] */ DWORD dwType,
                            /* [In] */ LPWSTR lpData,
                            /* [In] */ DWORD cbData
                            )
{
    if(ERROR_SUCCESS == ::RegSetValueW(hKey, lpSubKey, dwType, lpData, cbData))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegSetValueEx(
                              /* [In] */ HKEY hKey,
                              /* [In] */ LPWSTR lpValueName,
                              /* [0] */ DWORD Reserved,
                              /* [In] */ DWORD dwType,
                              /* [In] */ const BYTE* lpData,
                              /* [In] */ DWORD cbData
                              )
{
    if(ERROR_SUCCESS == ::RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegDeleteKey(
                             /* [In] */ HKEY hKey,
                             /* [In] */ LPWSTR lpSubKey
                             )
{
    if(ERROR_SUCCESS == ::RegDeleteKeyW(hKey, lpSubKey))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegDeleteKeyEx(
                               /* [In] */ HKEY hKey,
                               /* [In] */ LPWSTR lpSubKey
                               )
{
    int nRet = FALSE;
    int nRetCode = FALSE;
    HKEY hSubKey = NULL;
    int nIndex = 0;
    wchar_t wszSubSubKey[MAX_STRING_SIZE];
    int nRetCode02 = 0;

    nRetCode = KAERegOpenKey(hKey, lpSubKey, &hSubKey);
    if(!nRetCode)
        goto Exit0;

    while(nRetCode == TRUE)
    {
        nRetCode = KAERegEnumKey(hSubKey, nIndex, (wchar_t *)&wszSubSubKey, MAX_STRING_SIZE);
        if(nRetCode == TRUE)
        {
            nRetCode02 = KAERegDeleteKeyEx(hSubKey, wszSubSubKey);
            if(nRetCode02 == FALSE)
                goto Exit0;
        }
        else
            break;

        //	++ nIndex;
    }

    nRetCode = KAERegDeleteKey(hSubKey, L"");
    if(!nRetCode)
        goto Exit0;

    nRet = TRUE;
Exit0:
    if(hSubKey)
    {
        KAERegCloseKey(hSubKey);
        hSubKey = NULL;
    }
    return nRet;
}

unsigned int KAERegDeleteValue(
                               /* [In] */ HKEY hKey,
                               /* [In] */ LPWSTR lpValueName
                               )
{
    if(ERROR_SUCCESS == ::RegDeleteValueW(hKey, lpValueName))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAGRegUpKeySecurity(
                                 /* [In] */ HKEY hKey,
                                 /* [In] */const wchar_t *pwszKey
                                 )
{
    int nRet = FALSE;
    HKEY hTheKey = NULL;
    HKEY hOpenKey = NULL;
    wchar_t wszSubSubKey[MAX_STRING_SIZE];
    SID_IDENTIFIER_AUTHORITY SecIA = SECURITY_NT_AUTHORITY; 
    PSID pSid = NULL; 
    DWORD dwAclSize = 0;
    PACL pDacl = NULL;  
    SECURITY_DESCRIPTOR SecDesc; 
    int nIndex = 0;

    if (ERROR_SUCCESS != ::RegOpenKeyExW(hKey, pwszKey,  
        0, WRITE_DAC, &hTheKey))
        goto Exit0;

    if(FALSE == ::AllocateAndInitializeSid(&SecIA, 1,
        SECURITY_INTERACTIVE_RID, 0, 0, 0, 0, 0, 0, 0, &pSid))
        goto Exit0;

    dwAclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + ::GetLengthSid(pSid);

    pDacl = (PACL) new BYTE[dwAclSize];

    if (TRUE == ::InitializeAcl(pDacl, dwAclSize, ACL_REVISION))  

    {  

        if (TRUE == ::AddAccessAllowedAce(pDacl, ACL_REVISION, KEY_ALL_ACCESS,  

            pSid))  

        {   
            if (TRUE == ::InitializeSecurityDescriptor(&SecDesc,  

                SECURITY_DESCRIPTOR_REVISION))  

            {  

                if (TRUE == ::SetSecurityDescriptorDacl(&SecDesc, TRUE, pDacl, FALSE))  

                {  

                    if(ERROR_SUCCESS != ::RegSetKeySecurity(hTheKey,  

                        (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, &SecDesc))
                        goto Exit0;
                }
            }
        }
    }

    if (ERROR_SUCCESS != ::RegOpenKeyExW(hKey, pwszKey,  
        0, KEY_ALL_ACCESS, &hOpenKey))
        goto Exit0;

    while(KAERegEnumKey(hOpenKey, nIndex, wszSubSubKey, MAX_STRING_SIZE))
    {
        if(!KAGRegUpKeySecurity(hOpenKey, wszSubSubKey))
            goto Exit0;

        nIndex ++;
    }

    nRet =  TRUE;
Exit0:
    if(hTheKey)
    {
        KAERegCloseKey(hTheKey);
        hTheKey = NULL;
    }

    if(hOpenKey)
    {
        KAERegCloseKey(hOpenKey);
        hOpenKey = NULL;
    }

    if(pDacl)
    {
        delete[] (BYTE*) pDacl;
        pDacl = NULL;
    }
    return nRet;
}

unsigned int KAERegGetKeySecurity(
                                  HKEY hKey,
                                  SECURITY_INFORMATION SecurityInformation,
                                  PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                  LPDWORD lpcbSecurityDescriptor
                                  )
{
    if(ERROR_SUCCESS == ::RegGetKeySecurity(hKey, SecurityInformation, 
        pSecurityDescriptor, lpcbSecurityDescriptor))
        return TRUE;

    else
        return FALSE;

}

unsigned int KAERegSetKeySecurity(
                                  HKEY hKey,
                                  SECURITY_INFORMATION SecurityInformation,
                                  PSECURITY_DESCRIPTOR pSecurityDescriptor
                                  )
{
    if(ERROR_SUCCESS == ::RegSetKeySecurity(hKey, SecurityInformation,
        pSecurityDescriptor))
        return TRUE;

    else
        return FALSE;
}

unsigned int KAERegCopyKey(HKEY hKey, wchar_t *pszFromKeys, wchar_t *pszToKeys)
{
    unsigned int nRet = FALSE;
    HKEY hSubKey = NULL, hNewSubKey = NULL;
    wchar_t     achKey[MAX_STRING_SIZE] = L"";   // buffer for subkey name
    DWORD    cbName = MAX_STRING_SIZE;                   // size of name string 
    wchar_t     achClass[MAX_STRING_SIZE] = L"";  // buffer for class name 
    DWORD    cchClassName = MAX_STRING_SIZE;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey = 0;              // longest subkey size 
    DWORD    cchMaxClass = 0;              // longest class string 
    DWORD    cValues = 0;              // number of values for key 
    DWORD    cchMaxValue = 0;          // longest value name 
    DWORD    cbMaxValueData = 0;       // longest value data 
    DWORD    cbSecurityDescriptor = 0; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
    DWORD i = 0, retCode = 0; 

    wchar_t  achValueName[MAX_STRING_SIZE] = L"";
    BYTE achValue[MAX_STRING_SIZE] = {0}; 
    DWORD cchValueName = MAX_STRING_SIZE, cchValue = MAX_STRING_SIZE; 
    DWORD cchValueType = 0;
    wchar_t achSubKeys[MAX_STRING_SIZE] = L"", achNewSubKeys[MAX_STRING_SIZE] = L"";


    if(!KAERegOpenKey(hKey, pszFromKeys, &hSubKey))
        goto Exit0;
    if (!hSubKey)
    {
        goto Exit0;
    }
    if(!KAERegCreateKey(hKey, pszToKeys, &hNewSubKey))
        goto Exit0;
    if (!hNewSubKey)
    {
        goto Exit0;
    }
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hSubKey,                    // key handle 
        CW2T(achClass),                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 

    // Enumerate the subkeys, until RegEnumKeyEx fails.

    // 	printf( "\nNumber of values: %d\n", cValues);

    for (i=0, retCode = ERROR_SUCCESS; i < cValues; i++) 
    { 
        cchValueName = MAX_STRING_SIZE; 
        cchValue = MAX_STRING_SIZE;
        achValueName[0] = L'\0'; 
        retCode = RegEnumValue(hSubKey, i, 
            CW2T(achValueName), 
            &cchValueName, 
            NULL, 
            &cchValueType,
            (BYTE*)achValue,
            &cchValue);

        if (retCode == ERROR_SUCCESS ) 
        { 
            retCode = RegSetValueEx(
                hNewSubKey, 
                CW2T(achValueName),
                NULL,
                cchValueType,
                (BYTE*)achValue,
                cchValue
                );

            // 			switch(cchValueType)
            // 			{
            // 			case REG_BINARY:
            // 			case REG_DWORD:
            // 				printf("(%d) %s = %d\n", i+1, achValueName, *(DWORD*)achValue);
            // 				break;
            // 			case REG_SZ:
            // 			case REG_EXPAND_SZ:
            // 			case REG_MULTI_SZ:
            // 				printf("(%d) %s = %s\n", i+1, achValueName, achValue);
            // 				break;


            // 			}
        } 
    }    

    // 	printf( "Number of subkeys: %d\n", cSubKeys);

    for (i = 0; i < cSubKeys; i++) 
    { 
        cbName = MAX_STRING_SIZE;
        retCode = RegEnumKeyEx(hSubKey, i,
            CW2T(achKey), 
            &cbName, 
            NULL, 
            NULL, 
            NULL, 
            &ftLastWriteTime); 
        if (retCode == ERROR_SUCCESS) 
        {
            // 			printf( "(%d) %s\n", i + 1, achKey);

            wcscpy(achNewSubKeys, pszToKeys);

            wcscat(achNewSubKeys , L"\\");
            wcscat(achNewSubKeys, achKey);

            wcscpy(achSubKeys, pszFromKeys);

            wcscat(achSubKeys , L"\\");
            wcscat(achSubKeys, achKey);
            if(!KAERegCopyKey(hKey, achSubKeys, achNewSubKeys))
                goto Exit0;


        }

    }

    nRet = TRUE;
Exit0:
    if(hSubKey)
    {
        KAERegCloseKey(hSubKey);
        hSubKey = NULL;
    }
    if(hNewSubKey)
    {
        KAERegCloseKey(hNewSubKey);
        hNewSubKey = NULL;
    }
    return nRet;
} 

int GetRealClsidFile(wchar_t *pwszClsid, wchar_t *pwszFileName, wchar_t *pwszTreatAsClsid, int nControl)
{
    int nRet = FALSE;
    wchar_t wszKey[MAX_STRING_SIZE] = L"";
    int nControlNow = nControl;
    HKEY hKey = NULL;
    wchar_t wszData[MAX_STRING_SIZE] = L"";
    LONG lSize = MAX_STRING_SIZE;
    wchar_t wszFileName[MAX_STRING_SIZE] = L"";
    wchar_t wszTreatAsClsid[MAX_STRING_SIZE] = L"";

    if(nControlNow > 4)
        goto Exit0;

    nControlNow ++;

    wcscpy(wszKey, L"CLSID\\");
    wcscat(wszKey, pwszClsid);

    if(TRUE != KAERegOpenKey(HKEY_CLASSES_ROOT, wszKey, &hKey, KEY_READ))
        goto Exit0;

    if(TRUE == KAERegQueryValue(hKey, L"TreatAs", wszData, &lSize))
    {
        if(wcscmp(wszData, L"") == 0)
            goto Exit0;

        if(TRUE == GetRealClsidFile(wszData, wszFileName, wszTreatAsClsid, nControlNow))
        {
            wcscpy(pwszFileName, wszFileName);
            wcscpy(pwszTreatAsClsid, wszTreatAsClsid);
            goto Exit1;
        }
    }

    memset(wszData, 0, sizeof(wchar_t) * MAX_STRING_SIZE);
    lSize = MAX_STRING_SIZE;
    if(TRUE == KAERegQueryValue(hKey, L"InprocServer32", wszData, &lSize))
    {
        if(wcscmp(wszData, L"") == 0)
            goto Exit0;
        wcscpy(pwszFileName, wszData);
        if(nControlNow > 1)
            wcscpy(pwszTreatAsClsid, pwszClsid);
        else
            wcscpy(pwszTreatAsClsid, L"");

        goto Exit1;

    }

    memset(wszData, 0, sizeof(wchar_t) * MAX_STRING_SIZE);
    lSize = MAX_STRING_SIZE;
    if(TRUE == KAERegQueryValue(hKey, L"LocalServer32", wszData, &lSize))
    {
        if(wcscmp(wszData, L"") == 0)
            goto Exit0;
        wcscpy(pwszFileName, wszData);
        if(nControlNow > 1)
            wcscpy(pwszTreatAsClsid, pwszClsid);
        else
            wcscpy(pwszTreatAsClsid, L"");

        goto Exit1;

    }

    goto Exit0;

Exit1:
    nRet = TRUE;
Exit0:
    if(hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }
    return nRet;
}

BOOL SetRegKeyPrivilege(LPWSTR lpRegKey, DWORD AccessPermissions, ACCESS_MODE AccessMode)
{
    PACL  pNewAcl  = NULL;
    PACL pNewAclSys = NULL;
    BOOL  bRet     = FALSE;
    LONG  lRet     = 0;
    DWORD dwLen    = MAX_PATH;
    PACL  pOrgAcl = NULL;
    PACL pSysAcl = NULL;
    PSECURITY_DESCRIPTOR ppSecurityDescriptor = NULL;

    EXPLICIT_ACCESS ea;
    TCHAR szUserName[MAX_PATH];

    if (NULL == lpRegKey)
        goto Exit0;

    if (!GetUserName(szUserName, &dwLen))
        goto Exit0;

    lRet = GetNamedSecurityInfo( CW2T(lpRegKey), 
        SE_REGISTRY_KEY,
        DACL_SECURITY_INFORMATION,
        NULL, NULL, &pOrgAcl, NULL, &ppSecurityDescriptor);

    if (ERROR_SUCCESS != lRet)
        goto Exit0;

    RtlZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    BuildExplicitAccessWithName(&ea, 
        szUserName, 
        AccessPermissions, 
        AccessMode, 
        SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    lRet = SetEntriesInAcl(1, &ea, pOrgAcl, &pNewAcl);
    if (ERROR_SUCCESS != lRet)
        goto Exit0;
    /*
    lRet = SetNamedSecurityInfo(lpRegKey, 
    SE_REGISTRY_KEY, 
    DACL_SECURITY_INFORMATION, 
    NULL, NULL, 0, NULL);
    */
    lRet = SetNamedSecurityInfo( CW2T(lpRegKey), 
        SE_REGISTRY_KEY, 
        DACL_SECURITY_INFORMATION, 
        NULL, NULL, pNewAcl, NULL);

    if (ERROR_SUCCESS != lRet)
        goto Exit0;

    _tcscpy_s(szUserName, MAX_PATH, _T("SYSTEM"));
    lRet = GetNamedSecurityInfo( CW2T(lpRegKey), 
        SE_REGISTRY_KEY,
        DACL_SECURITY_INFORMATION,
        NULL, NULL, &pSysAcl, NULL, &ppSecurityDescriptor);

    if (ERROR_SUCCESS != lRet)
        goto Exit0;

    RtlZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    BuildExplicitAccessWithName(&ea, 
        szUserName, 
        AccessPermissions, 
        AccessMode, 
        SUB_CONTAINERS_AND_OBJECTS_INHERIT);

    lRet = SetEntriesInAcl(1, &ea, pSysAcl, &pNewAclSys);
    if (ERROR_SUCCESS != lRet)
        goto Exit0;
    /*
    lRet = SetNamedSecurityInfo(lpRegKey, 
    SE_REGISTRY_KEY, 
    DACL_SECURITY_INFORMATION, 
    NULL, NULL, 0, NULL);
    */
    lRet = SetNamedSecurityInfo( CW2T(lpRegKey), 
        SE_REGISTRY_KEY, 
        DACL_SECURITY_INFORMATION, 
        NULL, NULL, pNewAclSys, NULL);

    if (ERROR_SUCCESS != lRet)
        goto Exit0;


    bRet = TRUE;

Exit0:

    if (NULL != pNewAcl)
        LocalFree(pNewAcl);
    if (NULL != pNewAclSys)
        LocalFree(pNewAclSys);

    return bRet;
}

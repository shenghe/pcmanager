#pragma once

#include <ShlObj.h>
#include <atlcoll.h>
#include <atlstr.h>

// #include <miniutil/bkenvdir.h>
//
//     CAtlArray<CString> arrPath;
// 
//     BkEnvDir::Get(CSIDL_APPDATA, arrPath);

#define SHELL_FOLDER_KEY_NAME   L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"

class BkEnvDir
{
public:
    BkEnvDir()
    {
    }

    ~BkEnvDir()
    {
    }

    void Init()
    {
        _FillCSidlMap();
    }

    BOOL Get(DWORD dwCSIDL, CAtlArray<CString> &arrPathRet)
    {
        arrPathRet.RemoveAll();

        const CAtlMap<DWORD, __CsidlKey>::CPair *pPair = m_mapCsidl.Lookup(dwCSIDL);
        if (!pPair)
            return FALSE;

        if (pPair->m_value.bCommon)
        {
            CString strPath;

            BOOL bRet = _GetPath(HKEY_LOCAL_MACHINE, pPair->m_value.lpszValueName, strPath);
            if (!bRet)
                return FALSE;

            arrPathRet.Add(strPath);
        }
        else
        {
            CRegKey reg(HKEY_USERS);

            LONG lRet = ERROR_SUCCESS;
            DWORD dwIndex = 0, dwLen = 0, dwMaxLen = 0;
            CString strSID;

            lRet = ::RegQueryInfoKey(reg, NULL, NULL, NULL, NULL, &dwMaxLen, NULL, NULL, NULL, NULL, NULL, NULL);

            while (TRUE)
            {
                dwLen = dwMaxLen;
                lRet = reg.EnumKey(dwIndex, strSID.GetBuffer(dwLen + 1), &dwLen, NULL);
                strSID.ReleaseBuffer(dwLen);
                if (ERROR_NO_MORE_ITEMS == lRet)
                    break;

                if (ERROR_SUCCESS == lRet)
                {
                    CString strPath;
                    CRegKey subkey;

                    subkey.Open(reg, strSID, KEY_READ);

                    if (NULL != subkey.m_hKey)
                    {
                        BOOL bRet = _GetPath(subkey, pPair->m_value.lpszValueName, strPath);

                        if ( bRet && ( FALSE == strPath.IsEmpty()) )
                        {
                            arrPathRet.Add(strPath);
                        }

                        subkey.Close();
                    }
                }
                
                dwIndex ++;
            }
        }

        return TRUE;
    }

protected:

    struct __CsidlKey
    {
        __CsidlKey(LPCTSTR _lpszValueName = NULL, BOOL _bCommon = FALSE)
            : lpszValueName(_lpszValueName)
            , bCommon(_bCommon)
        {
        }
        LPCTSTR lpszValueName;
        BOOL    bCommon;
    };

    CAtlMap<DWORD, __CsidlKey> m_mapCsidl;

    void _FillCSidlMap()
    {
        m_mapCsidl[CSIDL_COMMON_ADMINTOOLS]         = __CsidlKey(L"Common Administrative Tools", TRUE);
        m_mapCsidl[CSIDL_COMMON_APPDATA]            = __CsidlKey(L"Common AppData", TRUE);
        m_mapCsidl[CSIDL_COMMON_DESKTOPDIRECTORY]   = __CsidlKey(L"Common Desktop", TRUE);
        m_mapCsidl[CSIDL_COMMON_DOCUMENTS]          = __CsidlKey(L"Common Documents", TRUE);
        m_mapCsidl[CSIDL_COMMON_PROGRAMS]           = __CsidlKey(L"Common Programs", TRUE);
        m_mapCsidl[CSIDL_COMMON_STARTMENU]          = __CsidlKey(L"Common Start Menu", TRUE);
        m_mapCsidl[CSIDL_COMMON_STARTUP]            = __CsidlKey(L"Common Startup", TRUE);
        m_mapCsidl[CSIDL_ADMINTOOLS]                = __CsidlKey(L"Common Templates", TRUE);
        m_mapCsidl[CSIDL_COMMON_MUSIC]              = __CsidlKey(L"CommonMusic", TRUE);
        m_mapCsidl[CSIDL_COMMON_PICTURES]           = __CsidlKey(L"CommonPictures", TRUE);
        m_mapCsidl[CSIDL_COMMON_VIDEO]              = __CsidlKey(L"CommonVideo", TRUE);

        // Not used in win7
        m_mapCsidl[CSIDL_COMMON_FAVORITES]          = __CsidlKey(L"Common Favorites", TRUE);

        m_mapCsidl[CSIDL_APPDATA]                   = __CsidlKey(L"AppData", FALSE);
        m_mapCsidl[CSIDL_INTERNET_CACHE]            = __CsidlKey(L"Cache", FALSE);
        m_mapCsidl[CSIDL_COOKIES]                   = __CsidlKey(L"Cookies", FALSE);
        m_mapCsidl[CSIDL_DESKTOPDIRECTORY]          = __CsidlKey(L"Desktop", FALSE);
        m_mapCsidl[CSIDL_FAVORITES]                 = __CsidlKey(L"Favorites", FALSE);
        m_mapCsidl[CSIDL_FONTS]                     = __CsidlKey(L"Fonts", FALSE);
        m_mapCsidl[CSIDL_HISTORY]                   = __CsidlKey(L"History", FALSE);
        m_mapCsidl[CSIDL_LOCAL_APPDATA]             = __CsidlKey(L"Local AppData", FALSE);
        m_mapCsidl[CSIDL_PERSONAL]                  = __CsidlKey(L"Personal", FALSE);
        m_mapCsidl[CSIDL_PROGRAMS]                  = __CsidlKey(L"Programs", FALSE);
        m_mapCsidl[CSIDL_RECENT]                    = __CsidlKey(L"Recent", FALSE);
        m_mapCsidl[CSIDL_SENDTO]                    = __CsidlKey(L"SendTo", FALSE);
        m_mapCsidl[CSIDL_STARTMENU]                 = __CsidlKey(L"Start Menu", FALSE);
        m_mapCsidl[CSIDL_STARTUP]                   = __CsidlKey(L"Startup", FALSE);
        m_mapCsidl[CSIDL_TEMPLATES]                 = __CsidlKey(L"Templates", FALSE);
        m_mapCsidl[CSIDL_CDBURN_AREA]               = __CsidlKey(L"CD Burning", FALSE);
        m_mapCsidl[CSIDL_MYMUSIC]                   = __CsidlKey(L"My Music", FALSE);
        m_mapCsidl[CSIDL_MYPICTURES]                = __CsidlKey(L"My Pictures", FALSE);
        m_mapCsidl[CSIDL_MYVIDEO]                   = __CsidlKey(L"My Video", FALSE);
        m_mapCsidl[CSIDL_NETHOOD]                   = __CsidlKey(L"NetHood", FALSE);
        m_mapCsidl[CSIDL_PRINTHOOD]                 = __CsidlKey(L"PrintHood", FALSE);

        // Not used in win7
        m_mapCsidl[CSIDL_PROFILE]                   = __CsidlKey(L"Local Settings", FALSE);
    }


    BOOL _GetPath(HKEY hRootKey, LPCWSTR lpszValueName, CString& strPathRet)
    {
        CRegKey reg;

        LONG lRet = reg.Open(hRootKey, SHELL_FOLDER_KEY_NAME, KEY_READ);
        if (ERROR_SUCCESS != lRet)
            return FALSE;

        DWORD dwLen = 0;

        reg.QueryStringValue(lpszValueName, NULL, &dwLen);
        lRet = reg.QueryStringValue(lpszValueName, strPathRet.GetBuffer(dwLen + 1), &dwLen);
        strPathRet.ReleaseBuffer(dwLen);
        if (ERROR_SUCCESS != lRet)
            return FALSE;

        return TRUE;
    }
};
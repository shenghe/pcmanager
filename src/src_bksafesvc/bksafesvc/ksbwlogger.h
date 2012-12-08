/**
* @file    ksbwlogger.h
* @brief   ...
* @author  bbcallen
* @date    2010-08-28 16:26
*/

#ifndef KSBWLOGGER_H
#define KSBWLOGGER_H

#include "ksbwdetect2\wrappers\scom\ksdll.h"
#include "ksbwdetect2\iksbwlogupload.h"
#include "ksbwdetect2\ksbwiiddef.h"
#include "ksbwdetect2\ksbwlogtypedef.h"

#define BKMOD_NAME_SUBDIR_KSE   L"kse"
#define BKMOD_NAME_KSBWDET2     L"ksbwdet2.dll"

#define enumLogType_BK_CLOUD__RESCAN_DELAY  (enumKSBWLogType)(enumLogType_BK_CLOUD_DATA + 1)

class CKsbwLogger
{
public:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    static CKsbwLogger& Instance()
    {
        static CKsbwLogger s_Instance;
        return s_Instance;
    }

    HRESULT Init()
    {
        CObjGuard Guard(m_ObjLock);

        if (!m_hMod_ksbwdet2.GetModuleHandle())
        {
            CWinPath path;
            path.GetModuleFileName((HMODULE)&__ImageBase);
            path.RemoveFileSpec();
            path.Append(BKMOD_NAME_SUBDIR_KSE);
            path.Append(BKMOD_NAME_KSBWDET2);

            HRESULT hr = m_hMod_ksbwdet2.Open(path.m_strPath);
            if (FAILED(hr))
            {
                return hr;
            }
        }

        if (!m_spiKsbwLogger)
        {
            HRESULT hr = m_hMod_ksbwdet2.GetClassObject(
                CLSID_KSBWLogUpload,
                IID_IKSBWLogUpload,
                (void**)&m_spiKsbwLogger);
            if (FAILED(hr))
                return hr;

            hr = m_spiKsbwLogger->Init(NULL);
            if(FAILED(hr))
            {
                m_spiKsbwLogger.Release();
                return hr;
            }
        }


        return S_OK;
    }

    HRESULT Uninit()
    {
        CObjGuard Guard(m_ObjLock);

        if (m_spiKsbwLogger)
        {
            m_spiKsbwLogger->UnInit(NULL);
            m_spiKsbwLogger.Release();
        }

        m_hMod_ksbwdet2.Release();

        return S_OK;
    }

    HRESULT SendLog(enumKSBWLogType emLogType, const char* pszLogInfo)
    {
        CObjGuard Guard(m_ObjLock);

        if (!m_spiKsbwLogger || !pszLogInfo)
            return E_POINTER;

        return m_spiKsbwLogger->SendLog(enumLogType_BK_CLOUD_DATA, pszLogInfo);
    }


    HRESULT SendRescanDelay(
        LPCWSTR lpszMD5,
        DWORD   dwLevel,
        DWORD   dwDelay)
    {
        CString strFormat;
        strFormat.Format(
            L"bkhash=%s&bklevel=%lu&bkdelay=%lu",
            lpszMD5,
            dwLevel,
            dwDelay);

        return SendLog(enumLogType_BK_CLOUD__RESCAN_DELAY, (LPCSTR)CW2A(strFormat, CP_UTF8));
    }


protected:

    CObjLock    m_ObjLock;
    KSDll       m_hMod_ksbwdet2;
    CComPtr<IKSBWLogUpload> m_spiKsbwLogger;
};

#endif//KSBWLOGGER_H
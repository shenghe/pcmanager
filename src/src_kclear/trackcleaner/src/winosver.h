/**
* @file    winosver.h
* @brief   ...
* @author  bbcallen
* @date    2009-04-09  11:53
*/

#ifndef WINOSVER_H
#define WINOSVER_H

//#include "winmodbase.h"

//NS_WINMOD_BEGIN

#define WINMOD_UNKNOWN_OS   0x00000000
#define WINMOD_XP_SP0       0x05010000
#define WINMOD_XP_SP1       0x05010100
#define WINMOD_XP_SP2       0x05010200
#define WINMOD_XP_SP3       0x05010300
#define WINMOD_2K3_SP0      0x05020000
#define WINMOD_2K3_SP1      0x05020100
#define WINMOD_2K3_SP2      0x05020200
#define WINMOD_VISTA_SP0    0x06000000
#define WINMOD_VISTA_SP1    0x06000100
#define WINMOD_VISTA_SP2    0x06000200
#define WINMOD_WIN7_SP0     0x06010000

class CWinOSVer
{
public:

    enum {
        em_OS_MajorVer_Vista    = 6,
        em_OS_MajorVer_Win2k3   = 5,
        em_OS_MajorVer_WinXP    = 5,

        em_OS_MinorVer_Win2k3   = 2,
        em_OS_MinorVer_WinXP    = 1,
    };

    static DWORD GetOSAndSP()
    {
        OSVERSIONINFOEX osver;
        memset(&osver, 0, sizeof(OSVERSIONINFOEX));
        osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        BOOL br = GetVersionEx((OSVERSIONINFO*) &osver);
        if (!br)
            return 0;

        
        DWORD dwOSAndSP = 0;
        dwOSAndSP |= (LOBYTE(osver.dwMajorVersion)      << 24);
        dwOSAndSP |= (LOBYTE(osver.dwMinorVersion)      << 16);
        dwOSAndSP |= (LOBYTE(osver.wServicePackMajor)   << 8);
        return dwOSAndSP;
    }

    static BOOL IsX86()
    {
        SYSTEM_INFO sysInfo;
        memset(&sysInfo, 0, sizeof(sysInfo));
        GetSystemInfo(&sysInfo);

        if (PROCESSOR_ARCHITECTURE_INTEL == sysInfo.wProcessorArchitecture)
            return TRUE;

        return FALSE;
    }

    static BOOL IsVista()
    {
        return 0 == CompareMajor(em_OS_MajorVer_Vista);
    }

    static BOOL IsVistaOrLater()
    {
        return 0 <= CompareMajor(em_OS_MajorVer_Vista);
    }

    static BOOL IsWinXPOrLater()
    {
        return 0 <= CompareVersion(em_OS_MajorVer_WinXP, em_OS_MinorVer_WinXP);
    }

    static BOOL IsWin2k3()
    {
        return 0 == CompareVersion(em_OS_MajorVer_Win2k3, em_OS_MinorVer_Win2k3);
    }


    /**
    * @retval   >0  current OS is greater than compared version
    * @retval   <0  current OS is less than compared version
    * @retval   0   current OS is equal to compared version
    */
    static int CompareVersion(DWORD dwMajorVer, DWORD dwMinorVer)
    {
        OSVERSIONINFO osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        ::GetVersionEx(&osInfo);

        if (osInfo.dwMajorVersion > dwMajorVer)
        {
            return 1;
        }
        else if (osInfo.dwMajorVersion < dwMajorVer)
        {
            return -1;
        }

        return osInfo.dwMinorVersion - dwMinorVer;
    }


    static int CompareMajor(DWORD dwMajorVer)
    {
        OSVERSIONINFO osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        ::GetVersionEx(&osInfo);

        return osInfo.dwMajorVersion - dwMajorVer;
    }
};

//NS_WINMOD_END

#endif//WINOSVER_H
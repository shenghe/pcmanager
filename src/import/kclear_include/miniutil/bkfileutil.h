//////////////////////////////////////////////////////////////////////////
//   File Name: bkfileutil.h
// Description: File Utilities
//     Creator: Zhang Xiaoxuan
//     Version: 2009.6.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlfile.h>

#pragma warning(push)
#pragma warning(disable: 4819)
#include <atlsecurity.h>
#pragma warning(pop)

class CBkFileUtil
{
public:
    static BOOL CreateFullPath(LPCTSTR lpszFileName)
    {
        BOOL bRet = FALSE;
        CString strPath;
        int nPos = 3;
        DWORD dwFileAttrib = 0;
        CString strFileName = lpszFileName;

        while (TRUE)
        {
            nPos = strFileName.Find(_T('\\'), nPos) + 1;
            if (0 == nPos)
            {
                bRet = TRUE;
                break;
            }

            strPath = strFileName.Left(nPos);
            dwFileAttrib = ::GetFileAttributes(strPath);
            if (INVALID_FILE_ATTRIBUTES == dwFileAttrib)
            {
                bRet = ::CreateDirectory(strPath, NULL);
                if (!bRet)
                    break;

                /*bRet = */FixFilePrivilege(strPath);
//                 if (!bRet)
//                     break;
            }
        }

        return bRet;
    }

    static BOOL DenyExecute(LPCTSTR lpszFileName)
    {
        BOOL bRet = FALSE;
        CDacl dacl;

        bRet = ::AtlGetDacl(lpszFileName, SE_FILE_OBJECT, &dacl);
        if (!bRet)
            goto Exit0;

        dacl.RemoveAllAces();

        bRet = dacl.AddAllowedAce(Sids::World(), FILE_GENERIC_READ | FILE_GENERIC_WRITE);
        bRet = dacl.AddDeniedAce(Sids::World(), FILE_GENERIC_EXECUTE);
        if (!bRet)
            goto Exit0;

        bRet = ::AtlSetDacl(lpszFileName, SE_FILE_OBJECT, dacl);
        if (!bRet)
            goto Exit0;

Exit0:

        return bRet;
    }

    static BOOL FixFilePrivilege(LPCTSTR lpszFileName)
    {
        if (!_IsVistaOrLater())
            return TRUE;

        BOOL bRet = FALSE;
        CDacl dacl;

        bRet = ::AtlGetDacl(lpszFileName, SE_FILE_OBJECT, &dacl);
        if (!bRet)
            goto Exit0;

        bRet = dacl.AddAllowedAce(Sids::Users(), FILE_GENERIC_READ | FILE_GENERIC_EXECUTE);
        if (!bRet)
            goto Exit0;

        bRet = ::AtlSetDacl(lpszFileName, SE_FILE_OBJECT, dacl);
        if (!bRet)
            goto Exit0;

Exit0:

        return bRet;
    }

    static HRESULT CreateFile(LPCTSTR lpszFileName, CAtlFile& theFile)
    {
        if (!CreateFullPath(lpszFileName))
            return ::AtlHresultFromLastError();

        HRESULT hRet = theFile.Create(lpszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE, 0, CREATE_ALWAYS);
        if (FAILED(hRet))
            return hRet;

        FixFilePrivilege(lpszFileName);
//         if (!FixFilePrivilege(lpszFileName))
//             return BKERR_FROM_LASTERROR();

        return S_OK;
    }

    static HRESULT FileExist(LPCTSTR lpszFileName)
    {
        DWORD dwRet = ::GetFileAttributes(lpszFileName);
        if (INVALID_FILE_ATTRIBUTES == dwRet)
        {
            dwRet = ::GetLastError();
            if (ERROR_FILE_NOT_FOUND == dwRet || ERROR_PATH_NOT_FOUND == dwRet)
                return S_FALSE;

            return ::AtlHresultFromWin32(dwRet);
        }

        return S_OK;
    }

private:

    static BOOL _IsVistaOrLater()
    {
        OSVERSIONINFOEX vi = { sizeof(OSVERSIONINFOEX) };
        ::GetVersionEx((LPOSVERSIONINFO)&vi);

        if (vi.dwMajorVersion >= 6)
            return TRUE;
        else
            return FALSE;
    }
};

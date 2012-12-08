#pragma once

#include "beikesafeminidownloader.h"

/*

www.ijinshan.com/safe/ijinshan_cs.json

{
    "link" : [
        ["《天龙八部》惊现59个木马变种!", "http://www.beike.cn/supesite/html/98/n-98.html"],
        ["贝壳CEO陈睿：木马比想象的狡猾", "http://www.beike.cn/supesite/html/96/n-96.html"]
    ]
}
*/

#define PUSH_DATA_LIST_NODE_NAME  L"link"



struct BkPushADInfo 
{
    CString strText;
    CString strUrl;
};

class CBkPushAD
{
public:
    CBkPushAD(LPCTSTR lpszPushAddr, LPCTSTR lpszLocalPath, HWND hWndNotify, UINT uMsgNotify)
        : m_strPushAddr(lpszPushAddr)
        , m_strLocalPath(lpszLocalPath)
        , m_hWndNotify(hWndNotify)
        , m_uMsgNotify(uMsgNotify)
    {

    }

    static void GetLocal(LPCTSTR lpszLocalPath, CAtlArray<BkPushADInfo> &arrADInfo)
    {
        BkJson::Document doc;

        arrADInfo.RemoveAll();

        BOOL bRet = doc.LoadFile(lpszLocalPath);
        if (!bRet)
            return;

        if (!doc.HasKey(PUSH_DATA_LIST_NODE_NAME))
            return;

        int nCount = doc[PUSH_DATA_LIST_NODE_NAME].Count();

        arrADInfo.SetCount(nCount);

        for (int i = 0; i < nCount; i ++)
        {
            arrADInfo[i].strText = (LPCTSTR)doc[PUSH_DATA_LIST_NODE_NAME][i][0];
            arrADInfo[i].strUrl = (LPCTSTR)doc[PUSH_DATA_LIST_NODE_NAME][i][1];
            arrADInfo[i].strText.Trim();
            arrADInfo[i].strUrl.Trim();
        }
    }

    static void GetNew(LPCTSTR lpszPushAddr, LPCTSTR lpszLocalPath, HWND hWndNotify, UINT uMsgNotify)
    {
        HANDLE hThread = ::CreateThread(NULL, 0, _GetNewADThreadProc, new CBkPushAD(lpszPushAddr, lpszLocalPath, hWndNotify, uMsgNotify), 0, NULL);
        ::CloseHandle(hThread);
    }

protected:

    CString m_strPushAddr;
    CString m_strLocalPath;
    HWND m_hWndNotify;
    UINT m_uMsgNotify;

    static DWORD WINAPI _GetNewADThreadProc(LPVOID pvParam)
    {
        CBkPushAD *pThis = (CBkPushAD *)pvParam;

        if (!pThis)
            return -1;

        CString strPushAddr = pThis->m_strPushAddr;
        CString strLocalPath = pThis->m_strLocalPath;
        HWND hWndNotify = pThis->m_hWndNotify;
        UINT uMsgNotify = pThis->m_uMsgNotify;
        BOOL bSuccess = FALSE;
        CString strFileName;
        ULONGLONG ullFileSize = 0;
        CBkMiniDownloader downloader;
        CBkTemporaryFile tmpFile;
        HRESULT hRet = E_FAIL;
        CStringA strPushContent;
        DWORD dwRead = 0;
        BkJson::Document doc;
		CPath strTmpFile( strLocalPath );
		strTmpFile.RemoveFileSpec();

        delete pThis;
        pThis = NULL;

        hRet = tmpFile.Create( strTmpFile, GENERIC_WRITE | GENERIC_READ);
        if (FAILED(hRet))
            goto Exit0;

        for (int i = 0; i < 3; i ++)
        {
            hRet = downloader.Download(strPushAddr, tmpFile, NULL);
            if (SUCCEEDED(hRet) || E_ABORT == hRet)
                break;
        }
        if (FAILED(hRet))
            goto Exit0;

        hRet = tmpFile.GetSize(ullFileSize);
        if (FAILED(hRet))
            goto Exit0;

        hRet = tmpFile.Seek(3, FILE_BEGIN);
        if (FAILED(hRet))
            goto Exit0;

        hRet = tmpFile.Read(strPushContent.GetBuffer((int)(ullFileSize + 2)), (int)(ullFileSize - 3), dwRead);
        if (FAILED(hRet))
        {
            strPushContent.ReleaseBuffer(0);
            goto Exit0;
        }

        strPushContent.ReleaseBuffer((int)ullFileSize - 3);

        {
            if (!doc.LoadString(CA2T(strPushContent, CP_UTF8)))
                goto Exit0;
        }

        if (!doc.HasKey(PUSH_DATA_LIST_NODE_NAME))
            goto Exit0;

        tmpFile.Close(strLocalPath);

        bSuccess = TRUE;

Exit0:

        if (hWndNotify && ::IsWindow(hWndNotify))
        {
            ::PostMessage(hWndNotify, uMsgNotify, bSuccess, 0);
        }

        return 0;
    }
private:
};
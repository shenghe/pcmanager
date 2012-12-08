#include "stdafx.h"
#include "szyaml.h"
#include "szstr.h"

#ifdef _SZYAML_DEBUG_TRACE
#include "kconsole.h"
#endif

//////////////////////////////////////////////////////////////////////////
// SZYamlNode

SZYamlNode& SZYamlNode::operator=(SZYamlNode node)
{
    m_strKey = node.m_strKey;
    m_strValue = node.m_strValue;

    return *this;
}

SZYamlNode& SZYamlNode::operator=(LPCTSTR lpszValue)
{
    m_strValue = lpszValue;

    return *this;
}

SZYamlNode& SZYamlNode::operator=(int nValue)
{
    m_strValue.Format(_T("%d"), nValue);

    return *this;
}

SZYamlNode::operator LPCTSTR()
{
    return String();
}

LPCTSTR SZYamlNode::String()
{
    return m_strValue;
}

SZYamlNode::operator int()
{
    return Int();
}
int SZYamlNode::Int()
{
    return _szttoi(m_strValue);
}

LPCTSTR SZYamlNode::Key()
{
    return m_strKey;
}

void SZYamlNode::SetKey(LPCTSTR lpszName)
{
    if (lpszName)
        m_strKey = lpszName;
}

void SZYamlNode::SetValue(LPCTSTR lpszValue)
{
    if (lpszValue)
        m_strValue = lpszValue;
}

void SZYamlNode::SetValue(int nValue)
{
    m_strValue.Format(_T("%d"), nValue);
}

//////////////////////////////////////////////////////////////////////////
// SZYamlHandle

SZYamlHandle::SZYamlHandle()
    : m_posItem(NULL)
    , m_pDoc(NULL)
    , m_bFitKey(FALSE)
{
}

SZYamlHandle& SZYamlHandle::operator=(const SZYamlHandle& handle)
{
    m_pDoc = handle.m_pDoc;
    m_posItem = handle.m_posItem;
    m_bFitKey = handle.m_bFitKey;

    return *this;
}

SZYamlHandle& SZYamlHandle::operator=(LPCTSTR lpszValue)
{
    if (Valid() && lpszValue)
    {
        SZYamlNode& theNode = m_pDoc->_GetNode(m_posItem);

        theNode = lpszValue;
    }

    return *this;
}

SZYamlHandle& SZYamlHandle::operator=(int nValue)
{
    if (Valid())
    {
        SZYamlNode& theNode = m_pDoc->_GetNode(m_posItem);

        theNode = nValue;
    }

    return *this;
}

SZYamlHandle SZYamlHandle::operator[](LPCTSTR lpszName)
{
    return Find(lpszName);
}

SZYamlHandle SZYamlHandle::Find(LPCTSTR lpszChildName, LPCTSTR lpszConditionSubKey/* = NULL*/, LPCTSTR lpszConditionSubKeyValue/* = NULL*/)
{
    SZYamlHandle yhRet;

    if (Valid() && lpszChildName)
    {
        SZYamlHandle yhTmp;

        yhTmp._SetPosition(m_pDoc, m_pDoc->_GetChildPos(m_posItem, lpszChildName));

        if (lpszChildName)
            yhTmp.m_bFitKey = TRUE;

        while (yhTmp.Valid() && lpszConditionSubKey && lpszConditionSubKeyValue)
        {
            if (0 == _tcscmp(yhTmp[lpszConditionSubKey].String(), lpszConditionSubKeyValue))
                break;

            yhTmp = yhTmp.Next();
        }

        yhRet = yhTmp;
    }

    return yhRet;
}

LPCTSTR SZYamlHandle::Key()
{
    if (Valid())
        return m_pDoc->_GetNode(m_posItem).Key();
    else
        return NULL;
}

SZYamlHandle SZYamlHandle::Next()
{
    SZYamlHandle tmpHandle;

    if (Valid())
    {
        tmpHandle._SetPosition(m_pDoc, m_pDoc->_GetNextPos(m_posItem, m_bFitKey));
        tmpHandle.m_bFitKey = m_bFitKey;
    }

    return tmpHandle;
}

BOOL SZYamlHandle::Valid()
{
    return (NULL != m_pDoc && NULL != m_posItem);
}

LPCTSTR SZYamlHandle::String()
{
    if (Valid())
        return m_pDoc->_GetNode(m_posItem).String();
    else
        return NULL;
}

int SZYamlHandle::Int()
{
    if (Valid())
        return m_pDoc->_GetNode(m_posItem).Int();
    else
        return NULL;
}

SZYamlHandle SZYamlHandle::AddChild(LPCTSTR lpszChildName, LPCTSTR lpszValue/* = NULL*/)
{
    SZYamlHandle tmpHandle;

    if (Valid() && lpszChildName)
    {
        SZYamlNode newNode;

        newNode.SetKey(lpszChildName);
        newNode.SetValue(lpszValue);

        tmpHandle._SetPosition(m_pDoc, m_pDoc->_AddChild(m_posItem, newNode));
    }

    return tmpHandle;
}

void SZYamlHandle::RemoveChild(LPCTSTR lpszChildName)
{
    if (Valid() && lpszChildName)
    {
        POSITION pos = m_pDoc->_GetChildPos(m_posItem, lpszChildName);

        while (pos)
        {
            m_pDoc->_RemoveChild(pos);
            pos = m_pDoc->_GetChildPos(m_posItem, lpszChildName);
        }
    }
}

void SZYamlHandle::SetInvalid()
{
    m_posItem   = NULL;
    m_pDoc      = NULL;
    m_bFitKey   = FALSE;
}

void SZYamlHandle::_SetPosition(SZYamlDocument* pDoc, const POSITION pos)
{
    m_pDoc = pDoc;
    m_posItem = pos;
}

//////////////////////////////////////////////////////////////////////////
// SZYamlDocument

BOOL SZYamlDocument::LoadFile(LPCTSTR lpszFileName)
{
    BOOL bResult = FALSE, bRet = FALSE;
    DWORD dwFileSize = 0;
    LPSTR lpszText = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    CString strText;

    if (!lpszFileName)
        goto Exit0;

    hFile = ::CreateFile(
        lpszFileName, GENERIC_READ, FILE_SHARE_READ, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
        );
    if (INVALID_HANDLE_VALUE == hFile)
        goto Exit0;

    dwFileSize = ::GetFileSize(hFile, NULL);
    lpszText = new char[dwFileSize + 1];
    if (!lpszText)
        goto Exit0;

    bRet = ::ReadFile(hFile, lpszText, dwFileSize, &dwFileSize, NULL);
    if (!bRet)
        goto Exit0;

    lpszText[dwFileSize] = '\0';

    _DecodeText(lpszText, strText);

#ifdef _SZYAML_DEBUG_TRACE
    kconsole::printf(_T("%s\r\n"), strText);
#endif

    bResult = Load(strText);

Exit0:

    if (lpszText)
    {
        delete[] lpszText;
        lpszText = NULL;
    }

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return bResult;
}

BOOL _CreateFullPath(LPCTSTR lpszFileName)
{
    BOOL bRet = FALSE;
    CString strPath;
    int nPos = 3;
    DWORD dwFileAttrib = 0;
    CString strFileName = lpszFileName;

    if (!lpszFileName)
        return FALSE;

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
                bRet;
        }
    }

    return bRet;
}

BOOL SZYamlDocument::SaveFile(LPCTSTR lpszFileName, UINT uCodePage, int nDumpIndent)
{
    BOOL bResult = FALSE, bRet = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwWrite = 0;
    CString strText;
    CStringA strWrite;

    if (!lpszFileName)
        goto Exit0;

    bRet = _CreateFullPath(lpszFileName);
    if (!bRet)
        goto Exit0;

    hFile = ::CreateFile(
        lpszFileName, GENERIC_WRITE, FILE_SHARE_READ, 
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
        );
    if (INVALID_HANDLE_VALUE == hFile)
        goto Exit0;

    Dump(strText, nDumpIndent);
    if (strText.IsEmpty())
        goto Exit0;

    _EncodeText(strText, strWrite, uCodePage);

    bRet = ::WriteFile(hFile, strWrite, strWrite.GetLength(), &dwWrite, NULL);
    if (!bRet)
        goto Exit0;

    ::SetEndOfFile(hFile);

    bResult = TRUE;
Exit0:

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return bResult;
}

struct _BOMFlagType
{
    int     nBOMSize;
    LPCSTR  pszBOM;
    BOOL    bIsMutibyte;
    UINT    uMultiByteCodePage;
};
static _BOMFlagType BOMFlags[] = {
    { 3, "\xef\xbb\xbf", TRUE,  CP_UTF8 }, 
    { 2, "\xff\xfe",     FALSE, CP_ACP }, 
    { 0, NULL,           TRUE,  CP_THREAD_ACP }, 
};
static int nBOMFlagCount = sizeof(BOMFlags) / sizeof(_BOMFlagType);

void SZYamlDocument::_EncodeText(LPCTSTR lpszSrc, CStringA &strTextOut, UINT uCodePage)
{
    int nBOMType;

    for (nBOMType = 0; nBOMType < nBOMFlagCount - 1; nBOMType ++)
    {
        if (BOMFlags[nBOMType].uMultiByteCodePage == uCodePage)
            break;
    }

    strTextOut = BOMFlags[nBOMType].pszBOM;
    if (BOMFlags[nBOMType].bIsMutibyte)
        strTextOut += CT2A(lpszSrc, uCodePage);
    else
        strTextOut += CT2W((LPCWSTR)lpszSrc, uCodePage);
}

void SZYamlDocument::_DecodeText(LPCSTR lpszSrc, CString &strTextOut)
{
    int nBOMType;

    for (nBOMType = 0; nBOMType < nBOMFlagCount - 1; nBOMType ++)
    {
        if (0 == memcmp(BOMFlags[nBOMType].pszBOM, lpszSrc, BOMFlags[nBOMType].nBOMSize))
            break;
    }

    lpszSrc += BOMFlags[nBOMType].nBOMSize;

    if (BOMFlags[nBOMType].bIsMutibyte)
        strTextOut = CA2T(lpszSrc, BOMFlags[nBOMType].uMultiByteCodePage);
    else
        strTextOut = CW2T((LPCWSTR)lpszSrc, BOMFlags[nBOMType].uMultiByteCodePage);
}

BOOL SZYamlDocument::Load(LPCTSTR lpszText)
{
    BOOL bResult = FALSE;
    BOOL bNotFinish = TRUE;
    CString strText = lpszText, strLine, strKey, strValue;
    int nThisLinePos = 0, nNextLinePos = 0, nColonPos = 0;
    int nIndent = 0, nLineNum = 1;
    struct _KeyStackNode 
    {
        _KeyStackNode()
            : nIndent(0)
            , nChildIndent(0)
            , bIsList(FALSE)
        {
        }
        int nIndent;
        int nChildIndent;
        CString strKey;
        BOOL bIsList;
    };
    CAtlList<_KeyStackNode> listKeyStack;
    BOOL bNewChild = FALSE, bIsListItem = FALSE;

    m_YamlItemList.RemoveAll();

    if (!lpszText)
        goto Exit0;

    while (bNotFinish)
    {
        nNextLinePos = strText.Find(_T('\n'), nThisLinePos);
        if (-1 == nNextLinePos)
        {
            bNotFinish = FALSE;
            strLine = strText.Mid(nThisLinePos);
        }
        else
            strLine = strText.Mid(nThisLinePos, nNextLinePos - nThisLinePos);

        nIndent = 0;

        // Get indent
        while (_T(' ') == strLine[nIndent])
            nIndent ++;

        nColonPos = strLine.Find(_T(':'));

        strKey = strLine.Left(nColonPos).Trim();
        strValue = strLine.Mid(nColonPos + 1).Trim();

        if (_T('-') == strKey[0])
        {
            int nIndentMore = 1;

            while (_T(' ') == strKey[nIndentMore])
                nIndentMore ++;

            nIndent += nIndentMore;

            strKey = strKey.Mid(nIndentMore);

            bIsListItem = TRUE;
        }
        else
            bIsListItem = FALSE;

        if (bNewChild)
        {
            _KeyStackNode &LastKey = listKeyStack.GetTail();
            LastKey.nChildIndent = nIndent;

            if (bIsListItem)
                LastKey.bIsList = TRUE;

#ifdef _SZYAML_DEBUG_TRACE
            { // Trace Key Stack
                kconsole::printf(_T("  "));

                for (POSITION pos = listKeyStack.GetHeadPosition(); pos != NULL; listKeyStack.GetNext(pos))
                {
                    _KeyStackNode key = listKeyStack.GetAt(pos);
                    kconsole::printf(_T("(%d, '%s', %d)"), key.nChildIndent, key.strKey, key.bIsList);
                }

                kconsole::printf(_T("\r\n"));
            }
#endif
        }

        strLine = strLine.Mid(nIndent);

        if (strLine.IsEmpty())
            continue;

        while (!listKeyStack.IsEmpty())
        {
            _KeyStackNode &LastKey = listKeyStack.GetTail();
            if (LastKey.nChildIndent == nIndent)
                break;

#ifdef _SZYAML_DEBUG_TRACE
            kconsole::printf(_T(" ### (%d, %d)\r\n"), LastKey.nChildIndent, nIndent);
#endif

            if (LastKey.nChildIndent < nIndent)
            {
#ifdef _SZYAML_DEBUG_TRACE
                kconsole::printf(_T(" * ERROR: Line %d, Indent Error\r\n"), nLineNum);
#endif
                goto Exit0;
            }

            listKeyStack.RemoveTail();

#ifdef _SZYAML_DEBUG_TRACE
            { // Trace Key Stack
                kconsole::printf(_T("  "));

                for (POSITION pos = listKeyStack.GetHeadPosition(); pos != NULL; listKeyStack.GetNext(pos))
                {
                    _KeyStackNode key = listKeyStack.GetAt(pos);
                    kconsole::printf(_T("(%d, '%s', %d)"), key.nChildIndent, key.strKey, key.bIsList);
                }

                kconsole::printf(_T("\r\n"));
            }
#endif
        }

        if (bIsListItem && !bNewChild)
        {
            _KeyStackNode &LastKey = listKeyStack.GetTail();
            SZYamlNode newNode;

            newNode.SetKey(LastKey.strKey);
            _AppendNode((int)listKeyStack.GetCount() - 1, newNode);
        }

        if (1 < strValue.GetLength() && strValue[0] == strValue[strValue.GetLength() - 1]
            && (_T('\'') == strValue[0] || _T('\"') == strValue[0]))
            strValue = strValue.Mid(1, strValue.GetLength() - 2);

        if (strValue.IsEmpty())
        {
            _KeyStackNode &NewKey = listKeyStack.GetAt(listKeyStack.AddTail());

            NewKey.nIndent = nIndent;
            NewKey.strKey = strKey;

            bNewChild = TRUE;
        }
        else
            bNewChild = FALSE;

        {
            SZYamlNode newNode;

            newNode.SetKey(strKey);
            newNode.SetValue(strValue);
            _AppendNode((int)listKeyStack.GetCount() - (bNewChild ? 1 : 0), newNode);
        }

        nThisLinePos = nNextLinePos + 1;
        ++ nLineNum;
    }

    SZYamlHandle::_SetPosition(this, m_YamlItemList.GetHeadPosition());

    bResult = TRUE;

Exit0:

    return bResult;
}

BOOL SZYamlDocument::LoadBuffer(LPVOID lpBuffer, DWORD dwSize)
{
    BOOL bResult = FALSE;
    CString strText;
    LPSTR lpszText = NULL;

    if (!lpBuffer || dwSize == 0)
        goto Exit0;

    lpszText = new char[dwSize + 1];
    if (!lpszText)
        goto Exit0;

    memcpy(lpszText, lpBuffer, dwSize);
    lpszText[dwSize] = '\0';

    _DecodeText(lpszText, strText);

    bResult = Load(strText);

Exit0:

    if (lpszText)
    {
        delete[] lpszText;
        lpszText = NULL;
    }

    return bResult;
}

SZYamlNode& SZYamlDocument::GetRootNode()
{
    return m_YamlItemList.GetHead().node;
}

void SZYamlDocument::GetRootHandle(SZYamlHandle& rootHandle)
{
    rootHandle._SetPosition(this, m_YamlItemList.GetHeadPosition());
}

SZYamlNode& SZYamlDocument::_GetNode(POSITION pos)
{
    return m_YamlItemList.GetAt(pos).node;
}

POSITION SZYamlDocument::_GetChildPos(POSITION pos, LPCTSTR lpszChildName/* = NULL*/)
{
    _YamlItem &item = m_YamlItemList.GetAt(pos);

    while (TRUE)
    {
        m_YamlItemList.GetNext(pos);
        if (!pos)
            break;

        _YamlItem &itemChild = m_YamlItemList.GetAt(pos);

        if (item.nIndent + 1 < itemChild.nIndent)
            continue;

        if (item.nIndent + 1 > itemChild.nIndent)
        {
            pos = NULL;
            break;
        }

        if (!lpszChildName)
            break;

        if (0 == _tcscmp(itemChild.node.Key(), lpszChildName))
            break;
    }

    return pos;
}

POSITION SZYamlDocument::_GetNextPos(POSITION pos, BOOL bFitName)
{
    _YamlItem &item = m_YamlItemList.GetAt(pos);

    while (TRUE)
    {
        m_YamlItemList.GetNext(pos);
        if (!pos)
            break;

        _YamlItem &itemNext = m_YamlItemList.GetAt(pos);

        if (item.nIndent > itemNext.nIndent)
        {
            pos = NULL;
            break;
        }

        if (item.nIndent < itemNext.nIndent)
            continue;

        if (!bFitName)
            break;

        if (0 == _tcscmp(item.node.Key(), itemNext.node.Key()))
            break;
    }

    return pos;
}

POSITION SZYamlDocument::_InsertNodeAfter(POSITION posAfter, int nIndent, SZYamlNode& newNode)
{
    _YamlItem newItem;

    newItem.nIndent = nIndent;
    newItem.node = newNode;

    return m_YamlItemList.InsertAfter(posAfter, newItem);
}

POSITION SZYamlDocument::_InsertNodeBefore(POSITION posBefore, int nIndent, SZYamlNode& newNode)
{
    _YamlItem newItem;

    newItem.nIndent = nIndent;
    newItem.node = newNode;

    return m_YamlItemList.InsertBefore(posBefore, newItem);
}

POSITION SZYamlDocument::_AppendNode(int nIndent, SZYamlNode& newNode)
{
    //kconsole::printf(_T("%s%s: '%s'\r\n"),  CString(_T(' '), nIndent * 4), newNode.Key(), newNode.String());

    return _InsertNodeAfter(m_YamlItemList.GetTailPosition(), nIndent, newNode);
}

POSITION SZYamlDocument::_AddChild(POSITION pos, SZYamlNode& newNode)
{
    POSITION posInsert = NULL;
    _YamlItem &item = m_YamlItemList.GetAt(pos);
    int nChildIndent = item.nIndent + 1;

    while (TRUE)
    {
        m_YamlItemList.GetNext(pos);
        if (!pos)
            break;

        _YamlItem &itemChild = m_YamlItemList.GetAt(pos);

        if (nChildIndent < itemChild.nIndent)
            continue;

        if (nChildIndent > itemChild.nIndent)
        {
            posInsert = pos;
            break;
        }

        if (0 == _tcscmp(itemChild.node.Key(), newNode.Key()))
        {
            if (itemChild.node.String()[0] != _T('\0') || newNode.String()[0] != _T('\0'))
                return NULL;

            posInsert = pos;
        }
        else
        {
            if (posInsert)
            {
                posInsert = pos;
                break;
            }
        }
    }

    if (posInsert)
        return _InsertNodeBefore(posInsert, nChildIndent, newNode);
    else
        return _AppendNode(nChildIndent, newNode);
}

void SZYamlDocument::_RemoveChild(POSITION pos)
{
    if (!pos)
        return;

    _YamlItem &item = m_YamlItemList.GetAt(pos);
    int nIndent = item.nIndent;


    while (TRUE)
    {
        POSITION next = pos;
        m_YamlItemList.GetNext(next);
        if (!next)
            break;

        _YamlItem &childItem = m_YamlItemList.GetAt(next);
        if (childItem.nIndent <= nIndent)
            break;

        m_YamlItemList.RemoveAt(next);
    }
    
    m_YamlItemList.RemoveAt(pos);
}

void SZYamlDocument::Dump(CString& strText, int nDumpIndent/* = SZYAML_DUMP_INDENT*/)
{
    POSITION pos = m_YamlItemList.GetHeadPosition();
    struct _KeyStackNode 
    {
        _KeyStackNode()
            : nIndent(0)
            , nChildIndent(0)
            , bIsList(FALSE)
        {
        }
        int nIndent;
        int nChildIndent;
        CString strKey;
        BOOL bIsList;
    };
    CAtlList<_KeyStackNode> listKeyStack;
    int nLastIndent = -1;
    BOOL bListItem = FALSE, bPrintThisLine = TRUE;
    CString strLine;

    strText = _T("");

    while (pos)
    {
        _YamlItem &item = m_YamlItemList.GetAt(pos);

        if (listKeyStack.IsEmpty())
            bListItem = FALSE;
        else
            bListItem = listKeyStack.GetTail().bIsList;

        if (item.nIndent == nLastIndent + 1)
        {
            _KeyStackNode &newkey = listKeyStack.GetAt(listKeyStack.AddTail());
            POSITION posNext = _GetNextPos(pos, TRUE);

            newkey.nIndent = item.nIndent;
            newkey.strKey = item.node.Key();
            newkey.bIsList = (posNext != NULL);
        }
        else if (item.nIndent == nLastIndent)
        {
            _KeyStackNode &newkey = listKeyStack.GetTail();
            POSITION posNext = _GetNextPos(pos, TRUE);

            newkey.nIndent = item.nIndent;
            newkey.strKey = item.node.Key();
            newkey.bIsList = (posNext != NULL);
        }
        else if (item.nIndent < nLastIndent)
        {
            for (nLastIndent -= item.nIndent; nLastIndent > 0; -- nLastIndent)
                listKeyStack.RemoveTail();

            if (listKeyStack.IsEmpty())
                bPrintThisLine = TRUE;
            else
            {
                _KeyStackNode &newkey = listKeyStack.GetTail();

                newkey.nIndent = item.nIndent;
                if (newkey.strKey != item.node.Key())
                {
                    newkey.strKey = item.node.Key();
                    POSITION posNext = _GetNextPos(pos, TRUE);
                    newkey.bIsList = (posNext != NULL);
                }
                else
                    bPrintThisLine = !newkey.bIsList;
            }
        }

#ifdef _SZYAML_DEBUG_TRACE
        { // Trace Key Stack
            kconsole::printf(_T("  "));

            for (POSITION pos = listKeyStack.GetHeadPosition(); pos != NULL; listKeyStack.GetNext(pos))
            {
                _KeyStackNode key = listKeyStack.GetAt(pos);
                kconsole::settextcolor(TRUE, TRUE, TRUE, FALSE);
                kconsole::printf(_T("(%d, '%s', %d)"), key.nIndent, key.strKey, key.bIsList);
                kconsole::settextcolor(TRUE, TRUE, TRUE, TRUE);
            }

            kconsole::printf(_T("\r\n"));
        }
#endif

        // 这里对不确定长度的%s不使用Format,是因为MIN_CRT的格式化输出限制长度为1024,见atlstr.h
        // by bbcallen 2009-07-02
        if (bPrintThisLine)
        {
            if (bListItem)
            {
                strLine.Format(
                    _T("%s-%s"),  
                    CString(_T(' '), (item.nIndent - 1) * nDumpIndent),  
                    CString(_T(' '), nDumpIndent - 1)
                    );
                strLine.Append(item.node.Key());
                strLine.Append(_T(": "));
                strLine.Append(item.node.String());
                strLine.Append(_T("\r\n"));
            }
            else
            {
                strLine.Format(
                    _T("%s"),  
                    CString(_T(' '), item.nIndent * nDumpIndent)
                    );

                strLine.Append(item.node.Key());
                strLine.Append(_T(": "));
                strLine.Append(item.node.String());
                strLine.Append(_T("\r\n"));
            }
#ifdef _SZYAML_DEBUG_TRACE
            kconsole::printf(strLine);
#endif
            strText += strLine;
        }
        else
            bPrintThisLine = TRUE;

        nLastIndent = item.nIndent;

        m_YamlItemList.GetNext(pos);
    }
}

void SZYamlDocument::SetEmpty()
{
    m_YamlItemList.RemoveAll();

    SetInvalid();
}
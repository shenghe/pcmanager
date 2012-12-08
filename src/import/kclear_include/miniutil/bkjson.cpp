//////////////////////////////////////////////////////////////////////////
//   File Name: bkjson.cpp
// Description: Beike JSON DOM-like parser
//     Creator: Zhang Xiaoxuan
//////////////////////////////////////////////////////////////////////////

#include "bkjson.h"

namespace BkJson
{

//////////////////////////////////////////////////////////////////////////

//#define MAX_ITEM_DUMP_INLINE 5
#define MAX_ITEM_DUMP_INLINE 1

// Minicrt do not support %*s, so use this
void _AppendSpace(CString& strString, int nSpaceCount)
{
    int nLen = strString.GetLength();
    LPWSTR lpszBuffer = strString.GetBuffer(nLen + nSpaceCount + 1) + nLen;
    for (int i = 0; i < nSpaceCount; i ++)
    {
        lpszBuffer[i] = L' ';
    }

    strString.ReleaseBuffer(nLen + nSpaceCount);
}

Handle::Handle(Value *pNodeParent, Value *pNode)
    : m_pNodeParent(pNodeParent)
    , m_pNode(pNode)
{
}
Handle::~Handle()
{
}

Handle::operator BoolType()
{
    if (m_pNode && Value::TypeBool == m_pNode->GetType())
        return ((ValueBool *)m_pNode)->GetValue();

    return False;
}

Handle::operator LPCTSTR()
{
    if (m_pNode && Value::TypeString == m_pNode->GetType())
        return ((ValueString *)m_pNode)->GetValue();

    return NULL;
}

Handle::operator LONG()
{
    if (m_pNode && Value::TypeLong == m_pNode->GetType())
        return ((ValueLong *)m_pNode)->GetValue();

    return 0;
}

Handle::operator ULONG()
{
    return (ULONG)(LONG)(*this);
}

BOOL Handle::HasKey(LPCTSTR lpszKey)
{
    if (m_pNode && Value::TypeObject == m_pNode->GetType())
        return ((ValueObject *)m_pNode)->HasChild(lpszKey);

    return FALSE;
}

BOOL Handle::RemoveKey(LPCTSTR lpszKey)
{
    if (m_pNode && Value::TypeObject == m_pNode->GetType())
        return ((ValueObject *)m_pNode)->RemoveChild(lpszKey);

    return FALSE;
}

Handle Handle::operator[](LPCTSTR lpszKey)
{
    if (!m_pNode)
        return _GetNullHandle();

    if (Value::TypeObject != m_pNode->GetType())
    {
        if (!m_pNodeParent)
            return _GetNullHandle();

        ValueObject *pNewValue = NULL;

        if (Value::TypeObject == m_pNodeParent->GetType())
        {
            pNewValue = (ValueObject *)((ValueObject *)m_pNodeParent)->_ChangeChildType(m_pNode, Value::TypeObject);
        }
        else if (Value::TypeArray == m_pNodeParent->GetType())
        {
            pNewValue = (ValueObject *)((ValueArray *)m_pNodeParent)->_ChangeItemType(m_pNode, Value::TypeObject);
        }

        m_pNode = pNewValue;
    }

    return ((ValueObject *)m_pNode)->GetChild(lpszKey);
}

Handle Handle::operator[](int nIndex)
{
    if (!m_pNode)
        return _GetNullHandle();

    if (Value::TypeArray != m_pNode->GetType())
    {
        if (!m_pNodeParent)
            return _GetNullHandle();

        ValueArray *pNewValue = NULL;

        if (Value::TypeObject == m_pNodeParent->GetType())
        {
            pNewValue = (ValueArray *)((ValueObject *)m_pNodeParent)->_ChangeChildType(m_pNode, Value::TypeArray);
        }
        else if (Value::TypeArray == m_pNodeParent->GetType())
        {
            pNewValue = (ValueArray *)((ValueArray *)m_pNodeParent)->_ChangeItemType(m_pNode, Value::TypeArray);
        }

        m_pNode = pNewValue;
    }

    return ((ValueArray *)m_pNode)->GetItem(nIndex);
}

Handle& Handle::operator=(NullType nullValue)
{
    UNREFERENCED_PARAMETER(nullValue);

    if (m_pNodeParent && m_pNode)
    {
        ValueNull *pNewValue = NULL;

        if (Value::TypeObject == m_pNodeParent->GetType())
        {
            pNewValue = (ValueNull *)((ValueObject *)m_pNodeParent)->_ChangeChildType(m_pNode, Value::TypeNull);
        }
        else if (Value::TypeArray == m_pNodeParent->GetType())
        {
            pNewValue = (ValueNull *)((ValueArray *)m_pNodeParent)->_ChangeItemType(m_pNode, Value::TypeNull);
        }

        m_pNode = pNewValue;
    }

    return *this;
}

Handle& Handle::operator=(BoolType bValue)
{
    if (m_pNodeParent && m_pNode)
    {
        ValueBool *pNewValue = NULL;

        if (Value::TypeObject == m_pNodeParent->GetType())
        {
            pNewValue = (ValueBool *)((ValueObject *)m_pNodeParent)->_ChangeChildType(m_pNode, Value::TypeBool);
        }
        else if (Value::TypeArray == m_pNodeParent->GetType())
        {
            pNewValue = (ValueBool *)((ValueArray *)m_pNodeParent)->_ChangeItemType(m_pNode, Value::TypeBool);
        }

        if (pNewValue)
            pNewValue->SetValue(bValue);

        m_pNode = pNewValue;
    }

    return *this;
}

Handle& Handle::operator=(LPCSTR szValue)
{
    return (*this = CA2W(szValue, CP_UTF8));
}

Handle& Handle::operator=(LPCWSTR szValue)
{
    if (m_pNodeParent && m_pNode)
    {
        ValueString *pNewValue = NULL;

        if (Value::TypeObject == m_pNodeParent->GetType())
        {
            pNewValue = (ValueString *)((ValueObject *)m_pNodeParent)->_ChangeChildType(m_pNode, Value::TypeString);
        }
        else if (Value::TypeArray == m_pNodeParent->GetType())
        {
            pNewValue = (ValueString *)((ValueArray *)m_pNodeParent)->_ChangeItemType(m_pNode, Value::TypeString);
        }

        if (pNewValue)
            pNewValue->SetValue(szValue);

        m_pNode = pNewValue;
    }

    return *this;
}

Handle& Handle::operator=(LONG lValue)
{
    if (m_pNodeParent && m_pNode)
    {
        ValueLong *pNewValue = NULL;

        if (Value::TypeObject == m_pNodeParent->GetType())
        {
            pNewValue = (ValueLong *)((ValueObject *)m_pNodeParent)->_ChangeChildType(m_pNode, Value::TypeLong);
        }
        else if (Value::TypeArray == m_pNodeParent->GetType())
        {
            pNewValue = (ValueLong *)((ValueArray *)m_pNodeParent)->_ChangeItemType(m_pNode, Value::TypeLong);
        }

        if (pNewValue)
            pNewValue->SetValue(lValue);

        m_pNode = pNewValue;
    }

    return *this;
}

int Handle::Count()
{
    if (!m_pNode)
        return 0;

    if (Value::TypeArray == m_pNode->GetType())
        return ((ValueArray *)m_pNode)->Count();
    else if (Value::TypeObject == m_pNode->GetType())
        return ((ValueObject *)m_pNode)->Count();

    return 0;
}

void Handle::SetCount(int nNewCount)
{
    if (m_pNode && Value::TypeArray == m_pNode->GetType())
        ((ValueArray *)m_pNode)->SetCount(nNewCount);
}

BOOL Handle::RemoveItem(int nIndex)
{
    if (m_pNode && Value::TypeArray == m_pNode->GetType())
        return ((ValueArray *)m_pNode)->RemoveItem(nIndex);

    return FALSE;
}

void Handle::RemoveAllChildren()
{
    if (m_pNode && Value::TypeObject == m_pNode->GetType())
        return ((ValueObject *)m_pNode)->RemoveAllChildren();
}

BOOL Handle::IsValid()
{
    return (NULL != m_pNode);
}

BOOL Handle::IsNull()
{
    if (!m_pNode)
        return TRUE;

    return (m_pNode->GetType() == Value::TypeNull);
}

int Handle::GetType()
{
    if (!m_pNode)
        return Value::TypeError;
    
    return m_pNode->GetType();
}

CString Handle::GetName()
{
    if (!m_pNode)
        return L"";

    return m_pNode->GetName();
}

POSITION Handle::FindFirstChild()
{
    if (m_pNode && Value::TypeObject == m_pNode->GetType())
        return ((ValueObject *)m_pNode)->FindFirstChild();

    return NULL;
}

POSITION Handle::FindNextChild(POSITION pos)
{
    if (m_pNode && Value::TypeObject == m_pNode->GetType())
        return ((ValueObject *)m_pNode)->FindNextChild(pos);

    return NULL;
}

Handle Handle::GetAt(POSITION pos)
{
    if (!m_pNode)
        return _GetNullHandle();

    if (Value::TypeObject != m_pNode->GetType())
        return _GetNullHandle();

    return ((ValueObject *)m_pNode)->GetAt(pos);
}

Handle& Handle::_GetNullHandle()
{
    static Handle s_nullHandle(NULL, NULL);
    return s_nullHandle;
}

Value::Value(Value *pNodeParent, POSITION posInParent, int nNodeType)
    : Handle(pNodeParent, this)
    , m_nType(nNodeType)
    , m_posInParent(posInParent)
{
}

Value::~Value()
{
}

int Value::GetType()
{
    return m_nType;
}

int Value::GetParentType()
{
    if (NULL == m_pNodeParent)
        return TypeNull;

    return m_pNodeParent->m_nType;
}

CString Value::GetName()
{
    if (NULL == m_pNodeParent)
        return L"";

    if (TypeObject != m_pNodeParent->m_nType)
        return L"";

    if (NULL == m_pNodeParent->m_posInParent)
        return L"";

    ValueObject* m_pParentObject = (ValueObject*)m_pNodeParent;
    return m_pParentObject->m_mapChilds.GetKeyAt(m_posInParent);
}

BOOL Value::_Parse(ParseContext &context)
{
    UNREFERENCED_PARAMETER(context);

    return FALSE;
}

BOOL Value::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    UNREFERENCED_PARAMETER(strDump);
    UNREFERENCED_PARAMETER(bMultiLine);
    UNREFERENCED_PARAMETER(nIndent);
    UNREFERENCED_PARAMETER(nIndentAdd);

    return FALSE;
}

BOOL Value::_IsBlankChar(TCHAR chText)
{
    switch (chText)
    {
    case _T(' '):
    case _T('\r'):
    case _T('\n'):
    case _T('\t'):
        return TRUE;
    default:
        return FALSE;
    }
}

void Value::_SkipBlankChar(ParseContext &context)
{
    int nEnd = context.nStart + context.nLength - 1;

    while (_IsBlankChar(context.lpszText[context.nStart]) && context.nStart <= nEnd)
    {
        ++ context.nStart;
        -- context.nLength;
    }
}

BOOL Value::_ExpectChar(ParseContext &context, TCHAR chExpected)
{
    int nEnd = context.nStart + context.nLength - 1;

    _SkipBlankChar(context);

    if (context.nStart == nEnd || chExpected != context.lpszText[context.nStart])
        return FALSE;

    ++ context.nStart;
    -- context.nLength;

    return TRUE;
}

ULONG Value::_HexToULong(LPCTSTR lpszValue)
{
    LPCTSTR pchValue = lpszValue;
    ULONG ulValue = 0;
    int nSize = 4;

    while (*pchValue && nSize != 0)
    {
        ulValue <<= 4;

        if (_T('a') <= *pchValue && _T('f') >= *pchValue)
            ulValue |= (*pchValue - _T('a') + 10);
        else if (_T('A') <= *pchValue && _T('F') >= *pchValue)
            ulValue |= (*pchValue - _T('A') + 10);
        else if (_T('0') <= *pchValue && _T('9') >= *pchValue)
            ulValue |= (*pchValue - _T('0'));
        else
            return 0;

        ++ pchValue;
        -- nSize;
    }

    return ulValue;
}

TCHAR Value::_ControlCharToRealChar(LPCTSTR lpszControlChar)
{
    switch (lpszControlChar[0])
    {
    case _T('\"'):
        return _T('\"');
    case _T('\\'):
        return  _T('\\');
    case _T('/'):
        return  _T('/');
    case _T('b'):
        return  _T('\b');
    case _T('f'):
        return  _T('\f');
    case _T('n'):
        return  _T('\n');
    case _T('r'):
        return  _T('\r');
    case _T('t'):
        return  _T('\t');
    case _T('u'):
        return (TCHAR)_HexToULong(lpszControlChar + 1);
    default:
        return NULL;
    }
}

BOOL Value::_GetString(ParseContext &context, CString& strRet)
{
    if (!_ExpectChar(context, _T('\"')))
    {
        context.SetError(Error::ErrExpectStringStart);
        return FALSE;
    }

    int nStringStart = context.nStart, nStringLength = context.nLength;
    LPCTSTR lpszStringEnd = context.lpszText + context.nStart;

    strRet.Empty();

    while (_T('\"') != *lpszStringEnd && lpszStringEnd < context.lpszText + nStringStart + nStringLength)
    {
        if (_T('\\') == *lpszStringEnd)
        {
            ++ lpszStringEnd;

            TCHAR chControl = _ControlCharToRealChar(lpszStringEnd);
            if (!chControl)
            {
                context.SetError(Error::ErrUnknownControlChar, (int)(lpszStringEnd - context.lpszText));
                return FALSE;
            }

            if (_T('u') == *lpszStringEnd)
                lpszStringEnd += 4;

            strRet += chControl;
        }
        else
            strRet += *lpszStringEnd;

        ++ lpszStringEnd;
    }

    if (lpszStringEnd == context.lpszText + nStringStart + nStringLength)
    {
        context.SetError(Error::ErrExpectStringEnd, nStringStart + nStringLength);
        return FALSE;
    }

    nStringLength = (int)(lpszStringEnd - context.lpszText) - nStringStart;

    context.nLength -= nStringStart + nStringLength + 1 - context.nStart;
    context.nStart = nStringStart + nStringLength + 1;

    return TRUE;
}

BOOL Value::_GetLong(ParseContext &context, LONG& lRet)
{
    BOOL bMinus = FALSE;
    lRet = 0;

    if (_T('-') == context.lpszText[context.nStart])
    {
        bMinus = TRUE;

        ++ context.nStart;
        -- context.nLength;

        if (0 == context.nLength)
        {
            context.SetError(Error::ErrExpectNumber);
            return FALSE;
        }
    }

    while (_T('0') <= context.lpszText[context.nStart] && _T('9') >= context.lpszText[context.nStart] && context.nLength > 0)
    {
        lRet = lRet * 10 + (context.lpszText[context.nStart] - _T('0'));

        ++ context.nStart;
        -- context.nLength;
    }

    if (bMinus)
        lRet = -lRet;

    return TRUE;
}

Value* Value::_ParseValue(ParseContext &context)
{
    LPCTSTR lpszValue = NULL;
    int nType = TypeError;

    _SkipBlankChar(context);

    lpszValue = context.lpszText + context.nStart;

    switch (*lpszValue)
    {
    case _T('\"'): // String

        nType = TypeString;

        break;

    case _T('{'): // Object

        nType = TypeObject;

        break;

    case _T('['): // Array

        nType = TypeArray;

        break;

    case _T('t'): // Bool
    case _T('f'):

        nType = TypeBool;

        break;

    case _T('n'): // Null

        nType = TypeNull;

        break;

    case _T('-'): // Number
    case _T('0'):
    case _T('1'):
    case _T('2'):
    case _T('3'):
    case _T('4'):
    case _T('5'):
    case _T('6'):
    case _T('7'):
    case _T('8'):
    case _T('9'):

        nType = TypeLong;

        break;

    default:
        break;
    }

    return _CreateValueByType(nType);
}

Value* Value::_CreateValueByType(int nType)
{
    Value *pNewValue = NULL;

    switch (nType)
    {
    case TypeNull:
        pNewValue = new ValueNull(this, NULL);
    	break;
    case TypeBool:
        pNewValue = new ValueBool(this, NULL);
        break;
    case TypeString:
        pNewValue = new ValueString(this, NULL);
        break;
    case TypeLong:
        pNewValue = new ValueLong(this, NULL);
        break;
    case TypeArray:
        pNewValue = new ValueArray(this, NULL);
        break;
    case TypeObject:
        pNewValue = new ValueObject(this, NULL);
        break;
    default:
        break;
    }

    return pNewValue;
}

BOOL Value::_DumpString(CString &strDump, LPCTSTR lpszString, int nLength)
{
    CString strString;

    while (*lpszString)
    {
        switch (*lpszString)
        {
        case _T('\"'):
            strString += _T("\\\"");
        	break;
        case _T('\\'):
            strString += _T("\\\\");
            break;
        case _T('/'):
            strString += _T("\\/");
            break;
        case _T('\b'):
            strString += _T("\\b");
            break;
        case _T('\f'):
            strString += _T("\\f");
            break;
        case _T('\n'):
            strString += _T("\\n");
            break;
        case _T('\r'):
            strString += _T("\\r");
            break;
        case _T('\t'):
            strString += _T("\\t");
            break;
        default:
            // added by zhangrui 2009-09-01
            if ((*lpszString >  0   && *lpszString <  32) ||
                (*lpszString >= 127 && *lpszString <= 255))
            {
                // 转义不可打印的ASCII字符
                strString.AppendFormat(_T("\\u%04x"), (unsigned int)*lpszString);
            }
            else
            {
                strString += *lpszString;
            }
        }

        lpszString ++;
    }

    strDump.AppendChar(_T('"'));
    strDump.Append(strString);
    strDump.AppendChar(_T('"'));

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// BkJson Null Node Definition

BOOL ValueNull::_Parse(ParseContext &context)
{
    if (_tcsncmp(context.lpszText + context.nStart, _T("null"), 4) != 0)
        return FALSE;

    context.nStart += 4;
    context.nLength -= 4;

    return TRUE;
}

BOOL ValueNull::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    UNREFERENCED_PARAMETER(bMultiLine);
    UNREFERENCED_PARAMETER(nIndent);
    UNREFERENCED_PARAMETER(nIndentAdd);

    strDump += _T("null");

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// BkJson Bool Node Definition

BoolType ValueBool::GetValue()
{
    return m_bValue;
}

void ValueBool::SetValue(BoolType bValue)
{
    m_bValue = bValue;
}

BOOL ValueBool::_Parse(ParseContext &context)
{
    if (_tcsncmp(context.lpszText + context.nStart, _T("true"), 4) == 0)
    {
        m_bValue = True;

        context.nStart += 4;
        context.nLength -= 4;

        return TRUE;
    }
    else if (_tcsncmp(context.lpszText + context.nStart, _T("false"), 5) == 0)
    {
        m_bValue = False;

        context.nStart += 5;
        context.nLength -= 5;

        return TRUE;
    }

    return FALSE;
}

BOOL ValueBool::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    UNREFERENCED_PARAMETER(bMultiLine);
    UNREFERENCED_PARAMETER(nIndent);
    UNREFERENCED_PARAMETER(nIndentAdd);

    strDump += (m_bValue == True) ? _T("true") : _T("false");

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// BkJson String Node Definition

BOOL ValueString::_Parse(ParseContext &context)
{
    return _GetString(context, m_strValue);
}

BOOL ValueString::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    UNREFERENCED_PARAMETER(bMultiLine);
    UNREFERENCED_PARAMETER(nIndent);
    UNREFERENCED_PARAMETER(nIndentAdd);

    return _DumpString(strDump, m_strValue, m_strValue.GetLength());
}

LPCTSTR ValueString::GetValue()
{
    return m_strValue;
}

void ValueString::SetValue(LPCTSTR szValue)
{
    m_strValue = szValue;
}

//////////////////////////////////////////////////////////////////////////
// BkJson Long Number Node Definition

BOOL ValueLong::_Parse(ParseContext &context)
{
    return _GetLong(context, m_lValue);
}

BOOL ValueLong::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    UNREFERENCED_PARAMETER(bMultiLine);
    UNREFERENCED_PARAMETER(nIndent);
    UNREFERENCED_PARAMETER(nIndentAdd);

    CString strTemp;

    strTemp.Format(_T("%d"), m_lValue);

    strDump += strTemp;

    return TRUE;
}

LONG ValueLong::GetValue()
{
    return m_lValue;
}

void ValueLong::SetValue(LONG lValue)
{
    m_lValue = lValue;
}

//////////////////////////////////////////////////////////////////////////
// BkJson Array Node Definition

ValueArray::~ValueArray()
{
    RemoveAllItem();
}

void ValueArray::RemoveAllItem()
{
    Value *pNodeTemp = NULL;

    for (size_t i = 0; i < m_arrNodes.GetCount(); i ++)
    {
        pNodeTemp = m_arrNodes.GetAt(i);
        m_arrNodes.SetAt(i, NULL);
        delete pNodeTemp;
    }

    m_arrNodes.RemoveAll();
}

BOOL ValueArray::_Parse(ParseContext &context)
{
    BOOL bRet = FALSE;

    if (!_ExpectChar(context, _T('[')))
    {
        context.SetError(Error::ErrExpectArrayStart);
        return FALSE;
    }

    if (_ExpectChar(context, _T(']')))
    {
        return TRUE;
    }

    Value *pNewChildNode = NULL;

    while (TRUE)
    {
        bRet = FALSE;

        pNewChildNode = _ParseValue(context);

        if (pNewChildNode)
        {
            bRet = pNewChildNode->_Parse(context);
            if (bRet)
            {
#pragma warning(push)
#pragma warning(disable: 4312)
                pNewChildNode->m_posInParent = (POSITION)m_arrNodes.GetCount();
#pragma warning(pop)
                m_arrNodes.Add(pNewChildNode);
            }
            else
            {
                delete pNewChildNode;
            }
        }

        if (context.IsError())
            return FALSE;

        if (!bRet)
        {
            context.SetError(Error::ErrUnknownValueType);
            return FALSE;
        }

        _SkipBlankChar(context);

        if (_T(']') == context.lpszText[context.nStart])
        {
            ++ context.nStart;
            -- context.nLength;
            break;
        }

        if (_T(',') != context.lpszText[context.nStart])
        {
            context.SetError(Error::ErrExpectNextKey);
            return FALSE;
        }

        ++ context.nStart;
        -- context.nLength;
    }

    return TRUE;
}

BOOL ValueArray::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    int nItemCount = (int)m_arrNodes.GetCount(), nIndex = 0;
    BOOL bItemSingleLine = _HasGrandSon();
    BOOL bItemNewLine = nItemCount > MAX_ITEM_DUMP_INLINE || bItemSingleLine;

    strDump += _T('[');

    if (bMultiLine && bItemNewLine)
        strDump += _T("\r\n");

    for (nIndex = 0; nIndex < nItemCount; nIndex ++)
    {
        if (bMultiLine && (bItemSingleLine || (bItemNewLine && nIndex % MAX_ITEM_DUMP_INLINE == 0)))
        {
            if (0 != nIndex)
                strDump += _T("\r\n");

            _AppendSpace(strDump, nIndent + nIndentAdd);
//             strDump.AppendFormat(_T("%*s"), nIndent + nIndentAdd, _T(""));
        }

        m_arrNodes[nIndex]->_Dump(strDump, bMultiLine, nIndent + nIndentAdd, nIndentAdd);

        if (nIndex != nItemCount - 1)
        {
            strDump += _T(",");

            if (bMultiLine)
                strDump += _T(' ');
        }
    }

    if (bMultiLine && bItemNewLine)
    {
        strDump += _T("\r\n");
        _AppendSpace(strDump, nIndent);
//         strDump.AppendFormat(_T("\r\n%*s"), nIndent, _T(""));
    }
    
    strDump += _T(']');

    return TRUE;
}

Value* ValueArray::_ChangeItemType(Value *pOldItem, int nNewType)
{
    if (pOldItem->GetType() == nNewType)
        return pOldItem;

    Value *pNodeTemp = NULL, *pNodeNew = NULL;

    for (size_t i = 0; i < m_arrNodes.GetCount(); i ++)
    {
        pNodeTemp = m_arrNodes.GetAt(i);
        if (pNodeTemp == pOldItem)
        {
            pNodeNew = _CreateValueByType(nNewType);
            if (pNodeNew)
            {
                m_arrNodes.SetAt(i, pNodeNew);
#pragma warning(push)
#pragma warning(disable: 4312)
                pNodeNew->m_posInParent = (POSITION)(i);
#pragma warning(pop)
                delete pNodeTemp;
            }

            break;
        }
    }

    return pNodeNew;
}

BOOL ValueArray::_HasGrandSon()
{
    int nItemCount = (int)m_arrNodes.GetCount();

    for (int nIndex = 0; nIndex < nItemCount; nIndex ++)
    {
        int nItemType = m_arrNodes[nIndex]->GetType();
        if (TypeArray == nItemType || TypeObject == nItemType)
            return TRUE;
    }

    return FALSE;
}

int ValueArray::Count()
{
    return (int)m_arrNodes.GetCount();
}

void ValueArray::SetCount(int nNewCount)
{
    int nOldCount = Count();

    if (nOldCount == nNewCount)
        return;

    if (nNewCount < nOldCount)
    {
        for (int i = nNewCount; i < nOldCount; i ++)
        {
            Value *pNode = m_arrNodes.GetAt(i);
            if (pNode)
                delete pNode;
        }
    }

    m_arrNodes.SetCount(nNewCount);

    if (nNewCount > nOldCount)
    {
        for (int i = nOldCount; i < nNewCount; i ++)
        {
#pragma warning(push)
#pragma warning(disable: 4312)
            m_arrNodes[i] = new ValueNull(this, (POSITION)i);
#pragma warning(pop)
        }
    }
}

Handle ValueArray::GetItem(int nIndex)
{
    if (nIndex < 0)
        return _GetNullHandle();

    if (nIndex >= (int)m_arrNodes.GetCount())
        SetCount(nIndex + 1);

    return *m_arrNodes.GetAt(nIndex);
}

BOOL ValueArray::RemoveItem(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_arrNodes.GetCount())
        return FALSE;

    Value *pNodeTemp = m_arrNodes.GetAt(nIndex);

    m_arrNodes.RemoveAt(nIndex);

    delete pNodeTemp;

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// BkJson Object Node Definition

ValueObject::~ValueObject()
{
    RemoveAllChild();
}

int ValueObject::Count()
{
    return (int)m_mapChilds.GetCount();
}

void ValueObject::RemoveAllChild()
{
    POSITION posLast = NULL, pos = m_mapChilds.GetHeadPosition();
    CRBMap<CString, Value *>::CPair *pairRet = NULL;
    Value *pNodeTemp = NULL;

    while (pos)
    {
        posLast = pos;
        pairRet = m_mapChilds.GetNext(pos);
        if (pairRet)
        {
            pNodeTemp = pairRet->m_value;
            m_mapChilds.SetValueAt(posLast, NULL);
            delete pNodeTemp;
        }
    }

    m_mapChilds.RemoveAll();
}

BOOL ValueObject::_Parse(ParseContext &context)
{
    BOOL bRet = FALSE;

    if (!_ExpectChar(context, _T('{')))
    {
        context.SetError(Error::ErrExpectObjectStart);
        return FALSE;
    }

    if (_ExpectChar(context, _T('}')))
    {
        return TRUE;
    }

    Value *pNewChildNode = NULL;
    CString strKey;

    while (TRUE)
    {
        bRet = _GetString(context, strKey);
        if (!bRet)
            return FALSE;

        if (!_ExpectChar(context, _T(':')))
        {
            context.SetError(Error::ErrExpectValue);
            return FALSE;
        }

        bRet = FALSE;

        pNewChildNode = _ParseValue(context);

        if (pNewChildNode)
        {
            bRet = pNewChildNode->_Parse(context);
            if (bRet)
            {
                POSITION posInParent = m_mapChilds.SetAt(strKey, pNewChildNode);
                pNewChildNode->m_posInParent = posInParent;
            }
            else
                delete pNewChildNode;
        }

        if (context.IsError())
            return FALSE;

        if (!bRet)
        {
            context.SetError(Error::ErrUnknownValueType);
            return FALSE;
        }

        _SkipBlankChar(context);

        if (_T('}') == context.lpszText[context.nStart])
        {
            ++ context.nStart;
            -- context.nLength;
            break;
        }

        if (_T(',') != context.lpszText[context.nStart])
        {
            context.SetError(Error::ErrExpectNextKey);
            return FALSE;
        }

        ++ context.nStart;
        -- context.nLength;
    }

    return TRUE;
}

BOOL ValueObject::_Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd)
{
    POSITION posLast = NULL, pos = m_mapChilds.GetHeadPosition();
    CRBMap<CString, Value *>::CPair *pairRet = NULL;
    Value *pNodeChild = NULL;

    strDump += _T('{');

    if (bMultiLine)
        strDump += _T("\r\n");

    while (pos)
    {
        posLast = pos;
        pairRet = m_mapChilds.GetNext(pos);
        if (pairRet)
        {
            if (bMultiLine)
                _AppendSpace(strDump, nIndent + nIndentAdd);
//                 strDump.AppendFormat(_T("%*s"), nIndent + nIndentAdd, _T(""));

            _DumpString(strDump, pairRet->m_key, pairRet->m_key.GetLength());

            if (bMultiLine)
                strDump += _T(" : ");
            else
                strDump += _T(":");

            pNodeChild = pairRet->m_value;
            pNodeChild->_Dump(strDump, bMultiLine, nIndent + nIndentAdd, nIndentAdd);

            if (pos)
                strDump += _T(",");

            if (bMultiLine)
                strDump += _T("\r\n");
        }
    }

    if (bMultiLine)
        _AppendSpace(strDump, nIndent);
//         strDump.AppendFormat(_T("%*s"), nIndent, _T(""));

    strDump += _T('}');

    return TRUE;
}

Value* ValueObject::_ChangeChildType(Value *pOldChild, int nNewType)
{
    if (pOldChild->GetType() == nNewType)
        return pOldChild;

    Value *pNodeNew = NULL;
    POSITION posLast = NULL, pos = m_mapChilds.GetHeadPosition();
    CRBMap<CString, Value *>::CPair *pairRet = NULL;

    while (pos)
    {
        posLast = pos;
        pairRet = m_mapChilds.GetNext(pos);
        if (pairRet->m_value == pOldChild)
        {
            pNodeNew = _CreateValueByType(nNewType);
            if (pNodeNew)
            {
                m_mapChilds.SetValueAt(posLast, pNodeNew);
                pNodeNew->m_posInParent = posLast;
            }

            delete pOldChild;

            break;
        }
    }

    return pNodeNew;
}

BOOL ValueObject::HasChild(LPCTSTR lpszKey)
{
    return m_mapChilds.Lookup(lpszKey) != NULL;
}

Handle ValueObject::GetChild(LPCTSTR lpszKey)
{
    CRBMap<CString, Value *>::CPair *pairRet = m_mapChilds.Lookup(lpszKey);
    if (!pairRet)
        return NewChild(lpszKey);
    else
        return Handle(this, pairRet->m_value);
}

Handle ValueObject::NewChild(LPCTSTR lpszKey)
{
    Value* pNewNode = new ValueNull(this, NULL);

    //m_mapChilds[lpszKey] = pNewNode;
    POSITION posInParent    = m_mapChilds.SetAt(lpszKey, pNewNode);
    pNewNode->m_posInParent = posInParent;

    return *pNewNode;
}

BOOL ValueObject::RemoveChild(LPCTSTR lpszKey)
{
    return ValueObject::m_mapChilds.RemoveKey(lpszKey);
}

POSITION ValueObject::FindFirstChild()
{
    if (m_mapChilds.IsEmpty())
        return NULL;

    return m_mapChilds.GetHeadPosition();
}

POSITION ValueObject::FindNextChild(POSITION pos)
{
    if (NULL == pos)
        return NULL;

    m_mapChilds.GetNext(pos);
    return pos;
}

Handle ValueObject::GetAt(POSITION pos)
{
    if (NULL == pos)
        return Handle();

    return Handle(this, m_mapChilds.GetValueAt(pos));
}

void Document::Clear()
{
    m_rootNode.RemoveAllChild();
}

BOOL Document::LoadString(LPCTSTR lpszText)
{
    CString strTextTemp = lpszText;
    ParseContext context(strTextTemp, strTextTemp.GetLength());

    m_err.Clear();
    m_rootNode.RemoveAllChild();

    BOOL bRet = m_rootNode._Parse(context);

    if (!bRet)
    {
        m_err = context.err;
        m_rootNode.RemoveAllChild();
    }

    return bRet;
}

Error& Document::GetError()
{
    return m_err;
}

CString Document::Dump(BOOL bMultiLine/* = FALSE*/, int nIndent/* = 4*/)
{
    BOOL bRet = FALSE;
    CString strDump;

    bRet = m_rootNode._Dump(strDump, bMultiLine, 0, nIndent);
    if (!bRet)
        return _T("");

    return strDump;
}


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// added by bbcallen 2009-08-25
BOOL Document::LoadFile(LPCTSTR lpszFileName)
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

    bResult = LoadString(strText);

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

BOOL Document::SaveFile(LPCTSTR lpszFileName, UINT uCodePage, BOOL bMultiLine, int nIndent)
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

    strText = Dump(bMultiLine, nIndent);
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

void Document::_EncodeText(LPCTSTR lpszSrc, CStringA &strTextOut, UINT uCodePage)
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

void Document::_DecodeText(LPCSTR lpszSrc, CString &strTextOut)
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


};
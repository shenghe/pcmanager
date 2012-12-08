//////////////////////////////////////////////////////////////////////////
//   File Name: bkjson.h
// Description: Beike JSON DOM-like parser
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.18 - Create
//              2009.5.20 - 1.0 - First Usable Version
//              2009.5.22 - 1.1 - Fix Some Bug
//
// NOT SUPPORT: 1. Float
//              2. \u0000
//              3. Comment
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Usage
//
/* 

    BkJson::Document jsonDoc;

    jsonDoc.LoadString(_T("{\"key\" : \"value\", \"array\" : [1,2,3,4,5]}"));

    LONG lValue = jsonDoc[_T("array")][4];
    // lValue = 5

    CString strValue = jsonDoc[_T("key")];
    // strValue = "value"

    jsonDoc[_T("key2")] = 12345;

    CString strDump;

    strDump = jsonDoc.Dump();
    // strDump = {"key":"value","key2":12345,"array":[1,2,3,4,5]}

    strDump = jsonDoc.Dump(TRUE);
    // strDump = {
    //              "key" : "value", 
    //              "key2" : 12345, 
    //              "array" : [1, 2, 3, 4, 5]
    //           }

    int nCount = jsonDoc[_T("array")].Count();
    // nCount = 5

    jsonDoc[_T("array")][7] = 8;

    nCount = jsonDoc[_T("array")].Count();
    // nCount = 8

    strDump = jsonDoc.Dump();
    // strDump = {"key":"value","key2":12345,"array":[1,2,3,4,5,null,null,8]}

*/

#pragma once

#include <atlstr.h>
#include <atlcoll.h>

namespace BkJson
{

class Handle;
class Value;
class ValueNull;
class ValueBool;
class ValueString;
class ValueLong;
class ValueArray;
class ValueObject;
class Document;

struct NullType
{
};

struct BoolType
{
    BoolType(BOOL b)
        : bValue(b)
    {
    }

    BOOL operator==(BoolType& obj)
    {
        return bValue == obj.bValue;
    }
    BOOL bValue;
};

// Boolean False Definition
__declspec(selectany) BoolType False   = BoolType(FALSE); 
// Boolean True Definition
__declspec(selectany) BoolType True    = BoolType(TRUE);
// Null Definition
__declspec(selectany) NullType Null    = NullType();

//////////////////////////////////////////////////////////////////////////
// BkJson Error Definition

class Error
{
public:

    enum {
        ErrNull         = 0, 
        ErrExpectObjectStart, 
        ErrExpectObjectEnd, 
        ErrExpectStringStart, 
        ErrExpectStringEnd, 
        ErrExpectNumber, 
        ErrExpectValue, 
        ErrExpectNextKey, 
        ErrUnknownControlChar, 
        ErrUnknownValueType, 
        ErrExpectArrayStart, 
        ErrExpectArrayEnd, 
    };

    Error()
        : m_nErrCode(ErrNull)
        , m_nLine(-1)
        , m_nLinePos(-1)
    {
    }

    virtual ~Error()
    {
    }

    void Clear()
    {
        m_nLine         = -1;
        m_nLinePos      = -1;

#ifdef BKJSON_ERROR_HAS_STRING

        m_strErr.Empty();

#endif
    }

    void SetError(int nErrCode, int nLine, int nPos)
    {
        m_nErrCode      = nErrCode;
        m_nLine         = nLine;
        m_nLinePos      = nPos;
    }

    BOOL IsError()
    {
        return (-1 != m_nLinePos);
    }

    int GetLine()
    {
        return m_nLine;
    }

    int GetLinePos()
    {
        return m_nLinePos;
    }

#ifdef BKJSON_ERROR_HAS_STRING

    LPCTSTR GetString()
    {
        return m_strErr;
    }

    void SetString(LPCTSTR lpszError)
    {
        m_strErr = lpszError;
    }

    static LPCTSTR GetErrorStringFromCode(int nErrCode)
    {
        switch (nErrCode)
        {
        case ErrNull:
            return _T("No Error");

        case ErrExpectObjectStart:
            return _T("Expect Object Start ( { )");

        case ErrExpectObjectEnd:
            return _T("Expect Object End ( } )");

        case ErrExpectStringStart:
            return _T("Expect String Start ( \" )");

        case ErrExpectStringEnd:
            return _T("Expect String End ( \" )");

        case ErrExpectNumber:
            return _T("Expect Number ( 0 ~ 9 )");

        case ErrExpectValue:
            return _T("Expect Value ( : )");

        case ErrExpectNextKey:
            return _T("Expect Next Key ( , )");

        case ErrUnknownControlChar:
            return _T("Unknown Control Charactor");

        case ErrUnknownValueType:
            return _T("Unknown Value Type");

        case ErrExpectArrayStart:
            return _T("Expect String Start ( [ )");

        case ErrExpectArrayEnd:
            return _T("Expect String End ( ] )");

        default:

            return _T("Unknown Error");
        }
    }

#endif

protected:

    int m_nErrCode;
    int m_nLine;
    int m_nLinePos;

#ifdef BKJSON_ERROR_HAS_STRING

    CString m_strErr;

#endif

};

//////////////////////////////////////////////////////////////////////////
// BkJson Parser Context Definition

class ParseContext
{
public:
    ParseContext(LPCTSTR lpszJsonText, int nJsonLength)
        : lpszText(lpszJsonText)
        , nStart(0)
        , nLength(nJsonLength)
    {
    }

    void SetError(int nErrCode)
    {
        SetError(nErrCode, nStart);
    }

    void SetError(int nErrCode, int nPos)
    {
        int nLine, nLinePos;

        _PosToLinePos(nPos, nLine, nLinePos);
        err.SetError(nErrCode, nLine, nLinePos);

#ifdef BKJSON_ERROR_HAS_STRING

        CString strError;
        _MakeErrorString(strError, nErrCode, nPos, nLine, nLinePos);
        err.SetString(strError);

#endif

    }

    BOOL IsError()
    {
        return err.IsError();
    }

    LPCTSTR lpszText;
    int nStart;
    int nLength;
    Error err;

protected:

    void _PosToLinePos(int nPos, int &nLine, int &nLinePos)
    {
        const TCHAR *pchChar = lpszText;
        int nPosCalc = nPos;

        nLine = 1;
        nLinePos = 1;

        while (nPosCalc > 0)
        {
            nLinePos ++;

            if (_T('\n') == *pchChar)
            {
                nLine ++;
                nLinePos = 1;
            }
            
            pchChar ++;
            nPosCalc --;
        }

    }

#ifdef BKJSON_ERROR_HAS_STRING

    void _MakeErrorString(CString &strError, int nErrCode, int nPos, int nLine, int nLinePos)
    {
        int nPosNextLine = nPos;
        CString strText = lpszText;

        while (_T('\r') != lpszText[nPosNextLine] && _T('\n') != lpszText[nPosNextLine] && _T('\0') != lpszText[nPosNextLine])
            nPosNextLine ++;

        strText = strText.Mid(nPos - nLinePos + 1, nPosNextLine - nPos + nLinePos - 1);

        strError.Format(
            _T("\r\n### JSON Parse ERROR ###\r\n#\r\n# Line %d, Charactor %d\r\n#\r\n# %s\r\n# %*s^\r\n# %*s%s\r\n#\r\n\r\n"), 
            nLine, nLinePos, strText, nLinePos - 1, _T(""), nLinePos - 1, _T(""), Error::GetErrorStringFromCode(nErrCode)
            );

#ifdef BKJSON_DEBUG
        ::OutputDebugString(strError);
#endif

    }

#endif

};

//////////////////////////////////////////////////////////////////////////
// BkJson Node Handler Definition

class Handle
{
public:
    Handle(Value *pNodeParent = NULL, Value *pNode = NULL);
    virtual ~Handle();

    operator BoolType();
    operator LPCTSTR();
    operator LONG();
    operator ULONG();

    BOOL HasKey(LPCTSTR lpszKey);
    BOOL RemoveKey(LPCTSTR lpszKey);

    Handle operator[](LPCTSTR lpszKey);
    Handle operator[](int nIndex);

    Handle& operator=(NullType nullValue);
    Handle& operator=(BoolType bValue);
    Handle& operator=(LPCSTR szValue);
    Handle& operator=(LPCWSTR szValue);
    Handle& operator=(LONG lValue);

    int Count();
    void SetCount(int nNewCount);
    BOOL RemoveItem(int nIndex);
    void RemoveAllChildren();

    BOOL IsValid();
    BOOL IsNull();
    int GetType();
    CString GetName();

    POSITION FindFirstChild();
    POSITION FindNextChild(POSITION pos);
    Handle   GetAt(POSITION pos);

protected:

    Handle& _GetNullHandle();

    Value *m_pNodeParent;
    Value *m_pNode;
};

//////////////////////////////////////////////////////////////////////////
// BkJson Base Node Definition

class Value : public Handle
{
public:
    Value(Value *pNodeParent, POSITION posInParent, int nNodeType = TypeNull);
    ~Value();

    enum {
        TypeError   = 0, 
        TypeNull    = 1, 
        TypeObject, 
        TypeBool, 
        TypeString, 
        TypeLong, 
        TypeFloat, 
        TypeArray, 
    };

    int     GetType();
    int     GetParentType();
    CString GetName();

protected:

    friend ValueArray;
    friend ValueObject;

    int m_nType;

    // 在父结点中的位置
    // 如果父结点是ValueArray,  那么 m_posInParent 就是 CAtlArray 的下标
    // 如果父结点时ValueObject, 那么 m_posInParent 就是 CAtlMap 的 POSTION
    POSITION m_posInParent;

    static BOOL  _IsBlankChar(TCHAR chText);
    static void  _SkipBlankChar(ParseContext &context);
    static BOOL  _ExpectChar(ParseContext &context, TCHAR chExpected);
    static ULONG _HexToULong(LPCTSTR lpszValue);
    static TCHAR _ControlCharToRealChar(LPCTSTR lpszControlChar);
    static BOOL  _GetString(ParseContext &context, CString& strRet);
    static BOOL  _GetLong(ParseContext &context, LONG &lRet);
    static BOOL  _DumpString(CString &strDump, LPCTSTR lpszString, int nLength);

    Value* _ParseValue(ParseContext &context);
    Value* _CreateValueByType(int nType);

    virtual BOOL _Parse(ParseContext &context);
    virtual BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);
};

//////////////////////////////////////////////////////////////////////////
// BkJson Null Node Definition

class ValueNull : public Value
{
public:
    ValueNull(Value *pNodeParent, POSITION posInParent)
        : Value(pNodeParent, posInParent, TypeNull)
    {
    }

protected:

    BOOL _Parse(ParseContext &context);
    BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);
};

//////////////////////////////////////////////////////////////////////////
// BkJson Bool Node Definition

class ValueBool : public Value
{
public:
    ValueBool(Value *pNodeParent, POSITION posInParent)
        : Value(pNodeParent, posInParent, TypeBool)
        , m_bValue(False)
    {
    }

    BoolType GetValue();
    void SetValue(BoolType bValue);

protected:

    BoolType m_bValue;

    BOOL _Parse(ParseContext &context);
    BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);
};

//////////////////////////////////////////////////////////////////////////
// BkJson String Node Definition

class ValueString : public Value
{
public:
    ValueString(Value *pNodeParent, POSITION posInParent)
        : Value(pNodeParent, posInParent, TypeString)
    {
    }

    LPCTSTR GetValue();
    void SetValue(LPCTSTR szValue);

protected:

    CString m_strValue;

    BOOL _Parse(ParseContext &context);
    BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);
};

//////////////////////////////////////////////////////////////////////////
// BkJson Long Number Node Definition

class ValueLong : public Value
{
public:
    ValueLong(Value *pNodeParent, POSITION posInParent)
        : Value(pNodeParent, posInParent, TypeLong)
        , m_lValue(0)
    {
    }

    LONG GetValue();
    void SetValue(LONG lValue);

protected:
    LONG m_lValue;

    BOOL _Parse(ParseContext &context);
    BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);
};

//////////////////////////////////////////////////////////////////////////
// BkJson Array Node Definition

class ValueArray : public Value
{
public:
    ValueArray(Value *pNodeParent, POSITION posInParent)
        : Value(pNodeParent, posInParent, TypeArray)
    {
        m_arrNodes.SetCount(0, 5);
    }
    ~ValueArray();

    void RemoveAllItem();

    int Count();
    void SetCount(int nNewCount);

    Handle GetItem(int nIndex);
    BOOL RemoveItem(int nIndex);

protected:

    friend Handle;

    CAtlArray<Value *> m_arrNodes;

    BOOL _Parse(ParseContext &context);
    BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);

    Value* _ChangeItemType(Value *pOldItem, int nNewType);

    BOOL _HasGrandSon();
};

//////////////////////////////////////////////////////////////////////////
// BkJson Object Node Definition

class ValueObject : public Value
{
public:
    ValueObject(Value *pNodeParent, POSITION posInParent)
        : Value(pNodeParent, posInParent, TypeObject)
    {
    }
    ~ValueObject();

    int Count();

    void RemoveAllChild();
    BOOL HasChild(LPCTSTR lpszKey);
    Handle GetChild(LPCTSTR lpszKey);
    Handle NewChild(LPCTSTR lpszKey);
    BOOL RemoveChild(LPCTSTR lpszKey);

    POSITION FindFirstChild();
    POSITION FindNextChild(POSITION pos);
    Handle   GetAt(POSITION pos);

protected:

    friend Handle;
    friend Document;
    friend Value;

    CRBMap<CString, Value *> m_mapChilds;

    BOOL _Parse(ParseContext &context);
    BOOL _Dump(CString &strDump, BOOL bMultiLine, int nIndent, int nIndentAdd);

    Value* _ChangeChildType(Value *pOldChild, int nNewType);
};

class Document : public Handle
{
public:
    Document()
        : Handle(NULL, &m_rootNode)
        , m_rootNode(NULL, NULL)
    {
    }

    BOOL LoadFile(LPCTSTR lpszFileName);
    BOOL SaveFile(LPCTSTR lpszFileName, UINT uCodePage = CP_UTF8, BOOL bMultiLine = FALSE, int nIndent = 4);

    void Clear();
    BOOL LoadString(LPCTSTR lpszText);
    Error& GetError();

    CString Dump(BOOL bMultiLine = FALSE, int nIndent = 4);

protected:

    static void _EncodeText(LPCTSTR lpszSrc, CStringA &strTextOut, UINT uCodePage);
    static void _DecodeText(LPCSTR lpszSrc, CString &strTextOut);

    ValueObject m_rootNode;

    Error m_err;
};



};

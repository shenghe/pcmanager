#pragma once

#include <atlcoll.h>
#include <atlstr.h>

#define SZYAML_DUMP_INDENT 4

class SZYamlNode;
class SZYamlHandle;
class SZYamlDocument;

class SZYamlNode
{
public:
    SZYamlNode& operator=(SZYamlNode node);

    SZYamlNode& operator=(LPCTSTR lpszValue);

    SZYamlNode& operator=(int nValue);

    // Get string value
    operator LPCTSTR();
    LPCTSTR String();

    // Get int value
    operator int();
    int Int();

    // Get node key
    LPCTSTR Key();

    void SetKey(LPCTSTR lpszKey);

    void SetValue(LPCTSTR lpszValue);

    void SetValue(int nValue);

protected:

    CString m_strKey;
    CString m_strValue;
};

class SZYamlHandle
{
public:
    SZYamlHandle();

    // Copy
    SZYamlHandle& operator=(const SZYamlHandle& handle);

    // Set Value
    SZYamlHandle& operator=(LPCTSTR lpszValue);
    SZYamlHandle& operator=(int nValue);

    // Get child (with filter)
    SZYamlHandle operator[](LPCTSTR lpszName);

    // Get child (with filter and sub key condition)
    SZYamlHandle Find(LPCTSTR lpszChildName, LPCTSTR lpszConditionSubKey = NULL, LPCTSTR lpszConditionSubKeyValue = NULL);

    // Get node key
    LPCTSTR Key();

    // Get next
    SZYamlHandle Next();

    // Is valid handle
    BOOL Valid();

    // Get string value
    LPCTSTR String();

    // Get int value
    int Int();

    // Add a new child
    SZYamlHandle AddChild(LPCTSTR lpszChildName, LPCTSTR lpszValue = NULL);

    // Remove a child
    void RemoveChild(LPCTSTR lpszChildName);

protected:

    void SetInvalid();

    friend class SZYamlDocument;

    // Init data
    void _SetPosition(SZYamlDocument* pDoc, const POSITION pos);

    SZYamlDocument* m_pDoc;
    POSITION m_posItem;
    BOOL m_bFitKey;
};

class SZYamlDocument : public SZYamlHandle
{
public:
    BOOL LoadFile(LPCTSTR lpszFileName);
    BOOL SaveFile(LPCTSTR lpszFileName, UINT uCodePage = CP_UTF8, int nDumpIndent = SZYAML_DUMP_INDENT);

    BOOL Load(LPCTSTR lpszText);
    BOOL LoadBuffer(LPVOID lpBuffer, DWORD dwSize);

    SZYamlNode& GetRootNode();

    void GetRootHandle(SZYamlHandle& rootHandle);

    void Dump(CString& strText, int nDumpIndent = SZYAML_DUMP_INDENT);

    void SetEmpty();

protected:

    friend class SZYamlHandle;

    struct _YamlItem 
    {
        int nIndent;
        SZYamlNode node;

        _YamlItem& operator=(const _YamlItem& yi)
        {
            nIndent = yi.nIndent;
            node = yi.node;

            return *this;
        }
    };

    CAtlList<_YamlItem> m_YamlItemList;

    static void _EncodeText(LPCTSTR lpszSrc, CStringA &strTextOut, UINT uCodePage);
    static void _DecodeText(LPCSTR lpszSrc, CString &strTextOut);

    SZYamlNode& _GetNode(POSITION pos);
    POSITION _GetChildPos(POSITION pos, LPCTSTR lpszChildName = NULL);
    POSITION _GetNextPos(POSITION pos, BOOL bFitName);
    POSITION _InsertNodeAfter(POSITION posAfter, int nIndent, SZYamlNode& newNode);
    POSITION _InsertNodeBefore(POSITION posBefore, int nIndent, SZYamlNode& newNode);
    POSITION _AppendNode(int nIndent, SZYamlNode& newNode);
    POSITION _AddChild(POSITION pos, SZYamlNode& newNode);
    void     _RemoveChild(POSITION pos);
};
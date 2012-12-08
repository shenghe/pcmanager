/********************************************************************
* CreatedOn: 2006-11-27   15:05
* FileName:  KXMLCursor.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLCursor_H_
#define __KXMLCursor_H_

#include "KXMLDoc.h"

_KAN_DOM_BEGIN

//////////////////////////////////////////////////////////////////////////
// 除了文件操作LoadFile和SaveFile外
// 输入输出的字符串都应该是UTF8编码,少数包含宽字符的接口会被转化为UTF8
class KXMLCursor
{
public:
    KXMLCursor();

    ~KXMLCursor();


    long LoadFile(const TKChar* pcszFileName);

    long LoadString(const char* pcszInput);

    long LoadDOMName(const char* pcszName);

    long SaveFile(const TKChar* pcszFileName);

    long ToString(std::string* pstrOutput);

    long ToCompactString(std::string* pstrRet);

    template<class _KOtherDOMCursor>
    long ToOtherDOM(_KOtherDOMCursor& otherDOMCursor);



    //long GetLastInternalError();
    void DumpPath();

    void PopPathDump(std::vector<std::string>& pathInfo);
    




    long PhrasePath(const char* pcszPath, std::vector<std::string>& pathItemList);

    //////////////////////////////////////////////////////////////////////////
    // 下面的操作为其他接口使用

    //////////////////////////////////////////////////////////////////////////
    // structure operations
    void PrepareForRead();

    void PrepareForWrite();




    void BeginChild(const char* pcszName, bool bWrite);

    void BeginFirstChild(bool bWrite);

    void ToNextBrother();

    bool NoMoreBrother();

    void EndChild();

    bool IsNull();

    bool IsLeaf();



    void BeginFirstArrayElem(bool bWrite);

    void ToNextArrayElem();

    bool ArrayElemIsEnd();

    void EndArrayElem();


    void BeginArray(bool bWrite);

    void EndArray();

    bool IsArray();



    long EraseAllChildren();

    long RemoveChildren(const char* pcszName);

    long GetObjName(std::string& strName);


    template<class _KOtherDOMCursor>
    long CopyNodeTo(_KOtherDOMCursor& otherDOMCursor);



    template<class T>
    long ObjDataExchange(bool bWrite, T& value);

    template<class T>
    long ObjDataExchange(bool bWrite, T& value, const T& defValue);

    template<class T>
    long BasicDataExchange(bool bWrite, T& value);

    template<class T>
    long BasicDataExchange(bool bWrite, T& value, const T& defValue);
    


    //////////////////////////////////////////////////////////////////////////
    // data operations
    template<class T>
    long SetObjValue(const T& value);

    // 当找不到合适的item时,如果pDefValue不为NULL,则返回成功,且value为*pDefValue
    // 否则返回失败
    template<class T>
    long GetObjValue(T& value);

    template<class T>
    long GetObjValue(T& value, const T& defValue);




    template<class T>
    long SetObjAttribute(const char* pcszAttName, const T& value);

    // 当找不到合适的item时,如果pDefValue不为NULL,则返回成功,且value为*pDefValue
    // 否则返回失败
    template<class T>
    long GetObjAttribute(const char* pcszAttName, T& value);

    template<class T>
    long GetObjAttribute(const char* pcszAttName, T& value, const T& defValue);


private:
    KXMLCursor(const KXMLCursor&);
    KXMLCursor& operator=(const KXMLCursor&);

    long _InitCursor();
    void _UninitCursor();
    void _ClearCursor();

    long _VerifyCursor();




    //void _TraceError(long lRet);




    //////////////////////////////////////////////////////////////////////////
    // 由于重载比模板优先级高，所以这里可以屏蔽危险的用法
    typedef const void*             K_PCVOID;
    typedef const char*             K_PCCHAR;
    typedef const unsigned char*    K_PCUCHAR;
    typedef const wchar_t*          K_PCWCHAR;

    long GetObjValue(K_PCVOID  value,    K_PCVOID  defStr);
    long GetObjValue(K_PCVOID  value);
    long GetObjValue(K_PCCHAR  value,    K_PCCHAR  defStr);
    long GetObjValue(K_PCCHAR  value);
    long GetObjValue(K_PCUCHAR value,    K_PCUCHAR defStr);
    long GetObjValue(K_PCUCHAR value);
    long GetObjValue(K_PCWCHAR value,    K_PCWCHAR defStr);
    long GetObjValue(K_PCWCHAR value);



    class KXMLLocalScope
    {
    public:
        KXMLLocalScope(): m_bWrite(false), m_hObj(NULL)
        {
        }

        KXMLLocalScope(
            bool                bWrite,
            const KXMLHANDLE&   hObj,
            const std::string&  strObjName
        ):
            m_bWrite(bWrite),
            m_hObj(hObj),
            m_strObjName(strObjName)
        {
        }

        bool        m_bWrite;
        KXMLHANDLE  m_hObj;
        std::string m_strObjName;
    };

    bool m_bInitFlag;

    KXMLDoc m_xmlDoc;

    std::vector<KXMLLocalScope> m_scopeStack;

    //long                        m_lLastInternalError;
    std::vector<std::string>    m_pathDump;
};

_KAN_DOM_END

#include "KXMLCursorImp.h"

#endif//__KXMLCursor_H_

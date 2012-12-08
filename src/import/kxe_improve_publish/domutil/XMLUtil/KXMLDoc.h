/********************************************************************
* CreatedOn: 2006-11-27   9:53
* FileName:  KXMLDoc.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLDoc_H_
#define __KXMLDoc_H_

#include "KXMLBasic.h"
#include "KXMLItemAccess.h"

_KAN_DOM_BEGIN

//////////////////////////////////////////////////////////////////////////
// 除了文件操作LoadFile和SaveFile外
// 输入输出的字符串都应该是UTF8编码,少数包含宽字符的接口会被转化为UTF8
class KXMLDoc
{
public:
    KXMLDoc();

    ~KXMLDoc();

    //////////////////////////////////////////////////////////////////////////
    // i/o operations
    long LoadFile(const TKChar* pcszFileName);

    long SaveFile(const TKChar* pcszFileName);

    long LoadXml(const char* pcszXML);

    long LoadXMLRootName(const char* pcszName);

    long ToXMLString(std::string* pstrRet);

    long ToCompactXMLString(std::string* pstrRet);


    //////////////////////////////////////////////////////////////////////////
    // structure operations
    long PhrasePath(const char* pcszPath, std::vector<std::string>& pathItemList);

    KXMLHANDLE OpenItemPath(const char* pcszPath, bool bCreateIfNotExist);

    KXMLHANDLE OpenChild(
        const KXMLHANDLE&   hParent,
        const char*         pcszName,
        bool                bCreateIfNotExist
    );

    KXMLHANDLE OpenNext(
        const KXMLHANDLE&   hCurrent,
        const char*         pcszName,
        bool                bCreateIfNotExist
    );

    KXMLHANDLE OpenFirstChildForRead(const KXMLHANDLE& hParent);

    KXMLHANDLE OpenNextBrotherForRead(const KXMLHANDLE& hCurrent);

    long EraseAllChildren(const KXMLHANDLE& hParent);

    long RemoveChildren(const KXMLHANDLE& hParent, const char* pcszName);

    KXMLManager& XMLManager();

    long GetName(const KXMLHANDLE& hXML, std::string& strName);

    //bool IsLeaf(const KXMLHANDLE& hXML);



    //////////////////////////////////////////////////////////////////////////
    // data operations
    template<class T>
    long SetValue(KXMLHANDLE hXMLCursor, const T& value);

    template<class T>
    long GetValue(KXMLHANDLE hXMLCursor, T& value);

    template<class T>
    long SetAttribute(KXMLHANDLE hXMLCursor, const char* pcszAttName, const T& value);

    template<class T>
    long GetAttribute(KXMLHANDLE hXMLCursor, const char* pcszAttName, T& value);



private:
    KXMLDoc(const KXMLDoc&);
    KXMLDoc& operator=(const KXMLDoc&);



    template<class ItemWriter, class T>
    long _SetXMLItem(const ItemWriter& writer, const T& value);

    template<class ItemReader, class T>
    long _GetXMLItem(const ItemReader& reader, T& value);



    template <class ItemWriter>
    long _SetXMLItem(const ItemWriter& writer, const std::string& strValue);

    template <class ItemReader>
    long _GetXMLItem(const ItemReader& reader, std::string& strValue);

    template <class ItemWriter, class _Traits, class _Alloc>
    long _SetXMLItem(
        const ItemWriter& writer,
        const std::basic_string<char, _Traits, _Alloc>& strValue
    );

    template <class ItemReader, class _Traits, class _Alloc>
    long _GetXMLItem(
        const ItemReader& reader,
        std::basic_string<char, _Traits, _Alloc>& strValue
    );




    template <class ItemWriter, class _Traits, class _Alloc>
    long _SetXMLItem(
        const ItemWriter& writer,
        const std::basic_string<wchar_t, _Traits, _Alloc>& wstrValue
    );

    template <class ItemReader, class _Traits, class _Alloc>
    long _GetXMLItem(
        const ItemReader& reader,
        std::basic_string<wchar_t, _Traits, _Alloc>& wstrValue
    );



    template <class ItemWriter>
    long _SetXMLItem(const ItemWriter& writer, wchar_t wcValue);

    template <class ItemReader>
    long _GetXMLItem(const ItemReader& reader, wchar_t& wcValue);


    template <class ItemWriter>
    long _SetXMLItem(const ItemWriter& writer, const wchar_t* pwszValue);


    //////////////////////////////////////////////////////////////////////////
    // 危险的重载，应当禁止
    template <class ItemReader>
    long _GetXMLItem(const ItemReader& reader, void* value);

    template <class ItemReader>
    long _GetXMLItem(const ItemReader& reader, char* value);

    template <class ItemReader>
    long _GetXMLItem(const ItemReader& reader, unsigned char* value);

    template <class ItemReader>
    long _GetXMLItem(const ItemReader& reader, wchar_t* value);



    bool m_bInitFlag;

    KXMLManager m_xmlManager;
};


_KAN_DOM_END


#include "KXMLDocImp.h"


#endif//__KXMLDoc_H_

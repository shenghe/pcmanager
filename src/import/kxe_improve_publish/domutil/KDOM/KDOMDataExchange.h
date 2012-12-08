/********************************************************************
* CreatedOn: 2006-12-12   17:44
* FileName:  KDOMDataExchange.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMDataExchange_H_
#define __KDOMDataExchange_H_

#include "KDOMBasic.h"
#include "KDOMMacro.h"
#include "KDOMAutoChild.h"
#include "KDOMAutoArrayElem.h"
#include "KDOMAutoMap.h"

#include <vector>
#include <list>
#include <map>
#include <utility>

#include <ctype.h>

_KAN_DOM_BEGIN

// bool
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, bool& bValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, bValue);
}

// short
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, signed short& sValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, sValue);
}

// unsigned short
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, unsigned short& usValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, usValue);
}

// int
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, signed int& nValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, nValue);
}

// unsigned int
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, unsigned int& unValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, unValue);
}

// long
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, signed long& lValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, lValue);
}

// unsigned long
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, unsigned long& ulValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, ulValue);
}

// __int64
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, __int64& i64Value, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, i64Value);
}

// char
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, char& cValue, bool bWrite)
{
    int nValue = cValue;    // char类型作为数值类型转换

    long lRet = domCur.BasicDataExchange(bWrite, nValue);
    if (SUCCEEDED(lRet) && !bWrite)
    {   // 读操作时,需要检查数值范围
        if (nValue > SCHAR_MAX || nValue < SCHAR_MIN)
        {   // 超出数值范围
            domCur.DumpPath();
            return E_KDOM_DATA_OUT_OF_RANGE;
        }
        else
        {
            cValue = nValue;
        }
    }

    return lRet;
}

// signed char
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, signed char& cValue, bool bWrite)
{
    int nValue = cValue;    // char类型作为数值类型转换

    long lRet = domCur.BasicDataExchange(bWrite, nValue);
    if (SUCCEEDED(lRet) && !bWrite)
    {   // 读操作时,需要检查数值范围
        if (nValue > SCHAR_MAX || nValue < SCHAR_MIN)
        {   // 超出数值范围
            domCur.DumpPath();
            return E_KDOM_DATA_OUT_OF_RANGE;
        }
        else
        {
            cValue = nValue;
        }
    }

    return lRet;
}

// unsigned char
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, unsigned char& ucValue, bool bWrite)
{
    unsigned int unValue = ucValue; // unsigned char类型作为数值类型转换

    long lRet = domCur.BasicDataExchange(bWrite, unValue);
    if (SUCCEEDED(lRet) && !bWrite)
    {   // 读操作时,需要检查数值范围
        if (unValue > UCHAR_MAX)
        {   // 超出数值范围
            domCur.DumpPath();
            return E_KDOM_DATA_OUT_OF_RANGE;
        }
        else
        {
            ucValue = unValue;
        }
    }
    
    return lRet;
}



//////////////////////////////////////////////////////////////////////////
// see MSDN about "/Zc"
#ifdef _NATIVE_WCHAR_T_DEFINED

// wchar_t
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, wchar_t& wcValue, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, wcValue);
}

#endif



// char*
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, const char* pszValue, bool bWrite)
{
    assert(bWrite);
    if (bWrite)
    {
        return domCur.SetObjValue(pszValue);
    }
    
    return E_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING;
}

// wchar_t*
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, const wchar_t* pwszValue, bool bWrite)
{
    assert(bWrite);
    if (bWrite)
    {
        return domCur.SetObjValue(pwszValue);
    }

    return E_KDOM_DATA_TYPE_DO_NOT_SUPPORT_WRITING;
}








//////////////////////////////////////////////////////////////////////////
// std::xstring
template<class _KDOMCursor, class _CharType, class _Traits, class _Alloc>
inline long KXMLDataExchange(
    _KDOMCursor& domCur,
    std::basic_string<_CharType, _Traits, _Alloc>& strValue,
    bool bWrite
)
{
    return domCur.BasicDataExchange(bWrite, strValue);
}




//////////////////////////////////////////////////////////////////////////
// std::vector
template<class _KDOMCursor, class T, class _Alloc>
inline long KXMLDataExchange(_KDOMCursor& domCur, std::vector<T, _Alloc>& stlVec, bool bWrite)
{
    long lRet;

    //KANDOM::KDOMAutoArrayEx<_KDOMCursor> domArrayIter(&domCur, bWrite);

    if (bWrite)
    {
        // 如果是写操作,则删除所有子结点
        lRet = domCur.EraseAllChildren();
        if (FAILED(lRet))
        {
            domCur.DumpPath();
            return lRet;
        }

        // 如果列表为空,则直接返回
        if (stlVec.empty())
        {
            return S_OK;
        }
    }
    else
    {   // 如果是读操作,则vec需要先清空
        stlVec.clear();
    }

    {
        // 对每个元素进行数据交换
        KANDOM::KDOMAutoArrayElemEx<_KDOMCursor> domArrayIter(&domCur, bWrite);

        if (bWrite)
        {
            if (!stlVec.empty())
            {   // 由于MoveToNext()会产生一个null对象,所以这里将第一个元素的写入提前
                std::vector<T, _Alloc>::iterator iter = stlVec.begin();

                lRet = domCur.ObjDataExchange(bWrite, *iter);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                ++iter; // 循环从第二个元素开始
                for (NULL; iter != stlVec.end(); ++iter)
                {
                    // domIter平移到下一个元素
                    domArrayIter.MoveToNext();

                    lRet = domCur.ObjDataExchange(bWrite, *iter);
                    if (FAILED(lRet))
                    {
                        return lRet;
                    }
                }
            }
        }
        else
        {
            while (!domArrayIter.End())
            {
                stlVec.push_back(T());
                lRet = domCur.ObjDataExchange(bWrite, stlVec.back());
                if (FAILED(lRet))
                {
                    return lRet;
                }

                // 平移到下一个元素
                domArrayIter.MoveToNext();
            }
        }


        // 退出时autoChild自动返回上层
    }

    return S_OK;
}








//////////////////////////////////////////////////////////////////////////
// std::list
template<class _KDOMCursor, class T, class _Alloc>
inline long KXMLDataExchange(_KDOMCursor& domCur, std::list<T, _Alloc>& stlList, bool bWrite)
{
    long lRet;

    KANDOM::KDOMAutoArrayEx<_KDOMCursor> domArrayIter(&domCur, bWrite);

    if (bWrite)
    {
        // 如果是写操作,则删除所有子结点
        lRet = domCur.EraseAllChildren();
        if (FAILED(lRet))
        {
            domCur.DumpPath();
            return lRet;
        }

        // 如果列表为空,则直接返回
        if (stlList.empty())
        {
            return S_OK;
        }
    }
    else
    {   // 如果是读操作,则vec需要先清空
        stlList.clear();
    }

    {
        // 对每个元素进行数据交换
        KANDOM::KDOMAutoArrayElemEx<_KDOMCursor> domArrayIter(&domCur, bWrite);

        if (bWrite)
        {
            if (!stlList.empty())
            {   // 由于MoveToNext()会产生一个null对象,所以这里将第一个元素的写入提前
                std::list<T, _Alloc>::iterator iter = stlList.begin();

                lRet = domCur.ObjDataExchange(bWrite, *iter);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                ++iter; // 循环从第二个元素开始
                for (NULL; iter != stlVec.end(); ++iter)
                {
                    // domIter平移到下一个元素
                    domArrayIter.MoveToNext();

                    lRet = domCur.ObjDataExchange(bWrite, *iter);
                    if (FAILED(lRet))
                    {
                        return lRet;
                    }
                }
            }
        }
        else
        {
            while (!domArrayIter.End())
            {
                stlVec.resize(stdVec.size());
                lRet = domCur.ObjDataExchange(bWrite, stlList.back());
                if (FAILED(lRet))
                {
                    return lRet;
                }

                // 平移到下一个元素
                domArrayIter.MoveToNext();
            }
        }


        // 退出时autoChild自动返回上层
    }

    return S_OK;
}





//////////////////////////////////////////////////////////////////////////
// std::map
template<class _KDOMCursor, class K, class T, class PR, class _Alloc>
inline long KXMLDataExchange(_KDOMCursor& domCur, std::map<K, T, PR, _Alloc>& stlMap, bool bWrite)
{
    long lRet;

    if (bWrite)
    {
        // 写操作会将当前结点类型变成array
        //// 如果是写操作,则删除所有子结点
        //lRet = domCur.EraseAllChildren();
        //if (FAILED(lRet))
        //{
        //    domCur.DumpPath();
        //    return lRet;
        //}

        // 如果列表为空,则直接返回
        if (stlMap.empty())
        {
            return S_OK;
        }
    }
    else
    {   // 如果是读操作,则map需要先清空
        stlMap.clear();
    }

    {
        // 对每个元素进行数据交换
        

        if (bWrite)
        {
            if (!stlMap.empty())
            {
                KANDOM::KDOMAutoMapEx<_KDOMCursor> autoMap(&domCur, bWrite);

                std::map<K, T, PR, _Alloc>::iterator iter = stlMap.begin();
                for (NULL; iter != stlMap.end(); ++iter)
                {
                    lRet = autoMap.SetMapElem(iter->first, iter->second);
                    if (FAILED(lRet))
                    {
                        return lRet;
                    }
                }
            }
        }
        else
        {
            KANDOM::KDOMAutoMapElemEx<_KDOMCursor> autoMapElem(&domCur, bWrite);

            while (!autoMapElem.End())
            {
                std::pair<K, T> value;


                lRet = autoMapElem.GetMapElem(value.first, value.second);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                stlMap.insert(value);

                // 平移到下一个元素
                autoMapElem.MoveToNextElem();
            }
        }


        // 退出时autoChild自动返回上层
    }

    return S_OK;
}






_KAN_DOM_END

#endif//__KDOMDataExchange_H_

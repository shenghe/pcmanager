/********************************************************************
* CreatedOn: 2006-12-11   14:50
* FileName:  KDOMAccessImp.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMAccessImp_H_
#define __KDOMAccessImp_H_

#include "KDOMBasic.h"
#include "KDOMAccess.h"
#include "KDOMAutoPath.h"

_KAN_DOM_BEGIN

template<class _KDOMCursor>
inline KDOMAccess<_KDOMCursor>::KDOMAccess(): m_bInitFlag(false)
{
}

template<class _KDOMCursor>
inline KDOMAccess<_KDOMCursor>::~KDOMAccess()
{
}



template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::LoadFile(const TKChar* pcszFileName)
{
    assert(pcszFileName);
    int nRetCode = m_domCursor.LoadFile(pcszFileName);

    m_bInitFlag = SUCCEEDED(nRetCode);
    
    return nRetCode;
}

template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::LoadString(const char* pcszInput)
{
    assert(pcszInput);
    int nRetCode = m_domCursor.LoadString(pcszInput);

    m_bInitFlag = SUCCEEDED(nRetCode);

    return nRetCode;
}

template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::LoadDOMName(const char* pcszName)
{
    assert(pcszName);
    int nRetCode = m_domCursor.LoadDOMName(pcszName);

    m_bInitFlag = SUCCEEDED(nRetCode);

    return nRetCode;
}

// 载入另一个DOM对象
template<class _KDOMCursor>
template<class _KOtherDOMCursor>
inline long KDOMAccess<_KDOMCursor>::LoadDOM(KDOMAccess<_KOtherDOMCursor>& otherDomAccess)
{
    // 避免自复制
    if ((void*)this == (void*)&otherDomAccess)
        return S_OK;

    int nRetCode = otherDomAccess.m_domCursor.ToOtherDOM(m_domCursor);

    m_bInitFlag = SUCCEEDED(nRetCode);

    return nRetCode;
}




template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::SaveFile(const TKChar* pcszFileName)
{
    assert(pcszFileName);
    return m_domCursor.SaveFile(pcszFileName);
}


template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::ToString(std::string* pstrOutput)
{
    assert(m_bInitFlag);
    assert(pstrOutput);
    return m_domCursor.ToString(pstrOutput);
}

template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::ToCompactString(std::string* pstrRet)
{
    assert(m_bInitFlag);
    assert(pstrRet);
    return m_domCursor.ToCompactString(pstrRet);
}





template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::GetDataName(std::string* pstrDataName)
{
    assert(m_bInitFlag);
    assert(pstrDataName);

    return m_domCursor.GetObjName(*pstrDataName);
}

template<class _KDOMCursor>
inline long KDOMAccess<_KDOMCursor>::PopPathDump(std::vector<std::string>& dumpPath)
{
    assert(m_bInitFlag);

    m_domCursor.PopPathDump(dumpPath);

    return 0;
}

template<class _KDOMCursor>
inline _KDOMCursor& KDOMAccess<_KDOMCursor>::Cursor()
{
    return m_domCursor;
}






template<class _KDOMCursor>
template<class T>
inline long KDOMAccess<_KDOMCursor>::WriteData(const char* pcszPath, T& value)
{
    assert(m_bInitFlag);
    assert(pcszPath);

    m_domCursor.PrepareForWrite();

    {
        KDOMAutoPath autoPath(pcszPath, &m_domCursor, true);

        return m_domCursor.ObjDataExchange(true, value);
    }
}

template<class _KDOMCursor>
template<class T>
inline long KDOMAccess<_KDOMCursor>::ReadData(const char* pcszPath, T& value)
{
    assert(m_bInitFlag);
    assert(pcszPath);

    m_domCursor.PrepareForRead();

    {
        KDOMAutoPath autoPath(pcszPath, &m_domCursor, false);

        return m_domCursor.ObjDataExchange(false, value);
    }
}

template<class _KDOMCursor>
template<class T>
inline long KDOMAccess<_KDOMCursor>::ReadData(const char* pcszPath, T& value, const T& defValue)
{
    assert(m_bInitFlag);
    assert(pcszPath);

    int nRetCode = ReadData(pcszPath, value);
    if (FAILED(nRetCode))
    {
        value = defValue;
    }

    return S_OK;
}

_KAN_DOM_END

#endif//__KDOMAccessImp_H_

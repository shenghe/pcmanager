/********************************************************************
* CreatedOn: 2006-12-12   14:41
* FileName:  KJSONHandleImp.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KJSONHandleImp_H_
#define __KJSONHandleImp_H_

#include "KJSONHandle.h"

_KAN_DOM_BEGIN


inline KJSONHandle::KJSONHandle():
    m_pValue(NULL)
{
}

inline KJSONHandle::KJSONHandle(const KJSONHandle& rHandle):
    m_pValue(rHandle.m_pValue)
{
}

inline KJSONHandle& KJSONHandle::operator=(const KJSONHandle& rHandle)
{
    m_pValue = rHandle.m_pValue;

    return *this;
}

inline KJSONHandle::KJSONHandle(Json::Value* pValue): m_pValue(pValue)
{   
}

inline KJSONHandle::~KJSONHandle()
{
    m_pValue = NULL;
}




inline Json::Value* KJSONHandle::ValuePtr()
{
    return m_pValue;
}

inline Json::Value& KJSONHandle::ValueRef()
{
    return *m_pValue;
}

_KAN_DOM_END


#endif//__KJSONHandleImp_H_

/********************************************************************
* CreatedOn: 2006-12-12   14:28
* FileName:  KJSONHandle.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KJSONHandle_H_
#define __KJSONHandle_H_

#include "KJSONBasic.h"

_KAN_DOM_BEGIN

class KJSONHandle
{
public:
    KJSONHandle();

    KJSONHandle(const KJSONHandle& rHandle);

    KJSONHandle& operator=(const KJSONHandle& rHandle);

    KJSONHandle(Json::Value* pValue);

    ~KJSONHandle();

    Json::Value* ValuePtr();

    Json::Value& ValueRef();

private:
    Json::Value* m_pValue;
};

_KAN_DOM_END


#include "KJSONHandleImp.h"


#endif//__KJSONHandle_H_

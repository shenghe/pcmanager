/********************************************************************
* CreatedOn: 2006-12-12   18:00
* FileName:  KJSONDataAccess.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KJSONDataAccess_H_
#define __KJSONDataAccess_H_

#include "KJSONCursor.h"
#include "../KDOM/KDOMMacro.h"
#include "../KDOM/KDOMDataExchange.h"
#include "../KDOM/KDOMAccess.h"
#include "../KDOM/KDOMAutoChild.h"
#include "../KDOM/KDOMAutoArrayElem.h"
#include "../KDOM/KDOMAutoPath.h"

_KAN_DOM_BEGIN

typedef KDOMAccess<KJSONCursor>             KJSONDataAccess;
typedef KDOMAutoChildEx<KJSONCursor>        KJSONAutoChild;
typedef KDOMAutoArrayElemEx<KJSONCursor>    KJSONAutoArrayElem;
typedef KDOMAutoPathEx<KJSONCursor>         KJSONAutoPath;

_KAN_DOM_END

#endif//__KJSONDataAccess_H_

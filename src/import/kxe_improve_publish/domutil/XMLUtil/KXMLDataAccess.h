/********************************************************************
* CreatedOn: 2006-11-28   21:25
* FileName:  KXMLDataAccess.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLDataAccess_H_
#define __KXMLDataAccess_H_

#include "KXMLCursor.h"
#include "../KDOM/KDOMMacro.h"
#include "../KDOM/KDOMDataExchange.h"
#include "../KDOM/KDOMAccess.h"

_KAN_DOM_BEGIN

typedef KDOMAccess<KXMLCursor>          KXMLDataAccess;
typedef KDOMAutoChildEx<KXMLCursor>     KXMLAutoChild;
typedef KDOMAutoArrayElemEx<KXMLCursor> KXMLAutoArrayElem;
typedef KDOMAutoPathEx<KXMLCursor>      KXMLAutoPath;
typedef KDOMAutoBrotherEx<KXMLCursor>   KXMLAutoBrother;


//template<class KDOMCursor, class T, class _Alloc>
//inline int KXMLDataExchange(KDOMCursor& domCur, KXMLDataAccess& xmlDataAccess, bool bWrite)
//{
//    return 0;
//}


_KAN_DOM_END

#endif//__KXMLDataAccess_H_

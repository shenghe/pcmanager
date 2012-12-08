/********************************************************************
* CreatedOn: 2006-12-31   14:36
* FileName:  KDOMKSCEDataExchange.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMKSCEDataExchange_H_
#define __KDOMKSCEDataExchange_H_

#include "KDOMDataExchange.h"

#include "../../KSCE/SCOMCommon/SCOMDef.h"
#include "../../KSCE/KSCECoreCommon/KSCEAddress.h"

#include <istream>
#include <ostream>

_KAN_DOM_BEGIN

//////////////////////////////////////////////////////////////////////////
// GUID
template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, GUID& guid, bool bWrite)
{
    return domCur.BasicDataExchange(bWrite, guid);
}
// end of GUID
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// KSCEAddress
//inline std::ostringstream& operator<<(std::ostringstream& os, const KSCEAddress& kAddress)
//{
//    os << kAddress.GetGUID() << ":" << kAddress.GetIdentify();
//
//    return os;
//}
//
//inline std::ostringstream& operator<<(std::ostringstream& os, KSCEAddress& kAddress)
//{
//    os << kAddress.GetGUID() << ":" << kAddress.GetIdentify();
//
//    return os;
//}
//
//inline std::istringstream& operator>>(std::istringstream& is, KSCEAddress& kAddress)
//{
//    GUID guidEx;
//    long idEx;
//
//    char tempChar;
//
//    is >> guidEx >> tempChar >> idEx;
//
//    kAddress = KSCEAddress(guidEx, idEx);
//
//    return is;
//}


template<class _KDOMCursor>
inline long KXMLDataExchange(_KDOMCursor& domCur, KSCEAddress& kAddress, bool bWrite)
{
    long lRet = -1;
    GUID guidEx;
    long idEx;

    if (bWrite)
    {
        guidEx = kAddress.GetGUID();
        idEx   = kAddress.GetIdentify();
    }

    {
        KDOMAutoChildEx<_KDOMCursor> autoChild("guid", &domCur, bWrite);

        lRet = domCur.ObjDataExchange(bWrite, guidEx);
        if (FAILED(lRet))
        {
            return lRet;
        }
    }

    {
        KDOMAutoChildEx<_KDOMCursor> autoChild("id", &domCur, bWrite);

        lRet = domCur.ObjDataExchange(bWrite, idEx);
        if (FAILED(lRet))
        {
            return lRet;
        }
    }
    

    if (!bWrite)
    {
        kAddress = KSCEAddress(guidEx, idEx);
    }

    return S_OK;
}
// end of KSCEAddress
//////////////////////////////////////////////////////////////////////////


_KAN_DOM_END


#endif//__KDOMKSCEDataExchange_H_

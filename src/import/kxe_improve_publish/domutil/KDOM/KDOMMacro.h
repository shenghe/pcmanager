/********************************************************************
* CreatedOn: 2006-12-12   17:41
* FileName:  KDOMMacro.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMMacro_H_
#define __KDOMMacro_H_

#include "KDOMBasic.h"
#include "KDOMAutoChild.h"


//////////////////////////////////////////////////////////////////////////
// 这些宏已经保证将自己被定义在命名空间KANDOM
// 所以不可以在命名空间KANDOM内部使用这些宏
//
// 注意,这些宏只能在全局名字空间内使用

#define KDX_ENUM_DEFINE(_ENUM)\
	_KAN_DOM_BEGIN                                          \
	template<class _KDOMCursor>                             \
	inline long KXMLDataExchange(                           \
	_KDOMCursor& _domCur, _ENUM& _obj, bool _bWrite    \
	)                                                       \
	{                                                       \
		return KXMLDataExchange(_domCur, (unsigned int&)_obj, _bWrite);		\
	}\
	_KAN_DOM_END


#define KDX_BEGIN(_CLASS)                                   \
    _KAN_DOM_BEGIN                                          \
    template<class _KDOMCursor>                             \
    inline long KXMLDataExchange(                           \
        _KDOMCursor& _domCur, _CLASS& _obj, bool _bWrite    \
    )                                                       \
    {                                                       \
        int _nRetCode = -1;


//////////////////////////////////////////////////////////////////////////
// 非内建类型的xml序列化
#define KDX_OBJ_NM(_NAME, _MEMBER_OBJ)                  \
        do {                                            \
            KDOMAutoChildEx<                            \
                _KDOMCursor                             \
            > domAutoChild(_NAME, &_domCur, _bWrite);   \
                                                        \
            _nRetCode = _domCur.ObjDataExchange(        \
                _bWrite, _obj._MEMBER_OBJ               \
            );                                          \
            if (FAILED(_nRetCode))                      \
            {                                           \
                return _nRetCode;                       \
            }                                           \
        } while (0)

#define KDX_OBJ_EX(_NAME, _MEMBER_OBJ, _DEFAULT)        \
        do {                                            \
            KDOMAutoChildEx<                            \
                _KDOMCursor                             \
            > domAutoChild(_NAME, &_domCur, _bWrite);   \
                                                        \
            _nRetCode = _domCur.ObjDataExchange(        \
                _bWrite, _obj._MEMBER_OBJ, _DEFAULT     \
            );                                          \
            if (FAILED(_nRetCode))                      \
            {                                           \
                return _nRetCode;                       \
            }                                           \
        } while (0)


#define KDX_END()                                       \
        return S_OK;                                    \
    }                                                   \
    _KAN_DOM_END






// 当处在其他名字空间内的时候,使用下面的宏替代 KDX_BEGIN/KDX_END

#define KDX_BEGIN_IN_NAMESPACE(_CLASS, PREV_NS) \
    };                                          \
    KDX_BEGIN(PREV_NS::_CLASS)

#define KDX_END_IN_NAMESPACE(PREV_NS)           \
    KDX_END();                                  \
    namespace PREV_NS {





#endif//__KDOMMacro_H_

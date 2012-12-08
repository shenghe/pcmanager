/********************************************************************
* CreatedOn: 2006-2-16   10:59
* FileName:  KChar.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KChar_H_
#define __KChar_H_

#include <wchar.h>
#include <tchar.h>
#include <assert.h>

typedef char            KChar;
typedef wchar_t         WKChar;
typedef unsigned char   KByte;

#ifdef UNICODE

typedef WKChar          TKChar;
#define KTEXT(x)        L##x

#else//UNICODE

typedef KChar           TKChar;
#define KTEXT(x)        x

#endif//UNICODE






#ifdef _NATIVE_WCHAR_T_DEFINED

#define TREATE_WCHAR_T_AS_BUILD_IN_TYPE

#endif//_NATIVE_WCHAR_T_DEFINED













#endif//__KChar_H_

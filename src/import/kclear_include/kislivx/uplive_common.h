//Common.h
//升级程序公共库的公共头文件。
//

#pragma once
#ifndef __UPLIVE_COMMON_H__
#define __UPLIVE_COMMON_H__
#define  NS_BEGIN(name) namespace name {
#define  NS_END(name)   };

#include <tchar.h>
#include <string>
#include <assert.h>
#include <windows.h>
#include <basetyps.h>

//////////////////////////////////////////////////////////////////////////
//断言定义。
#ifndef ASSERT
	#define ASSERT assert
#endif

//////////////////////////////////////////////////////////////////////////
//数据类型定义。
typedef		unsigned char		u_char;
typedef		unsigned char		byte;
typedef		unsigned int		u_int;
typedef		unsigned short		u_short;
typedef		unsigned long		u_long;
//
typedef		unsigned char       uint8;
typedef		unsigned int        uint32;
typedef		int                 int32;
//////////////////////////////////////////////////////////////////////////
// 
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&);               \
	TypeName& operator=(const TypeName&)

// An older, deprecated, politically incorrect name for the above.
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) DISALLOW_COPY_AND_ASSIGN(TypeName)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
	TypeName();                                    \
	DISALLOW_COPY_AND_ASSIGN(TypeName)

//////////////////////////////////////////////////////////////////////////
// std::tstring 字符定义。
NS_BEGIN(std)
#ifdef _UNICODE
	typedef wstring				tstring;
#else
	typedef string				tstring;
#endif
NS_END(std)

#define IsStringValid(x) ((x) && (x)[0])

#define TCHAR_SIZE_OF(szBuf) (sizeof(szBuf) / sizeof(TCHAR))

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#endif	//__UPLIVE_COMMON_H__

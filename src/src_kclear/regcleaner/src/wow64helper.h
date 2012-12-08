#ifndef REGCLEANER_WOW64HELPER_H_
#define REGCLEANER_WOW64HELPER_H_

//////////////////////////////////////////////////////////////////////////

typedef enum tagWow64Type {
    WOW64_DEFAULT = 0,      // 访问文件和注册表都使用默认模式
    FORCE_WOW64_64 = 1,     // 强制访问64位注册表子集, 关闭文件重定向
    FORCE_WOW64_32 = 2,     // 强制访问32位注册表子集
    WOW64_MISC = 3          // 混杂模式, 按照默认方式访问注册表, 
                            //   不关闭文件重定向和关闭的情况下，只要有一种文件存在，就算文件存在
} Wow64Type;

//////////////////////////////////////////////////////////////////////////

inline DWORD GetRegAccess(Wow64Type wow64type) 
{
    DWORD retval = 0;

    switch (wow64type)
    {
    case FORCE_WOW64_64:
        retval = KEY_WOW64_64KEY;
        break;

    case FORCE_WOW64_32:
        retval = KEY_WOW64_32KEY;
        break;

    case WOW64_DEFAULT:
    case WOW64_MISC:
    default:
        retval = 0;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

#endif // REGCLEANER_WOW64HELPER_H_

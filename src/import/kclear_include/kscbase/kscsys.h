#ifndef KSCSYS_INC_
#define KSCSYS_INC_

//////////////////////////////////////////////////////////////////////////

#include "kscbase.h"

//////////////////////////////////////////////////////////////////////////

typedef enum tagWinVersion {
	WINVERSION_PRE_2000 = 0,
	WINVERSION_2000 = 1,
	WINVERSION_XP = 2,
	WINVERSION_SERVER_2003 = 3,
	WINVERSION_VISTA = 4,
	WINVERSION_2008 = 5,
	WINVERSION_WIN7 = 6,
} WinVersion;

// 获得操作系统版本
WinVersion KGetWinVersion();

// 获得ServicePack的版本
void KGetServicePackLevel(int* major, int* minor);

// 获得CPU的个数
int KNumberOfProcessors();

// 获得物理内存的总量
__int64 KAmountOfPhysicalMemory();

// 获得磁盘空闲的总量
__int64 KAmountOfFreeDiskSpace(const std::wstring& path);

// 获得操作系统的版本号
void KGetOsVersion(unsigned int *major, unsigned int *minor);

// 获得显示器的个数
int KGetDisplayCount();

// 判断是64位操作系统
BOOL IsX64System();

// 关闭64位的文件重定向
BOOL DisableWow64FsRedirection(void** oldvalue);

// 恢复64位的文件重定向
BOOL RevertWow64FsRedirection(void* oldvalue);

//////////////////////////////////////////////////////////////////////////

#endif	// KSCSYS_INC_

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#include "kxefun.h"
#include "serviceprovider/ikxeipcmanageserviceprovider.h"

int KxEGetServicePort(unsigned short* psPort)
{
	KxEPipeClientT<IKxEIPCManageServiceProvider> ipcClient;
	return ipcClient.GetServicePort(L"", *psPort);
}

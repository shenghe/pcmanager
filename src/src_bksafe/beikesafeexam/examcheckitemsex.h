#pragma once

#include <safeexam/beikesafeexam.h>
using namespace BKSafeExamItemEx;

class CExamCheckItemsEx
{
public:
	CExamCheckItemsEx(void);
	~CExamCheckItemsEx(void);
	//GUEST账户
	int GuestUserIsEnable();
	int SetGuestUserDisable();
	int StartupGuest();
	//共享资源
	int IsHaveShareResource();
	int RemoveAllShareResource();
	int RemoveShareResource(LPCTSTR pszResource);
	int GetShareResourceCount();
	SHARE_RESOURCES EnumAnResources(int nIndex);
	int SetItemShareResourceSafe(LPCTSTR pszRes);
	int SetItemShareResourceUnSafe(LPCTSTR pszRes);
	//远程桌面
	int RemoteDesktopIsEnable();
	int SetRemoteDesktopDisable();
	int StartupRemoteDesktop();
	//组策略
	int GpeditIsDisable();
	int SetGpeditEnable();
	//不能显示隐藏文件
	int ShowHideFileIsEnable();
	int ShowHideFileEnable();
	//修复磁盘文件夹无法打开
	int DriverOrDirectoryOpenIsEnable();
	INT DriverOrDirectoryOpenEnable();
private:
	CSimpleArray<SHARE_RESOURCES> m_arrResource;
	SERVICE_STATUS _GetSrvStatus(LPCTSTR pszSrvName);
	BOOL _StartSrv(LPCTSTR pszSrvName);
};

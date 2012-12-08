#pragma once
#include <string>
/**
*  该类主要实现访问控制
*/
class KAccessControl
{
public:
	/**
	* @brief   设置权限
	* @param   hToken Access token handle
	* @param   privilegeName The name of privilege to enable/disable
	* @param   enablePrivilege To enable or disable privilege
	* @return  bool
	* @example:
	*  HANDLE handle = GetCurrentProcess();
	HANDLE hToken = NULL;		
	OpenProcessToken(handle,(TOKEN_ADJUST_PRIVILEGES |TOKEN_QUERY),&hToken);
	SetPrivilege(hToken, SE_RESTORE_NAME, true);
	CloseHandle(hToken);
	CloseHandle(handle);
	*/
	static bool SetPrivilege(
		HANDLE hToken,          
		const std::wstring&  privilegeName, 
		bool enablePrivilege = true  
		);


	/**
	* @brief   设置当前进程的权限
	* @param   wstring privilegeName
	* @param   bool enablePrivilege
	* @return  bool
	*/
	static bool SetCurrentProcessPrivilege(const std::wstring&  privilegeName, 
		bool enablePrivilege = true );
};

#ifndef  _REGOPT_H_
#define  _REGOPT_H_

#include <windows.h>
#include <atlstr.h>
#include <process.h>
#include <atlconv.h>
#include <atlcoll.h>



#define BEGIN		{
#define END			}

#define		ERROR_NOMEMOEY		0xFF0001;	//内存不足
#define		ERROR_NOFAILKEY		0xFF0002;	//无效键值
#define		ERROR_NODEF			0xFF0003	//无效默认值


typedef struct _REGVALUE
{	
	//值名
	CString		strValueName;	
	//类型
	DWORD		dwType;
	CString		strValue;

}REGVALUE;

typedef CSimpleArray<REGVALUE> VALUEINFO; 

/************************************************************************/
//返回值代表是否继续递归枚举
//HKEY		rootKey			根键
//LPCTSTR	lpItemOrKey		注册表项路径
//LPCTSTR	lpName			值名
//LPBYTE	lpValue			值
//DWORD		dwType			值类型
//void*		pUserData		自定义数据
/************************************************************************/
typedef BOOL (WINAPI *EnumerateRegFun) (HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* pUserData);

class CRegOpt
{
public:
	CRegOpt(){ s_bUserBreak = FALSE; };
	~CRegOpt(){};
public:
	//枚举所有键值
	void	DoRegEnumeration(HKEY rootKey,LPCTSTR lpItemPath,EnumerateRegFun fnRegFun,void* pUserData=NULL);

	//获得当前键下的所有子键
	BOOL	DoEnumCurrnetSubKeyEx(HKEY hRootKey,LPCTSTR lpcKey,CSimpleArray<CString>& vec_Key,BOOL bRes=FALSE,BOOL bFullPath =FALSE);
	BOOL	DoEnumCurrnetSubKey(HKEY hRootKey,LPCTSTR lpcKey,CSimpleArray<CString>& vec_Keys);
	BOOL	DoEnumCurrnetSubKey(HKEY hRootKey,LPCTSTR lpcKey,CAtlMap<CString,char>& map_Keys);
	//获得当前键下的所有值,如果bRes为真则进行递归向下枚举
	BOOL	DoEnumCurrnetValue(HKEY hRootKey,LPCTSTR lpcKey,
				CSimpleArray<REGVALUE>& vec_Value,BOOL bRes=FALSE
				);

	BOOL	CrackRegKey(wchar_t* regstring,HKEY& root, wchar_t*& subkey,wchar_t*& value);
	//获得根键及子键
	BOOL	CrackRegKey(CString& strRegPath,HKEY& root,CString& strSubKey);

	//删除子键
	BOOL	RegDelnode(HKEY hKeyRoot, LPCTSTR lpSubKey);
	//删除值
	BOOL	RegDelValue(HKEY hKeyRoot,LPCTSTR lpSubKey,LPCTSTR lpValue);
	//键是否存在
	BOOL	FindKey(HKEY hKeyRoot,LPCTSTR lpSubKey);
	//获得指定键的默认值
	BOOL	GetDefValue(HKEY hKeyRoot,LPCTSTR lpSubKey,CString& ValueName,CString& Value);
	//获得错误码
	DWORD	GetErrorCode();
	//
	BOOL GetHKEYToString(HKEY hRootKey,LPCTSTR lpszSubKey,CString& strRegFullPath);
	
	void	UserBreadFlag(BOOL bUserBreak) { s_bUserBreak = bUserBreak; };
	BOOL	s_bUserBreak;
private:
	BOOL	RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey);
private:
	DWORD	m_iErrCode;
};

#endif	//_REGOPT_H
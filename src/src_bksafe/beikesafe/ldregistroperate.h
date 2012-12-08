#pragma once

#include "strsafe.h"

namespace RegisterOperate
{
#define REGPATH_MAX_LEN 512
#define FILEPATH_MAX_LEN 512
#define DEFAULT_BUFFER_MAX_LEN 50




#define ZEROMEM(x) ZeroMemory(x, sizeof(x))
#define SAFE_DELETE_PTR(x) if ((NULL != x)){delete (x); x = NULL;}
#define SAFE_DELETE_ARRAY_PTR(p)	if((p)){delete [](p);(p)=NULL;}
	//内部自己使用
#define STRPRINF_STR(x,y) StringCbPrintf((x), sizeof((x)), TEXT("%s"), (y));
#define STRPRINF_INT(x,y) StringCbPrintf((x), sizeof((x)), TEXT("%d"), (y));
#define STRPRINF(x,y,z) StringCbPrintf((x), sizeof((x)), (y), (z));
#define STRCPY(x,y) StringCbCopy((x), sizeof((x)), (y));

	typedef struct _CLSID_STRUCT
	{
		WORD cbSize;
		WCHAR _pszFullRegPath[REGPATH_MAX_LEN];
		WCHAR _pszCLSID[DEFAULT_BUFFER_MAX_LEN];
		WCHAR _pszFileFullPath[FILEPATH_MAX_LEN];
		WCHAR _pszFileRegPath[REGPATH_MAX_LEN];
		WCHAR _pszDes[DEFAULT_BUFFER_MAX_LEN];
		_CLSID_STRUCT()
		{
			cbSize = sizeof(_CLSID_STRUCT);
			ZEROMEM(_pszCLSID);
			ZEROMEM(_pszFileRegPath);
			ZEROMEM(_pszFileFullPath);
			ZEROMEM(_pszFullRegPath);
			ZEROMEM(_pszDes);
		}
	}CLSID_STRUCT , *LPCLSID_STRUCT;

	class CRegisterOperate
	{
	public:
		~CRegisterOperate(void);
		static CRegisterOperate* GetRegOperatePtr()
		{
			static CRegisterOperate regOp;
			return &regOp;
		}
		//获取注册表值(字符型),如果在调用获取注册表值的函数是，注册表类型是字符型，并且是多字节型，但是没有调用专门的多字节函数，这是返回
		//的值，将会把多字节中的每一个信息段，拼接到返回字符床中，中间使用空格来分割
		int GetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPTSTR pszValue, DWORD cbSize);
		//获取注册表值(数值型)
		int GetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, DWORD& dwValue);
		//获取二进制值
		int GetBinaryRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPBYTE pByteValue, DWORD cbSize);
		//设置二进制值
		int SetBinaryRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPCBYTE pByteValue, DWORD cbSize);
		//获取多字节字符值
		int GetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, CAtlArray<CString>& arrString);
		//获取注册表键值类型，因为需要根据不同的类型，调用不同的接口
		DWORD GetRegType(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName);
		//枚举子键
		int EnumSubKey(HKEY hKey, LPCTSTR pszSubPath, CAtlArray<CString>& arrKey);
		//枚举CLSID(这个里面，包含枚举到指定路径下的CLSID以及CLSID对应的文件的路径)
		int EnumCLSIDAndFile(HKEY hKEY, LPCTSTR pszSubPath, CAtlArray<CLSID_STRUCT>& arrClsid);
		// 设置注册表键值 REG_SZ
		int SetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPCTSTR pszValue, DWORD cbSize);
		int SetRegValue2(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPCTSTR pszValue, DWORD cbSize);
		// 设置注册表键值 REG_DWORD
		int SetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, DWORD dwValue);
		//删除键值
		int DeleteRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName);
		//删除键
		int DeleteAnKey(HKEY hKey, LPCTSTR pszSubPath);
		//检测注册表键是否存在
		int KeyIsExist(HKEY hKey, LPCTSTR pszSubPath);
		//枚举键值
		int EnumRegValue(HKEY hKey, LPCTSTR pszSubPath, CAtlArray<CString>& arrRegValue);
		//创建子键
		int CreateAnKey(HKEY hKey, LPCTSTR pszSubPath, HKEY &hNewKey);
	private:
		CRegisterOperate(void);
		//打开注册表键
		HKEY _OpenKey(HKEY hKey, LPCTSTR pszSubPath); 
		void _CloseKey(HKEY hKey);
		//获取REG_SZ注册表键值
		int _GetValue(HKEY hKey, LPCTSTR pszName, LPTSTR pszValue, DWORD dwType, DWORD cbSize, DWORD cbSize2);
		//获取REG_MUL_SZ注册表键值
		int _GetValue_Mul_SZ(HKEY hKey, LPCTSTR pszName, LPTSTR pszValue, DWORD dwType, DWORD cbSize);
		//解析多字节字符串，并把每一个单独的放到arrStr中
		int _ParseMulSzBuf(LPCTSTR pszBuf, CAtlArray<CString>& arrStr);
	};
}



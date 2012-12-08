#ifndef _H_INCLUDE_GLOBAL_DATA_IESCANENG
#define _H_INCLUDE_GLOBAL_DATA_IESCANENG

namespace IEScanEng
{
#define CAPTION_MAX_LEN 100
#define REGPATH_MAX_LEN 512
#define FILEPATH_MAX_LEN 512
#define PATH_MAX_LEN 1024
#define DEFAULT_BUFFER_MAX_LEN 50
#define DES_MAX_LEN 256
#define IESCANENG_VER 1

#define ZEROMEM(x) ZeroMemory(x, sizeof(x))
#define SAFE_DELETE_PTR(x) if ((NULL != x)){delete (x); x = NULL;}

	//回调函数
	typedef int (WINAPI *PFNIEScanEngCallBack)(ULONG uMsg, WPARAM wParam, LPARAM lParam, void* lpConext);

	//////////////////////////////////////////////////////////////////////////
	//结构体

	typedef struct _ITEM_STRUCT_INFO_EXPORT
	{//导出的子项结构
		int _nSubItemID;//子项ID
		int _nGroupID;//组ID
		WCHAR _pszSubItemCaption[CAPTION_MAX_LEN];//子项标题
		_ITEM_STRUCT_INFO_EXPORT()
		{
			_nSubItemID = -1;
			_nGroupID = -1;
			ZEROMEM(_pszSubItemCaption);
		}
	}ITEM_STRUCT_INFO_EXPORT, *LPITEM_STRUCT_INFO_EXPORT;


	typedef struct _GROUP_STRUCT_INFO_EXPORT
	{//导出的分组信息
		int _nGroupID;
		WCHAR _pszGroupCaption[CAPTION_MAX_LEN];
		int _nShow;//是否显示
		int _nCheck;//是否选中

		CAtlArray<ITEM_STRUCT_INFO_EXPORT*> _vetSubItem;
		_GROUP_STRUCT_INFO_EXPORT()
		{
			_nShow = -1;
			_nCheck = -1;
			_nGroupID = -1;
			ZEROMEM(_pszGroupCaption);
			_vetSubItem.RemoveAll();
		}
	}GROUP_STRUCT_INFO_EXPORT, *LPGROUP_STRUCT_INFO_EXPORT;

	typedef struct _SCAN_RESULT_EXPORT
	{//回调函数中导出的结构体
		int _nGroupID;
		int _nSubItemID;
		int _nLevel;
		int _nScanIndex;//这个值表示是扫描结果的一个索引值，是唯一的
		WCHAR _pszPath[PATH_MAX_LEN];//扫描的路径
		WCHAR _pszFilePath[FILEPATH_MAX_LEN];//文件路径信息
		WCHAR _pszCaption[DEFAULT_BUFFER_MAX_LEN];//关于扫描出此项的描述信息
		WCHAR _pszDes[DES_MAX_LEN];//扫描出此项的描述信息
		WCHAR _pszValue[REGPATH_MAX_LEN];//扫描到的值
		WCHAR _pszDefValue[REGPATH_MAX_LEN];//系统默认的值
		WCHAR _pszFixValue[REGPATH_MAX_LEN];//指定使用的修复值
		_SCAN_RESULT_EXPORT()
		{
			_nGroupID = -1;
			_nSubItemID = -1;
			_nScanIndex = -1;
			_nLevel = -1;
			ZEROMEM(_pszCaption);
			ZEROMEM(_pszDes);
			ZEROMEM(_pszPath);
			ZEROMEM(_pszValue);
			ZEROMEM(_pszDefValue);
			ZEROMEM(_pszFixValue);
			ZEROMEM(_pszCaption);
		}
	}SCAN_RESULT_EXPORT, *LPSCAN_RESULT_EXPORT;

	//导出接口
	interface IIEScanEng
	{
		//必须先调用这个接口，如果版本不一致，就会出想初始化失败，下面的接口就不用调用了
		virtual int InitIEScanEng() = 0;
		//根据给定的组ID, 和子项ID来进行扫描，如果nGroup = -1 && nID == -1则枚举所有的，
		//如果nGroup != -1 && nID == -1则枚举指定组下的所有子项, 
		//注意nGroup必须为有效值
		virtual int Scan(int nGroup, int nID, PFNIEScanEngCallBack pfnCallBack, void* lpConext) = 0;
		//根据给定的信息，去执行修复操作
		//注意nGroup必须为有效值
		virtual int Fix(LPSCAN_RESULT_EXPORT lpFixStruct) = 0;
		//通过ID修复，一般都是直接修复成默认值
		virtual int Fix(int nGroup, int nID) = 0;
		//获取安全等级描述信息
		virtual int GetSafeLevel(int nLevel, LPWSTR pszSafeLevel, DWORD cbSize) = 0;
		//获取修复方式描述信息
		virtual int GetFixOp(int nFixOp, LPWSTR pszFixOp, DWORD cbSize) = 0;
		//获取扫描到的分组的个数
		virtual int GetGroupCount() = 0;
		//枚举扫描到的结构体
		virtual GROUP_STRUCT_INFO_EXPORT* EnumGroup(ULONG uIndex = 0) = 0;
		//获取指定的分组
		virtual GROUP_STRUCT_INFO_EXPORT* GetAnGroup(ULONG uGroupID = 0) = 0;
	};
}

#endif
///////////////////////////////////////////////////////////////
//
//	Filename: 	KSearchSoftwarePathFromUninstallList.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-10-30  18:45
//	Comment:	
//
///////////////////////////////////////////////////////////////

#ifndef _KSearchSoftwarePathFromUninstallList_h_
#define _KSearchSoftwarePathFromUninstallList_h_
#include <atlpath.h>
#include <atlrx.h>

//-------------------------------------------------------------
// 
// 通过搜索卸载列表 查找软件所在目录
//
//-------------------------------------------------------------


class KSearchSoftwarePathFromUninstallList
{
public:

	/**
	* @brief   从卸载列表中查找已安装的软件的主目录
	* @param   [in]  pszDisplayName,    [搜索条件1] 卸载列表的显示名称中必须包含该字符串.
											该条件不能为NULL, 且大小写敏感.

	* @param   [in]  pszFileName,       [搜索条件2] 软件的主目录下必须存在指定的文件. 
											文件名不含完整路径, 但可以含相对路径,
											比如: "Plugins\\TraceEraser.dll".
											该条件不能为NULL

	* @param   [out] pszMainDir,        找到的主目录

	* @param   [in]  nOutSize,          保存目录的数组大小, 建议MAX_PATH.

	* @return  搜索成功返回TURE，没有找到返回FALSE
	* @remark  
	*/
	BOOL SearchSoftwarePathFromUninstallList( 
		LPCTSTR  pszDisplayName,
		LPCTSTR  pszFileName,
		LPTSTR   pszMainDir,
		int      nOutSize
		);

private:
	class UNINSTALL_DATA 
	{
	public:
		CString strDisplayName;
		CString strDisplayIcon;
		CString strUninstallString;
	};


private:
	BOOL QueryUninstallData();

	BOOL OnFoundUninstallItem( const UNINSTALL_DATA* pUninstallData );

	BOOL GetPathFromDisplayIcon( LPCTSTR pszDisplayIcon, CString& strPath );

	BOOL GetPathFromUninstallString( LPCTSTR pszUninstallString, CString& strPath );

	BOOL SearchFileInSpecPath( LPCTSTR pszFileName, LPCTSTR pszStartPath, ATL::CPath& strPath );

	BOOL SearchFirstPath( LPCTSTR pszCmdLine, LPCTSTR* ppszStart, LPCTSTR* ppszEnd );

	BOOL GetFirstPath( LPCTSTR pszCmdLine, LPTSTR pszFileName, int nSize );

	BOOL _KillParamEx( LPCTSTR pszCmdLine, LPTSTR pszFileName, int nSize );

	BOOL IsFileExists( LPCTSTR pszFile );

	BOOL IsPossibleSeperator( TCHAR c );

	BOOL IsAlpha( TCHAR c );

	BOOL IsNumber( TCHAR c );

	LPCTSTR HasRundll( LPCTSTR pszCmdLine );

private:
	LPCTSTR  m_pszDisplayName;
	LPCTSTR  m_pszFileName;
	LPTSTR   m_pszMainDir;
	int      m_nOutSize;
};



#endif // _KSearchSoftwarePathFromUninstallList_h_
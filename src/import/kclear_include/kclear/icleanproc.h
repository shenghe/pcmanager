#ifndef _I_CLEANPROC_H_
#define  _I_CLEANPROC_H_

//注册表扫描和清理回调函数定义
typedef BOOL (WINAPI *ScanRegCallBack)  (void* pMain,int iType,LPCTSTR lpcszKeyFullPath,LPCTSTR lpcszValueName,LPCTSTR lpcszValueData);
typedef BOOL (WINAPI *ScanRegCallBackError)  (void* pMainError,int iType,LPCTSTR lpcszKeyFullPathError,LPCTSTR lpcszValueNameError);

//文件扫描和清理回调函数定义
typedef BOOL (WINAPI *ScanFileCallBack) (void* pMainError,int iType,LPCTSTR lpcszFileFullPath,DWORD dwFileLowPart,LONG dwFileHighPart);
typedef BOOL (WINAPI *ScanFileCallBackError) (void* pMainError,int iType,LPCTSTR lpcszFileFullPath,DWORD dwFileLowPart,LONG dwFileHighPart);


/************************************************************************/
//注册表清理结构
/************************************************************************/
//清理注册表结构标准C接口

typedef struct _RegInfo
{	
	int			iType;
	TCHAR		szKeyFullPath[MAX_PATH*4];
	TCHAR		szValueName[MAX_PATH*4];

}RegInfo;

//清理注册表结构非标准C接口(内部使用)
typedef struct _RegInfo_in
{	
	int			iType;
	CString		strKeyFullPath;
	CString		strValueName;

}RegInfo_in;

typedef CSimpleArray<RegInfo_in>		VEC_REGINFO;


/************************************************************************/
//文件清理结构
/************************************************************************/
//清理文件结构标准C接口
typedef struct _FileInfo
{	
	int			iType;
	TCHAR		szFileFullPath[MAX_PATH*4];

}FileInfo;

//清理文件结构非标准C接口(内部使用)
typedef struct _FileInfo_in
{	
	int			iType;
	CString		strFileFullPath;

}FileInfo_in;

typedef CSimpleArray<FileInfo_in>		VEC_FILEINFO;



/************************************************************************/
//垃圾文件通配符结构
/************************************************************************/

//标准C接口
typedef struct  _FILEEXTS
{
	int			iType;						//类型
	TCHAR		szFileExts[MAX_PATH*4];		//文件通配符

}FILEEXTS,*PFILEEXTS;

typedef struct  _SCANPATH
{	
	int			iType;						//类型扫描路径
	TCHAR		szScanFile[MAX_PATH*4];		//文件路径

}SCANPATH,*PSCANPATH;


typedef struct  _GARBAINFO
{
	PFILEEXTS	pFileExts;
	int			iExitsCount;

	PSCANPATH   pScanPath;
	int			iPathCount;

}GARBAINFO;


//非标准C接口内部使用
typedef struct  _FILEEXTS_IN					
{
	int			iType;				//类型
	CString		strFileExts;		//文件通配符

}FILEEXTS_IN;


typedef struct  _SCANPATH_IN
{	
	int			iType;				//类型扫描路径
	CString		strScanFile;		//文件路径

}SCANPATH_IN;

typedef struct  _GARBAINFO_IN
{
	CSimpleArray<FILEEXTS_IN>		vec_fileExtsInfo;
	CSimpleArray<SCANPATH_IN>		vec_scanPathInfo;

}GARBAINFO_IN;


//清理接口
class __declspec(uuid("0E2D6083-2A33-4c8e-9940-ECF0657880FF")) IKClearProc
{
public:
	virtual void DZ_SetCleanCallBack(ScanRegCallBack fnRegCallBack,ScanFileCallBack fnFileCallBack,void* pMain) = 0;
	virtual void DZ_SetCleanCallBackError(ScanRegCallBackError fnRegCallBackError,ScanFileCallBackError fnFileCallBackError,void* pMainError) = 0;

	virtual BOOL DZ_StartScan(int* iType,int iCount) = 0;
	virtual BOOL DZ_StopScan() = 0;

	virtual BOOL DZ_StartScanBage(GARBAINFO* garbInfo) =0;
	virtual BOOL DZ_StopScanBage()=0;

	virtual BOOL DZ_CleanReg(RegInfo* pRegInfo,int iCount) =0;
	virtual BOOL DZ_CleanFile(FileInfo* pFileInfo,int iCount)=0;

	virtual void DZ_IsScanClean(BOOL bClean=FALSE) = 0;

	virtual BOOL DZ_ResetRegsiter(LPCTSTR strRegFilePath=NULL)=0;
	virtual BOOL DZ_GetNoinstallApp(int n) = 0;
};


/************************************************************************/
//清理类型定义
/************************************************************************/
#define	CLEAN_COMPATE						0000			//扫描完成

#define BEGINPROC(x)						(x+100)
#define ENDPROC(x)							(x+200)
#define SCANING(x)							(x+300)

#define IECLEAN								1000
#define	IECLEAN_TEMPFOLDER					1001			//ie临时文件夹
#define	IECLEAN_VISITHOST					1002			//访问网址记录
#define IECLEAN_COOKIE						1003			//cookie文件夹
#define IECLEAN_ADDRBSR						1004			//地址栏
#define IECLEAN_INDEXDATA					1005			//清空index.data	
#define IECLEAN_HISTORY                     1006
#define MAXTHON_COOKIES                     1007             //maxthon cooies
#define THEWORLD_COOKIES                    1008

#define WINSHISTORY							2000
#define WINSHISTORY_RUNDLG					2001			//运行对话框
#define WINSHISTORY_RUNHISTORY				2002			//最近访问过的目录
#define WINSHISTORY_OPENSAVE				2003			//打开与保存记录
#define WINSHISTORY_WINSIZE					2004			//窗口与大小
#define WINSHISTORY_RECENTDOCREG			2005			//最近使用的文档记录(注册表)
#define WINSHISTORY_RECENTDOCFILE			2005			//最近使用的文档记录(文件)
#define WINSHISTORY_FILEEXTS				2007			//文件扩展名历史记录
#define WINSHISTORY_USERASSIST				2008			//最近打开程序历史记录
#define WINSHISTORY_TRAYNOTIFY				2009			//通知区域图标历史记录
#define WINSHISTORY_NETERIVE				2010			//网络驱动器映射 
#define WINSHISTORY_FINDCMP					2011			//查找计算机
#define WINSHISTORY_FINDDOC					2012			//查找文档
#define WINSHISTORY_PRTPORT					2013			//打印端口
#define WINSHISTORY_WINLOG					2014			//windows日志文件
#define WINSHISTORY_WINTEMP					2015			//windows临时文件夹
#define WINSHISTORY_REGEDIT					2016			//注册表最后访问的位置
#define	WINSHISTORY_STREAM					2017			//浏览器注册表流

//其他
#define GOOGL_TOOLBAR						2018			//google工具栏
#define BAIDU_TOOLBAR						2019            //百度工具条搜索记录
#define QQ_TOOLBAR                          2034            //QQ工具条搜索记录
#define WINLIVE_TOOLBAR                     2035            //Windows Live工具条搜索记录

#define WINSHISTORY_PRGHISTORY				2020			//程序访问记录
#define MEMORY_DMP							2021			//内存转存
#define	WINSHISTORY_CLIPBOARD				2029			//剪贴板
#define WINDOWS_RECYCLE                     2030            //回收站
#define WINDOWS_SEARCH                      2031            //Windows 搜索记录
#define WINDOWS_NETHOOD                     2032            //Windows 网上邻居
#define WINDOWS_JUMPLIST                    2033            //Windows 7 跳转列表
#define WINDOWS_SUOLIETU                    2036            //缩略图缓存
//后续实现
//保留 2022-2029



#define REGCLEAN							3000
#define REGCLEAN_INVALIDDLL					3001			//无效动态链接库
#define REGCLEAN_INVALIDMENU				3002			//无效右键菜单
#define REGCLEAN_INVALIDAPPPATH				3003			//无效的应用程序路径
#define REGCLEAN_INVALIDFIRE				3004			//无效的防火墙
#define REGCLEAN_INVALIDMUI					3005			//无效MUI缓存
#define REGCLEAN_INVALIDHELP				3006			//无效的帮助菜单
#define REGCLEAN_INVALIDFONT				3007			//无效的字为实现
#define REGCLEAN_INVALIDINSTALL				3008			//无效的安装程序
#define REGCLEAN_INVALIDUNINSTALL			3009			//无效的反安装程序
#define REGCLEAN_INVALIDSTARTMENU			3010			//无效的开始菜单
#define REGCLEAN_INVALIDSTARTSRUN			3011			//无效的启动项目
#define REGCLEAN_FAILACTIVEX				3012			//错误的ActiveX信息
#define REGCLEAN_FAILAFILEASS				3013			//错误的文件关联
#define REGCLEAN_FAILCLASS					3014			//错误的类信息
#define REGCLEAN_REDUNDANCY					3015			//冗余信息


#define FILEGARBAGE							4000
#define	FILEGARBAGE_RECYCLED				4001			//回收站清理
#define FILEGARBAGE_STARTMENULINK			4002			//无效的开始菜单快捷方式
#define FILEGARBAGE_DESKTOPLINK				4003			//无效的桌面快捷方式
#define FILEGARBAGE_INVALIDDIR				4004			//无用的目录	<未实现,未定义>
#define FILEGARBAGE_EXTS					4005			//根据扩展名进行扫描


#define BROWSERSCLEAN						5000			
#define BROWSERSCLEAN_CHROME				5001			//谷歌浏览器清理
#define	BROWSERSCLEAN_FIREFOX				5002			//火狐浏览器
#define BROWSERSCLEAN_MAXTHON				5003			//遨游浏览器
#define	BROWSERSCLEAN_TT					5004			//腾讯TT浏览器
#define BROWSERSCLEAN_360					5005			//360浏览器
#define BROWSERSCLEAN_WORDWIN				5006			//世界之窗
#define BROWSERSCLEAN_MYIE2					5007			//MYIE2
#define BROWSERSCLEAN_SOGO                  5008            //SOGO搜狗浏览器历史记录
#define SOGO_COOKIES                        5009            //搜狗浏览器历史记录
#define SOGO_FORM                           5010            //搜狗浏览器自动保存的表单
#define SOGO_PASS                           5011            //搜狗浏览器自动保存的密码
#define SOGO_ADVFORM                        5012            //搜狗浏览器智能填表中已保存的表单

#define BROWSERSCLEAN_OPERA                  5013            //Opera浏览器历史记录
#define OPERA_COOKIES                       5014            //Opera浏览器Cookies
#define OPERA_FORM                          5015            //Opera浏览器自动保存的密码

#define IECLEAN_PASSWORD					6001			//ie密码
#define IECLEAN_SAVEFROM					6002			//ie保存表单

#define CHROME_PASSWORD						6003			//chrome密码
#define CHROME_SAVEFROM						6004			//chrome表单

#define FIREFOX_PASSWORD					6005			//firefox密码
#define FIREFOX_SAVEFROM					6006			//firefox表单

#define PASSANDFROM_360						6007			//360保存的用户数据

#define	WORDWIN_USER						6008			//世界之窗用户数据
#define FIREFOX_COOKIES                     6009            //firefox cookies
#define CHROME_COOKIES                      6010            //chrome cookies
#define TECENT_PASSANDFORM                  6011             //TT表单和密码
#define THEWORLD_PASS                       6012              // THE WORLD pass
#define THEWORLD_FORM                       6013             //THE WORLD   from
#define MAXTHON_FORM                        6014             //傲游自动表单
#define MAXTHON_PASS                        6015             //傲游密码
#define PASSANDFROM_TT                      6016              //TT


//聊天IM信息工具
//保留 7001~7005

// 下载工具
#define THUNDERDOWNLOADER                   9037			//迅雷下载
#define FLASHGETDOWNLOADER                  9038			//FlashGet下载
#define BITCOMETDOWNLOADER                  9039			//BitComet下载
#define QQDOWNLOADER                        9044            //QQ旋风

#define PPS_PLAYER                          9041            //PPS 播放器
#define PPTV_PLAYER                         9042            //PPTV 播放器
#define QVOD_PLAYER                         9043            //QVOD 播放器
#define FENGXING_PLAYER                     9045            // fengxing
#define SHESHOU_PLAYER                      9046            //sheshou
#define KUWOMUSIC_PLAYER                    9047             //酷我音乐盒
#define FUXINPDF_READER                     9048             //福昕PDF阅读器历史记录

#define ALIIM_IM                            9049             //阿里旺旺登录信息
#define WINLIVE_MSG                         9050             //WINDOWS LIVES MESSAGER
#define XUNLEI7_DOWNLOADER                  9051             //迅雷7

#endif
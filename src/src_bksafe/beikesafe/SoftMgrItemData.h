#ifndef _SOFTMGR_ITEM_DATA_H_
#define _SOFTMGR_ITEM_DATA_H_
#include <vector>
#include <algorithm>
#include <skylark2/midgenerator.h>

enum SOFT_TYPE
{
	ST_INVALID = -1, ST_IGNORE, ST_FREE, ST_BETA, ST_PAY,
};

enum UPDATE_SOFT_CHECK_TYPE
{
	USCT_INVALID = -1, USCT_ALL, USCT_FREE, USCT_BETA, USCT_PAY
};

// 软件属性
enum SoftAttri : ULONG
{
	SA_None				= (0<<0),
	SA_Beta				= (1<<0),
	SA_Green			= (1<<1),
	SA_Major			= (1<<2),
	SA_New				= (1<<3),
};

class CSoftListItemData
{
public:
	CSoftListItemData()
	{
		m_bIgnore = FALSE;
		m_bDaquan = FALSE;
		m_bChangeIcon = FALSE;
		m_attri = SA_None;
		m_bCancel = FALSE;
		m_bCheck = FALSE;
		m_bCharge = FALSE;
		m_bPlug = FALSE;
		m_bGreen = FALSE;
		m_bSetup = FALSE;
		m_bUpdate = FALSE;
		m_bDownLoad = FALSE;
		m_bDownloading = FALSE;
		m_bContinue = FALSE;
		m_bPause = FALSE;
		m_bLinking = FALSE;
		m_bIcon = FALSE;
		m_bFailed = FALSE;
		m_bLinkFailed = FALSE;
		m_bInstalling = FALSE;
		m_bUninstalling = FALSE;
		m_bWaitInstall = FALSE;
		m_bWaitUninstall = FALSE;
		m_bWaitDownload = FALSE;
		m_bBatchDown = FALSE;
		m_nURLCount = 0;
		m_dwSize = 0;
		m_fMark = 0.0;
		m_llDownloadTime = 0;

		m_nCount = 0;
		m_dwProgress = 0;
		m_dwID = -1;
		m_dwIDUpdate = -1;
		m_pImage = NULL;
		m_dwTime = 0;
		m_fSpeed = 0.0;
		m_dwStartTime = 0;
		m_rcName = CRect(0,0,0,0);
		m_dwFlags = 0;
		m_bUsingForOneKey = FALSE;
	}

#define STR_NEW_DETAIL_URL			L"http://baike.ijinshan.com/ksafe/client/v1_0/%s.html?mid=%s"

	CString GetInfoUrl()
	{
		CString		str;

		if (!m_strSoftID.IsEmpty())
		{
			CString     strMid;
			Skylark::CMidGenerator::Instance().GetMid( strMid );

			str.Format(STR_NEW_DETAIL_URL,m_strSoftID, strMid);
		}

		return str;
	}

public:
	BOOL m_bCheck;				//是否选中
	BOOL m_bCharge;				//是否收费
	BOOL m_bPlug;				//是否有插件
	BOOL m_bGreen;				//是否绿色
	ULONG m_attri;				//软件属性
	BOOL m_bSetup;				//是否安装
	BOOL m_bUpdate;				//是否升级
	BOOL m_bDownloading;		//是否正在下载
	BOOL m_bDownLoad;			//是否已经下载
	BOOL m_bContinue;			//是否继续
	BOOL m_bPause;				//是否暂停
	BOOL m_bLinking;			//正在连接服务器
	BOOL m_bIcon;				//是否load正确的icon
	BOOL m_bFailed;				//下载失败
	BOOL m_bLinkFailed;			//连接失败
	BOOL m_bInstalling;			//正在安装
	BOOL m_bUninstalling;		//正在卸载
	BOOL m_bWaitUninstall;		//等待卸载
	BOOL m_bWaitInstall;		//等在安装
	BOOL m_bWaitDownload;		//等待下载
	BOOL m_bBatchDown;			//是否批量下载
	BOOL m_bCancel;				//是否被取消
	BOOL m_bChangeIcon;			//需要替换图片
	BOOL m_bDaquan;				//是否是大全调用的安装
	BOOL m_bIgnore;				//是否升级忽视

	CString m_strPath;			//下载目录
	CString m_strName;			//软件名称
	CString m_strDescription;	//软件描述
	CString m_strPublished;		//发布时间
	CString m_strURL;			//详情页地址
	CString m_strDownURL;		//下载地址
	CString m_strSpeed;			//下载速度
	CString m_strVersion;		//版本信息
	CString m_strNewVersion;	//更新版本
	CString m_strOSName;		//适合操作系统
	CString m_strIconURL;		//软件图标下载地址
	CString m_strFileName;		//下载文件名
	CString m_strMD5;			//MD5值
	CString m_strType;			//软件类型
	CString m_strTypeShort;		//软件类型
	CString m_strTypeID;		//软件类型ID
	CString m_strSize;			//软件大小
	CString m_strSoftID;		//软件ID
	CString m_strOrder;			//热度
	CString m_strMainExe;		//软件的主程序
	float m_fSpeed;				//下载速度
	DWORD m_dwTime;				//下载所用的时间
	DWORD m_dwProgress;			//下载进度
	DWORD m_dwStartTime;		//刚开始下载时间
	int m_dwID;					//索引ID
	int m_dwIDUpdate;			//只用与升级列表的索引ID
	int m_nURLCount;			//下载连接的URL个数
	DWORD m_dwSize;
	float m_fMark;				//分数
	LONGLONG m_llDownloadTime;	//开始下载时间

	CString m_strCrc;

	CRect m_rcName;				//记录软件名称绘制区域
	CRect m_rcDes;				//记录软件描述绘制区域
	CRect m_rcNewInfo;			//记录新版功能绘制区域
	CRect m_rcFreeback;			//记录反馈区域
	CRect m_rcTautology;		//记录重试区域

	int m_nCount;				//下载图片的次数

	Gdiplus::Image *m_pImage;	//软件图标

	DWORD	m_dwDataID;			// 卸载信息在单独数组中的ID flz
	DWORD	m_dwFlags;			// 是否是分页项

	BOOL	m_bUsingForOneKey;	// 是否一键装机在使用中，屏蔽主界面的操作
};


#define		SOFT_INSTALLED			0
#define		SOFT_UNINSTALLED		1
#define		SOFT_UNINSTALLING		2
#define		SOFT_WAITUNINSTALL		3

enum UNIN_STATUS
{
	US_INVALID = -1, US_UNINSTALLED, US_INSTALLING, US_INSTALLED, US_WAIT_UNINSTALL, US_UNINSTALLING,
};

class CUninstallSoftInfo
{
public:
	CUninstallSoftInfo()
	{
		m_rcName = CRect(0, 0, 0, 0);
		m_rcFreq = CRect(0, 0, 0, 0);
		m_rcDeleteIcon = CRect(0, 0, 0, 0);
		m_rcViewDetail = CRect(0, 0, 0, 0);

		m_nSize = -1;
		m_nLastUse = -1;

		m_bShowDeleteIcon = FALSE;
		m_bRudimental = FALSE;
		m_bCleaned = FALSE;
		m_bNoUninstallInfo = FALSE;
		m_bSetup = TRUE;
		m_bCalcSize = FALSE;
		m_dwDataID = -1;
		m_pImage = NULL;
		m_dwUninstalled = 0;
		m_bVisited = FALSE; 
		m_bHide = FALSE;
		m_bWaitUninstall = FALSE;
		m_bUninstalling = FALSE;
		m_bIcon = FALSE;
		m_bCalcSizing = FALSE;
		m_bWaitCalc = FALSE;

		m_bIconLeg = TRUE;
	}

public:

	// 检测卸载回调中返回的信息
	CString		key;				// 注册表键名 -- 卸载时使用
	CString		name;				// 名称		-- 使用此名称调用 GetUninstallInfo 获得更详细信息 
	CString		disp_icon;			// 本地图标
	CString		loc;				// 本地路径
	CString		uni_cmd;			// 卸载命令
	CString		parent;				// 父软件名 -- 卸载父软件时此软件首先被卸载
	
	// 通过 GetUninstallInfo 获得的信息
	CString		m_strName;			//软件名称
	CString		m_strDescription;	//软件描述
	CString		m_strURL;			//详情页地址
	CString		m_strIconURL;		//软件图标下载地址
	CString		m_strSoftID;		//软件ID
	CString		m_strTypeName;		// 类别名称
	CString		m_strSize;			//大小
	CString		m_strFreq;			//上次使用距离今天的天数
	CString		m_strPath;			//路径

	CRect		m_rcName;
	CRect		m_rcFreq;
	CRect		m_rcDeleteIcon;
	CRect		m_rcViewDetail;

	DWORD		m_dwDataID;				// list 中索引ID
	LONGLONG	m_nSize;				// 用于排序的占用空间
	int			m_nLastUse;				// 用于排序的上次使用时间

	DWORD		m_dwUninstalled;		// 0 -- 未卸载  1 -- 已卸载
	BOOL		m_bSetup;
	BOOL		m_bUninstalling;		//正在卸载
	BOOL		m_bWaitUninstall;		//等待卸载
	BOOL		m_bIcon;
	BOOL		m_bCalcSize;
	BOOL		m_bVisited;				// 递归遍历时使用的标志
	BOOL		m_bHide;
	BOOL		m_bCalcFreq;			//是否可以计算频率
	BOOL		m_bNoUninstallInfo;		//没有卸载项
	BOOL		m_bRudimental;			//残留项
	BOOL		m_bCleaned;				//残留项的清理成功与否
	BOOL		m_bShowDeleteIcon;		//是否显示删除桌面图标
	BOOL		m_bCalcSizing;			//是否正在计算大小
	BOOL		m_bWaitCalc;

	BOOL		m_bIconLeg;				//图片是否合法

	Gdiplus::Image *m_pImage;
};

class CPowerSweepData
{
public:
	CPowerSweepData()
	{
		bChecked = FALSE;
	}

public:
	CString		name;
	CString		ins_loc;				// 安装目录
	CString		m_strType;				// 项目类型：文件、目录、注册表项
	CString		m_strPath;				// 路径
	BOOL		bChecked;				// 是否被选中，要删除
};

class CUninstallPowerSweepData
{
public:
	CUninstallPowerSweepData()
	{
		m_nCallType = 0;
		m_nCheckedNum = 0;

	}

	void Clean()
	{
		m_nCallType = 0;
		m_nCheckedNum = 0;

		for ( int i=0; i<m_arrPowerSweepData.GetSize(); i++ )
		{
			CPowerSweepData * p = m_arrPowerSweepData[i];
			if ( p != NULL )
			{
				delete p;
			}
		}

		m_arrPowerSweepData.RemoveAll();
	}

public:

	int								m_nCallType;		// 0 -- 检测   1 -- 清扫
	CSimpleArray<CPowerSweepData *> m_arrPowerSweepData;
	int								m_nCheckedNum;

};

//-----------------------------------------------------------------------
//管理 CSoftListItemData
typedef std::vector<CSoftListItemData*>		t_vecItemData;
typedef t_vecItemData::iterator		t_iterItemData;
typedef std::pair<t_iterItemData, t_iterItemData>	t_pairItemData;

struct CSoftItemData
{
public:
	void AddData(CSoftListItemData* pData)
	{
		m_vecItemData.push_back(pData);
	}

	const t_vecItemData& GetDataList()
	{
		return m_vecItemData;
	}

	enum SortType
	{ 
		SORT_BY_ID = 0x00, 
		SORT_BY_TYPE,
	};

	void SortData(SortType type)
	{
		switch ( type )
		{
		case SORT_BY_ID:
			{
				std::sort(m_vecItemData.begin(), m_vecItemData.end(), _Guid());
			}
			break;
		case SORT_BY_TYPE:
			{
				std::sort(m_vecItemData.begin(), m_vecItemData.end(), _Type());
			}
			break;
		}
	}

	CSoftListItemData* GetDataByID(LPCWSTR lpSoftID)
	{
		if ( lpSoftID == NULL || wcslen(lpSoftID) == 0 )
			return NULL;

		CSoftListItemData _item;
		_item.m_strSoftID = lpSoftID;

		try
		{
			t_pairItemData _pairRet = std::equal_range(m_vecItemData.begin(), m_vecItemData.end(), &_item, _Guid());
			if ( _pairRet.first != _pairRet.second )
				return (*_pairRet.first);
		}
		catch( ... )
		{

		}

		return NULL;
	}

	CSoftItemData()
	{
		m_vecItemData.clear();
	}

	~CSoftItemData()
	{
		m_vecItemData.clear();
	}

	struct _Guid
	{
		bool operator()(CSoftListItemData* _pLeft, CSoftListItemData* _pRight)
		{
			return (_wcsicmp(_pLeft->m_strSoftID, _pRight->m_strSoftID) < 0);
		}
	};

	struct _Type
	{
		bool operator()(CSoftListItemData* _pLeft, CSoftListItemData* _pRight)
		{
			return (_wcsicmp(_pLeft->m_strType, _pRight->m_strType) < 0);
		}
	};
private:
	t_vecItemData	m_vecItemData;
};
#endif
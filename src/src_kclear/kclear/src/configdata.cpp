// ConfigData.cpp: implementation of the CConfigData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ConfigData.h"
#include "3rdparty/hash/crc32.h"
#include "GetEnvPath.h"
#include "resource.h"
#include "misc/automemmanage.h"
#include "trashdefine.h"
#include "kscbase/kscsys.h"


//////////////////////////////////////////////////////////////////////////
DIRECTORYWORK::DIRECTORYWORK() :
bRecursive(FALSE),
bDefaultSelect(FALSE),
bRSelect(FALSE)
{}

DIRECTORYWORK::DIRECTORYWORK(const DIRECTORYWORK& cpy)
{
    bRecursive     = cpy.bRecursive;
    bDefaultSelect = cpy.bDefaultSelect ;
    bRSelect       = cpy.bRSelect;
    strFriendName  = cpy.strFriendName;
    iconindex      = cpy.iconindex;
    ulSize         = cpy.ulSize;
    id             = cpy.id;
    parent         = cpy.parent;
    bIsLeaf        = cpy.bIsLeaf;

    std::vector<CString>::const_iterator iter = cpy.strExtArray.begin();
    for(;iter != cpy.strExtArray.end();iter++)
        strExtArray.push_back(*iter);

    iter = cpy.strFilt.begin();
    for(;iter != cpy.strFilt.end();iter++)
        strFilt.push_back(*iter);

    iter = cpy.strDirectory.begin();
    for(;iter != cpy.strDirectory.end();iter++)
        strDirectory.push_back(*iter);
    
}

DIRECTORYWORK& DIRECTORYWORK::operator= (const DIRECTORYWORK& cpy)
{
    bRecursive     = cpy.bRecursive;
    bDefaultSelect = cpy.bDefaultSelect;
    strFriendName  = cpy.strFriendName;
    bRSelect       = cpy.bRSelect;
    iconindex      = cpy.iconindex;
    ulSize         = cpy.ulSize;
    id             = cpy.id;
    parent         = cpy.parent;
    bIsLeaf        = cpy.bIsLeaf;

    std::vector<CString>::const_iterator iter = cpy.strExtArray.begin();
    for(;iter != cpy.strExtArray.end();iter++)
        strExtArray.push_back(*iter);

    iter = cpy.strFilt.begin();
    for(;iter != cpy.strFilt.end();iter++)
        strFilt.push_back(*iter);

    iter = cpy.strDirectory.begin();
    for(;iter != cpy.strDirectory.end();iter++)
        strDirectory.push_back(*iter);
    return *this;
}

CConfigData::CConfigData()
{
    m_ConfigData.reserve(100);
}

CConfigData::~CConfigData()
{

}

int CConfigData::ReadConfig()
{
    _ReadConfigFile();
    return RC_SUCCESS;;
}

int CConfigData::ReadOneKeyConfig()
{
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.IE5");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.MSO");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.Outlook");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.Word");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\AntiPhishing");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Virtualized");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Low");
        workItem.strFriendName = L"IE浏览器缓存";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = BROWER_IE;
        m_ConfigData.push_back(workItem);
    }
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\SogouExplorer\\Webkit\\Cache");
        workItem.strFriendName = L"搜狗浏览器高速模式";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 25;
        workItem.id = BROWER_SOGO;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\Maxthon3\\Temp\\Webkit\\Cache");
        workItem.strFriendName = L"傲游浏览器3极速模式";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 23;
        workItem.id = BROWER_MATHRON;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%firefox%");
        workItem.strFriendName = L"火狐 Firefox";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 26;
        workItem.id = BROWER_FIREFOX;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Google\\Chrome\\User Data\\Default\\cache");
        workItem.strFriendName = L"谷歌 Chrome";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 10;
        workItem.id = BROWER_CHROME;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Opera\\Opera\\opcache");
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Opera\\Opera\\cache");
        workItem.strFriendName = L"Opera 浏览器";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 24;
        workItem.id = BROWER_OPERA;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%windir%\\temp");
        workItem.strFriendName = L"Windows 临时目录";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%driver%");
        workItem.strFilt.push_back(L"%program%");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_TEMP;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Microsoft\\Windows\\Explorer");
        workItem.strFriendName = L"缩略图缓存";
        workItem.strExtArray.clear();
        workItem.strExtArray.push_back(L"db");
        workItem.strFilt.clear();
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_SUOLIETU;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"");
        workItem.strFriendName = L"回收站";
        workItem.strFilt.push_back(L"");
        workItem.id = WIN_HUISHOUZHAN;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%windir%\\SoftwareDistribution\\Download");
        workItem.strFriendName = L"Windows 更新补丁";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_UPDATE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%systemdrive%\\MSOCache");
        workItem.strFriendName = L"Office 安装缓存目录";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_OFFICE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%windir%\\Prefetch");
        workItem.strFriendName = L"Windows 预读文件";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_PREFETCH;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%windir%\\Downloaded Program Files");
        workItem.strFriendName = L"已下载程序文件";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%downloadlist%");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_DOWNLOAD;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%TEMP%");
        workItem.strDirectory.push_back(L"%usertemp%");
        workItem.strFriendName = L"用户临时文件夹";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%windir%");
        workItem.strFilt.push_back(L"%driver%");
        workItem.strFilt.push_back(L"%program%");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_USERTEMP;
        m_ConfigData.push_back(workItem);

    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"%archivefiles%");
        workItem.strDirectory.push_back(L"%queuefiles%");
        workItem.strDirectory.push_back(L"%systemarchivefiles%");
        workItem.strDirectory.push_back(L"%systemqueuefiles%");
        workItem.strFriendName = L"Windows错误报告";
        workItem.strExtArray.clear();
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.clear();
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_ERRPORT;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = FALSE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"%windir%");
        workItem.strDirectory.push_back(L"%windir%\\debug");
        workItem.strDirectory.push_back(L"%windir%\\system32\\wbem");
        workItem.strDirectory.push_back(L"%windir%\\security\\logs");
        workItem.strFriendName = L"系统日志文件";
        workItem.strExtArray.clear();
        workItem.strExtArray.push_back(L"log");
        workItem.strExtArray.push_back(L"lo_");
        workItem.strExtArray.push_back(L"log.txt");
        workItem.strExtArray.push_back(L"bak");
        workItem.strExtArray.push_back(L"old");
        workItem.strFilt.clear();
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.id = WIN_LOG;
        m_ConfigData.push_back(workItem);
    }

    return RC_SUCCESS;;
}

int CConfigData::WriteConfig()
{
    return RC_FAILED;
}

BOOL CConfigData::ConvToLocatePath()
{
    if (m_ConfigData.size() <= 0)
        return FALSE;

    CGetEnvPath evnPath;


    for (size_t i = 0; i < m_ConfigData.size(); i++)
    {
        DIRECTORYWORK& dirword = m_ConfigData[i];



        for (size_t iIndex = 0; iIndex < dirword.strDirectory.size(); iIndex++)
        {
            CString strPath = dirword.strDirectory.at(iIndex);
            strPath = evnPath.GetRealPath(strPath);
            strPath.TrimLeft();
            strPath.TrimRight();
            dirword.strDirectory.at(iIndex) = strPath;
        }

        if (dirword.id == BROWER_IE)
        {
            _CheckChangeIE(dirword);
        }
    }

    return TRUE;
}
BOOL CConfigData::_CheckChangeIE(DIRECTORYWORK& dirword)
{
    BOOL bRet = FALSE;
    CGetEnvPath evnPath;
    TCHAR szBuffer[MAX_PATH] = { 0 };
    CString strPath;

    UINT uResult = GetSystemDirectory(szBuffer, MAX_PATH);

    if (uResult < 3 || szBuffer[1] != TEXT(':') || szBuffer[2] != TEXT('\\'))
    {
        goto Clear0;     
    }
    szBuffer[3] = TEXT('\0');
    strPath += szBuffer;
    memset(szBuffer, 0, sizeof(szBuffer));
    DWORD MaxSize = sizeof(szBuffer);
    GetUserName(szBuffer, &MaxSize);

    if (KGetWinVersion() >= 4) // win7
    {
        strPath += L"Users\\"; 
        strPath += szBuffer; 
        strPath += L"\\AppData\\Local\\Microsoft\\Windows\\Temporary Internet Files\\";

        if (strPath.CompareNoCase(dirword.strDirectory.at(0)) == 0)
        {
            dirword.strDirectory.clear();
            dirword.strDirectory.resize(1);
        }
    }
    else
    {
        strPath += L"Documents and Settings\\"; 
        strPath += szBuffer; 
        strPath += L"\\Local Settings\\Temporary Internet Files\\";

        if (strPath.CompareNoCase(dirword.strDirectory.at(0)) == 0)
        {
            dirword.strDirectory.clear();
            dirword.strDirectory.resize(1);
        }
    }
    bRet = TRUE;
Clear0:

    dirword.strDirectory.at(0) = strPath;

    return bRet;
}

CConfigData::DirWorkArray& CConfigData::GetConfigData()
{
    return m_ConfigData;
}

const CConfigData::DirWorkArray& CConfigData::GetConfigData() const
{
    return m_ConfigData;
}

DIRECTORYWORK& CConfigData::GetConfigItemByID(int nItemID)
{
    int iCount;

    for (iCount = 0; iCount < (int)m_ConfigData.size(); ++iCount)
    {
        if (nItemID == m_ConfigData[iCount].id)
        {
             break;
        }
    }
    if (iCount == m_ConfigData.size())
        return m_ConfigData[0];

    return m_ConfigData[iCount];
}

/************************************************************************/
/* 说明：添加顺序与扫描顺序一样，所以调整后需要关注扫描顺序             */
/************************************************************************/
int CConfigData::_ReadConfigFile()
{

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strFriendName = L"浏览器缓存目录";
        workItem.iconindex = 9;
        workItem.id = BROWER_TRASH;
        workItem.parent = NONE;
        workItem.bIsLeaf = FALSE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\");//根据这个判断是否IE默认缓存是否被修改过
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.IE5");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.MSO");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.Outlook");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Content.Word");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\AntiPhishing");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Virtualized");
        workItem.strDirectory.push_back(L"%CSIDL_INTERNET_CACHE%\\Low");
        workItem.strFriendName = L"IE\\360\\腾讯TT\\傲游\\世界之窗\\搜狗浏览器";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 3;
        workItem.id = BROWER_IE;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\SogouExplorer\\Webkit\\Cache");
		workItem.strFriendName = L"搜狗浏览器（高速模式）";
		workItem.strExtArray.push_back(L"*");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 25;
		workItem.id = BROWER_SOGO;
		workItem.parent = BROWER_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\Maxthon3\\Temp\\Webkit\\Cache");
		workItem.strFriendName = L"傲游3（极速模式）";
		workItem.strExtArray.push_back(L"*");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 23;
		workItem.id = BROWER_MATHRON;
		workItem.parent = BROWER_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%firefox%");
        workItem.strFriendName = L"火狐（Firefox）";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 26;
        workItem.id = BROWER_FIREFOX;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Google\\Chrome\\User Data\\Default\\cache");
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Google\\Chrome\\User Data\\Default\\Media Cache");
//        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Google\\Chrome\\User Data\\Default");
		workItem.strFriendName = L"谷歌(Chrome)";
		workItem.strExtArray.push_back(L"*");
//         workItem.strExtArray.push_back(L"data_*");
//         workItem.strExtArray.push_back(L"f_*");
//         workItem.strExtArray.push_back(L"index");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 10;
		workItem.id = BROWER_CHROME;
		workItem.parent = BROWER_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Opera\\Opera\\opcache");
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Opera\\Opera\\cache");
        workItem.strFriendName = L"Opera";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 24;
        workItem.id = BROWER_OPERA;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Apple Computer\\Safari");
        workItem.strFriendName = L"Apple Safari";
        workItem.strExtArray.push_back(L"Cache.db");
//        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 24;
        workItem.id = BROWER_SAFARI;
        workItem.parent = BROWER_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
	//   视频
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strFriendName = L"视频播放缓存目录";
        workItem.iconindex = 11;
        workItem.id = VIDEO_TRASH;
        workItem.parent = NONE;
        workItem.bIsLeaf = FALSE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Microsoft\\Media Player\\Transcoded Files Cache");
        workItem.strFriendName = L"windows media player";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 0;
        workItem.id = VIDEO_MEDIA_PALYER;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = FALSE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%pptv%");
		workItem.strFriendName = L"PPTV网络电视";
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 14;
		workItem.id = VIDEO_PPTV;
		workItem.parent = VIDEO_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = FALSE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%qqlive%");
		workItem.strFriendName = L"QQLive网络电视";
		workItem.strExtArray.push_back(L"tdl");
		workItem.strExtArray.push_back(L"tdi");
		workItem.strExtArray.push_back(L"tii");
		workItem.strExtArray.push_back(L"tpt");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 15;
		workItem.id = VIDEO_QQLIVE;
		workItem.parent = VIDEO_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = FALSE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%xunleikankan%");
		workItem.strFriendName = L"迅雷看看";
		workItem.strExtArray.push_back(L"cfg");
		workItem.strExtArray.push_back(L"dat");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 18;
		workItem.id = VIDEO_XUNLEI;
		workItem.parent = VIDEO_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = FALSE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%fengxing%");
		workItem.strFriendName = L"风行网络电视";
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 20;
		workItem.id = VIDEO_FENGXING;
		workItem.parent = VIDEO_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = FALSE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%youku%");
		workItem.strFriendName = L"优酷(iku)加速器";
		workItem.strExtArray.push_back(L"dat");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 19;
		workItem.id = VIDEO_YOUKU;
		workItem.parent = VIDEO_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%tudou%");
        workItem.strFriendName = L"飞速土豆";
        workItem.strExtArray.push_back(L"tdd!");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 17;
        workItem.id = VIDEO_TUDOU;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%ku6%");
        workItem.strFriendName = L"极速酷6";
        workItem.strExtArray.push_back(L"k6p");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_KU6;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%storm%");
        workItem.strFriendName = L"暴风影音";
        workItem.strExtArray.push_back(L"");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_STORM;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%qvod%");
        workItem.strFriendName = L"Qvod播放器";
        workItem.strExtArray.push_back(L"!mv");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_QVOD;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%ppstream%");
        workItem.strFriendName = L"PPS";
        workItem.strExtArray.push_back(L"ppsds.pgf");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_PPS;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%qqmusic%");
        workItem.strFriendName = L"QQMusic";
        workItem.strExtArray.push_back(L"tdl");
        workItem.strExtArray.push_back(L"tii");
        workItem.strExtArray.push_back(L"tdi");
        workItem.strExtArray.push_back(L"tpt");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_QQMUSIC;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%kuwo%");
        workItem.strFriendName = L"KUWO";
        workItem.strExtArray.push_back(L"wma");
        workItem.strExtArray.push_back(L"pack");
        workItem.strExtArray.push_back(L"mkv");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_KUWO;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%kugoo%");
        workItem.strFriendName = L"KuGoo";
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_KUGOO;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%pipi%");
        workItem.strFriendName = L"PIPIStream";
        workItem.strExtArray.push_back(L"jfrec");
        workItem.strExtArray.push_back(L"inpart");
        workItem.strExtArray.push_back(L"jfenc");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_PIPI;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\TTPlayer\\cache");
        //workItem.strDirectory.push_back(L"%ttplayer%");
        workItem.strFriendName = L"千千静听";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_TTPLAYER;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\SogouExplorer\\SogouVideoCache");
        workItem.strDirectory.push_back(L"%sogou%");
        workItem.strFriendName = L"搜狗浏览器视频缓存";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 12;
        workItem.id = VIDEO_SOGOU;
        workItem.parent = VIDEO_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

	// windows 
    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = FALSE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strFriendName = L"Windows系统垃圾文件";
        workItem.iconindex = 5;
        workItem.id = WIN_TRASH;
        workItem.parent = NONE;
        workItem.bIsLeaf = FALSE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"");
        workItem.strFriendName = L"回收站";
        workItem.strExtArray.clear();
        workItem.strExtArray.push_back(L"");
        workItem.strFilt.clear();
        workItem.strFilt.push_back(L"");
        workItem.iconindex = 30;
        workItem.id = WIN_HUISHOUZHAN;
        workItem.parent = WIN_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%TEMP%");
        workItem.strDirectory.push_back(L"%usertemp%");
		workItem.strFriendName = L"用户临时文件夹";
		workItem.strExtArray.push_back(L"*");
		workItem.strFilt.push_back(L"%windir%");
		workItem.strFilt.push_back(L"%driver%");
		workItem.strFilt.push_back(L"%program%");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 7;
		workItem.id = WIN_USERTEMP;
		workItem.parent = WIN_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = TRUE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%windir%\\temp");
        workItem.strDirectory.push_back(L"%wintemp%");
        workItem.strFriendName = L"Windows 临时目录";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%driver%");
        workItem.strFilt.push_back(L"%program%");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 5;
        workItem.id = WIN_TEMP;
        workItem.parent = WIN_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.clear();
		workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Microsoft\\Windows\\Explorer");
		workItem.strFriendName = L"缩略图缓存";
		workItem.strExtArray.clear();
		workItem.strExtArray.push_back(L"db");
		workItem.strFilt.clear();
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 29;
		workItem.id = WIN_SUOLIETU;
		workItem.parent = WIN_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.clear();
		workItem.strDirectory.push_back(L"%archivefiles%");
		workItem.strDirectory.push_back(L"%queuefiles%");
		workItem.strDirectory.push_back(L"%systemarchivefiles%");
		workItem.strDirectory.push_back(L"%systemqueuefiles%");
		workItem.strFriendName = L"Windows错误报告";
		workItem.strExtArray.clear();
		workItem.strExtArray.push_back(L"*");
		workItem.strFilt.clear();
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 28;
		workItem.id = WIN_ERRPORT;
		workItem.parent = WIN_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%windir%\\SoftwareDistribution\\Download");
		workItem.strFriendName = L"Windows 更新补丁";
		workItem.strExtArray.push_back(L"*");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 6;
		workItem.id = WIN_UPDATE;
		workItem.parent = WIN_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = TRUE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%windir%\\Prefetch");
		workItem.strFriendName = L"Windows 预读文件";
		workItem.strExtArray.push_back(L"pf");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 4;
		workItem.id = WIN_PREFETCH;
		workItem.parent = WIN_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}
	{
		DIRECTORYWORK workItem;
		workItem.bRecursive = TRUE;
		workItem.bDefaultSelect = FALSE;
		workItem.bRSelect = workItem.bDefaultSelect;
		workItem.strDirectory.push_back(L"%windir%\\Downloaded Program Files");
		workItem.strFriendName = L"已下载程序文件";
		workItem.strExtArray.push_back(L"*");
		workItem.strFilt.push_back(L"%downloadlist%");
		workItem.strFilt.push_back(L"%specfiles%");
		workItem.iconindex = 1;
		workItem.id = WIN_DOWNLOAD;
		workItem.parent = WIN_TRASH;
		workItem.bIsLeaf = TRUE;
		m_ConfigData.push_back(workItem);
	}

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = FALSE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"%minidump%");
        workItem.strDirectory.push_back(L"%memdump%");
        workItem.strFriendName = L"内存转储文件";
        workItem.strExtArray.clear();
        workItem.strExtArray.push_back(L"dmp");
        workItem.strFilt.clear();
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 22;
        workItem.id = WIN_DUMP;
        workItem.parent = WIN_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = FALSE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"%windir%\\");
        workItem.strDirectory.push_back(L"%windir%\\debug");
        workItem.strDirectory.push_back(L"%windir%\\system32\\wbem");
        workItem.strDirectory.push_back(L"%windir%\\security\\logs");
        workItem.strFriendName = L"系统日志文件";
        workItem.strExtArray.clear();
        workItem.strExtArray.push_back(L"log");
        workItem.strExtArray.push_back(L"lo_");
        workItem.strExtArray.push_back(L"log.txt");
        workItem.strExtArray.push_back(L"bak");
        workItem.strExtArray.push_back(L"old");
        workItem.strFilt.clear();
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 27;
        workItem.id = WIN_LOG;
        workItem.parent = WIN_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Microsoft\\Terminal Server Client\\Cache");
        workItem.strFriendName = L"远程桌面";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 0;
        workItem.id = WIN_REMOTE_DESKTOP;
        workItem.parent = WIN_TRASH;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);

    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%ksafe%");
        workItem.strFriendName = L"卫士缓存";
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 0;
        workItem.id = SOFT_KSAFE;
        workItem.parent = COMMON_SOFTWARE;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%office%");
        workItem.strFriendName = L"Office 安装缓存目录";
        workItem.strExtArray.push_back(L"*");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 0;
        workItem.id = WIN_OFFICE;
        workItem.parent = COMMON_SOFTWARE;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);

    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%CSIDL_APPDATA%\\Microsoft\\MSN Messenger");
        workItem.strDirectory.push_back(L"%CSIDL_PROGRAM_FILES_COMMON%\\Windows Live\\.cache");
        workItem.strFriendName = L"Windows Live Messenger";
        workItem.strExtArray.push_back(L"msi");
        workItem.strExtArray.push_back(L"sqm");
        workItem.strExtArray.push_back(L"ini");
        workItem.strExtArray.push_back(L"exe");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 30;
        workItem.id = WIN_LIVE_MGR;
        workItem.parent = COMMON_SOFTWARE;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.clear();
        workItem.strDirectory.push_back(L"%CSIDL_LOCAL_APPDATA%\\Google\\Picasa2\\db3");
        workItem.strFriendName = L"picasa";
        workItem.strExtArray.push_back(L"bigthumbs_0.db");
        workItem.strExtArray.push_back(L"previews_0.db");
        workItem.iconindex = 0;
        workItem.id = SOFT_PICASA;
        workItem.parent = COMMON_SOFTWARE;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }

    {
        DIRECTORYWORK workItem;
        workItem.bRecursive = TRUE;
        workItem.bDefaultSelect = FALSE;
        workItem.bRSelect = workItem.bDefaultSelect;
        workItem.strDirectory.push_back(L"%systemdrive%");
        workItem.strFriendName = L"系统盘垃圾文件深度搜索";
        workItem.strExtArray.push_back(L"tmp");
        workItem.strExtArray.push_back(L"_mp");
        workItem.strExtArray.push_back(L"gid");
        workItem.strExtArray.push_back(L"chk");
        workItem.strExtArray.push_back(L"??$");
        workItem.strExtArray.push_back(L"^*");
        workItem.strExtArray.push_back(L"temp");
        workItem.strExtArray.push_back(L"~mp");
        workItem.strExtArray.push_back(L"$*");
        workItem.strExtArray.push_back(L"___");
        workItem.strExtArray.push_back(L"$$$");
        workItem.strExtArray.push_back(L"@@@");
        workItem.strExtArray.push_back(L"---");
        workItem.strExtArray.push_back(L"ftg");
        workItem.strExtArray.push_back(L"%%%");
        workItem.strExtArray.push_back(L"fts");
        workItem.strExtArray.push_back(L"prv");
        workItem.strExtArray.push_back(L"fnd");
        workItem.strExtArray.push_back(L"err");
        workItem.strExtArray.push_back(L"diz");
        workItem.strExtArray.push_back(L"mch");
        workItem.strExtArray.push_back(L"mscreate.dir");
        workItem.strExtArray.push_back(L"imagedb.aid");
        workItem.strExtArray.push_back(L"errorlog.txt");
        workItem.strExtArray.push_back(L"comsetup.txt");
        workItem.strExtArray.push_back(L"directx.txt");
        workItem.strExtArray.push_back(L"mpcsetup.txt");
        workItem.strExtArray.push_back(L"netsetup.txt");
        workItem.strExtArray.push_back(L"thumbs.db");
        workItem.strFilt.push_back(L"%specfiles%");
        workItem.iconindex = 2;
        workItem.id = SYS_TRASH;
        workItem.parent = NONE;
        workItem.bIsLeaf = TRUE;
        m_ConfigData.push_back(workItem);
    }
    return TRUE;
}
int CConfigData::_WriteConfigFile()
{
    int nResult = TRUE;
    
    return nResult;
}

BOOL CConfigData::InitFiltDir()
{
   

    CGetEnvPath evnPath;

    unsigned int uFilterCount = sizeof(g_pszFilterDirs) / sizeof(LPCTSTR);

    for (unsigned int i = 0; i < uFilterCount; i++)
    {
        CString strPath;
        strPath = evnPath.GetRealPath(g_pszFilterDirs[i].GetBuffer());
        g_pszFilterDirs[i] = strPath.GetBuffer();
        
    }

    return TRUE;
}

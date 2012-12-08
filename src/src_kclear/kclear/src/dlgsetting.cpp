#include "stdafx.h"
#include "dlgsetting.h"
#include "kclear/libheader.h"
#include "kclrfactory.h"

//////////////////////////////////////////////////////////////////////////

KClearSettingDlg::KClearSettingDlg() 
    : CKuiDialogImpl<KClearSettingDlg>("IDR_DLG_SETTING")
    , m_extListCtrl(m_extList)
    , m_bCleanVCacheByTime(TRUE)
    , m_bEnableExtScan(FALSE)
	, m_bLoadSettiing(FALSE)
    , m_bScanCustomSelect(TRUE)
    , m_nReportLimit(150)
    , m_bFilterVedioTime(FALSE)
    , m_bEnableMonitor(TRUE)
{
    LoadDefaultSetting();
    LoadSetting();
}

KClearSettingDlg::~KClearSettingDlg()
{
}

//////////////////////////////////////////////////////////////////////////

BOOL KClearSettingDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
    InitUi();
    FillData();
    
    return TRUE;
}

void KClearSettingDlg::OnOK()
{
    m_bScanCustomSelect = GetItemCheck(IDC_RADIO_TRASH_CUSTOM);

    m_bFilterVedioTime = GetItemCheck(IDC_CHK_TRASH_VEDIO_TIME);
    
    m_nReportLimit = (int)m_boxSizeLimit.GetItemData(m_boxSizeLimit.GetCurSel());

    m_bEnableMonitor = GetItemCheck(IDC_CHK_TRASH_MONITOR);

    SaveSetting();
    EndDialog(IDOK);
}

void KClearSettingDlg::OnCancel()
{
    EndDialog(IDCANCEL);
}

void KClearSettingDlg::OnExtListSize(CRect rcZone)
{
    ::MoveWindow(m_extListCtrl, rcZone.left, rcZone.top, rcZone.Width(), rcZone.Height(), TRUE);
}

void KClearSettingDlg::OnComboboxReSize(CRect rcZone)
{
    m_boxSizeLimit.MoveWindow(rcZone);
}

void KClearSettingDlg::InitUi()
{
     // 创建下面的大小限制   
    m_boxSizeLimit.Create(GetRichWnd(), 
                           NULL, 
                           NULL, 
                           WS_CHILD|WS_VISIBLE|WS_DISABLED|CBS_DROPDOWNLIST|CBS_HASSTRINGS, 
                           0, 
                           IDC_CTL_TRASH_LIMIT_COMBOX
                           );
    m_boxSizeLimit.SetFont(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
    m_boxSizeLimit.InsertString(0, _T("50"));
    m_boxSizeLimit.SetItemData(0, 50);
    m_boxSizeLimit.InsertString(1, _T("150"));
    m_boxSizeLimit.SetItemData(1, 150);
    m_boxSizeLimit.InsertString(2, _T("300"));
    m_boxSizeLimit.SetItemData(2, 300);
    m_boxSizeLimit.InsertString(3, _T("500"));
    m_boxSizeLimit.SetItemData(3, 500);
    m_boxSizeLimit.InsertString(4, _T("1000"));
    m_boxSizeLimit.SetItemData(4, 1000);
    m_boxSizeLimit.SetCurSel(1);

    m_boxSizeLimit.EnableWindow(TRUE);

    for (int iCount = 0; iCount < 5; ++iCount)
    {
        if (m_nReportLimit == m_boxSizeLimit.GetItemData(iCount))
        {
            m_boxSizeLimit.SetCurSel(iCount);
            break;
        }
    }

    if (m_bScanCustomSelect)
    {
        SetItemCheck(IDC_RADIO_TRASH_RECOMMAND, FALSE);
        SetItemCheck(IDC_RADIO_TRASH_CUSTOM, TRUE);
    }
    else
    {
        SetItemCheck(IDC_RADIO_TRASH_CUSTOM, FALSE);
        SetItemCheck(IDC_RADIO_TRASH_RECOMMAND, TRUE);
    }
    

    SetItemCheck(IDC_CHK_TRASH_VEDIO_TIME, m_bFilterVedioTime);

    DWORD dwStyle = WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDRAWFIXED;
    m_extListCtrl.Create(GetRichWnd(), NULL, NULL, dwStyle, 0, IDC_CTL_EXT_LIST);
    DWORD dwExStyle = LVS_EX_FULLROWSELECT;
    dwExStyle |= m_extListCtrl.GetExtendedListViewStyle();
    m_extListCtrl.SetExtendedListViewStyle(dwExStyle);
    m_extListCtrl.InsertColumn(0, L" 类型", LVCFMT_LEFT, 120); 
    m_extListCtrl.InsertColumn(1, L" 说明", LVCFMT_LEFT, 160); 
    m_extListCtrl.SetFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));
    m_extListCtrl.Init();

    SetItemCheck(IDC_CHK_TRASH_MONITOR, m_bEnableMonitor);
}

BOOL KClearSettingDlg::LoadDefaultSetting()
{
    BOOL retval = FALSE;
    CStringA strXml;
    TCHAR szModule[MAX_PATH] = { 0 };
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlSetting = NULL;
    TiXmlElement* pXmlExts = NULL;
    TiXmlElement* pXmlItem = NULL;
    TiXmlElement* pVodCache = NULL;
    TiXmlElement* pExtScan = NULL;
    int nCleanByTime = 0;
    int nEnableExtScan = 0;

    m_extList.clear();

    GetModuleFileName(NULL, szModule, MAX_PATH);
    PathRemoveFileSpec(szModule);
    PathAppend(szModule, _T("data\\trashconf.dat"));
    CDataFileLoader datloader;
    if (!datloader.LoadFile(szModule, strXml))
        goto Exit0;

    xmlDoc.Parse(strXml);

    pXmlSetting = xmlDoc.FirstChildElement("setting");
    if (!pXmlSetting)
        goto Exit0;

    pXmlExts = pXmlSetting->FirstChildElement("exts");
    if (pXmlExts)
    {
        pXmlItem = pXmlExts->FirstChildElement("ext");
        while (pXmlItem)
        {            
            ExtEntry entry;

            TiXmlElement* pName = pXmlItem->FirstChildElement("name");
            if (!pName)
            {
                pXmlItem = pXmlItem->NextSiblingElement("ext");  
                continue;
            }

            TiXmlElement* pDescription = pXmlItem->FirstChildElement("description");
            if (!pDescription)
            {
                pXmlItem = pXmlItem->NextSiblingElement("ext");  
                continue;
            }

            TiXmlElement* pDefaulted = pXmlItem->FirstChildElement("defaulted");
            if (!pDefaulted)
            {
                pXmlItem = pXmlItem->NextSiblingElement("ext");  
                continue;
            }

            std::string strText = pDefaulted->GetText();
            entry.name = KUTF8_To_UTF16(pName->GetText());
            entry.description = KUTF8_To_UTF16(pDescription->GetText());
            entry.checked = (strText != "0");

            m_extList.push_back(entry);
            pXmlItem = pXmlItem->NextSiblingElement("ext");     
        }
    }

    pVodCache = pXmlSetting->FirstChildElement("vcache");
    if (pVodCache)
    {
        pVodCache->Attribute("cleanbytime", &nCleanByTime);
        m_bCleanVCacheByTime = nCleanByTime ? TRUE : FALSE;
    }

    pExtScan = pXmlSetting->FirstChildElement("extscan");
    if (pExtScan)
    {
        pExtScan->Attribute("enable", &nEnableExtScan);
        m_bEnableExtScan = nEnableExtScan ? TRUE : FALSE;
    }

    retval = TRUE;

Exit0:
    return retval;
}


BOOL KClearSettingDlg::LoadSetting()
{
    BOOL retval = FALSE;
    TCHAR szModule[MAX_PATH] = { 0 };
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlSetting = NULL;
    TiXmlElement* pXmlExts = NULL;
    TiXmlElement* pXmlItem = NULL;
    FILE* pFile = NULL;
    TiXmlElement* pVodCache = NULL;
    TiXmlElement* pExtScan = NULL;
	TiXmlElement* pFirstDay = NULL;
	TiXmlElement* pSelectedItems = NULL;
	TiXmlElement* pTotalSize = NULL;
	TiXmlElement* pLastDay = NULL;
    TiXmlElement* pScanCustom = NULL;
    TiXmlElement* pReportSize = NULL;
    TiXmlElement* pFilter = NULL;
    int nCleanByTime = 0;
    int nEnableExtScan = 0;
    IKClearMonitorSetting* piMonitorSetting = NULL;

    piMonitorSetting = KClearFactory::Instance().CreateMonitorSetting();
    if (piMonitorSetting)
    {
        piMonitorSetting->GetEnableMointor(&m_bEnableMonitor);
    }

    GetModuleFileName(NULL, szModule, MAX_PATH);
    PathRemoveFileSpec(szModule);
    PathAppend(szModule, _T("cfg\\trashconf.xml"));

    pFile = _wfopen(szModule, L"rb");
    if (!pFile)
        goto Exit0;

    if (!xmlDoc.LoadFile(pFile))
        goto Exit0;

    pXmlSetting = xmlDoc.FirstChildElement("setting");
    if (!pXmlSetting)
        goto Exit0;

    pXmlExts = pXmlSetting->FirstChildElement("exts");
    if (!pXmlExts)
        goto Exit0;

    pXmlItem = pXmlExts->FirstChildElement("ext");
    while (pXmlItem)
    {
        TiXmlElement* pName = pXmlItem->FirstChildElement("name");
        if (!pName)
        {
            pXmlItem = pXmlItem->NextSiblingElement("ext");
            continue;
        }

        TiXmlElement* pChecked = pXmlItem->FirstChildElement("checked");
        if (!pChecked)
        {
            pXmlItem = pXmlItem->NextSiblingElement("ext");
            continue;
        }

        std::string strText = pChecked->GetText();
        SetChecked(KUTF8_To_UTF16(pName->GetText()), strText != "0");

        pXmlItem = pXmlItem->NextSiblingElement("ext");
    }

    pVodCache = pXmlSetting->FirstChildElement("vcache");
    if (pVodCache)
    {
        pVodCache->Attribute("cleanbytime", &nCleanByTime);
        m_bCleanVCacheByTime = nCleanByTime ? TRUE : FALSE;
    }

    pExtScan = pXmlSetting->FirstChildElement("extscan");
    if (pExtScan)
    {
        pExtScan->Attribute("enable", &nEnableExtScan);
        m_bEnableExtScan = nEnableExtScan ? TRUE : FALSE;
    }

	pFirstDay = pXmlSetting->FirstChildElement("firstday");
	if (pFirstDay)
    {
        m_strFirstDay = pFirstDay->Attribute("value");
    }

	pTotalSize = pXmlSetting->FirstChildElement("size");
	if (pTotalSize)
    {
        m_strTotalSize = pTotalSize->Attribute("value");
    }

	pSelectedItems = pXmlSetting->FirstChildElement("selectItem");
	if (pSelectedItems)
    {
        m_strSelected = pSelectedItems->Attribute("value");
    }

	pLastDay = pXmlSetting->FirstChildElement("last");
	if (pLastDay)
    {
	    m_strLastSize = pLastDay->Attribute("size");
	    m_strLastDay = pLastDay->Attribute("day");
    }
	
    pScanCustom = pXmlSetting->FirstChildElement("customexam");
    if (pScanCustom)
    {
        pScanCustom->Attribute("value", &m_bScanCustomSelect);
        const char *pDef = pScanCustom->Attribute("def");
        if (!pDef)
        {
            m_bScanCustomSelect = TRUE;
            m_bDefaultCustom = TRUE;
        }
    }

    pReportSize = pXmlSetting->FirstChildElement("examstandard");
    if (pReportSize)
    {
        pReportSize->Attribute("value", &m_nReportLimit);
    }

    pFilter = pXmlSetting->FirstChildElement("vediotime");
    if (pFilter)
    {
        pFilter->Attribute("value", &m_bFilterVedioTime);
    }

    retval = TRUE;

Exit0:
    if (piMonitorSetting)
    {
        piMonitorSetting->Release();
        piMonitorSetting = NULL;
    }

    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }
	m_bLoadSettiing = retval;
	return retval;
}

BOOL KClearSettingDlg::SaveSetting()
{
    BOOL retval = FALSE;
    TiXmlDocument xmlDoc;
    TiXmlElement* pXmlItem = NULL;
    TiXmlElement* pXmlTime = NULL;
    TiXmlElement* pXmlItems = NULL;
    TiXmlElement* pXmlSetting = NULL;
    TiXmlElement* pVodCache = NULL;
    TiXmlElement* pExtScan = NULL;
	TiXmlElement* pSelectedItems = NULL;
	TiXmlElement* pTotalSize = NULL;
	TiXmlElement* pFirstDay = NULL;
	TiXmlElement* pLastDay = NULL;
    TiXmlElement* pScanCustom = NULL;
    TiXmlElement* pReportSize = NULL;
    TiXmlElement* pFilterVedio = NULL;
    TCHAR szConfPath[MAX_PATH] = { 0 };
    TiXmlDeclaration *pXmlDecl = new TiXmlDeclaration("1.0", "utf-8", "yes"); 
    size_t i;
    IKClearMonitorSetting* piMonitorSetting = NULL;

    piMonitorSetting = KClearFactory::Instance().CreateMonitorSetting();
    if (piMonitorSetting)
    {
        piMonitorSetting->SetEnableMointor(m_bEnableMonitor);
    }

    GetModuleFileName(NULL, szConfPath, MAX_PATH);
    PathRemoveFileSpec(szConfPath);
    PathAppend(szConfPath, _T("cfg\\trashconf.xml"));

    xmlDoc.LinkEndChild(pXmlDecl);

    pXmlSetting = new TiXmlElement("setting");
    if (!pXmlSetting)
        goto clean0;

    pXmlItems = new TiXmlElement("exts");
    if (!pXmlItems)
        goto clean0;

    for (i = 0; i < m_extList.size(); ++i)
    {
        TiXmlElement* pName = NULL;
        TiXmlElement* pChecked = NULL;
        TiXmlText* pText1 = NULL;
        TiXmlText* pText2 = NULL;

        pXmlItem = new TiXmlElement("ext");
        if (!pXmlItem)
            goto clean0;

        pName = new TiXmlElement("name");
        if (!pName)
            goto clean0;

        pChecked = new TiXmlElement("checked");
        if (!pChecked)
            goto clean0;

        pText1 = new TiXmlText(KUTF16_To_UTF8(m_extList[i].name.c_str()));
        if (!pText1)
            goto clean0;

        pText2 = new TiXmlText(m_extList[i].checked ? "1" : "0");
        if (!pText2)
            goto clean0;
		
        pName->LinkEndChild(pText1);
        pChecked->LinkEndChild(pText2);

        pXmlItem->LinkEndChild(pName);
        pXmlItem->LinkEndChild(pChecked);

        pXmlItems->LinkEndChild(pXmlItem);
    }

    pVodCache = new TiXmlElement("vcache");
    if (!pVodCache)
        goto clean0;

    pVodCache->SetAttribute("cleanbytime", m_bCleanVCacheByTime ? 1 : 0);

    pExtScan = new TiXmlElement("extscan");
    if (!pExtScan)
        goto clean0;

    pExtScan->SetAttribute("enable", m_bEnableExtScan ? 1 : 0);

	pTotalSize = new TiXmlElement("size");
	if (!pTotalSize)
		goto clean0;

	pTotalSize->SetAttribute("value", m_strTotalSize.c_str());

	pSelectedItems = new TiXmlElement("selectItem");
	if (!pSelectedItems)
		goto clean0;

	pSelectedItems->SetAttribute("value", m_strSelected.c_str());

	pLastDay = new TiXmlElement("last");
	if (!pLastDay)
		goto clean0;
	pLastDay->SetAttribute("day", m_strLastDay.c_str());
	pLastDay->SetAttribute("size", m_strLastSize.c_str());
	
	pFirstDay = new TiXmlElement("firstday");
	if (!pFirstDay)
		goto clean0;

	{
		if (m_strFirstDay.empty())
			m_strFirstDay = "0";
		pFirstDay->SetAttribute("value", m_strFirstDay.c_str());
	}

    pScanCustom = new TiXmlElement("customexam");
    if (!pScanCustom)
        goto clean0;
    pScanCustom->SetAttribute("value", m_bScanCustomSelect);
    pScanCustom->SetAttribute("def", m_bDefaultCustom);

    pReportSize = new TiXmlElement("examstandard");
    if (!pReportSize)
        goto clean0;
    pReportSize->SetAttribute("value", m_nReportLimit);

    pFilterVedio = new TiXmlElement("vediotime");
    if (!pFilterVedio)
        goto clean0;

    pFilterVedio->SetAttribute("value", m_bFilterVedioTime);

    pXmlSetting->LinkEndChild(pFilterVedio);
    pXmlSetting->LinkEndChild(pReportSize);
    pXmlSetting->LinkEndChild(pScanCustom);
	pXmlSetting->LinkEndChild(pFirstDay);
	pXmlSetting->LinkEndChild(pTotalSize);
	pXmlSetting->LinkEndChild(pLastDay);
	pXmlSetting->LinkEndChild(pSelectedItems);

    pXmlSetting->LinkEndChild(pXmlItems);
    pXmlSetting->LinkEndChild(pVodCache);
    pXmlSetting->LinkEndChild(pExtScan);

    xmlDoc.LinkEndChild(pXmlSetting);


    if (!xmlDoc.SaveFile(KUTF16_To_ANSI(szConfPath)))
        goto clean0;

    retval = TRUE;

clean0:
    if (piMonitorSetting)
    {
        piMonitorSetting->Release();
        piMonitorSetting = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
LRESULT KClearSettingDlg::OnNoneChecked(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_bEnableExtScan = FALSE;
	SetItemCheck(IDC_CHK_EXT_SCAN, m_bEnableExtScan);
	m_extListCtrl.EnableWindow(m_bEnableExtScan);
	return 0;
}
//////////////////////////////////////////////////////////////////////////

void KClearSettingDlg::SetChecked(const std::wstring& name, bool checked)
{
    std::vector<ExtEntry>::iterator i;

    for (i = m_extList.begin(); i != m_extList.end(); ++i)
    {
        if (i->name == name)
        {
            i->checked = checked;
            break;
        }
    }
}

void KClearSettingDlg::FillData()
{
    size_t i;

    for (i = 0; i < m_extList.size(); ++i)
    {
        m_extListCtrl.InsertItem((int)i, m_extList[i].name.c_str());
        m_extListCtrl.SetItemText((int)i, 1, m_extList[i].description.c_str());
    }

    SetItemCheck(IDC_CHK_CLEAN_VCACHE_BYTIME, m_bCleanVCacheByTime);
    SetItemCheck(IDC_CHK_EXT_SCAN, m_bEnableExtScan);

    m_extListCtrl.EnableWindow(m_bEnableExtScan);
}

BOOL KClearSettingDlg::GetExtList(std::vector<std::wstring>& vExts)
{
    BOOL retval = FALSE;
    size_t i;

    vExts.clear();

    if (m_extList.empty())
        goto clean0;

    for (i = 0; i < m_extList.size(); ++i)
    {
        if (m_extList[i].checked)
            vExts.push_back(m_extList[i].name);
    }

    retval = TRUE;

clean0:
    return retval;
}

void KClearSettingDlg::OnCleanVCacheByTime()
{
    m_bCleanVCacheByTime = GetItemCheck(IDC_CHK_CLEAN_VCACHE_BYTIME);
}

BOOL KClearSettingDlg::CleanVideoCacheByTime()
{
    return m_bCleanVCacheByTime;
}

BOOL KClearSettingDlg::IsExtScanEnable()
{
    return m_bEnableExtScan;
}

void KClearSettingDlg::OnExtScan()
{
    m_bEnableExtScan = GetItemCheck(IDC_CHK_EXT_SCAN);
    m_extListCtrl.EnableWindow(m_bEnableExtScan);
}

//////////////////////////////////////////////////////////////////////////
void KClearSettingDlg::SetCustomSelected(std::vector<UINT>& selectedItems)
{
	int iCount = 0;
	m_strSelected = "";
	for (iCount = 0; iCount < (int)selectedItems.size(); ++iCount)
	{
		if (iCount != 0)
		{
			m_strSelected += ",";
		}
		char buf[10] = {0};
		itoa(selectedItems[iCount], buf, 10);
		m_strSelected += buf;
	}
}
BOOL KClearSettingDlg::GetCustomSelected(std::vector<UINT>& selectedItems)
{
	int iCount = 0;
	BOOL bRet = FALSE;
	int iTemp = 0;
	char buf[10];
	if (!m_bLoadSettiing)
		goto Clear0;

	for (iCount = 0; iCount < (int)m_strSelected.length(); ++iCount)
	{
		if (m_strSelected[iCount] == ',')
		{
			buf[iTemp] = 0;
			selectedItems.push_back(atoi(buf));
			iTemp = 0;
			continue;
		}
		buf[iTemp ++] = m_strSelected[iCount];
		
		if (iCount + 1 == m_strSelected.length())
		{
			buf[iTemp] = 0;
			selectedItems.push_back(atoi(buf));
		}
	}

	bRet = TRUE;
Clear0:
	return bRet;
}
void KClearSettingDlg::SetClearTotalSize(ULONGLONG& qSize)
{

	m_strTotalSize = "";
	char buf[100] = {0};
	sprintf(buf, "%I64d", qSize);
	m_strTotalSize = buf;
}
BOOL KClearSettingDlg::GetClearTotalSize(ULONGLONG& qSize)
{
	BOOL bRet = FALSE;
	
	if (!m_bLoadSettiing)
		goto Clear0;

	qSize = 0;

	sscanf(m_strTotalSize.c_str(),"%I64d", &qSize);

	bRet = TRUE;
Clear0:
	return bRet;
}
void KClearSettingDlg::SetClearLastDay(time_t& lastTime)
{
	m_strLastDay = "";
	char buf[100] = {0};
	sprintf(buf, "%I64d", lastTime);
	m_strLastDay = buf;
}
BOOL KClearSettingDlg::GetClearLastDay(time_t& lastTime)
{
	BOOL bRet = FALSE;

	if (!m_bLoadSettiing)
		goto Clear0;
	lastTime = 0;

	sscanf(m_strLastDay.c_str(), "%I64d", &lastTime);

	bRet = TRUE;
Clear0:
	return bRet;
}
void KClearSettingDlg::SetLastSize(ULONGLONG& qSize)
{
	m_strLastSize = "";
	char buf[100] = {0};
	sprintf(buf, "%I64d", qSize);
	m_strLastSize = buf;
}
BOOL KClearSettingDlg::GetLastSize(ULONGLONG& qSize)
{
	BOOL bRet = FALSE;

	if (!m_bLoadSettiing)
		goto Clear0;
	qSize = 0;

	sscanf(m_strLastSize.c_str(),"%I64d", &qSize);

	bRet = TRUE;
Clear0:
	return bRet;
}
void KClearSettingDlg::SetFirstDay(time_t& qFirstDay)
{
	m_strFirstDay = "";
	char buf[100] = {0};
	sprintf(buf, "%I64d", qFirstDay);
	m_strFirstDay = buf;
}
BOOL KClearSettingDlg::GetClearFirstDay(CString& t)
{
	BOOL bRet = FALSE;
	std::wstring strFormat;
	if (!m_bLoadSettiing)
		goto Clear0;

	strFormat = AnsiToUnicode(m_strFirstDay);

	t.Format(L"%s", strFormat.c_str());

	bRet = TRUE;
Clear0:
	return bRet;
}

BOOL KClearSettingDlg::GetReportSize(int& nSize)
{
    BOOL bRet = FALSE;

    nSize = 0;

    if (!m_bLoadSettiing)
        goto Clear0;

    nSize = m_nReportLimit;

    bRet = TRUE;
Clear0:
    return bRet; 
}
BOOL KClearSettingDlg::CheckCustomSelect()
{
    return m_bScanCustomSelect;
}

BOOL KClearSettingDlg::IsFilterVedioTime()
{
    return m_bFilterVedioTime;
}
//////////////////////////////////////////////////////////////////////////
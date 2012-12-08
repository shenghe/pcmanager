#pragma once

#include <wtlhelper/whwindow.h>
#include "kuires.h"
#include "image3tabtree.h"
#include <vector>
#include "uihandler_onekey.h"
#include <algorithm>
#include "kscbase/kscsys.h"

class CKscOneKeyConfig
	: public CKuiDialogImpl<CKscOneKeyConfig>
	, public CWHRoundRectFrameHelper<CKscOneKeyConfig>
{
public:
	CKscOneKeyConfig(CKscMainDlg* refDialog, BOOL bSetting)
		: CKuiDialogImpl<CKscOneKeyConfig>("IDR_DLG_FOLDER_BROWSE"),
        m_dlg(refDialog)
	{
        Is_SelALL = FALSE;
		m_bSetting = bSetting;
	}

	~CKscOneKeyConfig(void){}

    std::vector<CString> m_defaultvec;
    std::vector<CString> m_customvec;

protected:
	 CKscMainDlg*        m_dlg;
     CSimpleArray<ClearTreeType>		m_arrClearType;
     CSimpleArray<ClearTreeType>		m_arrClearType2;
     CSimpleArray<ClearTreeInfo>		m_arrClearInfo;
     CAtlMap<CString, TreeInfo>		    m_TreeHenjiMap;
     CWH3StateCheckTreeCtrl10 m_Tree;
     BOOL Is_SelALL;
	 std::wstring m_strMakeSure;
	 BOOL m_bSetting;

     void initTreeContent();
     static BOOL WINAPI SoftTreeCallBack(void* pThis,ClearTreeInfo treeinfo);
     static BOOL WINAPI SoftInfoCallBack(void* pThis,SoftInfo softinfo);
    
public:

	
	void OnBkBtnClose()
	{
		EndDialog(0);
	}

    void SplitString(CString str, WCHAR split, std::vector <CString>& vcResult)
    {
        int iPos = 0; //分割符位置
        int iNums = 0; //分割符的总数
        CString strTemp = str;
        CString strRight;
        //先计算子字符串的数量

        CString strLeft;
        while (true)
        {
            iPos = strTemp.Find(split);
            if(iPos == -1)
                break;
            //左子串
            strLeft = strTemp.Left(iPos);
            //右子串
            strRight = strTemp.Mid(iPos + 1, strTemp.GetLength());
            strTemp = strRight;
            vcResult.push_back(strLeft);
        }
    }

    void OnMakeSure()
    {
		if (this->GetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL))
		{
			this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, TRUE);
// 			COneCleanSetting& setting = COneCleanSetting::Instance();
// 			setting.Set("makesure", L"yes");
		}
		else
		{
			this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, FALSE);
// 			COneCleanSetting& setting = COneCleanSetting::Instance();
// 			setting.Set("makesure", L"no");
		}

    }

    BOOL OnInitDialog(HWND wParam, LPARAM lParam)
    {
        m_dlg->SetItemVisible(ID_IMG_ONEKEY_CONFIG_GREENTIP, TRUE);
        m_Tree.Create(this->GetViewHWND(), NULL, NULL, WS_TABSTOP | WS_CHILD | TVS_CHECKBOXES | TVS_HASLINES | TVS_HASBUTTONS | TVS_FULLROWSELECT | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS, 
            0, IDC_TREE_CLR_GAOJI, NULL );
        m_Tree.SetFont(KuiFontPool::GetFont(FALSE, FALSE, FALSE));
		COneCleanSetting& setting = COneCleanSetting::Instance();
		setting.Get("makesure", m_strMakeSure);
		if (m_strMakeSure.compare(L"yes") == 0)
		{
			this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, TRUE);
		}
		else
		{
			this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, FALSE);
		}
        

        initTreeContent();
		if (m_bSetting)
		{
			this->SetItemText(ID_IMG_ONEKEY_CONFIG_OK, L"确认并开始清理");
		}
		else
		{
			this->SetItemText(ID_IMG_ONEKEY_CONFIG_OK, L"保存清理范围");
		}

        CString strMoudlePath;
        wchar_t szAppPath[MAX_PATH] = { 0 };


        GetModuleFileNameW(NULL, szAppPath, MAX_PATH);
        PathRemoveFileSpecW(szAppPath);
        strMoudlePath = szAppPath;
        strMoudlePath += L"\\";

        strMoudlePath +=L"data\\icon";
        DWORD dwsize = 0;
        //加载图标
        CAtlMap<CString,CString> mapIDvsBitName;
        for (int i=0;i<m_arrClearType.GetSize();i++)
        {
            ClearTreeType treetype = m_arrClearType[i];
            CString strUrl = treetype.strBmpurl;
            int n = strUrl.ReverseFind('/');
            if(n>0)strUrl.Delete(0, n+1);
            else strUrl = L"";
            mapIDvsBitName.SetAt(treetype.strTypeid, strUrl);

            for(int i = 0;i<m_arrClearType2.GetSize();i++)
            {
                ClearTreeType treetype2 = m_arrClearType2[i];
                if(treetype2.nSn = treetype.nSn)
                {
                    CString strUrl = treetype2.strBmpurl;
                    int n = strUrl.ReverseFind('/');
                    if(n>0)strUrl.Delete(0, n+1);
                    else strUrl = L"";
                    mapIDvsBitName.SetAt(treetype2.strTypeid, strUrl);

                    if(treetype2.nSn == 2)
                    {
                        for (int i=0;i<m_arrClearInfo.GetSize();i++)
                        {
                            ClearTreeInfo treeInfo = m_arrClearInfo[i];

                            if(treeInfo.strTypeid == treetype2.strTypeid)
                            {

                                strUrl = treeInfo.strBmpurl;
                                n = strUrl.ReverseFind('/');
                                if(n>0)strUrl.Delete(0, n+1);
                                else strUrl = L"";
                                mapIDvsBitName.SetAt(treeInfo.strClearid, strUrl);
                            }
                        }
                    }

                }

            }    

        }

        m_Tree.Init3State("IDB_CLR_TREE_ICON",strMoudlePath, mapIDvsBitName);
        m_Tree.SetDlghWnd(this->m_hWnd,3);

        for (int i=0;i<m_arrClearType.GetSize();i++)
        {
            ClearTreeType treetype = m_arrClearType[i];


            TVINSERTSTRUCT tvInsert;
            tvInsert.hParent = NULL;
            tvInsert.hInsertAfter = NULL;
            tvInsert.item.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_STATE;
            tvInsert.item.pszText = treetype.strTypeName.GetBuffer(treetype.strTypeName.GetLength());;
            tvInsert.item.hItem=NULL;
            tvInsert.item.state=INDEXTOSTATEIMAGEMASK(2);
            tvInsert.item.stateMask=TVIS_STATEIMAGEMASK;
            tvInsert.item.iImage=m_Tree.SelectBit(treetype.strTypeid);
            tvInsert.item.iSelectedImage=tvInsert.item.iImage;
            tvInsert.item.cChildren=0;
            tvInsert.item.lParam=0;
            HTREEITEM hCurrent = m_Tree.InsertItem( &tvInsert );


            for(int i=0;i<m_arrClearType2.GetSize();i++)
            {

                ClearTreeType treetype2 = m_arrClearType2[i];

                if(treetype2.nSn == treetype.nSn)
                {

                    tvInsert.hParent=hCurrent;
                    tvInsert.item.iImage=0;
                    tvInsert.item.pszText=treetype2.strTypeName.GetBuffer(treetype2.strTypeName.GetLength());
                    if(treetype2.nSn == 1 || treetype2.nSn == 3)
                    {
                        if(std::find(m_customvec.begin(),m_customvec.end(),treetype2.strTypeid)!=m_customvec.end())
                        {
                            treetype2.nCheck = 1;
                        }
                        else
                        {
                            treetype2.nCheck = 0;
                        }
                        tvInsert.item.state=INDEXTOSTATEIMAGEMASK(treetype2.nCheck+1);
                    }
                   
                    tvInsert.item.iImage=m_Tree.SelectBit(treetype2.strTypeid);
                    tvInsert.item.iSelectedImage=tvInsert.item.iImage;
                    HTREEITEM hSub2 = m_Tree.InsertItem(&tvInsert);

                    if (treetype2.nSn == 1 || treetype2.nSn == 3)
                    {

                        TreeInfo treeinfo;
                        treeinfo.hItem = hSub2;
                        treeinfo.iCheck = treetype2.nCheck+1;
                        treeinfo.strItemInfo = treetype2.strTypeName;
                        treeinfo.nSn = treetype2.nSn;
                        treeinfo.nType = 0;
                        m_TreeHenjiMap.SetAt(treetype2.strTypeid, treeinfo);

                    }

                    if(treetype.nSn == 2)
                    {
                        for (int i=0;i<m_arrClearInfo.GetSize();i++)
                        {
                            ClearTreeInfo treeInfo = m_arrClearInfo[i];

                            if(treeInfo.strTypeid == treetype2.strTypeid)
                            {

                                tvInsert.hParent=hSub2;
                                tvInsert.item.iImage=0;
                                tvInsert.item.pszText=treeInfo.strClearName.GetBuffer(treeInfo.strClearName.GetLength());

                                if(std::find(m_customvec.begin(),m_customvec.end(),treeInfo.strClearid)!=m_customvec.end())
                                {
                                    treeInfo.nCheck = 1;
                                }
                                else
                                {
                                    treeInfo.nCheck = 0;
                                }

                                tvInsert.item.state=INDEXTOSTATEIMAGEMASK(treeInfo.nCheck+1);//
                                tvInsert.item.iImage=m_Tree.SelectBit(treeInfo.strClearid);
                                tvInsert.item.iSelectedImage=tvInsert.item.iImage;
                                HTREEITEM hSub = m_Tree.InsertItem(&tvInsert);

                                m_Tree._ResetParentState(hSub);

                                TreeInfo treeinfo;
                                treeinfo.hItem = hSub;
                                treeinfo.iCheck = treeInfo.nCheck+1;
                                treeinfo.strItemInfo = treeInfo.strClearName;
                                treeinfo.nSn = treeInfo.nSn;
                                treeinfo.nType = 0;//atoi(CW2A(treeInfo.strTypeid));
                                m_TreeHenjiMap.SetAt(treeInfo.strClearid, treeinfo);

                            }
                        }
                    }
                }
            }
        }

        HTREEITEM hRoot = m_Tree.GetRootItem();
        m_Tree.Expand( hRoot);
        HTREEITEM hCurrent = m_Tree.GetNextItem( hRoot, TVGN_CHILD );
        while ( hCurrent != NULL )
        {
            m_Tree.Expand( hCurrent );
            hCurrent = m_Tree.GetNextItem( hCurrent, TVGN_NEXT );
        }

        m_Tree.Select( hRoot, TVGN_FIRSTVISIBLE );
        m_Tree.ResetItemState(hRoot);
        if(m_Tree.IsAllSelect())
        {
           // this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, TRUE);
            Is_SelALL = TRUE;
        }
        else 
        {
           // this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, FALSE);
            Is_SelALL = FALSE;
        }

        return TRUE;
    }


    LRESULT OnWmLBtn( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        
        if(m_Tree.IsAllSelect())
        {
            //this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, TRUE);
        }
        else 
        {
            //this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, FALSE);
        }

        if(m_Tree.IsAllNoSelect())
        {
            this->EnableItem(ID_IMG_ONEKEY_CONFIG_OK, FALSE);
        }
        else
        {
            this->EnableItem(ID_IMG_ONEKEY_CONFIG_OK, TRUE);
        }
  
        return TRUE;
    }

    void OnBkBtnSave()
    {
        CString strKeyTmp = _T("");
        POSITION pos;
        pos = m_TreeHenjiMap.GetStartPosition();
        while (pos!=NULL)
        {
            CString key = m_TreeHenjiMap.GetKeyAt(pos);
            TreeInfo treeinfo = m_TreeHenjiMap.GetNextValue(pos);
            if(m_Tree.GetCheckStateEx(treeinfo.hItem))
            {
            
                strKeyTmp += key;
                strKeyTmp += L",";
            }
            
        }

		if (this->GetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL))
		{
			COneCleanSetting& setting = COneCleanSetting::Instance();
			setting.Set("makesure", L"yes");
		}
		else
		{
			COneCleanSetting& setting = COneCleanSetting::Instance();
			setting.Set("makesure", L"no");
		}

        std::wstring m_strCus(strKeyTmp.GetBuffer());

        COneCleanSetting& setting = COneCleanSetting::Instance();

        setting.Set("custom_entrys", m_strCus);
		if (m_bSetting)
		{
			EndDialog(1);
		}
		else
		{
			EndDialog(0);
		}

    }

    void OnSetRecommand()
    {

        POSITION pos;
        pos = m_TreeHenjiMap.GetStartPosition();
        while (pos!=NULL)
        {
            CString key = m_TreeHenjiMap.GetKeyAt(pos);
            TreeInfo treeinfo = m_TreeHenjiMap.GetNextValue(pos);
            if(std::find(m_defaultvec.begin(),m_defaultvec.end(),key)!=m_defaultvec.end())
            {

                m_TreeHenjiMap[key].iCheck = 2;
                m_Tree.SetState(treeinfo.hItem,2);
                m_Tree._ResetParentState(treeinfo.hItem);
            }
            else
            {
                m_TreeHenjiMap[key].iCheck = 1;
                m_Tree.SetState(treeinfo.hItem,1);
                m_Tree._ResetParentState(treeinfo.hItem);

           
            }

        }
        this->EnableItem(ID_IMG_ONEKEY_CONFIG_OK, TRUE);
      //  this->SetItemCheck(ID_IMG_ONEKEY_CONFIG_SELALL, FALSE);
        Is_SelALL = FALSE;
        
    }




public:
	KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
        KUI_NOTIFY_ID_COMMAND(ID_IMG_ONEKEY_CONFIG_OK, OnBkBtnSave)
        KUI_NOTIFY_ID_COMMAND(ID_IMG_ONEKEY_CONFIG_CANCEL, OnBkBtnClose)
        KUI_NOTIFY_ID_COMMAND(ID_IMG_ONEKEY_CONFIG_SELALL, OnMakeSure)
        KUI_NOTIFY_ID_COMMAND(ID_IMG_ONEKEY_CONFIG_RECOMMAND, OnSetRecommand)
    KUI_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CKscOneKeyConfig)
		MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        MESSAGE_HANDLER_EX(MSG_CLR_CONFIG_TREELUP,OnWmLBtn)
		CHAIN_MSG_MAP(CKuiDialogImpl<CKscOneKeyConfig>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CKscOneKeyConfig>)
		MSG_WM_INITDIALOG(OnInitDialog)
		END_MSG_MAP()
};

typedef BOOL (*pCreateObject)( REFIID , void** );


void CKscOneKeyConfig::initTreeContent()
{
  
    CString strMoudlePath, strClean, strZClean;
    CString	m_ScanLogFile;
    CString m_CacheFile;
    wchar_t szAppPath[MAX_PATH] = { 0 };
    
    //初始化树
    m_arrClearType.RemoveAll();
    m_arrClearInfo.RemoveAll();
    ClearTreeType cleartreetype;

    cleartreetype.nSn = 1;
    cleartreetype.strTypeName = _T("系统垃圾");
    cleartreetype.strTypeid = _T("1");
    cleartreetype.strBmpurl = _T("//laji.bmp");
    m_arrClearType.Add(cleartreetype);

    cleartreetype.nSn = 2;
    cleartreetype.strTypeName = _T("使用痕迹");
    cleartreetype.strTypeid = _T("2");
    cleartreetype.strBmpurl = _T("//henji.bmp");
    m_arrClearType.Add(cleartreetype);



    ClearTreeType cleartreetype2;

//     cleartreetype2.nSn = 2;
//     cleartreetype2.strTypeName = _T("上网历史痕迹");
//     cleartreetype2.strTypeid = _T("101");
//     cleartreetype2.strBmpurl = _T("//Intenethistory.bmp");
//     m_arrClearType2.Add(cleartreetype2);
// 
//     cleartreetype2.nSn = 2;
//     cleartreetype2.strTypeName = _T("保存的网页表单和密码");
//     cleartreetype2.strTypeid = _T("103");
//     cleartreetype2.strBmpurl = _T("//password.bmp");
//     m_arrClearType2.Add(cleartreetype2);
// 
//     cleartreetype2.nSn = 2;
//     cleartreetype2.strTypeName = _T("系统历史记录");
//     cleartreetype2.strTypeid = _T("104");
//     cleartreetype2.strBmpurl = _T("//syshistory.bmp");
//     m_arrClearType2.Add(cleartreetype2);
// 
//     cleartreetype2.nSn = 2;
//     cleartreetype2.strTypeName = _T("应用程序历史记录");
//     cleartreetype2.strTypeid = _T("105");
//     cleartreetype2.strBmpurl = _T("//application.bmp");
//     m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("Internet Explorer上网痕迹");
	cleartreetype2.strTypeid = _T("101");
	cleartreetype2.strBmpurl = _T("//IEpassword.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("火狐浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("102");
	cleartreetype2.strBmpurl = _T("//FirefoxMac.bmp");
	m_arrClearType2.Add(cleartreetype2);


	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("谷歌浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("103");
	cleartreetype2.strBmpurl = _T("//googleChrome.bmp");
	m_arrClearType2.Add(cleartreetype2);


	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("傲游浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("104");
	cleartreetype2.strBmpurl = _T("//maxthon.bmp");
	m_arrClearType2.Add(cleartreetype2);


	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("搜狗浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("105");
	cleartreetype2.strBmpurl = _T("//sogo.bmp");
	m_arrClearType2.Add(cleartreetype2);


	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("Opera浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("106");
	cleartreetype2.strBmpurl = _T("//opera_.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("世界之窗浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("108");
	cleartreetype2.strBmpurl = _T("//windowofworld.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("其他浏览器上网痕迹");
	cleartreetype2.strTypeid = _T("109");
	cleartreetype2.strBmpurl = _T("//aboutIE.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("浏览器工具条搜索痕迹");
	cleartreetype2.strTypeid = _T("112");
	cleartreetype2.strBmpurl = _T("//toolbar.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("Windows系统历史痕迹");
	cleartreetype2.strTypeid = _T("110");
	cleartreetype2.strBmpurl = _T("//sys.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("Windows系统自带程序痕迹");
	cleartreetype2.strTypeid = _T("111");
	cleartreetype2.strBmpurl = _T("//filetype.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("办公软件");
	cleartreetype2.strTypeid = _T("115");
	cleartreetype2.strBmpurl = _T("//office.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("媒体播放软件");
	cleartreetype2.strTypeid = _T("116");
	cleartreetype2.strBmpurl = _T("//kmplayer.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("下载软件");
	cleartreetype2.strTypeid = _T("117");
	cleartreetype2.strBmpurl = _T("//download.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("压缩解压软件");
	cleartreetype2.strTypeid = _T("118");
	cleartreetype2.strBmpurl = _T("//winrar.bmp");
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 2;
	cleartreetype2.strTypeName = _T("其他软件");
	cleartreetype2.strTypeid = _T("120");
	cleartreetype2.strBmpurl = _T("//application.bmp");
	m_arrClearType2.Add(cleartreetype2);

 
    ClearTreeInfo cleartreeinfo;
    //typeid=101

//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("1003");
//     cleartreeinfo.nSn = 1;
//     cleartreeinfo.strClearName = _T("IE Cookies");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//cookies.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("1004");
//     cleartreeinfo.nSn = 2;
//     cleartreeinfo.strClearName = _T("IE地址栏历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//Adressleiste.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("1005");
//     cleartreeinfo.nSn = 3;
//     cleartreeinfo.strClearName = _T("IE访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//aboutIE.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("5002");
//     cleartreeinfo.nSn = 4;
//     cleartreeinfo.strClearName = _T("Firefox浏览器访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//FirefoxMac.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("5001");
//     cleartreeinfo.nSn = 5;
//     cleartreeinfo.strClearName = _T("谷歌浏览器访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//googleChrome.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("5004");
//     cleartreeinfo.nSn = 6;
//     cleartreeinfo.strClearName = _T("腾讯TT浏览器访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//tecentTT.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("5003");
//     cleartreeinfo.nSn = 7;
//     cleartreeinfo.strClearName = _T("傲游浏览器访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//maxthon.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("5005");
//     cleartreeinfo.nSn = 8;
//     cleartreeinfo.strClearName = _T("360浏览器访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//360.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("5006");
//     cleartreeinfo.nSn = 9;
//     cleartreeinfo.strClearName = _T("世界之窗浏览器访问历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//windowofworld.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2018");
//     cleartreeinfo.nSn = 10;
//     cleartreeinfo.strClearName = _T("谷歌工具栏搜索记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//googlelogo.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("101");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("1001");
//     cleartreeinfo.nSn = 11;
//     cleartreeinfo.strClearName = _T("IE缓存目录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//ie.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     //typeid=103
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6001");
//     cleartreeinfo.nSn = 1;
//     cleartreeinfo.strClearName = _T("Internet Explorer自动保存的表单数据（即时有效）");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//IEform.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6002");
//     cleartreeinfo.nSn = 2;
//     cleartreeinfo.strClearName = _T("Internet Explorer自动保存的密码记录（即时有效）");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6005");
//     cleartreeinfo.nSn = 3;
//     cleartreeinfo.strClearName = _T("Firefox 自动保存的表单数据和搜索历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//firefoxform.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6006");
//     cleartreeinfo.nSn = 4;
//     cleartreeinfo.strClearName = _T("Firefox 自动保存的密码记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//Firefoxpassword.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6003");
//     cleartreeinfo.nSn = 5;
//     cleartreeinfo.strClearName = _T("谷歌 Chrome 自动保存的表单数据");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//googleform.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6004");
//     cleartreeinfo.nSn = 6;
//     cleartreeinfo.strClearName = _T("谷歌 Chrome 自动保存的密码记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//googlepassword.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6008");
//     cleartreeinfo.nSn = 7;
//     cleartreeinfo.strClearName = _T("世界之窗保存的表单数据和密码记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//windowofworldpassword.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("103");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("6007");
//     cleartreeinfo.nSn = 8;
//     cleartreeinfo.strClearName = _T("360浏览器保存的表单数据和密码记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//360password.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
//     
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2001");
//     cleartreeinfo.nSn = 1;
//     cleartreeinfo.strClearName = _T("运行对话框历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//startmenu.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2002");
//     cleartreeinfo.nSn = 2;
//     cleartreeinfo.strClearName = _T("最近运行的程序历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//runhistory.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("2008");
//     cleartreeinfo.nSn =3;
//     cleartreeinfo.strClearName = _T("最近打开的程序历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//newprocess.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2005");
//     cleartreeinfo.nSn = 4;
//     cleartreeinfo.strClearName = _T("最近使用的文档记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//newdocuments.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2004");
//     cleartreeinfo.nSn = 5;
//     cleartreeinfo.strClearName = _T("窗口位置与大小历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//position.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
//     
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2014");
//     cleartreeinfo.nSn = 6;
//     cleartreeinfo.strClearName = _T("Windows系统日志");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//syslog.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 0;
//     cleartreeinfo.strClearid = _T("2009");
//     cleartreeinfo.nSn = 7;
//     cleartreeinfo.strClearName = _T("通知区域图标历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//notice.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2007");
//     cleartreeinfo.nSn = 8;
//     cleartreeinfo.strClearName = _T("文件扩展名历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//extension.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//    
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2031");
//     cleartreeinfo.nSn = 10;
//     cleartreeinfo.strClearName = _T("Windows 搜索记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//searchfile.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2032");
//     cleartreeinfo.nSn = 11;
//     cleartreeinfo.strClearName = _T("Windows 网上邻居");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//neiborhood.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//    
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2010");
//     cleartreeinfo.nSn = 12;
//     cleartreeinfo.strClearName = _T("网络驱动器映射历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//reflection-driven.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2011");
//     cleartreeinfo.nSn = 13;
//     cleartreeinfo.strClearName = _T("查找计算机历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//serchsys.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2012");
//     cleartreeinfo.nSn = 14;
//     cleartreeinfo.strClearName = _T("查找文件菜单历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//searchfile.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2013");
//     cleartreeinfo.nSn = 15;
//     cleartreeinfo.strClearName = _T("查找打印机历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//searchprinter.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2016");
//     cleartreeinfo.nSn = 16;
//     cleartreeinfo.strClearName = _T("注册表编辑器最后一次访问位置");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//regedit.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     //cleartreeinfo.nApp = 0;
//     //cleartreeinfo.strTypeid = _T("104");
//     //cleartreeinfo.nCheck = 0;
//     //cleartreeinfo.strClearid = _T("2017");
//     //cleartreeinfo.nSn = 17;
//     //cleartreeinfo.strClearName = _T("Windows浏览器注册表流");
//     //cleartreeinfo.strExtName = _T("");
//     //cleartreeinfo.nFileext = 0;
//     //cleartreeinfo.strBmpurl = _T("//browser-regedit.bmp");
//     //m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("104");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("2003");
//     cleartreeinfo.nSn = 18;
//     cleartreeinfo.strClearName = _T("文件打开与保存历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//openfolder.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9041");
//     cleartreeinfo.nSn = 38;
//     cleartreeinfo.strClearName = _T("PPS 播放器");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//pps.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9042");
//     cleartreeinfo.nSn = 39;
//     cleartreeinfo.strClearName = _T("PPTV 播放器");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//PPTV.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9043");
//     cleartreeinfo.nSn = 40;
//     cleartreeinfo.strClearName = _T("QVOD 播放器");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//Qvod.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9001");
//     cleartreeinfo.nSn = 41;
//     cleartreeinfo.strClearName = _T("Adobe Flash Player历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//adobeflash.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9002");
//     cleartreeinfo.nSn = 42;
//     cleartreeinfo.strClearName = _T("Adobe阅读器历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//adobereader.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9003");
//     cleartreeinfo.nSn = 43;
//     cleartreeinfo.strClearName = _T("Windows画图历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//painter.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9004");
//     cleartreeinfo.nSn = 44;
//     cleartreeinfo.strClearName = _T("Windows写字板历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//writer.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9005");
//     cleartreeinfo.nSn = 45;
//     cleartreeinfo.strClearName = _T("Windows媒体播放器最近播放列表");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//wmp.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
// //     cleartreeinfo.nApp = 0;
// //     cleartreeinfo.strTypeid = _T("105");
// //     cleartreeinfo.nCheck = 1;
// //     cleartreeinfo.strClearid = _T("9006");
// //     cleartreeinfo.nSn = 46;
// //     cleartreeinfo.strClearName = _T("Windows Live Messenger历史记录");
// //     cleartreeinfo.strExtName = _T("");
// //     cleartreeinfo.nFileext = 0;
// //     cleartreeinfo.strBmpurl = _T("//msn.bmp");
// //     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9007");
//     cleartreeinfo.nSn = 47;
//     cleartreeinfo.strClearName = _T("ACDSee打开的路径");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//acdsee.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9008");
//     cleartreeinfo.nSn = 48;
//     cleartreeinfo.strClearName = _T("暴风影音");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//baofeng.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9009");
//     cleartreeinfo.nSn = 49;
//     cleartreeinfo.strClearName = _T("KMPlayer播放历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//kmplayer.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9010");
//     cleartreeinfo.nSn = 50;
//     cleartreeinfo.strClearName = _T("酷狗音乐播放历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//kugou.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9011");
//     cleartreeinfo.nSn = 51;
//     cleartreeinfo.strClearName = _T("QQ影音");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//qqyingyin.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9012");
//     cleartreeinfo.nSn = 52;
//     cleartreeinfo.strClearName = _T("QQLive播放历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//qqlive.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9013");
//     cleartreeinfo.nSn = 53;
//     cleartreeinfo.strClearName = _T("RealPlayer/RealOne播放器");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//realplayer.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9014");
//     cleartreeinfo.nSn = 54;
//     cleartreeinfo.strClearName = _T("迅雷看看播放历史");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//xunleikk.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9015");
//     cleartreeinfo.nSn = 55;
//     cleartreeinfo.strClearName = _T("迅雷搜索历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//xunlei.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9016");
//     cleartreeinfo.nSn = 56;
//     cleartreeinfo.strClearName = _T("Google 桌面的搜索记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//googledesktop.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9017");
//     cleartreeinfo.nSn = 57;
//     cleartreeinfo.strClearName = _T("微软 Office历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//office.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9018");
//     cleartreeinfo.nSn = 58;
//     cleartreeinfo.strClearName = _T("微软 Word历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//word.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9019");
//     cleartreeinfo.nSn = 59;
//     cleartreeinfo.strClearName = _T("微软 Excel历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//excel.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9020");
//     cleartreeinfo.nSn = 60;
//     cleartreeinfo.strClearName = _T("微软 PowerPoint历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//powerpoint.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9021");
//     cleartreeinfo.nSn = 61;
//     cleartreeinfo.strClearName = _T("微软 Access历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//access.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9022");
//     cleartreeinfo.nSn = 62;
//     cleartreeinfo.strClearName = _T("微软 FrontPage历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//frontpage.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9023");
//     cleartreeinfo.nSn = 63;
//     cleartreeinfo.strClearName = _T("微软 SharePoint Designer历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//sharepoint.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//   
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9024");
//     cleartreeinfo.nSn = 64;
//     cleartreeinfo.strClearName = _T("微软 Publisher历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//publisher.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9025");
//     cleartreeinfo.nSn = 65;
//     cleartreeinfo.strClearName = _T("微软 Visio历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//visio.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9026");
//     cleartreeinfo.nSn = 66;
//     cleartreeinfo.strClearName = _T("WPS文字历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//wpsword.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9027");
//     cleartreeinfo.nSn = 67;
//     cleartreeinfo.strClearName = _T("WPS表格历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//wpsexcel.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9028");
//     cleartreeinfo.nSn = 68;
//     cleartreeinfo.strClearName = _T("WPS演示历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//wpspps.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9029");
//     cleartreeinfo.nSn = 69;
//     cleartreeinfo.strClearName = _T("UltraEdit编辑软件历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//ultraedit.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9030");
//     cleartreeinfo.nSn = 70;
//     cleartreeinfo.strClearName = _T("EditPlus编辑软件历史记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//editplus.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9031");
//     cleartreeinfo.nSn = 71;
//     cleartreeinfo.strClearName = _T("WinRAR文件菜单中的历史操作记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//winrar.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9032");
//     cleartreeinfo.nSn = 72;
//     cleartreeinfo.strClearName = _T("WinZip文件菜单中的历史操作记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//winzip.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
// 
//     cleartreeinfo.nApp = 0;
//     cleartreeinfo.strTypeid = _T("105");
//     cleartreeinfo.nCheck = 1;
//     cleartreeinfo.strClearid = _T("9033");
//     cleartreeinfo.nSn = 73;
//     cleartreeinfo.strClearName = _T("7ZIP文件菜单中的历史操作记录");
//     cleartreeinfo.strExtName = _T("");
//     cleartreeinfo.nFileext = 0;
//     cleartreeinfo.strBmpurl = _T("//7zip.bmp");
//     m_arrClearInfo.Add(cleartreeinfo);
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("101");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("1006");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("Internet Explorer 历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("101");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("1004");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("Internet Explorer 地址栏网址历史");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("101");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("1003");
		cleartreeinfo.nSn = 3;
		cleartreeinfo.strClearName = _T("Internet Explorer Cookies【同时包括傲游/世界之窗/腾讯TT/360浏览器的Cookies】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("101");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("1005");
		cleartreeinfo.nSn = 4;
		cleartreeinfo.strClearName = _T("index.dat记录的上网痕迹");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("101");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6001");
		cleartreeinfo.nSn = 5;
		cleartreeinfo.strClearName = _T("Internet Explorer 自动保存的表单【同时包括傲游/世界之窗/腾讯TT/360浏览器的表单】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("101");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6002");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("Internet Explorer 自动保存的密码【同时包括傲游/世界之窗/腾讯TT/360浏览器的密码】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//IEpassword.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		///////////////////////////////////////////////////////////////////////////////////////////
		///火狐浏览器
		//////////////////////////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("102");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5002");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("Mozilla Firefox 历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//FirefoxMac.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("102");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6009");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("Mozilla Firefox Cookies");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//FirefoxMac.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("102");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6006");
		cleartreeinfo.nSn = 3;
		cleartreeinfo.strClearName = _T("Mozilla Firefox 自动保存的表单和搜索记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//FirefoxMac.bmp");
		m_arrClearInfo.Add(cleartreeinfo);


		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("102");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6005");
		cleartreeinfo.nSn = 4;
		cleartreeinfo.strClearName = _T("Mozilla Firefox 自动保存的密码");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//FirefoxMac.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		//////////////////////////////////////////////////////////////////////
		//谷歌浏览器
		//////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("103");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5001");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("Chrome 历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//googleChrome.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("103");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6010");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("Chrome Cookies");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//googleChrome.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("103");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6004");
		cleartreeinfo.nSn = 5;
		cleartreeinfo.strClearName = _T("Chrome 自动保存的表单");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//googleChrome.bmp");
		m_arrClearInfo.Add(cleartreeinfo);


		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("103");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6003");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("Chrome 自动保存的密码");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//googleChrome.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		////////////////////////////////////////////////////////////////////////////////////
		//傲游浏览器上网痕迹
		////////////////////////////////////////////////////////////////////////////////////

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("104");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5003");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("傲游浏览器历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//maxthon.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("104");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("1007");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("傲游 Cookies【同时包括IE/世界之窗/腾讯TT/360浏览器的Cookies】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//maxthon.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("104");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6014");
		cleartreeinfo.nSn = 5;
		cleartreeinfo.strClearName = _T("傲游自动保存的表单【同时包括IE/世界之窗/腾讯TT/360浏览器的表单】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//maxthon.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("104");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6015");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("傲游自动保存的密码【同时包括IE/世界之窗/腾讯TT/360浏览器的密码】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//maxthon.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		////////////////////////////////////////////////////////////////////////////////
		//搜狗浏览器上网痕迹
		////////////////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("105");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5008");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("搜狗浏览器历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//sogo.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("105");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("5009");
		cleartreeinfo.nSn = 7;
		cleartreeinfo.strClearName = _T("搜狗浏览器Cookies");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//sogo.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("105");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("5010");
		cleartreeinfo.nSn = 8;
		cleartreeinfo.strClearName = _T("搜狗浏览器自动保存的表单");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//sogo.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("105");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("5011");
		cleartreeinfo.nSn = 9;
		cleartreeinfo.strClearName = _T("搜狗浏览器自动保存的密码");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//sogo.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("105");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("5012");
		cleartreeinfo.nSn = 10;
		cleartreeinfo.strClearName = _T("搜狗浏览器智能填表中已保存的表单");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//sogo.bmp");
		m_arrClearInfo.Add(cleartreeinfo);
		////////////////////////////////////////////////////////////////////////////////
		//OPERA浏览器
		////////////////////////////////////////////////////////////////////////////////

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("106");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5013");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("Opera浏览器历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//opera_.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("106");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("5014");
		cleartreeinfo.nSn = 7;
		cleartreeinfo.strClearName = _T("Opera浏览器Cookies");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//opera_.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("106");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("5015");
		cleartreeinfo.nSn = 8;
		cleartreeinfo.strClearName = _T("Opera浏览器自动保存的密码");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//opera_.bmp");
		m_arrClearInfo.Add(cleartreeinfo);
		////////////////////////////////////////////////////////////////////////////////
		//世界之窗浏览器
		////////////////////////////////////////////////////////////////////////////////

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("108");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5006");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("世界之窗历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//windowofworld.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("108");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("1008");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("世界之窗Cookies【同时包括IE/傲游/腾讯TT/360浏览器的Cookies】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//windowofworld.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("108");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6013");
		cleartreeinfo.nSn = 5;
		cleartreeinfo.strClearName = _T("世界之窗自动保存的表单【同时包括IE/傲游/腾讯TT/360浏览器的表单】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//windowofworld.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("108");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6012");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("世界之窗自动保存的密码【同时包括IE/傲游/腾讯TT/360浏览器的密码】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//windowofworld.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		////////////////////////////////////////////////////////////////////////////////
		//其他浏览器
		////////////////////////////////////////////////////////////////////////////////

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("109");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5004");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("腾讯TT浏览器历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//tecentTT.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("109");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6016");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("腾讯TT浏览器自动保存的表单和密码【同时包括IE/傲游/世界之窗/360浏览器的表单和密码】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//tecentTT.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("109");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("5005");
		cleartreeinfo.nSn = 3;
		cleartreeinfo.strClearName = _T("360安全浏览器历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//360.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("109");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("6007");
		cleartreeinfo.nSn = 4;
		cleartreeinfo.strClearName = _T("360安全浏览器自动保存的表单和密码【同时包括IE/傲游/世界之窗/腾讯TT浏览器的表单和密码】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//360.bmp");
		m_arrClearInfo.Add(cleartreeinfo);
		//////////////////////////////////////////////////////////////////////////////////
		//Windows系统历史痕迹
		/////////////////////////////////////////////////////////////////////////////////

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2001");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("“运行”对话框历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//startmenu.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2002");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("最近运行程序的历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//runhistory.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("2008");
		cleartreeinfo.nSn =3;
		cleartreeinfo.strClearName = _T("经常访问的程序和快捷方式记录【开始菜单中最近打开的程序记录也会被清除，请慎用】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//newprocess.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2005");
		cleartreeinfo.nSn = 4;
		cleartreeinfo.strClearName = _T("最近打开的文件历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//newdocuments.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("2004");
		cleartreeinfo.nSn = 5;
		cleartreeinfo.strClearName = _T("窗口位置与大小历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//position.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("2014");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("Windows日志文件");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//syslog.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("2009");
		cleartreeinfo.nSn = 7;
		cleartreeinfo.strClearName = _T("通知区域图标历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//notice.bmp");
		m_arrClearInfo.Add(cleartreeinfo);


		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("2007");
		cleartreeinfo.nSn = 8;
		cleartreeinfo.strClearName = _T("文件扩展名历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//extension.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		//cleartreeinfo.nApp = 0;
		//cleartreeinfo.strTypeid = _T("104");
		//cleartreeinfo.nCheck = 0;
		//cleartreeinfo.strClearid = _T("2030");
		//cleartreeinfo.nSn = 9;
		//cleartreeinfo.strClearName = _T("回收站");
		//cleartreeinfo.strExtName = _T("");
		//cleartreeinfo.nFileext = 0;
		//cleartreeinfo.strBmpurl = _T("//recyclebin .bmp");
		//m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2031");
		cleartreeinfo.nSn = 10;
		cleartreeinfo.strClearName = _T("Windows 搜索记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//searchfile.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2032");
		cleartreeinfo.nSn = 11;
		cleartreeinfo.strClearName = _T("Windows 网上邻居");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//neiborhood.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2010");
		cleartreeinfo.nSn = 12;
		cleartreeinfo.strClearName = _T("网络驱动器映射历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//reflection-driven.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2011");
		cleartreeinfo.nSn = 13;
		cleartreeinfo.strClearName = _T("查找计算机历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//serchsys.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2012");
		cleartreeinfo.nSn = 14;
		cleartreeinfo.strClearName = _T("查找文件菜单历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//searchfile.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2013");
		cleartreeinfo.nSn = 15;
		cleartreeinfo.strClearName = _T("查找打印机历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//searchprinter.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2016");
		cleartreeinfo.nSn = 16;
		cleartreeinfo.strClearName = _T("注册表编辑器最后一次访问位置");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//regedit.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2003");
		cleartreeinfo.nSn = 18;
		cleartreeinfo.strClearName = _T("文件打开与保存历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//openfolder.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2029");
		cleartreeinfo.nSn = 19;
		cleartreeinfo.strClearName = _T("剪贴板");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//openfolder.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("2033");
		cleartreeinfo.nSn = 20;
		cleartreeinfo.strClearName = _T("Windows 7 跳转列表【你已锁定跳转列表项目也会被清除，请慎用】");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//newdocuments.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		/*cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("110");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2036");
		cleartreeinfo.nSn = 21;
		cleartreeinfo.strClearName = _T("缩略图缓存");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//suolvtu.bmp");
		m_arrClearInfo.Add(cleartreeinfo);*/

		//////////////////////////////////////////////////////////////////////////////
		//浏览器工具条搜索痕迹
		///////////////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("112");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2018");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("谷歌工具栏搜索记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//googlelogo.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("112");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2019");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("百度工具条搜索记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//baidutoolbar.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("112");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("2034");
		cleartreeinfo.nSn = 3;
		cleartreeinfo.strClearName = _T("QQ工具条搜索记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//QQtoolbar.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

// 		cleartreeinfo.nApp = 0;
// 		cleartreeinfo.strTypeid = _T("112");
// 		cleartreeinfo.nCheck = 1;
// 		cleartreeinfo.strClearid = _T("2035");
// 		cleartreeinfo.nSn = 4;
// 		cleartreeinfo.strClearName = _T("Windows Live工具条搜索记录");
// 		cleartreeinfo.strExtName = _T("");
// 		cleartreeinfo.nFileext = 0;
// 		cleartreeinfo.strBmpurl = _T("//Windows_Live.bmp");
// 		m_arrClearInfo.Add(cleartreeinfo);

		//////////////////////////////////////////////////////////////////////////////
		//下载软件
		///////////////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("117");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9037");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("迅雷下载");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//xunlei.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("117");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9038");
		cleartreeinfo.nSn = 3;
		cleartreeinfo.strClearName = _T("FlashGet下载");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//Flashget.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("117");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9039");
		cleartreeinfo.nSn = 4;
		cleartreeinfo.strClearName = _T("BitComet下载");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//bitcomet.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("117");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("9051");
		cleartreeinfo.nSn = 5;
		cleartreeinfo.strClearName = _T("迅雷7搜索记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//xunlei7.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("117");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9044");
		cleartreeinfo.nSn = 41;
		cleartreeinfo.strClearName = _T("QQ 旋风");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//QQdownload.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		/////////////////////////////////////////////////////////////////////
		///媒体播放软件(116)
		/////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("116");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9041");
		cleartreeinfo.nSn = 1;
		cleartreeinfo.strClearName = _T("PP Stream播放历史");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//pps.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("116");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9042");
		cleartreeinfo.nSn = 2;
		cleartreeinfo.strClearName = _T("PPTV 播放器");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//PPTV.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 3;
		cleartreeinfo.strTypeid = _T("116");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9043");
		cleartreeinfo.nSn = 4;
		cleartreeinfo.strClearName = _T("QVOD 播放器");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//Qvod.bmp");
		m_arrClearInfo.Add(cleartreeinfo);
// 
// 		cleartreeinfo.nApp = 3;
// 		cleartreeinfo.strTypeid = _T("116");
// 		cleartreeinfo.nCheck = 0;
// 		cleartreeinfo.strClearid = _T("9045");
// 		cleartreeinfo.nSn = 5;
// 		cleartreeinfo.strClearName = _T("风行播放历史");
// 		cleartreeinfo.strExtName = _T("");
// 		cleartreeinfo.nFileext = 0;
// 		cleartreeinfo.strBmpurl = _T("//fengxing.bmp");
// 		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 3;
		cleartreeinfo.strTypeid = _T("116");
		cleartreeinfo.nCheck = 0;
		cleartreeinfo.strClearid = _T("9046");
		cleartreeinfo.nSn = 6;
		cleartreeinfo.strClearName = _T("射手影音播放历史");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//sheshou.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

// 		cleartreeinfo.nApp = 3;
// 		cleartreeinfo.strTypeid = _T("116");
// 		cleartreeinfo.nCheck = 0;
// 		cleartreeinfo.strClearid = _T("9047");
// 		cleartreeinfo.nSn = 7;
// 		cleartreeinfo.strClearName = _T("酷我音乐盒播放历史和搜索历史");
// 		cleartreeinfo.strExtName = _T("");
// 		cleartreeinfo.nFileext = 0;
// 		cleartreeinfo.strBmpurl = _T("//kuwo.bmp");
// 		m_arrClearInfo.Add(cleartreeinfo);

		//////////////////////////////////////////////////////////////////////////////
		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("120");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("9048");
		cleartreeinfo.nSn = 10;
		cleartreeinfo.strClearName = _T("福昕PDF阅读器历史记录");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//fixit.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("120");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("9049");
		cleartreeinfo.nSn = 11;
		cleartreeinfo.strClearName = _T("阿里旺旺登录信息");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//aliim.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

		cleartreeinfo.nApp = 0;
		cleartreeinfo.strTypeid = _T("120");
		cleartreeinfo.nCheck = 1;
		cleartreeinfo.strClearid = _T("9050");
		cleartreeinfo.nSn = 12;
		cleartreeinfo.strClearName = _T("Windows Live Messenger");
		cleartreeinfo.strExtName = _T("");
		cleartreeinfo.nFileext = 0;
		cleartreeinfo.strBmpurl = _T("//winlivemsg.bmp");
		m_arrClearInfo.Add(cleartreeinfo);

	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("120");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9002");
	     cleartreeinfo.nSn = 42;
	     cleartreeinfo.strClearName = _T("Adobe阅读器历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//adobereader.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("111");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9003");
	     cleartreeinfo.nSn = 43;
	     cleartreeinfo.strClearName = _T("Windows画图历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//painter.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("111");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9004");
	     cleartreeinfo.nSn = 44;
	     cleartreeinfo.strClearName = _T("Windows写字板历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//writer.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("111");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9005");
	     cleartreeinfo.nSn = 45;
	     cleartreeinfo.strClearName = _T("Windows媒体播放器最近播放列表");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//wmp.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("116");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9008");
	     cleartreeinfo.nSn = 48;
	     cleartreeinfo.strClearName = _T("暴风影音");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//baofeng.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("116");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9009");
	     cleartreeinfo.nSn = 49;
	     cleartreeinfo.strClearName = _T("KMPlayer播放历史");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//kmplayer.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("116");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9010");
	     cleartreeinfo.nSn = 50;
	     cleartreeinfo.strClearName = _T("酷狗音乐播放历史");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//kugou.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("116");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9011");
	     cleartreeinfo.nSn = 51;
	     cleartreeinfo.strClearName = _T("QQ影音");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//qqyingyin.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("116");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9012");
	     cleartreeinfo.nSn = 52;
	     cleartreeinfo.strClearName = _T("QQLive播放历史");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//qqlive.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("116");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9014");
	     cleartreeinfo.nSn = 54;
	     cleartreeinfo.strClearName = _T("迅雷看看播放历史");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//xunleikk.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("117");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9015");
	     cleartreeinfo.nSn = 55;
	     cleartreeinfo.strClearName = _T("迅雷搜索历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//xunlei.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9017");
	     cleartreeinfo.nSn = 57;
	     cleartreeinfo.strClearName = _T("微软 Office历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//office.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9018");
	     cleartreeinfo.nSn = 58;
	     cleartreeinfo.strClearName = _T("微软 Word历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//word.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9019");
	     cleartreeinfo.nSn = 59;
	     cleartreeinfo.strClearName = _T("微软 Excel历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//excel.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9020");
	     cleartreeinfo.nSn = 60;
	     cleartreeinfo.strClearName = _T("微软 PowerPoint历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//powerpoint.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9021");
	     cleartreeinfo.nSn = 61;
	     cleartreeinfo.strClearName = _T("微软 Access历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//access.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9022");
	     cleartreeinfo.nSn = 62;
	     cleartreeinfo.strClearName = _T("微软 FrontPage历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//frontpage.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9023");
	     cleartreeinfo.nSn = 63;
	     cleartreeinfo.strClearName = _T("微软 SharePoint Designer历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//sharepoint.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	   
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9024");
	     cleartreeinfo.nSn = 64;
	     cleartreeinfo.strClearName = _T("微软 Publisher历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//publisher.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9025");
	     cleartreeinfo.nSn = 65;
	     cleartreeinfo.strClearName = _T("微软 Visio历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//visio.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9026");
	     cleartreeinfo.nSn = 66;
	     cleartreeinfo.strClearName = _T("WPS文字历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//wpsword.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9027");
	     cleartreeinfo.nSn = 67;
	     cleartreeinfo.strClearName = _T("WPS表格历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//wpsexcel.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("115");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9028");
	     cleartreeinfo.nSn = 68;
	     cleartreeinfo.strClearName = _T("WPS演示历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//wpspps.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("120");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9029");
	     cleartreeinfo.nSn = 69;
	     cleartreeinfo.strClearName = _T("UltraEdit编辑软件历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//ultraedit.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("120");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9030");
	     cleartreeinfo.nSn = 70;
	     cleartreeinfo.strClearName = _T("EditPlus编辑软件历史记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//editplus.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("118");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9031");
	     cleartreeinfo.nSn = 71;
	     cleartreeinfo.strClearName = _T("WinRAR文件菜单中的历史操作记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//winrar.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("118");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9032");
	     cleartreeinfo.nSn = 72;
	     cleartreeinfo.strClearName = _T("WinZip文件菜单中的历史操作记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//winzip.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);
	 
	     cleartreeinfo.nApp = 0;
	     cleartreeinfo.strTypeid = _T("118");
	     cleartreeinfo.nCheck = 1;
	     cleartreeinfo.strClearid = _T("9033");
	     cleartreeinfo.nSn = 73;
	     cleartreeinfo.strClearName = _T("7ZIP文件菜单中的历史操作记录");
	     cleartreeinfo.strExtName = _T("");
	     cleartreeinfo.nFileext = 0;
	     cleartreeinfo.strBmpurl = _T("//7zip.bmp");
	     m_arrClearInfo.Add(cleartreeinfo);



    //垃圾
    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("IE(含IE内核)浏览器");
    cleartreetype2.strTypeid = _T("IE_TEMP");
    cleartreetype2.strBmpurl = _T("//ie.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("搜狗浏览器(高速模式)");
    cleartreetype2.strTypeid = _T("BROWER_SOGO");
    cleartreetype2.strBmpurl = _T("//sogo.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("傲游3(极速模式)");
    cleartreetype2.strTypeid = _T("BROWER_MATHRON");
    cleartreetype2.strBmpurl = _T("//maxthon.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("火狐浏览器(Firefox)");
    cleartreetype2.strTypeid = _T("BROWER_FIREFOX");
    cleartreetype2.strBmpurl = _T("//FirefoxMac.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("谷歌浏览器(Chrome)");
    cleartreetype2.strTypeid = _T("BROWER_CHROME");
    cleartreetype2.strBmpurl = _T("//googleChrome.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("Opera浏览器");
    cleartreetype2.strTypeid = _T("BROWER_OPERA");
    cleartreetype2.strBmpurl = _T("//opera_.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("Windows临时目录");
    cleartreetype2.strTypeid = _T("WIN_TEMP");
    cleartreetype2.strBmpurl = _T("//sys.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);


	cleartreetype2.nSn = 1;
	cleartreetype2.strTypeName = _T("回收站");
	cleartreetype2.strTypeid = _T("WIN_HUISHOUZHAN");
	cleartreetype2.strBmpurl = _T("//huishouzhan.bmp");
	cleartreetype2.nCheck = 1;
	m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("Windows自动更新补丁");
    cleartreetype2.strTypeid = _T("WIN_UPDATE");
    cleartreetype2.strBmpurl = _T("//update.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 1;
	cleartreetype2.strTypeName = _T("Windows错误报告");
	cleartreetype2.strTypeid = _T("WIN_ERRPORT");
	cleartreetype2.strBmpurl = _T("//errrport.bmp");
	cleartreetype2.nCheck = 1;
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 1;
	cleartreetype2.strTypeName = _T("系统日志文件");
	cleartreetype2.strTypeid = _T("WIN_LOG");
	cleartreetype2.strBmpurl = _T("//logfile.bmp");
	cleartreetype2.nCheck = 1;
	m_arrClearType2.Add(cleartreetype2);

	cleartreetype2.nSn = 1;
	cleartreetype2.strTypeName = _T("缩略图缓存");
	cleartreetype2.strTypeid = _T("WIN_SUOLIETU");
	cleartreetype2.strBmpurl = _T("//suolvtu.bmp");
	cleartreetype2.nCheck = 1;
	m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("Office安装缓存");
    cleartreetype2.strTypeid = _T("OFFICE_CACHE");
    cleartreetype2.strBmpurl = _T("//cache.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("Windows预读文件");
    cleartreetype2.strTypeid = _T("WIN_PRE");
    cleartreetype2.strBmpurl = _T("//wread.bmp");
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("已下载程序文件");
    cleartreetype2.strTypeid = _T("WIN_DOWNLOAD");
    cleartreetype2.strBmpurl = _T("//download.bmp");
    m_arrClearType2.Add(cleartreetype2);

    cleartreetype2.nSn = 1;
    cleartreetype2.strTypeName = _T("用户临时文件");
    cleartreetype2.strTypeid = _T("U_TEMP");
    cleartreetype2.strBmpurl = _T("//utemp.bmp");
    cleartreetype2.nCheck = 1;
    m_arrClearType2.Add(cleartreetype2);

    


    //注册表

	if (TRUE)
	{
		cleartreetype.nSn = 3;
		cleartreetype.strTypeName = _T("冗余注册表");
		cleartreetype.strTypeid = _T("3");
		cleartreetype.strBmpurl = _T("//zhucebiao.bmp");
		m_arrClearType.Add(cleartreetype);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("不存在的DLL文件记录");
		cleartreetype2.strTypeid = _T("RST_SHAREDDLL");
		cleartreetype2.strBmpurl = _T("//dll.bmp");
		cleartreetype2.nCheck = 1;
		m_arrClearType2.Add(cleartreetype2);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("打开对话框历史记录");
		cleartreetype2.strTypeid = _T("RST_OPENSAVEMRU");
		cleartreetype2.strBmpurl = _T("//openfile.bmp");
		cleartreetype2.nCheck = 1;
		m_arrClearType2.Add(cleartreetype2);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("文件类型操作记录");
		cleartreetype2.strTypeid = _T("RST_EXTHISTORY");
		cleartreetype2.strBmpurl = _T("//openfile.bmp");
		cleartreetype2.nCheck = 1;
		m_arrClearType2.Add(cleartreetype2);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("过期的开始菜单");
		cleartreetype2.strTypeid = _T("RST_STARTMENU");
		cleartreetype2.strBmpurl = _T("//startmenu.bmp");
		cleartreetype2.nCheck = 1;
		m_arrClearType2.Add(cleartreetype2);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("缺失的MUI引用");
		cleartreetype2.strTypeid = _T("RST_MUICACHE");
		cleartreetype2.strBmpurl = _T("//dlg.bmp");
		m_arrClearType2.Add(cleartreetype2);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("无效的帮助文件");
		cleartreetype2.strTypeid = _T("RST_HELP");
		cleartreetype2.strBmpurl = _T("//help.bmp");
		cleartreetype2.nCheck = 1;
		m_arrClearType2.Add(cleartreetype2);

		cleartreetype2.nSn = 3;
		cleartreetype2.strTypeName = _T("无效的应用程序路径");
		cleartreetype2.strTypeid = _T("RST_APPPATH");
		cleartreetype2.strBmpurl = _T("//application.bmp");
		cleartreetype2.nCheck = 1;
		m_arrClearType2.Add(cleartreetype2);
	}
    

}


BOOL WINAPI CKscOneKeyConfig::SoftInfoCallBack(void* pThis,SoftInfo softinfo)
{
    CUIHandlerOpt *pDlg = (CUIHandlerOpt*) pThis;
    CString str;
    str.Format(L"%s_%s_%s", softinfo.strClearid, softinfo.strTypeid, softinfo.strmethod);

    return TRUE;

}
BOOL WINAPI CKscOneKeyConfig::SoftTreeCallBack(void* pThis,ClearTreeInfo treeinfo)
{
    CKscOneKeyConfig *pDlg = (CKscOneKeyConfig*) pThis;

    CString str;
    str.Format(L"%s_%s_%s", treeinfo.strClearid, treeinfo.strTypeid, treeinfo.strClearName);
    ////OutputDebugStringW(str);
    ClearTreeInfo cleartreeinfo;
    cleartreeinfo.nApp = treeinfo.nApp;
    cleartreeinfo.strTypeid = treeinfo.strTypeid;
    cleartreeinfo.nCheck = treeinfo.nCheck;
    cleartreeinfo.strClearid = treeinfo.strClearid;
    cleartreeinfo.nSn = treeinfo.nSn;
    cleartreeinfo.strClearName = treeinfo.strClearName;
    cleartreeinfo.strExtName = treeinfo.strExtName;
    cleartreeinfo.nFileext = treeinfo.nFileext;
    cleartreeinfo.strBmpurl = treeinfo.strBmpurl;
    cleartreeinfo.strBmpcrc = treeinfo.strBmpcrc;
    pDlg->m_arrClearInfo.Add(cleartreeinfo);

    return TRUE;
}


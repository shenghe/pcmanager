#pragma once

typedef struct _TREE_TYPE
{
	CString strTypeid;
	CString strTypeName;
	int nSn;
	CString strBmpurl;
	CString strBmpcrc;
    int nCheck;
}ClearTreeType, *PClearTreeType;

typedef struct _SOFTWARE_INFO
{
	CString strTypeid;
	CString strClearid;
	CString strmethod;
	CString strpath;
	CString strpara;
	CString strkeyname;
}SoftInfo, *PSoftInfo;

typedef struct _TREE_INFO
{
	CString strTypeid;
	CString strClearid;
	int nCheck;
	int nSn;
	CString strClearName;
	int nFileext;
	CString strExtName;
	int nApp;
	CString strBmpurl;
	CString strBmpcrc;
}ClearTreeInfo, *PClearTreeInfo;


typedef BOOL (WINAPI *SoftInfoCallBack) (void* pThis,SoftInfo softinfo);
typedef BOOL (WINAPI *SoftTreeCallBack) (void* pThis,ClearTreeInfo treeinfo);

class __declspec(uuid("9124d40d-95d2-4398-81c4-b100363e925d")) ICleanMgr
{
public:
	//加载清理库
	//Return	int;
	//param	CString lib_file
	//param	void* pType;	CAtlMap<CString, TreeType>& pType
	//param void* pInfo;	CSimpleArray<TreeInfo>& m_arrScanReg
	virtual int LoadTreetruct(CString lib_file) = 0;
	//应用程序规则库
	virtual void SetSoftInfoCallBack(SoftInfoCallBack pSoftInfoCallBack,void* pThis) = 0;
	//树结构信息
	virtual void SetTreeCallBack(SoftTreeCallBack pSoftTreeCallBack,void* pThis) = 0;

};
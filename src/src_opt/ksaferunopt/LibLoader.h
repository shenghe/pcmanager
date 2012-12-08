
#pragma once

//#include <libheader.h>

#include <common/markup.h>
#include "runoptimize/ksaferunstrdefine.h"
#include <libheader/libheader.h>
#include <common/utility.h>
#include <runoptimize/interface.h>

#define VERSION_RUN_DAT		1


class CFindData
{
public:
	CFindData(DWORD nTypex, LPCTSTR lpstrNamex, LPCTSTR lpAppNamex)
	{
		nType		= nTypex;
		lpstrName	= lpstrNamex;
		lpAppName	= lpAppNamex;
	}
	DWORD	nType;
	LPCTSTR	lpstrName;
	LPCTSTR	lpAppName;
};

class CLibLoader
{
public:
	CLibLoader()
	{
		m_bLoaded = FALSE;
	}
	virtual ~CLibLoader(){}

// 	static CLibLoader* GetPtr()
// 	{
// 		static CLibLoader x;
// 		return &x;
// 	}

	static	BOOL UpdateXML( BkDatLibHeader& libHeader,CSimpleArray<DWORD> & idDelList, CSimpleArray<CString> & strAddList)
	{
		CLibLoader	loadx;
		return loadx._internUpdateXML(libHeader,idDelList,strAddList);
	}

	BOOL _internUpdateXML( BkDatLibHeader& libHeader,CSimpleArray<DWORD> & idDelList, CSimpleArray<CString> & strAddList)
	{
		BOOL		bRes = FALSE;
		CString		szLibPath;
		CString		szLibPathTmp;

		szLibPath = GetDefaultLib();
		if ( !szLibPath.IsEmpty() )
		{
			szLibPathTmp	=  szLibPath + _T(".tmp");
			CMarkup	xmlDoc;
			BkDatLibHeader	headerOld;

			if (CDataFileLoader().GetLibDatHeader(szLibPath,headerOld) && headerOld.llVersion.QuadPart == libHeader.llUpdateForVer.QuadPart)
			{
				if ( LoadXMLToMarkUp(szLibPath,xmlDoc) && xmlDoc.FindElem(KSAFE_RUN_XML_ROOT) )
				{
					xmlDoc.IntoElem();
					while ( xmlDoc.FindElem(KSAFE_RUN_XML_ITEM) )
					{
						DWORD	nId = _ttoi(xmlDoc.GetAttrib(KSAFE_RUN_XML_ITEM_ID));
						if ( idDelList.Find(nId) != -1 )
						{
							xmlDoc.RemoveElem();
							bRes = TRUE; 
						}
					}

					xmlDoc.OutOfElem();
					for ( INT i = 0; i < strAddList.GetSize(); i++)
					{
						xmlDoc.AddChildSubDoc(strAddList[i]);
						bRes = TRUE; 
					}
				}
			}

			if ( bRes )
			{
				CString	strDOC = xmlDoc.GetDoc();
				BkDatLibEncodeParam2	px(headerOld.dwType,libHeader.llVersion,strDOC,libHeader.nFormatVer);
				bRes = CDataFileLoader().Save(szLibPathTmp,px.GetEncodeParam());
			}

			if ( bRes )
				bRes = ::MoveFileEx( szLibPathTmp, szLibPath, MOVEFILE_REPLACE_EXISTING);
		}

		return bRes;
	}


	BOOL LoadXMLToMarkUp( LPCTSTR lpFile, CMarkup& xmlDoc )
	{
		BOOL			bRes = FALSE;
		CDataFileLoader	loader;
		CString			strXML;
		BkDatLibHeader	header;

		::SetThreadLocale( MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED) ); 

		if (loader.GetLibDatHeader(lpFile,header) && header.nFormatVer>1)
			return FALSE;
		if ( loader.LoadFile(lpFile,strXML) && xmlDoc.SetDoc(strXML) )
			bRes = TRUE;
#if 1	//yykxx
		else if ( xmlDoc.Load(lpFile) )
			bRes = TRUE;
#endif

		return bRes;
	}

	virtual BOOL LoadLib(LPCTSTR lpLibDat=NULL)
	{
		BOOL bRes = FALSE;
		_DeInit();
		if (lpLibDat!=NULL)
			m_strDatPath = lpLibDat;
		else
			m_strDatPath = GetDefaultLib();

		CMarkup	markup;
		if (LoadXMLToMarkUp(m_strDatPath, markup))
		{
			LoadItem(markup,m_dataArray);
		}

		return bRes;
	}

	virtual void LoadItem(CMarkup& markup, CSimpleArray<KSRunInfo>& dataArray)
	{
		if (markup.FindElem(KSAFE_RUN_XML_ROOT))
		{
			markup.IntoElem();
			while (markup.FindElem(KSAFE_RUN_XML_ITEM))
			{

#define GET_DATA_STRING(markup,tag,strSplit)		{markup.SavePos();if(markup.FindElem(tag))strSplit=markup.GetData();markup.RestorePos();}
#define GET_DATA_INT(markup,tag,nvalue)		{markup.SavePos();if(markup.FindElem(tag)){CString strSplit=markup.GetData();nvalue=_ttoi(strSplit);}markup.RestorePos();}

				KSRunInfo	runinfo;
				runinfo.nItemId = _ttoi(markup.GetAttrib(KSAFE_RUN_XML_ITEM_ID));

				markup.IntoElem();
				GET_DATA_STRING(markup,KSAFE_RUN_XML_ITEM_NAME,runinfo.strName);
				GET_DATA_STRING(markup,KSAFE_RUN_XML_ITEM_DISPLAY,runinfo.strDisplay);
				GET_DATA_STRING(markup,KSAFE_RUN_XML_ITEM_DESC,runinfo.strDesc);
				GET_DATA_STRING(markup,KSAFE_RUN_XML_ITEM_WARNING,runinfo.strWarning);
				GET_DATA_STRING(markup,KSAFE_RUN_XML_ITEM_APPNAME,runinfo.strAppName);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_NODELAY,runinfo.bNoDealyShow);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_SYSTEM,runinfo.bSystem);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_TYPE,runinfo.nType);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_ADVICE,runinfo.nAdvice);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_REGPATH,runinfo.bRegPath);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_SHOW_REMAINS,runinfo.bShowRemain);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_EXTRA_FLAG,runinfo.nExtraFlag);
				GET_DATA_STRING(markup,KSAFE_RUN_XML_ITEM_OPENRATE,runinfo.strOpenRate);
				GET_DATA_INT(markup,KSAFE_RUN_XML_ITEM_USERNUM,runinfo.nUserCount);
				markup.OutOfElem();

				dataArray.Add(runinfo);
			}
			markup.OutOfElem();
		}
	}

	int GetCount()
	{
		return m_dataArray.GetSize();
	}

	KSRunInfo& GetInfo(int iIndex)
	{
		return m_dataArray[iIndex];
	}

	CString GetFileName(LPCTSTR lpFullPath)
	{
		LPCTSTR lpstr = _tcsrchr(lpFullPath,_T('\\'));

		if (lpstr==NULL)
			return CString(lpFullPath);
		else
			return CString(lpstr+1);
	}

	BOOL FindDatInfo(CFindData& data, KSRunInfo** pDatInfo = NULL)
	{
		BOOL	bFind = FALSE;
		for ( int j=0; j<this->GetCount(); j++)
		{
			KSRunInfo&	libinfo = this->GetInfo(j);

			if (data.nType==libinfo.nType)
			{
				CString cTmp;
				CString	strName;
				CString cLibName;
				
				if (data.nType!=KSRUN_TYPE_TASK)
					strName = GetFileName(data.lpstrName);
				else 
				{
					strName = data.lpstrName;
				}

				if (data.nType==KSRUN_TYPE_TASK)
				{
					if (strName[0] != _T('\\'))
						cTmp = _T("\\");
					else
						cTmp = _T("");

					cTmp.Append(strName);
					strName = cTmp;
					strName.Replace(_T(".job"), _T(""));

					if (libinfo.strName[0] != _T('\\'))
						cTmp = _T("\\");
					else
						cTmp = _T("");

					cTmp.Append(libinfo.strName);
					cLibName = cTmp;
					cLibName.Replace(_T(".job"), _T(""));

					CString	strAppName	= GetFileName(data.lpAppName);

					if(cLibName.CompareNoCase(strName)==0 &&
						libinfo.strAppName.CompareNoCase(strAppName)==0 )
					{
						bFind = TRUE;
					}

				}
				else
				{
					CString	strAppName	= GetFileName(data.lpAppName);
					if(libinfo.strName.CompareNoCase(strName)==0 &&
						libinfo.strAppName.CompareNoCase(strAppName)==0 )
					{
						bFind = TRUE;
					}
				}


				if (!bFind && libinfo.bRegPath)
				{
					CString	strApp		= data.lpAppName;
					CString	strAppDat	= libinfo.strAppName;

					strApp.MakeLower();
					strAppDat.MakeLower();

					if (strApp.Find(strAppDat)!=-1)
						bFind = TRUE;
				}

				if (bFind)
				{
					if (pDatInfo!=NULL)
						*pDatInfo = &this->GetInfo(j);

					break;
				}
			}
		}
		return bFind;
	}

protected:
	virtual CString GetDefaultLib()
	{
		CString	strpath;
		CAppPath().Instance().GetLeidianDataPath(strpath,TRUE);
		strpath += STR_KSAFE_RUN_OPTIMIZE_DAT;
		return strpath;
	}	
	
	virtual BOOL _DeInit()
	{
		if ( m_bLoaded )
		{
			m_dataArray.RemoveAll();
			m_bLoaded = FALSE;
		}
		return TRUE;
	}

protected:
	BOOL						m_bLoaded;
	CString						m_strDatPath;
	CSimpleArray<KSRunInfo>		m_dataArray;
};

class CLibLoader2: public CLibLoader
{
public:
	CLibLoader2(){}
	~CLibLoader2(){};

	BOOL LoadLib(LPCTSTR lpLibDat=NULL)
	{
		BOOL bRes = FALSE;
		_DeInit();
		if (lpLibDat!=NULL)
			m_strDatPath = lpLibDat;
		else
			m_strDatPath = GetDefaultLib();

		CMarkup	markup;
		if (LoadXMLToMarkUp(m_strDatPath, markup))
		{
			LoadItem(markup, m_arrayLibData);
		}

		return bRes;
	}

	void LoadItem(CMarkup& markup, CSimpleArray<KSysotpRegInfo>& dataArray)
	{
		if (markup.FindElem(KSAFE_RUN_XML_ROOT))
		{
			markup.IntoElem();
			while (markup.FindElem(KSAFE_RUN_XML_ITEM))
			{

#define GET_DATA_STRING(markup,tag,strSplit)		{markup.SavePos();if(markup.FindElem(tag))strSplit=markup.GetData();markup.RestorePos();}
#define GET_DATA_INT(markup,tag,nvalue)		{markup.SavePos();if(markup.FindElem(tag)){CString strSplit=markup.GetData();nvalue=_ttoi(strSplit);}markup.RestorePos();}

				KSysotpRegInfo	libInfo;
				libInfo.nID = _ttoi(markup.GetAttrib(KSAFE_REG_SYSOPT_XML_ITEM_ID));

				markup.IntoElem();
				GET_DATA_INT(markup,	KSAFE_REG_SYSOPT_XML_ITEM_TYPE,			libInfo.nType);
				GET_DATA_INT(markup,	KSAFE_REG_SYSOPT_XML_ITEM_VALUE_TYPE,	libInfo.nRegValueType);
				GET_DATA_INT(markup,	KSAFE_REG_SYSOPT_XML_ITEM_VISTA,		libInfo.bNt6OnlyHave);
				GET_DATA_INT(markup,	KSAFE_REG_SYSOPT_XML_ITEM_COMPARE,		libInfo.nCompareType);
				GET_DATA_STRING(markup, KSAFE_REG_SYSOPT_XML_ITEM_DESC,			libInfo.strDesc);
				GET_DATA_STRING(markup, KSAFE_REG_SYSOPT_XML_ITEM_REGKEY,		libInfo.strRegKey);
				GET_DATA_STRING(markup, KSAFE_REG_SYSOPT_XML_ITEM_DEF_VALUE,	libInfo.strDefaultValue);
				GET_DATA_STRING(markup, KSAFE_REG_SYSOPT_XML_ITEM_OPT_VALUE,	libInfo.strOptimizedValue);
				markup.OutOfElem();

				dataArray.Add(libInfo);
			}
			markup.OutOfElem();
		}
	}

	KSysotpRegInfo& GetLibInfoByIndex(int iIndex)
	{
		return m_arrayLibData[iIndex];
	}

	CKSafeSysoptRegInfo& GetOptInfoByIndex(int iIndex)
	{
		return m_arraySysoptData[iIndex];
	}

	int GetOptInfoCount()
	{
		return m_arraySysoptData.GetSize();
	}

	CString GetDefaultLib()
	{
		CString	strpath;
		CAppPath().Instance().GetLeidianDataPath(strpath,TRUE);
		strpath += STR_KSAFE_REG_OPTIMIZE_DAT;
		return strpath;
	}

	HKEY GetRegKeyRoot(CString strRegRoot)
	{
		HKEY hKey = HKEY_CURRENT_USER;
		if (strRegRoot.CompareNoCase(KSAFE_REG_ROOT_HKCU) == 0)
		{
			hKey = HKEY_CURRENT_USER;
			goto Exit0;
		}
		else if (strRegRoot.CompareNoCase(KSAFE_REG_ROOT_HKLM) == 0)
		{
			hKey = HKEY_LOCAL_MACHINE;
			goto Exit0;
		}
		else 
		{
			goto Exit0;
		}
Exit0:
		return hKey;
	}

	int SplitCString(CString &strIn, CSimpleArray<CString>& strAryOut, CString strSplit)
	{
		if (strSplit.IsEmpty())
		{
			strAryOut.RemoveAll();
			return 0;
		}

		int nStart = 0;
		int nEnd = 0;
		CString strTmp;
		while ( (nEnd = strIn.Find(strSplit.GetString(), nStart)) != -1 )
		{
			strTmp = strIn.Mid(nStart, nEnd-nStart);
			strAryOut.Add(strTmp);
			nStart = nEnd+1;
		}
		strTmp = strIn.Mid(nStart, strIn.GetLength());
		strAryOut.Add(strTmp);

		return strAryOut.GetSize();
	}

	int GetLibCount()
	{
		return m_arrayLibData.GetSize();
	}

	int ConvertLibToOptData()
	{
		int nOptItemCount = 0;
		m_arraySysoptData.RemoveAll();
		for (int nCount = 0; nCount < this->GetLibCount(); nCount++)
		{
			KSysotpRegInfo& libInfo = this->GetLibInfoByIndex(nCount);
			if (libInfo.strRegKey.Find(KSAFE_REGSYSOPT_SEPARATOR) == -1)
			{
				CKSafeSysoptRegInfo sysoptInfo;
				CString strRegKey	= libInfo.strRegKey;
				int nLeft = strRegKey.Find(KSAFE_REGSYSOPT_BACKSLASH);
				int nRight= strRegKey.GetLength() - strRegKey.ReverseFind(KSAFE_REGSYSOPT_BACKSLASH) - 1;
				CString strRegRoot	= strRegKey.Left(nLeft);
				CString strRegValueName = strRegKey.Right(nRight);
				CString strRegSubKey	= strRegKey.Mid(nLeft + 1, strRegKey.GetLength() - 1 - nLeft - nRight - 1);

				sysoptInfo.m_nID			= libInfo.nID;
				sysoptInfo.m_nCompareType	= libInfo.nCompareType;
				sysoptInfo.m_nType			= libInfo.nType;
				sysoptInfo.m_hKeyRoot		= GetRegKeyRoot(strRegRoot);
				sysoptInfo.m_strSubKey		= strRegSubKey;
				sysoptInfo.m_strValueName	= strRegValueName;
				sysoptInfo.m_bIsNt6OnlyHave = libInfo.bNt6OnlyHave;
				sysoptInfo.m_strDesc = libInfo.strDesc;
				sysoptInfo.m_strDefalutValue	= libInfo.strDefaultValue;
				sysoptInfo.m_strOptimizedValue	= libInfo.strOptimizedValue;
				sysoptInfo.m_nRegValueType		= libInfo.nRegValueType;
				m_arraySysoptData.Add(sysoptInfo);
				nOptItemCount++;
			}
			else if (libInfo.strRegKey.Find(KSAFE_REGSYSOPT_SEPARATOR) > 0)
			{
				CKSafeSysoptRegInfo sysoptInfo;
				CSimpleArray<CString> strAryRegKey;
				CSimpleArray<CString> strAryRegDefValue;
				CSimpleArray<CString> strAryRegOptValue;
				int nSubItemCout  = SplitCString(libInfo.strRegKey, strAryRegKey, KSAFE_REGSYSOPT_SEPARATOR);
				int nSubItemCout2 = SplitCString(libInfo.strDefaultValue, strAryRegDefValue, KSAFE_REGSYSOPT_SEPARATOR);
				int nSubItemCout3 = SplitCString(libInfo.strOptimizedValue, strAryRegOptValue, KSAFE_REGSYSOPT_SEPARATOR);

				sysoptInfo.m_nID			= libInfo.nID;
				sysoptInfo.m_nCompareType	= libInfo.nCompareType;
				sysoptInfo.m_nType			= libInfo.nType;
				sysoptInfo.m_bIsNt6OnlyHave = libInfo.bNt6OnlyHave;
				sysoptInfo.m_strDesc		= libInfo.strDesc;
				sysoptInfo.m_nRegValueType	= libInfo.nRegValueType;
				
				for (int n = 0; n < nSubItemCout; n++)
				{
					CKSafeSysoptRegInfo sysoptInfo2	= sysoptInfo;
					sysoptInfo2.m_strDefalutValue	= strAryRegDefValue[n];
					sysoptInfo2.m_strOptimizedValue = strAryRegOptValue[n];

					CString strRegKey	= strAryRegKey[n];
					int nLeft = strRegKey.Find(KSAFE_REGSYSOPT_BACKSLASH);
					int nRight= strRegKey.GetLength() - strRegKey.ReverseFind(KSAFE_REGSYSOPT_BACKSLASH) - 1;
					CString strRegRoot	= strRegKey.Left(nLeft);
					CString strRegValueName = strRegKey.Right(nRight);
					CString strRegSubKey	= strRegKey.Mid(nLeft + 1, strRegKey.GetLength() - 1 - nLeft - nRight - 1);

					sysoptInfo2.m_hKeyRoot  = GetRegKeyRoot(strRegRoot);
					sysoptInfo2.m_strSubKey = strRegSubKey;
					sysoptInfo2.m_strValueName = strRegValueName;
					m_arraySysoptData.Add(sysoptInfo2);
					nOptItemCount++;
				}
			}
		}
		return nOptItemCount;
	}

	virtual BOOL _DeInit()
	{
		if ( m_bLoaded )
		{
			m_arrayLibData.RemoveAll();
			m_arraySysoptData.RemoveAll();
			m_bLoaded = FALSE;
		}
		return TRUE;
	}

public:
	CSimpleArray<KSysotpRegInfo>	 m_arrayLibData;
	CSimpleArray<CKSafeSysoptRegInfo> m_arraySysoptData;
};
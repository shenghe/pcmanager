
#include "stdafx.h"
#include "garbageclean.h"

CGarbageClean::CGarbageClean()
{
	m_bScan = TRUE;
}

CGarbageClean::~CGarbageClean()
{

}

//BOOL CGarbageClean::TEST()
//{	
//
//	USERDATA userData;
//	userData.pObject= this;
//
//	userData.iType = REGCLEAN_INVALIDDLL;
//	
//	CSimpleArray<CString> vec_fileExts;
//
//	CString strExts = _T("*.??$");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.tmp");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.~*");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.---");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.^*");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.??~");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.??$");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*._mp");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.temp");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.~mp*");
//	vec_fileExts.Add(strExts);
//	strExts = _T("*.bak");
//	vec_fileExts.Add(strExts);
//	strExts = _T("thumbs.db");
//	vec_fileExts.Add(strExts);
//
//
//	m_fileOpt.DoFileEnumeration(_T("C:\\"),vec_fileExts,TRUE,TRUE,myEnumerateFile,&userData);
//	return TRUE;
//}


BOOL CGarbageClean::ScanGarbageFile(LPCTSTR lpScanPath,CSimpleArray<FILEEXTS_IN>& vec_fileExts_In)
{	
	if (m_bScan ==FALSE)
		return TRUE;
	
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = FILEGARBAGE;
	
		if (vec_fileExts_In.GetSize() ==0)
		{	
			FILEEXTS_IN  file;
			file.iType =0;
			file.strFileExts= _T("*.*");

			vec_fileExts_In.Add(file);
		}

		m_fileOpt.DoFileEnumerationEx(lpScanPath,vec_fileExts_In,TRUE,TRUE,myEnumerateFileEx,&userData);		
	}
	catch (...)
	{	
		return FALSE;		
	}

	return TRUE;
}

BOOL CGarbageClean::GarbageFile(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{	
	if (m_bScan ==FALSE)
		return FALSE;

	return g_fnScanFile(g_pMain,iType,lpFileOrPath,filesize.LowPart,filesize.HighPart);
}

BOOL CGarbageClean::myEnumerateFile(LPCTSTR lpFileOrPath, void* _pUserData,LARGE_INTEGER filesize)
{	
	if (_pUserData ==NULL)
	{
		return TRUE;
	}

	USERDATA* pUserData = (USERDATA*) _pUserData;
	CGarbageClean* pObject = (CGarbageClean*) pUserData->pObject;
	
	return TRUE;
}

BOOL CGarbageClean::myEnumerateFileEx(int iType,LPCTSTR lpFileOrPath, void* _pUserData,LARGE_INTEGER filesize)
{	
	if (_pUserData == NULL)
	{
		return TRUE;
	}

	USERDATA* pUserData = (USERDATA*) _pUserData;
	CGarbageClean* pObject = (CGarbageClean*) pUserData->pObject;

	return pObject->GarbageFile(iType,lpFileOrPath,filesize);;
}

/****************************************************************
* NAME     : strcmpEx
* FUNCTION    : 支持通配符?,*的字符号比较函数
* PROCESS    : 略
* INPUT     : str1:字符串通1，可包含*,?。 
*        str1:字符串通，可包含*,?。 
* OUTPUT    : 
* EXTERN    : 略
* UPDATE    : 略
* RETURN    : 0:匹配，1:代表不匹配
* PROGRAMMED   : ZhitengLin
* DATE(ORG)    : 2008-10-16
* CALL     : 略
* SYSTEM    : 略
****************************************************************/
TCHAR strcmpEx( const TCHAR *str1, const TCHAR *str2) 
{ 
	size_t slen1 = 0; 
	size_t slen2 = 0; 

	/*定义内循环的范围*/
	size_t lbound = 0; 
	size_t upbound = 0; 

	size_t i, j, k;

	/*实际使用时根据strl的长度来动态分配表的内存*/ 
	TCHAR *matchmap = NULL;
	TCHAR cRet;
	size_t maxLen = 0;
	slen1 = wcslen(str1); 
	slen2 = wcslen(str2); 
	maxLen = slen1>=slen2?slen1:slen2;
	maxLen++;
	matchmap = (TCHAR*)malloc(maxLen*maxLen);

	wmemset(matchmap, 0, maxLen*maxLen); 

	/*遍历目标字符串符串*/
	for(i = 0; i< slen1; ++i) { 
		/*遍历通配符串*/
		size_t bMatched = 0; 
		size_t upthis = upbound; 
		for(j = lbound; j<=upthis ; ++j) { 
			/*匹配当前字符*/ 
			if(str1[i] == str2[j] || str2[j] == '?') { 
				*(matchmap+i*maxLen+j) = 1; 
				if(0 == bMatched) { 
					lbound = j+1; 
				} 
				upbound = j+1; 
				bMatched = 1; 
				if(i == slen1 - 1){ 
					/*考虑末尾是*的特殊情况*/
					for(k = j+1 ; k < slen2 && '*' == str2[k] ; ++k) { 
						*(matchmap+i*maxLen+k) = 1;
					} 
				} 
			}else if(str2[j] == '*') { 
				if(0 == bMatched) { 
					lbound = j; 
				} 
				/*遇到星号，目标字符串到末尾都能得到匹配*/
				for(k = i; k< slen1; ++k) { 
					*(matchmap+k*maxLen+j) = 1;
				} 
				k = j; 
				while( '*' == str2[++k]) { 
					*(matchmap+i*maxLen+k) = 1;
				} 
				if(str1[i] == str2[k] || str2[k] == '?') { 
					*(matchmap+i*maxLen+k) = 1;
					upbound = k+1; 
					if(i == slen1 - 1) { 
						/*考虑末尾是*的特殊情况*/
						for(k = k+1 ; k < slen2 && '*' == str2[k] ; ++k){ 
							*(matchmap+i*maxLen+k) = 1;
						} 
					} 
				}else{ 
					upbound = k; 
				} 
				bMatched = 1; 
			} 
		} 
		/*没有匹配到*/
		if(!bMatched ) { 
			free(matchmap);
			return 1; 
		} 
	}
	cRet = *(matchmap+(slen1-1)*maxLen+slen2-1);
	free(matchmap);

	return (cRet==0?1:0); 
}

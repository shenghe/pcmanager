/**
* @file    pinyin
* @brief   ÓÃÓÚºº×Ö×ªÆ´Òô
* @author  ËïºêÎ°
* @date    2010/05/07 16:02
*/
#pragma once
#include <atlstr.h>
#include <atlcoll.h>
#include <map>

using std::map;
using std::make_pair;

typedef struct _TY_PINYIN
{
	// 	CStringA strHanzi;
	CStringA strPinyin;
	CStringA strSuoxi;
}Pinyin, *PPinyin;

static map<CStringA, Pinyin> mPinyin;

static void GetWenziPinyin(CStringA& strWenzi, CStringA& strSuoxie)
{
	map<CStringA, Pinyin>::iterator iter;

	iter = mPinyin.find(strWenzi);
	while (iter != mPinyin.end())
	{
		strWenzi = iter->second.strPinyin;
		strSuoxie = iter->second.strSuoxi;
		break;
	}

}

static void GetPinyin(CStringA& strPinyin, CStringA& strSuoxie)
{
	BYTE btElement;
	UINT nSrcLength, nCnt;
	CStringA str1Chs;
	strSuoxie = "";
	if(strPinyin.IsEmpty())
	{
		return ;
	}
	CStringA strChs, strEng, strPinyin1, strsuo;
	str1Chs = "11";
	strPinyin1.Empty();
	strChs.Empty();
	strEng.Empty();
	nSrcLength = strPinyin.GetLength(); 
	bool bret = false;
	CStringA strPinyinFu = "";
	CStringA strsuoxieFu = "";
	for(nCnt = 0; nCnt < nSrcLength; )
	{
		btElement = (BYTE)strPinyin.GetAt(nCnt);
		if(btElement < 128)
		{
			strEng.Format("%c", btElement);
			if(bret)
			{
				strPinyin1 = strPinyinFu +" "+ strPinyin1;
				if(strSuoxie.Find(strsuoxieFu)<0)
					strSuoxie = strsuoxieFu +" "+ strSuoxie;
			}
			strPinyin1 +=strEng;
			strSuoxie += strEng;
			strPinyinFu = "";
			strsuoxieFu = "";
			bret = false;
			nCnt++;
		}
		else
		{
			str1Chs.SetAt(0, btElement);
			str1Chs.SetAt(1, (BYTE)strPinyin.GetAt(nCnt+1));
			strChs = str1Chs;
			GetWenziPinyin(strChs,strsuo);
			int n = strChs.Find(" ");
			if(n>0)
			{

				CStringA str2Fu = strChs;
				str2Fu.Delete(0,n+1);
				//if(str2Fu.Find(L" ")>0)AfxMessageBox(strPinyin);
				strPinyinFu += str2Fu;
				bret = true;
				strPinyin1 +=strChs.Left(n);

				str2Fu = strsuo;
				str2Fu.Delete(0,2);
				strsuoxieFu += str2Fu;
				strSuoxie += strsuo.Left(1);
			}
			else
			{
				strPinyinFu += strChs;
				strPinyin1 +=strChs;
				strsuoxieFu += strsuo;
				strSuoxie += strsuo;
			}
			nCnt += 2;
		}
	}
	if(bret)
	{
		strPinyin1 = strPinyinFu +" "+ strPinyin1;
		if(strSuoxie.Find(strsuoxieFu)<0)
			strSuoxie = strsuoxieFu +" "+ strSuoxie;
	}
	strPinyin = strPinyin1;
}




static void LoadFont(LPCSTR LpFile)
{
	mPinyin.clear();
	CDataFileLoader	loader;
	BkDatLibContent new_c;
	if(loader.GetLibDatContent(CA2W(LpFile),new_c)==FALSE)
		return;

	TiXmlDocument plugins(LpFile);
	//plugins.LoadFile();
	plugins.Parse((const char*)new_c.pBuffer);

	TiXmlHandle hDoc(&plugins);
	TiXmlElement* pElem;
	TiXmlHandle hroot(NULL);
	pElem=hDoc.FirstChildElement().Element();
	hroot=TiXmlHandle(pElem);
	pElem=hroot.FirstChildElement().Element();

	TiXmlElement* pSign_elem=hroot.Element()->FirstChildElement("font");
	if(!pSign_elem)return ;
	for(pSign_elem;pSign_elem;pSign_elem=pSign_elem->NextSiblingElement("font"))
	{
		Pinyin pinyin;	
		CStringA strHanzi = pSign_elem->Attribute("c");
		pinyin.strPinyin = pSign_elem->Attribute("p");
		pinyin.strSuoxi = pSign_elem->Attribute("s");

		mPinyin.insert(make_pair(strHanzi,pinyin));
	}
}
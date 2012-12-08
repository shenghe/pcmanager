// ----------------------------------------------------------------------------- 
//	FileName	: make_chsoeminfos.cpp
//	Author		: Zqy
//	Create On	: 2007-11-10 18:34:26
//	Description	: 
//
// ----------------------------------------------------------------------------- 
#pragma once 

#include "stdafx.h"
#include "oeminforw.h"

// ----------------------------------------------------------------------------- 

struct _KMakeAllOemInfos
{
	KWString m_strRoot;
	_KMakeAllOemInfos()
	{
		{
			WCHAR szPath[MAX_PATH] = {0};

			HMODULE hModule = GetModuleHandle(L"kis.dll");

			ASSERT(hModule);

			GetModuleFileName(hModule, szPath, (DWORD)wcslen(szPath));

			WCHAR* p = wcsrchr(szPath, L'\\');

			ASSERT(p);

			p[1] = 0;

			m_strRoot = szPath;
		}

		make_chsiso();
	}

	void make_chsiso()
	{
		KWString strPath = m_strRoot + L"..\\..\\mui\\chs\\oem\\0x00000001\\oem.dat";

		KFeatureWriter w;

		w.Start()
			.StartGroup(0)
			.EndGroup(0);
		w.End();
		w.Write(strPath.c_str());

	}
};

	
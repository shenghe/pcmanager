#include "360clean.h"
#include <algorithm>
#include <vector>
#include <string>
#include "kcheckappexist.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;

C360Clean::C360Clean()
{
	for (int i = 0; i<10000; i++)
	{
		m_bFlag[i] = FALSE;
	}
}

C360Clean::~C360Clean()
{

}

BOOL C360Clean::Scan360Clean()
{	

	//文件部分
	CSimpleArray<CString> vec_file;
	CString strPath; 
	BOOL bRetCheck = _Check360Exist();
	if (!bRetCheck)
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_360);
		return bRetCheck;
	}

	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_360),0,0,0);
	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"360se.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	
	if (TRUE == m_bScan)
	{
		vec_file.Add(_T("history.dat"));
		vec_file.Add(_T("user.dat"));

		strPath = _T("%APPDATA%\\360se\\data");
		m_appHistory.CommfunFile(BROWSERSCLEAN_360,strPath,vec_file);

		g_fnScanReg(g_pMain, BEGINPROC(BROWSERSCLEAN_360), 0, 0, 0);
		CString strReg;
		strReg = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\TypedURLs";
		CSimpleArray<CString> vec_reg;
		m_appHistory.CommfunReg(BROWSERSCLEAN_360, strReg, vec_reg);
		g_fnScanReg(g_pMain, ENDPROC(BROWSERSCLEAN_360), 0, 0, 0);
	}
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_360),str.c_str(),0,0);
	
	return TRUE;
}

BOOL C360Clean::Scan360User(int nType)
{	
	//文件部分
	//CSimpleArray<CString> vec_file;
	//CString strPath; 

	std::wstring str;

	std::vector<std::wstring>::iterator it;
	if (nType == PASSANDFROM_360)
	{
		if (!_Check360Exist())
		{
			g_vsNoinstallapp.Add(PASSANDFROM_360);
			return TRUE;
		}
	}
	if (nType == PASSANDFROM_TT)
	{
		if (!_CheckTTExist())
		{
			g_vsNoinstallapp.Add(PASSANDFROM_TT);
			return TRUE;
		}
	}

	g_fnScanFile(g_pMain,BEGINPROC(nType),0,0,0);

	if (nType == PASSANDFROM_360)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"360se.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

	if (nType == PASSANDFROM_TT)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"ttraveler.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

	if (!m_bFlag[nType])
	{
		str = L"可以清理";
		m_bFlag[nType] = TRUE;
	}

_exit_:
	g_fnScanFile(g_pMain,ENDPROC(nType),str.c_str(),0,0);
	
	return TRUE;
}

BOOL C360Clean::Clean360User(CString str)
{
	HMODULE hMoudle = LoadLibrary(L"InetCpl.cpl");

	if (hMoudle == NULL)
	{
		goto clean0;
	}
	void* pVoid = NULL;
	pVoid = (void*)GetProcAddress(hMoudle, "ClearMyTracksByProcessW");
	if (pVoid == NULL)
	{
		goto clean0;
	}

	::ShellExecute(NULL, _T("open"), _T("rundll32.exe"),  _T("InetCpl.cpl,ClearMyTracksByProcess 16"), NULL, SW_HIDE);
	::ShellExecute(NULL, _T("open"), _T("rundll32.exe"),  _T("InetCpl.cpl,ClearMyTracksByProcess 32"), NULL, SW_HIDE);

clean0:
	if (hMoudle)
	{
		FreeLibrary(hMoudle);
		hMoudle = NULL;
	}

	return TRUE;
}
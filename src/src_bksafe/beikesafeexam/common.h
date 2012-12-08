#pragma once
#include "atlcoll.h"

namespace BEIKESAFEEXAM
{
	class CCommon
	{
	public:
		static CCommon* GetCommonPtr()
		{
			static CCommon com;
			return &com;
		}
		CCommon(void);
		~CCommon(void);
	public:
		BOOL	 NetGatewayExists();
		BOOL	 HaveInternetConnection();
		void	 OutPutLogToFileA(LPCSTR pszFormat, ...);
		void	 OutPutLogToFileW(LPCWSTR pszFormat, ...);
		void	 OutPutLogToDebugView(LPCSTR pszFormat, ...);
		BOOL	 IsWin64();
		int		 VerifyFile(CAtlArray<CString>& arrFile);
		int		 ParsexmlCmdLine(LPCTSTR pszCmdLine, CAtlArray<CString>& arrStr);
// 		int		 ConvertKWSIDToExamID(int nKwsID);
	};
}



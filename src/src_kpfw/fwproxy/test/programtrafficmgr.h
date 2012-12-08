#pragma once

class CProgramTrafficMgr
{
public:
	CProgramTrafficMgr(void);
	~CProgramTrafficMgr(void);
	BOOL Init();
	BOOL Uninit();
private:
	static DWORD WINAPI ThreadGetProgramTraffic( PVOID pParam );
	HANDLE m_hThread;
	HANDLE m_hExit;
};

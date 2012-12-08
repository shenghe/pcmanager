#pragma once


namespace Wnd_LayOut_Mgr
{
	struct UI_Group
	{
		DWORD						dwGroupID;
		CSimpleArray<DWORD>			arrCtrlID;

		UI_Group()
		{
			dwGroupID = -1;
			arrCtrlID.RemoveAll();
		}

		UI_Group(DWORD id, DWORD ctrl)
		{
			dwGroupID = id;
			arrCtrlID.Add(ctrl);
		}
		void Reset()
		{
			dwGroupID = -1;
			arrCtrlID.RemoveAll();
		}
	};

	class CWndLayOutMgr
	{
	public:
		~CWndLayOutMgr(void);
		static CWndLayOutMgr* Instance()
		{
			static CWndLayOutMgr layoutmgr;
			return &layoutmgr;
		}

		int		PushUIGroup(DWORD dwGroupID, UI_Group uiGroup);

		int		SwitchUI(DWORD dwGroupID);

	private:
		CWndLayOutMgr(void);

		CAtlMap<DWORD, UI_Group>	m_mapUI;
	};

}

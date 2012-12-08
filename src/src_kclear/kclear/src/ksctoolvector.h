#pragma once

//////////////////////////////////////////////////////////////////////////

class KscToolVector : public CWindowImpl<KscToolVector>
{
public:
	DECLARE_WND_CLASS(L"KscToolVector");

	KscToolVector();
	~KscToolVector();

	BEGIN_MSG_MAP_EX(KscToolVector)
		MSG_WM_SIZE(OnSize)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	BOOL InitTools();
	void UnInitTools();
	BOOL SwitchTool(int nIndex);

protected:
	void OnSize(UINT nType, CSize size);
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);

	BOOL LoadTool(int nIndex, const std::wstring& strDll);
	void UpdateToolSize(HWND hWnd);

	typedef HWND (__stdcall *ShowDlg_t)(HWND hParent);

	typedef struct tagKscTool {
		HMODULE hDll;
		ShowDlg_t pShowDlg;
		HWND hWnd;
	} KscTool;

	KscTool m_hTools[6];
	BOOL m_fSingleMode;
	RECT m_rcClient;
	BOOL m_fForKSafe;
};

//////////////////////////////////////////////////////////////////////////

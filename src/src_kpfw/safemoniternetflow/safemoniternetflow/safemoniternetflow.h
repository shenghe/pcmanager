// safemoniternetflow.h
#pragma once

class KAppModule : public CAppModule
{
public:
	KAppModule(): m_hModRichEdit2(NULL)
	{
	}


	HRESULT		Init(HINSTANCE hInstance);
	void		Uninit();
	HRESULT		Run();
	BOOL		CheckInstance();
	void		SetActiveWindow(HWND hWndActive);

private:
	void _InitUIResource(void);
	BOOL _CheckIsUniquely(void);
	BOOL _CheckIs64BitOp(void);

private:
	HMODULE m_hModRichEdit2;
};

extern KAppModule _Module;

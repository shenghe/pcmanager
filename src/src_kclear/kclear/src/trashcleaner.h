

#pragma once
HWND __stdcall ShowDlg(HWND hParent);

extern bool __cdecl GetCleaner(const GUID& riid, void** ppv);

bool __stdcall HasTrash();
bool __stdcall SetTrashNotifyHwnd(HWND hWnd);

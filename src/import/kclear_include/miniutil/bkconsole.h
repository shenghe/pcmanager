#pragma once

#include <atlstr.h>

class bkconsole
{
public:
    bkconsole(LPCTSTR console_title = NULL, SHORT console_height = 300, SHORT console_width = 80, BOOL bAllowClose = FALSE)
    {
        if (is_existent)
            return;

        ::AllocConsole();
        
        if (console_title)
            ::SetConsoleTitle(console_title);

        Attach(console_height, console_width);

        if (!bAllowClose)
            DisableCloseButton();

        is_existent = true;
    }

    ~bkconsole()
    {
        if (is_existent)
        {
            ::FreeConsole();

            is_existent = FALSE;
        }
    }

    static void __cdecl settextcolor(BOOL bRed, BOOL bGreen, BOOL bBlue, BOOL bIntensity)
    {
        ::SetConsoleTextAttribute(
            ::GetStdHandle(STD_OUTPUT_HANDLE), 
            (bRed       ? FOREGROUND_RED        : 0) | 
            (bGreen     ? FOREGROUND_GREEN      : 0) | 
            (bBlue      ? FOREGROUND_BLUE       : 0) | 
            (bIntensity ? FOREGROUND_INTENSITY  : 0)
            );
    }

    static BOOL __cdecl printf(const TCHAR *format, ...)
    {
        CString strTrace;
        va_list args;
        DWORD dwWriten = 0;

        va_start(args, format);
        strTrace.FormatV(format, args);
        va_end(args);

        if (is_existent)
        {
            return ::WriteConsole(::GetStdHandle(STD_OUTPUT_HANDLE), strTrace, strTrace.GetLength(), &dwWriten, NULL);
        }
        else
        {
            DWORD dwAttrib = INVALID_FILE_ATTRIBUTES;
            CString strPath;

            ::GetModuleFileName(NULL, strPath.GetBuffer(MAX_PATH + 1), MAX_PATH);
            strPath.ReleaseBuffer();
            strPath = strPath.Left(strPath.ReverseFind(_T('\\')) + 1) + _T("Trace");

            dwAttrib = ::GetFileAttributes(strPath);
            if ((INVALID_FILE_ATTRIBUTES != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & dwAttrib)))
            {
                ::OutputDebugString(strTrace);
            }

            return TRUE;
        }
    }

    static TCHAR _cdecl getch(HANDLE hEventExit = NULL)
    {
        INPUT_RECORD ir;
        DWORD dwGet = 0;
        TCHAR chRet = 0;

        if (!is_existent)
            return 0;

        while (TRUE)
        {
            if (hEventExit)
            {
                dwGet = ::WaitForSingleObject(hEventExit, 0);
                if (WAIT_OBJECT_0 == dwGet)
                    break;
            }

            ::ReadConsoleInput(::GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &dwGet);

#ifdef _UNICODE
            chRet = ir.Event.KeyEvent.uChar.UnicodeChar;
#else
            chRet = ir.Event.KeyEvent.uChar.AsciiChar;
#endif

            ir.Event.KeyEvent.dwControlKeyState &= RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED | RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED;

            if (KEY_EVENT == ir.EventType && ir.Event.KeyEvent.bKeyDown && _T('\0') != chRet && 0 == ir.Event.KeyEvent.dwControlKeyState)
                break;
        }

        return chRet;
   }

private:
    void Attach(SHORT console_height, SHORT console_width)
    {
        HANDLE  hStd;
        COORD   size;

        hStd = ::GetStdHandle(STD_OUTPUT_HANDLE);
        size.X = console_width;
        size.Y = console_height;
        ::SetConsoleScreenBufferSize(hStd, size);
    }

    void DisableCloseButton()
    {
        HMENU hMenu = ::GetSystemMenu(::GetConsoleWindow(), FALSE);

        ::EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
        ::DestroyMenu(hMenu);
    }

    static BOOL is_existent;
};

__declspec(selectany) BOOL bkconsole::is_existent = FALSE;
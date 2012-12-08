// bkdbtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
    int nRet = CppUnitMain(L"bkdb");
    getchar();
    return nRet;
}


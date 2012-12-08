#pragma once


//////////////////////////////////////////////////////////////////////////
#include "kscbase/ksclock.h"


class CStubbornRegs
{
public:
    CStubbornRegs();
    ~CStubbornRegs();

    

    BOOL Init();
    void UnInit();

    static CStubbornRegs& Instance()
    {
        static CStubbornRegs SRegInst;
        return SRegInst;
    }

    void AddReg(const CStringW& strRegPath);
    BOOL Sync();

    BOOL IsStubbornReg(const CStringW& strRegPath);

private:
    BOOL _LoadFromFile();
    BOOL _SaveToFile();

private:
    CAtlMap<CStringW, BOOL> m_regMap;
    CAtlList<CStringW> m_regList;
    KLock klock;
};

//////////////////////////////////////////////////////////////////////////

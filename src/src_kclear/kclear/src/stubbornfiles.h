#pragma once


//////////////////////////////////////////////////////////////////////////
#include "kscbase/ksclock.h"


class CStubbornFiles
{
public:
    CStubbornFiles();
    ~CStubbornFiles();

    

    BOOL Init();
    void UnInit();

    static CStubbornFiles& Instance()
    {
        static CStubbornFiles SFileInst;
        return SFileInst;
    }

    void AddFile(const CStringW& strFilePath);
    BOOL Sync();

    BOOL IsStubbornFile(const CStringW& strFilePath);

private:
    BOOL _LoadFromFile();
    BOOL _SaveToFile();

private:
    CAtlMap<CStringW, BOOL> m_fileMap;
    CAtlList<CStringW> m_fileList;
    KLock klock;
};

//////////////////////////////////////////////////////////////////////////

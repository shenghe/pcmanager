// ConfigData.h: interface for the CConfigData class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

class DIRECTORYWORK
{
public:
    std::vector<CString> strDirectory;
    CString strFriendName;
    std::vector<CString> strExtArray;
    std::vector<CString> strFilt;
    BOOL bRecursive;     // 是否查找子目录
    BOOL bDefaultSelect;
    BOOL bRSelect;//推荐选项
    int  iconindex;
    ULONGLONG ulSize;
    int id;
    int parent;
    BOOL bIsLeaf;

public: 
    DIRECTORYWORK();

    DIRECTORYWORK(const DIRECTORYWORK& cpy);

    DIRECTORYWORK& operator= (const DIRECTORYWORK& cpy);

};

class CConfigData  
{
public:

    CConfigData();

    virtual ~CConfigData();

public:

    typedef std::vector<DIRECTORYWORK> DirWorkArray;


    virtual int ReadConfig();
    virtual int WriteConfig();
    int ReadOneKeyConfig();

    BOOL ConvToLocatePath();
    BOOL InitFiltDir();

    DirWorkArray& GetConfigData();
    const DirWorkArray& GetConfigData() const;
    DIRECTORYWORK& GetConfigItemByID(int nItemID);

    DirWorkArray m_ConfigData;

private:
    BOOL _CheckChangeIE(DIRECTORYWORK& dirword);
protected:
    int _ReadConfigFile();
    int _WriteConfigFile();
};




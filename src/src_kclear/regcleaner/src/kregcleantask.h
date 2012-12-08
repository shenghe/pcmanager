#ifndef KREGCLEANTASK_H_
#define KREGCLEANTASK_H_

//////////////////////////////////////////////////////////////////////////

#include "kclear/ionekeyclean.h"
#include "kclear/kclsiddef.h"
#include "common/kavcom.h"
#include "registrydata.h"
#include "ksclock.h"
#include "kbackupreg.h"

//////////////////////////////////////////////////////////////////////////

class KRegCleanTask : public ICleanTask
{
public:
    KRegCleanTask();
    ~KRegCleanTask();

public:
    //初始化
    HRESULT __stdcall Initialize();

    //反初始化
    HRESULT __stdcall Uninitialize();

    //设置回调
    void __stdcall SetCallback(ICleanCallback* pCallBack);

   //停止清除
    bool __stdcall StopClean();

    //开始清除
    bool __stdcall StartClean(int nTask,const char* szTask);

    void __stdcall Release()
    {
        delete this;
    }

private:
    void SelectCleanItem(int nTask,const char* szTask);
    bool SplitStrTask( 
        /*[in]*/  std::wstring str, 
        /*[out]*/ std::vector <std::wstring>& vcResult,
        /*[in]*/  char delim = ','
        );

    static DWORD WINAPI ScanThreadProc(LPVOID lpParameter);

    static bool	ScanCallback(CRegistryData::SCAN_RESULT* result, LPVOID lpParam);
    static bool	ScanTypeCallBack(CRegistryData::REG_SCAN_TYPE rstType, LPVOID lpParam);
    
    ICleanCallback* m_pCallBack;       //回调
    CRegistryData RegClean;
    bool m_bStop;			
    HANDLE m_hScanThread;	
    KLock m_lockTips;			
    KLock m_lockTree;	
    int	m_nRegistryCount; //扫描到的项
    typedef	std::vector<CRegistryData::SCAN_RESULT*>	VSR;
    typedef std::map<CRegistryData::REG_SCAN_TYPE, VSR>	MTRP;
    MTRP m_scanResultArray;
    typedef std::map<CRegistryData::REG_SCAN_TYPE, bool>	CLEAN_MAP;
    CLEAN_MAP m_CleanMap; 
    CKBackupReg *regBack;
    std::map<std::wstring , bool> TaskMap;
    std::vector<std::wstring> TaskKey;
    std::vector<std::wstring> TaskDesc;
};

//////////////////////////////////////////////////////////////////////////

bool __cdecl GetCleaner(const GUID& riid, void** ppv);

//////////////////////////////////////////////////////////////////////////

#endif // KREGCLEANTASK_H_

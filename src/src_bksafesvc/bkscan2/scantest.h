#pragma  once


class CTestCallBack :
    public IEnumCallBack
{
    virtual void    RecordCleanInfo( const CString& FilePath, void* pvFlag, PVOID pParam )
    {

    }
    virtual HRESULT OperateFile(  const CString& FilePath, void*   pvFlag,  PVOID pParam )
    {
        return S_OK;
    }
    virtual BOOL IsExitEnumWork()
    {
        return FALSE;
    }

    virtual void    AddRelationDir( const CString& DirPath, int setp )
    {

    }
    virtual BOOL IsFilterDir( const CString& DirPath, BOOL& br )
    {
        return FALSE;
    }
    virtual void AddCancelDoubleDir( const CString& DirPath, BOOL b)
    {

    }
};


void TestRemoveDisk();

void TestFileScan();

void TestMutiFileScan();

void TestScanMode();

void TestMutiScan();

void TestWhileScanMode();
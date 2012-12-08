#include "stdafx.h"
#include "autoruntest.h"



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
    virtual BOOL IsFilterDir( const CString& DirPath, BOOL& br )
    {
        return FALSE;
    }
    virtual void    AddRelationDir( const CString& DirPath, int Step ){}

    virtual void    AddCancelDoubleDir( const CString& DirPath, BOOL b ){}
};

VOID TestAutoRun()
{
    CTestCallBack callback;
    CAutorunEnumTask AutorunEnumTask;
    AutorunEnumTask.SetCallback( &callback );
    AutorunEnumTask.EnumScanFile();
}



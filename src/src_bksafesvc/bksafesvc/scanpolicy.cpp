#include "stdafx.h"
#include "common.h"
#include "scanpolicy.h"
#include "enumtask.h"

//快速扫描 扫描内存、启动项、关键目录，关联目录
HRESULT CFastScanPolicy::StartScan( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);
    do 
    {  
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( TRUE );

        CAutorunEnumTask AutorunEnum;
        AutorunEnum.EnumScanFile( pScanContext, BKATR_SHOW_ONLY_CAN_REPAIR );
        AutorunEnum.EnumScanFile( pScanContext, BKATR_SHOW_ONLY_CAN_NOT_REPAIR );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFile( pScanContext );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( FALSE );
        pScanContext->SetAddRelationDir( FALSE );
        //
        CCriticalDirEnumTask CriticalDirEnum;
        CriticalDirEnum.EnumScanFile( pScanContext );
        CAllRootDirEnumTask  AllRootDirEnum( FALSE );
        AllRootDirEnum.EnumScanFile( pScanContext );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        CCustomDirEnumTask RelationDirEnum;
        RelationDirEnum.EnumScanFileForRelationDir( pScanContext, pScanContext->GetRelationDir() );
    }while( false );

    return S_OK;
}

HRESULT CFastScanPolicy::StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& CustomParamMap )
{
    StartScan( pScanContext );

    do 
    {     
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        CCustomDirEnumTask CustomDirEnum;
        CustomDirEnum.EnumScanFileForCusomDir( pScanContext, CustomParamMap, BkScanStepCriticalDir );
    }while( false );

    return S_OK;
}


HRESULT CIeFixScanPolicy::StartScan( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);
    do 
    {       
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( FALSE );
        //
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFileForIeFixPolicy( pScanContext );
    }while( false );

    return S_OK;
}


HRESULT COnlyMemoryScanPolicy::StartScan( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);
    do 
    {       
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( FALSE );
        //
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFile( pScanContext );
    }while( false );

    return S_OK;
}

//体检扫描
HRESULT CCheckScanPolicy::StartScan( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);

    do 
    {       
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( FALSE );
        //
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFileForCheckPolicy( pScanContext );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        CAutorunEnumTask AutorunEnum;
        AutorunEnum.EnumScanFile( pScanContext, 0 );
    }while( false );

    return S_OK;
}

//体检扫描
HRESULT CCheckScanPolicy::StartScanForCacheMemory( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);

    do 
    {       
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( FALSE );
        //
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFileForCheckPolicy( pScanContext );
    }while( false );

    return S_OK;
}

//体检扫描
HRESULT CCheckScanPolicy::StartScanForCacheAutoRun( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);

    do 
    {       
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( FALSE );

        CAutorunEnumTask AutorunEnum;
        AutorunEnum.EnumScanFile( pScanContext, 0 );
    }while( false );

    return S_OK;
}


//自定义扫描 扫描内存、启动项、关键目录，关联目录，自定义目录
HRESULT CCustomScanPolicy::StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& CustomDirMap )
{
    _ASSERT(pScanContext);
    do 
    {  
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( FALSE );
        //
        CAutorunEnumTask AutorunEnum;
        AutorunEnum.EnumScanFile( pScanContext, BKATR_SHOW_ONLY_CAN_REPAIR );
        AutorunEnum.EnumScanFile( pScanContext, BKATR_SHOW_ONLY_CAN_NOT_REPAIR );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFile( pScanContext );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( FALSE );
        pScanContext->SetAddRelationDir( FALSE );
        CCustomDirEnumTask CustomDirEnum;
        CustomDirEnum.EnumScanFileForCusomDir( pScanContext, CustomDirMap, BkScanStepCustomDir  );
    }while( false );

    return S_OK;
}



//全盘扫描 扫描内存、启动项、关键目录，关联目录，所有盘符
HRESULT CFullDiskScanPolicy::StartScan( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);
    do 
    {  
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir( TRUE );

        //
        //CMemoryEnumTask MemoryScan;
        //MemoryScan.EnumScanFile( pScanContext );
        //if ( pScanContext->IsExitEnumWork() )
        //{
        //    break;   
        //}

        //CAutorunEnumTask AutorunEnum;
        //AutorunEnum.EnumScanFile( pScanContext );
        //if ( pScanContext->IsExitEnumWork() )
        //{
        //    break;
        //}
        ////
        //pScanContext->SetAddCancelDoublePathForFile( FALSE );
        //pScanContext->SetAddRelationDir( FALSE );
        ////

        //CCriticalDirEnumTask CriticalDirEnum;
        //CriticalDirEnum.EnumScanFile( pScanContext );
        //CAllRootDirEnumTask  AllRootDirEnum( FALSE );
        //AllRootDirEnum.EnumScanFile( pScanContext );
        //if ( pScanContext->IsExitEnumWork() )
        //{
        //    break;
        //}

        //CCustomDirEnumTask RelationDirEnum;
        //RelationDirEnum.EnumScanFileForRelationDir( pScanContext, pScanContext->GetRelationDir() );
        //if ( pScanContext->IsExitEnumWork() )
        //{
        //    break;
        //}

        CAllRootDirEnumTask  AllRootDirRecursionEnum( TRUE );
        AllRootDirRecursionEnum.EnumScanFile( pScanContext );
    }while ( false );

    return S_OK;
}


//扫描内存 扫描启动项 扫描移动磁盘
HRESULT CMoveDiskScanPolicy::StartScan( CScanContext* pScanContext )
{
    _ASSERT(pScanContext);
    do 
    {  
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir(FALSE);

        //
        CAutorunEnumTask AutorunEnum;
        AutorunEnum.EnumScanFile( pScanContext, BKATR_SHOW_ONLY_CAN_REPAIR );
        AutorunEnum.EnumScanFile( pScanContext, BKATR_SHOW_ONLY_CAN_NOT_REPAIR );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }

        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFile( pScanContext );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;   
        }

        //
        pScanContext->SetAddCancelDoublePathForFile( FALSE );
        //
        CMoveDisEnumTask MoveDiskEnumTask;
        MoveDiskEnumTask.EnumScanFile( pScanContext );
    }while ( FALSE );

    return S_OK;
}


//右键扫描 扫描内存和用户指定目录
HRESULT CRightScanPolicy::StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& RightParamMap )
{
    _ASSERT(pScanContext);
    do 
    {  
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( TRUE );
        pScanContext->SetAddRelationDir(FALSE);
        //
        CMemoryEnumTask MemoryScan;
        MemoryScan.EnumScanFile( pScanContext );
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        //
        pScanContext->SetAddCancelDoublePathForFile( FALSE );
        //
        CCustomDirEnumTask CustomDirEnum;
        CustomDirEnum.EnumScanFileForCusomDir( pScanContext, RightParamMap, BkScanStepCustomDir );
    }while (false);

    return S_OK;
}


HRESULT COnlyCustomScanPolicy::StartScan( CScanContext* pScanContext, CAtlMap<CString, BOOL>& CustomParamMap )
{
    _ASSERT(pScanContext);
    do 
    {     
        if ( pScanContext->IsExitEnumWork() )
        {
            break;
        }
        CCustomDirEnumTask CustomDirEnum;
        CustomDirEnum.EnumScanFileForCusomDir( pScanContext, CustomParamMap, BkScanStepCustomDir );
    }while( false );

    return S_OK;
}




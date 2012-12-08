#include "stdafx.h"
#include "bkautorunimp.h"
#include "autorunscanner.h"
#include "autorunitem.h"

CAutorunHandle::CAutorunHandle() :         ///< Construcor
m_pScanner(NULL)
{

}


CAutorunHandle::~CAutorunHandle()         ///< Destructor
{
	Close();
}


HRESULT WINAPI CAutorunHandle::Create( BOOL bEnableFeedDebug )  ///< 创建并初始化扫描器
{
	Close();
    HRESULT hr = E_FAIL;

    m_pScanner = new CAutorunScanner();
    if ( NULL == m_pScanner )
    {
        return E_OUTOFMEMORY;
    }

    hr = m_pScanner->Initialize();
    if (FAILED(hr))
    {
        delete m_pScanner;
        m_pScanner = NULL;
    }

	return S_OK;
}
void   WINAPI CAutorunHandle::Close()                        ///< 关闭并释放扫描器
{
    if ( NULL != m_pScanner )
    {
        m_pScanner->Uninitialize();
        delete m_pScanner;
        m_pScanner = NULL;
    }
}


BOOL  WINAPI  CAutorunHandle::FindFirstEntryItem( DWORD dwEntryID )    ///< 寻找第一个启动项目
{
    if ( FALSE == CheckOperateData() )
    {
        return E_FAIL;
    }
	return m_pScanner->FindFirstEntryItem( dwEntryID );
}


BOOL  WINAPI  CAutorunHandle::FindNextEntryItem()                                ///< 寻找下一个启动项目
{
    if ( FALSE == CheckOperateData() )
    {
        return E_FAIL;
    }
	return m_pScanner->FindNextEntryItem();
}


HRESULT WINAPI CAutorunHandle::ExportTo(IAutorunHandle& target)       ///< 导出当前启动项目条目,以便后续进行清除
{
    return E_FAIL;
}


LPCWSTR WINAPI CAutorunHandle::GetInformation()                       ///< 获取用来显示的信息
{
    if ( FALSE == CheckOperateData() )
    {
        return L"";
    }
	return m_pScanner->GetInformation();
}


DWORD  WINAPI CAutorunHandle::GetEntryID()                   ///< [所有启动项] 获取启动项ID
{
    if ( FALSE == CheckOperateData() )
    {
        return E_FAIL;
    }
	return m_pScanner->GetCurrentEntryID();
}


DWORD  WINAPI CAutorunHandle::GetEntryType()                 ///< [所有启动项] 获取启动项类型ID
{
    if ( FALSE == CheckOperateData() )
    {
        return E_FAIL;
    }
	return CAutorunItem::GetEntryTypeByID( GetEntryID() );
}


HRESULT WINAPI CAutorunHandle::CleanEntry()                   ///< [所有启动项] 删除当前启动项对应的value,尚需完善
{
    if ( FALSE == CheckOperateData() )
    {
        return E_FAIL;
    }
	return m_pScanner->CleanCurrentEntry();
}


DWORD WINAPI  CAutorunHandle::GetFileCount()                 ///< [所有启动项] 获取启动项的文件数目
{
    if ( FALSE == CheckOperateData() )
    {
        return E_FAIL;
    }
	 return m_pScanner->GetFileCount();
}


LPCWSTR  WINAPI CAutorunHandle::GetFilePathAt(DWORD dwIndex)   ///< [所有启动项] 获取启动项的文件路径
{
    if ( FALSE == CheckOperateData() )
    {
        return L"";
    }
	return m_pScanner->GetFilePathAt(dwIndex);
}


//////////////
///////////////
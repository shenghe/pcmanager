#ifndef __BKAUTORUNIMP_H__
#define __BKAUTORUNIMP_H__

#include "bkautorun.h"
#include "com_s\com\comobject.h"
class CAutorunScanner;

class CAutorunHandle
	: public IAutorunHandle
{
public:
	CAutorunHandle();          ///< Construcor
	~CAutorunHandle();         ///< Destructor

protected:
	virtual HRESULT WINAPI Create( BOOL bEnableFeedDebug );  ///< 创建并初始化扫描器
	virtual void    WINAPI Close();                        ///< 关闭并释放扫描器


	virtual BOOL    WINAPI FindFirstEntryItem( DWORD dwEntryID );    ///< 寻找第一个启动项目
	virtual BOOL    WINAPI FindNextEntryItem();                                ///< 寻找下一个启动项目

	virtual HRESULT WINAPI ExportTo( IAutorunHandle& target);       ///< 导出当前启动项目条目,以便后续进行清除

	virtual LPCWSTR WINAPI GetInformation();                       ///< 获取用来显示的信息

	virtual DWORD   WINAPI GetEntryID();                   ///< [所有启动项] 获取启动项ID
	virtual DWORD   WINAPI GetEntryType();                 ///< [所有启动项] 获取启动项类型ID
	virtual HRESULT WINAPI CleanEntry();                   ///< [所有启动项] 删除当前启动项对应的value,尚需完善

	virtual DWORD   WINAPI GetFileCount();                 ///< [所有启动项] 获取启动项的文件数目
	virtual LPCWSTR WINAPI GetFilePathAt(DWORD dwIndex);   ///< [所有启动项] 获取启动项的文件路径



	SCOM_BEGIN_COM_MAP(CAutorunHandle)
		SCOM_INTERFACE_ENTRY(IAutorunHandle)
	SCOM_END_COM_MAP()

private:
    BOOL CheckOperateData()
    {
        return (NULL!=m_pScanner) ? TRUE : FALSE;        
    }
    CAutorunScanner*  m_pScanner;
};



#endif //__BKAUTORUNIMP_H__
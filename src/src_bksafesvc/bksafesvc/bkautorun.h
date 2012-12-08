#ifndef __BKAUTORUN_H__
#define __BKAUTORUN_H__




interface  __declspec(uuid("7FC9D945-223D-4dbb-94BD-E822AFD7B907"))
IAutorunHandle
	: public IUnknown
{
	virtual HRESULT WINAPI Create( BOOL bEnableFeedDebug = FALSE )				= 0;   ///< 创建并初始化扫描器
	virtual void    WINAPI Close()												= 0;   ///< 关闭并释放扫描器


	virtual BOOL    WINAPI FindFirstEntryItem( DWORD dwEntryID = 1 )	= 0;   ///< 寻找第一个启动项目
	virtual BOOL    WINAPI FindNextEntryItem()									= 0;   ///< 寻找下一个启动项目

	virtual HRESULT WINAPI ExportTo( IAutorunHandle& target)					= 0;   ///< 导出当前启动项目条目,以便后续进行清除

	virtual LPCWSTR WINAPI GetInformation()										= 0;   ///< 获取用来显示的信息

	virtual DWORD   WINAPI GetEntryID()											= 0;   ///< [所有启动项] 获取启动项ID                        //
	virtual DWORD   WINAPI GetEntryType()										= 0;   ///< [所有启动项] 获取启动项类型ID                    //
	virtual HRESULT WINAPI CleanEntry()											= 0;   ///< [所有启动项] 删除当前启动项对应的value,尚需完善  //未实现
	virtual DWORD   WINAPI GetFileCount()										= 0;   ///< [所有启动项] 获取启动项的文件数目
	virtual LPCWSTR WINAPI GetFilePathAt( DWORD dwIndex )						= 0;   ///< [所有启动项] 获取启动项的文件路径
};


#define FN_BkCreateAutorunHandle            "BkCreateAutorunHandle"
EXTERN_C HRESULT WINAPI BkCreateAutorunHandle( IAutorunHandle **ppiAutorunHandle );
typedef HRESULT (WINAPI *PFN_BkCreateAutorunHandle)( IAutorunHandle **ppiAutorunHandle );



#endif //__BKAUTORUN_H__
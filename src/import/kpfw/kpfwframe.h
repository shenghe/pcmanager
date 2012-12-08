
#pragma once

interface IKpfwFrame
{
	/**
	*@param void
	*@return TRUE: msn已经开启  FLASE msn 已经关闭
	*/
	virtual STDMETHODIMP_(BOOL) GetMsnEnable() = 0;


	/**
	*@param TRUE 开启msn  FLASE 关闭msn
	*@return TRUE: 设置成功  FLASE 设置失败
	*/
	virtual STDMETHODIMP_(BOOL) SetMsnEnable( BOOL bEnable ) = 0;

	/**
	*@param void 
	*@return TRUE: 打开成功 FLASE 打开失败
	×@ 打开应用程序规则编辑器
	*/
	virtual STDMETHODIMP_(BOOL) OpenAppRule( ) = 0;

	/**
	*@param void 
	*@return TRUE: 打开成功 FLASE 打开失败
	×@ 打开网络链接查看器
	*/
	virtual STDMETHODIMP_(BOOL) OpenNetAct( ) = 0;

	/**
	*@param void 
	*@return TRUE: 打开成功 FLASE 打开失败
	×@ 打开日志查看器
	*/
	virtual STDMETHODIMP_(BOOL) OpenLog( ) = 0;


	/**
	*@param void 
	*@return TRUE: 打开成功 FLASE 打开失败
	×@ 关闭网镖主界面
	*/
	virtual STDMETHODIMP_(BOOL) CloseKpfwMain( LPCTSTR pszHtml ) = 0;

	/**
	*@param [out]返回ip字符串，[in]输入ip
	*@return 转换成功 FLASE 转换失败
	×@ ULOG 转ip字符串
	*/
	virtual STDMETHODIMP_(BOOL) IpToString( ULONG uIp, LPTSTR pszIp ) = 0;

	/**
	*@param [out]pszMac 返回字符串 [in] n0, n1, n2, n3, n4, n5
	*@return TRUE: 转换成功 FLASE 转换失败
	×@ mac转字符串
	*/
	virtual STDMETHODIMP_(BOOL) MacToString( int n0, int n1, int n2, int n3, int n4, int n5, LPTSTR pszMac ) = 0;

	/**
	*@param [in]bOpen TRUE 为打开文件 FALSE 为保存文件
	*       [out] pszFullPath 放回用户选中的文件, 如果为空字符串表示用户没有选择路径
	*@return TRUE: 打开成功 FLASE 打开失败
	×@ 用户打开文件路径选择器
	*/
	virtual STDMETHODIMP_(BOOL) OpenFileDlg( BOOL bOpen, LPTSTR pszFullPath ) = 0;
		
};
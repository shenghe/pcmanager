/** 
* @file irescancallback.h
* @brief 得知文件黑白时的文件回调接口类定义
* @author BAO<baoderong@kingsoft.com>
* @date November 10,2010
*/

#ifndef IRESCANCALLBACK_H
#define IRESCANCALLBACK_H

/** 
* @interface IReScanCallback
* @brief 扫描过的文件回调接口
*/
interface __declspec(uuid("269D58B4-3B39-405f-8439-C5A3178ABF73"))
 IReScanCallback : public IUnknown
{
public:

	/** 
	* @brief 对一个已知黑白的文件做回调处理
	*
	* @param[in] const wchar_t *pszFileName 回扫文件名
	* @param[in] BYTE pszMD5[16] 回扫文件的MD5
	* @param[in[ const wchar_t *pszChannel 回扫文件的通道号
	* @param[in] unsigned int uFlag 保存文件来源（如全盘，快扫，监控），可以按位或
	* @param[in] const unsigned int uFileType 文件类型，0为白，1为黑，3为恢，4为丢弃
	* @return 标准的HRESULT值
	*/
	virtual HRESULT __stdcall NotifyFileResult(
		const wchar_t *pszFileName,
		const BYTE pszMD5[16], 
		const wchar_t *pszChannel,
		unsigned int uFlag, 
		unsigned int uFileType) = 0;
};

#endif

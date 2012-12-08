////////////////////////////////////////////////////////////////
//
//	Filename: 	ikxeactivity.h
//	Creator:	yangjun  <yangjun@kingsoft.com>
//	Date:		2009-5-19  9: 35
//	Comment:	
//
///////////////////////////////////////////////////////////////


#ifndef _ikxeactivity_h_
#define _ikxeactivity_h_

#include <Unknwn.h>

///////////////////////////////////////////////////////////////
//
// interface IKxeActivity 活跃度统计接口
//
///////////////////////////////////////////////////////////////

MIDL_INTERFACE("E19AA587-600D-499d-A091-F22691E9F4B2")
IKxeActivity : public IUnknown
{
	virtual int __stdcall Init() = 0;

	virtual int __stdcall Start() = 0;

	/**
	* @brief   通知活跃信息
	* @param   [in] uProductId          产品类别
	* @param   [in] pwszProductVersion  产品版本信息(可传空)
	* @remark
	*/
	virtual int __stdcall NoticeActivity( 
		/*[in]*/ unsigned int uProductId, 
		/*[in]*/ const wchar_t *pwszProductVersion
		) = 0;

	virtual int __stdcall Stop() = 0 ;

	virtual int __stdcall Uninit() = 0;

};

#define E_GETUUID_SUCCESSS			 0	///< 获取成功 
#define E_GETUUID_ERROR_UNKNOWN		-1	///< 获取失败，原因未知 
#define E_GETUUID_ERROR_NONETWORK	-2	///< 获取失败,没有联网 
#define E_GETUUID_ERROR_SMALLBUFFER	-3	///< 获取失败,缓冲区长度太小

/**
* @brief	获取UUID
* @param	[out]	 pszUUID 返回的用户UUID
* @param	[in out] pnLen	 输入时为传进的pszUUID指向的缓冲区字字符数
输出时为实际copy的字节数

* @param	[int]	 bRefresh 如果有缓存的UUID,是否重新计算UUID
* @return	见E_GETUUID_XXX
* @remark	如果内部还没有缓存UUID或
*/
int __stdcall GetUUID(char* pszUUID,
					  int* pnLen,
					  bool bRefresh
					  );

#endif // _ikxeactivity_h_
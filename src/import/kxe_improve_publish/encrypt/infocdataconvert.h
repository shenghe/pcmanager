#pragma once

using namespace std;

struct IInfocDataDecoder// : public IUnknown
{
	// 功能：获得下一个 Infoc 解密过后的对
	// 参数：
	// pcbAttrNameSize  ：pszAttrName 的大小（BYTE 数目）
	// pszAttrName	    ：解密出来的名字
	// pcbAttrValueSize ：pbyAttrValue 的大小（BYTE 数目）
	// pbyAttrValue		：解密出来对应名字的值字符串
	virtual long GetNextData(
		IN OUT      DWORD       *pcbAttrNameSize, 
		OUT         BYTE        *pszAttrName, 
		IN OUT      DWORD       *pcbAttrValueSize, 
		OUT         BYTE        *pbyAttrValue) = 0;

	// 释放资源
	virtual void Release() = 0;
};

// 获得解密接口 IInfocDataDecoder
long GetDataDecoder(
					IN      DWORD       cbUrlLength,		// pbyUrlBuffer 的大小（BYTE数目）
					IN      BYTE*       pbyUrlBuffer,		// 需要解密的字符串
					OUT IInfocDataDecoder** ppiDecoder);	// 解密接口指针

class IInfocDataEncoder//: IUnknown
{
public:
	// 功能：获得加密接口的版本号（目前未实现，仅仅返回1）
    // 参数：
	// pdwVersion：IInfocDataEncoder 的 pdwVersion 版本号
	virtual long __stdcall GetEncoderVersion(
        OUT     DWORD       *pdwVersion) = 0;

	// 功能：添加 Infoc 所需的字段对
	// 参数：
	// dwNameSize：		名字字符串的大小（字符数）
	// pszName：		名字称字符串
	// dwAttrSize：		对应名字的结果的大小（字符数）
	// pszAttrValue：	对应名字的结果字符串
	virtual long __stdcall AddStringAttr(
		IN			DWORD       dwNameSize,
		IN const	TCHAR       *pszName, 
		IN			DWORD       dwAttrSize,
		IN const	TCHAR       *pszAttrValue) = 0;

	// 功能：获得结果字符串
	// 参数：
	// pcbUrlLength：参数 pbyUrlBuffer 的大小（BYTE 数目）
	// pbyUrlBuffer：结果缓存
    virtual long __stdcall FormatInfocDataToUrl(
        IN OUT  DWORD       *pcbUrlLength, 
        OUT     BYTE*       pbyUrlBuffer) = 0;

	// 功能：释放此接口的资源（每次加密完成之后都必须调用）
    virtual void __stdcall Release() = 0;

	// 获得加密接口指针
};

long GetDataEncoder(OUT IInfocDataEncoder **ppiEncoder);



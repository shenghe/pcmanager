//kdownloadfile.h
// 为分析模块准备index，换句话说是下载好所有的index文件。

#pragma once

#ifndef __UPLIVE_KREQUEST_HTTP_FILE_H__
#define __UPLIVE_KREQUEST_HTTP_FILE_H__

#include "uplive_common.h"
#include "winhttp/winhttpwrapper.h"


BOOL WinHttp_Initialize();

BOOL WinHttp_Uninitialize();

BOOL WinHttp_RequestHttpFile(IN LPCWSTR lpszUrl);

BOOL WinHttp_RequestHttpFile(IN LPCWSTR lpszUrl, 
							 IN OUT std::string& strReturnData, 
							 IN IStatusCallback * _pIStatusCallback = NULL);

#endif //__UPLIVE_KREQUEST_HTTP_FILE_H__
/***************************************************************
 * @date:   2007-12-06
 * @author: BrucePeng
 * @brief:  Define some common functions
 */
#ifndef K_FILE_DOWNLOADER_COMMON_H_
#define K_FILE_DOWNLOADER_COMMON_H_

#include "KFileDownloadDefine.h"
#include "IDownloadThreadCallback.h"
#include "curl/curl.h"

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

bool SetOurHttpHeader(IN const char* pszURL, OUT curl_slist** ppCurlSlist);

bool PerformCurlRequest(IN void* pvCurl, IN IDTCallback* m_pCallback);

bool EnableAutoRedirect(IN void* pvCurl, long lMaxRedirs, bool bAutoRefer);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif//K_FILE_DOWNLOADER_COMMON_H_

/***************************************************************
 * @date:   2007-10-26
 * @author: BrucePeng
 * @brief:  Define the interface for download thread callback
 */

#ifndef I_DOWNLOAD_THREAD_CALLBACK_H_
#define I_DOWNLOAD_THREAD_CALLBACK_H_

#include "KFileDownloadDefine.h"

class IDownloadThreadCallback
{
public:
    IDownloadThreadCallback(void){}
    virtual ~IDownloadThreadCallback(void){}

public:
    /**
    * @brief Query if exit is needed
    *
    * @return If exit is needed
    * - true   yes
    *   false  no
    */
    virtual bool __stdcall IsExitNeeded(void) = 0;

    /**
    * @brief Get the proxy information if any
    *
    * @param[OUT]  stProxyInfo     The proxy information
    *
    * @return If successfully
    * - true   yes
    *   false  no
    */
    virtual bool __stdcall GetProxyInfo(IN ST_PROXY_INFO& stProxyInfo) = 0;

    /**
    * @brief Get the URL of download file
    *
    * @param[OUT]  pszURL          The buffer to store value
    * @param[IN]   size            The size of buffer "pszURL"
    *
    * @return If successfully
    * - true   yes
    *   false  no
    */
    virtual bool __stdcall GetDownloadUrl(OUT char* pszURL, IN size_t size) = 0;

    /**
    * @brief Get the timeout for connecting
    *
    * @return The value of timeout
    */     
    virtual unsigned long __stdcall GetConnectTimeout(void) = 0;

    /**
     * @brief Get the timeout for sending
     * 
     * @return The value of timeout
     */
    virtual unsigned long __stdcall GetSendTimeout(void) = 0;

    /**
     * @brief Get the timeout for receiving data
     *
     * @return The value of timeout
     */

    virtual unsigned long __stdcall GetRecvTimeout(void) = 0;

    /**
    * @brief Get the "file block information" by user defined thread ID
    *
    * @return The pointer to file block information
    *
    * @note On error, return NULL
    */
    virtual ST_FILE_BLOCK_INFO* __stdcall GetFileBlockInfoByID(IN int nID) = 0;

};

typedef IDownloadThreadCallback IDTCallback;

#endif//I_DOWNLOAD_THREAD_CALLBACK_H_


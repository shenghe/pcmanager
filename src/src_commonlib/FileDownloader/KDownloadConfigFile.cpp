/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the configuration file object
 */

#include "KDownloadConfigFile.h"
#include <assert.h>

//Define the size of buffer not used in the beginning
const int cnBufSizeNotUsed      = 0x17;

//Define the value of reserved parameter
//It's used for validating
const int cnValueOfReserved     = 0x17;

bool CDownloadConfigFile::Create(const char* pszFileName)
{
    if(NULL == pszFileName)
    {
        assert(false);
        return false;
    }

    //If a file has been opened, close it first
    this->Close();

    //Create file
    m_hConfigFile = ::CreateFileA(pszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    return (INVALID_HANDLE_VALUE != m_hConfigFile) ? true:false;
}

bool CDownloadConfigFile::Close(void)
{
    if(INVALID_HANDLE_VALUE != m_hConfigFile)
        CloseHandle(m_hConfigFile);

    m_hConfigFile = INVALID_HANDLE_VALUE;
    return true;
}

bool CDownloadConfigFile::ReadConfigInfo(ST_DC_INFO& stDCInfo)
{
    BOOL    bRet        = FALSE;
    DWORD   dwRead      = 0;
    DWORD   dwRet       = 0;

    if(INVALID_HANDLE_VALUE == m_hConfigFile)
        return false;

    //Set file pointer to beginning of valid data
    dwRet = ::SetFilePointer(m_hConfigFile,
        cnBufSizeNotUsed,
        NULL,
        FILE_BEGIN
    );
    if(INVALID_SET_FILE_POINTER == dwRet)
        return false;

    //Start reading configuration information
    //Read URL
    bRet = ::ReadFile(m_hConfigFile,
        stDCInfo.szDownloadURL,
        defURL_MAX_LEN,
        &dwRead,
        NULL
    );
    if(!bRet || defURL_MAX_LEN != dwRead)
        return false;

    //Read local path file name
    bRet = ::ReadFile(m_hConfigFile,
        stDCInfo.szLocalPFName,
        defPFN_MAX_LEN,
        &dwRead,
        NULL
    );
    if(!bRet || defPFN_MAX_LEN != dwRead)
        return false;

    //Read length of file
    bRet = ::ReadFile(m_hConfigFile,
        &(stDCInfo.uFileLength),
        sizeof(unsigned),
        &dwRead,
        NULL
    );
    if(!bRet || sizeof(unsigned) != dwRead)
        return false;

    //read number of block
    bRet = ::ReadFile(m_hConfigFile,
        &(stDCInfo.uBlockNumber),
        sizeof(unsigned),
        &dwRead,
        NULL
    );
    if(!bRet || sizeof(unsigned) != dwRead)
        return false;

    //Read block info
    for(unsigned i = 0; i < stDCInfo.uBlockNumber; ++i)
    {
        ST_BP_INFO  stBPInfo;

        bRet = ::ReadFile(m_hConfigFile,
            &stBPInfo,
            sizeof(ST_BP_INFO),
            &dwRead,
            NULL
        );
        if(!bRet || sizeof(ST_BP_INFO) != dwRead)
            return false;
        
        if(cnValueOfReserved != stBPInfo.uReserved)
            return false;
        
        stDCInfo.BPInfolist.push_back(stBPInfo);
    }

    return true;
}

bool CDownloadConfigFile::WriteConfigInfo(ST_DC_INFO& stDCInfo)
{
    bool bRet = WriteConfigHeadInfo(stDCInfo.szDownloadURL,
        stDCInfo.szLocalPFName,
        stDCInfo.uFileLength,
        stDCInfo.uBlockNumber
    );
    return bRet ? WriteConfigProgressInfo(stDCInfo.BPInfolist):false;
}

bool CDownloadConfigFile::WriteConfigProgressInfo(
    BPInfoList&   infoList
)
{
    DWORD   dwRet       = 0;
    BOOL    bRet        = FALSE;
    DWORD   dwWritten   = 0;

    if(INVALID_HANDLE_VALUE == m_hConfigFile)
        return false;

    //Set file pointer
    dwRet = ::SetFilePointer(m_hConfigFile,
        cnBufSizeNotUsed + defURL_MAX_LEN 
        + defPFN_MAX_LEN + sizeof(unsigned) * 2,
        NULL,
        FILE_BEGIN
    );
    if(INVALID_SET_FILE_POINTER == dwRet)
        return false;

    BPInfoList::iterator it = infoList.begin();
    for(; it != infoList.end(); ++it)
    {
        it->uReserved = cnValueOfReserved;

        bRet = ::WriteFile(m_hConfigFile,
            static_cast<ST_BP_INFO* >(&(*it)),
            sizeof(ST_BP_INFO),
            &dwWritten,
            NULL
        );
        if(!bRet || sizeof(ST_BP_INFO) != dwWritten)
            return false;
    }
    SetEndOfFile(m_hConfigFile);
    return true;
}

bool CDownloadConfigFile::WriteConfigHeadInfo(
    const char*     pszDownloadURL,
    const char*     pszLocalPFName,
    unsigned        uFileLength,
    unsigned        uBlockNumber
)
{
    BOOL    bRet            = FALSE;
    DWORD   dwWritten       = 0;
    DWORD   dwRet           = 0;

    if(!pszDownloadURL || !pszLocalPFName)
        return false;

    if(INVALID_HANDLE_VALUE == m_hConfigFile)
        return false;

    //Set file pointer to beginning of valid data

    dwRet = ::SetFilePointer(m_hConfigFile,
        cnBufSizeNotUsed,
        NULL,
        FILE_BEGIN
    );
    if(INVALID_SET_FILE_POINTER == dwRet)
        return false;

    //Write URL
    char  szURL[defURL_MAX_LEN];
    
    if(strlen(pszDownloadURL) >= defURL_MAX_LEN)
        return false;
    
    strncpy(szURL, pszDownloadURL, defURL_MAX_LEN);

    bRet = ::WriteFile(m_hConfigFile,
        szURL,
        defURL_MAX_LEN,
        &dwWritten,
        NULL
    );
    if(!bRet || defURL_MAX_LEN != dwWritten)
        return false;

    //Write local path file name
    char  szLocalPFName[defPFN_MAX_LEN];
    if(strlen(pszLocalPFName) >= defPFN_MAX_LEN)
        return false;

    strncpy(szLocalPFName, pszLocalPFName, defPFN_MAX_LEN);

    bRet = ::WriteFile(m_hConfigFile,
        szLocalPFName,
        defPFN_MAX_LEN,
        &dwWritten,
        NULL
    );
    if(!bRet || defPFN_MAX_LEN != dwWritten)
        return false;

    //Write length of file
    bRet = ::WriteFile(m_hConfigFile,
        &uFileLength,
        sizeof(unsigned),
        &dwWritten,
        NULL
    );
    if(!bRet || sizeof(unsigned) != dwWritten)
        return false;

    //Write number of block
    bRet = ::WriteFile(m_hConfigFile,
        &uBlockNumber,
        sizeof(unsigned),
        &dwWritten,
        NULL
    );
    if(!bRet || sizeof(unsigned) != dwWritten)
        return false;

    return true;
}


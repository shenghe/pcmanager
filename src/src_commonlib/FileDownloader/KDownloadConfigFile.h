/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the configuration file object
 */

#ifndef K_DOWNLOAD_CONFIG_FILE_H_
#define K_DOWNLOAD_CONFIG_FILE_H_

#include "KFileDownloadDefine.h"

/**
 * @struct  st_block_progress_info
 * 
 * Define structure to store progress information
 * for downloading of file blocks
 */
typedef struct st_block_progress_info
{
    //The identity of the block
    unsigned        uID;
    
    //The start position of the block
    //The block >= uStart
    unsigned        uStart;

    //The end position of the block
    //The block < uEnd
    unsigned        uEnd;

    //How many bytes completed
    unsigned        uCompleted;

    //Reserved 
    unsigned        uReserved;

}ST_BLOCK_PROGRESS_INFO, ST_BP_INFO;

/**
 * Define new type, Block Progress Information List
 */
typedef std::list<ST_BP_INFO>   BPInfoList;

/**
 * @struct  st_download_config_info
 * To store configuration information
 */
typedef struct st_download_Config_info
{
    //The URL of file being downloaded
    char            szDownloadURL[defURL_MAX_LEN];

    //The local path file name
    char            szLocalPFName[defPFN_MAX_LEN];

    //The length of file being downloaded
    unsigned        uFileLength;

    //The number of block 
    //that the file being downloaded is split to
    unsigned        uBlockNumber;

    //The block downloading progress list of the file
    BPInfoList      BPInfolist;
    
}ST_DOWNLOAD_CONFIG_INGO, ST_DC_INFO;

class CDownloadConfigFile
{
public:
    //The default constructor
    CDownloadConfigFile(void)
    :m_hConfigFile(INVALID_HANDLE_VALUE){}

    //The default destructor
    ~CDownloadConfigFile(void){}

public:
    /**
     * @brief Open/Create a configuration file
     * If a file exists, the function opens it
     * If a file does not exist, the function creates a file
     *
     * @param[IN]   pszFilename     The path file name
     *
     * @return If Succeeded
     * - true   yes
     *   false  no
     */ 
    bool Create(const char* pszFileName);
    
    /**
     * @brief Close the configuration file opened
     * 
     * @return If Succeeded
     * - true   yes
     *   false  no
     */
    bool Close(void);

    /**
     * @brief Read the Config-Info
     *
     * @param[OUT]  stDCInfo    The Config-Info
     *
     * @return If Succeeded
     * - true   yes
     *   false  no
     */
    bool ReadConfigInfo(ST_DC_INFO& stDCInfo);

    /**
     * @brief Write the Config-Info
     *
     * @param[IN]   stDCInfo    The Config-Info
     * 
     * @return If Succeeded
     * - true   yes
     *   false  no
     */ 
    bool WriteConfigInfo(ST_DC_INFO& stDCInfo);

    /**
     * @brief Write the progress part of Config-Info
     *
     * @param[IN]   infoList    The progress information
     * 
     * @return If Succeeded
     * - true   yes
     *   false  no
     */
    bool WriteConfigProgressInfo(BPInfoList& infoList);

    /**
     * @brief Write the head of Config-Info
     *
     * @param[IN]   pszDownloadURL  The URL
     * @param[IN]   pszLocalPFName  The path file name
     * @param[IN]   uFileLength     The length of file
     * @param[IN]   uBlockNumber    The number of blocks
     *
     * @return If Succeeded
     * - true   yes
     *   false  no
     */
    bool WriteConfigHeadInfo(
        const char*     pszDownloadURL,
        const char*     pszLocalPFName,
        unsigned        uFileLength,
        unsigned        uBlockNumber
    );

private:
    CDownloadConfigFile(const CDownloadConfigFile&);
    CDownloadConfigFile& operator=(const CDownloadConfigFile&);

private:
    //The handle of config file
    HANDLE      m_hConfigFile;
};

#endif//K_DOWNLOAD_CONFIG_FILE_H_


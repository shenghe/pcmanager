/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the file object(being downloaded)
 */

#ifndef K_DOWNLOAD_FILE_H_
#define K_DOWNLOAD_FILE_H_

#include <Windows.h>

class CDownloadFile
{
public:
    //The default constructor
    CDownloadFile(void):m_hFile(INVALID_HANDLE_VALUE){}
    
    //The default destructor
    ~CDownloadFile(void){}

public:
    /**
     * @brief Open/Create a file
     * If the file exists, just open it, or create a new file
     *
     * @param[IN]   pszPathFileName The path file name
     * 
     * @return If succeeded
     * -true    yes
     *  false   no
     */
    bool Create(IN const char* pszPathFileName);

    /**
     * @brief Close the file
     * 
     * @return If succeeded
     * -true    yes
     *  false   no
     */
    bool Close(void);

    /**
     * @brief Empty the file
     * Make the file size = 0
     *
     * @return If succeeded
     * -true    yes
     *  false   no
     */
    bool Empty(void);

    /**
     * @Brief Move the file pointer from the beginning
     *
     * @param[IN]   lMove   How many bytes to move
     *
     * @return If succeeded
     * -true    yes
     *  false   no
     */
    bool SetPointerFromBegin(IN long lMove);

public:
    /**
     * @Brief Write data to a file
     * Write from the current file pointer
     *
     * @param[IN]   pBuffer The pointer to data buffer
     * @param[IN]   ulBytes How many bytes to write
     *
     * @return If succeeded
     * -true    yes
     *  false   no
     */
    inline bool Write(IN const void* pBuffer, IN unsigned long ulBytes);

private:
    //The underlying file handle
    HANDLE      m_hFile;
};

inline bool CDownloadFile::Write(IN const void* pBuffer, IN unsigned long ulBytes)
{
    DWORD dwWritten = 0;
    if(!WriteFile(m_hFile, pBuffer, ulBytes, &dwWritten, NULL))
        return false;
    return (dwWritten == ulBytes);
}

#endif//K_DOWNLOAD_FILE_H_


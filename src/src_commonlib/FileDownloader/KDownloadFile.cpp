/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the file object(being downloaded)
 */

#include "KDownloadFile.h"

bool CDownloadFile::Create(const char *pszPathFileName)
{
    if(NULL == pszPathFileName)
        return false;

    //Close file If It has been opened
    this->Close();

    //Create file
    m_hFile = CreateFileA(pszPathFileName,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    return (INVALID_HANDLE_VALUE != m_hFile);
}

bool CDownloadFile::Close(void)
{
    if(INVALID_HANDLE_VALUE != m_hFile)
        CloseHandle(m_hFile);
  
    m_hFile = INVALID_HANDLE_VALUE;
    return true;
}

bool CDownloadFile::Empty(void)
{
    if(!SetPointerFromBegin(0))
        return false;

    return SetEndOfFile(m_hFile) ? true:false; 
}

bool CDownloadFile::SetPointerFromBegin(long lMove)
{
    DWORD dwRet = 0;
    dwRet = SetFilePointer(m_hFile, lMove, NULL, FILE_BEGIN);
    return (INVALID_SET_FILE_POINTER != dwRet);
}

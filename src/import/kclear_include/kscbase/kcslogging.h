//////////////////////////////////////////////////////////////////////////
// 
//  @Create Author  : Zhang Zexin 
//  @Create Date    : 2011 - 01 - 28
//  @Description    : 迷你日志模块，为垃圾清理
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////
#include <sstream>
//////////////////////////////////////////////////////////////////////////

typedef enum log_level
{
    LOG_LEVEL_ONEKEYCLEAN         = 0x01,
    LOG_LEVEL_TRASHCLEAN          = 0x02,
    LOG_LEVEL_REGCLEAN            = 0x04,
    LOG_LEVEL_TRACKCLEAN          = 0x08
}LOG_LEVEL;

#define CLEAR_LOG_FILE  "%s_%s.log"
//////////////////////////////////////////////////////////////////////////
#define KCLEAR_LOG(Condition) \
    CKcsLogging(Condition)
//////////////////////////////////////////////////////////////////////////
class CKcsLogging
{
public:
    CKcsLogging();
    CKcsLogging(LOG_LEVEL level);
    virtual ~CKcsLogging();

    std::ostream& stream() { return m_stream; }

    virtual CKcsLogging& operator << (int nVal);
    virtual CKcsLogging& operator << (unsigned int nVal);
    virtual CKcsLogging& operator << (std::string strVal);
    virtual CKcsLogging& operator << (long nVal);
    virtual CKcsLogging& operator << (ULONGLONG uVal);

    /**
     * 清理 n天前的日志
     * 默认清理 7天前的日志
     **/
    void ClearLogFileByDays(int nLastDay = 7);

    CKcsLogging& Log(LOG_LEVEL nLevel);

protected:
    std::ostringstream m_stream;
    LOG_LEVEL m_eLevel;
    CRITICAL_SECTION m_LogLock;  

protected:
    std::string _GetFix();
    BOOL _CreateDirectoryNested(LPSTR lpszDir);
    void _Lock();
    void _UnLock();
};

/***************************************************************
 * @date:   2007-12-04
 * @author: BrucePeng
 * @brief:  
 */
#ifndef K_DEBUG_INFO_RECORD_H_
#define K_DEBUG_INFO_RECORD_H_

#include <Windows.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief Record what happened simply
 *
 * @param[IN]   pszTitile   Just the type of message
 *                          for example, error
 * @param[IN]   pszText     The message
 * @param[IN]   nThreadID   The ID of thread who record
 *                          Default: 0  --- the main thread
 *
 * @return If Record successfully
 * -true    YES
 *  false   NO
 */
bool RecordWhatHappened(
    IN  const char* pszTitle, 
    IN  const char* pszText,
    IN  int         nThreadID  = 0
);

#ifdef __cplusplus
}
#endif

#endif //K_DEBUG_INFO_RECORD_H_

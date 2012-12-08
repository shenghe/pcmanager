//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KAVPublic.h
//  Version     :   1.0
//  Creater     :   Freeway Chen
//  Date        :   2000-8-29 0:19:56
//  Comment     :   Kingsoft AntiVirus Public header file
//
//////////////////////////////////////////////////////////////////////////////////////



#ifndef _KAVPUBLIC_H
#define _KAVPUBLIC_H    1

//#include "KSCOMError.h"
//#include "KString.h"


#define KAV_USE_ARGUMENT(arg) (arg)


//#define KAV_PROCESS_ERROR(Condition)    \
//    if (!(Condition)) \
//        goto Exit0;
#define KAV_PROCESS_ERROR(Condition)    if (!(Condition))   goto Exit0;


//#define KAV_PROCESS_SUCCESS(Condition)    \
//    if (Condition) \
//        goto Exit1;
#define KAV_PROCESS_SUCCESS(Condition)  if (Condition)      goto Exit1;


//#define KAV_COM_PROCESS_ERROR(Condition)    \
//    if (FAILED(Condition))  \
//        goto Exit0; 
#define KAV_COM_PROCESS_ERROR(Condition)    if (FAILED(Condition))      goto Exit0;


//#define KAV_COM_PROCESS_SUCCESS(Condition)  \
//    if (SUCCEEDED(Condition))   \
//        goto Exit1;
#define KAV_COM_PROCESS_SUCCESS(Condition)  if (SUCCEEDED(Condition))   goto Exit1;


//#define KAV_COM_PROCESS_SUCCESS(Condition)    \
//    if (SUCCEEDED(Condition))  \
//        goto Exit1;
#define KAV_COM_PROCESS_SUCCESS(Condition)  if (SUCCEEDED(Condition))   goto Exit1;


//#define KAV_PROCESS_ERROR_RET_CODE(Condition, Code)    \
//    if (!(Condition)) \
//    {   \
//        nResult = Code;    \
//        goto Exit0; \
//    }
#define KAV_PROCESS_ERROR_RET_CODE(Condition, Code) if (!(Condition)) { nResult = Code; goto Exit0; }


// KAV_COM_PROCESS_ERROR_RETURN_ERROR
//#define KAV_COM_PROC_ERR_RET_ERR(Condition) \
//    if (FAILED(Condition))  \
//    {   \
//        krResult = Condition;   \
//        goto Exit0; \
//    }
#define KAV_COM_PROC_ERR_RET_ERR(Condition) if (FAILED(Condition)) { krResult = Condition; goto Exit0; }


// KAV_COM_PROCESS_ERROR_RETURN_CONDITION
//#define KAV_COM_PROC_ERR_RET_CONDITION(Condition) \
//    if (FAILED(Condition))  \
//    {   \
//        krResult = Condition;   \
//        goto Exit0; \
//    }
#define KAV_COM_PROC_ERR_RET_CONDITION(Condition)   if (FAILED(Condition)) { krResult = Condition; goto Exit0; }


//#define KAV_COM_PROC_ERROR_RET_CODE(Condition, Code) \
//    if (FAILED(Condition))  \
//    {   \
//        krResult = Code;   \
//        goto Exit0; \
//    }
#define KAV_COM_PROC_ERROR_RET_CODE(Condition, Code)    if (FAILED(Condition)) { krResult = Code; goto Exit0; }


//#define KAV_COM_RELEASE(pInterface) \
//    if (pInterface) \
//    {   \
//        (pInterface)->Release();  \
//        ((void *)(pInterface)) = NULL;  \
//    }
#define KAV_COM_RELEASE(pInterface) if (pInterface) { (pInterface)->Release(); ((pInterface)) = NULL; }


//#define KAV_DELETE_ARRAY(pArray)    \
//    if (pArray) \
//    { \
//        delete [](pArray);  \
//        (pArray) = NULL;    \
//    }
#define KAV_DELETE_ARRAY(pArray)    if (pArray) { delete [](pArray); (pArray) = NULL; }


//#define KAV_DELETE(p)    \
//    if (p) \
//    { \
//        delete (p);  \
//        (p) = NULL;    \
//    }
#define KAV_DELETE(p)   if (p) { delete (p); (p) = NULL; }


#ifdef _DEBUG

//#define KAV_ASSERT_EXIT(Condition)  assert(Condition); \
//    if (!(Condition))     \
//        goto Exit0;
#define KAV_ASSERT_EXIT(Condition)  assert(Condition); if (!(Condition)) goto Exit0;

#else

//#define KAV_ASSERT_EXIT(Condition)  \
//    if (!(Condition))     \
//        goto Exit0;
#define KAV_ASSERT_EXIT(Condition)  if (!(Condition))   goto Exit0;


#endif

#ifndef KAV_HANDLE_DEFINED
#define KAV_HANDLE_DEFINED

typedef void *  KAVHANDLE;    

#endif // KAV_HANDLE_DEFINED

#endif // _KAVPUBLIC_H

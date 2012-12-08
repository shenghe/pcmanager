#ifndef MD5_INC_
#define MD5_INC_

/**
 * @defgroup kxearchitecture_md5_group KXEngine MD5 Function
 * @section kxearchitecture_md5_group_support 提供者:廖兰君
 * @ref kxearchitecture_md5_example_page
 * @{
 */

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

/**
 * @brief 用于MD5计算的数据结构
 */
typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} cli_md5_ctx;

/**
 * @brief 初始化MD5的计算结构
 * @param[out] ctx 初始化好的结构进行返回
 */
extern void cli_md5_init(cli_md5_ctx *ctx);

/**
 * @brief 根据data数据进行MD5的计算
 * @param[in] ctx 通过cli_md5_init初始化的数据
 * @param[in] data 需要进行MD5计算的buffer
 * @param[in] size 指定的data的长度
 */
extern void cli_md5_update(cli_md5_ctx *ctx, void *data, unsigned long size);

/**
 * @brief 根据data数据进行MD5的计算
 * @param[in] result 获取计算得到的MD5的结果,该buffer为16字节长
 * @param[in] ctx 通过cli_md5_init及cli_md5_update计算后的数据结构
 */
extern void cli_md5_final(unsigned char *result, cli_md5_ctx *ctx);

#ifdef __cplusplus
};
#endif

//////////////////////////////////////////////////////////////////////////

/**
 * @}
 */

/**
 * @page kxearchitecture_md5_example_page The Example of MD5 Function
 * @section kxearchitecture_md5_example_support 提供者:廖兰君
 * @section kxearchitecture_md5_example KXEngine MD5 Example
 * @code

 #include "MD5.h"

int PathToNameHash( WCHAR *pwszFilePath,ULONGLONG &ullHash )
{
	ULONG		ulLoopCount;
	ULONG		ulHash1Tmp	=	0;
	ULONG		ulHash2Tmp	=	0;
	ULONG       ulFileNameLength = 0;
	PUCHAR		byPtr = NULL;
	UCHAR		ucTemp = 0;
	int			nStatus = 0;

	if ( pwszFilePath == NULL )
	{
		nStatus = ERROR_INVALID_PARAMETER;
		goto Exit0;
	}

	byPtr = (PUCHAR)pwszFilePath;
	ulFileNameLength = ( ULONG )wcslen( pwszFilePath );
	if( ulFileNameLength >= MAX_PATH )
	{
		return ERROR_INVALID_PARAMETER;
	}
	ulFileNameLength = ulFileNameLength * sizeof( WCHAR );

	for(ulLoopCount = 0; ulLoopCount < ulFileNameLength; ulLoopCount++)
	{
		ucTemp	= g_KAV_LowerToUpperTable[ byPtr[ ulLoopCount ] ];

		ulHash1Tmp = ( ulHash1Tmp * 37 ) + ucTemp;
		ulHash2Tmp = ( ulHash2Tmp * 31 ) + ucTemp;
	}
	ullHash = ulHash1Tmp;
	ullHash = ullHash<<32;
	ullHash += ulHash2Tmp;

Exit0:
	return nStatus;
}

int PathToMD5( WCHAR *wszFilePath,MD5 FileMD5)
{
	if( NULL == FileMD5 )
		return -1; 

	HANDLE hTheFile = NULL;
	int iStatus = 0;
	DWORD nRet = 0;    
	cli_md5_ctx md5ctx;
	BYTE *lpBuffer = NULL;

	hTheFile =
		CreateFile( (LPCWSTR)wszFilePath,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if( INVALID_HANDLE_VALUE == hTheFile)
	{
		iStatus = GetLastError();
		return iStatus;
	}

	lpBuffer = new BYTE[0x1000];
	if (!lpBuffer)
	{
		iStatus = 2;
		goto Exit0;
	}

	cli_md5_init(&md5ctx);

	for (;;)
	{
		iStatus = ReadFile( hTheFile,
			lpBuffer,
			0x1000,
			&nRet,
			NULL );
		if (!iStatus || !nRet)
			break;

		//
		// Idle for IO & CPU
		// 
		cli_md5_update(&md5ctx, lpBuffer, nRet);
	}

	cli_md5_final(FileMD5, &md5ctx);
	iStatus = 0;
Exit0:
	if( NULL != hTheFile )
		CloseHandle( hTheFile );
	if (lpBuffer)
	{
		delete[] lpBuffer;
		lpBuffer = NULL;
	}
	return iStatus;
}
 * @endcode
 */



#endif  // !MD5_INC_
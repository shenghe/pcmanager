//////////////////////////////////////////////////////////////////////
///		@file		IKxEMemFeatureOp.h
///		@author		YangBin
///		@date		2009-01-04
///
///		@brief		内存特征的接口定义
//////////////////////////////////////////////////////////////////////
#ifndef _IKXE_MEMORY_RES_OPERATE_H_
#define _IKXE_MEMORY_RES_OPERATE_H_

// 内存特征文件头结构体
typedef struct _KXE_MEM_FEATURE_FILE_HEADER
{
    DWORD dwFileFlag;       //文件标识
    SYSTEMTIME timestamp;   //文件生成时间
    DWORD dwDataSize;       //数据大小（不包括文件头）
    DWORD dwItemNum;        //保护程序个数
}KXE_MEM_FEATURE_FILE_HEADER, *PKXE_MEM_FEATURE_FILE_HEADER;

typedef struct _KXE_MEM_FEATURE_INDEX
{
    DWORD dwId;             //对每个程序自定义的ID，唯一的
    DWORD dwSigOffset;      //相应程序的特征数据块在文件中的偏移
}KXE_MEM_FEATURE_INDEX, *PKXE_MEM_FEATURE_INDEX;

//一个结构代表一个特征
typedef struct _KXE_MEM_FEATURE_SIGNATURE_BLOCK
{
    DWORD dwFeatureSize;           //特征大小
    DWORD dwFeatureOffset;         //特征偏移
    DWORD dwVirusNameSize;         //病毒名大小
    DWORD dwVirusNameOffset;       //病毒名偏移
    UCHAR data[1];                 //数据：先特征（单字节）后病毒名（宽字节）
}KXE_MEM_FEATURE_SIGNATURE_BLOCK, *PKXE_MEM_FEATURE_SIGNATURE_BLOCK;

//一个程序对应一个KXE_MEM_FEATURE_SIGNATURE
typedef struct _KXE_MEM_FEATURE_SIGNATURE
{
    DWORD dwBlockNum;   //特征个数
    KXE_MEM_FEATURE_SIGNATURE_BLOCK blocks[1];  //特征块数组，个数由dwBlockNum决定
}KXE_MEM_FEATURE_SIGNATURE, *PKXE_MEM_FEATURE_SIGNATURE;

#define KXE_MEM_FEATURE_GET_INDEX_NUM( _pFileHeader )    (((PKXE_MEM_FEATURE_FILE_HEADER)(_pFileHeader))->dwItemNum)

#define KXE_MEM_FEATURE_GET_FIRST_INDEX( _pFileHeader )  (PKXE_MEM_FEATURE_INDEX)((PCHAR)(_pFileHeader) + sizeof(KXE_MEM_FEATURE_FILE_HEADER))

#define KXE_MEM_FEATURE_GET_SIGNATURE( _pFileHeader, _SigOffest )     (PKXE_MEM_FEATURE_SIGNATURE)((PCHAR)KXE_MEM_FEATURE_GET_FIRST_INDEX( _pFileHeader ) + KXE_MEM_FEATURE_GET_INDEX_NUM( _pFileHeader ) * sizeof(KXE_MEM_FEATURE_INDEX) + (_SigOffest) )

#define KxEGetStructFieldOffSet( type, field ) ( ULONG_PTR )( &( ( type* ) 0 )->field  )

#define KXE_MEM_FEATURE_GET_FEATURE( _pSigBlock )   (PUCHAR)(_pSigBlock) + KxEGetStructFieldOffSet( KXE_MEM_FEATURE_SIGNATURE_BLOCK, data )

#define KXE_MEM_FEATURE_GET_VIRUSNAME( _pSigBlock )   (PWCHAR)((PUCHAR)(_pSigBlock) + KxEGetStructFieldOffSet( KXE_MEM_FEATURE_SIGNATURE_BLOCK, data ) + ((PKXE_MEM_FEATURE_SIGNATURE_BLOCK)(_pSigBlock))->dwVirusNameOffset)

#define KXE_MEM_FEATURE_GET_BLOCK_SIZE( _pSigBlock )  ((PKXE_MEM_FEATURE_SIGNATURE_BLOCK)(_pSigBlock))->dwFeatureSize + ((PKXE_MEM_FEATURE_SIGNATURE_BLOCK)(_pSigBlock))->dwVirusNameSize + KxEGetStructFieldOffSet( KXE_MEM_FEATURE_SIGNATURE_BLOCK, data )

#define MEMORY_FEATURE_FILE_FLAG    'fmgK'    //kingsoft game memory feature

/**
* @class        IKxEMemFeatureOpInFile
* @brief        防御子系统内存特征文件操作接口
*
* @details      定义直接操作内存特征文件的接口
*/
interface IKxEMemFeatureOpInFile
{
    /**
    * @brief        初始化内存资源
    * @param[in]    pwszMemResFileName 内存资源文件名	
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall Initialize( 
        IN PWCHAR pwszMemResFileName
        ) = 0;

    /**
    * @brief        反初始化内存资源
    * @param[in]    	
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall UnInitialize( ) = 0;

    /**
    * @brief        向内存特征文件插入一条特征
    * @param[in]    ulId            程序ID
    *               pszFeature      特征描述
    *               ulFeatureSize   特征大小
    *               pwszVirusName   病毒名
    *               ulVirusNameSize 病毒名大小（非字符个数，是字节）
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall InsertFeature( 
        IN ULONG  ulId, 
        IN PUCHAR pszFeature, 
        IN ULONG  ulFeatureSize,
        IN PWCHAR pwszVirusName,
        IN ULONG  ulVirusNameSize
        ) = 0;
    
    /**
    * @brief        从内存特征文件删除指定程序的某条特征
    * @param[in]    ulId            程序ID
    *               pszFeature      特征描述
    *               ulFeatureSize   特征大小
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall DeleteFeature( 
        IN ULONG  ulId, 
        IN PUCHAR pszFeature, 
        IN ULONG  ulFeatureSize 
        ) = 0;

    /**
    * @brief        从内存特征文件删除指定程序的所有特征
    * @param[in]    ulId    程序ID
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall DeleteFeatures( 
        IN ULONG ulId
        ) = 0;

    /**
    * @brief        删除整个内存特征文件
    * @param[in]    无
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall DeleteAll() = 0;

    /**
    * @brief        修改某程序的某条特征
    * @param[in]    ulId               程序ID
    *               pszOldFeature      旧特征描述
    *               ulOldFeatureSize   旧特征大小
    *               pszNewFeature      新特征描述
    *               ulNewFeatureSize   新特征大小
    *               pwszOldVirusName   旧病毒名
    *               ulOldVirusNameSize 旧病毒名大小
    *               pwszNewVirusName   新病毒名
    *               ulNewVirusNameSize 新病毒名大小
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall ModifyFeature( 
        IN ULONG  ulId, 
        IN PUCHAR pszOldFeature, 
        IN ULONG  ulOldFeatureSize, 
        IN PUCHAR pszNewFeature,
        IN ULONG  ulNewFeatureSize,
        IN PWCHAR pwszOldVirusName,
        IN ULONG  ulOldVirusNameSize,
        IN PWCHAR pwszNewVirusName,
        IN ULONG  ulNewVirusNameSize
        ) = 0;

    /**
    * @brief        将修改写入内存特征文件中
    * @param[in]    无
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall Flush() = 0;
};

/**
* @class        IKxEMemFeatureReadOp
* @brief        防御子系统读内存特征文件操作接口
*
* @details      定义读取内存特征文件的接口
*/
interface IKxEMemFeatureReadOp
{
    /**
    * @brief        初始化
    * @param[in]    pwszMemResFileName      内存资源文件名	
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall Initialize( 
        IN PWCHAR pwszMemResFileName
        ) = 0;

    /**
    * @brief        反初始化
    * @param[in]    	
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall UnInitialize( ) = 0;

    /**
    * @brief        根据程序ID得到相应内存特征
    * @param[in]    ulId           程序ID
                    ppszBlocks     内存特征buffer
                    pdwBlockNum    内存特征块数                   
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall GetFeaturesByProcId( 
        IN  ULONG  ulId, 
        OUT PUCHAR *ppszBlocks, 
        OUT PDWORD pdwBlockNum 
        ) = 0;

    /**
    * @brief        得到内存文件版本
    * @param[out]   pTime   返回的版本信息                   
    * @return       S_OK    成功
    *               E_FAIL  失败
    */
    virtual HRESULT __stdcall GetMemFileVersion( IN SYSTEMTIME *pTime ) = 0;

    /**
    * @brief        得到内存特征块数
    * @param[in]    无                  
    * @return       != 0    成功
    *               == 0    失败
    */
    virtual DWORD __stdcall GetMemProcNum() = 0;

    /**
    * @brief        得到第一个程序的特征块
    * @param[out]   pulProcId   返回的程序ID
    *               ppszBlocks  返回的程序内存特征块
    *               pdwBlockNum 返回的程序内存特征块数
    * @return       TRUE    成功
    *               FALSE   失败
    */
    virtual BOOL __stdcall GetFirstProc( 
        OUT ULONG  *pulProcId, 
        OUT PUCHAR *ppszBlocks,
        OUT PDWORD pdwBlockNum 
        ) = 0;

    /**
    * @brief        得到下一个程序的特征块
    * @param[out]   pulProcId   返回的程序ID
    *               ppszBlocks  返回的程序内存特征块
    *               pdwBlockNum 返回的程序内存特征块数
    * @return       TRUE    成功
    *               FALSE   失败，即遍历结束
    */
    virtual BOOL __stdcall GetNextProc(
        OUT ULONG  *pulProcId, 
        OUT PUCHAR *ppszBlocks,
        OUT PDWORD pdwBlockNum 
        ) = 0;
    
};

#endif
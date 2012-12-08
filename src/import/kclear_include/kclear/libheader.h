
#pragma once

#include <atlfile.h>
#include <atldef.h>
#include <atlstr.h>

/*	说明

	CDataFileLoader	loader;

	//获取库头的结构体
	BkDatLibHeader	header;
	loader.GetLibDatHeader( _T("C:\\pluginlib.dat"),header);

	//获取整个库的内容，buffer自动释放
	BkDatLibContent ct;
	loader.GetLibDatContent( _T("C:\\pluginlib.dat"),ct );

	//获取整个库的内容，buffer不自动释放
	BkDatLibContent ct(FALSE);
	loader.GetLibDatContent( _T("C:\\pluginlib.dat"),ct );
	delete ct.pBuffer;

	// 获取整个库的内容，自动转为CString
	BkDatLibContent2 ct2;
	loader.GetLibDatContent2( _T("C:\\pluginlib.dat"),ct2 );

	// 通过CString生成库，需要填写版本号，库格式的版本号，库的类型
	LARGE_INTEGER	i = {1};
	loader.Save( _T("C:\\pluginlib2.dat"),BkDatLibEncodeParam2(enumLibTypePlugine,i,ct2.strXML,1).GetEncodeParam() );

	// 通过Buffer生成库，需要填写版本号，库格式的版本号，库的类型
	LARGE_INTEGER	i = {1};
	BkDatLibEncodeParam	paramx(enumLibTypePlugine,i,pBuffer,nLen,1);
	loader.Save( _T("C:\\pluginlib2.dat"),paramx);

	// 通过文件生成
	BkDatLibEncodeFileParam	fileParam( _T("C:\\pluginlib2.xml"), nType, llVer, nFromatVer );
	if ( CDataFileLoader().Save( _T("C:\\pluginlib2.dat"),fileParam.GetEncodeParam() ) )
	iRet = 1;

	//生成升级库
	BkUpdateDatLibEncodeParam	uper(nTypex, llVerx, llVerForx,pBufferx,nLenx)
	CDataFileLoader().Save( _T("C:\\pluginlib2.dat"),uper);
*/


#define MED_CRYPT_XOR		0x1
#define DAT_LIB_MARK		'BKLH'
#define UPDATE_DAT_MARK		0x80000000
#define KEY_XOR_DEFA		0xab

#define IsUpdateDat(nType)		(nType&UPDATE_DAT_MARK)
#define MarkdatUpdate(nType)	(nType|UPDATE_DAT_MARK)
#define UnMarkdatUpdate(nType)	(nType&0x7fffffff)


typedef enum
{
	enumLibTypePlugine = 1,

	enumLibTypeVulFixSystem=100,
	enumLibTypeVulFixSystem64=enumLibTypeVulFixSystem+1,
	enumLibTypeVulFixOffice=enumLibTypeVulFixSystem+2,
	enumLibTypeVulFixOffice64=enumLibTypeVulFixSystem+3,
	enumLibTypeVulFixSoft=enumLibTypeVulFixSystem+4,

    enumLibTypeknowledge = 400,
    enumLibTypeBKMP = 401,
    enumLibTypeScanIE   = 500,
    enumLibTypeLibIEScan   = 501
};

#pragma pack(push,4)

struct BkDatLibHeader
{
	DWORD           dwSign;			// HEADER_SIGN
	DWORD           dwSize;			// header size
	LARGE_INTEGER   llVersion;		// 库的版本,,Update 升级后的库版本
	DWORD			nFormatVer;		// 库结构的版本
	DWORD           dwType;			// 库的类型
	DWORD           dwCheckSum;		// CRC校验
	LARGE_INTEGER	llUpdateForVer;	// 为哪个版本的库升级的包
	DWORD			nOffsetXML;		// XML的偏移位置
	DWORD			nSizeXML;		// XML的大小
	DWORD			nCryptMethod;	// 加密方式
	BYTE			szKey[32];		// 加密密钥
};

#pragma pack(pop)

class BkDatLibContent				// 获取库的所有内容的结构体
{
public:
	BkDatLibContent(BOOL bAutoRelease=TRUE)
	{
		pBuffer = NULL;
		nLen	= 0;
		bRelease= bAutoRelease; 
	};
	BOOL	SaveToFile(LPCTSTR	lpFileName)
	{
		CAtlFile	filex;
		if (SUCCEEDED(filex.Create(lpFileName,GENERIC_WRITE,0,CREATE_ALWAYS)))
		{
			return	SUCCEEDED(filex.Write(pBuffer,nLen));
		}
		return	FALSE;
	}
	virtual ~BkDatLibContent()
	{
		if ( bRelease )
		{
			if (pBuffer)
				delete pBuffer;
		}
		pBuffer = NULL;
		nLen = 0;
	};

	BkDatLibHeader	header;
	BYTE*			pBuffer;
	DWORD			nLen;
	BOOL			bRelease;
};

class BkDatLibContent2
{
public:
	BkDatLibContent2()
	{

	};

	BkDatLibContent	content;
	CStringA		strXML;
};

class BkDatLibEncodeParam
{
public:
	BkDatLibEncodeParam( DWORD nTypex, LARGE_INTEGER llversionx, BYTE* pBufferx, DWORD nLenx, DWORD nFormatTypex, BYTE xbtKey=KEY_XOR_DEFA)
	{
		dwType			= nTypex;
		dwFormatType	= nFormatTypex;
		llVersion		= llversionx;
		pBuffer			= pBufferx;
		nLen			= nLenx;
		btKey			= xbtKey;

		llVerFor.QuadPart = 0;
	};

	DWORD			dwType;			// 库类型
	LARGE_INTEGER	llVersion;		// 库的版本
	LARGE_INTEGER	llVerFor;		// 为哪个版本的库升级的包
	DWORD			dwFormatType;	// 库结构版本
	BYTE*			pBuffer;
	DWORD			nLen;
	BYTE			btKey;
};	

class BkDatLibEncodeFileParam
{
public:
	BkDatLibEncodeFileParam(LPCTSTR strXMLx, 
							DWORD nTypex, 
							LARGE_INTEGER llversionx, 
							DWORD nFormatTypex,
							BYTE  xbtKey=KEY_XOR_DEFA)
	{
		dwType			= nTypex;
		dwFormatType	= nFormatTypex;
		llVersion		= llversionx;
		strXMLFile		= strXMLx;
		btKey			= xbtKey;
		lpANSIBuffer	= NULL;
	};
	virtual ~BkDatLibEncodeFileParam()
	{
		if ( lpANSIBuffer != NULL)
		{
			delete lpANSIBuffer;
			lpANSIBuffer = NULL;
		}
	}

	BkDatLibEncodeParam GetEncodeParam()
	{
		BkDatLibEncodeParam	paramx(dwType,llVersion,NULL,0,dwFormatType,btKey);

		if ( !strXMLFile.IsEmpty() )
		{
			CAtlFile	filex;
			if ( SUCCEEDED(filex.Create(strXMLFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) )
			{
				DWORD		nSize = 0;
				ULONGLONG	nllSize = 0;
				filex.GetSize(nllSize);
				nSize = (DWORD)nllSize;

				if ( lpANSIBuffer != NULL)
				{
					delete lpANSIBuffer;
					lpANSIBuffer = NULL;
				}
				lpANSIBuffer = new BYTE[nSize];
				filex.Read(lpANSIBuffer,nSize);
				paramx.pBuffer = lpANSIBuffer;
				paramx.nLen	   = nSize;
			}
		}

		return paramx;
	}


	DWORD			dwType;			// 库类型
	LARGE_INTEGER	llVersion;		// 库的版本
	DWORD			dwFormatType;	// 库结构版本
	BYTE			btKey;
	CString			strXMLFile;

protected:
	BYTE*			lpANSIBuffer;
};

class BkDatLibEncodeParam2
{
public:
	BkDatLibEncodeParam2( DWORD nTypex, LARGE_INTEGER llversionx, const CString & strXMLx, DWORD nFormatTypex, BYTE xbtKey=KEY_XOR_DEFA)
	{
		dwType			= nTypex;
		dwFormatType	= nFormatTypex;
		llVersion		= llversionx;
		strXML			= strXMLx;
		lpANSIBuffer	= NULL;
		btKey			= xbtKey;
	};
	virtual ~BkDatLibEncodeParam2()
	{
		if ( lpANSIBuffer != NULL)
		{
			delete lpANSIBuffer;
			lpANSIBuffer = NULL;
		}
	}

	BkDatLibEncodeParam GetEncodeParam()
	{
		if ( lpANSIBuffer != NULL)
		{
			delete lpANSIBuffer;
			lpANSIBuffer = NULL;
		}

		BkDatLibEncodeParam	paramx(dwType,llVersion,NULL,0,dwFormatType,btKey);

		if ( !strXML.IsEmpty() )
		{
			INT				nAnsiLen = 0;
#ifdef _UNICODE
			nAnsiLen		= ::WideCharToMultiByte(CP_THREAD_ACP, 0, strXML, -1, NULL, 0, NULL, NULL);
			lpANSIBuffer	= new BYTE[nAnsiLen + 1];
			memset(lpANSIBuffer, 0, nAnsiLen);
			::WideCharToMultiByte(CP_THREAD_ACP,0, strXML,-1,(LPSTR)lpANSIBuffer,nAnsiLen,NULL,NULL);
#else
			nAnsiLen	 = (INT)strlen(strXML)+1;
			lpANSIBuffer = new BYTE[nAnsiLen];
			memcpy(lpANSIBuffer,strXML,nAnsiLen);
#endif
			paramx.pBuffer = lpANSIBuffer;
			paramx.nLen	   = nAnsiLen;
		}

		return paramx;
	}


	DWORD			dwType;			// 库类型
	LARGE_INTEGER	llVersion;		// 库的版本
	DWORD			dwFormatType;	// 库结构版本
	CString			strXML;
	BYTE			btKey;

protected:
	BYTE*			lpANSIBuffer;
};	

class BkUpdateDatLibEncodeParam : public BkDatLibEncodeParam
{
public:
	BkUpdateDatLibEncodeParam(DWORD nTypex, LARGE_INTEGER llVerx, LARGE_INTEGER llVerForx,BYTE* pBufferx, DWORD nLenx, BYTE xBtKey=KEY_XOR_DEFA)
		: BkDatLibEncodeParam(MarkdatUpdate(nTypex), llVerx, pBufferx, nLenx, 0, xBtKey)
	{
		llVerFor = llVerForx;
	}
};

class CDataFileLoader
{
public:

	BOOL SaveUpdateCab(LPCTSTR lpFileName, BkUpdateDatLibEncodeParam& upEncodeParam)
	{
		return Save(lpFileName, upEncodeParam);
	}

	void XorDat(BYTE* buffer, int nLen, BYTE btKey)
	{
		for ( int nIndex = 0; nIndex < nLen; nIndex++)
		{
			buffer[nIndex] = buffer[nIndex]^(BYTE)(btKey);
		}
	}

	// 将XML格式的文件转化成库格式
	BOOL Save(LPCTSTR lpFileName, BkDatLibEncodeParam& encodeParam)
	{
		ATLASSERT(encodeParam.pBuffer!=NULL);
		ATLASSERT(encodeParam.nLen!=0);

		BkDatLibHeader		datHeader;
		HANDLE	hFile		= INVALID_HANDLE_VALUE;
		DWORD	nWriteNum	= 0;
		BOOL	bRes		= FALSE;
		LPSTR	lpBuffer	= (LPSTR)encodeParam.pBuffer;
		INT		nAnsiLen	= encodeParam.nLen;
		INT		nUniLen		= 0;
		INT		nIndex		= 0;
		BYTE	szKey		= 0;

		memset(&datHeader,0,sizeof(BkDatLibHeader));
		datHeader.nCryptMethod	= MED_CRYPT_XOR;
		datHeader.dwSign		= DAT_LIB_MARK;
		datHeader.nOffsetXML	= sizeof(BkDatLibHeader);
		datHeader.dwSize		= sizeof(BkDatLibHeader);
		datHeader.dwType		= encodeParam.dwType;
		datHeader.nFormatVer	= encodeParam.dwFormatType;
		datHeader.llVersion		= encodeParam.llVersion;
		datHeader.llUpdateForVer= encodeParam.llVerFor;
		datHeader.nSizeXML		= nAnsiLen;
		datHeader.szKey[0]		= encodeParam.btKey;
		datHeader.dwCheckSum	= nAnsiLen+sizeof(BkDatLibHeader);

		szKey = datHeader.szKey[0];
		XorDat((BYTE*)lpBuffer,nAnsiLen,szKey);

		hFile = ::CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE )
			goto _error_exit;

		::WriteFile(hFile, &datHeader, sizeof(BkDatLibHeader), &nWriteNum, NULL);

		if ( nWriteNum != sizeof(BkDatLibHeader) )
			goto _error_exit;

		nWriteNum = 0;
		::SetFilePointer(hFile, datHeader.nOffsetXML, NULL, FILE_BEGIN);
		::WriteFile(hFile, lpBuffer, nAnsiLen, &nWriteNum, NULL);
		if ( nWriteNum != nAnsiLen )
			goto _error_exit;

		bRes = TRUE;
_error_exit:
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;

			if ( !bRes )
				::DeleteFile(lpFileName);
		}

		return bRes;
	}

	BOOL GetLibDatHeader( LPCTSTR lpFileName, BkDatLibHeader& datHeader )
	{
		BOOL	bRes		= FALSE;
		DWORD	nIndex		= 0;
		DWORD	nFSize		= 0;
		DWORD	nReadNum	= 0;
		HANDLE	hFile		= INVALID_HANDLE_VALUE;

		hFile = ::CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			goto _error_exit;

		nFSize = GetFileSize(hFile, NULL);
		if ( nFSize < sizeof(BkDatLibHeader) )
			goto _error_exit;

		if ( !::ReadFile( hFile, &datHeader, sizeof(BkDatLibHeader), &nReadNum, NULL ) )
			goto _error_exit;
		if ( nReadNum != sizeof(BkDatLibHeader) )
			goto _error_exit;
		if ( datHeader.dwSign != DAT_LIB_MARK )
			goto _error_exit;
		if ( datHeader.dwCheckSum != nFSize )
			goto _error_exit;
		if ( datHeader.nOffsetXML > nFSize )
			goto _error_exit;
		if ( datHeader.nOffsetXML + datHeader.nSizeXML > nFSize )
			goto _error_exit;

		bRes = TRUE;
_error_exit:
		if (hFile!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		return bRes;
	}

	BOOL GetLibDatContent2(LPCTSTR lpFileName, BkDatLibContent2 & content2 )
	{
		if ( !GetLibDatContent(lpFileName, content2.content) )
			return FALSE;	
		content2.strXML = (LPSTR)content2.content.pBuffer;
		return TRUE;
	}

	BOOL GetLibDatContent( LPCTSTR lpFileName, BkDatLibContent& content )
	{
		BOOL	bRes		= FALSE;
		DWORD	nIndex		= 0;
		DWORD	nFSize		= 0;
		DWORD	nReadNum	= 0;
		HANDLE	hFile		= INVALID_HANDLE_VALUE;
		BkDatLibHeader&		datHeader = content.header;
		BYTE*	pXMLBuffer	= NULL;

		hFile = ::CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			goto _error_exit;

		nFSize = GetFileSize(hFile, NULL);
		if ( nFSize < sizeof(BkDatLibHeader) )
			goto _error_exit;

		if ( !::ReadFile( hFile, &datHeader, sizeof(BkDatLibHeader), &nReadNum, NULL ) )
			goto _error_exit;
		if ( nReadNum != sizeof(BkDatLibHeader) )
			goto _error_exit;
		if ( datHeader.dwSign != DAT_LIB_MARK )
			goto _error_exit;
		if ( datHeader.dwCheckSum != nFSize )
			goto _error_exit;
		if ( datHeader.nOffsetXML > nFSize )
			goto _error_exit;
		if ( datHeader.nOffsetXML + datHeader.nSizeXML > nFSize )
			goto _error_exit;

		pXMLBuffer = new BYTE[datHeader.nSizeXML + 1];
		::SetFilePointer( hFile, datHeader.nOffsetXML, 0, FILE_BEGIN);
		nReadNum = 0;
		if ( !ReadFile( hFile, pXMLBuffer, datHeader.nSizeXML, &nReadNum, NULL) )
			goto _error_exit;
		if (nReadNum != datHeader.nSizeXML)
			goto _error_exit;

		if ( datHeader.nCryptMethod == MED_CRYPT_XOR )
		{
			BYTE bKey = datHeader.szKey[0];
			XorDat(pXMLBuffer,datHeader.nSizeXML,bKey);
			pXMLBuffer[ datHeader.nSizeXML ] = 0;
		}
		else
			goto _error_exit;

		content.pBuffer	= pXMLBuffer;
		content.nLen	= datHeader.nSizeXML;
		bRes			= TRUE;

_error_exit:
		if (hFile!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		if ( !bRes )
		{
			if ( pXMLBuffer!=NULL)
			{
				delete[] pXMLBuffer;
				pXMLBuffer = NULL;
			}
		}

		return bRes;
	}

	BOOL LoadFile(LPCTSTR lpFileName, CStringA& strXML)
	{
		BkDatLibContent2	ct2;

		if ( GetLibDatContent2(lpFileName, ct2) )
		{
			strXML = ct2.strXML;
			return TRUE;
		}

		return FALSE;
	}
};
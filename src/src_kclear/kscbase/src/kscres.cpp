#include <windows.h>
#include <ocidl.h>
#include <olectl.h>
#include "kscres.h"
#include "unzip.h"
#include "iowin32.h"
#include "kscfilepath.h"
#include "kscconv.h"
#pragma warning(push)
#pragma warning(disable: 6011)
#pragma warning(disable: 6386)
#include "tinyxml.h"
#include <atlbase.h>
#include <atlapp.h>
#pragma warning(pop)
#include <atlmisc.h>
#include <atlstr.h>
#include <ShlObj.h>


//////////////////////////////////////////////////////////////////////////

namespace 
{
	void* ZipOpenFunc(void* opaque, const char* filename, int mode) 
	{
		return (void*)filename;
	}

	uLong ZipReadFunc(void* opaque, void* stream, void* buf, uLong size)
	{
		uLong ret = 0;
		CMemFile* pMemFile = (CMemFile*)stream;

		if (!pMemFile)
			goto clean0;

		pMemFile->ReadFile(buf, size, &ret);

clean0:
		return ret;
	}

	uLong ZipWriteFunc(void* opaque, void* stream, const void* buf, uLong size)
	{
		uLong ret = 0;
		CMemFile* pMemFile = (CMemFile*)stream;

		if (!pMemFile)
			goto clean0;

		pMemFile->WriteFile(buf, size, &ret);

clean0:
		return ret;
	}

	long ZipSeekFunc (void* opaque, void* stream, uLong offset, int origin)
	{
		uLong ret = -1;
		CMemFile* pMemFile = (CMemFile*)stream;
		DWORD dwRetCode;

		if (!pMemFile)
			goto clean0;

		dwRetCode = pMemFile->SetFilePointer(offset, NULL, origin);
		if (INVALID_SET_FILE_POINTER == dwRetCode)
			goto clean0;

		ret = 0;

clean0:
		return ret;
	}

	long ZipTellFunc(void* opaque, void* stream)
	{
		long ret=-1;
		CMemFile* pMemFile = (CMemFile*)stream;
		DWORD dwRetCode;

		if (!pMemFile)
			goto clean0;

		dwRetCode = pMemFile->SetFilePointer(0, NULL, FILE_CURRENT);
		if (INVALID_SET_FILE_POINTER == dwRetCode)
			goto clean0;

		ret = dwRetCode;

clean0:
		return ret;
	}

	int ZipCloseFunc(void* opaque, void* stream)
	{
		int ret=-1;
		CMemFile* pMemFile = (CMemFile*)stream;

		if (!pMemFile)
			goto clean0;

		if (!pMemFile->CloseHandle())
			goto clean0;

		ret = 0;

clean0:
		return ret;
	}

	int ZipErrorFunc(void* opaque, void* stream)
	{
		return 0;
	}
}

inline BOOL CreateDirectory(const WTL::CString& strDir)
{
	BOOL retval = FALSE;
	WTL::CString strTemp = strDir;
	WTL::CString strBase;
	int nFind;

	if (GetFileAttributes(strDir) == FILE_ATTRIBUTE_DIRECTORY)
	{
		retval = TRUE;
		goto clean0;
	}

	if (strTemp[strTemp.GetLength() - 1] != _T('\\'))
		strTemp += _T("\\");

	nFind = strTemp.Find(_T("\\"));
	while (nFind != -1)
	{
		strBase += strTemp.Left(nFind + 1);
		strTemp.Delete(0, nFind + 1);

		if (GetFileAttributes(strBase) == INVALID_FILE_ATTRIBUTES)
		{
			if (!CreateDirectory(strBase, NULL))
				goto clean0;
		}

		nFind = strTemp.Find(_T("\\"));
	}

	retval = TRUE;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

extern CAppModule* _ModulePtr;

//////////////////////////////////////////////////////////////////////////

KAppRes::KAppRes() : m_hTempRes(INVALID_HANDLE_VALUE)
{
	PrepareRes();
	OpenResPack();
	LoadStringRes();
	LoadImageRes();
	LoadXmlRes();
	LoadFontRes();
}

KAppRes::~KAppRes()
{
	CloseResPack();
    if (m_hTempRes != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hTempRes);
        m_hTempRes = INVALID_HANDLE_VALUE;
    }
}

KAppRes& KAppRes::Instance()
{
	static KAppRes _singleton;
	return _singleton;
}

bool KAppRes::PrepareRes()
{
    bool retval = false;
    KFilePath pathRes = KFilePath::GetFilePath(g_hInstance);
    HRSRC hResInfo = NULL;
    HGLOBAL hResDat = NULL;
    PVOID pResBuffer = NULL;
    DWORD dwResBuffer;
    wchar_t szTempPath[MAX_PATH] = { 0 };
    wchar_t szTempFilePath[MAX_PATH] = { 0 };
    //BOOL fRetCode;
    //DWORD dwWritten;

    pathRes.RemoveExtension();
    pathRes.AddExtension(L"kui");

    if (GetFileAttributesW(pathRes) != INVALID_FILE_ATTRIBUTES)
    {
        m_strResPackPath = pathRes.value();
    }
    else
    {
        hResInfo = FindResourceW(_ModulePtr->GetResourceInstance(), L"kuires.dat", L"SKIN");
        if (!hResInfo)
            goto clean0;
        
        hResDat = LoadResource(_ModulePtr->GetResourceInstance(), hResInfo);
        if (!hResDat)
            goto clean0;

        pResBuffer = LockResource(hResDat);
        if (!pResBuffer)
            goto clean0;

        dwResBuffer = SizeofResource(_ModulePtr->GetResourceInstance(), hResInfo);
		m_memZipRes.SetData(pResBuffer, dwResBuffer);

       /* GetTempPathW(MAX_PATH, szTempPath);
        GetTempFileNameW(szTempPath, L"kui", 0, szTempFilePath);
        m_hTempRes = CreateFileW(szTempFilePath, GENERIC_ALL, FILE_SHARE_READ, NULL, 
            CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
        if (INVALID_HANDLE_VALUE == m_hTempRes)
            goto clean0;

        fRetCode = WriteFile(m_hTempRes, pResBuffer, dwResBuffer, &dwWritten, NULL);
        if (!fRetCode)
            goto clean0;

        fRetCode = FlushFileBuffers(m_hTempRes);

        m_strResPackPath = szTempFilePath;*/
    }
    
    retval = true;

clean0:
    return retval;
}

bool KAppRes::GetString(const std::string& strId, std::wstring& strText)
{
	bool retval = false;
	KStringStore::const_iterator i;

	i = m_mapStringTable.find(strId);
	if (i != m_mapStringTable.end())
	{
		retval = true;
		strText = i->second;
	}
	else
	{
		strText = L"[null]";
	}

	return retval;
}

const wchar_t* KAppRes::GetString(const std::string& strId)
{
	const wchar_t* retval = NULL;
	static std::wstring strText;

	if (!GetString(strId, strText))
		goto clean0;

	retval = strText.c_str();

clean0:
	return retval;
}

HFONT KAppRes::GetFont(const std::string& strId)
{
	HFONT retval = NULL;
	KFontStore::const_iterator i;

	i = m_mapFontTable.find(strId);
	if (i != m_mapFontTable.end())
	{
		retval = i->second;
	}

	return retval;
}

COLORREF KAppRes::GetColor(const std::string& strId)
{
	COLORREF retval = 0;
	KColorStore::const_iterator i;

	i = m_mapColorTable.find(strId);
	if (i != m_mapColorTable.end())
	{
		retval = i->second;
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////

bool KAppRes::OpenResPack()
{
	bool retval = false;
	zlib_filefunc_def zip_funcs;
	std::string strPathAnsi;
	int nRetCode;

	HRSRC hResInfo = NULL;
	HGLOBAL hResDat = NULL;
	PVOID pResBuffer = NULL;
	DWORD dwResBuffer = 0;

 	fill_win32_filefunc(&zip_funcs);
 	strPathAnsi = UnicodeToAnsi(m_strResPackPath);
 	m_pResPackData = unzOpen2(strPathAnsi.c_str(), &zip_funcs);

	if (m_pResPackData)
		goto UNZRESPACKDATA;

	if (strlen((const char*)&m_memZipRes) == 0)
	{//防止.kui格式错误导致unzOpen2返回空的m_pResPackData
		hResInfo = FindResourceW(_ModulePtr->GetResourceInstance(), L"kuires.dat", L"SKIN");
		if (!hResInfo)
			goto clean0;

		hResDat = LoadResource(_ModulePtr->GetResourceInstance(), hResInfo);
		if (!hResDat)
			goto clean0;

		pResBuffer = LockResource(hResDat);
		if (!pResBuffer)
			goto clean0;

		dwResBuffer = SizeofResource(_ModulePtr->GetResourceInstance(), hResInfo);
		m_memZipRes.SetData(pResBuffer, dwResBuffer);
	}

	zip_funcs.zopen_file = ZipOpenFunc;
	zip_funcs.zread_file = ZipReadFunc;
	zip_funcs.zwrite_file = ZipWriteFunc;
	zip_funcs.ztell_file = ZipTellFunc;
	zip_funcs.zseek_file = ZipSeekFunc;
	zip_funcs.zclose_file = ZipCloseFunc;
	zip_funcs.zerror_file = ZipErrorFunc;
	zip_funcs.opaque=NULL;
	m_pResPackData = unzOpen2((const char*)&m_memZipRes, &zip_funcs);

	if (!m_pResPackData)
		goto clean0;

UNZRESPACKDATA:
	nRetCode = unzGoToFirstFile(m_pResPackData);
	while (UNZ_OK == nRetCode)
	{
		char szCurrentFile[260];
		unz_file_info fileInfo;
		uLong dwSeekPos;
		uLong dwSize;

		nRetCode = unzGetCurrentFileInfo(
			m_pResPackData, 
			&fileInfo, 
			szCurrentFile, 
			sizeof(szCurrentFile), 
			NULL, 
			0, 
			NULL, 
			0
			);
		if (nRetCode != UNZ_OK)
			goto clean0;

		dwSeekPos = unzGetOffset(m_pResPackData);
		dwSize = fileInfo.uncompressed_size;
		m_mapResOffset.insert(KResOffset::value_type(szCurrentFile, KResInfo(dwSeekPos, dwSize)));

		nRetCode = unzGoToNextFile(m_pResPackData);
	}

clean0:
	return retval;
}

void KAppRes::CloseResPack()
{
	if (m_pResPackData)
	{
		unzClose(m_pResPackData);
		m_pResPackData = NULL;
	}
}

bool KAppRes::GetRawDataFromRes(
	const std::string& strId, 
	void** ppBuffer, 
	unsigned long& dwSize
	)
{
	bool retval = false;
	KResStore::iterator store;
	KResOffset::iterator offset;
	unsigned long dwOffset;
	int nRetCode;

	if (!ppBuffer)
		goto clean0;

	offset = m_mapResOffset.find(strId);
	if (offset == m_mapResOffset.end())
		goto clean0;

	dwOffset = offset->second.first;
	dwSize = offset->second.second;

	*ppBuffer = new unsigned char[dwSize+1];
	if (!*ppBuffer)
		goto clean0;

	nRetCode = unzSetOffset(m_pResPackData, dwOffset);
	if (nRetCode != UNZ_OK)
		goto clean0;

	nRetCode = unzOpenCurrentFile(m_pResPackData);
	if (nRetCode != UNZ_OK)
		goto clean0;

	nRetCode = unzReadCurrentFile(m_pResPackData, *ppBuffer, dwSize);
	if (0 == nRetCode)
		goto clean0;

	retval = true;

clean0:
	if (!retval)
	{
		if (ppBuffer)
		{
			if (*ppBuffer)
			{
				delete[] (*ppBuffer);
				*ppBuffer = NULL;
			}
		}
	}

	return retval;
}

void KAppRes::FreeRawData(void* pBuffer)
{
	if (pBuffer)
	{
		delete[] (unsigned char*)pBuffer;
		pBuffer = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	typedef struct tagEscapePattern {
		wchar_t* pOld;
		wchar_t* pNew;
	} EscapePattern;

	EscapePattern g_szXmlEscapePattern[4] = {
		{ L"{\\n}", L"\n" },
		{ L"{\\r}", L"\r" },
		{ L"{\\t}", L"\t" }	
	};

	void update_string(std::wstring& strValue)
	{
		size_t c;

		for (int i = 0; i < 3; ++i)
		{
			for (;;)
			{
				c = strValue.find(g_szXmlEscapePattern[i].pOld);
				if (c == std::wstring::npos)
					break;

				strValue.replace(
					c, 
					wcslen(g_szXmlEscapePattern[i].pOld), 
					g_szXmlEscapePattern[i].pNew
					);
			}
		}
	}
}

bool KAppRes::LoadStringRes()
{
	bool retval = false;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	TiXmlDocument xmlDoc;
	const TiXmlElement* pXmlChild = NULL;
	const TiXmlElement* pXmlItem = NULL;

	if (!GetRawDataFromRes("strings.xml", &pBuffer, dwBuffer))
		goto clean0;

	if (!xmlDoc.LoadBuffer((char*)pBuffer, (long)dwBuffer, TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlChild = xmlDoc.FirstChildElement("strings");
	if (!pXmlChild)
		goto clean0;

	pXmlItem = pXmlChild->FirstChildElement("string");
	while (pXmlItem) 
	{
		std::string strId;
		std::string strValue;

		strId = pXmlItem->Attribute("id");
		strValue = pXmlItem->Attribute("value");

		if (strId.length() && strValue.length())
		{
			std::wstring strTemp = Utf8ToUnicode(strValue);
			update_string(strTemp);
			m_mapStringTable[strId] = strTemp;
		}

		pXmlItem = pXmlItem->NextSiblingElement("string");
	}

	retval = true;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}

namespace
{
	HFONT CreateFont(const std::wstring& strFontFace, int nFontSize, 
		BOOL fBold = 0, BOOL fUnderline = 0, BOOL fItalic = 0)
	{
		HFONT retval = NULL;
		LOGFONTW logFont;

		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &logFont);
		wcscpy_s(logFont.lfFaceName, strFontFace.c_str());
		logFont.lfWeight = fBold ? FW_BOLD : FW_NORMAL;
		logFont.lfUnderline = fUnderline ? TRUE : FALSE;
		logFont.lfItalic = fItalic ? TRUE : FALSE;
		logFont.lfQuality = ANTIALIASED_QUALITY;
		logFont.lfHeight = nFontSize;

		retval = ::CreateFontIndirect(&logFont);

		return retval;
	}
}

bool KAppRes::LoadFontRes()
{
	bool retval = false;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	TiXmlDocument xmlDoc;
	const TiXmlElement* pXmlChild = NULL;
	const TiXmlElement* pXmlItem = NULL;

	if (!GetRawDataFromRes("fonts.xml", &pBuffer, dwBuffer))
		goto clean0;

	if (!xmlDoc.LoadBuffer((char*)pBuffer, (long)dwBuffer, TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlChild = xmlDoc.FirstChildElement("fonts");
	if (!pXmlChild)
		goto clean0;

	pXmlItem = pXmlChild->FirstChildElement("font");
	while (pXmlItem) 
	{
		std::string strId;
		std::wstring strFontface;
		int nFontSize = 11;
		int nUnderline = 0;
		int nBold = 0;
		int nItalic = 0;

		strId = pXmlItem->Attribute("id");
		strFontface = AnsiToUnicode(pXmlItem->Attribute("fontface"));
		pXmlItem->Attribute("fontsize", &nFontSize);
		pXmlItem->Attribute("underline", &nUnderline);
		pXmlItem->Attribute("bold", &nBold);
		pXmlItem->Attribute("italic", &nItalic);

		if (strId.length() && strFontface.length())
		{
			m_mapFontTable[strId] = CreateFont(
				strFontface,
				nFontSize,
				nBold ? TRUE : FALSE,
				nUnderline ? TRUE : FALSE,
				nItalic ? TRUE : FALSE
				);
		}

		pXmlItem = pXmlItem->NextSiblingElement("font");
	}

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}

namespace
{
	ULONG HexStringToULong(LPCSTR lpszValue, int nSize = -1)
	{
		LPCSTR pchValue = lpszValue;
		ULONG ulValue = 0;

		while (*pchValue && nSize != 0)
		{
			ulValue <<= 4;

			if ('a' <= *pchValue && 'f' >= *pchValue)
				ulValue |= (*pchValue - 'a' + 10);
			else if ('A' <= *pchValue && 'F' >= *pchValue)
				ulValue |= (*pchValue - 'A' + 10);
			else if ('0' <= *pchValue && '9' >= *pchValue)
				ulValue |= (*pchValue - '0');
			else
				return 0;

			++ pchValue;
			-- nSize;
		}

		return ulValue;
	}

	COLORREF HexStringToColor(LPCSTR lpszValue)
	{
		return RGB(
			HexStringToULong(lpszValue, 2), 
			HexStringToULong(lpszValue + 2, 2), 
			HexStringToULong(lpszValue + 4, 2)
			);
	}
}

bool KAppRes::LoadColorRes()
{
	bool retval = false;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	TiXmlDocument xmlDoc;
	const TiXmlElement* pXmlChild = NULL;
	const TiXmlElement* pXmlItem = NULL;

	if (!GetRawDataFromRes("colors.xml", &pBuffer, dwBuffer))
		goto clean0;

	if (!xmlDoc.LoadBuffer((char*)pBuffer, (long)dwBuffer, TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlChild = xmlDoc.FirstChildElement("colors");
	if (!pXmlChild)
		goto clean0;

	pXmlItem = pXmlChild->FirstChildElement("color");
	while (pXmlItem) 
	{
		std::string strId;
		std::string strColor;

		strId = pXmlItem->Attribute("id");
		strColor = pXmlItem->Attribute("value");

		if (strId.length() && strColor.length())
		{
			m_mapColorTable[strId] = HexStringToColor(strColor.c_str());
		}

		pXmlItem = pXmlItem->NextSiblingElement("color");
	}

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}

bool KAppRes::LoadXmlRes()
{
	bool retval = false;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	TiXmlDocument xmlDoc;
	const TiXmlElement* pXmlChild = NULL;
	const TiXmlElement* pXmlItem = NULL;

	if (!GetRawDataFromRes("xmls.xml", &pBuffer, dwBuffer))
		goto clean0;

	if (!xmlDoc.LoadBuffer((char*)pBuffer, (long)dwBuffer, TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlChild = xmlDoc.FirstChildElement("xmls");
	if (!pXmlChild)
		goto clean0;

	pXmlItem = pXmlChild->FirstChildElement("xml");
	while (pXmlItem) 
	{
		std::string strId;
		std::string strPath;

		strId = pXmlItem->Attribute("id");
		strPath = pXmlItem->Attribute("path");

		if (strId.length() && strPath.length())
		{
			m_mapXmlTable[strId] = strPath;
		}

		pXmlItem = pXmlItem->NextSiblingElement("xml");
	}

    retval = true;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}
/*
bool KAppRes::LoadStyleRes()
{
    bool retval = false;
    void* pBuffer = NULL;
    unsigned long dwBuffer = 0;
    TiXmlDocument xmlDoc;
    const TiXmlElement* pXmlChild = NULL;
    const TiXmlElement* pXmlItem = NULL;
    char* pStr = NULL;
    std::string strXml;

    if (!GetRawDataFromRes("styles.xml", &pBuffer, dwBuffer))
        goto clean0;

    pStr = new char[dwBuffer + 1];
    if (!pStr)
        goto clean0;

    memcpy(pStr, pBuffer, dwBuffer);
    *(pStr + dwBuffer) = '\0';

    strXml = pStr;

    KuiStyle::LoadStyles(strXml);

    retval = true;

clean0:
    if (pStr)
    {
        delete[] pStr;
        pStr = NULL;
    }

    if (pBuffer)
    {
        FreeRawData(pBuffer);
        pBuffer = NULL;
    }

    return retval;
}
*/
bool KAppRes::LoadImageRes()
{
	bool retval = false;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	TiXmlDocument xmlDoc;
	const TiXmlElement* pXmlChild = NULL;
	const TiXmlElement* pXmlItem = NULL;

	if (!GetRawDataFromRes("images.xml", &pBuffer, dwBuffer))
		goto clean0;

	if (!xmlDoc.LoadBuffer((char*)pBuffer, (long)dwBuffer, TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlChild = xmlDoc.FirstChildElement("images");
	if (!pXmlChild)
		goto clean0;

	pXmlItem = pXmlChild->FirstChildElement("image");
	while (pXmlItem) 
	{
		std::string strId;
		std::string strPath;

		strId = pXmlItem->Attribute("id");
		strPath = pXmlItem->Attribute("path");

		if (strId.length() && strPath.length())
		{
			m_mapImageTable[strId] = strPath;
		}

		pXmlItem = pXmlItem->NextSiblingElement("image");
	}

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	IStream* GetStreamObject(void* pBuffer, unsigned long dwSize)
	{
		IStream* retval = NULL;
		HRESULT hRetCode;
		HGLOBAL hGlobal = NULL;
		void* pGlobal = NULL;

		if (!pBuffer || !dwSize)
			goto clean0;

		hGlobal = GlobalAlloc(GPTR, dwSize);
		if (!hGlobal)
			goto clean0;

		pGlobal = GlobalLock(hGlobal);

		memcpy(pGlobal, pBuffer, dwSize);

		hRetCode = CreateStreamOnHGlobal(hGlobal, TRUE, &retval);

clean0:
		return retval;
	}

	IPicture* GetPictureObject(IStream* piStream)
	{
		IPicture* retval = NULL;
		HRESULT hRetCode;

		if (!piStream)
			goto clean0;

		hRetCode = OleLoadPicture(piStream, 0, FALSE, IID_IPicture, (void**)&retval);
		if (FAILED(hRetCode) || !retval)
			goto clean0;

clean0:
		return retval;
	}

	// 用来加载JPG或者GIF
	HBITMAP LoadAnImage(void* pBuffer, unsigned long dwSize)
	{
		HBITMAP retval = NULL;
		HBITMAP hTemp = NULL;
		IStream* piStream = NULL;
		IPicture* piPicture = NULL;
		HRESULT hRetCode;

		if (!pBuffer || !dwSize)
			goto clean0;

		piStream = GetStreamObject(pBuffer, dwSize);
		if (!piStream)
			goto clean0;

		piPicture = GetPictureObject(piStream);
		if (!piPicture)
			goto clean0;

		hRetCode = piPicture->get_Handle((unsigned int*)&hTemp);
		if (FAILED(hRetCode))
			goto clean0;

		retval = (HBITMAP)CopyImage(hTemp, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);

clean0:
		if (piPicture)
		{
			piPicture->Release();
			piPicture = NULL;
		}

		if (piStream)
		{
			piStream->Release();
			piStream = NULL;
		}

		return retval;
	}
}

//////////////////////////////////////////////////////////////////////////

HBITMAP KAppRes::GetImage(const std::string& strId)
{
	HBITMAP retval = NULL;
	KImageCache::const_iterator i;
	KImageStore::const_iterator j;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;

	i = m_mapImageCache.find(strId);
	if (i != m_mapImageCache.end())
	{
		retval = i->second;
		goto clean0;
	}

	j = m_mapImageTable.find(strId);
	if (j == m_mapImageTable.end())
		goto clean0;

	if (!GetRawDataFromRes(j->second, &pBuffer, dwBuffer))
		goto clean0;

	retval = LoadAnImage(pBuffer, dwBuffer);
	if (!retval)
		goto clean0;

	m_mapImageCache[strId] = retval;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}

IUnknown* KAppRes::GetImageEx(const std::string& strId)
{
	IUnknown* retval = NULL;
	KImageCacheEx::const_iterator i;
	KImageStore::const_iterator j;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	IStream* piStream = NULL;

	i = m_mapImageExCache.find(strId);
	if (i != m_mapImageExCache.end())
	{
		retval = i->second;
		goto clean0;
	}

	j = m_mapImageTable.find(strId);
	if (j == m_mapImageTable.end())
		goto clean0;

	if (!GetRawDataFromRes(j->second, &pBuffer, dwBuffer))
		goto clean0;

	piStream = GetStreamObject(pBuffer, dwBuffer);
	if (!piStream)
		goto clean0;

	retval = GetPictureObject(piStream);
	if (!retval)
		goto clean0;

	m_mapImageExCache[strId] = retval;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	if (piStream)
	{
		piStream->Release();
		piStream = NULL;
	}

	return retval;
}

Gdiplus::Image* KAppRes::GetImagePlus(const std::string& strId)
{
	Gdiplus::Image* retval = NULL;
	KImageCachePlus::const_iterator i;
	KImageStore::const_iterator j;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	IStream* piStream = NULL;

	i = m_mapImagePlusCache.find(strId);
	if (i != m_mapImagePlusCache.end())
	{
		retval = i->second;
		goto clean0;
	}

	j = m_mapImageTable.find(strId);
	if (j == m_mapImageTable.end())
		goto clean0;

	if (!GetRawDataFromRes(j->second, &pBuffer, dwBuffer))
		goto clean0;

	piStream = GetStreamObject(pBuffer, dwBuffer);
	if (!piStream)
		goto clean0;

	retval = new Gdiplus::Image(piStream);
	if (!retval)
		goto clean0;

	m_mapImagePlusCache[strId] = retval;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	if (piStream)
	{
		piStream->Release();
		piStream = NULL;
	}

	return retval;
}

namespace
{
	HICON ReadIconFromIcoFile(void* pBuffer, unsigned long dwBuffer)
	{
		HICON retval = NULL;
		BOOL bRet = FALSE;
		WCHAR szTempDir[MAX_PATH] = { 0 };
		WCHAR szTempFile[MAX_PATH] = { 0 };
		FILE* pFile = NULL;

		//GetTempPathW(MAX_PATH, szTempDir);
		SHGetSpecialFolderPath(NULL, szTempDir, CSIDL_APPDATA, FALSE);
		PathAppend(szTempDir, L"kingsoft\\kclear\\temp");
		bRet = CreateDirectory(szTempDir);
		if (!bRet)
		{
			goto clean0;
		}
		GetTempFileNameW(szTempDir, L"ico", 0, szTempFile);

		pFile = _wfopen(szTempFile, L"wb");
		if (!pFile)
			goto clean0;

		fwrite(pBuffer, 1, dwBuffer, pFile);
		fclose(pFile);
		pFile = NULL;

		retval = (HICON)LoadImageW(NULL, szTempFile, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

clean0:
		if (pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}

		DeleteFileW(szTempFile);

		return retval;
	}
}

static HICON _GetOemIcon(const std::string& strId)
{
    HICON retval = NULL;

    if ("IDI_HAND" == strId)
    {
        retval = LoadIcon(NULL, IDI_HAND);
    }
    else if ("IDI_QUESTION" == strId)
    {
        retval = LoadIcon(NULL, IDI_QUESTION);
    }
    else if ("IDI_EXCLAMATION" == strId)
    {
        retval = LoadIcon(NULL, IDI_EXCLAMATION);
    }
    else if ("IDI_ASTERISK" == strId)
    {
        retval = LoadIcon(NULL, IDI_ASTERISK);
    }

    return retval;
}

HICON KAppRes::GetIcon(const std::string& strId)
{
	HICON retval = NULL;
	KImageCacheEx::const_iterator i;
	KImageStore::const_iterator j;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;

    retval = _GetOemIcon(strId);
    if (retval)
        goto clean0;

	i = m_mapImageExCache.find(strId);
	if (i != m_mapImageExCache.end())
	{
		retval = (HICON)i->second;
		goto clean0;
	}

	j = m_mapImageTable.find(strId);
	if (j == m_mapImageTable.end())
		goto clean0;

	if (!GetRawDataFromRes(j->second, &pBuffer, dwBuffer))
		goto clean0;

	retval = ReadIconFromIcoFile(pBuffer, dwBuffer);
	if (!retval)
		goto clean0;

	m_mapImageExCache[strId] = (IUnknown*)retval;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	return retval;
}

bool KAppRes::GetXmlData(const std::string& strId, std::string& strXml)
{
	bool retval = false;
	KXmlStore::const_iterator i;
	void* pBuffer = NULL;
	unsigned long dwBuffer = 0;
	char* pStr = NULL;

	i = m_mapXmlTable.find(strId);
	if (i == m_mapXmlTable.end())
		goto clean0;

	if (!GetRawDataFromRes(i->second, &pBuffer, dwBuffer))
		goto clean0;

	pStr = new char[dwBuffer + 1];
	if (!pStr)
		goto clean0;

	memcpy(pStr, pBuffer, dwBuffer);
	*(pStr + dwBuffer) = '\0';

	strXml = pStr;

	retval = true;

clean0:
	if (pBuffer)
	{
		FreeRawData(pBuffer);
	}

	if (pStr)
	{
		delete[] pStr;
		pStr = NULL;
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////

HICON KAppRes::GetOemIcon(const std::string& strId)
{
	HICON retval = NULL;
	KImageCacheEx::const_iterator i;

	i = m_mapImageExCache.find(strId);
	if (i != m_mapImageExCache.end())
	{
		retval = (HICON)i->second;
		goto clean0;
	}

	retval = (HICON)::LoadIconA(NULL, strId.c_str());
	if (!retval)
		goto clean0;

	m_mapImageExCache[strId] = (IUnknown*)retval;

clean0:
	return retval;
}

//////////////////////////////////////////////////////////////////////////

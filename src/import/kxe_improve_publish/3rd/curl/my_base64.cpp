//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   Base64.cpp
//  Version     :   1.0
//  Creater     :   Dengzhenbo
//  Date        :   2005-2-28 16:35:10
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

// Base64.cpp: implementation of the Base64 class.
//
//////////////////////////////////////////////////////////////////////
//#include "../ValidationServer/stdafx.h"
#include "my_Base64.h"

#include <stdlib.h>
#include <assert.h>

#ifndef ASSERT
#define ASSERT assert
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/* Dos/Windows */
#define defCRLF										"\r\n"
#define defDoubleCRLF								"\r\n\r\n"

/* Linux/Unix */
#define defCRLF2									"\n"
#define defDoubleCRLF2								"\n\n"



#define BASE64_MAXLINE								76
#define BASE64_EOL									"\r\n"



unsigned char g_Base64IndexCharTable[256] = 
{
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x3E,0x41,0x41,0x41,0x3F,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x41,0x41,0x41,0x40,0x41,0x41,
    0x41,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
    0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x41,0x41,0x41,0x41,0x41,
    0x41,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
    0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41
};

/*

  函数名:	Base64Encode
  功能：	完成Base64的编码功能
  参数:		pcszIn		---		需要编码的字符串
			nInSize		---		需要编码字符串长度
			pszOut		---		接收编码之后的字符串指针，
								不能为空，大小可以通过调
								用EstimateEncodeBufferSize
								函数获得
			nOutSizeMax ---		参数pszOut的Buffer的大小
			pnReturnSize---		返回实际的编码之后文本的大小
			nNeedCRLF	---		是否需要回车换行，正常情况下面为TRUE
			

*/

int Base64Encode(const char *pcszIn, int nInSize, char *pszOut, int nOutSizeMax, int *pnReturnSize, int nNeedCRLF)
{
	int nResult = false;
	char szBase64Tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int nInPos  = 0;
	int nOutPos = 0;
	int nLineLen = 0;
	int i = 0;

	if (pcszIn == NULL || pszOut == NULL)
		goto Exit0;

	if (nOutSizeMax < (nInSize + 2) / 3 * 4)
		goto Exit0;

	/* Get three characters at a time from the input buffer and encode them */
	for (i = 0; i < nInSize / 3; i++) 
	{
		/* Get the next 2 characters */
		int c1 = pcszIn[nInPos++] & 0xFF;
		int c2 = pcszIn[nInPos++] & 0xFF;
		int c3 = pcszIn[nInPos++] & 0xFF;

		/* Encode into the 4 6 bit characters */
		pszOut[nOutPos++] = szBase64Tab[(c1 & 0xFC) >> 2];
		pszOut[nOutPos++] = szBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
		pszOut[nOutPos++] = szBase64Tab[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
		pszOut[nOutPos++] = szBase64Tab[c3 & 0x3F];
		nLineLen += 4;

		if (nNeedCRLF)
		{
			/* Handle the case where we have gone over the max line boundary */
			if (nLineLen >= BASE64_MAXLINE - 3)
			{
				char *cp = BASE64_EOL;

				pszOut[nOutPos++] = *cp++;

				if (*cp)
				{
					pszOut[nOutPos++] = *cp;
				}

				nLineLen = 0;
			}
		}
	}

	/* Encode the remaining one or two characters in the input buffer */
	switch (nInSize % 3) 
	{
	case 0:
		{
			if (nNeedCRLF)
			{
				char *cp = BASE64_EOL;
				pszOut[nOutPos++] = *cp++;

				if (*cp)
				{
					pszOut[nOutPos++] = *cp;
				}
			}

			break;
		}
	case 1:
		{
			int c1 = pcszIn[nInPos] & 0xFF;

			pszOut[nOutPos++] = szBase64Tab[(c1 & 0xFC) >> 2];
			pszOut[nOutPos++] = szBase64Tab[((c1 & 0x03) << 4)];
			pszOut[nOutPos++] = '=';
			pszOut[nOutPos++] = '=';

			if (nNeedCRLF)
			{
				char *cp = BASE64_EOL;
				pszOut[nOutPos++] = *cp++;

				if (*cp) 
				{
					pszOut[nOutPos++] = *cp;
				}
			}

			break;
		}
	case 2:
		{
			int c1 = pcszIn[nInPos++] & 0xFF;
			int c2 = pcszIn[nInPos] & 0xFF;

			pszOut[nOutPos++] = szBase64Tab[(c1 & 0xFC) >> 2];
			pszOut[nOutPos++] = szBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
			pszOut[nOutPos++] = szBase64Tab[((c2 & 0x0F) << 2)];
			pszOut[nOutPos++] = '=';

			if (nNeedCRLF)
			{
				char *cp = BASE64_EOL;
				pszOut[nOutPos++] = *cp++;

				if (*cp)
				{
					pszOut[nOutPos++] = *cp;
				}
			}

			break;
		}

	default: 
		{
			//assert(false);
			break;
		}
	}

	*pnReturnSize = nOutPos;

	ASSERT(nOutPos < nOutSizeMax);

	if (nOutPos < nOutSizeMax)
		*(pszOut + nOutPos) = '\0';

	nResult = true;

Exit0:

	return nResult;
}

/*

  函数名:	Base64Decode
  功能：	完成Base64的解码功能
  参数:		pcszIn		---		需要解码的字符串
			nInSize		---		需要解码字符串长度
			pszOut		---		接收解码之后的字符串指针，
								不能为空，大小为传入大小的+1即可
			nOutSizeMax ---		参数pszOut的Buffer的大小，一般为(nInSize + 1)大小
			pnReturnSize---		返回解码之后的大小

*/

int Base64Decode(const char *pcszIn, int nInSize, char *pszOut, int nOutSizeMax, int *pnReturnSize)
{
	int nResult = false;
	int i = 0;
	int k = 0;
	int n = 0;
	int nCount = 0;
	char szBuf[4] = {0};

	if (pcszIn == NULL || pszOut == NULL)
		goto Exit0;

	if (nOutSizeMax < (nInSize + 3) / 4 * 3)
		goto Exit0;

	while (i < nInSize)
	{
		if (
			*(pcszIn + i) == '\r' || 
			*(pcszIn + i) == '\n' || 
			*(pcszIn + i) == ' '
			)
		{
			i++;
			continue;
		}

		szBuf[k] = *(pcszIn + i);

		k++;
		i++;

		/* already read 4 bytes */
		if (k == 4)
		{
			for (n = 0; n < 4; n++)
				szBuf[n] = g_Base64IndexCharTable[szBuf[n]];

			/* Ok,now 4 bytes -> 3 bytes! */

			/* byte 1 */
			if (nCount < nOutSizeMax)
			{
				*(pszOut + nCount) = ((szBuf[0] << 2) & 0xFC) | ((szBuf[1] >> 4) & 0x03);
				nCount++;
			}
			else
				goto Exit0;

			/* maybe meet last one or two '=' */
			if (szBuf[2] != '@')
			{
				/* byte 2 */
				if (nCount < nOutSizeMax)
				{
					*(pszOut + nCount) = ((szBuf[1] << 4) & 0xF0) | ((szBuf[2] >> 2) & 0x0F);
					nCount++;
				}
				else
					goto Exit0;

				if (szBuf[3] != '@')
				{
					/* 3rd byte */
					if (nCount < nOutSizeMax)
					{
						*(pszOut + nCount) = ((szBuf[2] << 6) & 0xC0) | (szBuf[3] & 0x3F);
						nCount++;
					}
					else
						goto Exit0;
				}
				else
				{
					/* assert(false); */
				}

			}
			else
			{
				/* assert(false); */
			}

			k = 0;
		}

	}

	/* last 4 bytes should have been handled */
	/* assert(k == 0); */

	*pnReturnSize = nCount;

	if (nCount < nOutSizeMax)
		*(pszOut + nCount) = '\0';

	nResult = true;

Exit0:

	return nResult;
}

/*

  函数名:	EstimateEncodeBufferSize
  功能：	估计Base64编码之后需要Buffer间大小
  参数:		nInSize		 ---	需要编码的Buffer大小
			pnOutSizeMax ---	估计编码需要的Buffer大小

*/

int EstimateEncodeBufferSize(int nInSize, int *pnOutSizeMax)
{
	int nResult = false;
	const int nAdditionalSize = 1024 * 5;
	int nTemp1 = 0;
	int nTemp2 = 0;

	if (pnOutSizeMax == NULL)
		goto Exit0;

	*pnOutSizeMax = 0;
	
	nTemp1 = (nInSize + 2) / 3 * 4;
	nTemp2 = (nTemp1 + BASE64_MAXLINE - 1) / BASE64_MAXLINE * sizeof(BASE64_EOL);

	*pnOutSizeMax = nTemp1 + nTemp2 + nAdditionalSize;
	
	nResult = true;

Exit0:

	return nResult;
}

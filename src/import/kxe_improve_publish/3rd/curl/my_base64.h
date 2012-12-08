//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   Base64.h
//  Version     :   1.0
//  Creater     :   Dengzhenbo
//  Date        :   2005-2-28 16:39:31
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////

// Base64.h: interface for the Base64 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASE64_H__06A66B11_A2E2_497C_80C9_A6E783D83985__INCLUDED_)
#define AFX_BASE64_H__06A66B11_A2E2_497C_80C9_A6E783D83985__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//Base64编码
int Base64Encode(const char *pcszIn, int nInSize, char *pszOut, int nOutSizeMax, int *pnReturnSize, int nNeedCRLF);
//Base64解码
int Base64Decode(const char *pcszIn, int nInSize, char *pszOut, int nOutSizeMax, int *pnReturnSize);

//估计Encode的Buffer大小.
int EstimateEncodeBufferSize(int nInSize, int *pnOutSizeMax);

#endif // !defined(AFX_BASE64_H__06A66B11_A2E2_497C_80C9_A6E783D83985__INCLUDED_)

/**
* @file    avploader.h
* @brief   ...
* @author  bbcallen
* @date    2010-04-09 14:28
*/

#ifndef AVPLOADER_H
#define AVPLOADER_H

#include "skylark2\skylarkbase.h"
#include "skylark2\bkeng.h"
#include "skylark2\bkatrun.h"
#include "skylark2\bkbak.h"
#include "skylark2\bkcache.h"

/**
* @brief    加载引擎模块
* @param    lpszEngineDir   引擎模块所在的目录,如果为NULL,会取当前模块所在目录
*/
EXTERN_C HRESULT WINAPI BKEngLoad(LPCWSTR lpszEngineDir = NULL);

#endif//AVPLOADER_H
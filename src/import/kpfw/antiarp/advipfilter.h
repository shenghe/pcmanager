////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : advipfilter.h
//      Version   : 1.0
//      Comment   : 定义网镖高级包过滤拦截信息
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "unknwn.h"

// 扫荡波攻击
_declspec(selectany) extern const GUID CLSID_MS08_067 = { 0xf2d18cf7, 0x9caf, 0x4675, 0x92, 0x67, 0xcb, 0x7e, 0x14, 0x1f, 0x6d, 0x2c};

// 上兴远程控制接收
_declspec(selectany) extern const GUID CLSID_SHANGXIN_RECV = { 0x73b8ddb8, 0x52e7, 0x493b, 0x85, 0x60, 0xc3, 0xa9, 0xa, 0x9c, 0xee, 0x9e};

// 上兴远程控制发送
_declspec(selectany) extern const GUID CLSID_SHANGXIN_SEND = { 0xfa422e0f, 0xa26e, 0x492b, 0x9e, 0xa6, 0x61, 0x9b, 0x8c, 0xe1, 0x64, 0x59};
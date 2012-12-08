#pragma once
// 用于判断程序是否运行在虚拟机上

#ifndef __DETECTVM_H__
#define __DETECTVM_H__

// 函数功能：判断程序是否运行在Virtual PC上
// 返回值：true--是，false--否
bool IsInsideVPC();

// 函数功能：判断程序是否运行在VMWare上
// 返回值：true--是，false--否
bool IsInsideVMWare();

#endif
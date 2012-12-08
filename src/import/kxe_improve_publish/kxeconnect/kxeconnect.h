//////////////////////////////////////////////////////////////////////
///		@file		kxeconnect.h
///		@author		luopeng
///		@date		2008-11-12 9:07:58
///
///		@brief		网络连接相关封装
//////////////////////////////////////////////////////////////////////

#pragma once
#include <windows.h>

/**
* @defgroup kxeconnect_interface_group KXEngine Architecture Network Interface
* @{
*/

/// 用于表示一个连接的数据结构
typedef struct tagMIB_TCPEXROW{
	DWORD dwState;      ///< state of the connection,参见MIB_TCP_STATE_*的宏定义
	DWORD dwLocalAddr;  ///< address on local computer
	DWORD dwLocalPort;  ///< port number on local computer
	DWORD dwRemoteAddr; ///< address on remote computer
	DWORD dwRemotePort; ///< port number on remote computer
	DWORD dwProcessId;
} MIB_TCPEXROW, *PMIB_TCPEXROW;

/// 在获取总的连接时所用的表达结构
typedef struct tagMIB_TCPEXTABLE{
	DWORD dwNumEntries;
	MIB_TCPEXROW table[1];
} MIB_TCPEXTABLE, *PMIB_TCPEXTABLE;

/// 该连接已经关闭
#define MIB_TCP_STATE_CLOSED       1

/// 该连接处于监听状态
#define MIB_TCP_STATE_LISTEN       2

/// 该连接已经SYN_SENT
#define MIB_TCP_STATE_SYN_SENT     3

/// 该连接已经SYN_RCVD
#define MIB_TCP_STATE_SYN_RCVD     4

/// 该连接已经建立
#define MIB_TCP_STATE_ESTAB        5

/// 该连接已经FIN_WAIT1
#define MIB_TCP_STATE_FIN_WAIT1    6

/// 该连接已经FIN_WAIT2
#define MIB_TCP_STATE_FIN_WAIT2    7

/// 该连接已经CLOSE_WAIT
#define MIB_TCP_STATE_CLOSE_WAIT   8

/// 该连接CLOSEING
#define MIB_TCP_STATE_CLOSING      9

/// 该连接LAST_ACK
#define MIB_TCP_STATE_LAST_ACK     10

/// 该连接已经TIME_WAIT
#define MIB_TCP_STATE_TIME_WAIT    11

/// 该连接已经DELETE_TCB
#define MIB_TCP_STATE_DELETE_TCB   12
/**
 * @brief 获取到当前连接的所有进程相关的信息
 * @param[out] ppTcpTable 获取到的信息,由函数内部分配内存
 * @return 0 成功, 其他为失败错误码
 */
int KxEGetExtendTcpTable(MIB_TCPEXTABLE** ppTcpTable);

/**
 * @brief 释放获取到的table列表
 * @param[in] pTcpTable 通过KxEGetExtendTcpTable获取的列表
 * @return 0 成功, 其他为失败错误码
 */
int KxEFreeExtendTcpTable(MIB_TCPEXTABLE* pTcpTable);

#include "kxeconnect.inl"

/**
 * @}
 */

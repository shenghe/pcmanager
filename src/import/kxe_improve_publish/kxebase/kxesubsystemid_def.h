//////////////////////////////////////////////////////////////////////
///		@file		kxesubsystemid_def.h
///		@author		luopeng
///		@date		2008-9-10 09:07:58
///
///		@brief		各个子系统ID定义
//////////////////////////////////////////////////////////////////////

#pragma once

/// 各个子系统定义枚举量,此值最大为0xffff
typedef enum _KXE_SUB_SYSTEM_ID
{
	enum_SubSystem_Base = 0,  ///< 基础子系统
	enum_SubSystem_Scan,      ///< 扫描子系统
	enum_SubSystem_FileMon,   ///< 文件监控子系统
	enum_SubSystem_Popo,      ///< 泡泡子系统
	enum_SubSystem_White,     ///< 白名单子系统
	enum_SubSystem_Update,    ///< 升级子系统
    enum_SubSystem_Defend     ///< 主动防御子系统
} KXE_SUB_SYSTEM_ID;

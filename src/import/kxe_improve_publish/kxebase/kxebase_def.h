//////////////////////////////////////////////////////////////////////
///		@file		kxebase_def.h
///		@author		luopeng
///		@date		2009-6-8 09:07:58
///	
///		@brief		统一使用的定义
//////////////////////////////////////////////////////////////////////
#pragma once

//金山软件注册表大路径
#define		KXE_KINGSOFT_PATH		L"SOFTWARE\\Kingsoft\\"

#ifndef KXENGINE_BETA
	/// 大引擎配置起始路径
	#define	KXE_ENGINE_CONFIG_PATH	L"SOFTWARE\\Kingsoft\\KISCommon\\KXEngine\\"
#else
	/// 大引擎配置起始路径
	#define	KXE_ENGINE_CONFIG_PATH	L"SOFTWARE\\Kingsoft\\Beta_Antivirus\\KXEngine\\"
#endif

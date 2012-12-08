/********************************************************************
	created:	2010/06/09
	created:	9:6:2010   17:05
	filename: 	kscconf.h
	author:		Jiang Fengbing
	
	purpose:	读写配置文件

	使用方法:
		KConfigure(szSettingPath).get("autorun", nAutorun).get("type", nType);
*********************************************************************/
#ifndef KSCCONF_INC_
#define KSCCONF_INC_

//////////////////////////////////////////////////////////////////////////

#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////

class KConfigure
{
public:
	KConfigure(const std::wstring& strConf);
	~KConfigure();

	KConfigure& Instance();
	KConfigure& Get(const std::string& strName, std::wstring& strValue);
	KConfigure& Get(const std::string& strName, int& nValue);
	KConfigure& Set(const std::string& strName, const std::wstring& strValue);
	KConfigure& Set(const std::string& strName, int nValue);

private:
	bool Load();
	bool Save();

	typedef std::map<std::string, std::wstring> StringStore;
	typedef std::map<std::string, int> IntegerStore;

	StringStore m_vStringStore;
	IntegerStore m_vIntegerStore;
	std::wstring m_strConfPath;
};

//////////////////////////////////////////////////////////////////////////

#endif	// KSCCONF_INC_

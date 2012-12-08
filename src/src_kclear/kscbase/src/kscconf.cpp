#include "kscconf.h"
#include "tinyxml.h"
#include "kscconv.h"

//////////////////////////////////////////////////////////////////////////


KConfigure::KConfigure(const std::wstring& strConf) : m_strConfPath(strConf)
{
	Load();
}

KConfigure::~KConfigure()
{
	Save();
}

KConfigure& KConfigure::Instance()
{
	return *this;
}

bool KConfigure::Load()
{
	bool retval = false;
	TiXmlDocument xmlDoc;
	const TiXmlElement *pXmlSetting = NULL;
	const TiXmlElement *pXmlStrings = NULL;
	const TiXmlElement *pXmlIntegers = NULL;
	const TiXmlElement *pXmlChild = NULL;

	if (!xmlDoc.LoadFile(UnicodeToAnsi(m_strConfPath).c_str(), TIXML_ENCODING_UTF8))
		goto clean0;

	pXmlSetting = xmlDoc.FirstChildElement("setting");
	if (!pXmlSetting)
		goto clean0;

	pXmlStrings = pXmlSetting->FirstChildElement("strings");
	if (pXmlStrings)
	{
		pXmlChild = pXmlStrings->FirstChildElement("entry");
		while (pXmlChild)
		{
			const char* szName = NULL;
			const char* szValue = NULL;

			szName = pXmlChild->Attribute("name");
			szValue = pXmlChild->Attribute("value");

			if (szName && szValue)
			{
				m_vStringStore[szName] = Utf8ToUnicode(szValue);
			}

			pXmlChild = pXmlChild->NextSiblingElement("entry");
		}
	}

	pXmlIntegers = pXmlSetting->FirstChildElement("integers");
	if (pXmlIntegers)
	{
		pXmlChild = pXmlIntegers->FirstChildElement("entry");
		while (pXmlChild)
		{
			const char* szName = NULL;
			int nValue;

			szName = pXmlChild->Attribute("name");
			pXmlChild->Attribute("value", &nValue);

			if (szName)
			{
				m_vIntegerStore[szName] = nValue;
			}

			pXmlChild = pXmlChild->NextSiblingElement("entry");
		}
	}

	retval = true;

clean0:
	return retval;
}

bool KConfigure::Save()
{
	bool retval = false;
	TiXmlDocument xmlDoc;  
	TiXmlDeclaration *pXmlDecl = new TiXmlDeclaration("1.0", "utf-8", "yes"); 
	TiXmlElement *pXmlSetting = new TiXmlElement("setting");
	TiXmlElement *pXmlStrings = new TiXmlElement("strings");
	TiXmlElement *pXmlIntegers = new TiXmlElement("integers");
	TiXmlElement *pXmlChild = NULL;
	StringStore::const_iterator i;
	IntegerStore::const_iterator j;

	xmlDoc.LinkEndChild(pXmlDecl);

	for (i = m_vStringStore.begin(); i != m_vStringStore.end(); ++i)
	{
		std::string strName;
		std::wstring strValue;

		strName = i->first;
		strValue = i->second;

		pXmlChild = new TiXmlElement("entry");
		if (!pXmlChild)
			goto clean0;

		pXmlChild->SetAttribute("name", strName.c_str());
		pXmlChild->SetAttribute("value", UnicodeToUtf8(strValue).c_str());
		pXmlStrings->LinkEndChild(pXmlChild);
	}
	pXmlSetting->LinkEndChild(pXmlStrings);

	for (j = m_vIntegerStore.begin(); j != m_vIntegerStore.end(); ++j)
	{
		std::string strName;
		int nValue;

		strName = j->first;
		nValue = j->second;

		pXmlChild = new TiXmlElement("entry");
		if (!pXmlChild)
			goto clean0;

		pXmlChild->SetAttribute("name", strName.c_str());
		pXmlChild->SetAttribute("value", nValue);
		pXmlIntegers->LinkEndChild(pXmlChild);
	}
	pXmlSetting->LinkEndChild(pXmlIntegers);

	xmlDoc.LinkEndChild(pXmlSetting);

	retval = xmlDoc.SaveFile(UnicodeToAnsi(m_strConfPath).c_str());

clean0:
	return retval;
}

KConfigure& KConfigure::Get(const std::string& strName, std::wstring& strValue)
{
	StringStore::const_iterator i;

	i = m_vStringStore.find(strName);
	if (i != m_vStringStore.end())
		strValue = i->second;

	return *this;
}

KConfigure& KConfigure::Get(const std::string& strName, int& nValue)
{
	IntegerStore::const_iterator i;

	i = m_vIntegerStore.find(strName);
	if (i != m_vIntegerStore.end())
		nValue = i->second;

	return *this;
}

KConfigure& KConfigure::Set(const std::string& strName, const std::wstring& strValue)
{
	m_vStringStore[strName] = strValue;

	return *this;
}

KConfigure& KConfigure::Set(const std::string& strName, int nValue)
{
	m_vIntegerStore[strName] = nValue;

	return *this;
}

//////////////////////////////////////////////////////////////////////////

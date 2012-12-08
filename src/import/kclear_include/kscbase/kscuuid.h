/********************************************************************
	created:	2010/06/09
	created:	9:6:2010   23:09
	filename: 	get_guid.h
	author:		Jiang Fengbing
	
	purpose:	获得用户GUID
*********************************************************************/

#ifndef GET_GUID_INC_
#define GET_GUID_INC_

//////////////////////////////////////////////////////////////////////////


#include <string>

//////////////////////////////////////////////////////////////////////////

class KscUuid
{
private:
	KscUuid();
	~KscUuid();

public:
	static KscUuid& Instance()
	{
		static KscUuid _singleton;
		return _singleton;
	}

	bool GetUUID(std::string& strUUID);
	bool GetUUID(std::wstring& strUUID);

private:
	bool WriteToReg(const std::string& strGuid);
	bool ReadFromReg(const char szPath[], std::string& strGuid);
	bool ReadFromReg(std::string& strGuid);

	std::string m_strUUID;
};

//////////////////////////////////////////////////////////////////////////

#endif	// GET_GUID_INC_

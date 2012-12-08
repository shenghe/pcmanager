//////////////////////////////////////////////////////////////////////
///		@file		ikxeuncompressfile.h
///		@author		luopeng
///		@date		2009-6-25 09:07:58
///
///		@brief		½âÑ¹½Ó¿Ú
//////////////////////////////////////////////////////////////////////

#include <Unknwn.h>
MIDL_INTERFACE("7DC28E90-BDA5-440d-BBBA-75940CDB4C49")
IKxEUncompressFile : public IUnknown
{
public:
	virtual int __stdcall UncompressFile(
		const wchar_t* pwszSourceFileName,
		const wchar_t* pwszTargetDir
		) = 0;
};
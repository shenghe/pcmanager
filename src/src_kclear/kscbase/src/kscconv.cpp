#include "kscconv.h"

//////////////////////////////////////////////////////////////////////////

std::wstring AnsiToUnicode(const std::string& strAnsi) 
{
	std::wstring retval;

	if (strAnsi.size())
		retval = KANSI_TO_UTF16(strAnsi.c_str());

	return retval;
}

std::string UnicodeToAnsi(const std::wstring& strUnicode) 
{
	std::string retval;

	if (strUnicode.size())
		retval = KUTF16_To_ANSI(strUnicode.c_str());

	return retval;
}

std::string UnicodeToUtf8(const std::wstring& strUnicode) 
{
	std::string retval;

	if (strUnicode.size())
		retval = KUTF16_To_UTF8(strUnicode.c_str());

	return retval;
}

std::wstring Utf8ToUnicode(const std::string& strUtf8) 
{
	std::wstring retval;

	if (strUtf8.size())
		retval = KUTF8_To_UTF16(strUtf8.c_str());

	return retval;
}

std::string Utf8ToAnsi(const std::string& strUtf8) 
{
	std::string retval;
	std::wstring strTemp;

	strTemp = Utf8ToUnicode(strUtf8);
	retval = UnicodeToAnsi(strTemp);

	return retval;
}

std::string AnsiToUtf8(const std::string& strAnsi) 
{
	std::string retval;
	std::wstring strTemp;

	strTemp = AnsiToUnicode(strAnsi);
	retval = UnicodeToUtf8(strTemp);

	return retval;
}

//////////////////////////////////////////////////////////////////////////

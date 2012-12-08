#pragma once


#include <string>
#include <map>
#include <vector>

typedef std::basic_string<TCHAR> tstring;
typedef std::map<tstring, tstring> attributes;
#ifndef LPCTSTR 
typedef const TCHAR *LPCTSTR;
#endif 

struct TextPart
{
	TextPart()
	{ }
	TextPart(LPCTSTR sztag, LPCTSTR szval)
	{
		if(sztag) tag = sztag;
		if(szval) val = szval;
	}
	BOOL isText() const
	{
		return tag.empty();
	}
	BOOL isLink() const
	{
		return !tag.empty() && tag==_T("a");
	}
	BOOL isBold() const 
	{
		return !tag.empty() && tag==_T("b");
	}
	tstring tag, val;
	attributes attrs;
};
typedef std::vector<TextPart> TextParts;

class CHyperTextParser
{
public:
	void Parse( LPCTSTR sz, TextParts &parts );	
	void _ParseAttribute( LPCTSTR pb, LPCTSTR pe, TextPart &tt );
};
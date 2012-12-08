#include "StdAfx.h"
#include "HyperTextParser.h"

void CHyperTextParser::Parse( LPCTSTR sz, TextParts &parts )
{
	LPCTSTR p = sz;
	while(true)
	{
		const TCHAR tag_begin = _T('<');
		const TCHAR tag_end = _T('>');

		LPCTSTR pn = _tcschr(p, tag_begin );
		if( pn )
		{
			if( pn>p )
			{
				tstring s;
				s.assign( p, pn-p );
				parts.push_back( TextPart(NULL, s.c_str()) );
			}
			
			TextPart tt;
			LPCTSTR ptagEnd=NULL, pne=NULL, pe=NULL, pee=NULL;
			++pn;
			ptagEnd = pn;
			while( *ptagEnd && *ptagEnd!=' ' && *ptagEnd!='>' )
				++ ptagEnd;
			tt.tag.assign( pn, ptagEnd-pn );
			pne = _tcschr(ptagEnd, tag_end);
			++pne;

			_ParseAttribute(ptagEnd, pne, tt);

			pe = _tcschr( pne, tag_begin );
			pee = _tcschr( pe, tag_end );

			tt.val.assign(pne, pe-pne);
			//_tcprintf(_T("%s - %s \n"), tt.tag.c_str(), tt.val.c_str());
			parts.push_back( tt );

			p = ++pee;
		}
		else
		{
			if(*p)
			{
				//_tcprintf(_T("  - %s \n"), p);
				parts.push_back( TextPart(NULL, p) );
			}
			break;
		}
	}
}

void CHyperTextParser::_ParseAttribute( LPCTSTR pb, LPCTSTR pe, TextPart &tt )
{
	LPCTSTR pa = pb;
	while(pa<pe)
	{
		while( *pa==_T(' ') )
			++pa;

		LPCTSTR pabegin = pa;
		while( isalpha(*pa) )
		{
			++pa;
		}
		if(pa>pabegin && *pa==_T('='))
		{
			tstring attr, attrv;
			attr.assign(pabegin, pa-pabegin);

			++pa;
			LPCTSTR pvbegin=pa;
			while(*pa && *pa!=_T(' ') && *pa!=_T('>'))
			{
				++pa;
			}
			attrv.assign(pvbegin, pa-pvbegin);
			tt.attrs[attr] = attrv;
			//_tcprintf(_T("attr: %s - %s \n"), attr.c_str(), attrv.c_str());
		}
		while( *pa==_T(' ') )
			++pa;
		++pa;
	}
}
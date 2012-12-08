//////////////////////////////////////////////////////////////////////
///	 @file:   	ci_char_traits.h
///	 @author:	luopeng
///  @date:   	2004/11/21
///	
///	 @brief: 	不区分大小写的char_traits
//////////////////////////////////////////////////////////////////////
#ifndef CI_CHAR_TRAITS
#define CI_CHAR_TRAITS

#include <iosfwd>
#include <ostream>

struct ci_char_traits : public std::char_traits<char> 
              // just inherit all the other functions
              //  that we don't need to replace
{
	static bool eq(char c1, char c2)
    {
		return toupper(c1) == toupper(c2);
	}

	static bool lt(char c1, char c2)
    { 
		return toupper(c1) <  toupper(c2); 
	}

	static int compare(const char* s1, const char* s2, size_t n)
    {
		return _memicmp(s1, s2, n); 
	}
	
	static const char* find(const char* s, int n, char a)
	{
		while (n-- > 0 && toupper(*s) != toupper(a))
		{
			++s;
		}
		return n >= 0 ? s : 0;
	}
};

struct ci_wchar_traits : public std::char_traits<wchar_t> 
              // just inherit all the other functions
              //  that we don't need to replace
{
	static bool eq(wchar_t c1, wchar_t c2)
    {
		return towupper(c1) == towupper(c2);
	}

	static bool lt(wchar_t c1, wchar_t c2)
    { 
		return towupper(c1) <  towupper(c2); 
	}

	static int compare(const wchar_t* s1, const wchar_t* s2, size_t n)
    {
		int nRet = 0;
		for (size_t i = 0; i < n; ++i)
		{
			nRet = towupper(s1[i]) - towupper(s2[i]);
			if (nRet != 0)
			{
				break;
			}
		}
		return nRet;
	}
	
	static const wchar_t* find(const wchar_t* s, int n, wchar_t a)
	{
		while (n-- > 0 && towupper(*s) != towupper(a))
		{
			++s;
		}
		return n >= 0 ? s : 0;
	}
};

template<class _E, class _Tr, class _A> inline
std::basic_ostream<_E, _Tr>& __cdecl operator<<(std::basic_ostream<_E, _Tr>& _O,
												const std::basic_string<_E, ci_char_traits, _A>& _X)
{
	_O << _X.c_str();
	return (_O);
}

#endif //CI_CHAR_TRAITS

//////////////////////////////////////////////////////////////////////
///		@file		kxe_stl.h
///		@author		luopeng
///		@date		2005-7-26 17:04:42
///
///		@brief		kxe中针对stl的封装,来源于以前的KSCE
//////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
#include <list>
#include <string>
#include <queue>
#include <map>
#include <sstream>
#include <set>
#include <deque>
#include <fstream>
#include "ci_char_traits.h"

namespace kxe
{	
	typedef std::basic_string<char, std::char_traits<char>, std::allocator<char> > string;	
	typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > wstring;	
	typedef std::basic_ostringstream<char, std::char_traits<char>, std::allocator<char> > ostringstream;
	typedef std::basic_ostringstream<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > wostringstream;
	typedef std::basic_istringstream<char, std::char_traits<char>, std::allocator<char> > istringstream;
	typedef std::basic_istringstream<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > wistringstream;
	typedef std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > stringstream;
	typedef std::basic_stringstream<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > wstringstream;
	typedef std::basic_ofstream<char, std::char_traits<char> > ofstream;
	typedef std::basic_ifstream<char, std::char_traits<char> > ifstream;
	typedef std::basic_fstream<char, std::char_traits<char> > fstream;
	typedef std::basic_ofstream<wchar_t, std::char_traits<wchar_t> > wofstream;
	typedef std::basic_ifstream<wchar_t, std::char_traits<wchar_t> > wifstream;
	typedef std::basic_fstream<wchar_t, std::char_traits<wchar_t> > wfstream;

	typedef std::basic_string<char, ci_char_traits, std::allocator<char> > ci_string;
	typedef std::basic_string<wchar_t, ci_wchar_traits, std::allocator<wchar_t> > ci_wstring;
#ifdef UNICODE
	typedef ci_wstring ci_tstring;
	typedef wstring tstring;
	typedef wostringstream tostringstream;
	typedef wistringstream tistringstream;
	typedef wstringstream tstringstream;
	typedef wofstream tofstream;
	typedef wifstream tifstream;
	typedef wfstream tfstream;
	/*typedef wcout tcout;
	typedef wcin tcin;
	typedef wclog tclog;
	typedef wcerr tcerr;*/
#else
	typedef ci_string ci_tstring;
	typedef string tstring;
	typedef ostringstream tostringstream;
	typedef istringstream tistringstream;
	typedef stringstream tstringstream;
	typedef ofstream tofstream;
	typedef ifstream tifstream;
	typedef fstream tfstream;
	/*typedef cout tcout;
	typedef cin tcin;
	typedef clog tclog;
	typedef cerr tcerr;*/
#endif // UNICODE

	template <typename T1, typename T2, typename T3 = std::less<T1> >
	struct map
	{
		typedef std::map<T1, T2, T3, std::allocator<std::pair<T1, T2> > > type;
	};

	template <typename T1, typename T2, typename T3 = std::less<T1> >
	struct multimap
	{
		typedef std::multimap<T1, T2, T3, std::allocator<std::pair<T1, T2> > > type;
	};

	template <typename T1, typename T2 = std::less<T1> >
	struct set
	{
		typedef std::set<T1, T2, std::allocator<T1> > type;
	};

	template <typename T1, typename T2 = std::less<T1> >
	struct multiset
	{
		typedef std::multiset<T1, T2, std::allocator<T1> > type;
	};

	template <typename T>
	struct vector
	{
		typedef std::vector<T, std::allocator<T> > type;
	};

	template <typename T>
	struct list
	{
		typedef std::list<T, std::allocator<T> > type;
	};

	template <typename T>
	struct queue
	{
		typedef std::queue<T, std::allocator<T> > type;
	};	

	template <typename T1, typename T2 = vector<T1>, typename T3 = less<typename T2::value_type> >
	struct prority_queue
	{
		typedef std::priority_queue<T1, T2, T3> type;
	};

	template <typename T>
	struct deque
	{
		typedef std::deque<T, std::allocator<T> > type;
	};

// 	template <typename T>
// 	struct type_trait
// 	{
// 		typedef T value_type;
// 		typedef T& reference_type;
// 		typedef T* pointer_type;
// 		typedef const T& const_reference_type;
// 		typedef const T* const_pointer_type;
// 	};
// 
// 	template <typename T>
// 	struct type_trait<T&>
// 	{
// 		typedef T value_type;
// 		typedef T& reference_type;
// 		typedef T* pointer_type;
// 		typedef const T& const_reference_type;
// 		typedef const T* const_pointer_type;
// 	};
// 
// 	template <typename T>
// 	struct type_trait<T*>
// 	{
// 		typedef T value_type;
// 		typedef T& reference_type;
// 		typedef T* pointer_type;
// 		typedef const T& const_reference_type;
// 		typedef const T* const_pointer_type;
// 	};
// 
// 	template <typename T>
// 	struct type_trait<const T&>
// 	{
// 		typedef T value_type;
// 		typedef T& reference_type;
// 		typedef T* pointer_type;
// 		typedef const T& const_reference_type;
// 		typedef const T* const_pointer_type;
// 	};
// 
// 	template <typename T>
// 	struct type_trait<const T*>
// 	{
// 		typedef T value_type;
// 		typedef T& reference_type;
// 		typedef T* pointer_type;
// 		typedef const T& const_reference_type;
// 		typedef const T* const_pointer_type;
// 	};

	template <typename T>
	struct type2type
	{
		typedef T type;
	};
};


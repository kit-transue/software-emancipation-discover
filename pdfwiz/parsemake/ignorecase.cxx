/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#include <string>
#include <functional>
using namespace std;

template<typename charT> struct ichar_traits {
	typedef charT char_type;
	typedef char_traits<char_type> basetraits;

	static void assign(char_type &c1, char_type const &c2)
		{ basetraits::assign(c1, c2); }
	static bool eq(char_type const &c1, char_type const &c2)
		{ return tolower(c1) == tolower(c2); }
	static bool ne(char_type const &c1, char_type const &c2)
		{ return tolower(c1) != tolower(c2); }
	static bool lt(char_type const &c1, char_type const &c2)
		{ return tolower(c1) < tolower(c2); }
	static char_type eof()
		{ return basetraits::eof(); }

	// speedup functions
	static int compare(char_type const *s1, char_type const *s2, size_t n)
		{ while (n && *s1 != eof() && *s2 != eof()) {
				if (lt(*s1, *s2)) return -1;
				if (lt(*s2, *s1)) return 1;
				++s1;
				++s2;
				--n;
			}
			if (*s1 != eof())
				return 1;
			if (*s2 != eof())
				return -1;
			return 0;
		}
	//static char_type const *find(char_type const *s, int n, char_type const &a);
	static size_t length(char_type const *s)
		{ return basetraits::length(s); }
	static char_type *copy(char_type *s1, char_type const *s2, size_t n)
		{ return basetraits::copy(s1, s2, n); }
	static char_type *move(char_type *s1, char_type const *s2, size_t n)
		{ return basetraits::move(s1, s2, n); }
	static char_type *assign(char_type *s, size_t n, char_type const &a)
		{ return basetraits::assign(s, n, a); }
};


template <typename charT, typename _Tr = ichar_traits<charT> > struct bas_str_less_i :
	binary_function<basic_string<charT>, basic_string<charT>, bool>
{
	bool operator() (basic_string<charT> const &x, basic_string<charT> const &y) const
		{ return _Tr::compare(x.c_str(), y.c_str(), x.length() + 1) < 0; }
};


#include <iostream>
#include <set>

#if 0
typedef basic_string<char, ichar_traits<char> > striing;

int main()
{
	typedef set<striing, less <striing> > St;
	St s;
	s.insert("Hello");
	s.insert("bye");
	s.insert("hello");
	St::iterator i = s.begin();
	while (i != s.end()) {
		cout << i->c_str() << endl;
		++i;
	}
	return 0;
}
#else

int main()
{
	typedef set<string, bas_str_less_i<char> > St;
	St s;
	s.insert("hello");
	s.insert("bye2");
	s.insert("bye");
	s.insert("Hello");
	s.insert("Hello2");
	St::iterator i = s.begin();
	while (i != s.end()) {
		cout << i->c_str() << endl;
		++i;
	}
	return 0;
}
#endif

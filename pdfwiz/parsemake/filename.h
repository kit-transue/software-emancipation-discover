// filename.h
// 15.dec.97 Kit Transue

#if !defined(_FILENAME_H)
#define _FILENAME_H

#include <string>
#include <functional>

using namespace std;


class filename
{
public:
	// class data
	static char const filename::SEPARATOR; // = '\\';

	// static functions:
	static bool is_absolute_path(string const &);
	static bool is_root(string const &);
	static bool is_UNC_name(string const &);
	static string common_root(string const &onefile, string const &otherfile);
	static string drivename(string const &);
	static bool is_command(string const &line, string const &commandlist);

	// construct/copy/destroy
	filename(string const &);

	// access
	string const & name() const;
	string basename() const;
	string dirname() const;
private:
	string data;
};



// CREATE STRING EXTRACTOR ntfname_extractor(string &s):

// instances of this class are created by the instantiator template function below.
// The instance holds whatever parameters are required to perform the extraction
// --in this case, the target string to fill--.  The instance also provides
// a distinctive signature for the shift operator to attach to.

template<typename charT, typename traits>
class fname_extractor_class {
public:
	fname_extractor_class(basic_string<charT, traits> &s) : str(s) {};
	basic_string<charT, traits> & str;
	basic_istream<charT, traits> &extract(basic_istream<charT, traits> &);
};


// overloaded >> operator that performs extraction operation using the stream
template<typename charT, typename traits> basic_istream<charT, traits> &
operator>> (basic_istream<charT, traits> &is, fname_extractor_class<charT, traits> &fne);

// template function that instantiates an appropriate fname_extractor_class,
// encapsulating the target string reference, and providing a signature for
// the overloaded shift operator to handle.
template<typename charT, typename traits> fname_extractor_class<charT, traits>
ntfname_extractor(basic_string<charT, traits> & s);


// STRING EXTRACTOR ntfname_extractor(string &s):

template<typename charT, typename traits> basic_istream<charT, traits> &
fname_extractor_class<charT, traits>::extract(basic_istream<charT, traits> &is)
{
	str.erase();

	// consume leading whitespace
	is >> ws;

	charT c;
	bool inquote = false;
	bool end = false;
	while (!end && is.get(c)) {
		if (c == '"')
			inquote = !inquote;
		else if (isspace(c) && !inquote) {
			is.putback(c);
			end = true;
		}
		else
			str += c;
	}
	return is;
}

// template function that performs operation
template<typename charT, typename traits> basic_istream<charT, traits> &
operator>> (basic_istream<charT, traits> &is, fname_extractor_class<charT, traits> &fne)
{
	return fne.extract(is);
}

// template function that instantiates an appropriate fname_extractor_class,
// encapsulating the target string reference, and providing a signature for
// the overloaded shift operator to handle.
template<typename charT, typename traits> fname_extractor_class<charT, traits>
ntfname_extractor(basic_string<charT, traits> & s)
{
	fname_extractor_class<charT, traits> f(s);
	return f;
}


#if 0  // sample  ntfname_extractor usage:
#include <strstream>
#include <iostream>
int main()
{
	string s;
	strstream t;
	t << "hello, filename: c:\"Program Files\"\\tmp\"\" done";
	for (int i = 0; i < 5; ++i) {
		t >> ntfname_extractor(s);
		cout << '-' << s << '-' << endl;
	}
	return 0;
}
#endif // sample usage


template<typename charT> struct ntfname_traits {
	typedef charT char_type;
	typedef char_traits<char_type> basetraits;

	static void assign(char_type &c1, char_type const &c2)
		{ basetraits::assign(c1, c2); }
	static bool eq(char_type const &c1, char_type const &c2)
		{
			return tolower(c1) == tolower(c2)
				|| ((c1 == '/' || c1 == '\\') &&
				 	(c2 == '/' || c2 == '\\'));
		}
	static bool ne(char_type const &c1, char_type const &c2)
		{ return !eq(c1, c2); }
	static bool lt(char_type const &c1, char_type const &c2)
		{ return tolower(c1) < tolower(c2) && ne(c1, c2); }
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
			if (n) {
				if (*s1 != eof()) return 1;
				if (*s2 != eof()) return -1;
			}
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


template <typename charT, typename _Tr = ntfname_traits<charT> > struct ntfname_str_less :
	binary_function<basic_string<charT>, basic_string<charT>, bool>
{
	bool operator() (basic_string<charT> const &x, basic_string<charT> const &y) const
		{ return _Tr::compare(x.c_str(), y.c_str(), x.length() + 1) < 0; }
};



#endif // !defined(_FILENAME_H)

// option_extractor.h
// 10.feb.98 kit transue
// option extractor that extracts "-F" or "/F" from input.
// note that the VisualC++ compiler and NMAKE both consider a leading
// slash to be an option, but all other slashes may be path separator.
// Such convention makes this extractor with single character lookahead
// (and single character putback) possible.

// currently fetches only the first character of the option

#include <iostream>
#include <string>


using namespace std;


template<typename charT, typename traits>
class option_extractor_class {
public:
	option_extractor_class(basic_string<charT, traits> &s) : str(s) {};
	basic_string<charT, traits> & str;
	basic_istream<charT, traits> & extract(basic_istream<charT, traits> &is) {
		
		str.erase();

		// consume leading whitespace
		is >> ws;

		charT c;
		if (is.get(c)) {
			switch(c) {
				case '-':
				case '/':
					if (is.get(c))
						str += toupper(c);
					break;
				default:
					is.putback(c);
					break;
			}
		}
		return is;
	}
};


// template function that performs operation
template<typename charT, typename traits> basic_istream<charT, traits> &
operator>> (basic_istream<charT, traits> &is, option_extractor_class<charT, traits> &oe)
{
	return oe.extract(is);
}


// template function that instantiates an appropriate option_extractor_class,
// encapsulating the target string reference, and providing a signature for
// the overloaded shift operator to handle.

template<typename charT, typename traits>
option_extractor_class<charT, traits> option_extractor(basic_string<charT, traits> & s)
{
	option_extractor_class<charT, traits> oe(s);
	return oe;
}


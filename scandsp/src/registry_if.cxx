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
// registry_if.cxx
// 11.dec.97 Kit Transue
// map-semantic access to NT registry

#include <windows.h>

#include <utility> 	// for pair
#include <cassert>

#include "registry_if.h"


/* the registry is an interesting beast.  It is hierarchical in nature,
and traversing the hierarchy requires opening each level.  It is probably
desirable to close open handles when they are not in use.

Translating this into array-like access may be a little tricky to do
efficiently.  There should be some caching of open nodes, and some
mechanism for closing them after some time.  References will probably
be close to each other, so such caching would be useful.  But this
is all complicated, so efficiency be damned.

Finally, the terminology is whacked.  Instead of key-value pairings, there
is a key-value-value triple, where the first value is part of the key.
This class attempts to hide such malarky.
*/

//\\//\\//\\//\\//\\//\\//\\
// class static members
char const registry::SEPARATOR = '\\';


//\\//\\//\\//\\//\\//\\//\\
// useful declarations
typedef pair<HKEY, LPCSTR> registry_key;

//\\//\\//\\//\\//\\//\\//\\
// static functions.  Using static functions instead of member functions
// so windows.h need not be included from registry_if.h


static registry_key
open_from_string(char * buffer, HKEY currentkey, bool create)
{
	registry_key ret(0,0);
	// find first component remaining in path....
	char * end;
	bool last_component = false;
	for (end = buffer; *end && *end != registry::SEPARATOR; ++end);
	if (*end == 0)
		last_component = true;
	// ...and use it for next attempt to open
	*end = 0;

	if (last_component) {
		ret = registry_key(currentkey, buffer);
	}
	else {
		bool success = false;
		HKEY newsubkey;
		if (create) {  // try first to open key writable
			DWORD disposition;
			success = ( ERROR_SUCCESS ==
				RegCreateKeyEx(currentkey, buffer, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &newsubkey, &disposition)
			);
		}
		if (! success) {
			// either we're opening for read, or open for write failed:
			success = ( ERROR_SUCCESS ==
				RegOpenKeyEx(currentkey, buffer, NULL, KEY_READ, &newsubkey)
			);
		}
		if (success) {
			ret = open_from_string(end + 1, newsubkey, create);
			// make sure not to close the return value:
			if (ret.first != newsubkey)
				RegCloseKey(newsubkey);
		}
	}
	return ret;
}

static string
get_string_from_key(registry_key rk)
{
	HKEY key = rk.first;
	LPCTSTR subkey = rk.second;
	DWORD data_size = 0;
	DWORD lpType;

	string ret;

	// could use a 2k buffer and only do memory allocation if ERR_TOO_LONG
	// find length of string:
	if ( (ERROR_SUCCESS ==
		RegQueryValueEx(key, subkey, NULL, &lpType, NULL, &data_size))
		&& (lpType == REG_SZ)
	) {
		char *data = new char[data_size];
		if ( ERROR_SUCCESS ==
			RegQueryValueEx(key, subkey, NULL, &lpType, (LPBYTE)data, &data_size)
		) {
			// The returned data includes final null.
			bool has_null = data_size > 0 && data[data_size - 1] == '\0';
			ret.append(data, has_null ? data_size - 1 : data_size);
		}
		delete[] data;
	}
	return ret;
}

static registry_key
open_from_string_top(string const &s, bool create = false)
{
	// STATIC BUFFER MEANS THIS ISN'T RE-ENTRANT
	static char buffer[MAX_PATH];

	const int BUFLEN = sizeof(buffer) / sizeof(buffer[0]);
	s.copy(buffer, BUFLEN);
	assert(s.length() < BUFLEN);
	buffer[min(s.length(), BUFLEN - 1)] = 0;

	registry_key ret(0, 0);
	bool success = true;
	HKEY key;

	// find first component remaining in path....
	char * end;
	bool last_component = false;
	for (end = buffer; *end && *end != registry::SEPARATOR; ++end);
	if (*end == 0)
		last_component = true;
	// ...and use it for next attempt to open
	*end = 0;

	// toplevel keys:
	if (!strcmp(buffer, "HKEY_CLASSES_ROOT"))
		 key = HKEY_CLASSES_ROOT;
	else if (!strcmp(buffer, "HKEY_CURRENT_CONFIG"))
		 key = HKEY_CURRENT_CONFIG;
	else if (!strcmp(buffer, "HKEY_CURRENT_USER"))
		 key = HKEY_CURRENT_USER;
	else if (!strcmp(buffer, "HKEY_LOCAL_MACHINE"))
		 key = HKEY_LOCAL_MACHINE;
	else if (!strcmp(buffer, "HKEY_USERS"))
		 key = HKEY_USERS;
	// HKEY_PERFORMANCE_DATA
	// HKEY_DYN_DATA
	else 
		success = false;

	// recurse if possible and necessary:
	if ( success && ! last_component) {
		ret = open_from_string(end + 1, key, create);
	}
	return ret;
}

//\\//\\//\\//\\//\\//\\//\\
// class member function implementations

string 
registry::at(string const &s)
{
	string ret;
	registry_key key = open_from_string_top(s);
	if (key.first) {
		ret = get_string_from_key(key);
		RegCloseKey(key.first);
	}
	return ret;
}

bool 
registry::exists(string const &s)
{
	registry_key key = open_from_string_top(s);
	if (key.first)
		RegCloseKey(key.first);
	return (key.first != 0);
}

void
registry::set(string const &s, string const &value)
{
	registry_key key = open_from_string_top(s, true);
	if (key.first) {
		RegSetValueEx(key.first, key.second, 0, REG_SZ, (LPBYTE)value.c_str(), value.length() + 1);

		RegCloseKey(key.first);
	}
}

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
// pdfwiz_setttings.cxx
// 24.Mar.98 Kit Transue

// Settings for pdf wizard

#include <string>
using namespace std;

#include "registry_if.h"
#include "setech_utils.h"  // for current version

#include "pdfwiz_settings.h"

static string
prefs_base()
{
	string ret =
		"HKEY_CURRENT_USER\\Software\\Software Emancipation Technology\\DISCOVER\\";
	ret += setech_utils::version();
	ret += "\\PDF Wizard\\";
	return ret;
}

bool
settings::old_filenames()
{
	bool ret = false;

	string key = prefs_base() + "Use quotes in PDF";
	if (registry::exists(key)) {
		ret = (registry::at(key) == "false");
	}
	return ret;
}

void
settings::set_old_filenames(bool x)
{
	string key = prefs_base() + "Use quotes in PDF";
	string value((x) ? "false" : "true");
	registry::set(key, value);
}

string const&
settings::compiler_list()
{
	static string cl("$(CC) $(CPP) cl.exe cl");

	string key = prefs_base() + "Compiler names";
	if (registry::exists(key)) {
		cl = registry::at(key);
	}

	return cl;
}

void
settings::set_compiler_list(string const &value)
{
	string key = prefs_base() + "Compiler names";
	registry::set(key, value);
}

string const &
settings::make_aliases()
{
	static string makes("$(MAKE) nmake.exe nmake omake make");

	string key = prefs_base() + "Names for MAKE";
	if (registry::exists(key)) {
		makes = registry::at(key);
	}

	return makes;
}


void 
settings::set_make_aliases(string const &value)
{
	string key = prefs_base() + "Names for MAKE";
	registry::set(key, value);
}

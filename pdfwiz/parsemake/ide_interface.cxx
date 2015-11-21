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
// ide_interface.cxx
// 11.dec.97 Kit Transue
// access to ide settings



#include "registry_if.h"

#include "ide_interface.h"

using namespace std;


// class static implementations:
ide_interface *
ide_interface::get_platform_interface()
{
	// return a vc5 or vc4 interface that is appropriate for the current platform
	ide_interface * ret = 0;
	// try and open a DevStudio 5 registry entry that should exist:
	static char VC5KEY[] = "HKEY_CURRENT_USER\\Software\\Microsoft\\DevStudio\\5.0";
	if (registry::exists(VC5KEY)) {
		ret = new vc5_ide;
	} else {
		ret = new vc4_ide;
	}
	return ret;
}

ide_interface::~ide_interface()
{
}

ide_interface::stringvec const &
vc5_ide::last_projects()
{
	static char projkey[] = "HKEY_CURRENT_USER\\Software\\Microsoft\\DevStudio\\5.0\\Recent File List\\Project1";

	if (lps.empty()) {
		for (int i = 0; i < 4; ++i) {
			projkey[sizeof(projkey)/sizeof(projkey[0]) - 2] = i + '1';
			string ret(registry::at(projkey));
	
			// convert project name to .mak, if project was found
			if (!ret.empty()) {
				size_t xpos = ret.find_last_of(".");  // should be constant somewhere
				if (xpos != string::npos) {
					ret.replace(xpos, string::npos, ".mak", 4);
				}
				else {
					ret += ".mak";
				}
			}
			lps.push_back(ret);
		}
	}
	return lps;
}

string
vc5_ide::standard_includes()
{
	//"Software\\Microsoft\\DevStudio\\5.0\\Build System\\Components\\Platforms\\Win32 (x86)\\Directories\\Include Dirs"
	return string();
}


string
vc5_ide::standard_defines()
{
	// probably a lookup from our table based on the complier version
	return string();
}

string
vc5_ide::standard_flags()
{
	// probably a lookup from our table based on the complier version
	return string();
}




ide_interface::stringvec const &
vc4_ide::last_projects()
{
	static char projkey[] = "HKEY_CURRENT_USER\\Software\\Microsoft\\Developer\\Recent File List\\Project1";

	if (lps.empty()) {
		for (int i = 0; i < 4; ++i) {
			projkey[sizeof(projkey)/sizeof(projkey[0]) - 2] = i + '1';
			string ret(registry::at(projkey));
	
			// convert project name to .mak, if project was found
			if (!ret.empty()) {
				size_t xpos = ret.find_last_of(".");  // should be constant somewhere
				if (xpos != string::npos) {
					ret.replace(xpos, string::npos, ".mak", 4);
				}
				else {
					ret += ".mak";
				}
			}
			lps.push_back(ret);
		}
	}
	return lps;
}

string
vc4_ide::standard_includes()
{
	return string();
}


string
vc4_ide::standard_defines()
{
	return string();
}

string
vc4_ide::standard_flags()
{
	return string();
}




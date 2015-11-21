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
// filetypes.cxx
// 4.Dec.97 Kit Transue
// decision support for file names

#include "filetypes.h"
#include <string>
using namespace std;

// globals
filetype_context global_filetypes;
static filetype const unknowN("", "unknown", filetype::UNKNOWN);
filetype const * const filetype::Unknown = &unknowN;


filetype::filetype(string const &ext, string const &desc, ekind k) : extension(ext), description(desc), kind(k) {}

bool
filetype::include_in_pdf() const
{
	bool ret = false;
	switch (kind) {
		case CSOURCE:
		case CPPSOURCE:
		case HEADER:
			ret = true;
			break;
		case MAKEFILE:
		case EXECUTABLE:
		case OBJECT:
		case USER_TYPE:
		case UNIMPORTANT:
		case UNKNOWN:
			// ret = false;
			break;
	}
	return ret;
}

filetype const &
filetype_context::what_type(string const &filename)
{
	filetype const * ret = filetype::Unknown;
	string ext(extension(filename));
	if (!ext.empty())
	{
		tabletype::iterator ftptr;
		ftptr = ext_table.find(ext);
		if (ftptr != ext_table.end()) {
			ret = &(ftptr->second);
		}
	}
	return *ret;
}

filetype_context::filetype_context() : ext_delims(".")
{

	struct {char *ext; char *desc; filetype::ekind kind;} fttab[] =
	{
		{".c",	"C source",	filetype::CSOURCE},
		{".cxx",	"C++ source",	filetype::CPPSOURCE},
		{".cpp",	"C++ source",	filetype::CPPSOURCE},
		{".inl",	"C++ inline source",	filetype::CPPSOURCE},
		{".h",	"C/C++ header",	filetype::HEADER},
		{".obj",	"object file",	filetype::OBJECT},
		{".exe",	"executable",	filetype::EXECUTABLE},
		{".dll",	"DLL",	filetype::EXECUTABLE},
		{".lib",	"Library",	filetype::OBJECT},
		{".ocx",	"OLE Control",	filetype::EXECUTABLE},
		{".l",	"lex source",	filetype::USER_TYPE},
		{".y",	"yacc source",	filetype::USER_TYPE},
		{".pdb",	"Debugger Information",	filetype::UNIMPORTANT},
		{".mak",	"Makefile",	filetype::MAKEFILE},
		{".def",	"Module Definition File",	filetype::UNIMPORTANT},
		{".ncb",	"Program Database",	filetype::UNIMPORTANT},
		{".plg",	"Build log?",	filetype::UNIMPORTANT},
		{".opt",	"Developers Studio options file?",	filetype::UNIMPORTANT},
		{".dsw",	"Project Workspace File",	filetype::UNIMPORTANT},
		{".dsp",	"Project File",	filetype::UNIMPORTANT},
		{".pbi",	"Profile Prep output",	filetype::UNIMPORTANT},
		{".pbt",	"Profiling data",	filetype::UNIMPORTANT},
		{".pbo",	"Profiling data",	filetype::UNIMPORTANT},
		{".map",	"Mapfile",	filetype::UNIMPORTANT},
		{".ncb",	"Program Database",	filetype::UNIMPORTANT},
		{".mdp",	"Project Workspace",	filetype::UNIMPORTANT},
		{".ilk",	"Incremental Link information",	filetype::UNIMPORTANT},
		{".bsc",	"Browser Database",	filetype::UNIMPORTANT},
		{".sbr",	"Source Browser",	filetype::UNIMPORTANT},
		{".dbg",	"Debugger symbol file",	filetype::UNIMPORTANT},
		{".exp",	"Exports files",	filetype::UNIMPORTANT},
		{".res",	"Resource File",	filetype::UNIMPORTANT},
		{".ico",	"Icon File",	filetype::UNIMPORTANT},
		{".odl",	"Type Library/Interface Definition",	filetype::UNIMPORTANT},
		{".pch",	"Precompiled Headers",	filetype::UNIMPORTANT},
		{".tlb",	"Type Library",	filetype::UNIMPORTANT},
		{".tlh",	"Type Library Header",	filetype::UNIMPORTANT},
		{".tli",	"Type Library Header Implementation",	filetype::UNIMPORTANT},
		// should not appear in make output:
		{".cab",	"COM Cabinet File",	filetype::UNIMPORTANT},
		{".inf",	"Installation information",	filetype::UNIMPORTANT},
		{".ivt",	"InfoViewer Title",	filetype::UNIMPORTANT},
		{".dsm",	"Developer Studio Macro",	filetype::UNIMPORTANT},
	};

	for ( int i = 0; i < sizeof(fttab) / sizeof(fttab[0]); ++i) {
		add_type(filetype(fttab[i].ext, fttab[i].desc, fttab[i].kind));
	}
};


void
filetype_context::add_type(filetype const &x)
{
	ext_table.insert(tabletype::value_type(x.extension, x));
}

string
filetype_context::extension(string const &filename)
{
	string ret;
	size_t xpos = filename.find_last_of(ext_delims);
	if (xpos != string::npos) {	// attempt to look up extension
		ret = string(filename, xpos, string::npos);  // MSVC doesn't provide proper default arguments as spec'd in standard
	}
	return ret;
}

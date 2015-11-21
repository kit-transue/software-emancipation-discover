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
// CompilerOptions.C
//------------------------------------------
// synopsis:
// Interface to compiler options.
 

// INCLUDE FILES
#include <CompilerOptions.h>
#include <customize.h>
#include <machdep.h>


//////////     CompilerOptions Class Definition     //////////

char const CompilerOptions::make_output_location_string[] = "/%/.make ";
const size_t CompilerOptions::make_output_location_string_len =
        sizeof(make_output_location_string) - 1;

CompilerOptions::CompilerOptions()
{
}


CompilerOptions::~CompilerOptions()
{
}



char const *CompilerOptions::getName(const int compiler)
{
    char const *name = "UNKNOWN";

    switch ((CompilerOptions::CompilerType) compiler) {
        case CompilerOptions::C_COMPILER:
            name = customize::c_compiler();
            break;
        case CompilerOptions::CPP_COMPILER:
            name = customize::cplusplus_compiler();
            break;
    }
    return (name);
}



char const *CompilerOptions::getOptions(const int compiler)
{
    switch ((CompilerOptions::CompilerType) compiler) {
        case CompilerOptions::C_COMPILER: {
            char const *flags   = customize::getStrPref("cFlags");
            char const *defines = customize::getStrPref("cDefines");
            char const *includes= customize::getStrPref("cIncludes");
            merge_options(flags, defines, includes);
            // C compiler must have "-lang-c" flag.
            if (strstr((char const *)defaultOptions, "-lang-c") == 0) 
                defaultOptions += " -lang-c";
            } break;

        case CompilerOptions::CPP_COMPILER: {
            char const *flags   = customize::getStrPref("cPlusFlags");
            char const *defines = customize::getStrPref("cPlusDefines");
            char const *includes= customize::getStrPref("cPlusIncludes");
            merge_options(flags, defines, includes);

            // C++ compiler must have "-lang-c++" flag.
            if (strstr((char const *)defaultOptions, "-lang-c++") == 0) 
                defaultOptions += " -lang-c++";

            // C++ compiler must have "-D__cplusplus" flag.
            if (strstr((char const *)defaultOptions, "-D__cplusplus") == 0) 
                defaultOptions += " -D__cplusplus";
            } break;
    }

#ifdef _WIN32
    convert_back_slashes( defaultOptions.str() );
#endif
    return(defaultOptions);
}


void CompilerOptions::merge_options(char const *flags,
				    char const *defines,
				    char const *includes) {
    char const *flags_marker = strstr(flags, make_output_location_string);
    char const *defines_marker =
	    strstr(defines, make_output_location_string);
    char const *includes_marker =
	    strstr(includes, make_output_location_string);
    if (flags_marker || defines_marker || includes_marker) {
	size_t flags_before_len = flags_marker ? flags_marker - flags :
	        strlen(flags);
	size_t defines_before_len = defines_marker ?
		defines_marker - defines : strlen(defines);
	size_t includes_before_len = includes_marker ?
		includes_marker - includes : strlen(includes);
	defaultOptions.printf(" %.*s %.*s %.*s ", flags_before_len, flags,
			      defines_before_len, defines,
			      includes_before_len, includes);
	defaultOptions += make_output_location_string;
	if (flags_marker) {
	    defaultOptions += flags_marker + make_output_location_string_len;
	    defaultOptions += " ";
	}
	if (defines_marker) {
	    defaultOptions += defines_marker +
		    make_output_location_string_len;
	    defaultOptions += " ";
	}
	if (includes_marker) {
	    defaultOptions += includes_marker +
		    make_output_location_string_len;
	    defaultOptions += " ";
	}
    }
    else defaultOptions.printf(" %s %s %s ", flags, defines, includes);
}

//////////     end of CompilerOptions.C     //////////

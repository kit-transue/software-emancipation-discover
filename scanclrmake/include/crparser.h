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
#ifndef CRPARSER_H
#define CRPARSER_H

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif
#include <string>
#include "idcompiler.h"

namespace std {};
using namespace std;

class ParseLine;

class CRParser {
public:
    static bool parseCR(istream &input, const char *crFileName,
			const char *compiler_list,
			ostream &ostrm,
			bool verbose,
			bool allow_any_extension,
			bool allow_any_flags,
	      		string *forcedProjectName = 0);

    static void substituteEnvVars(string &);

    static bool isPathSep(char);
    static bool isAbsolute(const char *name);
    static void determineWd(string &wd, const char *dspFileName);

private:
    CRParser(const char *dspFileName, const char *compiler_list,
	     ostream &ostrm, bool verbose,
	     bool allow_any_extension, bool allow_any_flags,
	     string *forcedProjectName = 0);
    ~CRParser();
    static void appendToDirectory(const string &, string&);
    bool isValid();
    void trackHRules(ParseLine &);
    void trackSection(ParseLine &);
    void trackTarget(ParseLine &);
    void processBuildRule(ParseLine &);
    void trackHost(ParseLine &);
    void initLineState();
    bool extensionOK(const string &file);

    bool beVerbose;
    bool checkExtension;
    bool checkFlags;
    string *projectName;
    string *imposedProjectName;
    CompilerIdentifier compilerIdentifier;
    ostream &ostrm;

    // state of the parse, line-related:
    bool inBuildRules;
    int hRuleCount;
    int prevHRuleCount;
    typedef enum { no_sct, intro_sct, obj_sct, env_sct, bld_sct, misc_sct } section;
    section currentSection;
    int targetCount;
    int lineCountInSection;
    string targetName;
    string firstTargetName;
    string workingDirectory;
};

#endif // CRPARSER_H

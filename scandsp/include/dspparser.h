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
#ifndef DSPPARSER_H
#define DSPPARSER_H

#include <iostream>
#include <stack>
#include <list>
#include <map>
#include <set>
class Line;
#include "token.h"

// ===========================================================================
// This module provides a the function
//        DSPParser::parseDSP
// which parses a MS Visual Studio DSP file (input),
// and produces a list of the source files in the project (to ostrm),
// with codes indicating how each one is processed.
// There are also a few public helper functions.
// ===========================================================================

class DSPParser {
public:
    static bool parseDSP(std::istream &input, const char *dspFileName,
			 bool preprocess,
			 std::ostream &ostrm,
			 bool verbose,
	      		 std::string *forcedProjectName = 0,
			 std::string *cfg = NULL);

    typedef std::map<std::string, std::string> an_env_map;
    typedef std::set<std::string> a_str_set;

    static void substituteEnvVars(std::string &, void * = NULL);
    static void dropExtraPrefix(std::string &);

    static const char pathSep;
    static bool isPathSep(char);
    static bool isAbsolute(const char *name);
    static void determineWd(std::string &wd, const char *dspFileName);
    static bool analyzeVCIncludes(std::string &cmd_options, bool VC_syntax,
							    DSPParser * = 0);
    static bool analyzeVCDir(std::string &msvc_dir);

private:
    DSPParser(const char *dspFileName, bool preprocess, std::ostream &ostrm, 
	    bool verbose, std::string *forcedProjectName = 0,
			  std::string *cfg = NULL);
    ~DSPParser();
    void trackLine(const Line &);
    bool preprocessLine(const Line &);
    bool eval(const std::list<Token> &);
    void eval(std::string &result, const std::string &);
    std::string getAndEvalString(const Line &);
    void charStrip(char ch, std::string &result, const std::string &);
    void determineWd(const char *dspFileName);
    void determineVCIncludes();
    static void appendToDirectory(const std::string &, std::string &);
    static void filterSpuriousDotDots(const std::string &, std::string &);
    std::string addKey(const std::string&);
    std::string propKey(const std::string&);
    std::string currentCPPOptions();
    std::string environmentLookup(const std::string &key);
    bool isValid();
    void noteOptionIncludeDirs(const std::string &option);
    void noteVCIncludeDir(const std::string &dir);
    void checkIncludeDirs(std::set<std::string> &dirs, const std::string &origin);
    void checkOverridingCfg(const std::string &origin);
    void addToEnvironment(an_env_map &, const std::string &key,
			  const std::string &value);

    // per-object data
    struct ifState {
	bool active;
	bool testing;
    };
    struct group {
	bool hasName;
	std::string name;
    };
    std::stack<ifState> ifStack;
    bool doPreprocessing;
    bool beVerbose;
    std::stack<group> groupStack;

    // environmentStack tracks the values of variables such as
    // CPP which, when set within a Begin..End, revert to their
    // former values afterward.
    std::stack<an_env_map *> environmentStack;

    // currentEnvironment always returns the top of the environmentStack.
    an_env_map &currentEnvironment();
    bool lookupBool(std::string key, bool dflt);

    bool active;
    int beginEndLevel;
    std::string wd;
    std::string dspFileName;
    std::string vcIncludes;
    std::string *projectName;
    bool foundVC;
    a_str_set DSPIncludeDirs;
    a_str_set VCIncludeDirs;
    std::string *imposedProjectName;
    std::string *cfg;
    bool cfgValidated;
    std::string *cfgUsed;
    int cfgState;
    std::ostream& outStream;
};

extern const char *compilation_units_message_group_id;

#endif // DSPPARSER_H

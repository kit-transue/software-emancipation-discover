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
#ifndef DSWPARSER_H
#define DSWPARSER_H

#include <iostream>
#include <stack>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "parser.h"

// ===========================================================================
// This module provides a the function
//        DSWParser::parseDSW
// which parses a MS Visual Studio DSW file (input),
// and produces a list of the source files in the workspace (to ostrm),
// with codes indicating how each one is processed.
//
// Naturally, it does this by identifying the projects in the workspace
// and calling DSPParser::parseDSP for them.
//
// Other public member functions are not really to be called from
// outside the module.
// ===========================================================================

struct ProjectInfo {
    ProjectInfo();
    ProjectInfo(const ProjectInfo &);
    ~ProjectInfo();

    std::string project;
    std::string file;
    std::string *cfg;
    std::string wd;
};

class DSWParser : public Parser {
public:
    static bool parseDSW(std::istream &input, const char *dswFileName, 
	std::ostream &ostrm, bool verbose);
    static bool parseDSWForInfo(std::istream &input, const char *dswFileName, 
	std::vector<ProjectInfo> &, std::ostream &ostrm, bool verbose);
    void noteProject(const std::string &projectName,
				const std::string &fileName);
    static std::string nameForWorkspace(const std::string &dswFileName);
    DSWParser(const char *input,
              const char *dswFileName,
              std::ostream &,
              bool verbose,
              std::vector<ProjectInfo> &);
    ~DSWParser();
    bool file();
    bool horizontalSeparator();
    bool section();
    bool project();
    bool quotedName(std::string &name);
    bool projectLine();
    bool packageStuff();
    bool findClose();
    bool global();
    bool version(std::string &verNum);
    bool warning();
    bool lineBreak();
    bool listBy(bool (DSWParser::*delim)(), bool (DSWParser::*elem)());
    bool fileName(std::string &fn);
    static bool isDSW(const char *file_name);
private:
    std::ostream &outStream;
    bool beVerbose;
    bool hadTrouble;
    std::string wd;
    std::string originalWd;
    std::string dswFileName;
    std::vector<ProjectInfo> &projects;
};

#endif // DSWPARSER_H

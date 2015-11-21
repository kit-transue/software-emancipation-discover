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
#pragma warning (disable:4786)

#include <iostream>
#include <sstream>
#include <fstream>
#include <ctype.h>
#include <direct.h>
#include "dswparser.h"
#include "dspparser.h"
#include "optparser.h"
#include "msg.h"

using namespace std;

// ===========================================================================
// static member function
// Reads the DSW file, and for each project listed therein, reads the DSP
// file and gathers the parameters needed to parse it.
//
bool DSWParser::parseDSW(istream &input, const char *dswFileName, 
			 ostream &ostrm, bool verbose)
{
    vector<ProjectInfo> projects;
    bool isOk = parseDSWForInfo(input, dswFileName, projects, ostrm, verbose);

    vector<ProjectInfo>::iterator proj;
    for (proj = projects.begin(); proj != projects.end(); ++proj) {
	if ((chdir(proj->wd.c_str()) != 0)) {
	    msg("Failed to change working directory to $1.", error_sev) << proj->wd << eoarg << eom;
	    isOk = false;
	}
	else {
	    ifstream f(proj->file.c_str(), ios::in);
	    bool ok = f.is_open();
	    if (!ok) {
		msg("Failed to open $1.", catastrophe_sev) << proj->file << eoarg << eom;
		isOk = false;
	    }
	    else {
		string name = nameForWorkspace(string(dswFileName));
		ok = DSPParser::parseDSP(f, proj->file.c_str(), true, ostrm, 
		    verbose, &name, proj->cfg);
		if (!ok) {
		    msg("Failed to parse $1.", error_sev) << proj->file << eoarg << eom;
		}
	    }
	}
	msg("Noted project $1 at $2.", normal_sev) << proj->project << eoarg
	    << proj->file << eoarg << eom;
    }
    return isOk;
}

// ===========================================================================
bool combineCfgs(vector<string> &cfgs, vector<ProjectInfo> &projs, string &dswFileName)
{
    if (cfgs.size() != projs.size()) {
	msg("Number of CFGs found in OPT file, $1, does not match $2 projects in $3.",
	    warning_sev) << cfgs.size() << eoarg << projs.size() << eoarg
                         << dswFileName << eoarg << eom;
    }
    else {
        msg("Number of projects in $1 is $2.", normal_sev)
            << dswFileName << eoarg << projs.size() << eoarg << eom;
	vector<string>::iterator cfg;
	vector<ProjectInfo>::iterator proj;
        for (cfg = cfgs.begin(), proj = projs.begin();
	     cfg != cfgs.end();
	     ++cfg, ++proj) {
	    proj->cfg = new string(*cfg);
	    msg("Found CFG setting \"$1\" in workspace, for project $2.", normal_sev)
                << *cfg << eoarg << proj->project << eoarg << eom;
	}
    }
    return true;
}

// ===========================================================================
static string formOptName(string dswFileName)
{
    long end = dswFileName.size();
    if (DSWParser::isDSW(dswFileName.c_str())) {
	end -= 4;
    }
    string result = dswFileName.substr(0, end);
    result.append(".opt");
    return result;
}

// ===========================================================================
// static member function
// Reads the DSW file and makes a list of projects.
// Attempts to read a corresponding OPT file, to get the CFG setting.
//
bool DSWParser::parseDSWForInfo(istream &input, const char *dswFileName, 
			 vector<ProjectInfo> &projects, ostream &ostrm, bool verbose)
{
    stringstream string_input;
    {
	input.get(*string_input.rdbuf(), '\0');
#if 0
	if (input.eof() && string_input.pcount() == 0) {  
	    break;
	}
#endif
	if (input.bad()) {
	    msg("Read failure on $1.", catastrophe_sev) << dswFileName << eoarg << eom;
	    return false;
	}
    }
    string string_input_str = string_input.str();
    bool isOk = true;
    {
	DSWParser par(string_input_str.c_str(), dswFileName, ostrm, verbose, projects);
	isOk = par.file() && isOk;
    }
    {
	string optFileName = formOptName(dswFileName);
	vector<string> cfgs;
	string message;
	int status = optparse(optFileName.c_str(), cfgs, message);
	if (status != 0) {
	    msg(message.c_str(), warning_sev) << eom;
	}
	if (status == 0 || status == 1) {
	    // OK or warning
	    combineCfgs(cfgs, projects, string(dswFileName));
	}
    }
    return isOk;
}

// ===========================================================================
void DSWParser::noteProject(const std::string &projectName,
	                    const std::string &fileName)
{
    ProjectInfo info;
    info.project = projectName;
    info.file = fileName;
    info.wd = wd;
    projects.push_back(info);
}

// ===========================================================================
string DSWParser::nameForWorkspace(const string& dswFileName)
{
    const char *fn = dswFileName.c_str();
    long start = 0;
    for (const char *p = fn; *p != '\0'; p += 1) {
        if (DSPParser::isPathSep(*p)) {
	    start = (p - fn) + 1;
	}
    }
    long end = dswFileName.size();
    if (isDSW(fn)) {
	end -= 4;
    }
    return dswFileName.substr(start, end - start);
}

// ===========================================================================
DSWParser::DSWParser(const char *input, const char *dswFileName, ostream &ostrm, bool verbose,
                     vector<ProjectInfo> &projs)
  : Parser(input),
    outStream(ostrm),
    beVerbose(verbose),
    hadTrouble(false),
    dswFileName(dswFileName),
    projects(projs)
{
    DSPParser::determineWd(wd, dswFileName);
    originalWd = getcwd(NULL, 0);
}

// ===========================================================================
DSWParser::~DSWParser()
{
    chdir(originalWd.c_str());
}

// ===========================================================================
// Parsing functions follow.
// See comment in parser.C describing how these must behave when failing.

// ===========================================================================
bool DSWParser::file()
{
    string vernum;
    tracker isOk(*this);
    version(vernum) || isOk.fail();
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	warning() || isOk.fail();
    }
    if (isOk) {
	listBy(&DSWParser::horizontalSeparator, &DSWParser::section)
	    || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool DSWParser::horizontalSeparator()
{
    tracker isOk(*this);
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    // a line of at least 2 #'s
    if (isOk) {
	int poundCount = 0;
	for (;;) {
	    if (!parseChar('#')) {
		break;
	    }
	    poundCount += 1;
	}
	if (poundCount < 2) {
	    isOk.fail();
	}
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool DSWParser::section()
{
    tracker isOk(*this);
    project() || global() || isOk.fail();
    return isOk;
}

// ===========================================================================
bool DSWParser::project()
{
    tracker isOk(*this);
    projectLine() || isOk.fail();
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	packageStuff() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	packageStuff() || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool notQuote(char ch)
{
    return ch != '"';
}

// ===========================================================================
bool DSWParser::quotedName(string &name)
{
    tracker isOk(*this);
    if (isOk) {
	parseChar('"') || isOk.fail();
    }
    if (isOk) {
	parseSatisfying(name, &notQuote) || isOk.fail();
    }
    if (isOk) {
	parseChar('"') || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool isnum(char ch)
{
  return isdigit(ch) != 0;
}

// ===========================================================================
bool DSWParser::projectLine()
{
    tracker isOk(*this);
    string name;
    string file;
    string num;
    matchChars("Project: ") || isOk.fail();
    if (isOk) {
	quotedName(name) || isOk.fail();
    }
    if (isOk) {
	parseChar('=') || isOk.fail();
    }
    if (isOk) {
	quotedName(file) || fileName(file) || isOk.fail();
    }
    if (isOk) {
        matchChars(" - Package Owner=<") || isOk.fail();
    }
    if (isOk) {
	parseSatisfying(num, &isnum) || isOk.fail();
    }
    if (isOk) {
	parseChar('>') || isOk.fail();
    }
    if (isOk) {
	noteProject(name, file);
    }
    return isOk;
}

// ===========================================================================
bool DSWParser::packageStuff()
{
    string num;
    tracker isOk(*this);
    matchChars("Package=<") || isOk.fail();
    if (isOk) {
	parseSatisfying(num, &isnum) || isOk.fail();
    }
    if (isOk) {
	matchChars(">") || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	matchChars("{{{") || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	findClose() || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool notNewline(char ch)
{
    return ch != '\n';
}

// ===========================================================================
bool DSWParser::findClose()
{
    tracker isOk(*this);
    while(isOk) {
	if (matchChars("}}}")) {
	    break;
	}
	string lineContents;
	parseSatisfying(lineContents, notNewline) || isOk.fail();
	if (isOk) {
	    lineBreak() || isOk.fail();
	}
    }
    return isOk;
}

// ===========================================================================
bool DSWParser::global()
{
    tracker isOk(*this);
    matchChars("Global:") || isOk.fail();
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	findClose() || isOk.fail();
    }
    if (isOk) {
	packageStuff() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	lineBreak() || isOk.fail();
    }
    if (isOk) {
	packageStuff() || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool notSpace(char ch)
{
    return !isspace(ch);
}

// ===========================================================================
bool DSWParser::version(string &verNum)
{
    tracker isOk(*this);
    matchChars("Microsoft Developer Studio Workspace File, Format Version ")
	|| isOk.fail();
    if (isOk) {
	parseSatisfying(verNum, notSpace) || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool DSWParser::warning()
{
  return matchChars("# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!");
}

// ===========================================================================
bool DSWParser::lineBreak()
{
    return parseChar('\n');
}

// ===========================================================================
// Parse a list of 0 or more occurrences of 'elem', delimited by 
// 1 or more occurrences of 'delim'. There must be one delimiter at the start,
// one at the end, and one between each pair of 'elem's.
//
bool DSWParser::listBy(bool (DSWParser::*delim)(), bool (DSWParser::*elem)())
{
    tracker isOk(*this);
    (this->*delim)() || isOk.fail();
    if (isOk) {
        for (;;) {
	    tracker moreOk(*this);
	    (this->*elem)() || moreOk.fail();
	    if (moreOk) {
		(this->*delim)() || moreOk.fail();
	    }
	    if (!moreOk) {
		// We had a good thing at the top of the loop.
		break;
	    }
	}
    }
    return isOk;
}

// ===========================================================================
// needs care to not be terribly inefficient,
// and not go past end of line, etc.
// This implementation stops when the lookahead is " - ".
//
bool DSWParser::fileName(string &fn)
{
    fn = "";
    for (;;) {
	const char *p = peek();
	char ch = *p;
	if (ch == '\0' || ch == '\n'
	    || (ch == ' ' && p[1] == '-' && p[2] == ' ')) {
	    break;
	}
	parseItem(ch);
	fn.append(1, ch);
    }
    return true;
}

// ===========================================================================
// Return whether the named file is a DSW file, not a DSP.
//
bool DSWParser::isDSW(const char *file_name) {
    size_t len = strlen(file_name);
    if (len < 4) {
	return false;
    }
    char buf[5];
    const char *suf = file_name + strlen(file_name) - 4;
    memmove(buf, suf, 5);
    for (int i = 1; i < 4; i += 1) {
	buf[i] = tolower(buf[i]);
    }
    return strcmp(buf, ".dsw") == 0;
}

// ==========================================================================
//
ProjectInfo::ProjectInfo() : cfg(NULL) {}

// ==========================================================================
//
ProjectInfo::ProjectInfo(const ProjectInfo &that)
 : cfg(that.cfg ? new string(*(that.cfg)) : NULL),
   project(that.project),
   file(that.file),
   wd(that.wd)
{
}

// ==========================================================================
//
ProjectInfo::~ProjectInfo() 
{
    delete cfg;
}

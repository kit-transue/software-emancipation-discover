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

#ifdef _WIN32
#pragma warning (disable:4786)
#endif

#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <sstream>
#else
#include <iostream.h>
#include <strstream.h>
#endif
#include <string>
#ifdef _WIN32
#include <direct.h>
#endif
#include "crparser.h"
#include "msg.h"
#include "parseline.h"
#include "RegExp.h"

#ifdef hp10
#include <unistd.h> // for getcwd()
#endif
#include "util.h"

namespace std {}
using namespace std;

// ===========================================================================
bool CRParser::parseCR(istream &input, const char *crFileName,
		       const char *compiler_list,
		       ostream &ostrm,
		       bool verbose,
		       bool allow_any_extension,
		       bool allow_any_flags,
	               string *forcedProjectName)
{
    CRParser crpar(crFileName, compiler_list, ostrm, verbose,
		   allow_any_extension, allow_any_flags, forcedProjectName);
    int line_no = 0;
    crpar.initLineState();
    bool ok = crpar.isValid();
    if (ok) {
	if (verbose) {
	    msg("Beginning to scan config record.", normal_sev) << eom;
	}
	for (;;) {
#ifdef ISO_CPP_HEADERS
	    stringstream line;
#else
	    ostrstream line;
#endif
	    input.get(*line.rdbuf(), '\n');
#ifdef ISO_CPP_HEADERS
	    if (input.eof() && line.str().length() == 0) {  
#else
	    if (input.eof() && line.pcount() == 0) {  
#endif
		break;
	    }
	    if (input.bad()) {
		ok = false;
		msg("Error while reading line $1 of config record.", catastrophe_sev)
		    << (line_no + 1) << eom;
		break;
	    }
	    line_no += 1;
	    input.clear();
	    input.get(); // get the newline
	    if (input.bad()) {
		ok = false;
		msg("Error while reading end of line $1 from config record.", catastrophe_sev)
		    << line_no << eom;
		break;
	    }
	    input.clear();

	    string line_str = line.str();
    	    ParseLine pline(line_str.c_str());
	    crpar.trackHRules(pline);
	    crpar.trackSection(pline);
    	    crpar.trackTarget(pline);
    	    if (crpar.inBuildRules) {
		crpar.processBuildRule(pline);
	    }
	    if (false) {
		msg("failed to parse line number $1", error_sev) << line_no << eoarg << eom;
		ok = false;
	    }
	}
	if (verbose) {
	    msg("Done scanning config record, $1 lines processed.", normal_sev)
                << line_no << eom;
	}
    }
    return ok;
}

// ===========================================================================
static string getSuffix(const string &str)
{
    string::size_type dot_loc = str.rfind('.');
    if (dot_loc != string::npos) {
	return str.substr(dot_loc + 1);
    }
    return "";
}

// ===========================================================================
static string unquote(const string &s)
{
    if (s.length() != 0) {
	char q = s.at(0);
	if (q == '\'' || q == '"') {
	    bool done = false;
	    string result;
	    string::size_type length = s.length();
	    string::size_type i = 1;
	    while (i < length) {
		char ch = s.at(i);
		if (ch == q) {
		    done = (i == length - 1);
		    break;
		}
		if (ch == '\\') {
		    i += 1;
		    if (i == length) {
			break;
		    }
		    ch = s.at(i);
		}
		result.append(1, ch);
		i += 1;
	    }
	    if (done) {
		return result;
	    }
	}
    }
    return s;
}

// ===========================================================================
CRParser::CRParser(const char * /* crFileName */, const char *compiler_list,
		   ostream &ostrm, bool verbose,
		   bool allow_any_extension, bool allow_any_flags,
		   string *forcedProjectName)
  : beVerbose(verbose),
    checkExtension(!allow_any_extension),
    checkFlags(!allow_any_flags),
    projectName(NULL),
    imposedProjectName(forcedProjectName),
    compilerIdentifier(compiler_list, verbose),
    targetCount(0),
    ostrm(ostrm)
{
}

// ===========================================================================
CRParser::~CRParser()
{
    delete projectName;
}

// ===========================================================================
// returns whether the given path name is absolute (rather than relative)
//
bool CRParser::isAbsolute(const char *name)
{
    return isPathSep(name[0]) 
	|| (isalnum(name[0]) && name[1] == ':' && isPathSep(name[2]));
}

// ===========================================================================
// Interprets the given file name relative to the directory.
// If the file name is absolute, it remains unchanged.
// If the file name is relative, it is interpreted relative to the given
// directory.  The directory name should end in a separator character,
// i.e. \ or /
//
void CRParser::appendToDirectory(const string &dir, string &file)
{
    if (!isAbsolute(file.c_str())) {
	// Find initial occurrences of .. or . in the file name, and
	// eliminate them.
	int dir_pos = dir.length() - 1; // the separator
	for (;;) {
	    if (file[0] == '.' && file[1] == '.' && isPathSep(file[2])) {
		char tester = dir[dir_pos - 1];
		if (!isPathSep(tester) && tester != '.' && 
					!(tester == ':' && dir_pos == 2)) {
		    // Here's a directory name to eliminate.
		    int pos;
		    for (pos = dir_pos - 1; pos >= 0; pos -= 1) {
			if (isPathSep(dir[pos])) {
			    break;
			}
		    }
		    if (pos >= 0) {
			// Directory name is located at pos+1 .. dir_pos-1.
			dir_pos = pos;
			file = file.substr(3);
		    }
		}
	    }
	    else if (file[0] == '.' && isPathSep(file[1])) {
		file = file.substr(2);
	    }
	    else {
		break;
	    }
	}
	string t = dir;
	t.resize(dir_pos + 1);
	t.append(file);
	file = t;
    }
}

// ===========================================================================
void CRParser::determineWd(string &wd, const char *crFileName)
{
    string cwd = getcwd(NULL, 0);
    // Normalize wd to end with separator, because more from
    // file name will be appended.
    string::size_type len = cwd.length();
    if (len == 0 || !isPathSep(cwd[len - 1])) {
        scanclrmake::appendNameInDir(cwd, "");
    }
    // Append the directory portion of the DSP file name.
    wd = crFileName;
    appendToDirectory(cwd, wd);
    // Remove the file portion.
    string::size_type psPos = wd.rfind('\\');
    string::size_type psPos2 = wd.rfind('/');
    if (psPos == string::npos || psPos2 != string::npos && psPos2 > psPos) {
	psPos = psPos2;
    }
    if (psPos != string::npos) {
	wd.resize(psPos + 1);
    }
}

// ===========================================================================
bool CRParser::isPathSep (char ch)
{
    return ch == '\\' || ch == '/';
}

// ===========================================================================
static void quoteString(string &str)
{
    string result = "\"";
    string::size_type len = str.length();
    for (int i = 0; i < len; i += 1) {
	char ch = str[i];
	if (ch == '\\' || ch == '"' || ch == '\'') {
	    result.append("\\");
	}
	result.append(1, ch);
    }
    result.append("\"");
    str = result;
}

// ===========================================================================
bool CRParser::isValid()
{
    return true;
}

// ===========================================================================
void CRParser::substituteEnvVars(string &s)
{
    const char *p = s.c_str();
    if (strchr(p, '%') != NULL) {
	string result;
	string var;
	bool in_var = false;
	for (;; p += 1) {
	    char ch = *p;
	    if (ch == '\0') {
		if (in_var) {
		    result.append("%");
		    result.append(var);
		}
		break;
	    }
	    if (in_var) {
		if (ch == '%') {
		    const char *val = getenv(var.c_str());
		    if (val == NULL) {
			result.append("%");
			result.append(var);
			result.append("%");
		    }
		    else {
			result.append(val);
		    }
		    in_var = false;
		}
		else {
		    var.append(1, ch);
		}
	    }
	    else {
		if (ch == '%') {
		    in_var = true;
		    var = "";
		}
		else {
		    result.append(1, ch);
		}
	    }
	}
	s = result;
    }
}

// ===========================================================================
void CRParser::initLineState()
{
    inBuildRules = false;
    hRuleCount = 0;
    prevHRuleCount = 0;
    currentSection = no_sct;
    targetName = "";
    firstTargetName = "";
    workingDirectory = "";
    lineCountInSection = 0;
}


// ===========================================================================
void CRParser::trackHRules(ParseLine &line)
{
    prevHRuleCount = hRuleCount;
    if (line.isRule()) {
    	hRuleCount += 1;
    }
    else {
        hRuleCount = 0;
    }
}

// ===========================================================================
// Update the currentSection, inBuildRules, and lineCountInSection state.
//
void CRParser::trackSection(ParseLine &line)
{
    if (currentSection != no_sct) {
        if (lineCountInSection > 0 && hRuleCount > 0) {
	    // current line is a rule, ending section
	    currentSection = no_sct;
	    lineCountInSection = 0;
	    inBuildRules = false;
	}
	else {
	    lineCountInSection += 1;
	}
    }
    if (currentSection == no_sct && hRuleCount == 0) {
	// An intro section may begin at the top of the file,
	// or after 3 horizontal rules.
	if (targetCount == 0 ? (prevHRuleCount == 0 || prevHRuleCount == 2) : prevHRuleCount == 3) {
	    currentSection = intro_sct;
            targetCount += 1;
	}
	else if (prevHRuleCount > 0) {
	    // Another section may begin after a rule.
	    Regexp obj_re("object");
	    Regexp env_re("ariable|ption");
	    Regexp bld_re("uild|cript");
	    const char *l = line.getLine().c_str();
	    if (obj_re.Match(l, strlen(l), 0) >= 0) {
		currentSection = obj_sct;
	    }
	    else if (env_re.Match(l, strlen(l), 0) >= 0) {
		currentSection = env_sct;
	    }
	    else if (bld_re.Match(l, strlen(l), 0) >= 0) {
		currentSection = bld_sct;
	    }
	    else {
		currentSection = misc_sct;
	    }
	}
    }
    if (hRuleCount == 0 && lineCountInSection > 0) {
	// line from body of section
	if (currentSection == bld_sct) {
    	    inBuildRules = true;
	}
    }
}

// ===========================================================================
// Track the targetName and workingDirectory.
//
void CRParser::trackTarget(ParseLine &line)
{
    if (currentSection == intro_sct) {
	const char *l = line.getLine().c_str();
	Regexp tgt("arget ");
	if (tgt.Match(l, strlen(l), 0) >= 0) {
	    Regexp tgt2(" built by ");
	    const char *l2 = l + tgt.EndOfMatch(0);
	    if (tgt2.Match(l2, strlen(l2), 0) >= 0) {
		string t(l2, tgt2.BeginningOfMatch(0));
		targetName = t;
		if (targetCount == 1) {
		    firstTargetName = t;
		}
	    }
	}
	// Initial working directory was /d00d/sun5-O/obj/aset_CCcc/CCcc/src
	Regexp wd_re("directory was ");
	if (wd_re.Match(l, strlen(l), 0) >= 0) {
	    workingDirectory = l + wd_re.EndOfMatch(0);
	}
    }
    if (currentSection == no_sct && hRuleCount >= 2) {
	targetName = "";
	workingDirectory = "";
    }
}

// ===========================================================================
// Make sure the string contains no tabs, substituting spaces.
//
static string &untab(string &str)
{
    if (/* strchr(str.c_str(), '\t') != NULL */ true) {
	string result;
	const char *p = str.c_str();
	char ch;
	while ((ch = *p) != '\0') {
	    if (ch == '\t') {
    	    	ch = ' ';
	    }
    	    result.append(1, ch);
	    p += 1;
	}
    	str = result;
    }
    return str;
}

// ==========================================================================
// The flags are OK if there's something there which looks like a compiler
// option.
//
static bool flagsOK(const string &options)
{
    Regexp flag("(^| )(-|/)(I|D|c|o)");
    return flag.Match(options.c_str(), options.size(), 0) >= 0;
}

// ==========================================================================
// The extension of the source file is OK if it's anything that we've ever
// heard is used with Code Rover.
//
static bool extOK(const string &ext)
{
    static Regexp exts(
          "^(c|h|cc|cxx|cpp|hxx|hpp|java|c\\+\\+|sql|trg|pkg|pc|pcc|ec|sf|spb|sps|sp|spp|fp|fpp|ff"
           "|C|H|CC|CXX|CPP|HXX|HPP|JAVA|C\\+\\+|SQL|TRG|PKG|PC|PCC|EC|SF|SPB|SPS|SP|SPP|FP|FPP|FF)$");
    return exts.Match(ext.c_str(), ext.size(), 0) >= 0;
}

// ==========================================================================
// The extension of the source file is OK if it's anything that we've ever
// heard is used with Code Rover.
//
bool CRParser::extensionOK(const string &file)
{
    // At the end of a filename, a . followed by an extension which
    // contains no dot, slash, or backslash.
    Regexp extx("\\.([^\\./\\\\]*)$");
    string ext;
    if (extx.Match(file.c_str(), file.size(), 0) >= 0) {
	int bom = extx.BeginningOfMatch(1);
	int len = extx.EndOfMatch(1) - bom;
	ext = file.substr(bom, len);
    }
    bool ok = extOK(ext);
    if (!ok && !checkExtension) {
	printf("allowing unrecognized extension \"%s\" in \"%s\".",
		ext.c_str(), file.c_str());
    }
    return ok || !checkExtension;
}

// ===========================================================================
// Get the last name out of the path.  The resulting name will contain
// no slashes or backslashes.
//
string fileOf(const string &path)
{
    // Will fail if there's no non-empty name.
    Regexp filex("(^|/|\\\\)([^/\\\\]+)[/\\\\]*$");
    string file = "proj";
    if (filex.Match(path.c_str(), path.size(), 0) >= 0) {
        int bom = filex.BeginningOfMatch(2);
	int len = filex.EndOfMatch(2) - bom;
	file = path.substr(bom, len);
    }
    return file;
}

// ===========================================================================
// Process a build rule.  If it's recognizable as a directive to compile
// source, put a line to standard out.
// 
void CRParser::processBuildRule(ParseLine &line)
{
    /* For example, 
     *  rm -f lib_aset_CCcc.o
     *  /usr/ccs/bin/ld -r -o lib_aset_CCcc.o c_gen_be.o SET_file_table.o || (rm -f lib_aset_CCcc.o; false)
     *  -@ rm -f cfe.o;
     *  /net/oxen/vol04/SUNWspro/SC5.0/bin/cc -Dsun5 -I/para /paraset/src/paraset/CCcc/src/cfe.c || (rm -f cfe.o; false)
     *  cd /d00d/sun5-O/bin/7.3.0.95; /net/oxen/vol04/SUNWspro/SC5.0/bin/CC -g -c /d00d/sun5-O/bin/7.3.0.95/aset_jfe_date.c; rm /d00d/sun5-O/bin/7.3.0.95/aset_jfe_date.c
     */
    // Get the compiler name.
    const char *l = line.getLine().c_str();
    // Match non-whitespace, non-separators, followed by a space.
    Regexp cmpl("(^| |\t|/|\\\\)[^ \t/\\\\]+( |\t)");
    string compiler_name;
    string compiler_path;
    int cmpl_len = cmpl.Match(l, strlen(l), 0);
    int cmpl_end = cmpl.EndOfMatch(0);
    int cmpl_start = cmpl.BeginningOfMatch(0);
    int opt_start = cmpl_end;
    if (cmpl_len >= 0) {
	cmpl_end -= 1; // Don't use the final space.
	if (cmpl_start < cmpl_end && strchr(" \t/\\", l[cmpl_start]) != NULL) {
    	    cmpl_start += 1; // Don't use the initial separator or space.
	}
	compiler_name = string(l + cmpl_start, cmpl_end - cmpl_start);
        compiler_path = string(l, cmpl_end);
	scanclrmake::trim(compiler_path);
    }

    // Get the source file name.  It may be surrounded by quotes.
    // const char *tail_re_str = " \\|\\|";
    Regexp tail_re("; | \\|\\|");
    int tail_matched = tail_re.Match(l, strlen(l), cmpl_end);
    int tail = tail_re.BeginningOfMatch(0);
    tail = tail_matched >= 0 ? tail : strlen(l);
    // Source is a quoted or non-whitespace chunk at the end of the line.
    Regexp src_re("(\"[^\"]*\"|[^ \t]+)[ \t]*$");
    string source_name = "";
    int opt_end = 0;
    if (cmpl_end <= tail) {
	if (src_re.Match(l, tail, cmpl_end) >= 0) {
	    int src_end = src_re.EndOfMatch(0);
	    int src_start = opt_end = src_re.BeginningOfMatch(0);
	    while (src_end > src_start && l[src_end - 1] == ' ' || l[src_end - 1] == '\t') {
		src_end -= 1;
	    }
	    if (src_end > src_start && l[src_end - 1] == '"') {
		src_end -= 1;
	    }
	    if (src_end > src_start && l[src_start] == '"') {
		src_start += 1;
	    }
	    source_name = string(l + src_start, src_end - src_start);
	    char lead = source_name.at(0);
	    // Bag out of any source name which is actually an option.
	    if (lead == '-'
#ifdef _WIN32
		            || lead == '/'
#endif
					  ) {
		source_name = "";
	    }
#ifdef _WIN32
	    // Handle a path which is neither absolute nor relative.
	    else if (lead == '/' || lead == '\\') {
		if (workingDirectory.length() >= 2
		    && workingDirectory.at(1) == ':') {
		    // Prepend the drive letter to the source name.
		    string abs = string(workingDirectory.c_str(), 2);
		    abs.append(source_name);
		    source_name = abs;
		}
	    }
#endif
	}
    }

    string parser_name;
    compilerIdentifier.id(compiler_name, compiler_path, parser_name);
    
    if (opt_end < opt_start) {
    	opt_end = opt_start;
    }
    string options(l + opt_start, opt_end - opt_start);

    if (!(parser_name == "" || source_name == ""
                            || (checkFlags && !flagsOK(options))
			    || !extensionOK(source_name))) {
	string firstTargetFileName = fileOf(firstTargetName);
	ostrm << untab(firstTargetFileName);  // Use the first target.
	ostrm << "\t";
	ostrm << untab(workingDirectory); // Use the workingDirectory.
	ostrm << "\t";
	ostrm << untab(source_name);      // Grab the last element before || or eol.
	ostrm << "\t";
	ostrm << untab(parser_name);
	ostrm << "\t";
	ostrm << untab(options);          // Everything in between.
	ostrm << endl;
    }
}

// ===========================================================================
// Track the host name.  Warn if it's not the same as the current host.
//
void CRParser::trackHost(ParseLine &line)
{
    if (currentSection == intro_sct) {
	const char *l = line.getLine().c_str();
	Regexp hst("^ *(H|h)ost (\".*\"|[^ \"]*)( |$)");
	if (hst.Match(l, strlen(l), 0) >= 0) {
	    int bom = hst.BeginningOfMatch(2);
	    const char *b = l + bom;
	    int len = hst.EndOfMatch(2) - bom;
            if (l[bom] == '"') {
	        b -= 1;
	        len -= 1;
	    }
	    string t(b, len);
    	    char *buf = new char[len + 30];
            scanclrmake::gethostname(buf, len + 29);
	    if (t != string(buf)) {
		msg("Host of ClearMake build, $1, differs from current host $2.\n",
                    warning_sev) << t << eoarg << buf << eoarg << eom;
	    }
	    delete [] buf;
	}
    }
}

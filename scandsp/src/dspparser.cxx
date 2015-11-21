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
#include <direct.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "dspparser.h"
#include "line.h"
#include "parseline.h"
#include "registry_if.h"
#include "msg.h"
#include "startprocess.h"

using namespace std;
using namespace MBDriver;

const char *compilation_units_message_group_id = NULL;

// ===========================================================================
bool DSPParser::parseDSP(istream &input, const char *dspFileName,
			 bool preprocess, ostream &ostrm, bool verbose,
	                 string *forcedProjectName,
			 string *cfg)
{
    // There's no need to try here to find an overriding CFG value in an OPT
    // file, as we do for DSWs, because overriding is not a property of DSPs.
    // A DSP can be referenced from various DSWs a guess at finding the DSW
    // and its overriding could be wrong or right.

    DSPParser dsppar(dspFileName, preprocess, ostrm, verbose, forcedProjectName, cfg);
    int line_no = 0;
    bool ok = dsppar.isValid();
    if (ok) {
	dsppar.checkIncludeDirs(dsppar.VCIncludeDirs, "the Visual C++ installation");
	for (;;) {
	    bool line_ok = true;
	    stringstream line;
	    input.get(*line.rdbuf(), '\n');
	    if (input.eof() && line.str().length() == 0) {  
		break;
	    }
	    if (input.bad()) {
		line_ok = false;
		ok = false;
		break;
	    }
	    line_no += 1;
	    input.clear();
	    input.get(); // get the newline
	    if (input.bad()) {
		line_ok = false;
		ok = false;
		break;
	    }
	    input.clear();
	    if (line_ok) {
		string line_str = line.str();
		Line result;
		ParseLine par(line_str.c_str());
		line_ok = par.parseLine(result);
		if (line_ok) {
		    dsppar.trackLine(result);
		}
		else {
		    msg("failed to parse line number $1", error_sev) << line_no << eoarg << eom;
		    ok = false;
		}
	    }
	}
	string fileIdent = "the project file ";
	fileIdent.append(dspFileName);
	dsppar.checkIncludeDirs(dsppar.DSPIncludeDirs, fileIdent);
        dsppar.checkOverridingCfg(fileIdent);
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
DSPParser::an_env_map &DSPParser::currentEnvironment()
{
    return *(environmentStack.top());
}

// ===========================================================================
// Extract from the given options string those which are mergable, i.e.
// /D and /I options.
//
static void mergableOptions(const string &opts, string &mrg, bool &hasNonMrg)
{
    hasNonMrg = false;
    const char *op = opts.c_str();
    const char *p = op;
    bool merging = true;
    int mergeOptionPhase = 0;
    const char *copied_so_far = op;
    mrg = "";
    for (;;) {
	const char ch = *p;
	bool ending = false;
	bool go_on_merging = merging;
	if (ch == '/') {
	    go_on_merging = p[1] == 'D' || p[1] == 'I';
	    ending = go_on_merging != merging;
	    if (go_on_merging) {
		mergeOptionPhase = 1;
	    }
	}
	else if (ch == '\0') {
	    ending = true;
	}
	else if (mergeOptionPhase = 1) {
	    // the D or I
	    mergeOptionPhase = 2;
	}
	else if (isspace(ch)) {
	    if (mergeOptionPhase == 2) {
		// whitespace following D or I
		mergeOptionPhase = 3;
	    }
	    if (mergeOptionPhase == 4) {
		// whitespace following file or definition
		mergeOptionPhase = 5;
	    }
	}
	else if (mergeOptionPhase == 2 || mergeOptionPhase == 3) {
	    // non-whitespace follows D or I
	    mergeOptionPhase = 4;
	}
	else if (mergeOptionPhase == 5) {
	    // non-whitespace follows complete mergable option
	    go_on_merging = false;
	    ending = (go_on_merging != merging);
	}
	if (ending) {
	    if (merging) {
		for (const char *cp = copied_so_far; cp < p; cp += 1) {
		    mrg.append(1, *cp);
		}
	    }
	    else {
		if (p > copied_so_far) {
		    hasNonMrg = true;
		}
		copied_so_far = p;
	    }
	    merging = go_on_merging;
	    if (ch == '\0') {
		break;
	    }
	}
	// Advance to next character, skipping quoted stuff.
	p += 1;
	if (ch == '\'' || ch == '"') {
	    for (;;) {
		const char ch2 = *p;
		if (ch2 == '\0') {
		    break;
		}
		p += 1;
		if (ch2 == ch) {
		    break;
		}
	    }
	}
    }
}

// ===========================================================================
static void updateEnvironment(DSPParser::an_env_map &env, const string &key, const string &value) {
    DSPParser::an_env_map::iterator it = env.find(key);
    if (it != env.end()) {
	env.erase(it);
    }
    env.insert(DSPParser::an_env_map::value_type(key, value));
}

#if 0
// ===========================================================================
// Process the # ADD statement, which at least in some cases adds includes.
// # ADD is not yet completely understood.  From an OnDisplay example, it's
// known not to simply replace the previous value.  Another example led me
// to wait before seeking to combine the old with the new, but it has to be
// attempted.
//
static void addToEnvironment(DSPParser::an_env_map &env, const string &key, const string &value) {
    string updateVal = value;
    if (key == ";CPP") {
	DSPParser::an_env_map::iterator it = env.find(key);
	if (it != env.end()) {
	    updateVal = (*it).second;
	    ostream &mos = msg("Updating from $1 >>>>>>>>>>>>>>> $2", normal_sev) << updateVal << eoarg;
	    string mergable;
	    bool hasNonmergable;
	    mergableOptions(value, mergable, hasNonmergable);
	    if (hasNonmergable) {
		string oldMergable;
		bool oldHasNonmergable;
		mergableOptions(updateVal, oldMergable, oldHasNonmergable);
		// Prepend oldMergable options.
		updateVal = oldMergable;
	    }
	    // Append all new options.
	    updateVal.append(" ");
	    updateVal.append(value);
	    mos << updateVal << eoarg << eom;
	}
    }
    updateEnvironment(env, key, updateVal);
}
#else
// ===========================================================================
// Process the # ADD statement, which at least in some cases adds includes.
// # ADD is not yet completely understood.  From an OnDisplay example, it's
// known not to simply replace the previous value.  Another example led me
// to wait before seeking to combine the old with the new, but it has to be
// attempted.
//
// Now a couple of Foxboro examples in which the kind of merging attempted
// above is not appropriate, because the updates should just be added to the
// original.  I may have been misled on the OnDisplay, by another bug in 
// preprocessing, causing too many # ADD's to be interpreted.
//
void DSPParser::addToEnvironment(DSPParser::an_env_map &env, const string &key, const string &value) {
    string updateVal = value;
    if (key == ";CPP") {
	noteOptionIncludeDirs(value);
	DSPParser::an_env_map::iterator it = env.find(key);
	if (it != env.end()) {
	    updateVal = (*it).second;
	    ostream &mos = msg("Updating from $1 >>>>>>>>>>>>>>> $2", normal_sev) << updateVal << eoarg;
	    // Append all new options.
	    updateVal.append(" ");
	    updateVal.append(value);
	    mos << updateVal << eoarg << eom;
	}
    }
    updateEnvironment(env, key, updateVal);
}
#endif

// ===========================================================================
void DSPParser::noteOptionIncludeDirs(const string &opts)
{
    const char *op = opts.c_str();
    const char *p = op;
    int mergeOptionPhase = 0;
    string mrg;
    for (;;) {
	const char ch = *p;
	if (ch == '/') {
	    if (p[1] == 'I') {
		mergeOptionPhase = 1;
	    }
	}
	else if (mergeOptionPhase = 1) {
	    // the I
	    mergeOptionPhase = 2;
	}
	else if (isspace(ch) || ch == '\0') {
	    if (mergeOptionPhase == 2) {
		// whitespace following I
		mergeOptionPhase = 3;
	    }
	    if (mergeOptionPhase == 4) {
		// whitespace following directory
		DSPIncludeDirs.insert(mrg);
		mergeOptionPhase = 5;
	    }
	}
	else if (mergeOptionPhase == 2 || mergeOptionPhase == 3) {
	    // non-whitespace follows I
	    mergeOptionPhase = 4;
	    mrg = "";
	}
	else if (mergeOptionPhase == 5) {
	    // non-whitespace follows complete mergable option
	}
	if (mergeOptionPhase == 4) {
	    mrg.append(1, ch);
	}
	if (ch == '\0') {
	    break;
	}
	// Advance to next character, skipping quoted stuff.
	p += 1;
	if (ch == '\'' || ch == '"') {
	    for (;;) {
		const char ch2 = *p;
		if (ch2 == '\0') {
		    break;
		}
		p += 1;
		if (ch2 == ch) {
		    break;
		}
	    }
	}
    }
}

// ===========================================================================
void DSPParser::noteVCIncludeDir(const std::string &dir)
{
    VCIncludeDirs.insert(dir);
}

// ===========================================================================
// Determine whether 'name' refers to an extant directory.
static bool directory_exists(const string &name)
{
    struct _stat s;
    int ret = _stat(name.c_str(), &s);
    return (ret == 0 && (s.st_mode & _S_IFDIR));
}

// ===========================================================================
// Checks whether the include directories referenced by the VC++ include dir
// environment all exist, and provides a diagnostic if they don't.
//
void DSPParser::checkIncludeDirs(std::set<std::string> &dirs, const std::string &origin)
{
    a_str_set::iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it) {
	const string &includeDir = *it;
	// Determine whether that name refers to an extant directory.
	if (!directory_exists(includeDir)) {
	    msg("The directory $1, referenced by $2, does not exist.", warning_sev)
	         << includeDir << eoarg << origin << eoarg << eom;
	}
    }
}

// ===========================================================================
// Checks whether the overriding cfg setting was valid.
//
void DSPParser::checkOverridingCfg(const string &file)
{
    if (cfg != NULL && !cfgValidated) {
	msg("The configuration \"$1\" was not found in $2.", warning_sev)
            << *cfg << eoarg << file << eoarg << eom;
    }
    if (cfgUsed == NULL) {
	msg("No CFG used with $1.", warning_sev) << file << eoarg << eom;
    }
    else if (cfgUsed == cfg) {
	msg("Used CFG \"$1\" from workspace, for $2.", normal_sev)
	    << *cfg << eoarg << file << eoarg << eom;
    }
    else {
	msg("Used CFG \"$1\" as defined in $2.", normal_sev)
	    << *cfgUsed << eoarg << file << eoarg << eom;
    }
}

// ===========================================================================
string DSPParser::currentCPPOptions()
{
    return environmentLookup(addKey("CPP"));
}

// ===========================================================================
string DSPParser::environmentLookup(const string &key)
{
    an_env_map::iterator it = currentEnvironment().find(key);
    if (it != currentEnvironment().end()) {
	return (*it).second;
    }
    return "";
}

// ===========================================================================
// Determine an environment key for use with # ADD lines.
// The semicolon keeps these values separate from the values defined in, 
// e.g. CPP=cl.exe
//
string DSPParser::addKey(const string &baseKey)
{
    string result = ";";
    result.append(baseKey);
    return result;
}

// ===========================================================================
// Determine an environment key for use with # PROP lines.
// The comma keeps these values separate from the values defined in, 
// e.g. CPP=cl.exe
//
string DSPParser::propKey(const string &baseKey)
{
    string result = ",";
    result.append(baseKey);
    return result;
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
		if (ch == '\\' && s.at(i+1) == q && i+1 < length) {
		    i += 1;
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
string DSPParser::getAndEvalString(const Line &line)
{
    string value;
    string linestr = line.getString();
    eval(value, linestr);
    return value;
}

// ===========================================================================
static void cvttoupper(string &str)
{
    string::size_type l = str.length();
    string result;
    for (string::size_type i = 0; i < l; i += 1) {
	result.append(1, toupper(str.at(i)));
    }
    str = result;
}

// ===========================================================================
// Look up the boolean value of a key in the current environment.
//
bool DSPParser::lookupBool(string key, bool dflt)
{
    an_env_map::iterator it;
    it = currentEnvironment().find(key);
    bool result = dflt;
    if (it != currentEnvironment().end()) {
	string val = unquote((*it).second);
	cvttoupper(val);
	result = !(val == "0" || val == "NO" || val == "FALSE");
    }
    return result;
}

// ===========================================================================
// Find a CFG setting in the message string of a DSP file, e.g.
// !MESSAGE Possible choices for configuration are:
// !MESSAGE 
// !MESSAGE "atl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
//
static bool hasCfgString(string message, string &cfg_value)
{
    string::size_type p;
    p = message.find_first_not_of(" \t");
    bool ok = (p != string::npos);
    if (ok) {
	ok = (message.at(p) == '"');
    }
    string::size_type q;
    if (ok) {
	q = message.find('"', p+1);
	ok = (q != string::npos);
    }
    if (ok) {
	cfg_value = message.substr(p+1, q-(p+1));
    }
    return ok;
}

// ===========================================================================
// Return whether the message string is all whitespace.
//
static bool isBlank(string message)
{
    string::size_type p;
    p = message.find_first_not_of(" \t");
    return p == string::npos;
}

// ===========================================================================
// Track whether we're within a Begin Group "Source Files".
// When we get to End Source File, emit the line for the source file.
//
void DSPParser::trackLine(const Line &line)
{
    if (preprocessLine(line)) {
	Line::Kind kind = line.kind();
	const Entity &entity = line.entity();
	if (kind == Line::Begin) {
	    beginEndLevel += 1;
	    an_env_map &ol = *environmentStack.top();
	    environmentStack.push(new an_env_map(*(environmentStack.top())));
	    an_env_map &ne = *environmentStack.top();
	    if (entity.kind() == Entity::Group || entity.kind() == Entity::GroupName) {
		group g;
		g.hasName = entity.kind() == Entity::GroupName;
		if (g.hasName) {
		    g.name = entity.string();
		}
		groupStack.push(g);
	    }
	}
	else if (kind == Line::End && beginEndLevel > 0) {
	    if (entity.kind() == Entity::Group || entity.kind() == Entity::GroupName) {
		groupStack.pop();
	    }
	    if (entity.kind() == Entity::SourceFile) {
		string key = "SOURCE";
		an_env_map::iterator source_it;
		source_it = currentEnvironment().find(key);

		static const string exclude_key
                	= propKey("Exclude_From_Build");
		bool exclude = lookupBool(exclude_key, false);
		if (exclude) {
		    // Exclude this source file from build.
		}
		else if (source_it != currentEnvironment().end()) {
		    string file = unquote((*source_it).second);
		    string suffix = getSuffix(file);
		    string::iterator it;
		    for (it = suffix.begin(); it != suffix.end(); ++it) {
			*it = tolower(*it);
		    }
		    bool lang_is_c_plus_plus = (suffix == "cpp" || suffix == "cxx");
		    bool lang_is_c = (suffix == "c");
		    bool lang_is_java = (suffix == "java");
		    bool suffix_is_source = (lang_is_c_plus_plus || lang_is_c);
		    bool suffix_is_java_source = lang_is_java;
		    bool suffix_is_header = (suffix == "h" || suffix == "hpp" || suffix == "hxx");

		    bool group_absent = (groupStack.empty() || !groupStack.top().hasName);
		    bool group_is_source = false;
		    bool group_is_java_source = false;
		    bool group_is_header = false;
		    string group_id = "none";
		    if (!group_absent) {
			group_id = groupStack.top().name;
			if (group_id == "\"Source Files\"") {
			    group_is_source = true;
			}
			else if (group_id == "\"Java Source Files\"") {
			    group_is_java_source = true;
			}
			else if (group_id == "\"Header Files\"") {
			    group_is_header = true;
			}
		    }
		    else if (!groupStack.empty()) {
			group_id = "nameless";
		    }
		    string parser = "";
		    if (suffix_is_source) {
			parser = "ntcl()";
		    }
		    if (suffix_is_java_source) {
			parser = "ntjvc";
		    }
		    if (suffix_is_header) {
			parser = "ntclh";
		    }
		    
		    if (suffix_is_source || suffix_is_header || suffix_is_java_source) {
			outStream << ((imposedProjectName != NULL) ? imposedProjectName->c_str()
                                : ((projectName != NULL) ? unquote(*projectName) : ""));
			outStream << "\t";
			outStream << wd;
			outStream << "\t";
			filterSpuriousDotDots(wd, file);
			outStream << file;
			outStream << "\t";

			AutoMessageGroup *parent_mgid = NULL;
			AutoMessageGroup *child_mgid = NULL;
			if (compilation_units_message_group_id != NULL) {
			    parent_mgid = new AutoMessageGroup(AutoMessageGroup::id_str_to_num(compilation_units_message_group_id));
			    ostringstream msg_group_name;
			    msg_group_name << "parse file " << file;
			    child_mgid = new AutoMessageGroup(msg_group_name.str().c_str());
			    msg("Parsing for: Source File in $1", normal_sev) << dspFileName << eoarg << eom;
			    outStream << AutoMessageGroup::current();
			    outStream << "\t";
			}

			outStream << parser;
			outStream << "\t";
			if (lang_is_c || lang_is_c_plus_plus) {
			    outStream << currentCPPOptions();
			    outStream << " ";
			    outStream << vcIncludes;
			}
			if (lang_is_c_plus_plus) {
			    /* VC++ creates a .tlh file for each #import, in the
			     * same directory as it puts the object file. 
			     * This will normally be controlled by the Output_Dir property.
			     * Aset_CCcc needs to be told where to look. */
			    static const string outdir_key
				    = propKey("Output_Dir");
			    string outdir = environmentLookup(outdir_key);
			    if (outdir.length() > 0) {
				ostringstream msg_group_name;
				msg_group_name << "outdir location of preprocessed #imports" ;
				if (compilation_units_message_group_id == NULL) {
				    msg_group_name << " " << file;
				}
				AutoMessageGroup outdir_message_group(msg_group_name.str().c_str());
				msg("Outdir is $1.", normal_sev) << outdir << eoarg << eom;
				outdir = unquote(outdir);
				msg("Unquoted outdir is $1.", normal_sev) << outdir << eoarg << eom;
				filterSpuriousDotDots(wd, outdir);
				msg("Filtered outdir is $1.", normal_sev) << outdir << eoarg << eom;
				string outdir_path = outdir;
				appendToDirectory(wd, outdir_path);
				if (directory_exists(outdir_path)) {
				    list<string> import_args;
				    import_args.push_back("--import_dir");
				    import_args.push_back(outdir);
				    string import_string = prepare_command_arguments(import_args);
				    outStream << " " << import_string;
				}
				else {
				    msg("Output_Dir $1 does not exist.", error_sev)
					<< outdir_path << eoarg << eom;
				    if (projectName != NULL) {
					msg("Project $1 must be built using the IDE prior to starting DIScover.",
					        warning_sev)
					    << *projectName << eoarg << eom;
				    }
				}
			    }
			}
			outStream << endl;
			delete child_mgid;
			delete parent_mgid;
		    }
		    else {
			if (beVerbose) {
			    msg("Failed to digest source file $1 within group $2.", normal_sev)
				<< file << eoarg << group_id << eoarg << eom;
			}
		    }
		}
	    }
	    delete environmentStack.top();
	    environmentStack.pop();
	    beginEndLevel -= 1;
	}
	else if (kind == Line::Add && !line.getBool()) {
	    addToEnvironment(currentEnvironment(), addKey(line.key()), getAndEvalString(line));
	}
	else if (kind == Line::Prop && !line.getBool()) {
	    addToEnvironment(currentEnvironment(), propKey(line.key()), getAndEvalString(line));
	}
	else if (kind == Line::Project) {
	    const string *ln = line.getName();
	    if (ln != NULL) {
		delete projectName;
		projectName = new string(*ln);
	    }
	}
	else if (kind == Line::Message) {
	    // cfgState of 0   -> nothing interesting
            //             1   -> choices header seen
            //             -1  -> previous line had a cfg identifier
            //             n>1 -> number of blank lines seen, since header, is n-1
	    string messageString = line.getString();
	    string cfgString;
	    if (cfg == NULL) {
	    }
	    else if (cfgState == 0
                && messageString.find("choices for configuration are:") != string::npos) {
		cfgState = 1;
	    }
	    else if ((cfgState == -1 || (0 < cfgState && cfgState-1 < 2))
                     && hasCfgString(messageString, cfgString)) {
		cvttoupper(cfgString);
		string overridingCfg = *cfg;
		cvttoupper(overridingCfg);
		if (cfgString == overridingCfg) {
		    cfgValidated = true;
#if 0
		    msg("cfgValidated", normal_sev) << eom;
#endif
		}
#if 0
		else {
		    msg("Not cfgValidated \"$1\" vs. \"$2\".", normal_sev)
			<< cfgString << eoarg << overridingCfg << eoarg << eom;
		}
#endif
		cfgState = -1;
	    }
	    else if (cfgState > 0 && isBlank(messageString)) {
	        cfgState += 1;
	    }
	    else {
		cfgState = 0;
	    }
	}
#if 0
	outStream << "line: ";
	line.show(outStream);
	outStream << endl;
#endif
    }
}

// ===========================================================================
// Handle !IF, !ELSEIF, !ENDIF, and variable definitions.
// Returns 'true' if the line has not been consumed by preprocessing.
//
bool DSPParser::preprocessLine(const Line &line)
{
    bool is_preprocessing_line = false;
    Line::Kind kind = line.kind();
    if (doPreprocessing) {
	switch (kind) {
	case Line::If:
	case Line::Elseif:
	case Line::Else:
	case Line::Endif:
	case Line::Definition:
	    is_preprocessing_line = true;
	}
    }
    if (is_preprocessing_line) {
	switch (kind) {
	case Line::If:
	    {
		ifState newif;
		if (active) {
		    newif.active = active = eval(line.tokens());
		    newif.testing = !active;
		}
		else {
		    newif.active = false;
		    newif.testing = false;
		}
		ifStack.push(newif);
	    }
	    break;
	case Line::Elseif:
	    {
		if (!ifStack.empty()) {
		    ifState &state = ifStack.top();
		    if (state.testing) {
			state.active = active = eval(line.tokens());
			if (active) {
			    state.testing = false;
			}
		    }
		    else {
			state.active = active = false;
		    }
		}
	    }
	    break;
	case Line::Else:
	    {
		if (!ifStack.empty()) {
		    ifState &state = ifStack.top();
		    if (state.testing) {
			state.active = active = true;
			state.testing = false;
		    }
		    else {
			state.active = active = false;
		    }
		}
	    }
	    break;
	case Line::Endif:
	    {
		if (!ifStack.empty()) {
		    ifStack.pop();
		    if (ifStack.empty()) {
			active = true;
		    }
		    else {
			active = ifStack.top().active;
		    }
		}
	    }
	    break;
	case Line::Definition:
	    {
		if (active) {
		    updateEnvironment(currentEnvironment(), line.key(), getAndEvalString(line));
		}
	    }
	    break;
	default:
	    break;
	}
    }
    return !is_preprocessing_line && active;
}

const char DSPParser::pathSep = '\\';

// ===========================================================================
DSPParser::DSPParser(const char *dspFileName, bool preprocess, ostream &ostrm, 
		     bool verbose, string *forcedProjectName, string *cfg)
  : doPreprocessing(preprocess),
    outStream(ostrm),
    beVerbose(verbose),
    active(true),
    beginEndLevel(0),
    projectName(NULL),
    foundVC(false),
    imposedProjectName(forcedProjectName),
    cfgValidated(false),
    cfgState(0),
    cfgUsed(NULL),
    cfg(cfg),
    dspFileName(dspFileName)
{
    environmentStack.push(new an_env_map());
    determineWd(dspFileName);
    determineVCIncludes();
}

// ===========================================================================
DSPParser::~DSPParser()
{
    delete projectName;
}

// ===========================================================================
// Returns whether the given path name is absolute (rather than
// directory-relative).  A Windows drive-relative path name is considered
// absolute.
//
bool DSPParser::isAbsolute(const char *name)
{
    return isPathSep(name[0]) 
	|| (isalnum(name[0]) && name[1] == ':' && isPathSep(name[2]));
}

// ===========================================================================
// Interprets the given file name relative to the directory,
// yielding an absolute (i.e. local-machine-relative) path.
// If the file name is absolute, it remains unchanged.
// If the file name is relative, it is interpreted relative to the given
// directory.  The directory name should end in a separator character,
// i.e. \
// If the file name is drive-relative, e.g. \Applications\spacewar.exe,
// prepends the drive letter.
//
void DSPParser::appendToDirectory(const string &dir, string &file)
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
		    for (int pos = dir_pos - 1; pos >= 0; pos -= 1) {
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
#ifdef _WIN32
    else if (isPathSep(file.at(0))) {
	// File name is drive-relative, not really absolute.
	if (dir.length() >= 2 && isalnum(dir.at(0)) && dir.at(1) == ':') {
	    // Directory has a drive letter.
	    file.insert(0 /* insertion point */, dir, 0 /* reference point */,
						      2 /* num of chars */);
	}
    }
#endif
}

// ===========================================================================
// static member function
// Visual Studio is permissive regarding the ocurrence of \..\..
// in source file names.  If the current directory is the root, then
// .. also refers to the root.
//
// This function elminates such anomalous use of .. from a file name,
// so that it can be used readily during the model build.  The 'dir'
// parameter is the absolute pathname of the directory within which
// the name 'file' is interpreted.
//
void DSPParser::filterSpuriousDotDots(const string &dir, string &file)
{
    if (!isAbsolute(file.c_str()) && isAbsolute(dir.c_str())) {
	// Count the number of nested directory names in dir.
        int dir_nesting = 0;
	bool at_separator = false;
	int dir_len = dir.length();
	for (int i = 0; i < dir_len; i += 1) {
	    char ch = dir.at(i);
	    if (isPathSep(ch)) {
		at_separator = true;
	    }
	    else if (at_separator) {
		at_separator = false;
		dir_nesting += 1;
	    }
	}
	
	// Now go through 'file', tracking the directory nesting
	// and eliminating occurrences of .. which go too far.
	at_separator = true;
	int index = 0;
	while (index < file.length()) {
	    if (at_separator && file.length() > index + 3
		&& file.at(index) == '.'
		&& file.at(index + 1) == '.'
	        && isPathSep(file.at(index + 2))) {
		if (dir_nesting == 0) {
		    file.erase(index, 3);
		}
		else {
		    dir_nesting -= 1;
		    index += 3;
		}
	    }
	    else if (at_separator && file.length() > index + 2
		&& file.at(index) == '.'
	        && isPathSep(file.at(index + 1))) {
	        index += 2;
	    }
	    else {
		if (isPathSep(file.at(index))) {
		    at_separator = true;
		}
		else if (at_separator) {
		    at_separator = false;
		    dir_nesting += 1;
		}
		index += 1;
	    }
	}		
    }
}

// ===========================================================================
void DSPParser::determineWd(string &wd, const char *dspFileName)
{
    string cwd = getcwd(NULL, 0);
    // Normalize wd to end with separator, because more from
    // file name will be appended.
    string::size_type len = cwd.length();
    if (len == 0 || !isPathSep(cwd[len - 1])) {
	cwd.append(1, pathSep);
    }
    // Append the directory portion of the DSP file name.
    wd = dspFileName;
    appendToDirectory(cwd, wd);
    // Remove the file portion.
    string::size_type psPos = wd.rfind(pathSep);
    string::size_type psPos2 = wd.rfind('/');
    if (psPos == string::npos || psPos2 != string::npos && psPos2 > psPos) {
	psPos = psPos2;
    }
    if (psPos != string::npos) {
	wd.resize(psPos + 1);
    }
}

// ===========================================================================
void DSPParser::determineWd(const char *dspFileName)
{
    determineWd(wd, dspFileName);
}

// ===========================================================================
bool DSPParser::isPathSep (char ch)
{
    return ch == pathSep || ch == '/';
}

// ===========================================================================
// Evaluate an expression within an If or Elsif.
// This function probably omits much of what Microsoft supports,
// but it has worked fine in many, many cases.
// Returns a simple bool.
//
bool DSPParser::eval(const list<Token> &toks)
{
    string left;
    string right;
    string junk;
    int eq_count = 0;
    bool result = false;
    bool have_whitespace = false;
    list<Token>::const_iterator it;
    for (it = toks.begin(); it != toks.end(); ++it) {
	const Token &tok = *it;
	if (tok.kind() == Token::Delimiter && tok.string() == "==") {
	    eq_count += 1;
	}
	else {
	    if (tok.kind() == Token::Whitespace) {
		have_whitespace = true;
	    }
	    else if (tok.kind() != Token::NoKind) {
		string &appendee = eq_count == 0 ? left : eq_count == 1 ? right : junk;
		if (have_whitespace) {
		    if (appendee.length() > 0) {
			appendee.append(" ");
		    }
		    have_whitespace = false;
		}
		appendee.append(tok.string());
	    }
	}
    }
    if (eq_count == 1) {
	string left_value;
	eval(left_value, left);
        string stripped_left_value;
	charStrip('"', stripped_left_value, left_value);
	string right_value;
	eval(right_value, right);
        string stripped_right_value;
	charStrip('"', stripped_right_value, right_value);
	// Compare, ignoring case.
	cvttoupper(stripped_left_value);
	cvttoupper(stripped_right_value);
	result = (stripped_left_value == stripped_right_value);
    }
    return result;
}

// ===========================================================================
// Evaluate the str_in, substituting for
// variable references such as $(FOO) based on the current environment.
// Used only within the expressions of If and Elsif.
//
void DSPParser::eval(string &result, const string &str_in)
{
    result = "";
    const char *p = str_in.c_str();
    while (*p != '\0') {
	const char ch = *p;
	if (ch == '$' && p[1] == '(') {
	    string key;
	    const char *pc = p + 2;
	    while (*pc != ')' && *pc != '\0') {
		key.append(1, *pc);
		pc += 1;
	    }
	    an_env_map::iterator it;
	    if (cfgValidated && cfg != NULL && key == "CFG") {
		// There's an override on the CFG setting.
		result.append(*cfg);
		if (cfgUsed == NULL) {
		    cfgUsed = cfg;
		}
	    }
	    else if ((it = currentEnvironment().find(key)) != currentEnvironment().end()) {
		result.append((*it).second);
		if (cfgUsed == NULL && key == "CFG") {
		    cfgUsed = new string((*it).second);
		}
	    }
	    else {
		const char *val = getenv(key.c_str());
		if (val != NULL) {
		    result.append(val);
		}
	    }
	    p = pc;
	}
	else {
	    result.append(1, ch);
	}
	if (*p != '\0') {
	    p += 1;
	}
    }
}

// ===========================================================================
// Eliminate by deleting the char from str_in
// Used only within the expressions of If and Elsif.
//
void DSPParser::charStrip(char strip_ch, string &result, const string &str_in)
{
    result = "";
    const char *p = str_in.c_str();
    while (*p != '\0') {
	const char ch = *p;
	if (ch != strip_ch) {
	    result.append(1, ch);
	}
	p += 1;
    }
}

// ===========================================================================
// Get the value of the VC++ INCLUDE variable, and translate it into
// the needed command line options.
//
void DSPParser::determineVCIncludes()
{
    foundVC = analyzeVCIncludes(vcIncludes, false, this);
}

// ===========================================================================
// Get the value of the VC++ INCLUDE variable, and translate it into
// the needed command line options.
//
bool DSPParser::analyzeVCIncludes(string &cmd_options, bool VC_syntax, DSPParser *parser)
{
    bool foundVC = false;
    const char *key_beginning = "HKEY_CURRENT_USER\\Software\\Microsoft";
    const char *vers[] = {"\\DevStudio\\7.0", 
			  "\\DevStudio\\6.0",
			  "\\DevStudio\\5.0", 
			  "\\Developer",
			  NULL}; 
    const char *key_end = "\\Build System\\Components\\Platforms\\Win32 (x86)\\Directories\\Include Dirs";

    string incvar;
    for (int n = 0;; n += 1) {
	// Form key, concatenating 3 parts.
	string key = key_beginning;
	const char *v = vers[n];
	if (v == NULL) {
	    break;
	}
	key.append(v);
	key.append(key_end);

	// Look up key in registry.
	if (registry::exists(key)) {
	    incvar = registry::at(key);
	    foundVC = true;
	    break;
	}
    }
    if (!foundVC) {
	msg("Microsoft VC++ not found in registry.", catastrophe_sev) << eom;
    }

    substituteEnvVars(incvar);
    list<string> vcInclude_list;
    string::size_type len = incvar.length();
    bool within_option = false;
    string option;
    for (string::size_type i = 0;; i += 1) {
	if (i == len || incvar[i] == ';') {
	    if (within_option) {
		// substituteEnvVars(option);
		dropExtraPrefix(option);
		if (parser != NULL) {
		    parser->noteVCIncludeDir(option);
		}
		if (VC_syntax) {
		    option.insert(0, "/I");
		}
		else {
		    option.insert(0, "-I");
		}
		vcInclude_list.push_back(option);
		option = "";
	    }
	    within_option = false;
	    if (i == len) {
		break;
	    }
	}
	else if (!within_option && isspace(incvar[i])) {
	}
	else {
	    option.append(1, incvar[i]);
	    within_option = true;
	}
    }
    cmd_options = prepare_command_arguments(vcInclude_list);
    if (VC_syntax) {
	// A leading quote can be tricky to handle later on.
	// Replace "/I with /I".
	string::size_type pos;
	while ((pos = cmd_options.find("\"/I")) != string::npos) {
	    cmd_options.replace(pos, 3, "/I\"");
	}
    }
    return foundVC;
}

// ===========================================================================
// Get the installation location of MSVC.
//
bool DSPParser::analyzeVCDir(string &msvc_dir)
{
    bool foundVC = false;
    const char *key_beginning = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft";
    const char *vers[] = {"\\DevStudio\\7.0", 
			  "\\DevStudio\\6.0",
			  "\\DevStudio\\5.0", 
			  "\\Developer",
			  NULL}; 
    const char *key_end = "\\Products\\Microsoft Visual C++\\ProductDir";

    for (int n = 0;; n += 1) {
	// Form key, concatenating 3 parts.
	string key = key_beginning;
	const char *v = vers[n];
	if (v == NULL) {
	    break;
	}
	key.append(v);
	key.append(key_end);

	// Look up key in registry.
	if (registry::exists(key)) {
	    msvc_dir = registry::at(key);
	    foundVC = true;
	    break;
	}
    }
    if (!foundVC) {
	msg("Microsoft VC++ not found in registry.", catastrophe_sev) << eom;
    }
    return foundVC;
}

// ===========================================================================
bool DSPParser::isValid()
{
    return foundVC;
}

typedef struct var_name_list {
    string *var_name;
    struct var_name_list *next;
} var_name_list;

static bool is_on_list(string s, var_name_list *l)
{
    while (l != NULL) {
	string us = s;
	cvttoupper(us);
	string uv = *(l->var_name);
	cvttoupper(uv);
        if (us == uv) {
	    return true;
	}
        l = l->next;
    }
    return false;
}

// ===========================================================================
// Substitutes in s based on the user's environment variables.
// The argument 'sup' is a list of environment variables to suppress,
// because expansion is recursive, but we want to prevent runaways.
//
void DSPParser::substituteEnvVars(string &s, void *sup)
{
    var_name_list *suppress = (var_name_list *)sup;
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
                    if (is_on_list(var, suppress)) {
		    }
		    else {
			const char *val = getenv(var.c_str());
			if (val == NULL) {
			    result.append("%");
			    result.append(var);
			    result.append("%");
			}
			else {
			    // Substitute some more, in case the
			    // expansion refers to other env vars.
			    string v(val);
			    var_name_list to_suppress = { &var, suppress };
			    substituteEnvVars(v, &to_suppress);
			    result.append(v);
			}
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
// Scan the string containing a file path for an extra path prefix, 
// e.g. C:\WINNT\Profiles\sturner.003\Desktop\F:\devstudio\vc\include
// becomes f:\devstudio\vc\include.
//
void DSPParser::dropExtraPrefix(string &s)
{
    // Scan the contents of s for the last drive letter prefix,
    // preceded by \ 
    const char *p = s.c_str();
    const char *found = NULL;
    for (p = s.c_str(); *p != '\0'; p += 1) {
	if (p[0] == '\\' && p[1] != '\0' && p[2] == ':') {
	    found = p + 1;
	}
    }

    if (found != NULL) {
	// Drop everything up to 'found'.
	string temp = found;
	s = temp;
    }
}

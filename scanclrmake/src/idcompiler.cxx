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

#include "idcompiler.h"
#include "RegExp.h"
#include "startprocess.h"
#include "util.h"
#include "stdlib.h"
#include "msg.h"

namespace std {}
using namespace std;
using namespace MBDriver;

CompilerIdentifier::CompilerIdentifier(const char *compiler_list_prefs,
				       bool verbose)
: compilerListPrefs(compiler_list_prefs),
  beVerbose(verbose)
{
}

// ==========================================================================
// Check cache for value.
//
bool CompilerIdentifier::checkCache(const string &compiler_path,
			            string &id)
{
    CacheT::iterator iter;
    int n = 0;
    for (iter = cache.begin(); iter != cache.end(); ++iter, ++n) {
	if (iter->first == compiler_path) {
	    id = iter->second;
	    if (n > 0) {
		// Move the entry to the front of the vector, so as not
		// to lose it prematurely.
		cache.erase(iter);
		cache.insert(cache.begin(), make_pair(compiler_path, id));
	    }
	    return true;
	}
    }
    return false;
}

// ==========================================================================
// Retain value in cache.
//
void CompilerIdentifier::cacheId(const string &compiler_path,
			            const string &id)
{
    if (cache.size() >= 6) {
	cache.pop_back();
    }
    cache.insert(cache.begin(), make_pair(compiler_path, id));
}

// ==========================================================================
// Check for user override of compiler identification in the compiler
// list prefs.
// Return true if the compiler is identified via overriding.
// If so, id is set with the name of the compiler.
//
bool CompilerIdentifier::userOverride(const string &compiler_name,
			    string &id)
{
    // Translate the compiler_name to a parser, e.g. "ntcl".
    // The compiler list has the form:
    // <comp1>:<par1>/<comp2>:<par2>/.../<compn>:</parn>
    // Use the found compiler name to search the compiler list which came from
    // the command line.
    string cmpl_pat = "(^|/)";
    cmpl_pat.append(compiler_name);
    cmpl_pat.append(":");
    Regexp cmpl_re(cmpl_pat.c_str());
    string parser_name = "";
    bool have_parser = false;
    if (   compilerListPrefs
        && cmpl_re.Match(compilerListPrefs, strlen(compilerListPrefs), 0) >= 0) {
	int parser_start = cmpl_re.EndOfMatch(0); // the colon
	const char *p = strchr(compilerListPrefs + parser_start, '/');
	int parser_end = (p == NULL ? strlen(compilerListPrefs) : p - compilerListPrefs);
	parser_name = string(compilerListPrefs + parser_start, parser_end - parser_start);
	have_parser = true;
    }
    id = parser_name;
    return have_parser;
}

// ==========================================================================
// If s contains a newline, truncate it prior to that character.
//
static void one_line(string &s)
{
    string::size_type n = s.find('\n');
    if (n != string::npos) {
	s.erase(n);
    }
}

// ==========================================================================
// From the path and name taken from the config record, check the existence
// of the file it refers to, returning whether it exists, and if it exists
// set actual_path to the path where it can be invoked.
//
static bool findCompiler(const string &compiler_path,
                         const string &compiler_name,
                         string &actual_path)
{
    if (scanclrmake::executableExists(compiler_path)) {
	actual_path = compiler_path;
    	return true;
    }
#if 0
    This had a problem.  The output of the 'which' command was going to the tty,
    rather than being captured by the pipe.  Also, the reliance on sh would have been
    tenuous.

    // /usr/bin/sh -c 'which "cc"'
    const char *argv[] = { 0, 0, 0 };
    argv[0] = "-c";
    string argv1 = "which \"";
    argv1.append(compiler_name);
    argv1.append("\"");
    argv[1] = argv1.c_str();
    if (start_process_and_wait("/usr/bin/sh", argv, &actual_path) == 0) {
        msg("sh handled $1 $2, yielding $3", normal_sev) << argv[0] << eoarg
           << argv[1] << eoarg << actual_path << eoarg << eom;
        for (int k = 0; k < actual_path.length(); k += 1) {
	    msg("actual_path[$1] = '$2'", normal_sev) << k << eoarg << actual_path.at(k) << eoarg << eom;
	}
        one_line(actual_path);
        scanclrmake::trim(actual_path);
        msg("sh handled $1 $2, yielding $3", normal_sev) << argv[0] << eoarg
           << argv[1] << eoarg << actual_path << eoarg << eom;
	if (scanclrmake::executableExists(actual_path)) {
            msg("findCompiler returning $1", normal_sev) << actual_path << eoarg << eom;
	}
	return scanclrmake::executableExists(actual_path);
    }
#endif
    const char *p = getenv("PATH");
#ifdef _WIN32
    const char *delim = ";";
#else
    const char *delim = ":";
#endif
    if (p != NULL) {
	string search_path = p;
	string::size_type start = 0;
	string::size_type end;
	string::size_type limit = search_path.length();
	for (;;) {
	    end = search_path.find(delim, start);
	    if (end == string::npos) {
		end = limit;
	    }

	    // Have a directory to search in; check for the file.
    	    string path = search_path.substr(start, end - start);
	    scanclrmake::appendNameInDir(path, compiler_name);
	    if (scanclrmake::executableExists(path)) {
		actual_path = path;
		return true;
	    }
#ifdef _WIN32
	    path.append(".exe");
	    if (scanclrmake::executableExists(path)) {
		actual_path = path;
		return true;
	    }
#endif

	    if (end == limit) {
		break;
	    }
	    start = end + 1;
	}
    }
    return false;
}

// ==========================================================================
// Invoke a script to determine whether the given compiler path name is gcc.
// If so, return true and set id as per the script, i.e. to "gcc".
//
static bool tryGcc (const string &actual_path, string &id, bool verbose)
{
    // Form the command filename and args for the Perl script.
    string plcmd = scanclrmake::psethome();
    scanclrmake::appendNameInDir(plcmd, "bin");
    scanclrmake::appendNameInDir(plcmd, "disperl");
    string tryGccScript = scanclrmake::psethome();
    scanclrmake::appendNameInDir(tryGccScript, "mb");
    scanclrmake::appendNameInDir(tryGccScript, "trygcc.pl");
    static const char *argv[] = { 
        NULL, // script path
	scanclrmake::platformName(),
	NULL, // actual path
	NULL };
    argv[0] = tryGccScript.c_str();
    argv[2] = actual_path.c_str();

    int result = start_process_and_wait(plcmd.c_str(), argv, &id);
    if (result == 0) {
	one_line(id);
	scanclrmake::trim(id);
	if (id.length() > 0) {
	    return true;
	}
    }
    if (verbose) {
	// Hard to tell catastrophic failure from a simple case
	// of not encountering an actual compiler.
	msg("trygcc command failed with result $5: $1 $2 $3 $4\n", warning_sev)
            << plcmd << eoarg << tryGccScript << eoarg
            << scanclrmake::platformName() << eoarg << actual_path << eoarg
	    << result << eoarg << eom;
    }
    return false;
}

// ==========================================================================
// Invoke a script to determine what compiler is represented by the given
// compiler path name (already determined not to be gcc).  This takes into
// account the current platform. If it is a known compiler, returns true
// and sets id as per the script, i.e. with the name of the compiler used
// for configuration purposes, e.g. suncc.
//
static bool tryOther(const string &actual_path, string &id, bool verbose)
{
    // Form the command filename and args for the Perl script.
    string plcmd = scanclrmake::psethome();
    scanclrmake::appendNameInDir(plcmd, "bin");
    scanclrmake::appendNameInDir(plcmd, "disperl");
    string tryCompilerScript = scanclrmake::psethome();
    scanclrmake::appendNameInDir(tryCompilerScript, "mb");
    scanclrmake::appendNameInDir(tryCompilerScript, "trycompiler.pl");
    static const char *argv[] = { 
        NULL, // script path
	scanclrmake::platformName(),
	NULL, // actual path
	NULL };
    argv[0] = tryCompilerScript.c_str();
    argv[2] = actual_path.c_str();

    int result = start_process_and_wait(plcmd.c_str(), argv, &id);
    if (result == 0) {
	one_line(id);
	scanclrmake::trim(id);
	if (id.length() > 0) {
	    return true;
        }
    }
    if (verbose) {
	// Hard to tell catastrophic failure from a simple case
	// of not encountering an actual compiler.
	msg("trycompiler command failed with result $5: $1 $2 $3 $4\n", warning_sev)
            << plcmd << eoarg << tryCompilerScript << eoarg
            << scanclrmake::platformName() << eoarg << actual_path << eoarg
            << result << eoarg << eom;
    }
    return false;
}

// ==========================================================================
// Invoke a script for last-ditch identification of the compiler which
// appears in the config record by the given name, but which cannot be
// found on disk.  This takes into account the current platform.  If a
// guess is possible, returns true and sets id as per the script,
// i.e. with the name of the compiler used for configuration purposes,
// e.g. suncc.
//
static bool guessCompiler(const string &compiler_name,
                          string &id, bool verbose)
{
    // Form the command filename and args for the Perl script.
    string plcmd = scanclrmake::psethome();
    scanclrmake::appendNameInDir(plcmd, "bin");
    scanclrmake::appendNameInDir(plcmd, "disperl");
    string compilerIdScript = scanclrmake::psethome();
    scanclrmake::appendNameInDir(compilerIdScript, "mb");
    scanclrmake::appendNameInDir(compilerIdScript, "compilerid.pl");
    static const char *argv[] = { 
        NULL, // script path
	scanclrmake::platformName(),
	NULL, // compiler name
	NULL };
    argv[0] = compilerIdScript.c_str();
    argv[2] = compiler_name.c_str();

    int result = start_process_and_wait(plcmd.c_str(), argv, &id);
    if (result == 0) {
	one_line(id);
	scanclrmake::trim(id);
	if (id.length() > 0) {
	    return true;
	}
    }
    if (verbose) {
	// Hard to tell catastrophic failure from a simple case
	// of not encountering an actual compiler.
	msg("compilerid command failed with result $5: $1 $2 $3 $4\n", warning_sev)
	    << plcmd << eoarg << compilerIdScript << eoarg
	    << scanclrmake::platformName() << eoarg << compiler_name << eoarg
	    << result << eoarg << eom;
    }
    return false;
}

// ==========================================================================
// Return true if the compiler is identified.  If so, id is set with
// the name of the compiler used for configuration purposes, e.g. ntcl.
//
bool CompilerIdentifier::id(const string &compiler_name,
			    const string &compiler_path,
			    string &id)
{
    bool have_parser = false;
    string actual_path;
    if (checkCache(compiler_path, id)) {
	return !(id == "");
    }
    else if (compiler_path == "-@" || compiler_path == "") {
    }
    else if (userOverride(compiler_name, id)) {
	have_parser = true;
    }
    else if (findCompiler(compiler_path, compiler_name, actual_path)) {
	if (   tryGcc(actual_path, id, beVerbose)
            || tryOther(actual_path, id, beVerbose)) {
	    have_parser = true;
	}
    }
    else if (guessCompiler(compiler_name, id, beVerbose)) {
	have_parser = true;
    }
    if (have_parser) {
	cacheId(compiler_path, id);
    }
    else {
	cacheId(compiler_path, string(""));
    }
    return have_parser;
}

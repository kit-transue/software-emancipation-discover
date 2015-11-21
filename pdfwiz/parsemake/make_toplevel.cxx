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
// make_toplevel.cxx
// 17.Dec.97 Kit Transue
#pragma warning(disable:4786)

#include <string>
#include <map>
#include <cassert>
#include <iostream>
#include <fstream>
#include <stack>
#include <strstream>

#include "message.h"

#include "make_toplevel.h"
#include "pipebuf.h"
#include "mscondexpr.h"
#include "filetypes.h"
#include "filename.h"
#include "variabletable.h"
#include "vartable_chain.h"
#include "special_macros_table.h"
#include "macro_expander.h"
#include "option_filter.h"
#include "setech_utils.h"
#include "file_table.h"
#include "env_table.h"
#include "macrotable.h"
#include "pdfwiz_settings.h"


using namespace std;

#undef yyFlexLexer
#define yyFlexLexer yyMakeFlexLexer
#include <FlexLexer.h>

#undef yyFlexLexer
#define yyFlexLexer yyPreprocessFlexLexer
#include <FlexLexer.h>


// class member initialization
makefile * makefile::globalmakefile = NULL;


// constructors, destructors, and copy constructors
makefile::makefile(commandline const &cl, directory_vector const &dv) :
	cwds(dv),
	fname(cl.makefilename()),
	progress_currentchar(0), progress_totalchars(1)
{
	// all this is to set up the variable table:

	vartable_chain *vtc = new vartable_chain;
	symbols = vtc;

	// commandline defines override all others and cannot be written:
	variabletable * cldefs = new macrotable(false);
	deque<pair<string, macro_node> >::const_iterator i = cl.defines.begin();
	while (i != cl.defines.end()) {
		cldefs->insert(i->first, i->second, true);
		++i;
	}
	vtc->push_table(cldefs);

	// next go internal macros and environment; order depends on /E option:
	envtable = new env_table;
	if (cl.optionEnvVarOverride()) {
		vtc->push_table(envtable);
		vtc->push_table(new macrotable);
	}
	else {
		vtc->push_table(new macrotable);
		vtc->push_table(envtable);
	}

	// finally, builtin macros (like "CL")...
	if (!cl.optionNoBuiltinInference()) {
		string builtins(setech_utils::psethome());
		builtins += "/lib/builtin_nmake_macros";
		vtc->push_table(new file_table(builtins));
	}
	// ...and special macros ($*)
	specialtable = new special_macros_table;
	vtc->push_table(specialtable);
}

makefile::~makefile()
{
	target_map_type::iterator i = target_set.begin();
	while (i != target_set.end()) {
		delete i->second;
		++i;
	}
	delete symbols;
}

void
makefile::read_file()
{
	// if fname is absolute, this won't do anything untoward:
	string const & fullname = cwds.relative_to_absolute(fname);
	// but now we know fullname is cannonical, and ok to pass as a filename:
	// Not even sure if this is necessary!
	cwds.cd(filename(fullname).dirname());

	// open
	fstream input(fullname.c_str(), ios::in);
	if (!input.good()) {
		msgid("Error opening makefile") << fullname << eom;
	}
	else {
		// read file into pipe
		pipebuf pb;
		iostream strm(&pb);
		progress_totalchars = 0;
		while (input.good()) {
			input.get(pb);
			if (!input.eof()) {
				if (input.fail())  // empty line
					input.clear();
				assert(input.peek() == '\n');
				input.ignore(); // consume '\n'
				strm << '\n';
				++progress_totalchars;
			}
		}
		input.close();
		read_file(&strm);
	}
}

void
makefile::read_file(istream * input)
{
	pipebuf preprocessed_buf;
	iostream preprocessed_pipe(&preprocessed_buf);
	yyPreprocessFlexLexer preprocessor(input, &preprocessed_pipe);
	yyMakeFlexLexer makeLexer(&preprocessed_pipe, &cout);

	// as necessary:
	//makeLexer.set_debug(1);

	stack<istream *> inputstack;
	inputstack.push(input);
	while (!inputstack.empty()) {
		preprocessor.switch_streams(inputstack.top(), &preprocessed_pipe);
		while (preprocessor.yylex()) {
			// update status:
			progress_currentchar = preprocessor.lineno();

			int status = makeLexer.yylex();
			switch (status) {
				case 0:
					msgid("Unexepected EOF reading input file") << eom;
					break;
				case 1:
					// all is OK
					break;
				case 2:
					// include file specified.
					{
					ifstream * newstream = new ifstream(newincludename.c_str());
					if (newstream->is_open()) {
						inputstack.push(newstream);
						preprocessor.switch_streams(inputstack.top(), &preprocessed_pipe);
					}
					else {
						msgid("couldn't open include file:") << newincludename << eom;
					}
					break;
					}
				default:
					msgid("Internal error: makeLexer returned unexpected status ") << status << eom;
					break;
			}
		}
		if (inputstack.top() != input)
			delete inputstack.top();
		inputstack.pop();
	}
}


// modification

make_target *
makefile::add_atomic_target(string const &targetname)
{
	target_map_type::iterator t = target_set.find(targetname);
	if (t == target_set.end()) {
		make_target * newtarget = new make_target(targetname, cwds, *envtable);
		target_set.insert(
			target_map_type::value_type(targetname, newtarget)
		);
		t = target_set.find(targetname);
		assert (t != target_set.end());
	}
	return t->second;
}

void
makefile::add_target(string const &filename, bool is_suffix)
{
	// reset the current target:
	current_targets.clear();

	if (is_suffix) {
		current_targets.insert(add_atomic_target(filename));
	}
	else {
		bool have_default_target = !default_targets.empty();
		// expand macros:
		string newstring(global_macroexpander->expand_macros_in_string(filename, symbols));
		// expand {path}filename syntax:
		dependpath::stringset *targets =
			dependpath::split_into_paths(newstring.c_str(), cwds);
		//assert(!targets->empty());
		if (targets->empty()) {
			msgid("Error: no target for dependent") << filename << eom;
		}
		dependpath::stringset::iterator target;
		for (target = targets->begin(); target != targets->end(); ++target) {
			make_target *newmt = add_atomic_target(*target);
			newmt->cwd = cwds;
			current_targets.insert(newmt);
			if (!have_default_target)
				default_targets.insert(newmt->name());
		}
	}
}

void
makefile::add_suffixrule(string const &rule)
{
	// need to remove paths from syntax like .cxx{$(OBJPATH)}.obj::
	// this is a hack for now
	string nopathrule(rule);
	size_t lbrace = rule.find_first_of("{");
	size_t rbrace = rule.find_last_of("}");
	if (lbrace != string::npos && rbrace != string::npos && lbrace < rbrace) {
		nopathrule.erase(lbrace, rbrace - lbrace + 1);
	}

	size_t pos = nopathrule.find_first_of(" :"); // colons and spaces should be OK in suffixrule
	if (pos != string::npos)
		nopathrule.erase(pos, string::npos);
	add_target(nopathrule, true);
}

void
makefile::add_action(string const &s)
{
	//assert(!current_targets.empty());

	for (set<make_target *>::iterator current_target = current_targets.begin();
			current_target != current_targets.end();
			++current_target) {
		// tell the symbol table what target to expand to:
		specialtable->set_target(*current_target);

		// check to see if this is a !action:
		if (s[0] == '!') { // should really check for $** or $<, also:
			// do once for each dependent:
			set<string> const &depends = (*current_target)->dependents();
			for (set<string>::const_iterator i = depends.begin();
					i != depends.end();
					++i) {
				specialtable->set_depend(&*i);
				// expand macros in string:
				string &action = global_macroexpander->expand_macros_in_string(s, symbols);
				add_action(action.substr(1));  // be sure to strip leading !, or we'll recurse forever
			}
			specialtable->clear_depend();
		}
		else {
			// expand macros
			string &action = global_macroexpander->expand_macros_in_string(s, symbols);
			// add dependent to target
			(*current_target)->add_action(action);
		}
	}
}


void
makefile::add_includefile(string const & filename)
{
	// figure out new filename, add to whatever bookkeeping structures,
	// and set newincludename so it may be used to open the include file
	// when the lexer returns for the next line.


	// expand variables prior to processing
	string s(global_macroexpander->expand_macros_in_string(filename, symbols));

	// remove leading "include " or "!INCLUDE ":
	string::size_type start = s.find_first_of(" \t");
	string::size_type pos = s.find_first_not_of(" \t", start);
	s.erase(0,pos);

	// this is ugly, but appears to be the way nmake does this crap:
	// remove all quotes:
	pos = 0;
	while ((pos = s.find('"', pos)) != string::npos) {
		s.erase(pos,1);
	}

	// strip off all trailing whitespace:
	if ((pos = s.find_last_not_of(" \t\n")) != string::npos) {
		s.erase(pos + 1);
	}

	// only use INCLUDE for angle braces if first and last chars are braces.
	bool use_path = (s[0] == '<' && s[s.length() - 1] == '>');
	if (use_path) {
		// remove angle braces
		s.erase(0, 1);
		s.erase(s.length() - 1);
	}

	if (use_path) {
		string path = (symbols->lookup("INCLUDE")).value;
		s = dependpath::mySearchPath(path.c_str(), s, cwds);
	}
	else {
		s = cwds.relative_to_absolute(s);
	}
	newincludename = s;
}


void
makefile::add_dependent(string const &dependfile)
{
	assert(!current_targets.empty());
	set<make_target *>::iterator current_target;
	for (current_target = current_targets.begin(); current_target != current_targets.end(); ++current_target) {
		// add dependent to target, keeping track of dependent string
		string const * dfptr =
			&(*current_target)->add_dependent(dependfile);
		// and maintain reverse index:
		filetype const &type = global_filetypes.what_type(*dfptr);
		if (type.kind == filetype::CPPSOURCE
				|| type.kind == filetype::CSOURCE) {
			rindex.insert(rindex_type::value_type(dfptr, *current_target));
		}
	}
}

void
makefile::add_dependents(char const *yytext)
{
	string expanded(global_macroexpander->expand_macros_in_string(string(yytext), symbols));
	stringset *s = dependpath::split_into_paths(expanded.c_str(), cwds);
	dependpath::stringset::iterator i = s->begin();
	while (i != s->end()) {
		add_dependent(*i);
		++i;
	}
}

// accessor and reporting functions

string const &
makefile::name() const
{
	return fname;
}

void
makefile::dump() const
{
	cout << "Contents of makefile " << fname << endl;
	cout << "Targets:" << endl;
	target_map_type::const_iterator i = target_set.begin();
	while (i != target_set.end()) {
		cout << i->first << endl;
		vector<action> const & actvec = i->second->actions();
		vector<action>::const_iterator j = actvec.begin();
		while (j != actvec.end()) {
			cout << "    action: " << j->commandline << endl;
			++j;
		}
		make_target::stringpset const & depset = i->second->dependents();
		make_target::stringpset::const_iterator k = depset.begin();
		while (k != depset.end()) {
			cout << "    dependent: " << *k << endl;
			++k;
		}
		++i;
	}
	cout << "Default target(s):";
	set<string>::const_iterator ssi = default_targets.begin();
	while (ssi != default_targets.end()) {
		cout << " " << *ssi;
		++ssi;
	}
	cout << endl;
}

makefile::stringset *
makefile::list_all_depends(string const &target) const
{
	stringset * ret = new stringset;
	target_map_type::const_iterator firsttarget = target_set.find(target);
	if (firsttarget != target_set.end()) {
		// add all targets that are subtargets of this:
		make_target::stringpset const &dependentlist = firsttarget->second->dependents();
		make_target::stringpset::const_iterator i = dependentlist.begin();
		while (i != dependentlist.end()) {
			stringset * additionaldeps = list_all_depends(*i);
			stringset::iterator j = additionaldeps->begin();
			// last_inserted makes set union linear time
			stringset::iterator last_inserted = ret->begin();
			while (j != additionaldeps->end()) {
				last_inserted = ret->insert(last_inserted, *j);
				++j;
			}
			delete additionaldeps;
			++i;
		}
	}
	ret->insert(target);
	return ret;
}

makefile::stringset *
makefile::list_target_depends(string const &target) const
{
	// returns list of dependents for this target
	stringset * ret = new stringset;
	target_map_type::const_iterator firsttarget = target_set.find(target);
	if (firsttarget != target_set.end()) {
		// add all targets that are subtargets of this:
		make_target::stringpset const &dependentlist = firsttarget->second->dependents();
		make_target::stringpset::const_iterator i = dependentlist.begin();
		while (i != dependentlist.end()) {
			//if (target_set.find(*i) != target_set.end()) {
				//ret->insert(cwds.relative_to_absolute(*i));
				ret->insert(*i);
			//}
			++i;
		}
	}
	return ret;
}

set<action> *
makefile::list_makeactions(string const &target) const
{
	set<action> * ret = new set<action>;
	target_map_type::const_iterator firsttarget = target_set.find(target);
	if (firsttarget != target_set.end()) {
		// add actions that are make actions:
		// examine firsttarget->actions()
		{
			vector<action> const &actions = firsttarget->second->actions();
			vector<action>::const_iterator i = actions.begin();
			string const &makecommands = global_macroexpander->expand_macros_in_string(settings::make_aliases(), symbols);
			while (i != actions.end()) {
				if (filename::is_command(i->commandline, makecommands))
					ret->insert(*i);
				++i;
			}
		}


#if 0
		// then add all actions resulting from subtargets:
		make_target::stringpset const &dependentlist = firsttarget->second->dependents();
		make_target::stringpset::const_iterator i = dependentlist.begin();
		while (i != dependentlist.end()) {
			stringset * additionalactions = list_all_makeactions(*i);
			stringset::iterator j = additionalactions->begin();
			// last_inserted makes set union linear time
			stringset::iterator last_inserted = ret->begin();
			while (j != additionalactions->end()) {
				last_inserted = ret->insert(last_inserted, *j);
				++j;
			}
			delete additionalactions;
			++i;
		}
#endif


	}
	return ret;
}


makefile::stringset *
makefile::list_all_targets() const
{
	stringset * ret = new stringset;
	target_map_type::const_iterator i = target_set.begin();
	// because this is an ordered insert, we can make more efficient by introducing last_insert_at iterator
	while(i != target_set.end()) {
		// don't include suffixrules:
		if (!i->first.empty() && i->first[0] != '.')
			ret->insert(i->first);
		++i;
	}
	return ret;
}

makefile::stringset const *
makefile::list_default_targets() const
{
	return &default_targets;
}
	

static string
semis2minusEyes(string const &s)
{
	string ret(s);

	if (!ret.empty()) {
		string::size_type pos;

		// cleanup trailing semicolons and whitespace:
		pos = ret.find_last_not_of(" \t;");
		if (pos < ret.length() - 1)
			ret.erase(pos + 1);

		// not the most efficient way of doing this, but:
		// remove all empty paths:
		while ((pos = ret.find(";;")) != string::npos) {
			ret.erase(pos, 1);
		}
		
		ret.insert(0, "-I\"");
		for (pos = ret.find(';'); pos != string::npos; pos = ret.find(';')) {
			ret.replace(pos, 1, "\" -I\"");
		}
		ret += '\"';
	}
	return ret;
}


pair<string,string>
makefile::compile_line(string const &sourcefile) const
{
	// Returns strings for working directory and compile action.

	// there are a couple of possibilities to consider here:
	// a compile action is defined for the target associated with source
	// a target is defined, but suffix rules used to compile action
	// no target defined, suffix rules used

	pair<string,string> ret;
	string const * includepath = 0;

	rindex_type::const_iterator i = rindex.find(&sourcefile);
	if (i != rindex.end()) {
		action const *ap = i->second->compile_action();
		if (ap) {
			// expand INCLUDES
			ret.first = ap->dv.directory();
			ret.second = ap->commandline;
			includepath = &(ap->env.lookup("INCLUDE").value);
		}
	}
	if (ret.second.empty()) {
		// try suffix rule
		// I'm lazy at the moment; try and build an object asap:
		string::size_type pos = sourcefile.find_last_of(".");
		assert(pos != string::npos);  // sourcefile had to have had suffix
		string suffixrule(sourcefile, pos, string::npos);
		suffixrule += ".obj";
		target_map_type::const_iterator ruleptr = target_set.find(suffixrule);
		if (ruleptr != target_set.end()) {
			action const *ap = ruleptr->second->compile_action();
			if (ap) {
				// I don't know what's right for the environment
				// and the working directory here....
				ret.first = ap->dv.directory();
				ret.second = ap->commandline;
				// expand INCLUDES
				//cerr << "used suffixrule on " << sourcefile << endl;
			}
		}
		// for all suffix rules, there is a case where built-in suffix rules used
	}
	// fallback position:
	if (ret.second.empty()) {
		msgid("Failed to get flags for file; using default") << sourcefile << eom;
		static string const defaultline("cl.exe defaultarguments");
		ret.second = defaultline;
	}

	// finally, add the include path to the end of the string
	if (!includepath) {
		includepath = &(symbols->lookup("INCLUDE")).value;
	}
	if (!includepath->empty()) {
		ret.second += ' ';
		ret.second += semis2minusEyes(*includepath);
	}
	return ret;
}

string
makefile::member_as_absolute(string const &file) const
{
	return cwds.relative_to_absolute(file);
}

string const &
makefile::flags_db_version() const
{
	static string const db_version_id("#v2");
	return db_version_id;
}

void
makefile::write_flags_database(ostream * os, string const * compiler /*=0*/) const
{
	rindex_type::const_iterator i = rindex.begin();
	pipebuf pb;
	iostream pipe(&pb);

	while (i != rindex.end()) {
		// first the filename
		*os << '\"' <<
			i->second->cwd.relative_to_absolute(i->first->c_str())
			<< "\": ";

		// this should be the file type:
		// but we do nothing

		pair<string,string> compileline = compile_line(*(i->first));
		// add the working directory:
		*os << '\"' << compileline.first << "\" ";

		// force compiler name if specified
		if(compiler) {
			*os << *compiler << " ";
		}

		// write the compile action:
		// filter the compile action through the option filter...
		pipe << compileline.second;
		option_filter_main(pipe, *os);
		// (...and into the output stream)
		// the option filter sometimes doesn't eat all its input:
		pipe.get(*os->rdbuf());

		*os << endl;
		pipe.clear();
		++i;
	}
}

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
/* parser for makefiles */


/* Input from the makefile is processed in three layers:
	a preprocessing layer that expands macros (using lex)
	a standard makefile parser that identifies rules/actions (using lex)
	a parser for microsoft extensions (using a lexer and yacc)

	Lines are processed one at a time.  Data are fed from one lexer
	to the next by using a strstream like a pipe between each.  The
	first two lexers are called sequentially; the second calls
	yacc if necessary to parse the extensions; yacc in turn uses
	the last lexer to get its input.
*/

#pragma warning(disable:4786)
#pragma warning(disable:4503)

#include <iostream>
#include <ios>
#include <queue>
#include "make_toplevel.h"
#include "variabletable.h"

#include "message_manager.h"

#include "registry_if.h"
#include "prefsfile.h"
#include "filename.h"
#include "generate_pdf.h"
#include "ide_interface.h"
#include "make2prefs.h"

#include "new_make2proj.h"
#include "action.h"
#include "commandline.h"


int main(int argc, char **argv)
{
	bool debug_mode = false;
	directory_vector cwd;
	string projectname;
	string admindir;

	ide_interface *ide = ide_interface::get_platform_interface();
	string lastproj = *ide->last_projects().begin();
	delete ide;
	cout << lastproj << endl;

	// create list of actions:
	queue<action> pendingmakefiles;
	env_table environment;
	string actionline("nmake");
	for (int i = 1; i < argc; ++i) {
		// decide if we're ready to make new argument:
		if (!stricmp(argv[i], "/next")) {
			pendingmakefiles.push(action(actionline, cwd, environment));
			// reset the action:
			actionline = "nmake";
		}
		else if (!stricmp(argv[i], "--debug")) {
			++i;
			debug_mode = true;
		}
		else if (!stricmp(argv[i], "--projectname")) {
			++i;
			if (i >= argc) {
				msgid("--projectname requires an argument");
			}
			else {
				projectname = argv[i];
			}
		}
		else if (!stricmp(argv[i], "--admindir")) {
			++i;
			if (i >= argc) {
				msgid("--admindir requires an argument");
			}
			else {
				admindir = argv[i];
			}
		}
		else {
			bool quote_string = strchr(argv[i], ' ');
			// expand the action
			actionline += ' ';
			if (quote_string) actionline += '\"';
			actionline += argv[i];
			if (quote_string) actionline += '\"';
		}
	}
	// pushing this string/cwd/environment is what should happen in gui
	pendingmakefiles.push(action(actionline, cwd, environment));


	// this processing is what building the pdf is about
	prefsroot prefs;
	while (!pendingmakefiles.empty()) {
		action a(pendingmakefiles.front());
		pendingmakefiles.pop();
		commandline c(a.commandline);
		makefile::globalmakefile = new makefile(c, a.dv);
		makefile &tmf = *makefile::globalmakefile;
		prefs.addMakefile(tmf);
		tmf.read_file(); 
		if (debug_mode) {
			tmf.symbols->dump();
			tmf.dump();
		}

		set<string> const *targetset = &c.target();
		if (targetset->empty())
			targetset = tmf.list_default_targets();
		set<string>::const_iterator target;
		for (target = targetset->begin(); target != targetset->end(); ++target) {
			add_mftarget2graph(makefile::globalmakefile, *target);
			// grab all actions generated by this target:
			set<string> *alldeps = tmf.list_all_depends(*target);
			set<string>::iterator i = alldeps->begin();
			while (i != alldeps->end()) {
				set<action> *ap = tmf.list_makeactions(*i);
				set<action>::iterator ai = ap->begin();
				while (ai != ap->end()) {
					pendingmakefiles.push(*ai);
					++ai;
				}
				delete ap;
				++i;
			}
			delete alldeps;
		}
	}

	figure_clusters();
	if (debug_mode) {
		print_graph();
	}
	set<subproj *> &tmp = graph2pdf();

	set<subproj *>::iterator j;
	for (j = tmp.begin(); j != tmp.end(); ++j) {
		prefs.addSubProject(*j);
	}

	if (projectname.empty()) {
		projectname = prefs.suggestedProjectName();
	}
	prefs.setProjectName(projectname);

	if (admindir.empty()) {
		admindir = prefs.adminDir();
	}
	prefs.setAdminDir(admindir);

	prefs.write();
	//write_scoping_rules(&cout);

	delete makefile::globalmakefile;
	message_manager::shutdown();
	return 0;
}


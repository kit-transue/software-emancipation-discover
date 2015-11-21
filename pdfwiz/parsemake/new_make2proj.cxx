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
//
//
//

#pragma warning(disable:4786)

/*
basic algorithm is as follows:

we are passed a set of targets (with accompanying parsed makefiles).
Step one: create a complete target graph.  This is the structure that the
makefile would use to generate actions.  Where actions specify invocation
of make, perform analysis on this makefile as well.

Step two: break in clusters.  For each target in the target graph, list
all DLLs/executables that this target ends up in (the "executable set").
Create a map from each "executable set" to the targets this set is uses.
Each unique set is a cluster.

Output each cluster as a project, with its contents being the source files
listed as dependencies of the targets in the cluster.
*/


#include <map>
#include <set>
#include <string>
#include <utility>	// for pair<>
#include <ostream>

#include "generate_pdf.h"

// for debugging:
#include <iostream>


#pragma warning(disable:4503)

#include "make_toplevel.h"
#include "filetypes.h"
#include "filename.h"

#include "new_make2proj.h"


// questions: currently, we build from the dependency level, using
// list_all_depends().  Should we be better off with list_all_targets()?

typedef pair<makefile *, string> targetID;  // could be pair<makefile, target>
typedef string exeID;

class graphnode;

class cluster
{
public:
	void set_name(string const &);
//private:
	string name;
	set < targetID > whereused;
};


// globals:
typedef map < targetID, graphnode * > all_targets_type;
all_targets_type all_targets;

typedef map < set<exeID>, cluster *> cluster_type;
cluster_type clusters;



void
cluster::set_name(string const &n)
{
	name = n;
}


// build graph:
class graphnode
{
public:
	graphnode(targetID);
	void add_child(graphnode *);
	void add_parent(graphnode *);
	set<exeID> & exelist();
private:
	string name;
	set<graphnode *> parents;
	set<graphnode *> children;
	make_target * member;
	set<exeID> * execlist;
};


void
graphnode::add_child(graphnode * child)
{
	children.insert(child);
}

void
graphnode::add_parent(graphnode * parent)
{
	parents.insert(parent);
}

graphnode::graphnode(targetID t) : name(t.second), execlist(0), member(0)
{
	makefile *mf = t.first;
	string const &target = t.second;

	set<string> *children = mf->list_target_depends(target);
// TODO:
// add make actions to the list of pending makefiles to be processed
	for (set<string>::iterator child = children->begin(); child != children->end(); ++child) {
		// on loop, this will create duplicate nodes	
		all_targets_type::iterator i = all_targets.find(targetID(mf, *child));
		if (i == all_targets.end()) {
			targetID tid(mf,*child);
			i = all_targets.insert(
				all_targets_type::value_type(tid, new graphnode(tid))
			).first;
		}
		add_child(i->second);
		i->second->add_parent(this);
	}
}

void
add_mftarget2graph(makefile *mf, string const &target)
{
	// foreach target
	targetID root(mf, target);
	all_targets.insert(all_targets_type::value_type(root, new graphnode(root)));
}


void
figure_clusters()
{
	string first_target = all_targets.empty() ? string("") : all_targets.begin()->first.second;

	// for each target, add it to the list of targets in that cluster
	for (all_targets_type::iterator i = all_targets.begin(); i != all_targets.end(); ++i) {
//cout << "considering target: " << i->first.second << endl;

#if 0	// do clustering:
		set<exeID> key = (i->second->exelist());
		// make sure empty targets are put in orphaned list:
		if (key.empty())
			key.insert("orphaned");
#else	// don't do clustering:
		set<exeID> key;
		key.insert(first_target);
#endif

//cout << "used in " << key.size() << " executables: " << *key.begin() << endl;
		cluster_type::iterator here = clusters.find(key);
		if (here == clusters.end()) {
			here = clusters.insert(cluster_type::value_type(key, new cluster )).first;
		}
//cout << here->second;
		here->second->whereused.insert(i->first);
	}
}

		
set<exeID> &
graphnode::exelist()
{
	if (execlist == 0) {
		//exelist is storage, and acts as "been here" flag
		execlist = new set<exeID>;
		if (global_filetypes.what_type(name).kind == filetype::EXECUTABLE)
			execlist->insert(name);
		else {
			set<graphnode *>::iterator x = parents.begin();
			for (;x != parents.end(); ++x) {
				(*x)->exelist();
				set<exeID> parentset = (*x)->exelist();
#if defined(MSVC_NOT_BROKEN)
				execlist->insert(parentset.begin(), parentset.end());
#else
				for (set<exeID>::iterator y = parentset.begin();
						y != parentset.end();
						++y) {
					execlist->insert(*y);
				}
#endif
			}
		}
	}
	// else exelist is cached
	return *execlist;
}

#include <iostream>

void print_graph()
{
	cluster_type::const_iterator i = clusters.begin();
	for (; i != clusters.end(); ++i) {
		cout << "cluster used in executables:";
		set<exeID>::const_iterator j;
		for (j = i->first.begin(); j != i->first.end(); ++j) {
			cout << " " << *j;
		}

		cout << endl << " made up of:" ;
		set<targetID> &tmp = i->second->whereused;
		set<targetID>::const_iterator k;
		for (k = tmp.begin(); k != tmp.end(); ++k) {
			cout << " " << k->second;
		}
		cout << endl;
	}
}

static string
name_cluster( set<exeID> const &clusterkey )
{
	string ret("CLUSTER");
	// first, lookup in user-defined map:
	// well, maybe not!

	if (clusterkey.size() == 1) {
		// belongs to only one executable--name after that executable:

		// find only the last component of the name:
		filename f(*clusterkey.begin());
		ret = f.basename();
	}
	return ret;
}

set<subproj *>
graph2pdf()
{
	set <subproj *> ret;
	cluster_type::iterator i = clusters.begin();
	for (; i != clusters.end(); ++i) {
		// name the cluster based on where used:
		string pname(name_cluster(i->first));
		projname name(pname, "", pname);
		i->second->set_name(pname);

		// create a project for this cluster
		subproj *theproj = new subproj(name);
		theproj->set_selector("[[ W ]]");
		ret.insert(theproj);

		// add source files for all targets:
		set<targetID> &members = i->second->whereused;
		set<targetID>::iterator member;
		for (member = members.begin(); member != members.end(); ++member) {
			// add dependent source files to model:
			makefile::stringset * dependfiles = member->first->list_target_depends(member->second);

			makefile::stringset::iterator s = dependfiles->begin();
			while (s != dependfiles->end()) {
				// filter out non-source files
				filetype const &ftype = global_filetypes.what_type(*s);
				if (ftype.include_in_pdf()) {
					string const &fullname = member->first->member_as_absolute(*s);
					theproj->add_file(fullname.c_str());
				}
				++s;
			}
			delete dependfiles;
		}
	}
	return ret;
}


void
write_scoping_rules(ostream *out)
{
	// build map of exe->cluster usage:
	typedef map<exeID, set<cluster *> > themap_type;
	themap_type themap;

	// OK, now populate it:
	cluster_type::iterator tc;
	for (tc = clusters.begin(); tc != clusters.end(); ++tc) {
		set<exeID> const & exeset = tc->first;
		set<exeID>::const_iterator exe;
		for (exe = exeset.begin(); exe != exeset.end(); ++exe) {
			themap_type::iterator uses = themap.find(*exe);
			if (uses == themap.end()) {
				// not sure how to avoid this temp:
				set<cluster *> tmp;
				uses = themap.insert(themap_type::value_type(*exe, tmp)).first;
			}
			uses->second.insert(tc->second);
		}
	}

	// and print it:
	themap_type::iterator i;
	for (i = themap.begin(); i != themap.end(); ++i) {
		*out << "new_exe " << i->first << endl;
		set<cluster *>::iterator j;
		*out << "add_sll " << i->first;
		for (j = i->second.begin(); j != i->second.end(); ++j) {
			*out << " " << (*j)->name;
		}
		*out << endl;
	}
}



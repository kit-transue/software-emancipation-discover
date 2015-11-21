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
// generate_pdf.h
// 10.dec.97 Kit Transue
// generate pdf based on data structures built during parsing of makefile


#if !defined(_GENERATE_PDF_H)
#define _GENERATE_PDF_H

#pragma warning(disable:4786)

#include <string>
#include <iostream>
#include <set>
#include <map>

#include "sort_objects.h"

struct projname {
	projname(string const &project, string const& physical, string const &logical);
	projname();
	string project;
	string physical;
	string logical;
};


class subproj  // projects, actually
{
public:
	// public typedefs, class members
	typedef map<string *, subproj *, pless<string *> > subprojmap_type;

	// construct/copy/destroy
	subproj(string const &physname);  // logical/project names follow physical
	subproj(projname const& name);  // specify logical or project names
	virtual ~subproj();

	// compare
	bool operator< (subproj const &) const;

	// modify
	void add_file(string const &filename);
	void add_subproj(subproj *);
	void set_selector(string const &);
	void set_name(projname const &);

	// query
	string physical_name() const;
	string const &logical_name() const;
	string const &project_name() const;
	string common_root() const;
	string const &selector() const;
	// list of all root psets, for generating pset rules that cover
	// all member source files:
	// set<string> pset_roots();

	void write(ostream *out, int indent) const;
	//void write_contents(ostream * out, int indent) const;
	void write_name(ostream *out) const;
	void write_flist(ostream *out) const;

private:
	subprojmap_type subprojs;
	typedef set<string> stringset;
	stringset files;
	//string rootname;
	void indent(ostream * out, int n) const;
	subproj * parent;
	projname name;  // empty logical or project implies "mirror physical"
	string selector_text;
};


class project
{
public:
	// construct/copy/destroy
	project(subproj *rootsubproj = 0);
	virtual ~project();

	// modify
	void set_name(string const &newname);
	void set_srcroot();
	void add_subproj(subproj *);

	// access
	string const &name() const;
	string const &srcroot() const;
	string projectList() const;
	void write(ostream * out) const;
	void write_rules(ostream *out, string const &model_storage, string const& database) const;
	bool write_flist_to_admindir(string const &admindir) const;


private:
	set<subproj *> members;
	string projectname;
	string sourceroot;
};


#endif // !defined(_GENERATE_PDF_H)

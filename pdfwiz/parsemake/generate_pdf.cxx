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
// generate_pdf.cxx
// 11.Dec.97 Kit Transue

#pragma warning(disable:4503)
#pragma warning(disable:4786)

#include <string>
#include <iostream>
#include <cstring>
#include <cassert>

#include <fstream>

using namespace std;

#include "message.h"

#include "filename.h"
#include "pdfwiz_settings.h"
#include "dirtree.h"

#include "generate_pdf.h"



//\\//\\//\\//\\//\\//\\
//  misc static support stuff
//\\//\\//\\//\\//\\//\\

static char *SPACEPROTECT = "";
static char *MAKERULEPROTECT = "\"";

static void indent(ostream *out, int level)
{
	while (level) {
		*out << "  ";
		--level;
	}
}

static void
write_dirtree_as_pdf(dirnode const & dn, ostream *out, int indentlevel, string const &from)
{
	// figure out if this project has interesting components:
	if (dn.subdirs->size() == 1 && dn.files.empty()) {
		// uninteresting--flatten:
		dirnode const &interesting = dn.subdirs->begin()->second;
		write_dirtree_as_pdf(interesting, out, indentlevel, from);
	}
	else {
		// write name:
		string::size_type start = from.length();
		bool at_top = (start == dn.name().length());
			
		if (!at_top) {
			indent(out, indentlevel);
			*out << dn.name().substr(start, dn.name().length() -start - 1);
			*out << endl;
			indent(out, indentlevel);
			*out << "{" << endl;	// balance:}
		}

		// figure basename:
		start = dn.name().length();

		// count types of extensions:
		typedef map<string, int, ntfname_str_less<char> > msi_type;
		msi_type extcount;
		dirnode::files_type::const_iterator i = dn.files.begin();
		while (i != dn.files.end()) {
			string::size_type pos = i->rfind('.');
			if (pos != string::npos) {
				string ext(i->substr(pos));
				msi_type::iterator x = extcount.find(ext);
				if (x == extcount.end()) {
					extcount.insert(msi_type::value_type(ext,1));
				}
				else {
					++(x->second);
				}
			}
			++i;
		}

		// decide which ones should be written as wildcards, and
		// for those that should, write exclusions and wildcard:
		msi_type::iterator msii = extcount.begin();
		while (msii != extcount.end()) {
			set<string> &excludes = dn.missing_files(msii->first);
			if (msii->second > excludes.size()) {
				// write excludes
				set<string>::iterator exclude = excludes.begin();
				while (exclude != excludes.end()) {
					indent(out, indentlevel + 1);
					*out << '^' << *exclude << endl;
					++exclude;
				}
				// write wildcard:
				indent(out, indentlevel + 1);
				*out << '*' << msii->first << endl;
				// flag extension as wildcarded:
				msii->second = 0;
			}
			++msii;
		}

		// write remaining files:
		i = dn.files.begin();
		while (i != dn.files.end()) {
			bool write_file(true);
			// find the file's extension:
			string::size_type pos = i->rfind('.');
			if (pos != string::npos) {
				string ext(i->substr(pos));
				msi_type::iterator x = extcount.find(ext);
				assert(x != extcount.end());  // same algorithm as above, so better be here!
				write_file = x->second != 0;
			}
			if (write_file) {
				indent(out, indentlevel + 1);
				*out << *i << endl;
			}
			++i;
		}

		// write subdirs:
		dirnode::maptype::iterator j = dn.subdirs->begin();
		while (j != dn.subdirs->end()) {
			write_dirtree_as_pdf(j->second, out, indentlevel + 1, dn.name());
			++j;
		}

		// write epilogue
		if (!at_top) {
			indent(out, indentlevel);
			*out << /*balance:{*/ "}" << endl;
		}
	}
}

//\\//\\//\\//\\//\\//\\
//  projname stuff
//\\//\\//\\//\\//\\//\\

projname::projname(string const &pr, string const &ph, string const &lo) :
	project(pr),
	physical(ph),
	logical(lo)
{
}

projname::projname()
{
}


//\\//\\//\\//\\//\\//\\
//  subproj stuff
//\\//\\//\\//\\//\\//\\


/* a few notes on subprojects:
	1> they may contain files (may be at or below their physical root)
	2> they may contain other subprojects--things with new proj names
	3> the hierarchy is distinct from subproject containment.
		when writing a subproject, the files may be placed into
		expanded subprojects or subsequently flatttened if possible,
		but this is an implementation detail
*/

subproj::subproj(string const &physical) : parent(0)
{
	name.physical = physical;
}

subproj::subproj(projname const &pname) : name(pname), parent(0)
{
}

subproj::~subproj()
{
	while (!subprojs.empty()) {
		// surely there's a better way of doing this
		subprojmap_type::iterator i = subprojs.begin();
		subproj * tmp = i->second;
		subprojs.erase(i);
		delete tmp;
	}
}

// compare

bool
subproj::operator< (subproj const & s) const
{
	return physical_name() < s.physical_name();
}

// modify

void
subproj::add_file(string const &name)
{
	//assert(filename::is_absolute_path(name));
	files.insert(name);
}

void
subproj::add_subproj(subproj *subproject)
{
	assert(subproject);
	// adds a subproject to this project
	subprojs.insert(subprojmap_type::value_type(&(subproject->name.physical), subproject));
	// maintain parent relationship of inserted subproject:
	assert(!subproject->parent);
	subproject->parent = this;
}

void
subproj::set_selector(string const &new_selector_text)
{
	selector_text = new_selector_text;
}

void
subproj::set_name(projname const &new_name)
{
	name = new_name;
}


// query

string
subproj::physical_name() const
{
	string ret(name.physical);
	if (ret.empty())
		ret = common_root();
	return ret;
}

string const &
subproj::logical_name() const
{
	// TODO: what if name is empty?
	return name.logical;
}

string const &
subproj::project_name() const
{
	// TODO: what if name is empty?
	return name.project;
}

string
subproj::common_root() const
{
	string ret("C:\\");  // respectable default; currentdrive better....

	if (!files.empty()) {
		stringset::const_iterator i = files.begin();
		ret = *i;
		while (i != files.end()) {
			ret = filename::common_root(ret, *i);
			++i;
		}
	}
	return ret;
}

string const &
subproj::selector() const
{
	return selector_text;
}


void
subproj::write_name(ostream * out) const
{
	// calculate if physical name is relative to parent:
	string pn = physical_name();
	if (parent && !parent->physical_name().empty()
			&& pn.find(parent->physical_name()) == 0) {
		// strip off beginning:
		pn.erase(0, parent->physical_name().length());
	}

	// figure out if project name warrants special treatment:
	if (!name.project.empty() || !parent || !selector_text.empty()) {
		// write proj : phys <-> logical business:
		*out << project_name()
			<< " : "
			<< SPACEPROTECT << pn << SPACEPROTECT
			<< " "
			<< selector_text
			<< " <-> ";
		// make sure top-level logical names begin with slash:
		if (!parent && (logical_name()[0] != '/')) {
			*out << '/';
		}
		*out << logical_name();
	}
	else {
		// better hope there aren't spaces--the PDF can handle this,
		// but I don't know about the logical name it will infer!
		*out << SPACEPROTECT << pn << SPACEPROTECT;
	}
}


void
subproj::write(ostream * out, int indentlevel) const
{
	indent(out, indentlevel);
	write_name(out);
	*out << endl;

	indent(out, indentlevel);
	*out << '{' << endl;

	// do files first...
	// create a dirtree for files in this project:
	dirnode tree(common_root());
	for (stringset::const_iterator i = files.begin(); i != files.end(); ++i) {
		tree.addFile(*i);
	}
	write_dirtree_as_pdf(tree, out, indentlevel + 1, common_root());

	// ...then subprojects:
	for (subprojmap_type::const_iterator j = subprojs.begin(); j != subprojs.end(); ++j) {
		(*j->second).write(out, indentlevel + 1);
	}

	indent(out, indentlevel);
	*out << '}' << endl;
}

void
subproj::write_flist(ostream * out) const
{
	stringset::const_iterator i = files.begin();
	while (i != files.end()) {
		*out << *i << endl;
		++i;
	}
	subprojmap_type::const_iterator j = subprojs.begin();
	while (j != subprojs.end()) {
		j->second->write_flist(out);
		++j;
	}
}



// private:

void
subproj::indent(ostream * out, int n) const
{
	while (n--) {
		*out << "  ";
	}
}

//\\//\\//\\//\\//\\//\\
//  project stuff
//\\//\\//\\//\\//\\//\\

// construct/copy/delete

project::project(subproj * root)
{
	//assert(root);
	if (root)
		members.insert(root);
}

project::~project()
{
}

// modify

void
project::set_name(string const &newname)
{
	// make sure we use only the last path component, and strip off extension if available
	size_t extpos = newname.find_last_of(".");
	size_t drive = newname.find_first_of(":");
	size_t path = newname.find_last_of("\\");
	if (drive == string::npos)
		drive = 0;
	else
		++drive;
	if (path == string::npos)
		path = 0;
	else
		++path;
	if (path > drive)
		drive = path;
	if (extpos < drive)
		extpos = string::npos;
	projectname = newname.substr(drive, extpos - drive);

	if (!members.empty()) {
		projname pdf_pn(projectname, (*members.begin())->physical_name(), projectname);
		(*members.begin())->set_name(pdf_pn);
	}
}


void
project::set_srcroot()
{
	// calculate common root now:
	// not sure what value this has!

	if (!members.empty())
		sourceroot = (*members.begin())->common_root();
}


void
project::add_subproj(subproj *sp)
{
	members.insert(sp);
}



// access

string const &
project::name() const
{
	return projectname;
}

string const &
project::srcroot() const
{
	return sourceroot;
}

string
project::projectList () const
{
	string ret;
	set<subproj *>::const_iterator i;
	for (i = members.begin(); i != members.end(); ++i) {
		ret += '/';
		ret += (*i)->project_name();
		ret += ' ';
	}
	return ret;
}

void
project::write(ostream * out) const
{
	// main entry point for generating a PDF.

	// figure out if we're going to use new filenames:
	if (settings::old_filenames()) {
		SPACEPROTECT = "";
		MAKERULEPROTECT = "\"";
	}
	else {
		SPACEPROTECT = "\"";
		MAKERULEPROTECT = "";
	}

	// now generate the rest of the PDF accordingly:

	set<subproj *>::const_iterator i(members.begin());
	while (i != members.end()) {
		(*i)->write(out, 0);
		++i;
	}
}

void
project::write_rules(ostream * out, string const &model_storage, string const& database) const
{
	// this should probably not be quite so hardcoded!

	// projectline
	*out << "__rule001 : / <-> /__rule001" << endl;
	*out << "{" << endl;

	// pmod rule
	*out << "\t\"\" : "
		<< SPACEPROTECT
		<< srcroot()
		<< "(**).pmod\\%\\.pmoddir"
		<< SPACEPROTECT
		<< "\t=> "
		<< SPACEPROTECT
		<< model_storage
		<< "\\pmod\\(1).pmod"
		<< SPACEPROTECT
		<< endl;

	// pset rule without subdirs
	*out << "\t\"\" : "
		<< SPACEPROTECT
		<< srcroot()
		<< "(*)\\%\\.pset"
		<< SPACEPROTECT
		<< "\t=> "
		<< SPACEPROTECT
		<< model_storage
		<< "\\pset\\(1).pset"
		<< SPACEPROTECT
		<< endl;
	// pset rule with subdirs
	*out << "\t\"\" : "
		<< SPACEPROTECT
		<< srcroot()
		<< "(**)\\(*)\\%\\.pset"
		<< SPACEPROTECT
		<< "\t=> "
		<< SPACEPROTECT
		<< model_storage
		<< "\\pset\\(1)\\(2).pset"
		<< SPACEPROTECT
		<< endl;

	// make rule without subdirs
	*out << "\t.  : "
		<< SPACEPROTECT
		<< srcroot()
		<< "(*)\\%\\.make"
		<< SPACEPROTECT
		<< "\t=> "
		<< MAKERULEPROTECT
		<< "read_flags_db "
		<< SPACEPROTECT
		<< database
		<< SPACEPROTECT
		<< " "
		<< SPACEPROTECT
		<< srcroot()
		<< "(1)"
		<< SPACEPROTECT
		<< MAKERULEPROTECT
		<< endl;
	// make rule with subdirs
	*out << "\t.  : "
		<< SPACEPROTECT
		<< srcroot()
		<< "(**)\\(*)\\%\\.make"
		<< SPACEPROTECT
		<< "\t=> "
		<< MAKERULEPROTECT
		<< "read_flags_db "
		<< SPACEPROTECT
		<< database 
		<< SPACEPROTECT
		<< " "
		<< SPACEPROTECT
		<< srcroot()
		<< "(1)\\(2)"
		<< SPACEPROTECT
		<< MAKERULEPROTECT
		<< endl;

	// wrap things up
	*out << "}" << endl;
}

bool
project::write_flist_to_admindir(string const &admindir) const
{
	bool is_status_good = true;

	set<subproj *>::const_iterator spi = members.begin();
	while (spi != members.end()) {
		string flist_filename(admindir);
		flist_filename += '/';
		flist_filename += (*spi)->project_name();
		flist_filename += ".flist";
		ofstream flist(flist_filename.c_str(), ios::out);
		if (!(flist.good())) {
			msgid("Error opening flist for write") << flist_filename << eom;
			is_status_good = false;
		}
		else {
			(*spi)->write_flist(&flist);
			if (!(flist.good())) {
				msgid("Error while writing flist") << flist_filename << eom;
				is_status_good = false;
			}
			flist.close();
		}
		++spi;
	}
	return is_status_good;
}

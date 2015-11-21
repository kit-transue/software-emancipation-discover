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
#pragma warning(disable:4786)
#endif


#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif
#include "node.h"
#include "toplev_proj.h"
#include "utils.h"
#include "pathname.h"
#include "msg.h"

toplev_proj::toplev_proj(const string& p_path, const string& lname, const string& pname)
{
  physical_path = p_path;
  logical_name = lname;
  project_name = pname;

  // create root
  root = new node(physical_path);
}

toplev_proj::~toplev_proj()
{
  if (root)
    delete root;
}

const string& toplev_proj::get_physical_path() const
{
  return physical_path;
}

node* toplev_proj::addDir(pathname& dirpath)
{
  node *rt;
  node *new_root = root;

  string next_dir;
  int val = dirpath.get_first_dir(next_dir);
  while (val) {
	rt = new_root;
	new_root = findOneDirectory(next_dir, rt);
	if( !new_root) //stop if next_dir could not be found
	  break;
	val = dirpath.get_next_dir(next_dir);
  }
  // if val is not 0 at least one dir in path doesn't already exist, add it now
  while (val) {
	new_root = addOneDirectory(next_dir, rt);
	if (new_root)
	  rt = new_root;
	else {
	  msg("Could not add directory $1 in path $2.", error_sev) << next_dir << eoarg << dirpath.get_dirname() << eoarg << eom;
	  break;
	}

	val = dirpath.get_next_dir(next_dir);
  }
  return new_root;
}

//add a single directory dir_nm to parent
node* toplev_proj::addOneDirectory(string& dir_nm, node *parent)
{
  node * new_node = NULL;
  if ( !dir_nm.empty() && parent) {
    new_node = new node(dir_nm, parent);
    parent->addChild(new_node);
    return new_node;
  }
  return new_node;
}

//goes through all the children (one level) to find a child
//with the same dir_name as "name"
node* toplev_proj::findOneDirectory(const string& name, node *start)
{
  node *ret_node = NULL;
  node * ch = start->get_first_child();
  while(ch) {
    if (ch->dir_matches(name)) {
      ret_node = ch;
      break;
    }
    ch = ch->get_next_sibling();
  }
  return ret_node;
}

//this function will try to decrease/simplify the pdf structure in two ways:
// first by collapsing some of the directories in the tree structure and 
// second by adding file selectors with exclude operator where appropriate.
//NOTE: This function should be called only after all the input files are read.
void toplev_proj::simplify_structure()
{
  //do collapse on all children of root
  root->collapse_children();

  //add file selectors where it is possible
  filename path;
  root->check_physical_dir(path);
}


void toplev_proj::print(ofstream& fstr, bool read_only_flag)
{
  int level = 0;
  fstr << project_name << SPACE << COLON << SPACE << physical_path << SPACE;
  if (read_only_flag) {
    fstr << READ_ONLY_FLAG << SPACE;
  }
  fstr << MAPPER << SPACE << SLASH << logical_name << SPACE << BEGIN <<endl;
  root->print_files(fstr, level + 1);
  root->print_children(fstr, level + 1);
  fstr << END << endl;
}

//write pset rules of "rules" project
void toplev_proj::print_pset_rule(ofstream& fstr, string dir, int files_flag, string drive_nm)
{
  //if there is no slash or back slash at the end of dir, add it now
  if (!(dir.find_last_of(DIR_SEP) == dir.length() - 1))
	dir.append(SLASH);

  // pset rule for files in this directory
  //eg:"" : c:/(*)/%/.pset => $ADMINDIR/model/pset/c_drive/(1).pset
  if (files_flag > 0) {
    fstr << SPACE << QUOTE << QUOTE << SPACE << COLON << SPACE;
    fstr << dir << SRC_PSET_MAP << SPACE;
    fstr <<  RULES_MAPPER << SPACE << PSET_MODEL_RT;
	if (!drive_nm.empty())
	  fstr << drive_nm << SLASH;
    fstr <<	PSET_MODEL_MAP1 << endl;
  }
  //pset rule for files in all the directories below this one
  //eg:"" : c:/(**)/(*)/%/.pset => $ADMINDIR/model/pset/c_drive/(1)/(2).pset
  fstr << SPACE << QUOTE << QUOTE << SPACE << COLON << SPACE;
  fstr << dir << ALL_DIRS << SLASH << SRC_PSET_MAP << SPACE;
  fstr <<  RULES_MAPPER << SPACE << PSET_MODEL_RT;
  if (!drive_nm.empty())
    fstr << drive_nm << SLASH;
  fstr << PSET_MODEL_MAP2 << endl;
}

//write make rule of "rules" project
void toplev_proj::print_make_rule(ofstream& fstr, string dir, int files_flag)
{
  //if there is no slash or back slash at the end of dir, add it now
  if (!(dir.find_last_of(DIR_SEP) == dir.length() - 1))
    dir.append(SLASH);

  // make rule for files in this directory
  //eg:"" : c:/(*)/%/.make => ""$PSETHOME/bin/disperl" "$PSETHOME/mb/CR_GetFlags.pl" "c:/(1)" "
  if (files_flag > 0) {
    fstr << SPACE << QUOTE << QUOTE << SPACE << COLON << SPACE;
    fstr << dir << SRC_MAKE_MAP << SPACE;
    fstr <<  RULES_MAPPER << SPACE;
    fstr << QUOTE << MAKE_STRING << SPACE;
    fstr << QUOTE << dir << "(1)" << QUOTE << SPACE << QUOTE << endl;
  }
  //make rule for files in all the directories below this one
  //eg:"" : c:/(**)/(*)/%/.make  =>  ""$PSETHOME/bin/disperl" "$PSETHOME/mb/CR_GetFlags.pl" "c:/(1)/(2)" "
  fstr << SPACE << QUOTE << QUOTE << SPACE << COLON << SPACE;
  fstr << dir << ALL_DIRS << SLASH << SRC_MAKE_MAP << SPACE;
  fstr <<  RULES_MAPPER << SPACE;
  fstr << QUOTE << MAKE_STRING << SPACE;
  fstr << QUOTE << dir << "(1)/(2)" << QUOTE << SPACE << QUOTE << endl ;
}

//write pmod rule of "rules" project
void toplev_proj::print_pmod_rule(ofstream& fstr, string dir)
{ 
  //if there is no slash or back slash at the end of dir, add it now
  if (!(dir.find_last_of(DIR_SEP) == dir.length() - 1))
	dir.append(SLASH);
  //eg:"" : C:/(*).pmod/%/.pmoddir => $ADMINDIR/model/pmod/(1).pmod
  fstr << SPACE << QUOTE << QUOTE << SPACE << COLON << SPACE;
  fstr << dir << SRC_PMOD_MAP << SPACE;
  fstr << RULES_MAPPER << SPACE << PMOD_MODEL_RT;
  fstr << PMOD_MODEL_MAP << endl;
}

void toplev_proj::print_rules(ofstream& fstr)
{
  //for each drive print a separate pset rule
  bool not_drive = false;
  node * nd = root->get_first_child();
  while(nd) {
	filename dir = nd->get_dir();
	if (dir.is_drive()) {
	  string drive_name;
	  drive_name.append(1,dir.get_drive_letter());
	  drive_name.append(COLON);
	  print_pset_rule(fstr, drive_name, nd->num_of_files(), dir.get_name());
	  print_make_rule(fstr, drive_name, nd->num_of_files());
	}
	else
	  not_drive = true;
	nd = nd->get_next_sibling();
  }
  //print pset rule for the local drive
  if (not_drive) {
	string str = "";
	print_pset_rule(fstr, physical_path, root->num_of_files(), str);
	print_make_rule(fstr, physical_path, root->num_of_files());
  }

  print_pmod_rule(fstr, physical_path);
}

void toplev_proj::add_dirs_and_file(string& file)
{
  pathname fpath(file);
  node * dir_node = addDir(fpath);
  if (dir_node)
	dir_node->add_file(fpath);
}

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
#include "utils.h"
#include "msg.h"

#define THRESHOLD  6

bool node::has_only_one_child()
{
  return (child && !child->next_sib) ? true : false;
}

void node::append_dirname_to_path(filename& path)
{
  dir_name.append_name_to_path(path);
}

bool node::is_collapsable()
{
  return (!dir_name.is_drive() && file_list.empty() && has_only_one_child());
}

void node::remove_till_node(node* nd)
{
  node *ch = child;
  //add child of nd as child of this, and make this parent of all new children 
  child = nd->child;
  node * tmp_node = nd->child;
  while (tmp_node) {
    tmp_node->parent = this;
	tmp_node = tmp_node->next_sib;
  }

  if (!nd->file_list.empty()) {
    file_list = nd->file_list;
    file_suffixes = nd->file_suffixes;
	exclude_list = nd->exclude_list;
	all_files_included = nd->all_files_included;
  }
  nd->child = 0;
  nd->next_sib = 0;
  delete ch;
}

void node::collapse_node()
{
  if (is_collapsable()) {
    node *nd = child;
    nd->append_dirname_to_path(dir_name);
    while(nd->is_collapsable()) {
      nd = nd->child;
      nd->append_dirname_to_path(dir_name);
	}
    remove_till_node(nd);
  }
  collapse_children();
}

void node::collapse_children()
{
  node *ch = child;
  while(ch) {
	ch->collapse_node();
	ch = ch->next_sib;
  }
}

void node::get_physical_files(filename path)
{
  append_dirname_to_path(path);
  string pattern;
  //get all file suffixes and add them to the pattern to be searched for
  set<string>::iterator it;
  for(it = file_suffixes.begin(); it != file_suffixes.end(); it++) {
	pattern.append(*it);
	pattern.append(SPACE);
  }

  set<filename> all_files;
  files_in_directory(path.get_name(), pattern, all_files);
  int all_sz = all_files.size();
  int incl_sz = file_list.size();
  if (all_sz >= incl_sz) {
	set<filename> tmp_list;
	exclude_files(all_files, tmp_list);
    //exclude files only if number of files being excluded ids less than 
    //number of files included plus some THRESHOLD value
    int excl_sz = tmp_list.size() + THRESHOLD;
    if (excl_sz < incl_sz)
      exclude_list = tmp_list;
  }
  else {
	msg("Number of source files($1) less than number of iff files($2) in source directory: $3.", error_sev) << all_sz << eoarg << incl_sz << eoarg << path.get_name() << eoarg << eom;
  }
}

//get exclude file list, all files that not in the file_list set
void node::exclude_files(set<filename>& all_files, set<filename>& excl_list)
{
  set<filename>::iterator end = file_list.end();

  set<filename>::iterator it;
  for(it = all_files.begin(); it != all_files.end(); it++) {
    if (file_list.find(*it) == end)
	  excl_list.insert(*it);
  }
  if (excl_list.empty())
	all_files_included = true;
}

void node::check_physical_dir(filename path)
{
  if (file_list.size() > 0)
	get_physical_files(path);
  if (next_sib)
	next_sib->check_physical_dir(path);
  if (child) {	
    append_dirname_to_path(path);
	child->check_physical_dir(path);
  }
}

//if dir_name matches targ_str return 1
bool node::dir_matches(const string& targ_str)
{
  bool ret = (dir_name.compare(targ_str) == 0);
  return ret;
}

void node::add_file(pathname& file)
{
  //add file name to the list of filenames
  const string& name = file.get_basename();
  filename fl(name);
  file_list.insert(fl);
  //add file suffix to the list of all file siffixes
  string suff("*");
  suff.append(file.get_suffix());
  file_suffixes.insert(suff);
}

int node::num_of_files()
{
  return file_list.size();
}

//write files in this directory explicitly
void node::print_file_list(ofstream& fstr, int level)
{
  bool dir_is_slash = dir_name.is_slash();

  set<filename>::iterator it;
  for(it = file_list.begin(); it != file_list.end(); it++) {
	print_spaces(fstr, level);
	it->print(fstr, dir_is_slash);
	fstr << endl;
  }
}

//write file selectors for all files in this directory
void node::print_file_selectors(ofstream& fstr, int level)
{
  
  bool dir_is_slash = dir_name.is_slash();

  if (!exclude_list.empty()) {
    set<filename>::iterator it;
    for(it = exclude_list.begin(); it != exclude_list.end(); it++) {
	  print_spaces(fstr, level);
	  fstr << EXCLUDE;
	  it->print(fstr, dir_is_slash);
	  fstr << endl;
    } 
  }
  set<string>::iterator suf_it;
  for(suf_it = file_suffixes.begin(); suf_it != file_suffixes.end(); suf_it++) {
	print_spaces(fstr, level);
	fstr << *suf_it << endl;
  }
}

//write directory name and beginning brace
void node::print_dirname(ofstream& fstr, int level)
{
  print_spaces(fstr, level);
  if (dir_name.is_drive()) {
	dir_name.print(fstr, 0);
	fstr << SPACE << COLON << SPACE << dir_name.get_drive_letter() << COLON << SLASH;
	fstr << SPACE << MAPPER << SPACE;
	dir_name.print(fstr, 0);
	fstr << SPACE << BEGIN << endl;
  }
  else { // if root is parent and has "/", add SLASH to the directory name
	filename parent_dir = parent->get_dir();
	bool add_slash = parent_dir.is_slash();
	dir_name.print(fstr, add_slash);
	fstr << SPACE << BEGIN << endl;
  }
}

//write dir, files in dir and recursively print all children 
void node::print(ofstream& fstr, int level)
{
  print_dirname(fstr, level);
  print_files(fstr, level +1);
  print_children(fstr, level+1);
  print_block_end(fstr, level);
}

void node::print_children(ofstream& fstr, int level)
{
  node * ch = child;
  while(ch) {
    ch->print(fstr, level);
    ch = ch->next_sib;
  } 
}

void node::print_files(ofstream& fstr, int level)
{
  if (all_files_included || !exclude_list.empty())
	print_file_selectors(fstr, level);
  else
    print_file_list(fstr, level); 
}

void node::addChild(node *ch)
{
  node * first_child = child;
  child = ch;
  if (first_child)
    ch->next_sib = first_child;
}

node* node::get_first_child()
{
  return child;
}

node* node::get_next_sibling()
{
  return next_sib;
}

node* node::get_parent() const
{
  return parent;
}

const filename& node::get_dir() const
{
  return dir_name;
}

node::node(const string& nm, node * par)
{
  dir_name = nm;
  parent = par;
  child = 0;
  next_sib = 0;
  all_files_included = false;
}

node::~node()
{
  if (child)
    delete child;
  if (next_sib)
	delete next_sib;
  parent = 0;
}


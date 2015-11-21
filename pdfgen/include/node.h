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
#ifndef _NODE_H
#define _NODE_H

#ifdef ISO_CPP_HEADERS
#include <fstream>
#else
#include <fstream.h>
#endif
#include <string>
#include <set>
namespace std {};
using namespace std;

#include "pathname.h"

class node
{

 public:

  bool is_collapsable();
  void remove_till_node(node*);
  void collapse_node();
  void collapse_children();
  void get_physical_files(filename);
  void check_physical_dir(filename);
  void exclude_files(set<filename>&, set<filename>&);
  void append_dirname_to_path(filename&);
  bool dir_matches(const string&);
  void print_children(ofstream& , int);
  void print_file_list(ofstream&, int);
  void print_file_selectors(ofstream& , int);
  void print_files(ofstream&, int);
  void print_dirname(ofstream& ,int);
  void print(ofstream& ,int);
  bool has_only_one_child();
  void add_file(pathname& file);
  void addChild(node *);
  int  num_of_files();
  node *get_first_child();
  node *get_next_sibling();
  node *get_parent() const;
  const filename& get_dir() const;
  node(node& );
  node(const string& nm, node* parent = 0);
  ~node();

 private:
  set<filename> file_list;
  set<string> file_suffixes;
  set<filename> exclude_list;
  bool all_files_included;
  bool dir_is_drive;
  filename dir_name;
  node *parent;
  node *child;
  node *next_sib;
};

#endif

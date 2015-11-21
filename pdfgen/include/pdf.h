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
#ifndef _PDF_H
#define _PDF_H

#include <vector>
#ifdef ISO_CPP_HEADERS
#include <fstream>
#else
#include <fstream.h>
#endif
namespace std {};
using namespace std;
#include "toplev_proj.h"

class pdf
{

 public:
  void add_file(string& file);
  void read_list_of_iffs(string& file);
  int  number_of_projects() const;
  void print_rules_proj_line(ofstream&) const;
  void print_rules_project(ofstream&);
  void create_toplevel_proj(string& phy, string& log_name, string& proj_name);
  void simplify_pdf_structure();
  void build_pdf_structure(string&, string&, string&, string&);
  void print(string& filename, bool read_only_flag);
  void print_pdfs(string&, string&);
  toplev_proj* get_toplevel_proj(int);
  pdf(string& phy, string& log_name, string& proj_name);
  pdf();
  ~pdf();

 private:
  vector<toplev_proj*> top_projects;
};

#endif

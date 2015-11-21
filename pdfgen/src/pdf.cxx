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
#include <string>
#include <iterator>
#include "pdf.h"
#include "utils.h"
#include "msg.h"

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#define NEW_LINE '\n'

pdf::pdf(string& phy_path, string& log_name, string& proj_name)
{
  create_toplevel_proj(phy_path, log_name, proj_name);
}

pdf::pdf()
{
}

pdf::~pdf()
{
  vector<toplev_proj*>::iterator prIter;
  for (prIter = top_projects.begin(); prIter != top_projects.end(); prIter++) {
	toplev_proj* projPtr = *prIter;
	delete projPtr;
  }
}

toplev_proj* pdf::get_toplevel_proj(int index)
{
  return top_projects[index];
}


//create a new top level project
void pdf::create_toplevel_proj(string& phy_path, string& log_name, string& proj_name)
{
  if (proj_name.empty())
    proj_name = log_name;
  
  toplev_proj *top = new toplev_proj(phy_path, log_name, proj_name);

  //add the newly created project to array
  top_projects.push_back(top);

  return;
}


//return number of top level projects
int pdf::number_of_projects() const
{
  return top_projects.size();
}


//The read_only_flag adds [[ R ]] to the project line, making the project
//read only 
void pdf::print(string& filename, bool read_only_flag)
{
  ofstream fstr(filename.c_str());
  if (!fstr) {
    msg("Cannot open file $1 for writing pdf.", error_sev) << filename << eoarg << eom;
    return;
  }
  
  //for each top level project write the directory structure
  vector<toplev_proj*>::iterator projIter;
  for (projIter = top_projects.begin(); projIter != top_projects.end(); projIter++) {
    toplev_proj* proj = *projIter;
    proj->print(fstr, read_only_flag);
  }
  
  print_rules_project(fstr);
  fstr.close();
}


void pdf::print_pdfs(string& build_pdf, string& server_pdf)
{
  //print build_pdf with writable projects
  print(build_pdf, false);

  //print server_pdf with read only projects
  if (!server_pdf.empty()) {
    print(server_pdf, true);
  }
}


void pdf::print_rules_proj_line(ofstream& fstr) const
{
  fstr << RULES_PROJ << SPACE << COLON << SPACE << SLASH << SPACE;
  fstr << MAPPER << SPACE << SLASH << RULES_PROJ << SPACE << BEGIN << endl;
}


//write the "rules" project
void pdf::print_rules_project(ofstream& fstr)
{
  print_rules_proj_line(fstr);

  //for each top level project write pset and pmod rules
  vector<toplev_proj*>::iterator projIter;
  for (projIter = top_projects.begin(); projIter != top_projects.end(); projIter++) {
	toplev_proj* proj = *projIter;
    proj->print_rules(fstr);
  }

  print_block_end(fstr);
}

//read file with list of iffs 
void pdf::read_list_of_iffs(string& file)
{
  ifstream fstr(file.c_str());
  if (!fstr) {
	  msg("Cannot open file $1 for reading list of iff files.", error_sev) << file << eoarg << eom;
	  return;
  }

  string line;
  while(getline(fstr, line, NEW_LINE)) {
	if (!line.empty())
	  add_file(line);
  }

  fstr.close();                     
}


void pdf::simplify_pdf_structure()
{
  //for each top level project simplify structure
  vector<toplev_proj*>::iterator projIter;
  for (projIter = top_projects.begin(); projIter != top_projects.end(); projIter++) {
    toplev_proj* proj = *projIter;
    proj->simplify_structure();
  }
}


void pdf::build_pdf_structure(string& src_root, string& log_name, string& proj_name, string& ifflist_file )
{
  create_toplevel_proj(src_root, log_name, proj_name);
  read_list_of_iffs(ifflist_file);

  //simplify pdf structure unless SIMPLIFY_PDF environment variable is set 
  //to 0.
  char * val = getenv("SIMPLIFY_PDF");
  if (!val || strcmp(val, "0")) {
    simplify_pdf_structure();
  }

}


void pdf::add_file(string& file)
{
  vector<toplev_proj*>::iterator projIter;
#if 0
  for(projIter = top_projects.begin(); projIter != top_projects.end(); projIter++) {
	toplev_proj* proj = *projIter;
	string phy_path = proj->get_physical_path();
	int val = file.find(phy_path);
	int len = phy_path.length();
	if (val == 0 && file.find_first_of(DIR_SEP, len) == len) {
	  proj->add_dirs_and_file(file);
      break;
	}
  }
  if (projIter == top_projects.end())
	msg("Could not add file $1 to any of the projects.", error_sev) << file << eoarg << eom;
#endif

  projIter = top_projects.begin();
  toplev_proj* proj = *projIter;
  proj->add_dirs_and_file(file);
}
 

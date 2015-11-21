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
#include <string>
#include <map>
namespace std {};
using namespace std;

#include "msg.h"
#include "pdf.h"

#define DEFAULT_ROOT "/"

static bool process_arguments(int argc, char **argv, map<string, string>& options_map)
{
  bool ok_flag = true;
  int i = 1;
  while (i < argc) {
    string option = argv[i];
    if (option[0] == '-') {
      i++;
      string value;
      if (i < argc && *argv[i] != '-') {
	value = argv[i];
      }
      options_map.insert(pair<const string, string>(option, value));
    }
    i++;
  }

  return ok_flag;
}


static string find_argument_value(string arg_name, map<string, string>& opt_map)
{
  string arg_value="";

  map<string, string>::iterator it;
  it = opt_map.find(arg_name); 
  if ( it != opt_map.end() )
    arg_value = it->second;

  return arg_value;
}


int main(int argc, char ** argv)
{
  int ret_val = 0;
  map<string, string> options_map;
  bool ok = process_arguments(argc, argv, options_map);
  
  string lpservice_name = find_argument_value("-lpservice", options_map);
  string msg_groupid = find_argument_value("-message_group", options_map);
  if (!lpservice_name.empty() && !msg_groupid.empty()) {
    init_lp_service(lpservice_name.c_str(), msg_groupid.c_str());
  }

  string proj = find_argument_value("-project", options_map);
  string iff_file = find_argument_value("-ifflist", options_map);
  string pdf_file = find_argument_value("-pdf", options_map);

  if ( !ok || proj.empty() || iff_file.empty() || pdf_file.empty() ) {
    
    msg("Usage: pdfgen -project project_name -ifflist ifffile_list -pdf pdf_filename [-src_root root] [-server_pdf file] [-lpservice service_name -message_group mgid]", catastrophe_sev) << eom;
    
    ret_val = 1;
  } 
  else {
    string src_root = find_argument_value("-src_root", options_map);
    if (src_root.empty()) {
      src_root = DEFAULT_ROOT;
    }

    //build pdf structure
    pdf pdf_inst;
    pdf_inst.build_pdf_structure(src_root, proj, proj, iff_file);

    //print build pdf and server pdf(if -server_pdf option present)
    string server_pdffile = find_argument_value("-server_pdf", options_map);
    pdf_inst.print_pdfs(pdf_file, server_pdffile); 

  }

  return ret_val;
}

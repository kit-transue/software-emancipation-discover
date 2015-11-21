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
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string>
namespace std {};
using namespace std;

#include <sys/stat.h>

#include "pathname.h"
#include "utils.h"
#include "msg.h"

void replace_slash_with_backslash(string& dir)
{
  int ind = dir.find(SLASH);
  while(ind != -1) {
    dir.replace(ind, 1, BACK_SLASH);
	ind = dir.find(SLASH, ind);
  }
}

bool is_reg_file_type(string& dir, string& file)
{
  bool is_file = false; 
  int ret;

  //join directory name and file name to come up with an absolute path
  string full_filename = dir; 
  full_filename.append(SLASH); 
  full_filename.append(file);

#ifdef _WIN32
  replace_slash_with_backslash(full_filename);
  struct _stat stat_buf;
  ret = _stat(full_filename.c_str(), &stat_buf);
  is_file = (stat_buf.st_mode & _S_IFREG);
#else
  struct stat stat_buf;
  ret = ::stat(full_filename.c_str(), &stat_buf);
  is_file = S_ISREG(stat_buf.st_mode);
#endif

  return is_file;
}

string ls_cmd(string& dir, string& pattern)
{
#ifdef _WIN32
  replace_slash_with_backslash(dir);
  //if dir is just drive letter add "\\" eg c: -> c:\ 
  if (dir.length() == 2 && dir[1] == ':'){
    dir.append("\\");
  }
  string cmd = "cd /D ";
  cmd.append(dir);
  cmd.append(" && dir /B ");
  cmd.append(pattern);
#else
  string cmd = "cd \"";
  cmd.append(dir.c_str());
  cmd.append("\"; ls -d ");
  cmd.append(pattern.c_str());
#endif //_WIN32

  return cmd;
}


void files_in_directory(string dir, string& pattern, set<filename>& file_set)
{
  FILE* file;
  string cmd = ls_cmd(dir, pattern);
#ifdef _WIN32
  file = _popen(cmd.c_str(), "rt");
#else
  file = popen(cmd.c_str(), "r");
#endif

  if (file == NULL) {
    msg("Could not open pipe for command: $1", error_sev) << cmd << eoarg << eom;
    return;
  }

  char line[1024];
  string str;
  int len;
  while (!feof(file) ) {
    if (fgets(line, 1024, file) != NULL) {
      //remove the newline character from the end
      len = strlen(line);
      line[len -1] = '\0';
      str = line;
      
      //add regular files to the set of all files
      if (!str.empty() && is_reg_file_type(dir, str)) {
        filename fl(str);
        file_set.insert(fl);
      }
    }
  }
#ifdef _WIN32
  _pclose(file);
#else
  pclose(file);
#endif
}

bool name_has_space(const string& name)
{
  bool has_space = false;
  const char * ptr = name.c_str();
  while (ptr && *ptr) {
    if (isspace(*ptr)) {
	  has_space = true;
	  break;
	}
	ptr++;
  }
  return has_space;
}

void print_spaces(ofstream& fstr, int level) 
{
  for (int l = 0; l < level; l++)
    fstr << SPACE;
}

void print_block_end(ofstream& fstr, int level) 
{
  print_spaces(fstr, level);
  fstr << END << endl;
}


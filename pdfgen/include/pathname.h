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
#ifndef _PATHNAME_H
#define _PATHNAME_H

#ifdef ISO_CPP_HEADERS
#include <fstream>
#else
#include <fstream.h>
#endif

#include <string>
namespace std {};
using namespace std;

//This class add stricmp to the regular string class. stricmp is used
//to do case insensitive comparisions
class string_nocase : public string
{
public:
  string_nocase(const string& str):string(str){};
  string_nocase(char* str): string(str) {};
  string_nocase() {};
  int stricmp(const string&);
};

class filename
{
public:
  void append_name_to_path(filename&);
  bool is_slash();
  bool is_drive() { return dir_has_drive; }
  char get_drive_letter();
  int compare(const string&, int st_ind = 0);
  bool operator<(filename &);
  void print(ofstream&, int) const;
  const string& get_name() const { return name; }
  string& get_name() { return name; }
  filename(const filename& fl);
  filename(const string& str);
  filename() : name(""), dir_has_drive(0){};
private:
  bool dir_has_drive;
  string_nocase name;
};

//This function is necessary when objects of filename class are 
//used in a set like set<filename>.
bool operator<(filename, filename);

class pathname
{
public:
  int get_first_dir(string&);
  int get_next_dir(string&);
  const string& get_basename();
  const string& get_dirname();
  const string& get_suffix();
  pathname(string& );

  int last_off; //stores the offset in dirname that was got from the last call to get_next_dir 
  int last_dir; //flags the last directory in dirname
  string suffix;
  string basename;
  string dirname;
};

#endif

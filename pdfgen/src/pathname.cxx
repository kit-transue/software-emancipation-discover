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
#include "pathname.h"
#include "utils.h"
#include <string>

#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#define BACKSLASH '\\'
#define FWDSLASH '/'
#define DOT "."

int pathname::get_first_dir(string& dir)
{
  if (!dirname.empty())
	last_dir = 0;
  int ret_val = get_next_dir(dir);
  return ret_val;
}

int pathname::get_next_dir(string& dir)
{
  int ret_val = 0;
  if (!last_dir) {
	int val = dirname.find_first_of(DIR_SEP, last_off);
    if (val < 0) { 
	  last_dir = 1;				//mark last directory
      dir = dirname.substr(last_off, dirname.length() - last_off);
	}
    else {
	  dir = dirname.substr(last_off, val - last_off);
	  last_off = val+1;
	}
	ret_val = 1;
  }
  return ret_val;
}

const string& pathname::get_suffix()
{
  return suffix;
}

const string& pathname::get_basename()
{
  return basename;
}

const string& pathname::get_dirname()
{
  return dirname;
}

//constructors of pathname breaks the path name into dirname and basename
pathname::pathname(string& file)
{
  last_off = 0;
  if (file[0] == FWDSLASH || file[0] == BACKSLASH)
	last_off = 1;
  int val = file.find_last_of(DIR_SEP);
  if (val <= 0) {
	dirname = "";
	last_dir = 1;
	if (val == 0)
	  basename.assign(file, last_off, file.length());
	else
	  basename = file;
  }
  else {
    int len = file.length();
	dirname.assign(file, 0, val);
	basename.assign(file, val+1, len-(val+1));
	last_dir = 0;
  }
  // Look for a "." in name of file for its suffix
  val = basename.find_last_of(DOT);
  if (val > 0)
	suffix.assign(basename, val, basename.length());
}

//do case insensitive comparision on NT
int string_nocase::stricmp(const string& str)
{
#ifdef _WIN32
  return _stricmp(c_str(), str.c_str());
#else
  return compare(str);
#endif
}

filename::filename(const string& str) : name(str)
{
 string drive_pattern = "_drive";
 if (compare(drive_pattern, 1) == 0)
   dir_has_drive = true;
 else
   dir_has_drive = false;
}

filename::filename(const filename& fl) : name(fl.name), dir_has_drive(fl.dir_has_drive)
{
}

bool filename::operator<(filename& fl)
{
  int ret = name.stricmp(fl.get_name());
  return (ret < 0) ? 1 : 0;
}

bool filename::is_slash()
{
  return (name.compare(SLASH) == 0 || name.compare(BACK_SLASH) == 0);
}

int filename::compare(const string& dest_str, int st_index)
{
  string_nocase tmp_name = name.substr(st_index, name.length()-st_index);
  return tmp_name.stricmp(dest_str);
}

void filename::print(ofstream& fstr, int add_slash) const
{
  bool has_space = name_has_space(get_name());
  if (has_space)
	fstr << QUOTE;
  if (add_slash)
	fstr << SLASH;
  fstr << get_name();
  if (has_space)
    fstr << QUOTE;
}

char filename::get_drive_letter()
{
  return (dir_has_drive) ? name[0] : '\0';
}

void filename::append_name_to_path(filename& path)
{
  string& path_name = path.get_name();
  if (dir_has_drive) {
    path_name.assign(1, get_drive_letter());
    path_name.append(COLON);
  }
  else {
    //append slash if path is not empty and last character of path is not slash
    if (!path_name.empty() && (path_name.find(SLASH, path_name.size()-1) == -1))
      path_name.append(SLASH);
	path_name.append(get_name());
  }
}

bool operator<(filename fl1, filename fl2)
{
  string_nocase str = fl1.get_name();
  int ret = str.stricmp(fl2.get_name());
  return (ret < 0) ? 1 : 0;
}

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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std;

#include "simplify_globals.h"
//#include "parray.h"

const int MAXLINE = 8000;

void GetRemoveIncludeLine(string& str, const char* buf)
{
  str = "/* ";

  //check if there was a c style comment on that line to
  //avoid nested comments
  
  char* ptr = (char*) buf;
  while (ptr)
    {
      if (*(ptr + 1) == '*') // a comment
        break;

      ptr = strchr(ptr + 1, '/');
    }
 
  if (ptr)
    {
      *ptr = '\0';
      str += buf;
      *ptr = '/';

      str += " SIMPLIFY REMOVED */ ";

      str += ptr;
    }
  else
    {
      str += buf;
      str += " SIMPLIFY REMOVED */";
    }
}

//check whether this file is located in an include dir specified by -I
int TryIncludeDirs(string& str, const char* file)
{
  int ret = 0;
  int max_len = 0;
  int max_index = -1;

  int size = GetIncludeDirs().size();
  for (int i = 0; i < size; ++i)
    {
      const char* dir = GetIncludeDirs()[i];
      int len = strlen(dir);
      if (strncmp(dir, file, len) == 0)
        {
          if (len > max_len && file[len] == '/' || file[len] == '\\')
            {
              max_len = len;
              max_index = i;
            }
        }
    }

  if (max_index > -1)
    {
      const char* dir = GetIncludeDirs()[max_index];
      int len = strlen(dir);
      str = "<";
      str += file + len + 1;
      str += ">";
      ret = 1;
    }

  return ret;
}

int TrySourceDirAndBelow(string& str, const char* file, const char* source)
{
  int ret = 0;

  int len = strlen(source);
  if (strncmp(source, file, len) == 0 && file[len] == '/' || file[len] == '\\')
    {
      str = "\"";
      str += file + len + 1;
      str += '"';
      ret = 1;
    }

  return ret;
}

//check "../anydir" directory relative to the source
int TryOneLevelHigherSource(string& str, const char* file, const char* source)
{
  int ret = 0;

  char* ptr = (char*) strrchr(source, '/');
  char sep = '/';
  if (!ptr)
    {
      ptr = (char*) strrchr(source, '\\');
      sep = '\\';
    }

  if (ptr)
    {
      *ptr = '\0';
      int len = strlen(source);
      if (strncmp(source, file, len) == 0 && file[len] == '/' || file[len] == '\\')
        {
          str = "\"";
          str += "..";
          str += file + len;
          str += '"';
          ret = 1;
        }
      *ptr = sep;
    }

  return ret;
}

//check "../../anydir" directory relative to the source
int TryTwoLevelsHigherSource(string& str, const char* file, const char* source)
{
  int ret = 0;

  char* ptr = (char*) strrchr(source, '/');
  char sep = '/';
  if (!ptr)
    {
      ptr = (char*) strrchr(source, '\\');
      sep = '\\';
    }

  if (ptr)
    {
      *ptr = '\0';
      char* ptr1 = (char*) strrchr(source, sep);
      if (ptr1)
        {
          *ptr1 = '\0';
          int len = strlen(source);
          if (strncmp(source, file, len) == 0 && file[len] == '/' || file[len] == '\\')
            {
              str = "\"";
              str += "..";
              str += sep;
              str += "..";
              str += file + len;
              str += '"';
              ret = 1;
            }
          *ptr1 = sep;
        }
      *ptr = sep;
    }

  return ret;
}

void GetShortName(string& str, const char* file)
{
  const char* ptr = strrchr(file, '/');
  if (!ptr)
    ptr = strrchr(file, '\\');

  str = "<";
  str += ptr + 1;
  str += ">";
}

int GetIncludeName(string& str, const char* file, const char* source, int short_sub_names)
{
  int ret = 0;

  //get source dir
  char* ptr = (char*)strrchr(source, '/');
  char sep = '/';
  if (!ptr)
    {
      ptr = (char*)strrchr(source, '\\');
      sep = '\\';
    }

  if (ptr)
    *ptr = '\0';
 
  if (!ret)
    ret = TryIncludeDirs(str, file);

  if (!ret)
    ret = TrySourceDirAndBelow(str, file, source);

  if (!ret)
    ret = TryOneLevelHigherSource(str, file, source);

  if (!ret && short_sub_names)
    {
      ret = 1;
      GetShortName(str, file);
    }

  if (!ret)
    ret = TryTwoLevelsHigherSource(str, file, source);

  //last resort, use full file name
  if (!ret)
    {
      str = "\"";
      str += file;
      str += '"';
    }

  if (ptr)
    *ptr = sep;

  return ret;
}

void GetSubIncludeLine(string& str, const char* file, const char* source, 
                       int nocomments, int short_sub_names)
{
  str = "#include ";
  string incstr;
  GetIncludeName(incstr, file, source, short_sub_names);
  str += incstr;
  if (!nocomments)
    str += " /* SIMPLIFY SUBSTITUTED */";
}

void ModifySrc(const char* file, vector<int>& remove, vector<SymLoc*>& subs, 
               int nocomments, int short_sub_names)
{
  FILE* fin = fopen(file, "rw");
  if (!fin)
    return;

  // constructing a path to a temp directory
  char tmpPath[1024];
  memset(tmpPath,0,1024);
  char* pathToTmp = getenv("TMP");
  if(pathToTmp==NULL)
   pathToTmp = getenv("TEMP");
  if(pathToTmp==NULL)
   pathToTmp = getenv("TMPDIR");
  if(pathToTmp!=NULL) strcpy(tmpPath,pathToTmp);

#ifndef _WIN32
  char * tmpFile = tempnam(tmpPath, NULL);
#else
  char * tmpFile = _tempnam(tmpPath, NULL);
#endif

  FILE* fout = fopen(tmpFile, "w");
  if (!fout)
    {
      fclose(fin);
      return;
    }

  char buf[MAXLINE]; // This is very bad, to use fixed length char array to copy strings.
  int line = 0;

  while(fgets(buf, MAXLINE, fin))
    {
      ++line;

      // Remove newline, if present.
      int buflen = strlen(buf);
      int had_newline = (buf[buflen - 1] == '\n');
      if (had_newline)
        buf[buflen - 1] = '\0';

      int rem = 0;

      int size = remove.size();
      for (int i = 0; i < size; ++i)
        {
          if (line == (int)remove[i])
            {
              rem = 1;
              break;
            }
        }

      if (!rem)
        {
          if (had_newline)
            fprintf(fout, "%s\n", buf);
          else
            fprintf(fout, "%s", buf);
        }
      else
        {
          string str;

          // check if there is a comment on this line
          const char* ptr = strstr(buf, "/*");
          if (!ptr)
            ptr = strstr(buf, "//");

          if (!nocomments || ptr)
            {
              GetRemoveIncludeLine(str, buf);
              fprintf(fout, "%s\n", str.c_str());
            }

          //check substitutions
          int size = subs.size();
          for (int i = 0; i < size; ++i)
            {
              SymLoc* sub = subs[i];
              if (sub->line == line)
                {
                  GetSubIncludeLine(str, sub->file, file, nocomments, short_sub_names);
                  fprintf(fout, "%s\n", str.c_str());
                }
            }
        }
    }
  
  fclose(fin);
  fclose(fout);

  FileMove(tmpFile, file);
}










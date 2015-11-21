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
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <string>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#else
#include <unistd.h>
#endif

#include "InclusionNode.h"
#include "InclusionTree.h"
#include "iff_parser.h"
#include "simplify_globals.h"
#include "symbolStruct.h"

void StartLog(const char*);
void EndLog();
int Simplify(int do_query, int allow_sub, const char* cmd, int modify, int nocom, int short_names);
void Output(const char* st);
void PrintInclusionTree();

FILE* log_file = 0;
int g_verbose = 0;
int g_print = 0;
unsigned int source_id = 0; // global, set in ParserCallBacks.C.

void cleanup();
void AddIncludeDir(char* dir);
int  RunParser(char *cmd_fname);

int main(int argc, char** argv)
{
  int do_query = 0;
  int allow_sub = 1;
  char* cmd = 0;
  char* iff_name = 0;
  char* log_file_name = 0;
  int modify = 0;
  int nocomments = 0;
  int short_name = 0;
  int print_tree = 0;

  int error = 0;
  if (argc >= 2)
    {
      iff_name = argv[argc - 1];
    }
  else
    {
      error = 1;
    }

  int i = 1;
  while (i < argc - 1)
    {
      if (strcmp(argv[i], "-query") == 0)
	do_query = 1;
      else if (strcmp(argv[i], "-nosub") == 0)
	allow_sub = 0;
      else if (strcmp(argv[i], "-print") == 0)
	g_print = 1;
      else if (strcmp(argv[i], "-verbose") == 0)
	g_verbose = 1;
      else if (strcmp(argv[i], "-modify") == 0)
	modify = 1;
      else if (strcmp(argv[i], "-run_parser") == 0) {
	i++;
	int status = RunParser(argv[i]); 
	if(status != 0){
	  printf("-2\nUnable to execute parser\n");
	  exit(0);
	}
      } else if (strncmp(argv[i], "-I", 2) == 0)
	AddIncludeDir(argv[i] + 2);
      else if (strcmp(argv[i], "-nocomments") == 0)
	nocomments = 1;
      else if (strcmp(argv[i], "-short_sub_names") == 0)
	short_name = 1;
      else if (strcmp(argv[i], "-print_tree") == 0)
	print_tree = 1;
      else if (strcmp(argv[i], "-log") == 0)
	{
	  if (i + 1 < argc)
	    log_file_name = argv[i + 1];
	  else
	    {
	      error = 1;
	      break;
	    }
	  ++i;
	}
      else if (strcmp(argv[i], "-cmd") == 0)
	{
	  if (i + 1 < argc)
	    cmd = argv[i + 1];
	  else
	    {
	      error = 1;
	      break;
	    }
	  ++i;
	}
      
      ++i;
    }

  if (error)
    {
      fprintf(stderr, "usage: %s [-nosub] [-verbose] [-modify] [-cmd name] [-I<dir>]"
	              "[-log filename] [-print] [-nocomments] <iff_name>\n", argv[0]);
      return WRONG_COMMAND_LINE;
    }

  StartLog(log_file_name);

  FILE*f = fopen(iff_name, "r");
  if (f)
    {
      fclose(f);
      int retval = iff_parse(iff_name);  // negative -> error; positive -> success.
      if (retval > 0)
        {
	  Simplify(do_query, allow_sub, cmd, modify, nocomments, short_name);
	  if (print_tree)
	    PrintInclusionTree();
        }
      else
	{
	  // Parse error already reported.
	}
    }
  else
    {
      Error(CANNOT_OPEN_IFF, iff_name);
    }
  
  EndLog();
  cleanup();

  return NO_ERRORS;
}

void Error(int type, const char* msg)
{
  cleanup();
  switch (type)
    {
    case OUT_OF_MEMORY:
      Output("Out of Memory\n");
      break;
    case CANNOT_OPEN_IFF:
      Output("Cannot open IF file: ");
      Output(msg);
      Output("\n");
      break;
    case FILE_DOESNOT_EXIST:
      Output("File does not exist: ");
      Output(msg);
      Output("\n");
      break;
    }
  EndLog();
  cleanup();
  exit(type);
}

const char* MaybeGetSourceName()
{
  symbol* sym = GetSymbols()[source_id];
  return sym != NULL ? sym->name : NULL;
}

const char* GetSourceName()
{
  symbol* sym = GetSymbols()[source_id];
  assert(sym);
  return sym->name;
}

void FileMove(const char* f1, const char* f2)
{
#ifndef _WIN32
  string cmd = "mv ";
  cmd += f1;
  cmd += " ";
  cmd += f2;
  system(cmd.c_str());
#else
  MoveFileEx(f1, f2, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
#endif
}

void AddIncludeDir(char* dir)
{
    char buffer[PATH_MAX];
    char* path = realpath(dir, buffer);
    if (path) {
        GetIncludeDirs().push_back(path);
    }
}

const char* GetTimeStr()
{
  const char* ret = "";
  time_t t = time(0);
  if (t != -1)
    {
      ret = ctime(&t);
    }
  return ret;
}

void StartLog(const char* log_file_name)
{
  if (!log_file_name)
    return;

  log_file = fopen(log_file_name, "a");
  if (log_file)
    {
      fprintf(log_file, "------------------------------------------------------------------------\n");
      fprintf(log_file, "Simplify started at %s\n", GetTimeStr());
    }
  else
    {
      fprintf(stderr, "Cannot open %s for writing\n", log_file_name);
    }
}

void EndLog()
{
  if (log_file)
    {
      const char *source_name = MaybeGetSourceName();
      fprintf(log_file, "\nSimplify for %s finished at %s\n", 
	      source_name != NULL ? source_name : "<unknown>", GetTimeStr());
      fprintf(log_file, "------------------------------------------------------------------------\n\n");
      fclose(log_file);
      log_file = 0;
    }
}

const char* GetShortFileName(const char* name)
{
  const char* ptr = strrchr(name, '/');
  if (!ptr)
    ptr = strrchr(name, '\\');

  if (ptr)
    ++ptr;
  else
    ptr = name;

  return ptr;
}

void PrintDependency(FILE* file, symbol* f1, symbol* f2, symbol* f2par, symbol* sym)
{
  if (f2par)
    fprintf(file, "%s needs %s included in %s because of %s %s\n", 
	    GetShortFileName(f1->name), GetShortFileName(f2->name), 
	    GetShortFileName(f2par->name), sym->kind, sym->name);
  else
    fprintf(file, "%s needs %s because of %s %s\n", 
	    GetShortFileName(f1->name), GetShortFileName(f2->name), 
	    sym->kind, sym->name);
}

void OutputDependency(InclusionNode* node1, InclusionNode* node2, int symid)
{
  if (g_verbose && node1 != node2)
    {
      symbol* f1 = GetSymbols()[node1->GetId()];
      symbol* f2 = GetSymbols()[node2->GetId()];
      symbol* f2par = 0;
      if (node2->GetParent())
	f2par = GetSymbols()[node2->GetParent()->GetId()];
      
      symbol* sym = GetSymbols()[symid];
      
      if (log_file)
	PrintDependency(log_file, f1, f2, f2par, sym);

      if (g_print)
	PrintDependency(stdout, f1, f2, f2par, sym);
    }
}

void Output(const char* str)
{
  if (str)
    {
      if (log_file)
	fprintf(log_file, "%s", str);
      
      if(g_print)
	printf("%s", str);
    }
}

void PrintInclusionTree(InclusionNode* node, string spaces)
{
  while (node)
    {
      symbol* sym = GetSymbols()[node->GetId()];

      if (sym)
	{
          printf("%s%s", spaces.c_str(), GetShortFileName(sym->name));

	  if (node->IsMarked())
	    printf(" *");
	
	  printf("\n");
	}
      
      PrintInclusionTree(node->GetFirst(), spaces + "    "); 
      node =  node->GetNext();
    }
}

void PrintInclusionTree()
{
  printf("Inclusion Tree: (* means that a file needs to be included)\n\n");
  PrintInclusionTree(GetInclusionTree()->GetRoot(), "");
}

int RunParser(char *cmd_fname) {
  char buffer[8000];

  buffer[0] = '\0';
  FILE *f   = fopen(cmd_fname, "r");
  if(f == NULL)
    return 4;
  fgets(buffer, 8000, f);
  char *dir_end = strrchr(buffer, '/');
  if(dir_end == NULL)
    dir_end = strrchr(buffer, '\\');
  if(dir_end != NULL) {
    *dir_end = '\0';
    chdir(buffer);
  }
  fgets(buffer, 8000, f);
  char *last = strchr(buffer, '\n');
  if(strchr(buffer, '\r') != NULL && strchr(buffer, '\r') < last)
    last = strchr(buffer, '\r');
  if(last)
    *last = '\0';
#ifdef _WIN32
  STARTUPINFO sinfo;
  memset(&sinfo, 0, sizeof(sinfo));
  sinfo.cb = sizeof(sinfo);
  PROCESS_INFORMATION pinfo;
  if(!CreateProcess(NULL, buffer, NULL, NULL, TRUE,
		    CREATE_DEFAULT_ERROR_MODE | DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 
		    GetEnvironmentStrings(),
		    NULL, &sinfo, &pinfo)){
    return -1;
  }
  WaitForSingleObject(pinfo.hProcess, INFINITE);
  DWORD status;
  GetExitCodeProcess(pinfo.hProcess, &status);
  return (int)status;
  /*
  char exec_buffer[1024];
  char *spc = strchr(buffer, ' ');
  strncpy(exec_buffer, buffer, spc - buffer);
  exec_buffer[spc - buffer] = '\0';
  printf("%s\n", exec_buffer);
  int status =_spawnl(_P_WAIT, exec_buffer, exec_buffer, spc + 1, NULL);
  perror("Err");
  */
#else
  int status = system(buffer);
#endif
  return status;
}

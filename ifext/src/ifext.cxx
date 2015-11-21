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
#include <elsParser.h>
#include <systemMessages.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <stdio.h>

int do_debug    = 0;
FILE* log_fd = stderr;

#include "msg.h"

//int gtDisplay_is_open(){  return 0;}
extern "C" void driver_exit(int code) {exit(code);}
void dis_message(const char*, systemMessageType, const char*, ...) {}
int popup_Question(const char*, const char*, const char*, const char*){return 0;}
void system_message_force_logger(int) {}
extern "C" void _pure_error_(){}
extern int yy_parser_debug;	
extern "C" int OSapi_fwrite(){return 0;}
#define MAX_BUF_SIZE 4100
static int read_line(FILE*f, char* buf)
{
  for(;;){
    char*str = fgets(buf, MAX_BUF_SIZE, f);  
    if(!str)
      return 0;
    while(isspace(*str)) str++;
    int len = strlen(str);
    if(len==0)
      continue;
    int ch;
    while(len > 0 && (ch = str[len-1])){
      if(!isspace(ch))
	break;
      len--;
    }
    if(len==0)
      continue;  
    str[len] = '\0';
    return len;
  }
}

//
// Returns true on success, false on failure
//
static bool process_job(iffParser* parser, const char* job)
{
  char src_name[MAX_BUF_SIZE+1], out_name[MAX_BUF_SIZE+1];
  FILE* f = fopen(job, "r");
  if(!f){
    msg("cannot open file $1", catastrophe_sev) << job << eoarg << eom;
    return false;
  }
  int count = 0;
  for(;;){
    int len = read_line(f, src_name);
    if(len==0)
      break;
    len = read_line(f, out_name);
    if(len==0){
      msg("odd number of lines in file $1 (should be even)", catastrophe_sev) << job << eoarg << eom;
      return false;
    }
    ++count;
    parser->add_file(src_name, out_name, iffParser::INPUT);
  }
  if (count == 0) {
    msg("empty job file $1", catastrophe_sev) << job << eoarg << eom;
    return false;
  } else {
    return true;
  }
}

static void dump_all(iffParser* parser)
{
  int sz = parser->sources.size();
  for(int ii=0; ii<sz; ++ii){
    iffSrcFile*src=*parser->sources[ii];
    if (src->ftype == iffParser::INPUT) {
      const char*dump_file = src->fdump;
      if(!dump_file || !*dump_file) {
	src->print(cout);
      } else {
	ofstream ofs(dump_file, ios::out);
	src->print(ofs);
      }
    }
  }
}

int main(int argc, char**argv)
{
  if (argc == 1)
    return 0;

  const char* iff[10];
  int no_iff = 0;

  const char* out = NULL;
  const char* ext = NULL;
  const char* job = NULL;
  const char* ofile = "";
  const char* purge_prefix = NULL;
  const char *message_group = NULL;
  const char *lp_service = NULL;

  ifp_use_mmap = 0;
  for(int ii=1; ii<argc; ++ii){
   const char*arg = argv[ii];
   if(arg[0] == '-'){
    if(strcmp(arg, "-f")==0){
      if(job){
	msg("only one job file allowed", catastrophe_sev) << eom;
        exit(1);
      } else if(ii==(argc-1)){
	msg("-f used, but no job file is specified", catastrophe_sev) << eom;
        exit(1);
      } else {
	job = argv[++ii];
      }
    } else if(strcmp(arg, "-o")==0){
      if(out){
	msg("only one output file allowed", catastrophe_sev) << eom;
        exit(1);
      } else if(ii==(argc-1)){
	msg("-o used, but no output file is specified", catastrophe_sev) << eom;
        exit(1);
      } else {
	out = argv[++ii];
      }
    } else if(strcmp(arg, "-e")==0){
      if(ext){
	msg("only one extract file allowed", catastrophe_sev) << eom;
        exit(1);
      } else if(ii==(argc-1)){
	msg("-e used, but no extract file is specified", catastrophe_sev) << eom;
        exit(1);
      } else {
	ext = argv[++ii];
      }
    } else if(strcmp(arg, "-ifile")==0){
      ofile = argv[++ii];
    } else if(strcmp(arg, "-purge_prefix")==0){
      purge_prefix = argv[++ii];
    } else if(strcmp(arg, "-mmap")==0){
      ifp_use_mmap = 1;
    } else if(strcmp(arg, "-message_group")==0){
      if(message_group){
	msg("only one message group allowed", catastrophe_sev) << eom;
        exit(1);
      } else if(ii==(argc-1)){
	msg("-message_group used, but no message group is specified", catastrophe_sev) << eom;
        exit(1);
      } else {
	message_group = argv[++ii];
      }
    } else if(strcmp(arg, "-lp_service")==0){
      if(lp_service){
	msg("only one lp service allowed", catastrophe_sev) << eom;
        exit(1);
      } else if(ii==(argc-1)){
	msg("-lp_service used, but no service is specified", catastrophe_sev) << eom;
        exit(1);
      } else {
        lp_service = argv[++ii];
      }
    } else {
      msg("$1: wrong option: $2", catastrophe_sev) << argv[0] << eoarg << arg << eoarg << eom;
      exit(1);
    }
  } else {
    if(no_iff > 8)
      msg("too many inputs", error_sev) << eom;
    else
      iff[no_iff++] = arg;
  }
 }

  if (lp_service != NULL && message_group != NULL) {
    init_lp_service(lp_service, message_group);
  }
#ifndef MSG_VIEWER_DISABLED
  AutoMessageGroup msgs("intermediate file extraction");
#endif /* MSG_VIEWER_DISABLED */

  iffParser* parser = new iffParser;
  parser->set_output(ofile);

  if(job) {
    if(!process_job(parser, job)) {
      exit(1); // Error message already generated in process_job
    }
  }
  else if(ext)
    parser->add_file(ext, 0, iffParser::INPUT);

#if YYDEBUG
  yy_parser_debug = 1;
#endif
  for(int ind=0; ind<no_iff; ++ind){
    int code = parser->parse(iff[ind]);
    if(code)
      exit(code);
  }
  if(!purge_prefix && no_iff > 1)
    purge_prefix = "__DISCOVER_";
  if(purge_prefix)
    parser->remove_functions(purge_prefix);

  parser->extract();
  if(no_iff > 1)
    parser->transfer_ast_regions();

  parser->propagate_symbols();

  if(job){
    dump_all(parser);
  } else if(out){
    if(strcmp(out, "-")==0)
      parser->print(cout);
    else {
      ofstream ofs(out,ios::out);
      parser->print(ofs);
    }
  }
  return 0;
}

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
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include "genError.h"
#include "genWild.h"
#include "systemMessages.h"
#include "globals.h"
#include <messages.h>

void logger_start_transaction();
void logger_end_transaction();

void call_time_log(const char * nm, bool flag);

static wild_ptr brk_arr[100];
static int no_brk = 0;

static wild_ptr trc_arr[100];
static int no_trc = 0;

  static unsigned count=1;

extern "C" void gen_count()
{
    //  every diagnostic is at least a minor error;  some are
    //    crash errors as well
    globals::error_status(MINOR_ERROR);
    ++::count;
}

extern "C" void gen_count_reset() {::count=1;}

void gen_trace_level_prt(const char* name, int level, const char* sym){
   logger_start_transaction();
   int l_size = level > 0 ? level+1 : 1;
   char * spaces = new char[l_size+1];
   for(int ii=0; ii<l_size; ++ii)
     spaces[ii] = ' ';
   spaces[l_size] = 0;
   msg("stack trace: $1 $2", normal_sev) << ::count << spaces << sym << eoarg << name << eom;
   delete [] spaces;
   gen_count();
   logger_end_transaction();
}

int  gen_break_match(const char* name){
   return gen_wild_match((char*) name, brk_arr, no_brk);
}


int  gen_trace_match(const char* name){
   return gen_wild_match((char*) name, trc_arr, no_trc);
}
#define MAX_TRACE_DEPTH 200
static struct {const char* name; int stat;} trace_stack[MAX_TRACE_DEPTH];
static int trace_level = 0;
static int trace_status = -1;
static int break_status = -1;

int gen_trace_get_level() {return trace_level;}

extern "C" void gen_stack_prt(int level) {
  int max = trace_level < MAX_TRACE_DEPTH ? trace_level : MAX_TRACE_DEPTH;
   logger_start_transaction();
   if(level < 0) level += trace_level;
   if(level > trace_level - 2) level = trace_level - 2;
   for(int ii=level; ii<trace_level; ++ii){
     gen_trace_level_prt(  trace_stack[ii].name, 2, "---");
   }
   logger_end_transaction();
}

void gen_trace_set_level(int level) {
  int ii = trace_level < MAX_TRACE_DEPTH ? trace_level-1 : MAX_TRACE_DEPTH-1 ;
   for(; ii>=0; --ii){
     gen_trace_level_prt(  trace_stack[ii].name, ii, "<**");
   }

   if (level >= 0)
     trace_level = level;
}

void gen_trace_on() {trace_status=-1;}
void gen_trace_off(){trace_status=0;}

int gen_if_handler(const char * name, const char * expr)
{
  logger_start_transaction();
  msg("diagnostic: $1 ---- $2: $3", catastrophe_sev) << ::count << eoarg << name << eoarg << expr << eom;
//  gen_stack_prt (-3);
  gen_count();
  logger_end_transaction();

  return 1;
}

int gen_dbg_handler(const char*, int dbg_level)
{
    int ret = 0;
    if (trace_level > MAX_TRACE_DEPTH) {
	if (dbg_level) {
	    msg("TRACE LEVEL overflow: level $1, max $2") << trace_level << eoarg << MAX_TRACE_DEPTH << eom;
	    ;
	}
    } else
	ret = dbg_level && trace_stack[trace_level - 1].stat;

    return ret;
}

void gen_init_handler(const char* name)
{
    call_time_log(name, true);
    
    int curstat = 0;

    if(trace_level >= MAX_TRACE_DEPTH) {
	++trace_level; 
	return;
    }

    logger_start_transaction();
    trace_stack[trace_level].name = name;
    if (trace_status > 0)
	curstat = 1;
    else if (no_trc && trace_status)
	curstat = gen_trace_match(name);

    trace_stack[trace_level].stat = curstat;
    if(curstat) 
	gen_trace_level_prt(name, trace_level, "-->");
    ++trace_level;
    logger_end_transaction();

  if(break_status == 0)
      return;
  if( ! no_brk  || ! gen_break_match(name))
      return;

  msg("BREAKing in $1", warning_sev) << name << eom;

   return;
}

void gen_exit_handler(const char* name)
{
  call_time_log(name, false);

  logger_start_transaction();
  --trace_level;
  if(trace_level >=MAX_TRACE_DEPTH) return;
  const char *curname;
  int curstat = trace_stack[trace_level].stat;
  while(1){
       curname = trace_stack[trace_level].name;
       if(name == curname) break;
       msg("--- gen_exit_handler: $1: returned", error_sev) << curname << eom;
       --trace_level;
  }
  if(curstat) gen_trace_level_prt(name, trace_level, "<--");
  logger_end_transaction();
}


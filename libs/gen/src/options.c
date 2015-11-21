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
#include <string.h>
#include <signal.h>
#include <options.h>

static char * current_opt = 0;
static int    current_ind = 0;

static int main_argc;
static char ** main_argv;
extern void xref_aset_trap_handler();
extern void main_aset_trap_handler();

void set_main_options(ac, av) int ac; char **av; 
{
 main_argc = ac - 1;
 main_argv = av + 1;
}

int test_simple_option(opt) 
   const char * opt;   /* test whether this option was specified*/
{
  int ii;

  for(ii=0; ii<main_argc; ++ii){
    if(strcmp(main_argv[ii], opt)==0){
       return 1;
    }
  }
  return 0;
}

int check_main_option(opt)
const char *opt;
{
int i;

    for (i=0; i<main_argc; ++i)
    {
        if (main_argv[i][0] != '-')
            continue;
        if (strcmp(main_argv[i], opt) == 0)
            return 1;
    }
    return 0;
}

/*
 * sschmidt
 *
 * If given an opt, finds that option (assumed to start with -)
 * and return the following word.  If given NULL, return the 
 * current word if it immediately follows the desired option,
 * or resume looking through the options to find the flag.
 * Returns NULL if no more arguments to this option can be found.
 */
char *get_main_option(opt) 
  const char *opt;
{
  int ii;
  static const char *cur_opt = 0;
  static int cur_ind = 0;
  static int in_list = 0; /* Marks whether we are in the area
                             immediately following the flag */

  if (opt) {
    cur_opt = opt;
    cur_ind = 0;
    in_list = 0;
  }

  for(ii = cur_ind ; ii < main_argc ; ++ii) {
    if (main_argv[ii][0] != '-') {
      if (in_list) {
        cur_ind = ii+1;
        return main_argv[ii];
      }
    }
    else if (strcmp(main_argv[ii], cur_opt)==0) {
      if (ii == (main_argc-1)) {
        cur_ind = main_argc;
        return NULL;
      }
      else if (main_argv[ii+1][0] != '-') {
        cur_ind = ii+2;
        in_list = 1;
        return main_argv[ii+1];
      }
    }
    else {
      in_list = 0;
    }
  }
  return NULL;
}

char * get_option_from_list(no_opt, opt_arr, opt_ind, current_ind)
 int no_opt;
 const char **opt_arr;  /* array of acceptible options */

 int *opt_ind;    /* index of matching arg  */
 
 int * current_ind;       /* on input: start_search ind */
                          /* on output: next unsearched ind */


{

  int ii;
  int ind;

  for(ii=*current_ind; ii<main_argc; ++ii){
    if(main_argv[ii][0] != '-') continue;
    for(ind=0; ind < no_opt; ++ind){
      if(strcmp(main_argv[ii], opt_arr[ind])==0){
         *current_ind = ii+2;
         *opt_ind = ind;
         return main_argv[ii+1];
       }
    }
  }
  return 0;
}
#ifdef _ASET_OPTIMIZE
static void dummy_handler (int d) {}
#endif

static void quit_handler (int d)
{
    driver_exit(1);
}


/*
//------------------------------------------
// aset_set_signals
//
// Set signal handlers.
//------------------------------------------
*/

void aset_set_signals() 
{
    signal( SIGILL, main_aset_trap_handler );
#ifndef _WIN32
    signal( SIGBUS, main_aset_trap_handler );
    signal( SIGQUIT, main_aset_trap_handler );
#endif
    signal( SIGSEGV, main_aset_trap_handler );
#if defined(hp10)
    signal( SIGIOT, main_aset_trap_handler );
#endif
#ifdef _ASET_OPTIMIZE
    signal (SIGINT, dummy_handler);
#endif
}


void buildxref_set_signals()
{
    signal( SIGILL, xref_aset_trap_handler );
#ifndef _WIN32
    signal( SIGBUS, xref_aset_trap_handler );
    signal( SIGQUIT, xref_aset_trap_handler );
#endif
    signal( SIGSEGV, xref_aset_trap_handler );
#if defined(hp10)
    signal( SIGIOT, main_aset_trap_handler );
#endif
    signal (SIGINT, quit_handler);
}

/*
//------------------------------------------
// aset_reset_signals
//
// Set signal handlers.
//------------------------------------------
*/

void aset_reset_signals() 
{
    signal( SIGILL, SIG_DFL);
#ifndef _WIN32
    signal( SIGBUS, SIG_DFL);
#endif
    signal( SIGSEGV, SIG_DFL);
#if defined(hp10)
    signal( SIGIOT, SIG_DFL);
#endif
    signal (SIGINT, SIG_DFL);
}

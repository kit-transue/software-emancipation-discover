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
// not on windows #include <sys/resource.h>

#include "Args.h"
#include "cLibraryFunctions.h"
#include "customize.h"
#include "genError.h"
#include "genTry.h"
#include "machdep.h"
#include "messages.h"
#include "msg.h"
#include "options.h"
#include "psetmem.h"
#include "setprocess.h"
#include "xxinterface.h"

void create_tmp_dir();
int dd_set_hook();
void init_rel_ptrs();
int server_main(int argc, char *argv[], char *envp[]);


extern "C"
{
    void  driver_exit(int status);
    char* OSapi_setenv(const char*, const char*);

}

extern Args* DIS_args;
//extern const char* main_argv_0;
extern "C" char const *build_date;
extern "C" char const *build_version;
extern ostream* pmod_log_stream;
extern FILE *tee_file;

static char** new_argv;
static int new_argc;

void ShowLMErrorMessage()
{
    msg("Discover was unable to verify your license.\nPlease check the license server,\nthe license file, and the value of LM_LICENSE_FILE,\nor call customer support");
}


static void set_env()
{
  static const char* env_list[] = {
    "ADMINDIR",
    "sharedSrcRoot",
    "sharedModelRoot",
    };
  int sz = sizeof(env_list)/sizeof(const char*);
  for(int ii=0; ii<sz; ++ii){
    const char* var = env_list[ii];
    const char* val = OSapi_getenv((char*)var);
    if(!(val && *val)){
      val =  customize::getStrPref(var);
      if(val && *val)
	OSapi_setenv(var, val);
      else
	msg("Env. var. $1 is not set", warning_sev) << var << eom;
    }
  }
}

void print_build_date()
{
    Initialize(print_build_date);

    static int has_been_called = 0;

    if (! has_been_called) {
	genString build_date_string;
	build_date_string.printf(gettext(TXT("DISCOVER Version:        %s\nBuilt on:                %.6s %.4s %.5s\n")),
				 build_version,
				 build_date + 4, build_date + 24, build_date + 11);
	(*pmod_log_stream) << build_date_string.str() << flush;
	has_been_called = 1;
    }
}


int
init_main(int argc, char *argv[], char *envp[])
{
  // First, we do stuff that needs to be done first
  create_tmp_dir();

  char const *pset_home = OSapi_getenv ("PSETHOME");
  if (!pset_home || !pset_home[0]) {
    msg("*** DISCOVER Error: PSETHOME variable is not set. Quitting.", catastrophe_sev) << eom;
    _exit (1);
  }
    
  Initialize (main);

  // Next, we support halting on startup (maybe, for now)
  static int halt_flag;
  char const * tmp;
  if (0 != (tmp = OSapi_getenv("PSET_HALT"))) {
    halt_flag = OSapi_atoi(tmp);
    if (halt_flag > 60) halt_flag = 60;
    if ((tmp = OSapi_getenv("DISBUILD")) && !strcmp("update", tmp)) {
        halt_flag = 60;
    }
    msg("\nPlease attach debugger to process $1, go up 2, and set halt_flag to 0\n") << OSapi_getpid() << eom;
    msg("   You have $1 seconds before execution will resume...") << halt_flag << eom;
    while (halt_flag-- > 0)
      OSapi_sleep (1);
    msg("   Resuming\n") << eom;
  }

  // Next, we take care of some resource limits that we do not like.
  // In particular, we allow ourselves lots of open file handles.
#ifndef _WIN32
  struct rlimit rlm;
  getrlimit(RLIMIT_NOFILE,  &rlm);
  if (rlm.rlim_cur < 128 && rlm.rlim_max > 128) {
    rlm.rlim_cur = 128;
    setrlimit (RLIMIT_NOFILE, &rlm);
  }
#endif

  // Now we set up our stuff to deal with command line options.

  // Remember the original argv[0] so that class customize can examine it.
  //main_argv_0 = OSapi_strdup(argv[0]);

  // initialize model data structures
  dd_set_hook();
  init_rel_ptrs();
  
  // Copy arguments because X destroys some of them
  new_argv = (char**)psetmalloc((argc + 1) * sizeof(char*));
  new_argc = argc;
  for (int i = 0; i < argc; i++)
    new_argv[i] = OSapi_strdup(argv[i]);
  
  set_main_options (argc, new_argv);

  //Initialize log presentation service name and message group id. 
  char const *lp_service_name = get_main_option("-lpservice");
  if (lp_service_name) {
      char const *msg_grp_id = get_main_option("-message_group");
      if (msg_grp_id) {
          init_lp_service(lp_service_name, msg_grp_id);
      } 
  }

  DIS_args = new Args(argc, new_argv);
  customize::instance = new customize;
  set_env();
  genError* err;
  int error_code = 0;

  // Now we deal with command line options which should be honored even
  // if the licensing cannot be initialized.

  // --version
  print_build_date();
  if (check_main_option("--version"))
  {
    driver_exit (0);
  }

  char * tee_fname = get_main_option("-tee");
  if (tee_fname) {
    tee_file = fopen(tee_fname,"a");
  }

  // Now we initialize the licensing.
  if (_lc()) {
    _le();
    driver_exit(1);
  }
  
  // check out server license
  if (_lo(LIC_DISCOVER)) {
	ShowLMErrorMessage();
	driver_exit(4);
  }

  // Now we go into the real main, based on a cursory examination of
  // command line options.
  genTry {
    error_code = server_main(argc,argv,envp);
  } genCatch(err) {
    error_code = 3;
  }genEndtry;

  driver_exit (error_code);

  return error_code;
}

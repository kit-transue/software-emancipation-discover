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
// INCLUDE FILES

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <ctime>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <xxinterface.h>
#include <genString.h>
#include <gtRTL_cfg.h>
#ifndef _WIN32
#include <sys/param.h>
#endif 
#include <path.h>
#include <projectBrowser.h>
#include <X11/Xos.h>
#include <sys/resource.h>
#include <options.h>

#include "Args.h"

#define MAX_LEN 1024
#define MAX_PATH_LEN 2048

#define SWAP_REQ 140000
#define TMP_REQ 140000
#define MEMORY_REQ 32000
#define FTYPE 0
#define DTYPE 1
#define DFLT_TYPE 2
#define ABORT 3
#define WARNING 4 
#define DIAGNOSTIC 5
#define FATAL 6
#define NO_PREF 0
#define VAL_IS_YES 1
#define VAL_IS_NO 2
#define VAL_IS_EMPTY 3
#define VAL_IS_JUNK 4

extern Args* DIS_args;
extern "C" char const build_version[];

char *setenv_ifchanged( const char *, const char *, const char *);
long OSapi_freeDiskSpace(char *);
long OSapi_freeSwapSpace(void);
static int verbose_print(void);
static void strip_trailing_spaces(char *, char *);
#ifdef irix6
char *get_fileSystemType(char *);
#endif

extern int pref_found_in;

static long swap_spc = -1;
static long tmp_spc = -1;
static start_dis_flag = 0;
static bool no_pdf_option;
static bool msg_logger = 0;
static bool forgive_flag = 0;
static bool debugShowMakeRuleFlag = 0;
static bool env_save_flag = 0;
static bool tutorMode = 0;
static bool learnMode = 0;
static bool logFileNameDelete = 0;
static bool logPrefValues = 0;
static char *user_prefs_file = NULL;
static char *user_pdf_file = NULL;
static char *home_proj_pref = NULL;
static char *cppOutputFile = NULL;
static char *logFileName = NULL;

static char psethome[MAX_LEN];
static char psethome_bin[MAX_LEN];
static char psethome_lib[MAX_LEN];
static char tmpDir[MAX_LEN];
static char adminDir[MAX_LEN];
static char homeDir[MAX_LEN];
static char loginName[256];
static char host_name[256];
static char license_file[MAX_LEN];
static char session_logFile[MAX_LEN];
static char prog_name[256];

void delete_log_file()
{
  msg("---delete_log_file", normal_sev) << eom;
  if (logFileNameDelete && logFileName)
    OSapi_unlink(logFileName);
}

static int local_xerror(Display *dpy, XErrorEvent *rep)
{
  if (rep->error_code == BadValue) {
    msg("Could not set font path: BadValue error", warning_sev) << eom;
    return 1;
  }
  return 0;
}

extern int dis_setfont ();

// returns: 1 on success
//          0 on failure
int set_font_path()
{
  msg("---set_font_path", normal_sev) << eom;
  int res = dis_setfont();
  return res;
}

static int check_for_bw(void)
{
  msg("---check_for_bw", normal_sev) << eom;
  Window root = 0;
  int screen = 0;
  XWindowAttributes win_attributes;
  Display *dpy;
  char *disp = NULL;

  dpy = XOpenDisplay(disp);
  root=RootWindow(dpy, screen);
  if (!XGetWindowAttributes(dpy, root, &win_attributes)) {
    return  -1;
  }
  int depth = win_attributes.depth;
  return ((depth < 4) ? 0 : 1);
}

static char *get_user_prefs_val(const char * key)
{
  msg("---get_user_prefs_val", normal_sev) << eom;
  char *val = customize::getStrPref(key);
  msg("key $1, value $2", normal_sev) << key << eoarg << val << eoarg << eom;
  if ((val != NULL) && (pref_found_in == 0 || pref_found_in == 2))
    return val;
  else
    return NULL;
}

static int yes_or_no_pref_val(const char *key)
{
  msg("---yes_or_no_pref_val", normal_sev) << eom;
  int ret_val = NO_PREF;
  char *val = get_user_prefs_val(key);  
  if (val) {
    char new_val[MAX_LEN];
    strip_trailing_spaces(new_val, val);
    if (!strcasecmp(new_val, "yes"))
      ret_val = VAL_IS_YES;
    else if (!strcasecmp(new_val, "no"))
      ret_val = VAL_IS_NO;
    else if (!strcmp(new_val, ""))
      ret_val = VAL_IS_EMPTY;
    else
      ret_val = VAL_IS_JUNK;
  }
  return ret_val;
}

static int do_memory_check()
{
  msg("---do_memory_check", normal_sev) << eom;
  tmp_spc = OSapi_freeDiskSpace(tmpDir);
  if (tmp_spc == -1) { 
    msg("Unable to find available temporary space.", warning_sev) << eom;
    return 0;
  }
  if (tmp_spc < TMP_REQ) {
    msg("Available tmp space ($1) is less than recommended tmp space ($2).", warning_sev) << tmp_spc << eoarg << TMP_REQ << eoarg << eom;
  }
 
#if !defined(hp10)
  swap_spc = OSapi_freeSwapSpace();
  if (swap_spc == -1) 
    msg("Unable to determine available swap space.", warning_sev) << eom;
  else {
    if (swap_spc < SWAP_REQ) {
      msg("Available swap space ($1) is less than recommended swap space ($2)", warning_sev) << swap_spc << eoarg << SWAP_REQ << eoarg << eom;
    }
  }
	      
#else 
  msg ("$1: Informational: must be root to determine available swap space on HP", normal_sev) << prog_name << eoarg << eom;
#endif
    
#ifdef irix6
  char * efs = OSapi_getenv("PSET_EFS");
    if (efs) {
      if (!strcmp(efs, "1"))
	msg("$1: Enabling efs compatibility mode", normal_sev) << prog_name << eoarg << eom;
      else if (!strcmp(efs, "0"))
	msg("$1: Disabling efs compatibility mode", normal_sev) << prog_name << eoarg << eom;
      else
	msg("$1: Invalid value for PSET_EFS - must be either 0 (off) or 1 (on)", error_sev) << prog_name << eoarg << eom;
      return 0;
    }
    else {
      efs = get_fileSystemType(tmpDir);
      if (!strcmp(efs, "efs"))
	setenv_ifchanged("PSET_EFS", "1", NULL);
    }
#endif

#if defined(sun5) 
    struct rlimit rlp;
    if (!check_main_option("-core")) {
      rlp.rlim_cur = 0;
      rlp.rlim_max = 0;
      setrlimit(RLIMIT_CORE,&rlp);
    }
    rlp.rlim_cur = 256;
    rlp.rlim_max = 256;
    setrlimit(RLIMIT_NOFILE,&rlp);
#endif

  return 1;
}

void get_date_time(char *date_str)
{
 msg("---get_date_time", normal_sev) << eom;
 time_t cur_time;
 struct tm *tm_struct;
 
 time (&cur_time);
 tm_struct = localtime(&cur_time);
 strftime(date_str, 256, "%m%d%y-%T", tm_struct);
 return;
}

static void print_help_aset(void)
{
    msg("---print_help_aset", normal_sev) << eom;
    msg("Usage: discover [options]\n[-help|-?] display a brief help summary\n[-where] display installation directory\n[-version] DISCOVER version\n[-learn] start DISCOVER Learn\n[-home project] use the named project\n[-pdf project_definition_files] use the named project definition file(s)\n[-prefs preference_file] use the named preference file", normal_sev) << eom;
}

static void print_help_st_dis(void)
{
  msg("---print_help_st_dis", normal_sev) << eom;
  msg("start_discover  [-prefs prefsfilename]\nInteractively use Information Model\nprefsfilename : DIScover options selection file\n                If not set will look in order for:\n                $HOME/.psetPrefs,\n                $ADMINDIR/prefs/build.prefs", normal_sev) << eom;
}

static void print_where(void)
{
  msg("Installation directory: $1", normal_sev) << OSapi_getenv("PSETHOME") << eom;
  return;
}

static void print_which(char *exec, int argc)
{
  msg("$1", normal_sev) << exec << eom;
  if (argc > 3)
    print_help_st_dis();
  return;
}

static int check_for_arg_val(int argc, char **argv, int cur_val, int type)
{
  int next_val = cur_val + 1;
  char *next_opt = argv[next_val];

  if ((next_val >= argc) || (next_opt[0] == '-')) {
    msg("Option $1 needs an argument", error_sev) << argv[cur_val] << eom;
    return 0;
  }

  if ((type == FTYPE) || (type == DTYPE)) {
    struct OStype_stat buf;
    if (OSapi_stat (next_opt, &buf)) {
      msg("File or directory $1 does not exist.", error_sev) << next_opt << eoarg << eom;
      return 0;
    }
    switch (type) {
    case FTYPE:                      // check for regular file
      if (!S_ISREG(buf.st_mode)) {
	if (S_ISDIR(buf.st_mode))
	  msg("$1 is a directory, expected a file.", error_sev) << next_opt << eoarg << eom;
	else
	  msg("$1 is not a valid file.", error_sev) << next_opt << eoarg << eom;
	return 0;
      }
      if (!strcmp(argv[cur_val], "-license_file"))
	strcpy(license_file,argv[next_val]);
      break;
    case DTYPE:                     // check for directory type 
      if (!S_ISDIR(buf.st_mode)) {
	msg("$1 is not a valid directory.", error_sev) << next_opt << eoarg << eom;
	return 0;
      }
      break;
    default:
      break;
    }  
  }
  return 1;  
}

static int check_options(int argc, char **argv)
{
  msg("---check_options", normal_sev) << eom;
  for (int i = 1 ; i < argc; i++)
  {
    char * arg;
    arg = argv[i];
    if (arg[0] != '-')
      continue;
    arg++;
    if (!strcmp(arg, "?") || !strcmp(arg, "help") || !strcmp(arg, "h")) {
      if (start_dis_flag)
          print_help_st_dis();
      else
          print_help_aset();
      return 0;
    }
    if (!strcmp(arg, "where")) {
      print_where();
      return 0;
    }
    if (!strcmp(arg, "which")) {
      print_which(argv[0], argc);
      return 0;
    }
    if (!strncmp(arg, "vers", 4)) {
      msg("DIScover version $1", normal_sev) << build_version << eoarg << eom;
      return 0;
    }  
    if (!strcmp(arg, "envSave")) {
      env_save_flag = 1;
      continue;
    }      
    if (!strcmp(arg, "prefs") || !strcmp(arg, "userPrefs") || !strcmp(arg, "sysPrefs") || !strcmp(arg, "pdf") || !strcmp(arg, "P") ||!strcmp(arg, "license_file")  ) {
      if (!check_for_arg_val(argc, argv, i, FTYPE))
	return 0;
      else
        continue;
    }
    if (!strcmp(arg, "home") || !strcmp(arg, "dynamic_pdf") || !strcmp(arg, "r")|| !strcmp(arg, "x")) {
      if (!check_for_arg_val(argc, argv, i, DFLT_TYPE))
    	return 0;
      else
        continue;
    }
    if (!strcmp(arg, "envcheck") || !strcmp(arg, "xresource") || !strcmp(arg, "undo_forgive") || !strcmp(arg, "forgive") || !strcmp(arg, "msg_logger") || !strcmp(arg, "noUserPrefs") || !strncmp(arg, "DIS_", 4) || !strncmp(arg, "PSET_", 5) || !strcmp(arg, "core"))
      continue;

    if (!strcmp(arg, "psethome")) {
      if (!check_for_arg_val(argc, argv, i, DTYPE))
	return 0;
      else {
        setenv_ifchanged("PSETHOME", argv[i+1], NULL);
        continue;
      }
    }   
    
    if (!strcmp(arg, "exec") || !strcmp(arg, "Exec")) {
      if (!check_for_arg_val(argc, argv, i, DFLT_TYPE))
        return 0;
      else {
	char exec_path[MAX_LEN];
	strcpy(exec_path, argv[i+1]);
	strcpy(argv[i], "");
	strcpy(argv[i+1], "");
	OSapi_execv(exec_path, argv);
	msg("exec failed: $1 :$2", error_sev) << exec_path << eoarg << OSapi_strerror(errno) << eoarg << eom;
	return 0;
     }
    }
    
    if (!strncmp(arg, "d",1))
      continue;

    if (!strcmp(arg, "learn")) {
      learnMode = 1;
      continue;
    }
        
    if (!strcmp(arg, "tutor")) {
      tutorMode = 1;
      continue;
    }

    if (OSapi_getenv("NO_OPT_CHECK") == NULL) {   
      msg("Unrecognized option: $1", error_sev) << argv[i] << eom;
      return 0;
    }
  }
  return 1;
}


//set PSETHOME variable if not set
int set_psethome(char * argv_0)
{
  msg("---set_psethome", normal_sev) << eom;
  char *temp_ptr;
  char temp[MAX_LEN];
  
  temp_ptr = OSapi_getenv("PSETHOME");
  if (!temp_ptr) {
    msg("---PSETHOME environment variable is not set", normal_sev) << eom;
    char dir[MAX_LEN];
    strcpy(temp, argv_0);
    if (strchr(temp, '/') == NULL) {
      getcwd(dir, MAX_LEN);
      strcat(dir, "/");
      strcat(dir, temp);
    }
    else 
      strcpy(dir, temp);
    
    char *ptr = strrchr(dir, '/');
    if (ptr != NULL) {
      char *ptr1 = &dir[strlen(dir)-1];
      while (ptr1 != ptr) { *ptr = '\0'; ptr1--;}
      ptr1 = '\0';
    }
    if (!strcmp(dir, ""))
      getcwd(dir, MAX_LEN);
    
   int len = strlen(dir);
    if (!strcmp(&dir[len-4], "/bin"))
      dir[len-4] = '\0';
    setenv_ifchanged("PSETHOME", dir, NULL);
    strcpy(psethome, dir);
  }
  else {
    int l = strlen(temp_ptr);
    if (temp_ptr[l-1] == '/') {
      temp_ptr[l-1] = '\0';
      setenv_ifchanged("PSETHOME", temp_ptr, NULL);
    }
   strcpy(psethome, temp_ptr);
  }
  if (!strcmp(psethome, "")) {
    msg("Cannot find a suitable value for $PSETHOME.", error_sev) << eom;
    return 0;
  }
  strcpy(psethome_bin, psethome);
  strcat(psethome_bin, "/bin");
  
  strcpy(psethome_lib, psethome);
  strcat(psethome_lib, "/lib");
  
  return 1;
}

//create temp directory
int create_temp_dir(void)
{
  msg("---create_temp_dir", normal_sev) << eom;
  time_t cur_time;
  struct tm *tm_struct;
  char temp[MAX_LEN];
  char *temp1 = psetmalloc(MAX_LEN);
  int ret_val = 1;
  
  time (&cur_time);
  tm_struct = localtime(&cur_time);
  strftime(temp1, 256, ".%a.%b.%d.", tm_struct);
  strcpy(temp, tmpDir);
  strcat(temp, "/CODEINTEGRITY");
  if (temp1) strcat(temp, temp1);
  
  int num = 0;
  do {
    num++;
    sprintf(temp1, "%s%d", temp, num);
  }
  while (!OSapi_access(temp1, F_OK));
  if (OSapi_mkdir(temp1, 0777)) {
    msg("Unable to create writable temp directory: $1", error_sev) << temp << eom;
    ret_val = 0;
  }
  if (ret_val) {
    strcpy(tmpDir, temp1);
    setenv_ifchanged("TMPDIR", tmpDir, NULL);
  }

  psetfree(temp1);
  return ret_val;    
}

//set license file
void set_license_file(void)
{
  // The logic:
  // If -license_file passed on commandline, use it.
  // Else, if $LM_LICENSE_FILE specified, use it.
  // Otherwise, use $PSETHOME/lib/license.dat.
  // Nothing more!

  // license_file is file-static.  Non-empty only if -license_file option specified
  msg("---set_license_file", normal_sev) << eom;
  char temp[MAX_LEN];
  
  char *lm_file = OSapi_getenv("LM_LICENSE_FILE");

  if (!strcmp(license_file, "")) {
    if (lm_file) {	// LM_LICENSE_FILE specified
      strcpy(license_file, lm_file);
    }
    else {	// no LM_LICENSE_FILE; default to $PSETHOME/lib
      strcpy(license_file, psethome_lib);
      strcat(license_file, "/license.dat");
    }
  }
  else if (lm_file) {
    msg("Overriding license path %s", warning_sev) << lm_file << eoarg << eom;
  }
  setenv_ifchanged("LM_LICENSE_FILE", license_file, NULL);
  msg("Using license file $1", normal_sev) << license_file << eom;
}

int set_prefs_file(int& argc, char **argv)
{
  msg("---set_prefs_file", normal_sev) << eom;
  char *temp_ptr;
  char temp[MAX_LEN];
  struct OStype_stat buf;

  if (!user_prefs_file) {
    if (!(temp_ptr = get_main_option("-prefs")) && !(temp_ptr = get_main_option("-userPrefs"))) {
      msg("---No -prefs or -userPrefs option", warning_sev) << eom;
      strcpy(temp, homeDir);
      strcat(temp, "/.psetPrefs");
      if (OSapi_stat (temp, &buf)) { 
	strcpy(temp, adminDir);
	strcat(temp, "/prefs/build.prefs");
	if (OSapi_stat (temp, &buf)) {
	  msg("Unable to locate any prefs file in std locations ($HOME/.psetPrefs:$ADMINDIR/prefs/build.prefs)", error_sev) << eom;
	  return 0;
	}
      }
      if (access(temp, R_OK)) {
	msg("Unable to open prefs file ($1) for reading", error_sev) << temp << eoarg << eom;
	return 0;
      }           
      user_prefs_file = psetmalloc(MAX_LEN);
      strcpy(user_prefs_file,temp);
      setenv_ifchanged("prefsFile", temp, NULL);
    }
    else
      setenv_ifchanged("prefsFile", temp_ptr, NULL);
  }	
  else
    setenv_ifchanged("prefsFile", user_prefs_file, NULL);
  
  if (user_prefs_file) {
    argv[argc++] = strdup("-prefs");
    argv[argc++] = strdup(user_prefs_file);
    if (DIS_args) 
      delete DIS_args;
    DIS_args = new Args(argc, argv); //prefRegistry uses DIS_args, where -prefs option is checked, so update DIS_args as soon as possible 
    
  }
  msg("Using preference file $1", normal_sev) << OSapi_getenv("prefsFile") << eom;
  return 1;
}

int set_adminDir(void)
{
  msg("---set_adminDir", normal_sev) << eom;
  char *temp_ptr;
  char temp[MAX_LEN], path_var[MAX_PATH_LEN];
  
  temp_ptr = OSapi_getenv("ADMINDIR");
  if (!temp_ptr) {
    getcwd(temp, MAX_LEN);
    int len = strlen(temp);
    int len1 = strlen("AdminDir");
    if (temp[len] == '/') {
      temp[len] = '\0';
      len1++;
    }
    if (!strncmp(&temp[len-len1], "AdminDir", 8)) {
      setenv_ifchanged("ADMINDIR", temp, NULL);
      strcpy(adminDir, temp);
      msg("$ADMINDIR  environment variable unset, assuming current working dir", warning_sev) << eom;
    }
    else {
      msg("$ADMINDIR environment variable is undefined", error_sev) << eom;
      return 0;
    }
  }
  else
    strcpy(adminDir, temp_ptr);
  
  //set PATH = $PATH:$ADMINDIR/bin
  temp_ptr = OSapi_getenv("PATH");
  if (temp_ptr) {
    strcpy(path_var, temp_ptr);
    strcat(path_var, ":");
  }
  strcat(path_var, adminDir);
  strcat(path_var, "/bin");
  setenv_ifchanged("PATH", path_var, NULL);
  
  msg("Using ADMINDIR $1", normal_sev) << OSapi_getenv("ADMINDIR") << eom;
  return 1;
}

int set_learn_env(void)
{
  msg("---set_learn_env", normal_sev) << eom;
  char temp[MAX_LEN]; 
  strcpy(temp, psethome_bin);
  strcat(temp, "/Learn.setup");
  if (system(temp) < 0)
    return 0;
  if (strcmp(homeDir, "")) {
    strcpy(temp, homeDir);
    strcat(temp, "/CODEINTEGRITY/Learn/AdminDir");
    setenv_ifchanged("ADMINDIR", temp, NULL);
    user_prefs_file = psetmalloc(MAX_LEN);
    strcpy(user_prefs_file, temp);
    strcat(user_prefs_file, "/prefs/build.prefs");
    start_dis_flag = 1;
  }
  return 1;
}

void set_misc_env_vars(void)
{
 msg("---set_misc_env_vars", normal_sev) << eom;
 char *temp_ptr;
 char temp[MAX_LEN], path_var[MAX_PATH_LEN];

 /*temp_ptr = OSapi_getenv("LD_LIBRARY_PATH");
   if (temp_ptr){
   strcpy(temp, temp_ptr);
   strcat(temp, ":");
   strcat(temp, psethome_lib);
   }
   else
   strcpy(temp, psethome_lib);
   setenv_ifchanged("LD_LIBRARY_PATH", temp, NULL);
   */
 
 temp_ptr =  OSapi_getenv("SHLIB_PATH");
 if (temp_ptr) {
   strcpy(path_var, temp_ptr);
   strcat(path_var,":");
   strcat(path_var, psethome_lib);
 }
 else 
   strcpy(path_var, psethome_lib); 
 setenv_ifchanged("SHLIB_PATH", path_var, NULL);
 
 temp_ptr = OSapi_getenv("GALAXYHOME");
 if (!temp_ptr)
   setenv_ifchanged("GALAXYHOME", psethome, NULL);  
 
 strcpy(temp, psethome_lib);
 strcat(temp, "/hyperhelp");
 setenv_ifchanged("HHHOME", temp, NULL);
 setenv_ifchanged("HLPPATH", temp, NULL);
 //# New for bristol 5
 setenv_ifchanged("HHFALLBACKS", "1", NULL);
 setenv_ifchanged("HHQUIET", "1", NULL); 
 setenv_ifchanged("RCSHOME", psethome_bin, NULL);
 setenv_ifchanged("DIFFHOME", psethome_bin, NULL);
 setenv_ifchanged("PSETCFG", psethome_bin, NULL);
 strcpy(temp, psethome);
 strcat(temp, "/epoch/etc:");
 strcat(temp, psethome_bin);
 setenv_ifchanged("EMACSEXECPATH", temp, NULL);
 strcpy(temp, psethome);
 strcat(temp, "/epoch/info");
 setenv_ifchanged("EMACSINFODIR", temp, NULL);
 strcpy(temp, psethome_lib);
 strcat(temp, "/XKeysymDB");
 setenv_ifchanged("XKEYSYMDB", temp, NULL);

 
 // set color
 strcpy(temp , psethome_lib);
 strcat(temp, "/app-defaults/ParaSET-");
 temp_ptr = get_main_option("-xresource");
 if (!temp_ptr) {
   if (check_for_bw() == 0)
     strcat(temp, "bw");
   else 
     strcat(temp, "color");
 }
 else
   strcat(temp, temp_ptr);
 setenv_ifchanged("XENVIRONMENT", temp, NULL); 
}

int do_prelim_check (int& argc, char **argv)
{
 char *temp_ptr;

 temp_ptr = strrchr(argv[0], '/');
 if (temp_ptr) 
    temp_ptr++;
 else
   temp_ptr = argv[0];
 if (!strcmp(temp_ptr, "aset")) {
   start_dis_flag = 1;
   strcpy(prog_name, "start_discover");
 }
 else
   strcpy(prog_name, "temp_ptr");
 
 msg("---do_prelim_check", normal_sev) << eom;
 if (!check_options(argc, argv))
   return 0;

 if (!set_psethome(argv[0]))
   return 0;

 temp_ptr = OSapi_getenv("HOME");
 if (temp_ptr)
   strcpy(homeDir, temp_ptr);
 else
   strcpy(homeDir, "");

 temp_ptr = OSapi_getenv("USER");
 if (!temp_ptr) {
   temp_ptr = getlogin();
   if (temp_ptr) 
     strcpy(loginName, temp_ptr);
   else {
     msg("$1: USER environment variable not set", error_sev) << prog_name << eom;
     return 0;
   }
   setenv_ifchanged("USER", loginName, "");
 }
 else
   strcpy(loginName, temp_ptr);


 temp_ptr = OSapi_getenv("HOST");
 if (!temp_ptr) {
   OSapi_gethostname(host_name, 256);
   if (host_name)
     setenv_ifchanged("HOST", host_name, "");
 }
 else
   strcpy(host_name, temp_ptr);

 if (OSapi_getenv("DISPLAY") == NULL) {
   msg("$1: DISPLAY environment variable not set", error_sev) << prog_name << eom;
   return 0;
 }
 
 if (learnMode && !set_learn_env())
   return 0;

 temp_ptr = OSapi_getenv("TMPDIR");
 if (!temp_ptr) {
   strcpy(tmpDir, "/usr/tmp");
   setenv_ifchanged("TMPDIR", tmpDir, NULL);
   if (start_dis_flag)
     msg("$TMPDIR environment variable is not defined, using default (/usr/tmp)", warning_sev) << eom;
 }
 else
   strcpy(tmpDir, temp_ptr);

 if (!do_memory_check())
   return 0;

 if (!create_temp_dir()) 
   return 0;

 if (start_dis_flag && !set_adminDir())
   return 0;

 set_license_file();

 if (start_dis_flag && !set_prefs_file(argc, argv))
   return 0;
 
 set_misc_env_vars();

 return 1;
}

static void strip_trailing_spaces(char * new_str, char *old_str)
{
  msg("---strip_trailing_spaces", normal_sev) << eom;
  strcpy(new_str, old_str);
  char * ptr = &new_str[strlen(new_str)-1];
  while (isspace(*ptr)) {
    *ptr = '\0';
    ptr--;
  }
}

int verify_pdf_values(void)
{
  msg("---verify_pdf_values", normal_sev) << eom;
  char *temp_ptr, temp[MAX_LEN];
  struct OStype_stat buffer;

  temp_ptr = get_main_option("-pdf");
  no_pdf_option = (temp_ptr) ? 0 : 1;
  if (no_pdf_option) {
    user_pdf_file = get_user_prefs_val("pdfFileUser");
    if (!user_pdf_file) {
      msg("Preference value (psetPrefs.pdfFileUser) is missing", error_sev) << eom;
      return 0;
    }
  }
  
  temp_ptr = get_user_prefs_val("sharedSrcRoot");
  if (temp_ptr == NULL) {
    msg("Preference value (psetPrefs.sharedSrcRoot) is missing", error_sev) << eom;
    return 0;
  }
  strip_trailing_spaces(temp, temp_ptr);
  if (OSapi_stat (temp, &buffer) || !S_ISDIR(buffer.st_mode)) {
    msg("Preference value (psetPrefs.sharedSrcRoot) is not a valid directory", error_sev) << eom;
    return 0;
  }
  setenv_ifchanged("sharedSrcRoot", temp, NULL);
  
  temp_ptr = get_user_prefs_val("sharedModelRoot");
  if (!temp_ptr) {
    msg("Preference value (psetPrefs.sharedModelRoot) is missing", error_sev) << eom;  
    return 0;
  }
  strip_trailing_spaces(temp, temp_ptr);
  if (OSapi_stat (temp, &buffer) || !S_ISDIR(buffer.st_mode)) {
    msg("Preference value (psetPrefs.sharedModelRoot) is not a valid directory", error_sev) << eom;
    return 0;
  }
  setenv_ifchanged("sharedModelRoot", temp, NULL);
  
  temp_ptr = get_user_prefs_val("privateSrcRoot");
  if (!temp_ptr) { 
    msg("Preference value (psetPrefs.privateSrcRoot) is missing, using default value ($HOME/DISOVER/Private)", warning_sev) << eom;
    strcpy(temp, homeDir);
    strcat(temp, "/CODEINTEGRITY/Private");
  }
  else
    strip_trailing_spaces(temp, temp_ptr);

  if (OSapi_stat (temp, &buffer) || !S_ISDIR(buffer.st_mode)) {
      msg("Private src root directory is missing, creating ...", warning_sev) << eom;
      if (OSapi_mkdir(temp, 0777)) {
	msg("Private src root directory is missing, failed to create", error_sev) << eom;
	return 0;
      }
    }
  setenv_ifchanged("privateSrcRoot", temp, NULL);
  
  
  temp_ptr = get_user_prefs_val("privateModelRoot");
  if (!temp_ptr) {
    msg("Preference value (psetPrefs.privateModelRoot) is missing, using default value ($HOME/CODEINTEGRITY/Private)", warning_sev) << eom;
    strcpy(temp, homeDir);
    strcat(temp, "/CODEINTEGRITY/Private");
  }
  else
    strip_trailing_spaces(temp, temp_ptr);

  if (OSapi_stat (temp, &buffer) || !S_ISDIR(buffer.st_mode)) {
    msg("Private model root directory is missing, creating ...", warning_sev) << eom;
    if (OSapi_mkdir(temp, 0777)) {
      msg("Private model root directory is missing, failed to create", error_sev) << eom;
      return 0;
    }
  }
  setenv_ifchanged("privateModelRoot", temp, NULL);  

  return 1;
}

int set_session_logFile(void)
{
  msg("---set_session_logFile", normal_sev) << eom;
  char *temp_ptr, temp[MAX_LEN];
  struct OStype_stat buffer;
  char logFileDir[MAX_LEN];
  
  if (start_dis_flag) {
    temp_ptr = get_user_prefs_val("logFileDir");
    if (!temp_ptr) {
      strcpy(temp, adminDir);
      strcat(temp, "/log");
    }
    else
      strip_trailing_spaces(temp, temp_ptr);
    strcpy(logFileDir, temp);
  }
  else {
    strcpy(logFileDir, homeDir);
    strcat(logFileDir, "/CODEINTEGRITY");
    if (OSapi_stat (logFileDir, &buffer)) {
      if (OSapi_mkdir(logFileDir, 0777)) {
	msg("Could not create ~/CODEINTEGRITY for writing log file", error_sev) << eom;
	return 0;
      }
    }
  }

  temp_ptr = OSapi_getenv("PSET_LOGFILE");
  if (!temp_ptr) {
    msg("---PSET_LOGFILE env variable is not set", normal_sev) << eom;
    strcpy(temp, logFileDir);
    strcat(temp, "/");
    strcat(temp, loginName);
    strcat(temp, ".log");
    strcpy(session_logFile, temp);
    setenv_ifchanged("PSET_LOGFILE", temp, NULL);
  }
  else
    strcpy(session_logFile, temp_ptr);
  FILE * log_file;
  if ((log_file = fopen(session_logFile, "a")) == NULL) {
    msg("Could not open log file $1", error_sev) << session_logFile << eom;
    return 0;
  }
  if (fprintf(log_file, "Session log file:       \n") < 0) {
    msg("Could not write to the log file $1", error_sev) << session_logFile << eom;
    return 0;
  }
  fclose(log_file);

  if (start_dis_flag) {
    int NoLogFileMonitor = 0, log_monitor = VAL_IS_NO;

    log_monitor = yes_or_no_pref_val("logFileMonitor");
    if (log_monitor == VAL_IS_NO)
      NoLogFileMonitor = 1;
    else if (log_monitor == VAL_IS_JUNK || log_monitor == VAL_IS_EMPTY)
      msg("Preference value (psetPrefs.logFileMonitor) is invalid, assuming yes ...", warning_sev) << eom;
    
    int lfile_crt = yes_or_no_pref_val("logFileCreate");
    if (lfile_crt == VAL_IS_JUNK || lfile_crt == VAL_IS_EMPTY)
      msg("Preference value (psetPrefs.logFileCreate) is invalid, assuming no ...", warning_sev) << eom;

    if (lfile_crt == VAL_IS_YES) {
      int logFile_val = yes_or_no_pref_val("logFileNameFixed");
      if (logFile_val == VAL_IS_JUNK) {
	msg("Preference value (psetPrefs.logFileNameFixed) is invalid", error_sev) << eom;
	return 0;
      }
      if (logFile_val == VAL_IS_YES)
	sprintf(temp, "%s/%s.%s", logFileDir, loginName, prog_name);
      else {
	sprintf(temp, "%s/%s.", logFileDir, loginName);
	char date_str[256];
	get_date_time(date_str);
	strcat(temp, date_str);
      }
      logFileName = strdup(temp);
      if (NoLogFileMonitor) { 
	int fd = OSapi_open(logFileName, O_CREAT|O_APPEND|O_WRONLY, 0644);
	if (fd == -1) {
	  msg("Could not open log file name $1", error_sev) << logFileName << eom;
	  return 0;
	}
	OSapi_dup2(fd, 1);
	OSapi_dup2(fd, 2);
	OSapi_close(fd);
      }
    }
    else {
      if (NoLogFileMonitor) {
	int fd = OSapi_open("/dev/null", O_CREAT|O_TRUNC|O_WRONLY, 0666);
	if (fd == -1) {
	  msg("$1: Could not open log file: \"/dev/null\"", error_sev) << prog_name << eom;
	  return 0;
	}
	OSapi_dup2(fd, 1);
	OSapi_dup2(fd, 2);
      }
    }
  }  
  return 1;
}

int do_general_prefs_check(void)
{
  msg("---do_general_prefs_check", normal_sev) << eom;
  char *temp_ptr= NULL, comm[MAX_LEN];
  int pref_val = NO_PREF;
  
  pref_val = yes_or_no_pref_val("logFileUserDelete");
  if (pref_val == VAL_IS_YES)
    logFileNameDelete = 1;
  else if (pref_val == VAL_IS_JUNK)
    msg("Preference value (psetPrefs.logFileUserDelete) is invalid, assuming no ...", warning_sev) << eom; 
  
  pref_val = yes_or_no_pref_val("msgLogger");
  if (pref_val == VAL_IS_YES)
    msg_logger = 1;
  else if (pref_val == VAL_IS_JUNK || pref_val == VAL_IS_EMPTY )
    msg("Preference value (psetPrefs.msgLogger) is invalid, assuming no ...", warning_sev) << eom; 
  
  pref_val = yes_or_no_pref_val("acceptSyntaxErrors");
  if (pref_val == VAL_IS_YES)
    forgive_flag = 1;
  else if (pref_val == VAL_IS_JUNK) {
    msg("Preference value (psetPrefs.acceptSyntaxErrors) is invalid", error_sev) << eom; 
    return 0;
  }

  // get home project first either from command line or from preferences
  temp_ptr = get_main_option("-home");
  if (!temp_ptr)
    temp_ptr = get_main_option("-x");
  if (!temp_ptr)
    temp_ptr = get_user_prefs_val("projectHome");
  if (temp_ptr && strcmp(temp_ptr, "")) {
    home_proj_pref = psetmalloc(MAX_LEN);
    if (temp_ptr[0] == '/')
      strcpy(home_proj_pref, temp_ptr+1);
    else
      strcpy(home_proj_pref, temp_ptr);
  }
  
  if (env_save_flag || ((pref_val = yes_or_no_pref_val("debugEnvSave")) == VAL_IS_YES)) {
    int pid = getpid();
    sprintf(comm, "$PSETHOME/bin/misc.sh env_save $ADMINDIR/tmp/env_sh.%d $ADMINDIR/tmp/env_csh.%d", pid, pid); 
    if (system(comm) < 0)
      return 0;
  }
  else if (!env_save_flag && (pref_val == VAL_IS_JUNK || pref_val == VAL_IS_EMPTY))
    msg("Preference value (psetPrefs.debugEnvSave) is invalid, assuming no ...", warning_sev) << eom; 
  
  temp_ptr = get_user_prefs_val("debugVariablesList");
  if (temp_ptr && strcmp(temp_ptr, "")) {
    strcpy(comm, "$PSETHOME/bin/misc.sh set_debug_vars ");
    strcat(comm, host_name);
    strcat(comm, " ");
    strcat(comm, temp_ptr);
    if (system(comm) < 0)
      return 0;
  }
   
  temp_ptr = get_user_prefs_val("DIS_c.Cpp_Logfile");
  if (temp_ptr && strcmp(temp_ptr, "")) {
    cppOutputFile = psetmalloc(MAX_LEN);
    sprintf(cppOutputFile,"%s.%d", temp_ptr, getpid());
    setenv_ifchanged("PSET_CPP_FILE", cppOutputFile, NULL);
  }
  
  pref_val = yes_or_no_pref_val("debugShowMakeRuleOutput");
  if (pref_val == VAL_IS_YES) {
    debugShowMakeRuleFlag = 1;
    setenv_ifchanged("CPP_DEBUG", "true", NULL);
  }
  else if (pref_val == VAL_IS_JUNK) {
    msg("Preference value (psetPrefs.debugShowMakeRuleOutput) is invalid", error_sev) << eom; 
    return 0;
  }

  int getmodel_pref_val = yes_or_no_pref_val("getModel");
  if (getmodel_pref_val == VAL_IS_JUNK) {
    msg("Preference value (psetPrefs.getModel) is invalid", error_sev) << eom; 
    return 0;
  }
  
  int putmodel_pref_val = yes_or_no_pref_val("putModel");
  if (putmodel_pref_val == VAL_IS_JUNK) {
    msg("Preference value (psetPrefs.putModel) is invalid", error_sev) << eom; 
    return 0;
  }

  char * conf_sys = get_user_prefs_val("configuratorSystem");
  if (!conf_sys || !strcmp(conf_sys, ""))
    if (getmodel_pref_val == VAL_IS_YES || putmodel_pref_val == VAL_IS_YES) {
      msg("Preference value (psetPrefs.configuratorSystem) is invalid", error_sev) << eom;
      return 0;
    }
  if (!strcmp(conf_sys, "custom")) {
    temp_ptr = get_user_prefs_val("configuratorCustomDirectory");
    if (!temp_ptr || !strcmp(temp_ptr, "")) {
      msg("Preference value (psetPrefs.configuratorCustomDirectory) is invalid", error_sev) << eom;
      return 0;
    }
  }
	
  pref_val = yes_or_no_pref_val("logPrefValues");
  if (pref_val == VAL_IS_YES)
    logPrefValues = 1;
  else if (pref_val == VAL_IS_JUNK || pref_val == VAL_IS_EMPTY)
    msg("Preference value (psetPrefs.logPrefValues) is invalid, assuming no ...", warning_sev) << eom;
  
  return 1;
}

int do_prefs_check()
{
  msg("---do_prefs_check", normal_sev) << eom;

  int ret = 1;
  if (start_dis_flag)
    ret = verify_pdf_values();

  if (ret)
    ret = set_session_logFile();
  
  if ( ret && start_dis_flag) {
    ret = do_general_prefs_check();
    if (ret) {
      ret = verbose_print();
    }
  }
    
  return ret;
}
 
static int verbose_print(void)
{
  msg("---verbose_print", normal_sev) << eom;
  char *temp_ptr;
  
  msg("$1: General:", normal_sev) << prog_name << eom;
  msg("$1:   user                    = $2", normal_sev) << prog_name << eoarg << loginName << eom;
  time_t now;
  time(&now);
  msg("$1:   date                    = $2", normal_sev) << prog_name << eoarg << ctime(&now) << eom;
  msg("$1:   info                    = start interactive session", normal_sev) << prog_name << eom;

  msg("$1:   hostname                = $2", normal_sev) << prog_name << eoarg << host_name << eoarg << eom;
  
  msg("$1:   preferences file        = $2", normal_sev) << prog_name << eoarg << OSapi_getenv("prefsFile") << eom;
#if !defined(hp700) && !defined(hp10)
  msg("$1:   swap space              = $2k", normal_sev) << prog_name << eoarg << swap_spc << eom;
#endif
  msg("$1:   tmpdir space            = $2k", normal_sev) << prog_name << eoarg << tmp_spc << eom;
  msg("$1:   process ID              = $2", normal_sev) << prog_name << eoarg << getpid() << eom;
  msg("$1: PDF Values:", normal_sev) << prog_name << eom;
  msg("$1:   shared src root         = $2", normal_sev) << prog_name << eoarg << OSapi_getenv("sharedSrcRoot") << eom;
  msg("$1:   shared model root       = $2", normal_sev) << prog_name << eoarg << OSapi_getenv("sharedModelRoot") << eom;
  msg("$1:   private src root        = $2", normal_sev) << prog_name << eoarg << OSapi_getenv("privateSrcRoot") << eom;
  msg("$1:   private model root      = $2", normal_sev) << prog_name << eoarg << OSapi_getenv("privateModelRoot") << eom;

  msg("$1: Selected Options:", normal_sev) << prog_name << eom;
  if (home_proj_pref)
    msg("$1:   home project            = $2", normal_sev) << prog_name << eoarg << home_proj_pref << eom;
  if (cppOutputFile)
    msg("$1:   cpp output file         = $2", normal_sev) << prog_name << eoarg << cppOutputFile << eom;
  if (msg_logger)
    msg("$1:   use message logger      = yes", normal_sev) << prog_name << eom;

  msg("$1:   show make rule output   = $2", normal_sev) << prog_name << eoarg << (debugShowMakeRuleFlag ? "yes" : "no") << eoarg << eom;

  msg("$1:   accept syntax errors    = $2", normal_sev) << prog_name << eoarg << (forgive_flag ? "yes" : "no") << eoarg << eom;
  
  msg("$1:   DIScover version        = $2", normal_sev) << prog_name << eoarg << build_version << eoarg << eom;
  
  if (logPrefValues && (system("$PSETHOME/bin/misc.sh log_pref_values") < 0))
      return 0;

 return 1;
}

void append_options(int& argc, char **argv)
{
  msg("---append_options", normal_sev) << eom;
  argv[argc++] = strdup("-startEditor");
  if (start_dis_flag) {
    if (no_pdf_option) {
      for (char *p=strtok(user_pdf_file, "\t \n"); p; p=strtok(NULL, "\t \n")) {
	argv[argc++] = strdup("-pdf");
	argv[argc++] = strdup(p);
      }
    }
    if (msg_logger) 
      argv[argc++] = strdup("-msg_logger");
    if (forgive_flag)
      argv[argc++] = strdup("-forgive");
    if (home_proj_pref) {
      argv[argc++] = strdup("-home");
      argv[argc++] = strdup(home_proj_pref);
    }
    if (learnMode)
      msg("Starting Learn...", normal_sev) << eom;

    psetfree(user_prefs_file);
    psetfree(user_pdf_file);
    psetfree(home_proj_pref);
    psetfree(cppOutputFile);
  }

  if (tutorMode) {
    char tutor_file[MAX_LEN];
   
    argv[argc++] = strdup("-prefs");
    strcpy(tutor_file, homeDir);
    strcat(tutor_file, "/CODEINTEGRITY/Tutor/tutor.prefs");
    argv[argc++] = strdup(tutor_file);
    argv[argc++] = strdup("-noUserPrefs");

    argv[argc++] = strdup("-pdf");
    strcpy(tutor_file, OSapi_getenv("DISTutorHome"));
    strcat(tutor_file, "/tutor.pdf");
    argv[argc++] = strdup(tutor_file);
    msg("Starting Tutorial...", normal_sev) << eom;
  }

  set_main_options (argc, argv);
  DIS_args = new Args(argc, argv);

  msg("Installation directory: $1", normal_sev) << psethome << eom;
  msg("Session log file:       $1", normal_sev) << session_logFile << eom;  
  msg("License file:           $1", normal_sev) << license_file << eom;
  msg("Temp directory:         $1", normal_sev) << tmpDir << eom;  
}

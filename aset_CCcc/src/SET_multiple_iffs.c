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
/*
 * File contains most of the routines for supporting multiple IF file
 * generation.
 */

#ifdef SOFTWARE_EMANCIPATION_PARSER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "basic_hdrs.h"
#include "cmd_line.h"
#include "SET_multiple_iffs.h"
#include "SET_complaints.h"

#define MAX_FILENAME_SIZE 4096
#define LOCK_FILE_SUFFIX ".lck"

#define is_slash(f) (((f) == '/') || ((f) == '\\'))
#ifdef _WIN32
#define has_drive_letter(f)  (isalpha((f)[0]) && ((f)[1] == ':'))
#else
#define has_drive_letter(f) (0)
#endif  /*_WIN32 */

static FILE * GEN_LIST_FILE = NULL;

/* Converts the given string to lower case if preserve_filename_case 
   is false. */
void convert_to_lower_case(char *str)
{
  if (!preserve_filename_case) {
    char *ptr = str;
    while (*ptr != '\0') {
      *ptr = tolower(*ptr);
      ptr++;
    }
  }
}

/* Returns true if the file exists, false otherwise.
 */

static a_boolean does_file_exist(char* file)
{
  return is_regular_file(file);
}


/* Implements "mkdir -p", creates all missing directories in path a 
 * given path. Returns true if succeeds and false otherwise.
 */

static a_boolean create_dirs(char *path, int mode)
{
  a_boolean retval = TRUE;
  int i;
  char *new_path = 0;
  
  if (path && *path && retval) {
    i = strlen(path) - 1;
    /* for trailing slash */
    if (path[i] == '/' || path[i] == '\\') {
      path[i] = '\0';
      i--;
    }
    if (!is_directory(path)) {
      /* go back till the next slash */
      while(i >= 0) {
	if (path[i] == '/' || path[i] == '\\') break;
	i--;
      }
      if (i >= 0) {
	new_path = (char *)malloc(i+1);
	strncpy(new_path, path, i);
	new_path[i] = '\0';
	
	retval = create_dirs(new_path, mode);
        if (retval == TRUE) {
#ifdef _WIN32
	  if (_mkdir(path) != 0) {
#else
	  if (mkdir(path, mode) != 0) {
#endif
	    retval = FALSE;
	  }
	}
	if (new_path) free(new_path);
      }
    }
  }
  return retval;
}

/* Create non existing directories in the path for a given file.
 */

static a_boolean create_nonexisting_dirs_for(const char* file)
{
  a_boolean retval = TRUE;
  char* dirname = 0;

  if (file && *file) {
    /* fetch the directory name of the file */ 
    dirname = gs_directory_of((char *)file);

    /* create the directory if it does not already exist */
    if (dirname && !is_directory(dirname)) {
      retval = create_dirs(dirname, 0777);
    }

    /* free the space */
    if (dirname) { 
      free(dirname);
    }
  }
  return retval;
}

/* Returns $ADMINDIR/model/pset directory which will be the default 
 * base directory for placing IF files.
 */

static const char* default_base_dir(void)
{
  static char *base_dir = 0;
  const char *admindir;
  char dir[MAX_FILENAME_SIZE];

  if (!base_dir) {
    admindir = getenv("ADMINDIR");
    if (admindir) {
      strcpy(dir, admindir);
      strcat(dir, "/model/pset");
    }
    else strcpy(dir, "");

    base_dir = strdup(dir);
  }
  return base_dir;
}

/* The following function takes an absolute file name and calculates 
 * logical name for it. On Unix systems logical name is same as file 
 * name. On Windows the drive name is convert to 'driver_letter' 
 * appended with "_drive". For example: a file "c:\x\foo.c" has a 
 * logical name of "c_drive\x\foo.c".
 */

static void get_logical_name(const char* absolute_path, char *logical_name)
{
  if (absolute_path && *absolute_path) {
#ifdef _WIN32
    if (has_drive_letter(absolute_path)) {
      /* get the drive letter from absolute path */
      strncpy(logical_name, absolute_path, 1);

      /* add "_drive" to the name */
      strcat(logical_name, "_drive");

      /* add the rest of absolute path*/
      strcat(logical_name, absolute_path+2);
    } 
    else {
      strcpy(logical_name, absolute_path);
    }
#else
    strcpy(logical_name, absolute_path);
#endif
  }
  else strcpy(logical_name, "");
  
  return;
}

/* This function calculates the PSET/IF file location and name for a given
 * file with absolute path. The base directory in which all the PSET/IF files
 * should be placed can be passed as an argument to command line option 
 * --IF_file_base_dir. If IF_file_base_dir is null then 
 * $ADMINDIR/model/pset will be the default base directory. 
 * Based on iff_flag either ".pset" or ".pset.iff" suffix is added
 * to get either PSET filename or IF filename.
 */

static void get_PSET_or_IF_filename(const char* absolute_path, char* if_file, a_boolean iff_flag)
{
  char logical_name[MAX_FILENAME_SIZE] = "";  
  const char *bdir;

  /* Add the base dir first */
  if (IF_file_base_dir) {
    strcpy(if_file, IF_file_base_dir);
  } 
  else {
    bdir = default_base_dir();
    if (bdir) {
      strcpy(if_file, bdir);
    }
  }

  get_logical_name(absolute_path, logical_name);

  if (!is_slash(logical_name[0])) {
    strcat(if_file, "/");
  }

  /* Add the logical name */
  strcat(if_file, logical_name);

  /*Add IF file suffix*/
  strcat(if_file, ".pset");

  if (iff_flag) {
    strcat(if_file, ".iff");
  }

  /* If preserve_filename_case is false, convert the IF filename to 
     lower case. */ 
  convert_to_lower_case(if_file);

  return;
}


/* The following procedure creates lock file for the the specified IF file.
 * Returns TRUE if the lock file does not already exist and this succeeds in 
 * creating it, returns FALSE otherwise.
 */

a_boolean create_lock_file_for(const char *iff_filename)
{
  a_boolean ok = FALSE;
  char lock_filename[MAX_FILENAME_SIZE];
  strcpy(lock_filename, iff_filename);
  strcat(lock_filename, LOCK_FILE_SUFFIX);              /* lock filename */

  if (!does_file_exist(lock_filename)) {
    if (create_nonexisting_dirs_for(lock_filename) == FALSE) {
      ok = create_nonexisting_dirs_for(lock_filename);
    } 
    else ok = TRUE;
    if (ok) {
      int fd = open(lock_filename, O_EXCL | O_CREAT, 0777);
      if ( fd != -1) {
	close(fd);
	ok = TRUE;
      }
      else  {
        ok = FALSE;
      }
    }
  }
  return ok;
}

/* Deletes lock file of the specified IF file. */
a_boolean remove_lock_file_for(const char *iff_filename)
{
  a_boolean ret = FALSE;
  if (iff_filename) {
    char lock_filename[MAX_FILENAME_SIZE];

    strcpy(lock_filename, iff_filename);
    strcat(lock_filename, LOCK_FILE_SUFFIX);              /* lock filename */
    ret = (unlink(lock_filename) != -1) ? TRUE : FALSE;
  }
  return ret;
}

/* This procedure returns TRUE if IF file of the specified file should be
 * extracted, FALSE otherwise. The criteria for extracting IF file is
 * as follows:
 *   1. pset file should not exist (pset file existence implies the file is not 
 *                                  outdated in case of incremental model builds).
 *   2. IF file should not exist   (IF file existence implies it has already been 
 *                                  extracted for the file).
 *   3. Lock file should not exist (lock file existence implies some other 
 *                                  process is extracting this file).
 * If all of the above 3 conditions are true, then IF will be extracted by first
 * creating a lock file for it. 
 */

a_boolean extract_IF_file_for(const char *absolute_filename, char * if_filename, a_boolean is_header)
{
  a_boolean ok = FALSE;
  if (absolute_filename) {
    char tmp_filename[MAX_FILENAME_SIZE] = "";
    get_PSET_or_IF_filename(absolute_filename, tmp_filename, 0); 
    if (!does_file_exist(tmp_filename)) {
      strcat(tmp_filename, ".iff");                 
      if (!does_file_exist(tmp_filename) && create_lock_file_for(tmp_filename)) {
	strcpy(if_filename, tmp_filename);
	ok = TRUE;
      }
    }
  }
  return ok;
}


/* Adds the IF file name to the file mentioned as value of  
 * command line option "--generated_IF_files_file". The file
 * stores the IF file names of the files that will have an IF 
 * generated by the parser in the current run.  
 */
void add_to_generated_IF_files_file(char const *if_filename)
{
  static a_boolean first = TRUE;
  if (first && generated_IF_files_file) {
    GEN_LIST_FILE = fopen(generated_IF_files_file, "w");
    if (!GEN_LIST_FILE) {
      complain_str(catastrophe_csev, "Failure opening file $1", generated_IF_files_file);
    }
    first = FALSE;
  }
  if (GEN_LIST_FILE && if_filename) {
    fprintf(GEN_LIST_FILE, "%s\n", if_filename);
    fflush(GEN_LIST_FILE);
  }
}

void close_generated_IF_files_file(void)
{
  if (GEN_LIST_FILE) {
    close(GEN_LIST_FILE);
  }
}

#endif /*SOFTWARE_EMANCIPATION_PARSER*/


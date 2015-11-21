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
  configurator.c - mainline to a program invoked by ParaSet to control
                   user-definable processes, such as revision control etc.

  this file also includes some common functions needed by such code

  The way configurator and configshell work is the following: when
  configuration manager command is issued from some menu, aset starts
  configshell, and that process remains active during the rest of the
  discover session.  Then, for each configuration management command issued
  by the user (Manage->Copy, Manage->Put, Manage->Merge), a configurator
  process is spawned. The configurator executes the command by calling
  script(s), and reads and processes the output of the command. The processed
  result is sent back to configshell via pipe. Configshell may use this
  result to spawn configurator again, for another command. It communicates
  the results to aset using another pipe.
*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <varargs.h>
#include <utime.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

static int  has_save_file;
static char *save_file;
static char temp_save_file[MAXPATHLEN];

FILE *pipefile = 0;

static char *bin_path;
static char *xterm_input = 0;

static char *cm_bugno         = 0;
static char *cm_comment       = 0;
static char *cm_c_file        = 0; /* cm_c_file is used by put_pset only */
static char *cm_date          = 0;
static char *cm_description   = 0;
static char *cm_files         = 0;
static char *cm_file_type     = 0;
static char *cm_local_proj    = 0;
static char *cm_options       = 0;
static char *cm_pset_search   = 0;
static char *cm_revision_file = 0;
static char *cm_sysfile       = 0;
static char *cm_sys_proj      = 0;
static char *cm_version       = 0;
static char *cm_workfile      = 0;
static char *cm_rmpset        = 0;

#define MAX_SETENV 100
static char *setenv_buf[MAX_SETENV ];
static int setenv_idx = 0;

/*
  define
*/
#define SCRIPT_CM_COPY_PSET     "cm_copy_pset"
#define SCRIPT_CM_COPY_SRC      "cm_copy_src"
#define SCRIPT_CM_CREATE        "cm_create"
#define SCRIPT_CM_CREATE_DIR    "cm_create_dir"
#define SCRIPT_CM_CREATE_FILE   "cm_create_file"
#define SCRIPT_CM_DELETE_SRC    "cm_delete_src"
#define SCRIPT_CM_DELETE_PSET   "cm_delete_pset"
#define SCRIPT_CM_DIFF          "cm_diff"
#define SCRIPT_CM_GET_SRC       "cm_get_src"
#define SCRIPT_CM_GET_COMMENT   "cm_get_comment"
#define SCRIPT_CM_LOCK          "cm_lock"
#define SCRIPT_CM_OBSOLETE_SRC  "cm_obsolete_src"
#define SCRIPT_CM_OBSOLETE_PSET "cm_obsolete_pset"
#define SCRIPT_CM_PUT_END       "cm_put_end"
#define SCRIPT_CM_PUT_PSET      "cm_put_pset"
#define SCRIPT_CM_PUT_SRC       "cm_put_src"
#define SCRIPT_CM_FAST_PUT_SRC  "cm_fast_put_src"
#define SCRIPT_CM_FAST_MERGE_SRC "cm_fast_merge_src"
#define SCRIPT_CM_PUT_START     "cm_put_start"
#define SCRIPT_CM_SETUP_SRC     "cm_setup_src"
#define SCRIPT_CM_SETUP_PSET    "cm_setup_pset"
#define SCRIPT_CM_SRC_VERSION   "cm_src_version"
#define SCRIPT_CM_UNGET_SRC     "cm_unget_src"
#define SCRIPT_CM_UNGET_PSET    "cm_unget_pset"
#define SCRIPT_CM_UNLOCK        "cm_unlock"
#define SCRIPT_CM_VERSION       "cm_version"
#define SCRIPT_CM_VERSION_LIST  "cm_version_list"
#define SCRIPT_CM_VERSION_DATE  "cm_version_date"
#define SCRIPT_CM_IS_LOCKED     "cm_is_locked"

/*
  If you echo one message which is longer than 4000
  characters in the scripts, enlarge the next declaration.
*/
#define MAX_LEN 4000

/***************************************************************************
  interface to this executable:
  argc will be at least 2, and argv will contain all the input arguments
  argv[1] is the command string, such as "lock", "unlock", ...
  argv[2] is the user-option
  the remaining arguments, if any, are specified and parsed by the individual
        commands
  results are returned as stdout, which is being captured by the parent
        process.  That process pays attention only to lines beginning with
        two percent signs "%%", which are nominally "errors".  In the case of
        the stat command, the return string begins "%%stat;" and must be the
        only line beginning that way.
  For all commands that fail, at least one such %% line must be output, as
        this is the flag currently used by Paraset to indicate an error.
*************************************************************************/

#if defined(sun5) || defined(hp700)

static int getdtablesize()
{
    return sysconf(_SC_OPEN_MAX);
}

static char *getwd(pathname)
char *pathname;
{
    return getcwd(pathname, MAXPATHLEN);
}

#endif


/************************************************************************
  exit_with_code
  Clean up and exit.
************************************************************************/

static void exit_with_code(exitcode)
int exitcode;
{
    printf("==============================================================\n");
    exit(exitcode);
}

/************************************************************************
  error

  Display error message, preceded by "%%" and followed by newline. Accept
  one optional string argument to be inserted where the %s occurs in the
  original string.

  Note: This function is called both with the arg1, and without it. It
  counts on the fact that sprintf() won't use its argument if there's
  no % specifier in the format string.
************************************************************************/

static void error(string, arg1)
char *string;
char *arg1;
{
    char buffer[1024];
    sprintf(buffer, string, arg1);

    fprintf(pipefile,"%s", buffer);
    fflush(pipefile);
}

/************************************************************************
  cfg_splitpath:

  Given a complete filespec, parse out the first part as the pathpart
  (not including the trailing slash) then take the last part as filepart

  If there was no slash to parse with, the file is assumed to
  be in the current directory, and the whole string is the filepart,
  with the pathpart being "."
************************************************************************/

static void cfg_splitpath(filespec, pathpart, filepart)
char *filespec;
char *pathpart;
char *filepart;
{
    char *marker;
    strcpy(pathpart,filespec);
    marker = strrchr(pathpart, '/');

    if (!marker)
    {
        strcpy(pathpart,".");
        strcpy(filepart, filespec);
    }
    else
    {
        *marker = 0;
        strcpy(filepart,marker+1);
    }
}

/************************************************************************
  cfg_save_file
************************************************************************/

static void cfg_save_file(file)
char *file;
{
    struct stat stat_buf;
    char command[MAXPATHLEN*3];
    char sourcepathpart[MAXPATHLEN*3];
    char sourcefilepart[MAXPATHLEN*3];

    if (strlen(file))
    {
        int write_flag = 1;
        struct stat buf0;
        int status0 = stat(file,&buf0);
        int own_flag;
        if (status0 == 0)
        {
            own_flag = chmod(file,buf0.st_mode) == 0;
            if (!(own_flag || !own_flag && (S_IWGRP&buf0.st_mode)))
                write_flag = 0; /* no write priviledge for the source file */
        }
        else
            write_flag = 0; /* not exist */
        if (write_flag)
        {
            cfg_splitpath(file, sourcepathpart, sourcefilepart);
            save_file = file;
            sprintf(temp_save_file, "%s/#%s#~", sourcepathpart, sourcefilepart);

            sprintf(command, "rm -f %s;cp -p %s %s", temp_save_file, file, temp_save_file);
            isystem(command);
            has_save_file = 1;
        }
        else
            has_save_file = 0;
    }
}

/************************************************************************
  cfg_restore_files
************************************************************************/

static void cfg_restore_file()
{
    char command[MAXPATHLEN*3];

    if (has_save_file)
    {
        sprintf(command, "rm -f %s; mv %s %s", save_file, temp_save_file, save_file);
        isystem(command);
    }
}

/************************************************************************
  cfg_clean_files
************************************************************************/

static void cfg_clean_file()
{
    if (has_save_file)
        unlink(temp_save_file);
}

static int is_command(command, cmd)
char *command;
char *cmd;
{
    int ret_status;
    char *p;
    char *q = strchr(command,' ');
    if (q)
        *q = '\0';
    p = strrchr(command,'/');
    if (p)
        p++;
    else
        p = command;
    if (strcmp(p, cmd) == 0)
        ret_status = 1;
    else
        ret_status = 0;

    if (q)
        *q = ' ';

    return ret_status;
}

static int child_exit_status;
static void handler ()
{
    int pid;
    int options = WNOHANG;
    int status;

    pid = waitpid( -1, &status, options);

    child_exit_status = WEXITSTATUS (status);
}

static int display_flag; /* use by diff */
static int file_not_in_src_system = 0; /* use by diff */

/*
  The function executes "command" (through statement popen) and then reads
  and processes the output produced by the "command". The commands are
  invocations of various configuration manager scripts, for example
  cm_copy_src, cm_copy_pset etc. The processing consists in writing static
  variables cm_sysfile and cm_revision_file. open_command and "command"
  processes communicate through the pipe err_file.

  After "command" has finished executing, the result (variables cm_sysfile
  and cm_workfile) will be sent to configshell process. This is done using
  function send_back_files.
*/

static int open_command(command)
char *command;
/* execute one config-command */
{
    FILE *err_file;
    FILE *ver_list_file = 0;
    char line[MAX_LEN];
    int  idx;
    int got_an_error = 0;
    char tnm[L_tmpnam+1];
    char tnm0[L_tmpnam+1];
    char *origin_cmd = command;

    int diff_command         = is_command(command,SCRIPT_CM_DIFF);
    int setup_command        = is_command(command,SCRIPT_CM_SETUP_SRC) ||
                               is_command(command,SCRIPT_CM_SETUP_PSET);
    int version_command      = is_command(command,SCRIPT_CM_VERSION) ||
                               is_command(command,SCRIPT_CM_SRC_VERSION);
    int version_date_command = is_command(command,SCRIPT_CM_VERSION_DATE);
    int version_list_command = is_command(command,SCRIPT_CM_VERSION_LIST);
    int comment_command	     = is_command(command,SCRIPT_CM_GET_COMMENT);
    int echo_command         = !setup_command && !version_command
                               && !version_list_command
                               && !version_date_command
                               && !comment_command
                               && !(diff_command && !display_flag);
    int copy_command         = is_command(command, SCRIPT_CM_COPY_SRC) ||
	                       is_command(command, SCRIPT_CM_COPY_PSET);

    char buf[1024];
/*
  We have special, faster processing for clearcase checkout command; this
  string serves to recognize it.
*/
    char* clearcase_signature = "from version \"";
    int clearcase_signature_len = strlen(clearcase_signature);

    if (diff_command && file_not_in_src_system)
    {
        error("%%%%stat;d=1;\n");
        return 0;
    }

    if(xterm_input)
    {
        sprintf(buf, "%s < %s", command, xterm_input);
        command = buf;
    }

    /* accept the stderr */
    tmpnam(tnm0);
    sprintf(buf, "%s 2> %s", command, tnm0);
    command = buf;

    signal (SIGCHLD, handler);
    child_exit_status = -12345;
    err_file = popen(command, "r");
    if (err_file)
    {
        int fd = fileno(err_file);
        int end_of_file = 0;

        int first_setup_file = 1;

        while(1)
        {
            for (idx = 0;idx < MAX_LEN; idx++)
            {
            readAgain:
                errno = 0;
                if (read(fd, &line[idx], 1) == 1)
                {
                    if (echo_command)
                    {
                        fprintf(stderr,"%c",line[idx]);
                        fflush(stderr);
                    }
                    if (line[idx] == '\n')
                    {
                        char *err;
			char* aname = NULL;
			char* clearcase_version_number = NULL;
                        line[idx+1] = '\0';
/*
  Here is the purpose of this part of code: we don\'t want to call
  cm_setup_src for clearcase because it is slow. However we need to provide
  version number (i.e. full version file name) in variable cm_revision_file
  for aset. Previously we used cm_setup_src to do that; now we have to figure
  out the name from clearcase checkout output. The following if does
  precisely that: just puts together full clearcase file name from the output
  of clearcase checkout command (hopefully atria won\'t change output format
  for that command).
*/
			if (copy_command)
			{
			    aname = strstr(line, clearcase_signature);
			    if (aname != NULL)
			    {
				clearcase_version_number = strdup(aname)+clearcase_signature_len;
				aname = strrchr(clearcase_version_number, '\"');
				if (aname != NULL)
				    aname[0] = '\0';
				cm_revision_file = malloc(strlen(cm_sysfile)+2+strlen(clearcase_version_number)+1);
				strcpy(cm_revision_file, cm_sysfile);
				strcat(cm_revision_file, "@@");
				strcat(cm_revision_file, clearcase_version_number);
			    }
			}
                        if ((int)(err = strstr(line, "%%")) && !diff_command)
                        {
                            error("%s", err);
                            got_an_error = 1;
                        }
                        else if (setup_command)
                        {
                            line[idx] = '\0';
                            if (first_setup_file)
                            {
                                first_setup_file = 0;
                                cm_sysfile = strdup(line);
                            }
                            else
                                cm_revision_file = strdup(line);
                        }
                        else if (version_command)
                        {
                            line[idx] = '\0';
                            error("%%%%stat;v=%s;\n", line);
                        }
                        else if (version_list_command || version_date_command || comment_command )
                        {
                            if (!ver_list_file)
                            {
                                tmpnam(tnm);
                                ver_list_file = fopen(tnm,"w");
                            }
                            if (comment_command) {
                                char* comment_line;
                                comment_line = line;

                                /* Skip any leading blanks. */
                                while (comment_line && (*comment_line == ' ') )
                                    comment_line++;

                                /* Skip leading quote */
                                if (comment_line && (*comment_line == '"') )
                                    comment_line++;

                                /* Comments may end with '"\n', need to remove the quote. */
                                if (line[idx-1] == '"') {
                                    line[idx-1] = '\n';
                                    line[idx] = '\0';
                                }

                                /* Write out the comment. */
                                fprintf(ver_list_file,"%s", comment_line);
                            } else {
                                fprintf(ver_list_file,"%s", line);
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if (errno == EINTR)
                        goto readAgain;
                    end_of_file = 1;
                    break;
                }
            }
            if (end_of_file)
	    {
                break;
	    }
        }
        pclose(err_file);
    }
    else
    {
        error("%%%%\"%s\" could not be executed\n",origin_cmd);
        got_an_error = 1;
    }

    if (ver_list_file)
    {
        fclose(ver_list_file);
        /* don\'t unlink tnm which will be deleted in ParaSET */
        error("%%%%%s\n",tnm);
    }

    /* dump all errors */
    if (!version_command && !ver_list_file && !comment_command)
    {
        err_file = fopen(tnm0,"r");
        while(fgets(line,MAX_LEN-1,err_file))
        {
            error("%%%%%s",line);
            fprintf(stderr,"%s",line);
            fflush(stderr);
        }
        fclose(err_file);
    }

    unlink(tnm0);

    if (child_exit_status)
        got_an_error = 1;

    if (diff_command)
    {
        if (got_an_error)
        {
            error("%%%%stat;d=1;\n");
            got_an_error = 0;
        }
    }

    return got_an_error;
}

/************************************************************************
  isystem

  Construct and execute a system command from an argument list.  The
  argument list is constructed exactly as you would for printf()

  input:  format string, and zero or more arguments (numeric or string)
  output: error code.  (0 for success)
************************************************************************/

static int isystem(va_alist)
va_dcl
{
    char command[MAXPATHLEN*3];
    va_list args;
    char *format;
    int got_an_error;

    va_start(args);
    format = va_arg(args, char *);
    vsprintf(command, format, args);
    va_end(args);
    if (getenv("PSET_DEBUG_CONFIGURATOR"))
    {
        fprintf(stderr,"((debug cmd> %s))\n", command);
        fflush(stderr);
    }

    got_an_error = open_command(command);

    return (got_an_error);
}

static void setenv(name, val)
char *name;
char *val;
{
    int len = strlen(name) + strlen(val) + 2;
    char *cmd = (char *)malloc(len);
    setenv_buf[setenv_idx++] = cmd; /* to be freed later */
    if (setenv_idx == MAX_SETENV)
    {
        fprintf(stderr,"The buffer size (%d) for setenv is not enough in configurator.c\n",setenv_idx);
        exit(1);
    }
    sprintf(cmd,"%s=%s",name,val);
    putenv(cmd);
}

/************************************************************************
  cfg_command
************************************************************************/

static int cfg_command(command, backup)
char *command;
int backup;
{
    int status;
    int io_flag = 0;
    char *cmd = 0;

    if (backup &&
        (strncmp(command,SCRIPT_CM_GET_SRC,   strlen(SCRIPT_CM_GET_SRC))   == 0 ||
         strncmp(command,SCRIPT_CM_COPY_SRC,  strlen(SCRIPT_CM_COPY_SRC))  == 0 ||
         strncmp(command,SCRIPT_CM_FAST_PUT_SRC,   strlen(SCRIPT_CM_FAST_PUT_SRC))  == 0 ||
         strncmp(command,SCRIPT_CM_PUT_SRC,   strlen(SCRIPT_CM_PUT_SRC))   == 0))
    {
        io_flag = 1;
        cfg_save_file(cm_workfile);
    }

    status = isystem("%s/%s", bin_path, command);

    if (io_flag)
    {
        if (status)
            cfg_restore_file();
        else
            cfg_clean_file();
    }

    return status;
}

/*
    input: opt is a text which consists of all switchs and the values followed these switchws
           sw_ptn is the switch name
    output: ret which will contain the value followed this switch
*/
static void get_option(opt, sw_ptn, ret)
char *opt;
char *sw_ptn;
char **ret;
{
    char *p;
    if (p = strstr(opt, sw_ptn))
    {
        char *src_p = p;
        char *pp;
        p += strlen(sw_ptn);
        while(p[0] == ' ')
            p++;
        p = strdup(p);
        pp = strchr(p, ' ');
        if(pp)
        {
            *pp = '\0';
            /* remove this option out the origin one */
            strcpy(src_p,pp+1);
        }
        else
            *src_p = '\0';
    }
    *ret = p;
}

static int find_option(opt, sw_ptn)
char *opt;
char *sw_ptn;
{
    return (strstr(opt, sw_ptn) ? 1 : 0);
}

static void define_env()
{
    setenv("cm_bugno",        cm_bugno);
    setenv("cm_comment",      cm_comment);
    setenv("cm_c_file",       cm_c_file);
    setenv("cm_date",         cm_date);
    setenv("cm_description",  cm_description);
    setenv("cm_files",        cm_files);
    setenv("cm_local_proj",   cm_local_proj);
    setenv("cm_options",      cm_options);
    setenv("cm_pset_search",  cm_pset_search);
    setenv("cm_revision_file",cm_revision_file);
    setenv("cm_sysfile",      cm_sysfile);
    setenv("cm_sys_proj",     cm_sys_proj);
    setenv("cm_version",      cm_version);
    setenv("cm_workfile",     cm_workfile);
    setenv("cm_rmpset",       cm_rmpset);
}

static void send_back_files(cm_sysfile,cm_revision_file)
char *cm_sysfile;
char *cm_revision_file;
{
    char buf[1024];
    sprintf(buf,"sf=%s;vf=%s;",cm_sysfile,cm_revision_file);
    error("%%%%stat;%s\n", buf);
}

static void send_back_noexist()
{
    error("%%%%stat;not_exist=1;\n");
}

main(argc, argv)
int argc;
char **argv;
/************************************************************************
  main
  The main entry point of this code.
*************************************************************************/
{
    int  i;
    int  backup = 1;
    int  status = 0;

    char *comment;
    char *opt = strdup(argv[2]);
    char *p;
    char *pset_search;
    char sourcefilepart[MAXPATHLEN*3];
    char sourcepathpart[MAXPATHLEN*3];
    char *ver;
    char *wd, *wf, *sd, *sf;

/*
  Auxiliary data used to determine whether we are using clearcase
  configuration manager. The purpose is to speed up checkout for clearcase.
  See details below. Note that the detection of clearcase is based on string
  "clearcase" being somewhere in the PSETCFG (which is the path for cm_*
  scripts). If clearcase scripts are in directory which does not have
  "clearcase" in the path, there will be no performance improvement for
  checkout, but the functionality will of course be there.
*/
    char* clearcase;
    char* clearcase_signature = "clearcase";

    int sleeper = 0;
    int halt_flag= 1000;
    double floater= 0.0;
    char* dbg_env = getenv("PSET_DEBUG_CONFIGURATOR");
    char* dbg_env_time= 0;
    if ( dbg_env )
    {
	dbg_env_time = strchr(dbg_env, ':' );
	if ( dbg_env_time )
	{
	    *dbg_env_time = 0;
	    halt_flag= atoi(dbg_env_time+1);
	}
	if ( *dbg_env == '*' || strstr(argv[1], dbg_env) )
	    sleeper = 1;
	if ( dbg_env_time )
	    *dbg_env_time = ':';
	if ( sleeper )
	{
	    fprintf(stderr,"\nPlease attach debugger to proccess %d and set halt_flag to 0\n", getpid());
	    fprintf(stderr, "You have %d secunds before execution will resume...\n", halt_flag );
	    while ( halt_flag-- >0 )
	    {
		for (sleeper=1000000; sleeper > 0 ; sleeper-- )
		{
		    if ( halt_flag == 0 ) break;
		    floater = 3.0 * 4.0;
		}
	    }
	    fprintf(stderr, "\nResuming..\n");
	}
    }

    if (argc > 1 && strncmp(argv[argc-1], "-handle", 7) == 0)
    {
        int pipehandle = atoi(argv[argc-1]+7);
        if (pipehandle>2 && pipehandle<getdtablesize())
        {
            argc--;
            pipefile = fdopen(pipehandle,"w");
        }
        else
            pipefile = stderr;
    }
    else
        pipefile = stderr;

    /* setup bin_path */
    bin_path = strdup(getenv("PSETCFG"));
/*
  Determine whether configuration manager is clearcase or not.
*/
    clearcase = strstr(bin_path, clearcase_signature);

    if(find_option(opt, "-RM_PSET")){
	char *p   = strstr(opt, "-RM_PSET");
	cm_rmpset = strdup("YES");
	if(p){
	    char *pp = p + strlen("-RM_PSET");
	    strcpy(p, pp);
	}
    }
    else
	cm_rmpset = strdup("");
    if (strcmp(argv[1], "fast_merge_src") == 0 || strcmp(argv[1], "fast_put_src") == 0 || 
	strcmp(argv[1], "put_src") == 0 || strcmp(argv[1], "put_pset") == 0)
    {
        sf = argv[5];
        wf = argv[4];
        comment = strdup(argv[3]);
    }
    else
    {
        if (strcmp(argv[1], "put_start") && strcmp(argv[1], "put_end") && strcmp(argv[1], "create_file"))
        {
            sf = argv[3];
            wf = argv[4];
        }
        if (strcmp(argv[1], "get_src") && strcmp(argv[1], "copy_src"))  
	    comment = strdup("");
	else comment = strdup(argv[5]);
    }
	
    if (strcmp(argv[1], "put_pset") == 0)
        cm_c_file = strdup(argv[6]);
    else if (strcmp(argv[1], "copy_pset") == 0)
        cm_c_file = strdup(argv[5]);
    else
        cm_c_file = strdup("");

    if (strcmp(argv[1], "get_ver") == 0)
        cm_date = strdup(argv[5]);
    else
        cm_date = strdup("");

    display_flag = 0;
    if (strcmp(argv[1], "diff") == 0)
    {
        if (strcmp(argv[5],"1") == 0)
            display_flag = 1;
    }

    if (strcmp(argv[1], "put_start") && strcmp(argv[1], "put_end") && strcmp(argv[1], "create_file"))
    {
        cm_workfile = strdup(wf);

        cfg_splitpath(wf, sourcepathpart, sourcefilepart);
        wd = strdup(sourcepathpart);
        wf = strdup(sourcefilepart);
        cfg_splitpath(sf, sourcepathpart, sourcefilepart);
        sd = strdup(sourcepathpart);
        sf = strdup(sourcefilepart);
    }
    else
    {
        if (strcmp(argv[1], "create_file") == 0)
            cm_workfile = strdup(argv[3]);
        else
            cm_workfile = strdup("");
    }

    if (strcmp(argv[1], "unlock") == 0)
    {
        free(comment);
        get_option(opt, "-ParaUnlock", &comment);
    }

    get_option(opt, "-ParaRev", &ver);
    if (!ver)
        ver = strdup("");

    cm_file_type = strdup(argv[7]);
    setenv("cm_file_type", cm_file_type);

    /* cm_sysfile and cm_revision_file are defined by the next statement */
    cm_revision_file = strdup("");
    if (strcmp(argv[1], "copy_pset") && strcmp(argv[1], "put_pset"))
    {
        if (strcmp(argv[1], "delete_pset") && strcmp(argv[1], "create_dir") &&
            strcmp(argv[1], "put_start")   && strcmp(argv[1], "put_end")    &&
            strcmp(argv[1], "create_file") && strcmp(argv[1], "fast_merge_src"))
        {
/*
  If clearcase is the configuration manager, than we don\'t need version for
  cm_sysfile; we can just use cm_workfile. Therefore we can skip expensive
  call to SCRIPT_CM_SETUP_SRC. Note, however, that this performance
  improvement relies on clearcase scripts being in directory which has string
  "clearcase" somewhere in its path. If that is not the case, there will be
  no performance improvement. But the checkout will of course work.

  Exactly the same logic is applied for pset file checkout below.

  Important addition: this can only be done if initially cm_sysfile and
  cm_workfile are identical when entering the script - in other words, if
  argv[3] and argv[4] are the same.  If they are not, then the copy_src
  command is issued from Manage->Merge, and cm_sysfile may not be copied to
  cm_workfile.
*/
	    status = 0;
	    if (clearcase == 0 || (strcmp(argv[1], "copy_src") || strcmp(argv[3], argv[4])))
	    {
		isystem("%s/%s %s %s %s %s %s %s", bin_path, SCRIPT_CM_SETUP_SRC, wd, wf, sd, sf, argv[1], opt);
		status = child_exit_status;
	    }
	    else
		cm_sysfile = strdup(cm_workfile);
        }
        else
        {
            cm_revision_file = strdup("");
            cm_sysfile       = strdup("");
        }
    }
    else
    {
	status = 0;
	if (clearcase == 0 || (strcmp(argv[1], "copy_pset") || strcmp(argv[3], argv[4])))
	{
	    isystem("%s/%s %s %s %s %s %s %s", bin_path, SCRIPT_CM_SETUP_PSET, wd, wf, sd, sf, argv[1], opt);
	    status = child_exit_status;
	}
	else
	    cm_sysfile = strdup(cm_workfile);
    }

    if (strcmp(argv[1], "put_start") && strcmp(argv[1], "put_end") && strcmp(argv[1], "create_file"))
    {
        /* free the space */
        free(sd);
        free(sf);
        free(wd);
        free(wf);

        if (cm_sysfile == 0 || cm_revision_file == 0)
            exit(1);
    }

    if (status == 1)
    {
        error("%s fails\n",SCRIPT_CM_SETUP_SRC);
        return;
    }

    if (status == 99)
        file_not_in_src_system = 1;

    get_option(opt, "-ParaXterm", &xterm_input);

    cm_description = comment;
    cm_options     = opt;
    cm_version     = ver;

    get_option(opt, "-pset_search", &pset_search);
    if (pset_search == 0)
        cm_pset_search = strdup("");
    else
    {
        cm_pset_search = strdup("Y");
        free(pset_search);
    }

    if (getenv("PSET_DEBUG_CONFIGURATOR"))
    {
        int i;
        fprintf(stderr,"argc=%d  $Date: 1998/09/22 18:44:53EDT $\n",argc);
        for (i=0;i<argc;i++)
            fprintf(stderr,"argv[%d]=%s\n",i,argv[i]);
        fflush(stderr);
    }

    if (strcmp(argv[1], "put_start") == 0 || strcmp(argv[1], "put_end") == 0)
    {
        cm_bugno   = strdup(argv[3]);
        cm_comment = strdup(argv[4]);
        cm_files   = strdup(argv[5]);
        get_option(opt, "-local_proj", &cm_local_proj);
        get_option(opt, "-sys_proj",   &cm_sys_proj);
    }
    else
    {
        if (find_option(opt, "-sys_proj"))
            get_option(opt, "-sys_proj",   &cm_sys_proj);
        else
            cm_sys_proj = strdup("");
        if (find_option(opt, "-local_proj"))
            get_option(opt, "-local_proj",   &cm_local_proj);
        else
            cm_local_proj = strdup("");
        cm_bugno      = strdup("");
        cm_comment    = strdup("");
        cm_files      = strdup("");
    }

    define_env();

    if (argc >= 7)
    {
        argc -= 3;

        if (argc == 5)
        {
            if (strcmp(argv[1], "copy_pset") == 0)
            {
                if (status == 99) /* not exist */
                {
                    send_back_noexist();
                    exit_with_code(0);
                }
                else
                    exit_with_code(cfg_command(SCRIPT_CM_COPY_PSET, backup));
            }
            else if (strcmp(argv[1], "copy_src") == 0)
            {
		int return_code = cfg_command(SCRIPT_CM_COPY_SRC, backup);
                if (status != 1)
                    send_back_files(cm_sysfile,cm_revision_file);
                exit_with_code(return_code);
            }
            else if (strcmp(argv[1], "create_dir") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_CREATE_DIR, backup));
            else if (strcmp(argv[1], "create_file") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_CREATE_FILE, backup));
            else if (strcmp(argv[1], "delete_pset") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_DELETE_PSET, backup));
            else if (strcmp(argv[1], "delete_src") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_DELETE_SRC, backup));
            else if (strcmp(argv[1], "diff") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_DIFF, backup));
            else if (strcmp(argv[1], "get_src") == 0)
            {
                if (status != 1)
                    send_back_files(cm_sysfile,cm_revision_file);

                exit_with_code(cfg_command(SCRIPT_CM_GET_SRC, backup));
            }
            else if (strcmp(argv[1], "get_comment") == 0)
            {
                /*
                  cm_get_comment should return a file name
                  which contains the comment.
                */
                exit_with_code(cfg_command(SCRIPT_CM_GET_COMMENT, backup));
            }
            else if (strcmp(argv[1], "get_ver") == 0)
            {
                /*
                  cm_version should return a file name
                  which contains all interesting versions.
                */
                if (status == 0)
                {
                    if (strlen(cm_date))
                        exit_with_code(cfg_command(SCRIPT_CM_VERSION_DATE, backup));
                    else
                        exit_with_code(cfg_command(SCRIPT_CM_VERSION_LIST, backup));
                }
                else
                    exit_with_code(0);
            }
            else if (strcmp(argv[1], "lock") == 0)
            {
                if (status == 0)
                    exit_with_code(cfg_command(SCRIPT_CM_LOCK, backup));
                else
                    exit_with_code(0);
            }
            else if (strcmp(argv[1], "is_locked") == 0)
            {
                if (status == 0)
                    exit_with_code(cfg_command(SCRIPT_CM_IS_LOCKED, 0));
                else
                    exit_with_code(0);
            }
            else if (strcmp(argv[1], "obsolete_pset") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_OBSOLETE_PSET, backup));
            else if (strcmp(argv[1], "obsolete_src") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_OBSOLETE_SRC, backup));
            else if (strcmp(argv[1], "put_end") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_PUT_END, backup));
            else if (strcmp(argv[1], "put_pset") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_PUT_PSET, backup));
            else if (strcmp(argv[1], "put_src") == 0)
            {
                /* first time; create a file in source control system */
                if (status == 99)
                    cfg_command(SCRIPT_CM_CREATE, backup);

                if (status != 1)
                    send_back_files(cm_sysfile,cm_revision_file);

                exit_with_code(cfg_command(SCRIPT_CM_PUT_SRC, backup));
            }
            else if (strcmp(argv[1], "fast_put_src") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_FAST_PUT_SRC, backup));
            else if (strcmp(argv[1], "fast_merge_src") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_FAST_MERGE_SRC, backup));
            else if (strcmp(argv[1], "put_start") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_PUT_START, backup));
            else if (strcmp(argv[1], "setup_src") == 0) /* for -batch only copy pset */
            {
                if (status != 1)
                    send_back_files(cm_sysfile,cm_revision_file);

                exit_with_code(0);
            }
            else if (strcmp(argv[1], "src_version") == 0)
            {
                /*
                  cm_stat will return a text string with the following format
                  "%%%%stat;v=version;"
                */
                exit_with_code(cfg_command(SCRIPT_CM_SRC_VERSION, backup));
            }
            else if (strcmp(argv[1], "stat") == 0)
            {
                /*
                  cm_stat will return a text string with the following format
                  "%%%%stat;v=version;"
                */
                if (status == 0)
                    exit_with_code(cfg_command(SCRIPT_CM_VERSION, backup));
                else
                    exit_with_code(0);
            }
            else if (strcmp(argv[1], "unget_pset") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_UNGET_PSET, backup));
            else if (strcmp(argv[1], "unget_src") == 0)
                exit_with_code(cfg_command(SCRIPT_CM_UNGET_SRC, backup));
            else if (strcmp(argv[1], "unlock") == 0)
            {
                if (status == 0)
                    exit_with_code(cfg_command(SCRIPT_CM_UNLOCK, backup));
                else
                    exit_with_code(0);
            }
        }
        else
            error ("\%\%Internal error: Illegal configurator command - %s", argv[1]);
    }
    else
        error ("\%\%Internal error: Illegal configurator command - %s", argv[1]);

    free(bin_path);
    free(cm_bugno);
    free(cm_comment);
    free(cm_c_file);
    free(cm_date);
    free(cm_description);
    free(cm_files);
    free(cm_file_type);
    free(cm_local_proj);
    free(cm_options);
    free(cm_pset_search);
    free(cm_revision_file);
    free(cm_sysfile);
    free(cm_sys_proj);
    free(cm_version);
    free(cm_workfile);
    free(cm_rmpset);
    for(i=0;i<setenv_idx;i++)
        free(setenv_buf[i]);

    return 1;
}

/*
$Log: configurator.c  $
Revision 1.13 1998/09/22 18:44:53EDT lvku 
Added codes for debugging configurator.c
 * Revision 1.61  1994/07/29  17:09:45  so
 * the buffer for setenv commands is too small
 *
 * Revision 1.60  1994/07/29  16:04:18  so
 * cm_create_file, cm_put_start, cm_put_end, cm_unget_src, cm_unget_pset, cm_obsolete_src, cm_obsolete_pset
 *
*/

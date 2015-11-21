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
//------------------------------------------
// synopsis:
//    define the member functions for the class projModule
//
//------------------------------------------

// INCLUDE FILES

#include <sstream>
using namespace std;

#include <cLibraryFunctions.h>
#include <msg.h>
#include <prompt.h>
#include <machdep.h>

#include <saveXref.h>
#include <top_widgets.h>     // for push_busy_cursor, pop_cursor
#include <app.h>
#include <db_intern.h>
#include <cmd.h>
#include <cmd_enums.h>
#include <viewUIexterns.h>

#include "abstract_ui.h"
#include "gtDisplay.h"

#ifndef _question_h
#include <Question.h>
#endif
#include <ddict.h>

#include <XrefTable.h>
#include <transaction.h>
#include <fileEntry.h>
#include <feedback.h>
#include <proj.h>
#include <timer.h>
#include <driver_mode.h>
#include <disbuild_analysis.h>

#undef MAX

void update_path_hash_tables(char const *, projMap *, projNode *);
void backup_the_files(char const *, char const *, char);
int outdated_pset(char const *, char const *, char const *);
int char_based_merge (app *app_h, char const *srcf, char const *psetf, int unload_flag, int& do_not_save_flag);
bool is_forgiving();

#include <errorBrowser.h>
#include <proj_save.h>

#define NULL_NAME "NULL"

#include <vpopen.h>
#include <customize.h>
#include <messages.h>

#include <objOper.h>
#include <objRawApp.h>
#include <proj.h>
#include <tpopen.h>
#include <ddKind.h>
#include <path.h>
#include <pdf_tree.h>

#ifdef NEW_UI
#include <ui_cm_oper.h>
#else
#include <targetProject.h>
#endif

#include <xref.h>
#include <db.h>
#include <save.h>

#include <projBackEnd.h>
#include <fileCache.h>

#include <db_intern.h>
#include <RTL_externs.h>
#include <merge_dialog.h>

#include <NewPrompt.h>
#include <genTmpfile.h>

#include <scopeMgr.h>
#include <shell_calls.h>
#include <disbuild_analysis.h>

#define MRG_NOREVISION "no revision"

//extern void fine_grained_dependency_analysis(projNode*);
extern bool browser_delete_module(gtBase* parent, projModule *module, bool yes_to_all, char const *msg);
//void get_pset_fn (char const *, genString&);

static int check_out_pset_special(projModule*,char const *,char const *,char const *,char const *,char const *,char const *,int);

//void proj_path_report_app(app *, projNode *prj = NULL);
//bool els_parse_edg(projModule *);
//int els_parse_file(const Relational*);

//void proj_create_ctl_pm();
int get_file_names (char const *ln, projNode* src_proj,  projNode* dst_proj,
    genString& src_file, genString& dst_file,
    genString& src_paraset_file, genString& dst_paraset_file);

//bool do_delete_pset();
//void ste_smart_view_update (smtHeaderPtr, int with_text = 1);
//int ste_temp_view_suspend (smtHeaderPtr);

//extern "C" void cmd_journal_dump_pset(app *);
//extern "C" void ifd_dump_pset(app *);

void obj_delete_or_unload(Obj *root, int delete_flag);
//static int delete_module_internal(projModule* This, int new_flag, int type);
//projModule * symbol_get_module( symbolPtr& sym);

//boris: remove references to ddElement
//extern int module_get_dds (projModule *mod, objSet& dd_set);
extern int dd_set_aname (app *app_head, genString& aname);
extern int dd_get_aname (app *ah, genString& aname);
//extern int dd_contain_ddKind (app *app_head, ddKind ki);
//extern int dd_dependency_analysis (app *head, char& save_flag);
//extern int dd_dependency_analysis_for_check_in (app *head, char& save_flag);
#ifdef XXX_add_to_parser_failed
static objSet parsing_failed_set;

static void add_to_parser_failed (projModule* mod)
{
    parsing_failed_set.insert (mod);
}
#endif
static void append_string (ostream &stream, char const *arg)
{
    if (!arg) return;

    stream << "\"";

    // look for quote character to escape
    char const *quoted_str = arg;
    char const *next_quoted_str;
    while (next_quoted_str = strchr (quoted_str, '\"')) {
	while (quoted_str != next_quoted_str)
	    stream << *quoted_str++;
	stream << "\\\"";
	quoted_str++;
    }
    stream << quoted_str << "\" ";
}

// this function translates a string containing
// newline characters `\n` to one containing
// "\n" (two characters) in place of each '\n'
//
// NOTE WELL: The CALLER is responsible for
// deleting the returned string when done with it.

static char *newlines_to_backslash_n(char const *in)
{
    int nlcnt = 0;    // count of literal newlines in input
    int i     = 0;    // position in string

    char *out = NULL; // output buffer

    if (in) {
        // count the newlines in the file
        while (in[i]) {
            if (in[i] == '\n') nlcnt++;
            i++;
        }

        // allocate a buffer to contain the modified string
        out = new char[(i+1) + nlcnt];

        if (nlcnt) {
            int j = 0;
            int k = 0;

            while (in[j]) {
                if (in[j] == '\n') {
                    out[k++] = '\\';
                    out[k++] = 'n';
                } else {
                    out[k++] = in[j];
                }

                j++;
            }    
	    out[k] = 0;
        } else {
            strcpy(out, in);
        }
    }
    return out;
}

static void log_err(char const *err)
{
    Initialize(log_err);

    if (!err || strlen(err) == 0)
        return;

    msg("Configurator error: $1") << err << eom;
}

char *setenv_ifchanged(char const *name, char const *val, char *old_val)
{
    if (!val) return 0;
    int len = strlen(name) + strlen(val) + 2;
    char *new_val = (char *)malloc(len);
    OSapi_sprintf(new_val,"%s=%s",name,val);
    if (!old_val || (strcmp(new_val, old_val) != 0))
    {
	int nonzeroerror = OSapi_putenv(new_val);
	if (nonzeroerror) {
	    free(new_val);
	    new_val = NULL;
	}
    } else {
	free(new_val);
	new_val = old_val;
    }
    return new_val;
}
 
// Used by setup_configurator_envs() to save pointers to
// memory allocated from call to call. -jef
#define MAX_SETENV 20
static char *setenv_buf[MAX_SETENV];
static int setenv_idx = 0;

int setup_configurator_envs()
//
// Setup environment variables required for calling the configurator
// and free up memory used by any previous call to this routine. -jef
//
// Env vars: PSETCFG, cm_uselinks, cm_pendfilt, cm_optscripts
//
// Returns 1 if it was necessary to change one of the env vars, 0 otherwise
// Returns -1 if it is no configurator system
//
{
    char *oldenv_buf[MAX_SETENV];
    int oldenv_idx;
    int i;

    // Fetch some values that we will be needing
    genString custom_dir(customize::configurator_custom_dir());
    genString psetcfg_dir(customize::install_root());
    genString system_name(customize::configurator_system());

    if (customize::no_cm())
       return -1;

    // Do not free the old ones yet, wait until we have set the new values
    // Make sure any extras are set to NULL so we do not compare any bogus pointers
    for (i=MAX_SETENV; i>0; i--)
	if (i>setenv_idx)
	    oldenv_buf[i - 1] = NULL;
        else
	    oldenv_buf[i - 1] = setenv_buf[i - 1];
    oldenv_idx = setenv_idx;

    // Figure out the value for PSETCFG
    if ((system_name == (char const *)"custom") && custom_dir.length())
	psetcfg_dir = custom_dir;
    else
    {
	if (psetcfg_dir[psetcfg_dir.length()-1] != '/')
	    psetcfg_dir += "/config/";
	else
	    psetcfg_dir += "config/";
	psetcfg_dir += system_name;
    }

    // We are now ready to set the env vars
    setenv_idx = 0;
    setenv_buf[setenv_idx] = setenv_ifchanged("PSETCFG",
				    psetcfg_dir,
				    oldenv_buf[setenv_idx]);
    setenv_idx++;
    setenv_buf[setenv_idx] = setenv_ifchanged("cm_uselinks",
				    customize::configurator_use_softlinks() ? "Y" : "N",
				    oldenv_buf[setenv_idx]);
    setenv_idx++;
    setenv_buf[setenv_idx] = setenv_ifchanged("cm_optscripts",
                                    customize::configurator_optscripts_dir(),
				    oldenv_buf[setenv_idx]);
    setenv_idx++;

    int value_changed = 0;
    if (setenv_idx > oldenv_idx)
	value_changed = 1;
    // Ok, now we can free the previous values
    for (i=oldenv_idx; i>0; i--)
	if (oldenv_buf[i - 1] != setenv_buf[i - 1])
	{
	    value_changed = 1;
	    if (oldenv_buf[i - 1])
		free(oldenv_buf[i - 1]);
	}
    
    return value_changed;
}

static genString error_string;
static genString stat_string;
static genString cfg_message_string;
static genString *p_message_string = NULL;

static    FILE *writepipe=0;
static    FILE *readpipe =0;
static int config_depth = 0;
// prototype for shell_configurator ( ), used only from right here
int shell_configurator_reset();

int shell_configurator ( char const *cmd, char const *tag, char const *arg1 = 0, char const *arg2 = 0,
                         char const *arg3 = 0, char const *arg4 = 0, char const *arg5 = 0);
int shell_configurator_send (char const *cmd, char const *tag, char const *arg1 = 0, char const *arg2 = 0,
                         char const *arg3 = 0, char const *arg4 = 0, char const *arg5 = 0);
int shell_configurator_retrieve();
int configurator_internal ( char const *cmd, char const *tag, char const *arg1, char const *arg2, char const *arg3,
                   char const *arg4, char const *xterm_file, char const *file_type, int flag);
int shell_configurator_send_internal( char const *cmd,  char const *tag,  char const *arg1, char const *arg2,
                         char const *arg3, char const *arg4, char const *arg5);
int shell_configurator_retrieve_internal ();
extern int customize_no_cm();

int configurator_internal ( char const *cmd, char const *tag, char const *arg1, char const *arg2, char const *arg3,
                   char const *arg4, char const *xterm_file, char const *file_type, int flag)
// The interface to child process configshell.
//
// cmd: is the config-command
// tag: defined options
// arg1-4: data needed for config-command
// xterm_file: an input file to replace keystrokes for script recording or script execution
// file_type: "SRC" for source files: *.[cChH]
//          : "NOT_SRC" for not source file
{
    int env_var_changed = setup_configurator_envs();
    // If we had to change an env var, then we need to kill the configshell
    // so that the new values will be passed to the new configshell process
    // that will be created below
    if (env_var_changed)
			shell_configurator ( "exit", "");

    if (env_var_changed == -1)
        return 1;

    genString new_tag = tag; 
    if (is_model_build())
#ifndef _WIN32
	xterm_file = "/dev/null";
#else
	xterm_file = "nul";
#endif

    if (xterm_file && strlen(xterm_file))
    {
        if (new_tag.length())
            new_tag += " ";
        new_tag += "-ParaXterm ";
        new_tag += xterm_file;
    }
    tag = new_tag;

    // check if the user has disabled the use of the configshell window
    if (!customize::configurator_terse()) 
      {
	// user has not disabled the shell, so use the shell configurator
	if (flag == 1)
	  return shell_configurator_send ( cmd, tag, arg1, arg2, arg3, arg4, file_type);
	else if (flag == 2)
	  return shell_configurator_retrieve ();
	return shell_configurator ( cmd, tag, arg1, arg2, arg3, arg4, file_type);
      }
    else
      {
	// if the shell's already started, and we're not using it anymore,
	//   then close it
	if (!writepipe) 
	  shell_configurator ( "exit", "");
      }

    // Ignore the 'exit' command from aset_driver_cleanup()
    if (!strcmp(cmd, "icon"))  
			return 1; 
    if (!strcmp(cmd,"exit"))  
			return 0;

    ostringstream stream;
    char const *exe = customize::configurator ( );

    if (!exe) 
		{
			msg("ERROR: No configurator specified") << eom;
			return 0;
    }
    stream << exe << " " << cmd << " ";
    append_string (stream, tag  ? tag  : "");
    append_string (stream, arg1 ? arg1 : "");
    append_string (stream, arg2 ? arg2 : "");
    append_string (stream, arg3 ? arg3 : "");
    append_string (stream, arg4 ? arg4 : "");
    stream << ends;
    char const *cfg_cmd = stream.str().c_str();

    int error_found = 0;
    error_string = NULL;
    stat_string = NULL;

    char str[2048];
   
    FILE* pipe = vpopen_sync (cfg_cmd, "R");
    if (pipe) 
    {
	while (OSapi_fgets (str, sizeof(str)-1, pipe))
				{
	const int len = strlen (str);
	if (len == 0)
	    continue;
	if (strncmp(str,"%%@",2) == 0)
	    msg("Configurator diagnostic: $1", error_sev) << str << eom;
        }
        OSapi_fclose(pipe);
    }

    if (error_found) 
    {
	if (   strcmp (cmd, "stat") == 0
	    || strcmp(cmd, "src_version") == 0
	    || strcmp(cmd, "put_src") == 0
            || strcmp(cmd, "diff") == 0)
        {
            if (   error_string.length()
		&& strncmp (error_string, "stat;", 5) == 0) 
	    {
		stat_string = (char const *)error_string;
		error_found--;
	    }
	}
	else
	    log_err(error_string);
    }

    return !error_found;
}

int configurator ( char const *cmd, char const *tag, char const *arg1, 
	char const *arg2, char const *arg3, char const *arg4, 
	char const *xterm_file, char const *file_type)
// The interface to child process configshell.
//
// cmd: is the config-command
// tag: defined options
// arg1-4: data needed for config-command
// xterm_file: an input file to replace keystrokes for script recording or script execution
// file_type: "SRC" for source files: *.[cChH]
//          : "NOT_SRC" for not source file
{
    return configurator_internal(cmd,tag,arg1,arg2,arg3,arg4,xterm_file,file_type,0);
}
   
int configurator_to_icon ()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////
// The interface to child process configshell
//
// cmd: is the config-command
// tag: defined options
// arg1-5: data needed for config-command

int shell_configurator ( char const *cmd,  char const *tag,  char const *arg1, char const *arg2,
			char const *arg3, char const *arg4, char const *arg5)
{
  int ret = 0;
  shell_configurator_reset();
  if (shell_configurator_send(cmd,tag,arg1,arg2,arg3,arg4,arg5))
    ret = shell_configurator_retrieve();
  return ret;
}

int shell_configurator_send( char const *cmd,  char const *tag,  char const *arg1, char const *arg2,
                         char const *arg3, char const *arg4, char const *arg5)
{
  if (!customize::configurator())
  {
    msg("ERROR: No configurator specified") << eom;
    return 0;
  }
  int return_code= shell_configurator_send_internal( cmd, tag, arg1, arg2, arg3, arg4, arg5);
  if ( return_code == 1 )
    config_depth++;

  return return_code;
}

int shell_configurator_retrieve ()
{
 int return_code=shell_configurator_retrieve_internal();
 config_depth--;
 return return_code;
}

int shell_configurator_reset()
{
  while (config_depth > 0)
    shell_configurator_retrieve();
  return config_depth;
}


#ifndef _WIN32
///////////////////////////////////////////////////////////////////////
// The interface to child process configshell on UNIX
//
int shell_configurator_send_internal( char const *cmd,  char const *tag,  char const *arg1, char const *arg2,
                         char const *arg3, char const *arg4, char const *arg5)
{
  // We get the 'exit' command from aset_driver_cleanup()
  if (!strcmp(cmd,"exit"))
    {
      // if we haven't established child yet then don't bother;  just return
      if (!writepipe)
	return 0;
      else 
	{
	  OSapi_fprintf(writepipe,"exit \n");
	  OSapi_fflush(writepipe);
	  OSapi_fclose(writepipe); 
	  OSapi_fclose(readpipe);
	  writepipe=readpipe=0;
	  return 0;
	}
    }
    
  if (!strcmp (cmd, "icon"))
    {
      if (readpipe && writepipe)
	{
	  OSapi_fprintf (writepipe, "icon \n");
	  OSapi_fflush(writepipe);
	}
      return 0;
    }
  
  char const *exe = OSPATH(customize::configurator() );

  ostringstream stream;
  stream << exe << " " << cmd << " ";
  append_string (stream, tag ? tag : "");
  append_string (stream, realOSPATH((char const *)arg1));
  append_string (stream, realOSPATH((char const *)arg2));
  append_string (stream, arg3 ? arg3 : "");
  append_string (stream, arg4 ? arg4 : "");
  append_string (stream, arg5 ? arg5 : "");
  stream << ends;
  
  // cfg_cmd is allocated in newlines_to_backslash_n,
  // and must be deleted.
  
  char const *cfg_cmd = newlines_to_backslash_n( cnv_argstr_2OS(stream.str().c_str()) );
  
  if (!readpipe || !writepipe)
    {
      char configshell[MAXPATHLEN];
      strcpy (configshell, OSPATH(customize::install_root()));
      
      if (configshell[0] && configshell[strlen(configshell)-1]=='/')
	configshell[strlen(configshell)-1]='\0';	
      strcat(configshell,"/bin/configshell");

      if (OSapi_access(configshell,X_OK)) 
	{
	  msg("ERROR: No executable found: $1") << configshell << eom;
	  if (cfg_cmd) delete[] cfg_cmd;
	  return 0;
	}
      
      if (!is_gui())
	strcat(configshell," -batch");
      vpopen2way (configshell, &readpipe, &writepipe);
      
    }
  
  if (!readpipe || !writepipe)
    {
      msg("Pipes for Configshell cannot be created") << eom;
      if (cfg_cmd) delete[] cfg_cmd;
      return 0;
    }
  OSapi_fprintf(writepipe,"exec %s\n",cfg_cmd);
  OSapi_fflush(writepipe);
  if (cfg_cmd) delete[] cfg_cmd;
  return 1;
}

int shell_configurator_retrieve_internal ()
{
  // put up the hourglass ...
 push_busy_cursor();

  int error_found = 0;
  error_string = NULL;
  stat_string = NULL;
 
  char result[256];
  result[0]='\0';
  int i=10;
  while (result[0]!='$' && i>0)
    {
      i--;
      if (OSapi_feof(readpipe)) 
	{
	  error_string += "Configurator was aborted";
	  msg("configurator was aborted") << eom;
	  break;
	}
      OSapi_fgets (result,sizeof(result),readpipe);
      if (!strlen(result) || result[strlen(result)-1]!='\n')
	      strcat(result,"\n");
      if (strncmp (result, "%%stat;", 5) == 0)
	      stat_string = result+2;
      else if (!strncmp(result,"%%",2))
	      error_string += (result+2);
    }
  
 int return_code=1;
 if (strncmp(result,"$ok",3)) 
   {
     log_err(error_string);
     return_code=0;
   }
  
 p_message_string = NULL;
 pop_cursor();
 return return_code;
}

#else _WIN32
//////////////////////////////////////////////////////////////////////
//	The interface to process ConfigShell on NT is defined
//	in the separate source file

void ProjCM_InitConfigErr()
{
	// -- suppress the message if the user did not specify CM
	// : OLE Initialize should probably not be called if no_cm() is true ?
	if (!customize::no_cm())
		dis_message (C_CONFIGURATOR, MSG_ERROR, "M_NOCONFIGURATOR");
}

char const *	ProjCM_GetInstallRoot()
{
	return OSPATH(customize::install_root());
}

char const *	ProjCM_GetCfgCmd( char const *cmd,  char const *tag,  char const *arg1, char const *arg2,
                        char const *arg3, char const *arg4, char const *arg5)
{
  error_string = NULL;
  stat_string = NULL;

	ostrstream stream;
  stream << "exec ConfigShell " << cmd << " ";
  append_string (stream, tag ? tag : "");
  append_string (stream, arg1 ? realOSPATH((char const *)arg1) : "");
  append_string (stream, arg2 ? realOSPATH((char const *)arg2) : "");
  append_string (stream, arg3 ? realOSPATH((char const *)arg3) : "");
  append_string (stream, arg4 ? arg4 : "");
  append_string (stream, arg5 ? arg5 : "");
  stream << ends;

  // cfg_cmd is allocated in newlines_to_backslash_n,
  // and must be deleted.

  char const *cfg_cmd = newlines_to_backslash_n( cnv_argstr_2OS(stream.str()) );
  stream.rdbuf()->freeze(0);  // Allow stream to handle deallocation.
	return cfg_cmd;
}

int ProjCM_StoreResult(char const *result)
{
  int ret = 0;
  if ( result && *result )
  {
    if (strncmp(result,"$ok",3) == 0) 
       ret = 1;
    else if (strncmp (result, "%%stat;", 5) == 0)
       stat_string = result+2;
    else if (strncmp(result,"%%",2) == 0 && *(result+2) )
    {
      if ( cfg_message_string.str() )
	cfg_message_string += "\n";
      cfg_message_string += result+2;
    }
  }
  return ret;
}

int CM_ShowResult(int ok)
{
  if ( cfg_message_string.str() )
  {
    if ( p_message_string )
      (*p_message_string) += cfg_message_string;
    else if ( ok )
      dis_message(NULL, MSG_INFORM, "%s", cfg_message_string.str() );
    else
      dis_message(NULL, MSG_ERROR, "%s", cfg_message_string.str() );
  }
  cfg_message_string = 0;
  
  return ok;
}

// End of The interface to process configshell
///////////////////////////////////////////////////////////////////////
#endif


// get the filename returned from the configurator.
//
// ptn = "V" for stat and src_version which return the version number like "1.2"
// ptn = "sf" for put_src which return the system file name. In our environment, it returns
//            the pending file name
// ptn = "vf" for put_src which return the system version file name. In RCS, it returns
//            "*,v" file name
// ptn = "d" for diff result
//
// fn contains the returned filename
static void get_stat_string(char const *ptn, genString &fn)
{
    Initialize(get_stat_string);

    fn = 0;
    if (stat_string.length())
    {
        char *p = strstr((char *)stat_string,ptn);
        if (p)
        {
            char *q = strchr(p,';');
            if (q)
                *q = '\0';
            fn = p+strlen(ptn)+1;
            if (q)
               *q = ';';
        }
   }
}

void projModule::get_file_names_internal(genString& src_file,
					 genString& dst_file,
                                         genString& src_paraset_file,
					 genString& dst_paraset_file)
// to obtain src file name, dst file name, src pset name, dst pset name
{
    Initialize(projModule::get_file_names_internal);

    get_file_names(get_name(), get_project(), get_target_project(0),
                   src_file, dst_file, src_paraset_file, dst_paraset_file);
}

void sdo_ci_trigger(symbolArr &);
void sdo_co_trigger(symbolArr &);
void sdo_unco_trigger(symbolArr &);
void sdo_delete_trigger(symbolArr &);

// FUNCTION DEFINITIONS
//
// Update rtl for all local projects which have this source file checked
// into them.
//
void projModule::update_local_rtl_pmod()
{
    Initialize(projModule::update_local_rtl_pmod);

    genString   ln = NULL;
    genString   fn = NULL;

    projNode* proj = get_project();
    ln = get_name();
    proj->ln_to_fn(ln, fn);
//
// Go through all writable projects which contain this physical file name
// and update them.
//
    for (int i = 1; (proj = projList::search_list->get_proj(i)) && fn.length(); i++)
    {
// 
// Find out whether proj has this file name. 
//	
	ln = NULL;
	if (proj->is_writable())
	    proj->fn_to_ln(fn, ln);
	if (ln.length())
	{
	    start_transaction()
	    {
		projModule* src_mod = proj->find_module(ln);
		if (src_mod)
		{
//
// Update its src_mod.
//
		    src_mod->unload_module();
		    src_mod->invalidate_cache();
		    xref_notifier_report(-1, src_mod);
		    // must remove from xrefs after deleting app and module
		    symbolArr sym_files;
		    symbolPtr file_sym = src_mod;
		    file_sym = file_sym.get_xrefSymbol();
		    sym_files.insert_last(file_sym);
		    sdo_ci_trigger(sym_files);
		    Xref *xref = proj->get_xref();
		    if (xref)
			xref->remove_module(ln);
		    obj_delete(src_mod);
		}
	    }
	    end_transaction();
	}
    }
    return;
}

char const **projModule::get_version_list(char const *date, int &length)
// get the version list after date if date is defined
// get the whole version list if date is not defined.
// length is the number of entries in the list
// return the buffer that conatins the list
{
    Initialize(projModule::get_version_list);

    projNode *src_proj = get_project();
    projNode *dst_proj = projNode::get_home_proj();

    if (src_proj == dst_proj)
        dst_proj = get_target_project(0);

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    if (!get_file_names ((char const *)ln_name, src_proj, dst_proj, src_file, dst_file,
        src_paraset_file, dst_paraset_file))
        return 0;

    int success;
    if (date)
        success = configurator ( "get_ver", "", dst_file, src_file, date, "", cmd_xterm_input_file, "SRC");
    else
        success = configurator ( "get_ver", "", dst_file, src_file, "", "", cmd_xterm_input_file, "SRC");

    length = 0;
    if (!success || !error_string.length())
    {
        if (!error_string.length())
        {
	    msg("ERROR: Could not get any verison.") << eom;
        }
        return 0;
    }

    char const **ret;

    char line[256];
    char *path = error_string;

    path[strlen(path)-1] = '\0'; // remove the linefeed

    FILE *in = OSapi_fopen(path, "r");
    while(OSapi_fgets(line,255,in))
        length++;
    OSapi_fclose(in);

    ret = (char const **)calloc(sizeof(char const *),length);
    in = OSapi_fopen(path, "r");
    int idx = 0;
    while(OSapi_fgets(line,255,in))
    {
        line[strlen(line)-1] = '\0';
        ret[idx++] = strdup(line);
    }
    OSapi_fclose(in);

    OSapi_unlink(path);

    return ret;
}

// force_display = -1: call from targetProject::build_interface()
// force display = 2 - call came from put interface
// force display = 3 - call came from merge (from branch) interface
projNode * projModule::get_target_project(int force_display)
{
    Initialize(projModule::get_target_project);

    if (force_display == 0 && target_proj || force_display == -1)
        return target_proj;

    objArr projs;
	
    get_destination_candidates (projs);

    if (projs.size())
    {
	if (force_display == 2){ 
	    // PUT
            target_proj = checked_cast(projNode, projs[0]);
	}else
	    if(force_display == 1 || (force_display == 0 && projs.size() > 1)){
		// GET
		int   index = 0;
	        target_proj = NULL;
		while(target_proj == NULL && index < projs.size()){
		    target_proj = checked_cast(projNode, projs[index++]);
		    if(target_proj->find_module((char const *)ln_name) == NULL)
			target_proj = NULL;
		}
	    }else
		if(force_display == 3 && projs.size() > 1){
#ifndef NEW_UI
		    new targetProject(this, projs, force_display);
#else
                    return NULL;
#endif
		}
		else // if (!target_proj)
		    target_proj = checked_cast(projNode, projs[0]);
        return target_proj;
    }

    return 0;
}

projModule* projModule::projModule_of_app (app *h)
{
    return h ? h->get_module() : NULL;
}
#ifdef XXX_reparse_smtHeader_if_needed
static smtHeader * reparse_smtHeader_if_needed(smtHeader * ah)
{
  Initialize(reparse_smtHeader_if_needed);
  if (ah && ah->arn) {
    int save_modified = ah->is_modified ();
    ah->set_modified ();
    checked_cast(smtHeader, ah)->make_ast();
    if (save_modified == 0)
      ah->clear_modified ();
  }
  return ah;
}
#endif
smtHeader* dis_reload_header(app*);
#ifdef XXX_update_reparse_module //XXX: all old parsing; // projModule::update_reparse_module
//
// Load the module into memory if not already
// restore if needed otherwise import it
//   then reparse if the file is not yet correctly parsed
appPtr projModule::update_reparse_module()
{
    Initialize(projModule::update_reparse_module);
    app *ah = get_app();
    if (ah) {
	if (is_smtHeader(ah))
	{
#ifdef _WIN32
		ah = dis_reload_header(ah);
		if (ah)
#endif
		  reparse_smtHeader_if_needed((smtHeader*)ah);
	}
	return ah;
    }
    else {
	return reparse_module();
    }
}
#endif
void check_auto_save_copy(char const *fn)
{
    Initialize(check_auto_save_copy);

    genString auto_save_copy;
    char const *p = strrchr(fn,'/');
    if (p)
    {
        char *tmp = strdup(fn);
        char *p1 = strrchr(tmp,'/');
        *p1 = '\0';
        auto_save_copy.printf("%s/#%s#",tmp,p1+1);
        *p1 = '/';
        free(tmp);
    }
    else
        auto_save_copy.printf("#%s#",fn);
    
    struct OStype_stat auto_buf, stat_buf;
    if (OSapi_stat((char const *)auto_save_copy.str(), &auto_buf) != 0)
         return;
    if (OSapi_stat(fn, &stat_buf) != 0)
         return;

    if (auto_buf.st_mtime > stat_buf.st_mtime)
    {
        if (gtDisplay::is_open())
        {
	    int rep_flag = dis_question3 (CQ_AUTOSAVEEXISTS, T_LOADAUTOSAVE,
		B_LOADAUTOSAVE, NULL, B_LOADSOURCE,
		Q_LOADAUTOSAVE, fn);
            genString save_name;
            save_name.printf("%s~",auto_save_copy.str());
            if (rep_flag == 1)
            {
                OSapi_unlink(save_name.str());
                OSapi_rename(fn,save_name.str());
                OSapi_rename(auto_save_copy.str(),fn);
            }
            else
                OSapi_rename(auto_save_copy.str(),save_name.str());
        }
    }
}
#ifdef XXX_reparse_and_save_old_link
static void reparse_and_save_old_link(smtHeader *smtH, projModule *cur, int nosav = 0)
{
    Initialize(reparse_and_save_old_link);

    smtH->make_ast();
    if (!smtH->arn)
    {
        smtH->set_modified();
	if (!nosav)
	  ::db_save (smtH, NULL);
    }
    else 
        add_to_parser_failed (cur);
#endif //}
#ifdef XXX_projModule__reparse_module
appPtr projModule::reparse_module()
// reparse this module
{
  Initialize(projModule::reparse_module);
  appPtr ah = get_app();
  if (ah == 0) {
    char const *fn = (char const *)get_phys_filename();
    char const *pset_file = (char const *)paraset_file_name();
    bool has_pset = has_paraset_file(pset_file);
    if (!has_pset)
      return load_module();
    int db_format = db_get_db_format(fn);
    if (!db_format)
      return load_module();

    int wrk = 0;
    if (projModule::merge_module(this, fn, pset_file, 0, 1, 0, wrk) != 1) {
      ah =  load_module();
      if (ah && is_smtHeader(ah))
	reparse_smtHeader_if_needed((smtHeader *) ah);    
      return ah;
    }
    ah = restore_module();
    if (ah == 0) 
      return load_module();
  }
  reparse_and_save_old_link(checked_cast(smtHeader, ah), this, 1);
  return ah;
}
#endif
// projModule::diff_module
//
// Do a diff of this module with the given project

int projModule::diff_module(projNode* src_proj)
{
    return diff_module_c(src_proj, 1);
}

int projModule::diff_module_c(projNode* src_proj, int display_flag)
// get the difference between the file and the latest version in the source code control system
// return 1: if there exists difference
// otherwise return 0;
//
// src_proj: the local project
// display_flag = 1: display the difference
//              = 0: don"t display the difference
{
    Initialize(projModule::diff_module_c);

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    projNode *dst_proj = get_target_project(0);
    if (!dst_proj || dst_proj == projNode::get_home_proj())
        dst_proj = src_proj;
    
    if (!get_file_names (ln_name, src_proj, dst_proj, src_file, dst_file,
	src_paraset_file, dst_paraset_file))
        return 0;

    char const *flag;
    if (display_flag)
        flag = "1";
    else
        flag = "0";
    int success = configurator ( "diff", customize::configurator_diff(), dst_file, src_file, 
                                 flag, "", cmd_xterm_input_file, "SRC");

    genString res;
    get_stat_string("d", res);
    if (res.length())
        return 1; // different

    return 0; // same
}


int projModule::lock_module()
// Lock the module with the configuration manager
{
    projNode* dest_proj = get_project();
    projNode* non_home_proj = projHeader::non_current_project(ln_name);

    if (!non_home_proj) non_home_proj = dest_proj;

    if (!dest_proj || !non_home_proj) return 0;

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    if (!get_file_names (ln_name, non_home_proj, dest_proj, src_file, dst_file,
	src_paraset_file, dst_paraset_file))
        return 0;

    int success = configurator ( "lock", customize::configurator_lock(), src_file, dst_file,
                                 "", "", cmd_xterm_input_file, "SRC");

    if (!success) {
	msg("Configurator: lock failed moving $1 from $2 to $3") << (char const *)src_file << eoarg <<  non_home_proj << eoarg <<  dest_proj << eom;
    }

    return success;
}


// projModule::unlock_module
//
// Unlock the module with the configuration manager

int projModule::unlock_module()
{
    Initialize(projModule::unlock_module);

    genString msg = "";
    if (projModule::obtain_unlock_message(msg, ln_name) == 1)
        return unlock_module_c(msg.str());
    return 1;
}

int projModule::unlock_module_c(char const *str)
// unlock a module
//
// str != NULL: entered comment
// str = 0: create a comment like "Unlocked by <username>."
{
    Initialize(projModule::unlock_module_c);

    projNode* dest_proj = get_project();
    projNode* non_home_proj = projHeader::non_current_project(ln_name);

    if (!non_home_proj) non_home_proj = dest_proj;

    if (!dest_proj || !non_home_proj) return 0;

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    if (!get_file_names (ln_name, non_home_proj, dest_proj, src_file, dst_file,
	src_paraset_file, dst_paraset_file))
        return 0;

    genString cmd = customize::configurator_unlock();
    if (cmd.length())
        cmd += " ";
    cmd += "-ParaUnlock ";

    genTmpfile tnm("ulock");

    cmd += tnm.name();

    FILE *cmt = tnm.open_buffered();

    if (!str || !strlen(str))
        OSapi_fprintf(cmt,"Unlocked by %s.",customize::user());
    else
        OSapi_fprintf(cmt,"%s",str);

    tnm.close();

    int success = configurator ( "unlock", cmd, src_file, dst_file,
                                 "", "", cmd_xterm_input_file, "SRC");

    tnm.unlink();

    if (!success) {
	msg("Configurator: failed unlock, moving $1 from $2 to $3") << (char const *)src_file << eoarg << dest_proj << eoarg << non_home_proj << eom;
    }
	
    return success;
}

int projModule::create_new_file(char const *file_name)
// create a new file with the name file_name.
// This is a static member function.
{
    Initialize(projModule::create_new_file);
    
    int success = 0;/*FAILURE*/

    if (customize::no_cm())
    {
        char const *new_file = OSPATH(file_name);
        FILE *pfNew;
        if( pfNew = OSapi_fopen( new_file, "w" ) )
        {
            OSapi_fclose( pfNew );
            success = 1;
        }
    }
    else
        success = configurator ( "create_file", customize::configurator_create(), file_name, "", "", "",
                                 cmd_xterm_input_file, "SRC");

    return success;
}

int projModule::unget_module()
// unget a module; it is delete_module for RCS
{
    Initialize(projModule::unget_module);

    return delete_module (0, 1);
}

// static
int projModule::obtain_unlock_message(genString &msg, char const *nm, char const *button_txt)
{
    Initialize(projModule::obtain_unlock_message);

    genString buf = "Enter message for unlock module";
    if (nm && nm[0])
    {
        buf += " ";
        buf += nm;
    }
    else
        buf += "s";

    msg.printf("Unlocked by %s.",customize::user());
    return dis_prompt (PT_MESSAGEFORUNLOCK, buf.str(), B_OK, button_txt, B_CANCEL, PC_UNLOCKERRORMESSAGE, msg);

}


static int do_xref_special (char const *ln, projNode* src_proj, projNode* dst_proj)
{
    // copying is necessary only if projects are different

    if (src_proj == dst_proj)
        return 0;

    // get src and destination xref

    Xref *src_xref = src_proj->get_xref();
    Xref *dst_xref = dst_proj->get_xref();
 
    // move xref entry from global to local

    if (src_xref && src_xref->has_lxref()
     && dst_xref && dst_xref->has_lxref()) {
      
      // remove any old remants from local xref before copying over
      dst_xref->remove_module(ln);

      // copy from global into local xref
      src_xref->copy_module (ln, dst_xref);

      // save to disk copy of local xref
      dst_xref->save_lxref_by_module(ln);

      return 0;
    } 

    return -1;
}


projModule *projModule::remove_link(projNode *pn, char const *ln, int &flag)
{
    Initialize(projModule::remove_link);

    invalidate_cache();

    if (pn->is_script())
        return create_script_module(pn, ln);

    genString fn;
    pn->ln_to_fn_no_link_check(get_name(),fn);
    if (fn.str() == 0) return 0;
    struct OStype_stat stat_buf;
    int status = global_cache.lstat(fn, &stat_buf);
    if (!status && OSapi_S_ISLNK(&stat_buf))
    {
        flag = dis_question3 (CQ_LINKEXISTS, T_REMOVELINK,
			      B_OK, NULL, B_CANCEL,
			      Q_REMOVELINK, fn.str());
        global_cache.invalidate(fn.str());
        if (flag == 1)
        {
            OSapi_unlink(fn.str());
            projModule *dst_mod = pn->make_module (ln);
            return dst_mod;
        }
    }
    return 0;
}

static projModule *create_script_module_special(char const *nm, projNode *pn, char const *ln)
{
    Initialize(create_script_module_special);

    projNode *cur = pn;

    while(cur && !cur->get_map())
        cur = cur->find_parent();

    if (!cur)
        return 0;

    projMap *pm = cur->get_map();
    update_path_hash_tables(nm, pm, pn);
    genString fn;
    pn->ln_to_fn(ln, fn);
    projModule *dst_mod = proj_make_module(fn, pn, ln);
    pn->insert_module_hash(dst_mod);
    return dst_mod;
}

projModule *remove_link_special(char const *nm,projNode *pn, char const *ln, int &flag)
{
    Initialize(remove_link_special);

    if (pn->is_script())
        return create_script_module_special(nm, pn, ln);

    genString fn;
    pn->ln_to_fn_no_link_check(nm,fn);
    if (fn.str() == 0) return 0;
    struct OStype_stat stat_buf;
    int status = global_cache.lstat(fn, &stat_buf);
    if (!status && OSapi_S_ISLNK(&stat_buf))
    {
        flag = dis_question3 (CQ_LINKEXISTS, T_REMOVELINK,
			      B_OK, NULL, B_CANCEL,
			      Q_REMOVELINK, fn.str());
        global_cache.invalidate(fn.str());
        if (flag == 1)
        {
            OSapi_unlink(fn.str());
            projModule *dst_mod = pn->make_module (ln);
            return dst_mod;
        }
    }
    return 0;
}

static projModule* input_module_internal(projModule *This, char const *fn, char const *ln, projNode* pmod_pr, symbolPtr &file_sym, int type, char const *rev, char const *tar_dir, void * data);

static projModule* get_or_copy_module(symbolPtr& sym, int op, void * data)
// Gets the file into the current writable project
{
    Initialize(get_or_copy_module);
    
    projModule *pm = NULL; 
    push_busy_cursor();
    int unload = 0;

    if (sym.relationalp() && is_projModule(sym)) { 
	projModule * module = checked_cast(projModule,sym);
	if (module && !module->get_app())
	    unload = 1;
        pm = module->input_module(op, 0, 0, data);
    } else {
	symbolPtr xr = sym.get_xrefSymbol();
	if (xr->xrisnotnull()) {
	    if (xr.get_kind() != DD_MODULE)
		xr = xr->get_def_file();
	    if (xr->xrisnotnull() && xr.get_kind() == DD_MODULE) {
		genString ln, fn;
		ln = xr.get_name();
		projNode * pmod_pr = projHeader::get_pmod_project_from_ln(ln);
		if (pmod_pr) {
		    pmod_pr->ln_to_fn(ln,fn);
		    if (fn.length()) {
			if (!app::find_from_phys_name(fn))
			    unload = 1;
			pm = input_module_internal(NULL, fn, ln, pmod_pr, xr, op, 0, 0, data);
		    }
		}
	    }
	}
    } 

    //boris: Do not check the app existence. Let ::unload_module() to 
    //       take care about Frame docs, that do not have apps.
    // if (pm && unload && pm->get_app())
    if (pm && unload)
	pm->unload_module();
    pop_cursor();
    return pm;
}

static projModule* get_or_copy_module(symbolPtr& sym, int op)
{
  Initialize(get_or_copy_module);
  return get_or_copy_module(sym, op, NULL);
}

projModule* projModule::get_module_with_data(symbolPtr & sym, void * data)
{
    Initialize(projModule::get_module);
    
    return get_or_copy_module(sym, 1, data);
}

projModule* projModule::get_module(symbolPtr & sym)
{
    Initialize(projModule::get_module);
    
    return get_or_copy_module(sym, 1);
}

projModule* projModule::copy_module(symbolPtr & sym)
// copy the file into the current writable project
{
    Initialize(projModule::copy_module);

    return get_or_copy_module(sym, 0);
}

static void get_src_rev(char const *fn, char const *sys_fn, genString &rev)
// get the latest version from the source code control system
//
// fn: the local file name
// sys_fn: the system file name
// rev: will return the version number
{
    Initialize(get_src_rev);

    rev = 0;
    if (configurator ( "stat", 0, sys_fn, fn, "", "", cmd_xterm_input_file, "SRC"))
        get_stat_string("v", rev);
}

static int unload_uncheckedout(appPtr ap, int type, char const *nm)
{
    Initialize(unload_uncheckedout);

    char mod_flag = 0;
    if (ap->is_modified() || ap->is_src_modified())
        mod_flag = 1;

    char const *op;
    if (type == 0)
        op = "copy";
    else if (type == 1)
        op = "get";
    else if (type == 2)
        op = "copy previous version";
    else if (type == 3)
        op = "put";
    else // if (type == 4)
        op = "copy pset";

    char const *src = "file";
    if (!ap->is_src_modified())
        src = "information model for";
    if (mod_flag)
    {
        genString msg;
        msg.printf(TXT("The %s %s has been changed.\nDo you want to proceed with \"%s\" ?"),
                   src, nm, op);
	
	int answer = dis_question(T_UNLOADINGFILE,
				    B_YES, B_NO, Q_FILECHANGEPROCEED,
				    src, nm, op);
        return(answer == 1);
    }
    else
        return 1;
}

static int unload_before_input(appPtr ap, int type, char const *nm, char const *pset)
{
    Initialize(unload_before_input);

    char const *buffer;
    char mod_flag = 0;
    if (ap->is_modified() || ap->is_src_modified())
        mod_flag = 1;

    if (mod_flag)
	buffer = Q_SAVETHENRENAME;
    else
	buffer = Q_UNLOADTHENREALOAD;

    int unload_flag = 1;
    if (gtDisplay::is_open())
    {
        char const *title;
        if (type == 0)
            title = T_COPYMODULE;
        else if (type == 1)
            title = T_GETMODULE;
        else
            title = T_GETOLDMODULE;
	
	if (mod_flag)
	    unload_flag = dis_question3 (CQ_ALREADYLOADED, title, 
					 B_SAVERENAMECO, B_COWITHOUTSAVE,B_CANCEL,
					 buffer, nm);
	else
	    	    unload_flag = dis_question3 (CQ_ALREADYLOADED, title, 
					 B_CHECKOUT, NULL, B_CANCEL,
					 buffer, nm);

        if (unload_flag == 1 || unload_flag == 2)
        {
            if (mod_flag && unload_flag == 1)
            {
                ::db_save(ap,0);
                backup_the_files(nm, pset, 1);
            }
            OSapi_unlink(nm);
        }
    }
    return unload_flag;
}

bool projModule::is_raw_modified(char tp)
{
    Initialize(projModule::is_raw_modified);

    projModule *pm;

    return is_raw_modified(tp, pm);
}

bool projModule::is_raw_modified(char tp, projModule* &del)
// If the raw view is modified, no operation will continue until it is saved.
// If it is not for "copy pset", all un-touched raw views will be unloaded.
//
// tp == 0 for copy module
// tp == 1 for get module
// tp == 2 for get previous module
// putUI uses tp == 3 for is_raw_modified(3)
// tp == 4 for -batch only copy the pset out
{
    Initialize(projModule::is_raw_modified);

    genString fn;
    get_phys_filename(fn);

    appPtr raw_app;
    int the_raw_flag = 0;
    if (raw_app = app::get_header(App_RAW, fn))
    {
        if (raw_app->is_modified())
        {
            char const *op;
            if (tp == 0)
                op = "copy";
            else if (tp == 1)
                op = "get";
            else if (tp == 2)
                op = "copy previous version";
            else if (tp == 3)
                op = "put";
            else // if (tp == 4)
                op = "copy pset";

	    int answer = dis_question(T_UNLOADINGFILE, 
					B_YES, B_NO, Q_CHANGEPROCEED,fn, op);
	    return(answer != 1);
        }
        the_raw_flag = 1;
    }

    if (the_raw_flag && tp != 4) // tp == 4 only does checkout pset
    {        
        del = this;

    }

    return 0;
}

bool projModule::is_raw_modified(char const *fn, char tp, app* &del)
// If the raw view is modified, no operation will continue until it is saved.
// If it is not for "copy pset", all un-touched raw views will be unloaded.
//
// tp == 0 for copy module
// tp == 1 for get module
// tp == 2 for get previous module
// putUI uses tp == 3 for is_raw_modified(3)
// tp == 4 for -batch only copy the pset out
{
    Initialize(projModule::is_raw_modified);

    appPtr raw_app;
    int the_raw_flag = 0;
    if (raw_app = app::get_header(App_RAW, (char const *)fn))
    {
        if (raw_app->is_modified())
        {
            char const *op;
            if (tp == 0)
                op = "copy";
            else if (tp == 1)
                op = "get";
            else if (tp == 2)
                op = "copy previous version";
            else if (tp == 3)
                op = "put";
            else // if (tp == 4)
                op = "copy pset";


	    int answer = dis_question(T_UNLOADINGFILE, B_YES, B_NO, 
					Q_CHANGEPROCEED,
					fn, op);
	    return(answer != 1);
        }
        the_raw_flag = 1;
    }

    if (the_raw_flag && tp != 4) // tp == 4 only does checkout pset
    {        
        del = raw_app;
    }

    return 0;
}

#define COPY_MODULE_TYPE      0
#define GET_MODULE_TYPE       1
#define COPY_PREV_MODULE_TYPE 2
#define COPY_PSET_TYPE        4

projModule* projModule::input_module(int tp, char const *rev, char const *tar_dir, void *data)
{
    symbolPtr sym;
    return input_module_internal(this,NULL,NULL,NULL,sym,tp,rev,tar_dir, data); 
}

projModule* projModule::input_module(int tp, char const *rev, char const *tar_dir)
{
    Initialize(projModule::input_module);
    return input_module(tp, rev, tar_dir, NULL);
}


static char const *clearcase_signature = "@@";

static void get_clearcase_version (genString& vn, genString& aname)
{
    Initialize(get_clearcase_version__genString2);

    aname = 0;

    get_stat_string("vf", vn);
    if (vn.length())
	aname = strstr((char const *)vn, (char const *)clearcase_signature);

    if (aname.length())
	aname = (char const *)aname+2;
}

static int dir_not_exist(char const *file)
{
    Initialize(dir_not_exist);

    genString dir = file;
    char *p = strrchr((char *)dir,'/');
    Assert(p);
    *p = '\0';

    struct OStype_stat buf;
    if (global_cache.stat(dir,&buf))
        return 1;

    return 0;
}

static projModule* input_module_internal(projModule *This, char const *fn, char const *ln, projNode* pmod_pr, symbolPtr &file_sym, int tp, char const *rev, char const *tar_dir, void * data)
// will work for get_module and copy_module and get previous version
// tp == 0 for copy module; rev = 0
// tp == 1 for get module;  rev = 0
// tp == 2 for get previous module; rev contains the version number
// putUI uses tp == 3 for is_raw_modified(3)
// tp == 4 for -batch only copy the pset out; rev = 0
// tar_dir: if not zero, it will be the target dir to put the checked out file
{
    Initialize(input_module_internal);

    if (This)
	ln = This->get_name();

    projNode   *src_proj;
    projModule *src_mod = This; 

    if (This)
	src_proj = This->get_project();
    else
        src_proj = pmod_pr;

    projNode   *dst_proj = projNode::get_home_proj();
    projModule *dst_mod  = dst_proj->find_module ((char const *)ln);

    if (src_proj == dst_proj)
	if (This) {
	    src_proj = This->get_target_project(0);
	    src_mod = src_proj->find_module((char const *)ln);
        } else
	    return 0;

    genString src_file;
    genString dst_file;
    genString src_paraset_file;
    genString dst_paraset_file;

    if (This)
	if (tp == COPY_PSET_TYPE)
	    dst_proj = src_proj;

    if (!get_file_names (ln, src_proj, dst_proj, src_file, dst_file,
                         src_paraset_file, dst_paraset_file))
        return 0;

    if (dir_not_exist(dst_file))
    {
        int success = configurator ( "create_dir", "", src_file, dst_file, "", "", cmd_xterm_input_file, "SRC");
        if (!success)
        {
	    msg("INFORM: Failed to create directory for $1") << (char const *)src_file << eom;
            return 0;
        }

        // refresh here
        dst_proj->update_projects(0);
    }

// Notify views of entities in source to update to destination

    if (This)
	file_sym = symbolPtr(This).get_xrefSymbol();
    if (file_sym.xrisnotnull()) {
	symbolArr defineds;
	file_sym->get_link(is_defining, defineds);
	symbolPtr defined;
	ForEachS(defined, defineds) {
	    ddKind k = defined.get_kind();
	    if (dd_notify_xref_if_modified(k)) {
		xref_notifier_report(0 /* modified */, defined);
	    }
	}
    }

    int unload_flag = 1; // default is "unload then copy" or "unload then get"
    if ((dst_mod && dst_mod->get_app()) ||
	((src_mod && src_mod->get_app()) ||
	 (!src_mod && app::find_from_phys_name(fn)))) {
        appPtr ap = 0;
        if (dst_mod)
            ap = dst_mod->get_app();
        if (ap){
	    if(strcmp(src_file, dst_file))
	    {
		unload_flag = unload_before_input(ap, tp, dst_file, dst_paraset_file);
		if (unload_flag != 1 && unload_flag != 2)
		    return 0;
	    }else{
		unload_flag = unload_uncheckedout(ap, tp, dst_file);
		if (unload_flag != 1)
		    return 0;
	    }
        }
        else{
	    appPtr ap;
	    if (src_mod) 
		ap = src_mod->get_app();
	    else
		ap = app::find_from_phys_name(fn);
	    if (ap){
		unload_flag = unload_uncheckedout(ap, tp, src_file);
		if (unload_flag != 1)
		    return 0;
	    }
	}
    }
    projModule *del0 = 0;
    projModule *del1 = 0;
    app        *del2 = 0;

    if (src_mod) {
	if (strcmp(src_file, dst_file)) {
	    if ((dst_mod && dst_mod->is_raw_modified(tp, del1)) ||
		src_mod->is_raw_modified(tp, del0)) {
	        return 0;
            }
        } else {
            if (src_mod->is_raw_modified(tp, del0))
	        return 0;
        }
    } else {
	if (strcmp(src_file, dst_file)) {
	    if ((dst_mod && dst_mod->is_raw_modified(tp, del1)) || 
		projModule::is_raw_modified(fn, tp, del2))
		return 0;
	} else {
	    if(projModule::is_raw_modified(fn, tp, del2))
		return 0;
	}
    }

    int dst_mod_creation_flag = 0;
    if (!dst_mod)
    {
        dst_mod = dst_proj->make_module ((char const *)ln);
        if (dst_mod)
            dst_mod_creation_flag = 1;
    }

    if (!dst_mod)
    {
        int flag = 0;
	if (This)
	    dst_mod = This->remove_link(dst_proj, (char const *)ln, flag);
	else
	    dst_mod = remove_link_special((char const *)ln,dst_proj, (char const *)ln, flag);
        if (flag == -1)
            return 0;
	if(dst_mod)
	    dst_mod_creation_flag = 1;
    }

    if (!dst_mod)
    {
        msg("ERROR: Cannot create file: $1 for project $2") << (char const *)ln << eoarg << (char const *)dst_proj->get_name() << eom;
        return 0;
    }

    if (tp == COPY_PREV_MODULE_TYPE) // for get previous module
    {
        genString cur_rev;
        get_src_rev(dst_file, src_file, cur_rev);

        if (strcmp(rev,cur_rev) == 0) // latest version
            tp = GET_MODULE_TYPE;
    }

    int success = 1;

    genString cmd = customize::configurator_get();
    genString vn = "";

    if (tp == COPY_PREV_MODULE_TYPE) // for get previous module
    {
        cmd += " -ParaRev ";
        cmd += rev;
    }

    genString dst_file_tmp;
    if (tar_dir)
    {
        char const *fp = strrchr(dst_file.str(),'/');
        dst_file_tmp.printf("%s%s",tar_dir,fp);
    }
    else
        dst_file_tmp = dst_file.str();

    char const *tnm_for_comm = data ? (char const *) data : "";

    if (tp == COPY_MODULE_TYPE || (tp == GET_MODULE_TYPE && !customize::configurator_use_locks()) || tp == COPY_PREV_MODULE_TYPE) 
        success = configurator ("copy_src", cmd, src_file, dst_file_tmp, tnm_for_comm, "", cmd_xterm_input_file, "SRC");
    else if (tp != COPY_PSET_TYPE)
        success = configurator ("get_src",  cmd, src_file, dst_file_tmp, tnm_for_comm, "", cmd_xterm_input_file, "SRC");
    else // if (tp == COPY_PSET_TYPE)
        success = configurator ("setup_src",  cmd, src_file, dst_file_tmp, "", "", cmd_xterm_input_file, "SRC");
  
//
// Now, if configuration manager is clearcase, extract version name from
// stat_string and put it in aname. The idea is the following: only for
// clearcase will string "@@" be found in vn.  Therefore for all others aname
// will be 0 as a result of the strstr statement. Later, when aname is needed,
// if it is non zero (i.e., for clearcase) we will skip the expensive
// get_src_rev call; for all other configuration managers, we will call it,
// but they are fast anyway.
//
    genString aname;
    if (success)
	get_clearcase_version (vn, aname);

    if (src_mod)
	src_mod->invalidate_cache();
    dst_mod->invalidate_cache();

    int pset_exist = 0;
    if (success)
    {
        if (src_proj != dst_proj && tp != COPY_PREV_MODULE_TYPE || tp == COPY_PSET_TYPE)
        {
            int search_pset = 0;
            if (!xref_not_header_file(src_file) && tp == COPY_PSET_TYPE)
                search_pset = 1;
            int success0;
	    if (This)
		success0 = This->check_out_pset(dst_mod, dst_file, vn, src_paraset_file, dst_paraset_file, cmd_xterm_input_file, tar_dir, search_pset);
	    else
		success0 = check_out_pset_special(dst_mod, dst_file, vn, src_paraset_file, dst_paraset_file, cmd_xterm_input_file, tar_dir, search_pset);

            if (success0)
            {
                genString ne;
                get_stat_string("not_exist", ne);
                if (!ne.length())
                    pset_exist = 1;
            }
        }

        if (unload_flag == 1 || unload_flag == 2)
        {
            start_transaction()
            {
                if (dst_mod->get_app())
                {
                    dst_mod->unload_module();
                    del1 = 0;
                }
		if (src_mod)
                {
		    if (src_mod->get_app())
                    {
			src_mod->unload_module();
                        del0 = 0;
                    }
		}
                else
                {
		    app * src_app = app::find_from_phys_name(fn);
		    if (src_app)
                    {
			obj_unload(src_app);
                        if (src_app == del2)
                            del2 = 0;
                    }
                }
            }

            if (del0)
                del0->unload_module();
            if (del1)
                del1->unload_module();
            if (del2)
                obj_unload(del2);

            end_transaction();
        }
    }

    if (success)
    {
        int late_save = 0;
        if (has_paraset_file(dst_paraset_file) && tp != COPY_PSET_TYPE)
        {
            if (tp == COPY_PREV_MODULE_TYPE) // for previous module
                OSapi_unlink(dst_paraset_file);
            else if (db_get_db_format(dst_file) && ::outdated_pset(dst_file,dst_paraset_file, dst_file))
            { // for get and copy
                late_save = 1;
                // If it needs to be saved after merged, late_save will be set to 2.
                projModule::merge_module(dst_mod, dst_file, dst_paraset_file, 0, 1, 0, late_save);
            }
        }
        // if destination module is a paraset file and the pset file is
        // not out of date, then "copy" or "get" the fast way by just restoring the
        // file in the home pmod
        // if tar_dir is not null, it is for merge to obtain files for both base and other branch
        if (tp != COPY_PREV_MODULE_TYPE && tp != COPY_PSET_TYPE && tar_dir == 0)
        {
            if (dst_mod->get_app() || (!(::outdated_pset(dst_file,dst_paraset_file,dst_file))
                && dst_mod->is_paraset_file()))
            {
                app* ah = dst_mod->restore_module();               
                if (ah)
                {
		  
		  symbolArr sym_files;
		  symbolPtr file_sym = ah;
		  file_sym = file_sym.get_xrefSymbol();
		  sym_files.insert_last(file_sym);
 		  sdo_co_trigger(sym_files);
		  
                    ah->get_root();
                    // check for dd == 0
		    if (get_relation(dd_of_appHeader, ah))
		      dst_mod->setup_SWT_Entity_Status(ah);
//
// If aname is not already provided, use (expensive) call to get_src_rev to
// get it. However we expect that clearcase will provide aname (see above),
// and for other configuration managers the call is not so expensive.
//
		    if (aname.length() == 0)
			get_src_rev(dst_file, src_file, aname);

                    if (aname.length())
                    {
                        genString rev0;
                        dst_mod->get_rev(rev0, 0);
                        if (rev0.length() == 0 || strcmp(aname,rev0))
                        {
			    // set up for Software Tracking facility
			    int res = dd_set_aname (ah, aname);
                            if (res == 0)
                            {
                                ah->set_modified();
                                late_save = 2;
                            }
                        }
                    }
                    Xref* xr =  dst_proj->get_xref();
                    if (xr)
                        xr->register_file_for_crash_recovery(ah->get_filename());
                }
            } else {
                // do it the normal way by copying the info from the src pmod
		// Handle xref stuff
		do_xref_special((char const *)ln, src_proj, dst_proj);
	   }
        }
        else if (tp != COPY_PSET_TYPE) // for get previous module (assoc file)
        {

        }

        if (late_save == 2)
        {
            app* ah = dst_mod->restore_module();
	    int res = ah ? db_save_or_cancel (ah, /*(char const *)fn*/ 0, 1) : 0;
//boris: direct call without gpi envolve            ::db_save(ah, NULL);
        }

        if (tp == COPY_MODULE_TYPE)
	    msg("STATUS: Copy completed for $1.") << (char const *)ln << eom;
	else if (tp == COPY_PSET_TYPE)
        {
            bool pset_writtable = true;
            if (OSapi_access (dst_paraset_file, W_OK) < 0)
            {
                struct OStype_stat buf0;
                OSapi_stat(dst_paraset_file, &buf0);
                if (OSapi_chmod (dst_paraset_file, 0666) == 0)
                    OSapi_chmod (dst_paraset_file, buf0.st_mode);
                else
                    pset_writtable = false;
            }
            if (pset_exist && pset_writtable)
		msg("STATUS: Copy .pset completed for $1.") << (char const *)ln << eom;
            else
		msg("STATUS: The .pset file for $1 cannot be checked out.") << (char const *)ln << eom;
        }
        else
	    msg("STATUS: Copy completed for $1.") << (char const *)ln << eom;

	CM_COMMAND cm_cmnd = (tp) ? CM_GET : CM_COPY;
	scope_mgr.report_cm_command (cm_cmnd, src_proj, dst_proj, (char const *)ln);
    }
    else if (tp != COPY_PREV_MODULE_TYPE)
    {
	// do some report
	char const *cmd = tp ? "get" : "copy";
	symbolPtr sym;
	if (src_mod)
	    sym = src_mod;
	else
	    sym = file_sym;

	if (dst_mod_creation_flag)
	{
	    // delete the temporarily create destination module and print result
	    start_transaction()
	    {
		obj_delete_or_unload (dst_mod,1);
	    }
	    end_transaction();
	}

	dst_mod = NULL;
	if (tp == COPY_MODULE_TYPE)
	    msg("STATUS: Copy not completed for $1.") << (char const *)ln << eom;
	else
	    msg("STATUS: Get not completed for $1.") << (char const *)ln << eom;
    }

    if (dst_mod != NULL){
      char const *dst_pset = dst_mod->paraset_file_name();
	db_set_pset_written (dst_pset, 0);
      }
    return dst_mod;
}

projModule *projModule::get_prev_module(char const *rev)
// copy the file of version "rev" into the current writable project
{
    Initialize(projModule::get_prev_module);

    return input_module(2, rev, 0);
}

void projModule::get_rev_from_source(genString &rev)
// get the version number from the source file
//
// rev will return the version number
// if not found, rev.length() == 0 and issue one warning
{
    Initialize(projModule::get_rev_from_soource);

    genString src_file;
    get_phys_filename(src_file);

    int conf = 0;
    if (!config_depth)
      conf = configurator ( "src_version", "", src_file, src_file, "", "", cmd_xterm_input_file, "SRC");
    else
      conf = configurator_internal ( "src_version", "", src_file, src_file, "", "", cmd_xterm_input_file, "SRC", 2);
    if (conf)
      get_stat_string("v", rev);

    if (rev.length() == 0 && !customize::no_cm() )
    {
	msg("The version number of $1 could not be found.", warning_sev) << src_file.str() << eom;
    }
}

// tp = 0: for get and copy
// tp = 1: for put
void projModule::get_rev(genString &rev, int tp)
{
    Initialize(projModule::get_rev);

    rev = 0;
    app *ah = get_app();
    int unload_flag = 0;

    if (!ah)
    {
	ah = restore_module();
	unload_flag = 1;
    }

    if (ah)
    {
	int res = dd_get_aname (ah, rev);
	if (unload_flag)
	    obj_unload(ah);
    }

    if (rev.length() == 0 && tp)
	get_rev_from_source(rev);
}


int projModule::check_out_pset(projModule *local_mod, char const *local_file, char const *vf,
                               char const *sys_paraset_file, char const *local_paraset_file,
                               char const *xterm_file, char const *tar_dir, int search_pset)
// copy pset from system area into the local area
//
// local_mod: the local module
// local_file: the local file name
// vf: the version file for the source or header file
// sys_paraset_file: the system pset name
// local_paraset_file: the local pset name
// xterm_file: the file contains the input keystrokes for the creation or the execution of scripts
// tar_dir: if not zero, it is the directory where the pset is copied to
// search_pset: It will be set for the header file when -batch is running. When set, it will search the previous
//              version of pset if the current header file does not have a corresponding pset. If it is not set,
//              no pset will be checked out if the current file does not have a corresponding pset.
{
    Initialize(projModule::check_out_pset);

    char const *file_type = "SRC";
    if (xref_not_header_file(local_file) && xref_not_src_file(local_file) )
        file_type = "NOT_SRC";

    genString tmp_pset;
    if (tar_dir)
    {
        char const *fp = strrchr(local_paraset_file, '/');
        tmp_pset.printf("%s%s",tar_dir,fp);
    }
    else
        tmp_pset = local_paraset_file;

    if (dir_not_exist(tmp_pset))
    {
        int success = configurator("create_dir", "", sys_paraset_file,
                                   tmp_pset, "", "", cmd_xterm_input_file, "SRC");
        if (!success)
        {
	    msg("INFORM: Failed to create directory for $1") << sys_paraset_file << eom;
            return 0;
        }
    }

    genString opt = customize::configurator_get();
    if (opt.length())
        opt += " ";
    if (search_pset)
        opt += "-pset_search";
    int status = configurator ("copy_pset", opt.str(), sys_paraset_file,
                               tmp_pset, vf, "", xterm_file, file_type);
    local_mod->invalidate_cache();
    invalidate_cache();

    return status;
}

static int check_out_pset_special(projModule *local_mod, char const *local_file, char const *vf,
                               char const *sys_paraset_file, char const *local_paraset_file,
                               char const *xterm_file, char const *tar_dir, int search_pset)

// copy pset from system area into the local area
//
// local_mod: the local module
// local_file: the local file name
// vf: the version file for the source or header file
// sys_paraset_file: the system pset name
// local_paraset_file: the local pset name
// xterm_file: the file contains the input keystrokes for the creation or the execution of scripts
// tar_dir: if not zero, it is the directory where the pset is copied to
// search_pset: It will be set for the header file when -batch is running. When set, it will search the previous
//              version of pset if the current header file does not have a corresponding pset. If it is not set,
//              no pset will be checked out if the current file does not have a corresponding pset.
{
    Initialize(check_out_pset_special);

    char const *file_type = "SRC";
    if (xref_not_header_file(local_file) && xref_not_src_file(local_file))
        file_type = "NOT_SRC";

    genString tmp_pset;
    if (tar_dir)
    {
        char const *fp = strrchr(local_paraset_file, '/');
        tmp_pset.printf("%s%s",tar_dir,fp);
    }
    else
        tmp_pset = local_paraset_file;

    if (dir_not_exist(tmp_pset))
    {
        int success = configurator("create_dir", "", sys_paraset_file,
                                   tmp_pset, "", "", cmd_xterm_input_file, "SRC");
        if (!success)
        {
	    msg("INFORM: Failed to create directory for $1") << sys_paraset_file << eom;
            return 0;
        }
    }

    genString opt = customize::configurator_get();
    if (opt.length())
        opt += " ";
    if (search_pset)
        opt += "-pset_search";
    int status = configurator ("copy_pset", opt.str(), sys_paraset_file,
                               tmp_pset, vf, "", xterm_file, file_type);
    local_mod->invalidate_cache();

    return status;
}


int projModule::get_module_comment(genString& comment)
// Get the comment for this module.
// It will ask the configurator for the comment.
// Will only work if the CM system supports it and a comment was already set.
{
    Initialize(projModule::get_module_comment);

    genString src_file;
    get_phys_filename(src_file);

    // Only check for comments if get-with-comments is on and
    // the CM System is either ClearCase or Custom.
    // Note - should check for continuus later.
    bool success = customize::configurator_get_comments() &&
                   ( (strcmp(customize::configurator_system(),"clearcase") == 0) || 
                     (strcmp(customize::configurator_system(),"custom") == 0) );

    if (success) success =  configurator ("get_comment", "", src_file, src_file,
                                          "", "", cmd_xterm_input_file, "SRC");

    int length = 0;
    if (success) {
        if (error_string.length() > 0) {
            char line[256];
            char *path = error_string;
            path[strlen(path)-1] = '\0'; // remove the linefeed
            FILE* in = OSapi_fopen(path, "r");
            int idx = 0;
            while(OSapi_fgets(line,255,in)) {
                comment += line;
            }
            OSapi_fclose(in);
            OSapi_unlink(path);
        } else {
            // Need to supply a blank to erase any current comment.
	    comment = " ";
        }
    }

    return(0);
}


int projModule::put_start(char const *bug, char const *comment, char const *files)
// send some data to source code control system for a group of files which will be checked-in in the same time
// Only the first module needs to call this routine
// It passes the system project name, the local project name, the bug number, the comment, and the local names of files
//
// bug: a file name that contains the bug number
// comment: a file name that contains the entered comment
// files: a file name that contains the names of the checked-in files
{
    Initialize(projModule::put_start);

    genString opt = customize::configurator_put();
    opt += " -sys_proj ";
    opt += get_target_project(0)->get_ln();
    opt += " -local_proj ";
    opt += get_project()->get_ln();

    return configurator ( "put_start", opt, bug, comment, files, "", cmd_xterm_input_file, "SRC");
}


extern void call_from_put (int);

int projModule::fast_put_module(projNode* dst_proj, char const *opt, char const *comment, char const */* rev */ ,
				char const *xterm_file_name)
// Do fast check in a file. (without pset & checks for merge)
//
// dst_proj: The target sys project
//
// opt: user"s entered options
//
// comment: user"s entered comment
//
// xterm_file_name: the file to be used when script is execyted. This file is treated as the input (keystrokes).
{
    Initialize (projModule::fast_put_module);

    call_from_put(1);  // set so that in Xref::~Xref we do not remove crash rec file

    projModule* src_mod;
    projNode*   src_proj;
    projModule* dst_mod;

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    src_mod  = this;
    src_proj = this->get_project();

    if (!get_file_names ((char const *)ln_name, src_proj, dst_proj, src_file, dst_file,
        src_paraset_file, dst_paraset_file))
        return 0;

    push_busy_cursor();
    dst_mod = dst_proj->find_module(ln_name);
    int new_dst_mod = 0;
    if (!dst_mod)
    {
        new_dst_mod = 1;
        projNode *cur_proj = projNode::set_home_proj_temp(dst_proj);
        dst_mod = projNode::make_new_module (dst_file);
        projNode::set_home_proj(cur_proj);
    }

    if (dst_mod == 0) {
      msg("ERROR: Cannot create file: $1 for project $2\nPlease create corresponding directory for the file, then try again") << (char const *)ln_name << eoarg << dst_proj->get_name() << eom;
      return 0;
    }

    // save working module if it was modified
    app *head = get_app();
    if (head && head->is_modified())
        this->save_module();

    int success = 1;
    // create destination module if necessary

    src_mod->invalidate_cache();
    dst_mod->invalidate_cache();

    genTmpfile tnm("put_");

    FILE *cmt = tnm.open_buffered();
    if (comment)
        OSapi_fprintf(cmt,"%s",comment);
    tnm.close();

    // Check in the source file
    if (success)
    {
        if (!configurator ("fast_put_src", opt, tnm.name(),
                           src_file, dst_file, "", xterm_file_name, "SRC"))
            success = 0;
    }

    genString fn;
    genString vn = "";
    if (success && db_get_db_format(src_file))
    {
        get_stat_string("sf", fn);
        get_stat_string("vf", vn);
    }

    struct OStype_stat buf1;
    int ss = global_cache.stat(src_file,&buf1);
    if (ss != 0)
        OSapi_unlink(src_file);
    tnm.unlink();

    if (new_dst_mod)
    {
        projNode *cur_proj = projNode::set_home_proj_temp(dst_proj);
        obj_delete(dst_mod);
        projNode::set_home_proj(cur_proj);
    }

    pop_cursor();
    return success;
}

int projModule::notify_put_module(projNode* dst_proj, char const *opt, char const *comment, char const */* rev */ ,
				  char const *xterm_file_name)
// Do internal check in. (model only)
//
// dst_proj: The target sys project
//
// opt: user"s entered options
//
// comment: user"s entered comment
//
// xterm_file_name: the file to be used when script is execyted. This file is treated as the input (keystrokes).
{
    Initialize (projModule::notify_put_module);

    call_from_put(1);  // set so that in Xref::~Xref we do not remove crash rec file

    projModule* src_mod;
    projNode*   src_proj;
    projModule* dst_mod;

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    src_mod  = this;
    src_proj = this->get_project();

    if (!get_file_names ((char const *)ln_name, src_proj, dst_proj, src_file, dst_file,
        src_paraset_file, dst_paraset_file))
        return 0;

    push_busy_cursor();
    dst_mod = dst_proj->find_module(ln_name);
    int new_dst_mod = 0;
    if (!dst_mod){
	pop_cursor();
	return 0;
    }

    // save working module if it was modified
    app *head = get_app();
    if (head && head->is_modified())
        this->save_module();

    int success = 1;
    // create destination module if necessary

    src_mod->invalidate_cache();
    dst_mod->invalidate_cache();

    genString fn;
    genString vn = "";
    if (success && db_get_db_format(src_file))
    {
        get_stat_string("sf", fn);
        get_stat_string("vf", vn);
    }

    pop_cursor();
    return success;
}

bool delete_module_internal_call_cm (projModule *This, int type,
				     genString src_file,
				     genString dst_file,
				     genString src_paraset_file,
				     genString dst_paraset_file)
{
    Initialize("cm_delete_module_internal");

    if (type == 0)
    {
	configurator ("delete_src", customize::configurator_delete(),
		      src_file, dst_file, "", "", cmd_xterm_input_file, "SRC");
	if (has_paraset_file(dst_paraset_file))
	    configurator ("delete_pset", customize::configurator_delete(),
			  src_paraset_file, dst_paraset_file, "", "",
			  cmd_xterm_input_file, "SRC");
    }
    else // if (type == 1)
    {
	// boris: figure out if the files is newly created in the local area.
	bool src_existed = false;
#ifndef _WIN32
	if (strcmp(src_file, dst_file) != 0)  // not clearcase 
	    src_existed = (OSapi_access ((char const *)src_file, R_OK) == 0);
	else {
	    int len = strlen(src_file) + strlen(clearcase_signature) + 13;
	    char *clear_fn = new char [len];
	    strcpy (clear_fn, src_file);
	    strcat (clear_fn, clearcase_signature);
	    src_existed = (OSapi_access (clear_fn, R_OK) == 0);
	    delete clear_fn;
	}
#else // _WIN32
	src_existed = configurator ("stat", 0, src_file, dst_file, "", "",
				    cmd_xterm_input_file, "SRC");
#endif // _WIN32

	if ( src_existed )
	{
	    configurator ("unget_src", customize::configurator_unget(),
			  src_file, dst_file, "", "",
			  cmd_xterm_input_file, "SRC");
	    configurator ("unget_pset", customize::configurator_unget(),
			  src_paraset_file, dst_paraset_file, "", "",
			  cmd_xterm_input_file, "SRC");
	}
	else
	{
	    browser_delete_module(0, This, false, "This is a new file.");
	    return true;
	}
    }
    return false;
}

void delete_module_internal_call_triggers (projModule *This,
					   int new_flag, int type)
{
    if (new_flag == 0 || new_flag == 1) {
	symbolArr sym_files;
	symbolPtr file_sym = This;
	file_sym = file_sym.get_xrefSymbol();
	sym_files.insert_last(file_sym);
	if (type == 0)
	    sdo_delete_trigger(sym_files);
	else if (type == 1)
	    sdo_unco_trigger(sym_files);
    }
}

void delete_module_internal_finish (char const *name, projNode *dest_proj)
{
    scope_mgr.report_cm_command (CM_DELETE, dest_proj, (projNode *)NULL, name);

    theUI->refreshAfterModuleDelete();
}

// asynchronous interfaces for put/get/copy

int projModule::cfg_fast_put (projNode* proj, char const *opt, char const *comment)
{
    return fast_put_module (proj, opt, comment, 0, cmd_xterm_input_file);
}


#ifndef _WIN32
void put_psets(objArr& modules)
{
    Initialize(put_psets);

    if(modules.size() == 0)
	return;
//=========================================
    // Generate script to put pset files
    genTmpfile tmpfile("putscr");
    FILE *out_file = tmpfile.open_buffered();
    for(int i = 0; i < modules.size(); i++){
	projModule* module = checked_cast(projModule,modules[i]);
	genString src_file;
	genString src_paraset_file;
	genString dst_file;
	genString dst_paraset_file;
	module->get_file_names_internal (src_file, dst_file,
					 src_paraset_file, dst_paraset_file);
	fprintf(out_file, "%s\n", (char const *)src_paraset_file);
	fprintf(out_file, "%s\n", (char const *)dst_paraset_file);
    }
    tmpfile.close();
    // Execute script to put files
    genString cfg = OSapi_getenv("PSETCFG");
    if (cfg.str()){
	cfg += "/cm_fast_put_pset";
	if (!OS_dependent::access (cfg, X_OK)) {
	    if (vsysteml (cfg, tmpfile.name(), NULL)){
		fprintf(stderr, "Unable to execute cm_fast_put_pset\n");
		fflush(stderr);
	    }    
	}
    }
    OSapi_unlink(tmpfile.name());
//=========================================

}
#endif

//===========================================================
void NT_notify_checkin( projModule *mod, char const *fname, char const *comment)
{
  if ( fname )
  {
    msg("-----checkin----\n$1") << fname << eom;
    fflush(stdout);
  }
}
//===========================================================
static genString cm_event_comment;

#ifndef _WIN32
int dis_cm_event_put_internal( projModule *mod, char const *comment )
{
  Initialize(dis_cm_event_put_internal);

  NT_notify_checkin( mod, 0, comment );

  objArr successfuly_put;
  int success = mod->notify_put_module(mod->get_target_project(0), "", comment, "", "");
  if(success)
	successfuly_put.insert_last(mod);
  // Create script to put pset files
  put_psets(successfuly_put);
  for(int i = 0; i < successfuly_put.size(); i++)
  {
	  projModule* module = checked_cast(projModule,successfuly_put[i]);
	  module->update_local_rtl_pmod();
	  void* dummy = NULL;
	  view_flush_rtls(dummy);
  }

	return successfuly_put.size();
}
#endif

//===========================================================

extern int xref_hook_turn_off;

int projModule_set_version (app *ah)
{
    Initialize(projModule_set_version);
    int res = -1;
    if (!ah)                 return res;
    if (!is_model_build())       return res;
    //boris 980303: do not create DD_VERSION for the sake of old scripts
    if (!cmd_available_p(cmdCreateDD_VERSION)) return res;
    if (customize::no_cm())  return res;

    if (is_smtHeader(ah)) {
	smtHeader *smth = checked_cast(smtHeader,ah);
	if (smth->srcbuf == 0)
	    return res;
    }

    ddRoot* dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, ah));
    if (!dr) return res;

    ddElement *dd_mod = dr->lookup (DD_MODULE,ah->get_filename());
    if (!dd_mod) return res;
    projModule *pm = ah->get_module ();
    if (!pm) return res;

    ddElement *dd_ver = checked_cast(ddElement,get_relation(has_friend,dd_mod));
    if (dd_ver) {
	rem_relation(has_friend,dd_mod,dd_ver);
	delete dd_ver;
	dd_ver = 0;
    }

    genString ver;
    pm->get_rev_from_source (ver);
    if (ver.length() == 0)
	ver = MRG_NOREVISION;
    
    xref_hook_turn_off = 1;
    {
	dd_ver = dr->add_def (DD_VERSION, ver.length(), (char const *)ver, 0, "", 0, "");
	if (dd_ver) {
	    dd_ver->set_is_def();
	    dd_ver->set_def_file(pm->get_name());
	    put_relation (has_friend,dd_mod,dd_ver);
	}
    }
    xref_hook_turn_off = 0;
    return 0;
}

int projModule_set_version_start (projModule* pm)
{
    Initialize(projModule_set_version_start);
    int res = -1;
    if (!is_model_build())       return res;
    if (!cmd_available_p(cmdCreateDD_VERSION)) return res;
    if (customize::no_cm())  return res;
    if (!pm) return res;

    genString src_file;
    pm->get_phys_filename(src_file);
    configurator_internal ( "src_version", "", src_file, src_file, "", "", cmd_xterm_input_file, "SRC", 1);

    return 0;
}

void projModule_get_built_revision (projModule *mod, genString& ver)
{
    Initialize(projModule_get_built_revision);
    ver = 0;
    symbolPtr sym = mod;
    sym = sym.get_xrefSymbol();
    if (sym.xrisnotnull()) {
	symbolArr as;
	if (sym->get_link(have_friends, as)) {
	    symbolPtr vsym = as[0].get_xrefSymbol();
	    if (vsym.get_kind() == DD_VERSION)
		ver = vsym.get_name();
        }
    }
}

int projModule_retrieve_revision (projModule *mod, char const *rev, char const *dst_fname)
{
    Initialize(projModule_retrieve_revision);
    int res = -1;

    if (!mod || !rev || !dst_fname) return res;

    char const *src_file = mod->get_phys_filename ();
    genString cmd = customize::configurator_get();
    cmd += " -ParaRev ";
    cmd += rev;

    int success = configurator ("copy_src", cmd, src_file, dst_fname, "", "", cmd_xterm_input_file, "SRC");
    if (success != 0)
       res = 0;

    return res;
}


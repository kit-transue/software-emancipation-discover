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
#include <cLibraryFunctions.h>
#include <machdep.h>
#include <globals.h>
#include <psetmem.h>
#include <customize.h>
#include <genString.h>
#include <genError.h>
#include <objOper.h>
#include <MemSeg.h>
#include <RTL.h>
#include <systemMessages.h>
#include <setprocess.h>
#include <proj.h>
#include <path.h>
#include <path1.h>
#include <cmd.h>
#include <genTmpfile.h>
#include <xxinterface.h>
#include "tcl.h"
#include <options.h>
#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#ifdef _WIN32 
#include <windows.h>
#include <evalMetaChars.h>
#else
#include <sys/resource.h>
void    create_tmp_dir();
#endif

#include <genError.h>
#include "Interpreter.h"
#include "QueryTree.h"

int service_loop(long);
int service_init(int argc, char**argv);

#include <proj_save.h>
#include <disbuild_analysis.h>
#include <disbuild_update.h>

#ifdef _WIN32
typedef DWORD (WINAPI * UT32PROC)(LPVOID lpBuff, DWORD dwUserDefined,
	LPVOID *lpTranslationList);

typedef BOOL (WINAPI * PUTREGISTER)(HANDLE hModule, LPCSTR SixteenBitDLL,
	LPCSTR InitName, LPCSTR ProcName, UT32PROC* ThirtyTwoBitThunk,
	FARPROC UT32Callback, LPVOID Buff);

typedef VOID (WINAPI * PUTUNREGISTER)(HANDLE hModule);

extern "C" {
static PUTUNREGISTER UTUnRegister = NULL;
HINSTANCE TclWinGetTclInstance();
int TclWinGetPlatformId();
}
#endif


//// DECLARATIONS THAT SHOULD BE IN HEADER FILES
extern "C" void driver_exit(int status);
void load_pdfs_complete();
void re_generate_pdf_tree_for_refresh_top_proj();
extern "C" void aset_reset_signals();
void load_project_rtl (int num_of_pdf_files, genString* pdf_file);
void proj_create_ctl_pm();
void pmod_set_signals();
extern void InitializeSocketServer(const char *service);

static int StopServerCmd(ClientData cd, Tcl_Interp * interp, int argc, char const *argv[]) {
   Interpreter *i = (Interpreter *)cd;
   if(argc<2) {
       Tcl_AppendResult(i->interp,"Usage: stop_server -y",NULL);
       return TCL_ERROR;
   }
   if(strcmp(argv[1],"-y")==0) driver_exit(0);
   return TCL_OK;
}

static int init() {
   new cliCommandInfo("stop_server", StopServerCmd);
#ifdef XXX_have_xml_parser_for_qar_qtree
   QTreeCmd::Register();
#endif
   return 0;
}

static int dummy = init();

int do_debug    = 0;
FILE* log_fd = stderr;


extern ostream* pmod_log_stream;
static char*   log_file_name ;

char *server_name			= NULL;
static bool is_model_build_flag         = false;
static bool is_model_server_flag        = false;

bool is_aset()
{
    return false;
}

bool is_model_build()
{
    return is_model_build_flag;
}
 
bool is_model_server()
{
    return is_model_server_flag;
}
 
bool is_gui()
{
    return false;
}
 
char *cli_script = NULL;
extern bool  cli_exit;
//const char* main_argv_0;

int proj_count;
int source_file_count;
int total_file_count;
int pset_exists_count; 
int pset_up_to_date_count;
int entry_in_pmod_count;
int entry_in_pmod_uptodate_count ;

void dd_get_pmod_subprojects(objArr&);
extern void dd_build_xref (int num_load, char const **load_files,
			   fileLanguage *, int num_remove, char const **remove_files,
			   char const *home_proj_name, int save_files,
			   int save_includes, int update_xref,
			   char const **visible_projects, char const *);

extern "C" void dd_print_xref (char *xref_proj_name);


int CannotSendFlag = 0;

extern int ok_to_delete_old_pmod;

extern void simplify_batch(char const *proj_name);
extern void output_dependency_modules(char const *);

extern void share_re_load_pdf(char *pdf);
extern void prj_load_pdfs_start ();
extern void prj_load_pdfs_end ();

extern "C"
{
    void buildxref_set_signals();
    void _exit (int status);

    void gen_stack_prt(int level);
    bool do_pmod_command(int num_of_pmod_files, genString* pmod_file, const char* pmod_command);
}

//// STATIC DECLARATIONS
static void load_pdfs();





static void load_pdfs_xref_main ()
{
    load_pdfs();
    re_generate_pdf_tree_for_refresh_top_proj();
}

int get_option_genStrings(const char* opt, genString* opt_arr, int max_opt)
//  Looks for one or more occurrences of the specified
//  opt switch (starts with -) and reads all the following values
//  that do not have a leading - character into the opt_arr
{
    int ii = 0;
    if(ii >= max_opt)
	return 0;
    char* oo;
    do {
	oo = get_main_option(opt);
	if (oo != 0) {
	    opt_arr[ii++] = oo;
	    opt = 0;
	}
    } while (ii < max_opt && oo != 0);
    return ii;
}

//------------------------------------------
// driver_client_hook
//
// This function gets called whenever there is a client XEvent.
// This is used for getting communique from epoch
//------------------------------------------


void fast_exit(int exit_status);
static int already_exiting = 0;
extern "C" int driver_exiting() 
{
    return already_exiting;
}

extern "C" void driver_exit(int exit_status)
{
    if (already_exiting)
	fast_exit (exit_status);

    already_exiting = 1;
    projNode::terminate();

#ifndef _WIN32
    signal(SIGHUP, SIG_IGN);
#endif
    aset_reset_signals();

#ifdef _WIN32
	remove_tmp_dir();
#endif

    // Use our own wrapper for exiting
    fast_exit(exit_status);
}

void pmod_main_trap_handler(int)
{
    static int count = 0;

    Initialize(pmod_main_trap_handler);
    
    if (driver_exiting())
	driver_exit(1);
    
    //  Make sure exit code indicates crash
    globals::error_status(CRASH_ERROR);

    // disable copying of potentially corrupted temporary pmod 
    // and notify user
    ok_to_delete_old_pmod = 0;
    
    if ( count++ > 100 )
	driver_exit(1);

    pmod_set_signals();

    Error(ERR_INPUT);
}

void pmod_exit_trap_handler(int)
{
    Initialize(pmod_exit_trap_handler);
    
    if (driver_exiting())
	driver_exit(1);
    
    //  Make sure exit code indicates crash
    globals::error_status(CRASH_ERROR);

    // disable copying of possibly corrupt temporary pmod 
    ok_to_delete_old_pmod = 0;

    gen_stack_prt (1);

    driver_exit(1);
}

extern "C" void main_aset_trap_handler(int x) 
  { pmod_main_trap_handler(x);}
extern "C" void xref_aset_trap_handler(int x) 
  { pmod_exit_trap_handler(x);}

static void quit_handler (int)
{
    driver_exit(1);
}

void pmod_set_signals()
{
    signal( SIGILL, pmod_main_trap_handler );
#ifndef _WIN32
    signal( SIGBUS, pmod_main_trap_handler );
    signal( SIGQUIT, pmod_exit_trap_handler );
#endif
    signal( SIGSEGV, pmod_main_trap_handler );
#if defined(hp10)
    signal( SIGIOT, pmod_main_trap_handler );
#endif
    signal (SIGINT, quit_handler);
}


void verify_load_pdf(const char *fn)
// load a pdf file for the verify_pdf mode
//
// the pdf file name is fn
{
    Initialize(verifyy_load_pdf);

    genString pdf_file[2];
    pdf_file[0] = fn;
    load_project_rtl(1, pdf_file);
}


static void load_pdfs()
// load the pdf files used by pdf_debugger
{
    Initialize(load_pdfs);

    genString pdf_file[1024];
    int num_of_pdf_files = get_option_genStrings ("-pdf", pdf_file, 1024);
    load_project_rtl(num_of_pdf_files, pdf_file);
}

void re_load_pdfs(const char *pdf_filename)
// re-load the pdf file by file->Update projects->Load a new pdf file
//
// pdf_filename is the entered pdf file name
{
    Initialize(load_pdfs);

    clean_pdf_file_name();
    projList::search_list->writable(0);

    genString pdf_file[2];
    pdf_file[0] = pdf_filename;
    pdf_file[1] = 0;

    load_project_rtl(1, pdf_file);
}


static bool local_check_proj_name_for_build(projNode *pn, const char *proj_name)
{
    Initialize(local_check_proj_name_for_build);

    if (check_proj_name_for_build(pn, proj_name))
	return true;

    (*pmod_log_stream) << TXT("Error: Some modules exist higher than project ") << proj_name
    << TXT(" in the project hierarchy.") << endl;
    return false;
}

typedef struct
{
    char *proj_name;
} proj_name_entry;

genArr(proj_name_entry);

static genArrOf(proj_name_entry) proj_name_head;

static bool search_matched_project_name(projNode *pn, WildPair **m, int max_level, bool no_check)
{
    Initialize (projNode::search_matched_project_name);

    char const *pn_name = pn->get_ln();

    int level = count_level(pn_name) - 1;
    if (level >= max_level)
	return false;

    int len = m[level]->match(pn_name, MATCH_WITH_END);

    if (len == -1)
    {
	len = m[level]->match(pn_name, MATCH_WITHOUT_END);

	if (len == -1)
	    return false;
    }

    if (level == max_level - 1 && len == strlen(pn_name))
    {
	if (no_check || local_check_proj_name_for_build(pn, pn_name))
	{
	    proj_name_entry *p = proj_name_head.grow();
	    p->proj_name = OSapi_strdup(pn_name);
	    return true;
	}
	return false;
    }

    pn->refresh_projects();

    symbolArr& proj_contents = pn->rtl_contents();
    symbolPtr sym;
    bool no_check_flag = false;
    ForEachS(sym, proj_contents)
    {
	if (sym.get_kind() == DD_PROJECT)
	{
	    // child node might be what we want
	    appPtr app_head = sym.get_def_app();
	    if(app_head && is_projHeader(app_head))
	    {
		projNode *child_proj = projNodePtr(app_head->get_root());
		no_check_flag = search_matched_project_name(child_proj, m, max_level, no_check_flag);
		if (!no_check_flag)
		    break;
	    }
	}
    }
    return false;
}

static void convert_project_wild_cards(const char *p_name)
{
    Initialize(convert_project_wild_cards);

    genString new_s;
    if (p_name)
    {
	if (*p_name == '\"')
	    new_s = p_name + 1;
	else
	    new_s = p_name;
	char *p = (char *)new_s.str();
	if (p[new_s.length()-1] == '\"')
	    p[new_s.length()-1] = '\0';
    }
    int i;
    for (i=0;i<proj_name_head.size();i++)
    {
	proj_name_entry *p = proj_name_head[i];
	psetfree(p->proj_name);
    }
    proj_name_head.reset();

    if (!new_s.length())
	return;

    if (!need_expand_dir(new_s))
    {
	projNode* pn = projNode::find_project(new_s);
	if (!pn)
	{
	    (*pmod_log_stream) << TXT("Project ") << new_s.str() << TXT(" does not exist.") << endl;
	    return;
	}
	if (local_check_proj_name_for_build(pn, new_s))
	{
	    proj_name_entry *p = proj_name_head.grow();
	    p->proj_name = OSapi_strdup(new_s.str());
	}
    }
    else
    {
	WildPair **px;
	int level = count_level(new_s);
	px = (WildPair **)malloc(sizeof(WildPair *) * level);
	char *p = (char *)new_s.str() + 1;
	for(i=0;i<level;i++)
	{
	    p = strchr(p,'/');
	    if (p)
		*p = '\0';
	    px[i] = db_new(WildPair, (new_s));
	    if (p)
	    {
		*p = '/';
		p++;
	    }
	}

	projNode *proj;

	for (i=1; proj = projList::search_list->get_proj(i); i++)
	{
	    if (!proj->is_paraset_project())
		search_matched_project_name(proj, px, level, false);
	}

	for(i=0;i<level;i++)
	    delete px[i];
	psetfree(px);
    }
}

void load_and_refresh_projects(genString const &xref_projname)
{
    projNode *cproj;
    if(xref_projname.length()) {
        cproj = projNode::find_project(xref_projname);
        if (!cproj) {
            msg("Project $1 does not exist.", error_sev)
                << xref_projname.str() << eoarg << eom;
            driver_exit(0);
        }
        projNode::set_home_proj(cproj);
    } else {
        proj_create_ctl_pm();
        cproj = projNode::get_control_project();
    }
    projNode::set_current_proj (cproj);
    load_pdfs_complete();
       
    projNode *ctp = projNode::get_control_project();
    projNode *proj;

    for (int i = 1; (proj=projList::search_list->get_proj(i)) ; ++i) {
        if (proj != ctp)
            proj->refresh_all();
    }
}


void do_model_build_analysis(int num_of_projects, genString projects[],
                             int update_xref,
                             char const *visible_projects [1])
{
    char const *load_files[1024];
    fileLanguage load_file_types [1024];
    int num_of_load_files = 0;
    int save_files = 1;
    int save_includes = 1;
    char const *remove_files[1024];
    int num_of_remove_files = 0;
    char *report_file = NULL;

    // analysis mode takes the following values:
    // 0 - regular dependency
    // 1 - local
    // 2 - merge into local
    // 3 - summarize
    int analysis_mode = disbuild_analysis_mode ();
    if (analysis_mode == 3) {
        disbuild_analysis_summarize ();
    } else if (analysis_mode == 2) {
        disbuild_analysis_merge ();
    } else {
        if (num_of_projects) {
            if (analysis_mode) {
                disbuild_analysis_start (num_of_projects);
            }
            for(int i = 0; i < num_of_projects; i++) {
                convert_project_wild_cards(projects[i].str());
                for (int j=0;j<proj_name_head.size();j++) {
                    proj_name_entry *p = proj_name_head[j];
                    if (analysis_mode)
                        disbuild_analysis_proj (p->proj_name);
                    else {
                        msg("Dependency analysis for project $1") << p->proj_name << eom;
                        dd_build_xref(num_of_load_files, load_files,
                                      load_file_types,
                                      num_of_remove_files,
                                      remove_files, p->proj_name,
                                      save_files, save_includes,
                                      update_xref, visible_projects,
                                      report_file);
                    }
                }
            }
            if (analysis_mode) disbuild_analysis_complete ();
        } else {
            if (analysis_mode) {
                disbuild_analysis_pmod_projects ();
            } else {
                /* Do first pass (dependency analyzis)*/
                objArr sub_projects;
                dd_get_pmod_subprojects(sub_projects);
 
                for(    int proj_index = 0;
                        proj_index < sub_projects.size();
                        proj_index++) {
                    projNode *sub_proj = (projNode *)sub_projects[proj_index];
                    msg("Dependency analysis for project $1")
                        << sub_proj->get_ln() << eom;
                    dd_build_xref(num_of_load_files, load_files,
                                  load_file_types,
                                  num_of_remove_files, remove_files,
                                  sub_proj->get_ln(), save_files,
                                  save_includes, update_xref,
                                  visible_projects, report_file);
                }
            }
        }
 
        if (analysis_mode == 0) output_dependency_modules(report_file);
    }
} //do_model_build_analysis()

void do_model_build_update(int num_of_projects, genString projects[],
                           int multiple_proj_flag, int remove_modules)
{
    char const *load_files[1024];
    fileLanguage load_file_types [1024];
    int num_of_load_files = 0;
    int save_files = 1;
    int save_includes = 1;
    char const *remove_files[1024];
    int num_of_remove_files = 0;
    char *report_file = NULL;

    int res = -1;
    if (!multiple_proj_flag) {
        disbuild_update_usage_error("No project name after -update option");
    } else if (num_of_projects != 1) {
        disbuild_update_usage_error ("More than one project name after -update option");
    } else {
        convert_project_wild_cards(projects[0].str());
        proj_name_entry *p = proj_name_head[0];
        res = disbuild_update_project (p->proj_name, remove_modules);
    }
    driver_exit (res);
}

void
do_model_build_multiple(int num_of_projects, genString projects[],
                        int multiple_proj_flag,
                        int update_xref,
                        char const *visible_projects [1])
{
    char const *load_files[1024];
    fileLanguage load_file_types [1024];
    int num_of_load_files = 0;
    int save_files = 1;
    int save_includes = 1;
    char const *remove_files[1024];
    int num_of_remove_files = 0;
    char *report_file = NULL;

    for(int i = 0; i < num_of_projects; i++)
        {
            convert_project_wild_cards(projects[i].str());
            for (int j=0;j<proj_name_head.size();j++)
                {
                    proj_name_entry *p = proj_name_head[j];
 
                    msg("Updating project $1 ") << p->proj_name << eom;
                    dd_build_xref(num_of_load_files, load_files,
                                  load_file_types, num_of_remove_files,
                                  remove_files, p->proj_name, save_files,
                                  save_includes, update_xref,
                                  visible_projects, report_file);
                }
        }
}

int
do_model_build_default(genString &xref_projname,
                        int update_xref,
                       char const *visible_projects [1],
                       int num_of_pmod_files,
                       genString pmod_file[],
                       genString pmod_command )
{
    char const *load_files[1024];
    fileLanguage load_file_types [1024];
    int num_of_load_files = 0;
    int save_files = 1;
    int save_includes = 1;
    char const *remove_files[1024];
    int num_of_remove_files = 0;
    char *report_file = NULL;

    projNode* pn = projNode::find_project(xref_projname);
    if (!pn)
        {
            msg("Project $1 does not exist.", error_sev)
                << xref_projname.str() << eom;
            return 0;
        }
    if (!local_check_proj_name_for_build(pn, xref_projname))
        return 0;
 
    dd_build_xref(num_of_load_files, load_files, load_file_types,
                  num_of_remove_files, remove_files, xref_projname,
                  save_files, save_includes, update_xref,
                  visible_projects, report_file);
 
    if (num_of_pmod_files)
        do_pmod_command (num_of_pmod_files, pmod_file, pmod_command.str(
));
}


// =======================================
int server_main(int argc, char **argv, char **envp)
{
    Initialize(server_main);

    int update_xref = 0;

    is_model_server_flag = check_main_option("-service");
    int batch_flag = check_main_option("-batch");

    genString pmod_file[1024];
    int num_of_pmod_files = 0;
    genString pmod_command="";
    
    genString pdf_file[1024];
    int num_of_pdf_files = 0;

    char const *visible_projects [1];
    visible_projects[0] = NULL;

    genString projects[1024];
    int num_of_projects = 0;

    genString xref_projname = NULL;

    const char *model_service_name;

    /*this flag passed onto disbuild_update_project to delete modules
    without psets.*/
    bool remove_modules = 0;

    /* If the following variable is true, "Dislite:" prefix will NOT be added
       to the name of the service, thus making it NOT visible through dish2. */
    bool internal_service = false;

    enum option_type {
	ILLEGAL,
	TEE,
	PMOD,
        REMOVE,
	HOME,
	PRINT,
	PREFS,
	SAVE_FILES,
	PROJECT_FILE,
	PROJECT_NAME,
	CLI_SCRIPT,
	EXT_PROJ_LISTS,
	CM_REPORT,
	EXTRACT,
	LOG_FILE,
	SERVER_NAME,
	SERVICE_NAME,
	LP_SERVICE,
	AUTO_SHUTDOWN

    };

    option_type option, last_option;

    long auto_shutdown       = -1;

    genString all_file_string("*");
    genString sub_system_string("*");
    genString function_string("*");
    genString variable_string("*");
    genString class_string("*");
    genString struct_string("*");
    genString union_string("*");
    genString enum_string("*");
    genString typedef_string("*");
    genString macro_string("*");

    int analysis_flag      = 0;
    int update_flag        = 0;
    int multiple_proj_flag = 0;

    bool command_line_error = false;    // no errors so far

    if (!is_model_server_flag && !batch_flag) {
	msg ("Either -service or -batch must be specified.", error_sev) << eom;
	command_line_error = true;
    }

    // setup arguments in options structure
    set_main_options (argc, argv);

    // -save_proj : save project structure
    if (check_main_option("-save_proj")) {
	char *proj_name = get_main_option("-save_proj");
	buildxref_set_signals();
	prj_load_pdfs_start();
	load_pdfs_xref_main();
	prj_load_pdfs_end();
	int res = 0;
	if (proj_name && proj_name[0]) {
	    res = proj_save (proj_name, 0);
	}
	else
	    res = proj_save_all (0);
 
	driver_exit (0);
    }

    // -save_root : save all ROOT projects and save PMOD projects
    //              only if the .prj file is outdated
    if (check_main_option("-save_root")) {
	char *proj_name = get_main_option("-save_root");
	buildxref_set_signals();
	prj_load_pdfs_start();
	load_pdfs_xref_main();
	prj_load_pdfs_end();
	int res = 0;
	if (proj_name && proj_name[0]) {
	    res = proj_save (proj_name, 1);
	}
	else
	    res = proj_save_all ( 1 );
 
	driver_exit (0);
    }

    customize::set_write_flag(0);   // most options will not write to pmod

    last_option = option = ILLEGAL;

    char *ext_proj_name[1024];
    int ext_proj_name_count = 0;

    int i;
    for (i = 1; i < argc; i++) {
      char *swtch = argv[i];
      if (*swtch == '-') {
	if (!strcmp(swtch, "-batch")) {
	  cli_exit = true;
	  continue;
	}
	if (!strcmp(swtch,"-tee")) {
	  option = last_option = TEE;
	  continue;
	}
	if (!strncmp(swtch, "-pmod", 5) || !strncmp(swtch, "-pset", 5)) {
	  // only certain ones are legal
	  if (!do_pmod_command(-1, 0, swtch)) {
	    if (pmod_command.length()) {
	      msg("Commandline switch: $1 illegal after $2", error_sev) << swtch << eoarg << pmod_command.str() << eom;
	      option=last_option=ILLEGAL;
	      continue;
	    }
		
	    pmod_command = swtch;
	    option = last_option = PMOD;
	    continue;
	  }
	}
	if (!strcmp(swtch, "-core")) {
	  // we separately checked for this, so just skip the switch
	  option=last_option=ILLEGAL;
	  continue;
	}
	if (!strcmp(swtch, "-pdf")) {
	  option = last_option = PROJECT_FILE;
	  continue;
	}
	if (!strcmp(swtch, "-prefs")) {
	  option = last_option = PREFS;
	  continue;
	}
	if (!strcmp(swtch, "-analysis")) {
	  if(multiple_proj_flag) {
	    multiple_proj_flag = 0;
	    msg("Switch -update is ignored with -analysis", warning_sev) << eom;
	  }
	  is_model_build_flag = true;
	  update_xref     = 2;
	  analysis_flag   = 1;
	  xref_projname   = "/";
	  option=last_option = PROJECT_NAME;
	  customize::set_write_flag(1);
	  continue;
	}
	if (!strcmp(swtch, "-proj_list"))
	{
	    option = last_option = EXT_PROJ_LISTS;
	    continue;
	}
	// SS: EXAMINE THESE TWO
	if(!strcmp(swtch, "-source")){
	    option = last_option = CLI_SCRIPT;
	    cli_exit = true;
	    continue;
	}
	if(!strcmp(swtch, "-init")){
	    option = last_option = CLI_SCRIPT;
	    cli_exit = false;
	    continue;
	}

	if (!strcmp(swtch, "-update")) {
	  if (analysis_flag) {
	    msg("Switch -update is ignored with -analysis", warning_sev) << eom;
	    option=last_option = PROJECT_NAME;
	    continue;
	  }
	  is_model_build_flag = true;
	  update_xref        = 3;
	  multiple_proj_flag = 1;
	  update_flag        = 1;
	  xref_projname      = "/";
	  option = last_option = PROJECT_NAME;
	  customize::set_write_flag(1);
	  continue;
	}

        if (!strcmp(swtch, "-remove")) {
          remove_modules = 1;
          continue;
        }

	if (!strcmp(swtch, "-dfa_init")) {
	  is_model_build_flag = true;
	  update_xref     = 7;
	  multiple_proj_flag = 1;
	  xref_projname   = "/";
	  option = last_option = PROJECT_NAME;
	  customize::set_write_flag(1);
	  continue;
	}
	if (!strcmp(swtch, "-dfa")) {
	  is_model_build_flag = true;
	  update_xref     = 6;
	  multiple_proj_flag = 1;
	  xref_projname   = "/";
	  option = last_option = PROJECT_NAME;
	  customize::set_write_flag(1);
	  continue;
	}

	if(!strcmp(swtch, "-home")){
	  option = HOME;
	  continue;
	}
 
	if (!strcmp(swtch, "-log_file")) {
	    option = LOG_FILE;
	    continue;
	}
	if (!strcmp(swtch, "-service")) {
	    cli_exit = false;
	    option = SERVICE_NAME;
	    continue;
	}
	if (!strcmp(swtch, "-server_name")) {
	    cli_exit = false;
	    option = SERVER_NAME;
	    continue;
	}
	if (!strcmp(swtch, "-internal_service")) {
            internal_service = true;
	    continue;
	}
	if (!strcmp(swtch, "-auto_shutdown")) {
	    auto_shutdown = 0;
	    option = AUTO_SHUTDOWN;
	    continue;
	}
 
	if (!strcmp(swtch, "-lpservice") || !strcmp(swtch, "-message_group")) {
	    option = LP_SERVICE;
	    continue;
	}

	command_line_error = true;
	msg("Command line switch $1 unrecognized, or invalid in this context", error_sev) << swtch << eom;
	option=last_option=ILLEGAL;
      } else {

	switch (option) {

	case ILLEGAL:
	  // this parameter follows a switch that does not take arguments
	  msg("Unexpected argument : $1", error_sev) << swtch << eom;
	  command_line_error = true;
	  break;

	case TEE:
	  break;

	case PMOD:
	  // this parameter follows a switch that processes an external pmod
	  pmod_file[num_of_pmod_files++] = argv[i];
	  break;
		
	case HOME:  // -home switch
	  {
	    char *p = argv[i];
	    if (*p != '/')
	      xref_projname.printf("/%s",p);
	    else
	      xref_projname = p;

	    option = last_option;
	    break;
	    
	  }

	case PROJECT_FILE:
	  // specify pdf file
	  if (num_of_pdf_files < 1024) {
	    pdf_file[num_of_pdf_files++] = argv[i];
	  } else {
	    static bool not_reported = true;
	    if (not_reported) {
	      msg("Too many pdf files", error_sev) << eom;
	      not_reported = false;
	    }
	  }
	  break;

	case PROJECT_NAME:
	  // specify project name
	  if (num_of_projects < 1024) {
	    projects[num_of_projects++] = argv[i];
	  } else {
	    static bool not_reported = true;
	    if (not_reported) {
	      msg("Too many project names", error_sev) << eom;
	      not_reported = false;
	    }
	  }
	  break;
	  
	case CLI_SCRIPT:
	  cli_script = argv[i];
	  option     = ILLEGAL;
	  break;
		
	case PREFS:
	  // Preference options are processed elsewhere
	  break;

	case EXT_PROJ_LISTS:
	  if (ext_proj_name_count < 1024) {
	    ext_proj_name[ext_proj_name_count++] = argv[i];
	  } else {
	    static bool not_reported = true;
	    if (not_reported) {
	      msg("Too many projects in proj_list", error_sev) << eom;
	      not_reported = false;
	    }
	  }
	  break;

	case LOG_FILE:
	  log_file_name = argv[i];
	  pmod_log_stream = new ofstream(log_file_name, ios::out);
	  if(!pmod_log_stream ){
	    msg("failed opening for write log-file $1\n") << log_file_name << eom;
	    pmod_log_stream = &cout;
	  } else {
#ifdef _WIN32
            cout.rdbuf(pmod_log_stream->rdbuf());
            cerr.rdbuf(pmod_log_stream->rdbuf());
#endif // _WIN32
	  }
	  break;

	case SERVER_NAME:
	  server_name = argv[i];
	  break;

	case SERVICE_NAME:
	  model_service_name = argv[i];
	  break;

	case LP_SERVICE:
	  // already taken care of in main function.
	  break;

	case AUTO_SHUTDOWN:
	  {
	    long t = OSapi_atoi(argv[i]);
	    if (t >= 0) {
	      auto_shutdown = t;
	    }
	  }
	default:
	  option = last_option;
	  break;
	}
      }
    }
    if (command_line_error) {
	msg("Command line error", error_sev) << eom;
	driver_exit (1);
    }

    // initialize project stuff

    service_init(argc, argv);

    builtin_mmgr = new MemoryManager(NULL);

    if (!get_main_option("-core")) pmod_set_signals();

    load_project_rtl( num_of_pdf_files, pdf_file);  
    if (is_model_build()) {
        re_generate_pdf_tree_for_refresh_top_proj();
        if (analysis_flag) {
            do_model_build_analysis(num_of_projects, projects,
                                    update_xref, visible_projects);
	} else if (update_flag) {
            do_model_build_update(num_of_projects, projects,
                                  multiple_proj_flag, remove_modules);
	} else if (multiple_proj_flag) {
            do_model_build_multiple(num_of_projects, projects,
                                    multiple_proj_flag, update_xref,
                                    visible_projects);
        } else {
            return do_model_build_default(xref_projname, update_xref,
                                          visible_projects, num_of_pmod_files,
                                          pmod_file, pmod_command );
        }
    } else { // ! is_model_build()
        load_and_refresh_projects(xref_projname);

	if (model_service_name) {
	    genString tmp;
            if (!internal_service) {
                //If service type is not internal then add "Dislite:" prefix 
                //to the service name.
	        tmp.printf("Dislite:%s", model_service_name);
            }
            else tmp = model_service_name;
	    InitializeSocketServer(tmp);
	}

	service_loop(auto_shutdown);
    }
    driver_exit (0);
    return 0;
}

#ifdef _WIN32

extern "C"  {

typedef struct LibraryList {
    HINSTANCE handle;
    struct LibraryList *nextPtr;
} LibraryList;

static LibraryList *libraryList = NULL;	/* List of currently loaded DLL's.  */

static void
UnloadLibraries()
{
    LibraryList *ptr;

    while (libraryList != NULL) {
	FreeLibrary(libraryList->handle);
	ptr = libraryList->nextPtr;
	ckfree((char*)libraryList);
	libraryList = ptr;
    }
}

extern "C" void TclWinConvertError(DWORD errCode);

HINSTANCE
TclWinLoadLibrary(char *name)
{
    HINSTANCE handle;
    LibraryList *ptr;

    handle = LoadLibrary(name);
    if (handle != NULL) {
	ptr = (LibraryList*) ckalloc(sizeof(LibraryList));
	ptr->handle = handle;
	ptr->nextPtr = libraryList;
	libraryList = ptr;
    } else {
	TclWinConvertError(GetLastError());
    }
    return handle;
}

int 
TclWinSynchSpawn(void *args, int type, void **trans, Tcl_Pid *pidPtr)
{
    static UT32PROC UTProc = NULL;
    static int utErrorCode;

    if (UTUnRegister == NULL) {
	/*
	 * Load the Universal Thunking routines from kernel32.dll.
	 */

	HINSTANCE hKernel;
	PUTREGISTER UTRegister;
	char buffer[] = "TCL16xx.DLL";

	hKernel = TclWinLoadLibrary("Kernel32.Dll");
	if (hKernel == NULL) {
	    return 0;
	}

	UTRegister = (PUTREGISTER) GetProcAddress(hKernel, "UTRegister");
	UTUnRegister = (PUTUNREGISTER) GetProcAddress(hKernel, "UTUnRegister");
	if (!UTRegister || !UTUnRegister) {
	    UnloadLibraries();
	    return 0;
	}

	/*
	 * Construct the complete name of tcl16xx.dll.
	 */

	buffer[5] = '0' + TCL_MAJOR_VERSION;
	buffer[6] = '0' + TCL_MINOR_VERSION;

	/*
	 * Register the Tcl thunk.
	 */

	if (UTRegister(TclWinGetTclInstance(), buffer, NULL, "UTProc", &UTProc, NULL,
		NULL) == FALSE) {
	    utErrorCode = GetLastError();
	}
    }

    if (UTProc == NULL) {
	/*
	 * The 16-bit thunking DLL wasn't found.  Return error code that
	 * indicates this problem.
	 */

	SetLastError(utErrorCode);
	return 0;
    }

    UTProc(args, type, trans);
    *pidPtr = 0;
    return 1;
}

}

#endif

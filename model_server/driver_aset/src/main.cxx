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
#include <wchar.h>
#include <options.h>

#include <xxinterface.h>
#ifdef NEW_UI
#include <vport.h>
#include vstartupHEADER
#ifndef _mytimer_h
#include <mytimer.h>
#endif
#endif

#ifndef ISO_CPP_HEADERS
#include <locale.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <locale>
using namespace std;
#include <cassert>
#endif /* ISO_CPP_HEADERS */
 
#ifndef ISO_CPP_HEADERS
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#endif /* ISO_CPP_HEADERS */
#ifdef NEW_UI
#include <galaxy_undefs.h>
#endif
#include <cLibraryFunctions.h>
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include <gtRTL_cfg.h>
#ifndef _WIN32
#include <sys/param.h>
#endif 
#ifndef NEW_UI
#include <gtBase.h>
#include <gtRTL.h>
#include <gtRTL_cfg.h>
#include <gtPushButton.h>
#include <gtShell.h>
#include <gtNoParent.h>
#include <gtDisplay.h>
#endif
#include <time_log.h>
#include <genError.h>
#include <objOper.h>
#include <ldrNode.h>
#include <viewNode.h>
#ifndef NEW_UI
#include <viewGraHeader.h>
#include <graWorld.h>
#include <graWindow.h>
#endif
#include <driver.h>
#include <steView.h>
#include "setprocess.h"
#include "SymManager.h"
#include <MemSeg.h>
#include <clipboard.h>
#include <scrapbook.h>
#include <oodt_relation.h>
#include <RTL.h>
#include <ddict.h>
#include <messages.h>
#ifndef NEW_UI
#include <viewerShell.h>
#endif
#include "evalMetaChars.h"
#include <ParaCancel.h>
#include <objRawApp.h> /* Automatically substituted */
#include <customize.h>
#include <proj.h>
#include <path.h>
#include <projectBrowser.h>
#include <driver_mode.h>
#include <autosubsys.h>
#include <autolog.h>

#ifndef _path1_h
#include <path1.h>
#endif

#ifndef _patherr_h
#include <patherr.h>
#endif

#ifndef NEW_UI
#include <HelpUI.h>
#include <top_widgets.h>
#endif

#include <cmd.h>

#include <feedback.h>
#include <tpopen.h>
#include <timer.h>
#include <globals.h>

#ifndef _genTmpfile_h
#include <genTmpfile.h>
#endif

#include <attribute.h>

#ifdef NEW_UI
#include <vport.h>
#include veventHEADER
#include vnoticeHEADER
#undef printf 
#undef sprintf
#include <Application.h>
#include "DIS_rtlFilter/interface.h"
#include "DIS_ui/interface.h"
#undef printf
#include <gstate.h>
#include "Interpreter.h"
#endif // NEW_UI

#ifdef _WIN32
#undef VOID
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <string.h>
void __cdecl _vsysBeforeMSWMain(void *,void *,char *,int); //GALAXY initialization
extern "C" void TkWinXInit(HINSTANCE);
#endif /*_WIN32*/

#include <mpb.h>
#include <disbuild.h>
#include <disbuild_analysis.h>
#include <disbuild_update.h>
#include <msg.h>
#include "Interpreter.h"

void OSapi_cout_and_cerr();

extern FILE *tee_file; 

char *server_name                       = NULL;
static bool is_model_build_flag         = false;
static bool is_model_server_flag        = false;
static bool is_gui_flag                 = false;

bool is_aset()
{
    return true;
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
    return is_gui_flag;
}

static bool pset_server_service_flag = false;
static bool is_pset_server_in_server_mode()
{
    return pset_server_service_flag;
}

int CannotSendFlag = 0;


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
void TclWinInit(HINSTANCE);
}
#endif

#ifdef NEW_UI
class Tcl_Interp;
Tcl_Interp * make_interpreter();
Tcl_Interp* g_global_interp; // Global interperter used for top-level applicaton.
int interp_trace = 0; // Flag for watching tcl evaluations
int interp_errTrace = 0; // Flag for watching tcl evaluations
StateGroup* DIS_stateGroup;
#endif

char *cli_script = NULL;
char *branch = NULL;
//const char* main_argv_0;

int proj_count;
int source_file_count;
int total_file_count;
int pset_exists_count; 
int pset_up_to_date_count;
int entry_in_pmod_count;
int entry_in_pmod_uptodate_count ;

extern "C" char const build_date[];
extern "C" char const build_version[];

void dd_get_pmod_subprojects(objArr&);
extern void dd_build_xref (int num_load, char **load_files,
                           fileLanguage *, int num_remove, char **remove_files,
                           char *home_proj_name, int save_files,
                           int save_includes, int update_xref,
                           char** visible_projects , char*);

extern "C" void dd_print_xref (char *xref_proj_name);
extern void dd_tidy_xref (char *xref_proj_name);
extern void dd_merge_xref (int num_of_merge_xrefs, char **merge_xrefs,
                            char *dst_xref);


int dd_set_hook();

#ifndef NEW_UI
int set_font_path();
int do_prelim_check(int& argc, char **argv);
int do_prefs_check(void);
void append_options(int& argc, char **argv);
#else
#include "Application.h"
#endif
#include "Args.h"
extern Args *DIS_args;
int service_init(int argc, char**argv);

#include <proj_save.h>

#ifdef _WIN32
struct thread_params
{
  int argc;
  char **argv;
  char **envp;
  DWORD nThreadId;
  thread_params(){ argc = 0; argv = NULL; envp = NULL; nThreadId = 0; } 
};
thread_params BatchParams;
unsigned long gnMainThreadId = 0;
void batch_thread_proc( void * );

extern void InitializeConfigCriticalSection();
extern int  shell_configurator_init();
extern void shell_configurator_destroy();

#endif

//// DECLARATIONS THAT SHOULD BE IN HEADER FILES
void load_pdfs_complete();
void re_generate_pdf_tree_for_refresh_top_proj();
//void init_new_handler(unsigned, unsigned);
extern void pset_dispatch_own_callback (XEvent *);
void load_project_rtl (int num_pdf_files, genString* pdf_file);
extern void delete_scratch_apps();

//// DECLARATIONS OF EXTERNAL VARIABLES THAT SHOULD BE IN HEADER FILES
extern scrapbookPtr            scrapbook_instance;

//Global flag indicating that UI client is running
//It is used by MyDialog class to implement different behaviour
//for UI client and graphical views (now part of pset server) 
//when user selects 'Close' in a system menu
int fIsUIClient = 0;

#ifdef NEW_UI
    Application* DISui_app; 
    Application* DISmain_app; // My application service context  
    Application* DIScockpit_app; 
    Application* DISgpi_app; 
    Application* DISdormant_app; 
    Application* DISgroup_app; 
    Application* DISrtlFilter_app;
    Application* DISview_app;

#define DIS_WRAP(hook) \
vscrap * paste(hook,_wrap)(vservicePrimitive *pr,vsession *ses,vscrap *scr) \
{\
    vscrap * retval = 0;\
    start_transaction() {\
        retval = hook(pr,ses,scr);\
    } end_transaction();\
    return retval;\
}

    extern Server DISmainServer;
    DIS_WRAP(DISmainServer);
    void DISmainServerInit();

    extern Server DIScockpitServer;
    DIS_WRAP(DIScockpitServer);

    extern Server DISdormantServer;
    DIS_WRAP(DISdormantServer);
    void DISdormantServerInit();

    extern Server DISrtlFilterServer;
    DIS_WRAP(DISrtlFilterServer);

    extern Server DISviewServer;
    DIS_WRAP(DISviewServer);
    void DISviewServerInit();
#else
   int gala_current_screen;
   int gala_current_window;
   int gala_current_screen_dialog ;
   int gala_current_screen_layer ;

   extern "C" void send_to_cockpit_unco (){}
   extern "C" void send_to_cockpit_co (){}
   extern "C" void send_to_cockpit_delete (){}
   extern "C" void connect_to_cockpit (){}
   extern "C" void init_cockpit (){}
   extern "C" void send_to_cockpit_ci (){}
#endif // NEW_UI

static int included_flag      = 0;
static int analysis_flag      = 0;
static int multiple_proj_flag = 0;

extern ok_to_delete_old_pmod;

extern void simplify_batch(char const *proj_name);
extern void output_dependency_modules(char const *);
extern "C" void cmd_journal_log(char *f);
extern "C" void cmd_journal_rerecord(char *f);

extern void share_re_load_pdf(char *pdf);
extern void prj_load_pdfs_start ();
extern void prj_load_pdfs_end ();

extern "C"
{
    int home_proj_selector();
    void buildxref_set_signals();
    char *lm_errstring(int);
    //void _exit (int status);

#ifndef NEW_UI
    void  c_main( int *argc, char **argv );
    int  popup_very_first_window();
    int  popup_first_windows();
#endif

    void  steInitialize();
    void  ste_epoch( int, int, int, int, int );
    void  aset_set_signals();
    void  driver_exit(int);
    int   driver_exiting();
    void Remove_Old_Modules();


    void gen_stack_prt(int level);
    bool do_pmod_command(int num_of_pmod_files, genString* pmod_file, const char* pmod_command);
}

#ifndef NEW_UI
int system_message_logger(int);
int system_message_logger_init();
#endif

#ifndef _WIN32
extern int __is_master_pset;
#else
extern "C" int __is_master_pset;
#endif
static int new_argc;
static char** new_argv;

typedef void (*hook)( XEvent *foo); // - for GNU
void SetClientHook(hook);

void RTL_init();

void init_rel_ptrs();

int main_main(int argc, char *argv[], char *envp[]);
int xref_main(int argc, char *argv[], char *envp[]);
#ifdef _WIN32
unsigned long __stdcall output_thread_control(void *);
int redirect_output();
int nPipeReadHandle;
#endif

//// STATIC DECLARATIONS
static void load_pdfs();

static void load_pdfs_xref_main ()
{
    load_pdfs();
    re_generate_pdf_tree_for_refresh_top_proj();
}

int get_option_genStrings(const char *opt, genString *opt_arr, int max_opt)
//  Looks for one or more occurrences of the specified opt switch 
//  (starts with -) and reads all the following values that do not 
//  have a leading - character into the opt_arr.
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

#ifndef _WIN32
extern SymbolManager*          symmgr_instance;
#endif

//------------------------------------------
// driver_client_hook
//
// This function gets called whenever there is a client XEvent.
// This is used for getting communique from epoch
//------------------------------------------


void driver_client_hook( XEvent *event )
{
    if ( !event->xclient.window ) {
        if (event->xclient.data.b[0] == 1) {// is used for 19 bytes command
            msg ("invalid call to ste_send_shell_command", catastrophe_sev) << eom;
            exit(1);
            //ste_send_shell_command(0, &event->xclient.data.b[1]);
        }
        else
            switch (event->xclient.format) 
            {

              case 16 :
              case 8  :          
              case 32 :
                default :
                {
                int a1 = (int) event->xclient.data.l[0];
                int a2 = (int) event->xclient.data.l[1];
                int a3 = (int) event->xclient.data.l[2];
                int a4 = (int) event->xclient.data.l[3];
                int a5 = (int) event->xclient.data.l[4];
                ste_epoch(a1, a2, a3, a4, a5);
                }
                break;
            }
    } else
        pset_dispatch_own_callback (event);
}

extern "C" void driver_client_hook_C (XEvent *event) {
    driver_client_hook (event);
}

extern "C" void main_aset_trap_handler()
{
    static int count = 0;

    Initialize(main_aset_trap_handler);
    
    if (driver_exiting())
        driver_exit(1);
    
    //  Make sure exit code indicates crash
    globals::error_status(CRASH_ERROR);

    // disable copying of potentially corrupted temporary pmod 
    // and notify user
    ok_to_delete_old_pmod = 0;
    msg("ERROR: All changes to model made in this session are lost. Please exit and redo.") << eom;
    
    if ( count++ > 100 )
        driver_exit(1);

    aset_set_signals();

    Error(ERR_INPUT);
    driver_exit(1);
}


extern "C"  void xref_aset_trap_handler()
{
    Initialize(xref_aset_trap_handler);
    
    if (driver_exiting())
        driver_exit(1);
    
    //  Make sure exit code indicates crash
    globals::error_status(CRASH_ERROR);

    // for model build, disable copying of potentially corrupted 
    // temporary pmod 
    if (is_model_build())
        ok_to_delete_old_pmod = 0;

    msg("DIScover error: stack trace follows.", catastrophe_sev) << eom;
    gen_stack_prt (1);

    driver_exit(1);
}
//------------------------------------------
// aset_startup
//
// Run startup commands.
//------------------------------------------

static void aset_timer_operation (const char *operand)
{
    static bool timer_started = false;
    static bool timer_initialized = false;

    if (!strcmp (operand, "start")) {
        if (!timer_started) {
            if (!timer_initialized) aset_timer_operation ("init");
            timer_started = true;
            call_trace::start_logging();
        }
    } else if (!strcmp (operand, "stop")) {
        if (timer_started) {
            timer_started = false;
            call_trace::stop_logging();
        }
    } else if (!strcmp (operand, "init")) {
        if (!timer_initialized) {
            time_log::init();
            timer_initialized = true;
        }
    } else if (!strcmp (operand, "dump")){
        if (timer_initialized) dump_time_log_statistics();
    }
}


static void aset_startup()
{
    Initialize(aset_startup);

    char* option_value;

    static const char * opt_arr[] =
    {
        "-e", "-ee", "-T", "-pdc", "-pdexec",
        "-D", "-ui", "-rr"
    };

    static const char * msg_arr[] =
    {
        "Executing", "Executing", "Timing", "Debugger", "Debugger playback",
        "Dumping execution times",
        "Interacting", "Re-Recording"
    };

    static opt_len = sizeof(opt_arr) / sizeof(char*);

    char * dbg_choice = (char *)0; // default debugger selection
    char * dbg_exec_f = (char *)0; // file with debugger commands 
    int ind, cur_ind;

    bool die = false;
    if(get_main_option("-eee"))
           die = true;

    genError* err;
    genTry
    {
        char* fname = "aset-startup.c";
        if (OS_dependent::access(fname, R_OK) == 0)
            cmd_journal_execute(fname);

        cur_ind = 0;
        viewerShell* vs = NULL;

        if (option_value = get_main_option ("-pdc"))
            dbg_choice = option_value;
        if (option_value = get_main_option ("-pdexec"))
            dbg_exec_f = option_value;

        while(option_value = 
              get_option_from_list(opt_len, opt_arr, &ind, &cur_ind)){
            const char * msg1 = msg_arr[ind];
            msg("$1 $2 ...\n") << msg1 << eoarg << option_value << eom;

            switch(ind){
              case 0: // -e
#ifndef NEW_UI
                system_message_logger(1); // Turn on message logger
#endif
                cmd_journal_execute(option_value);
                break;

              case 1:  // -ee
#ifndef NEW_UI
                system_message_logger(1); // Turn on message logger
#endif
                cmd_journal_execute(option_value);
                die = true;
                break;

              case 2:  // -T
                aset_timer_operation (option_value);
                break;
              case 3:           // -pdc
                break;
              case 4:           // -pdexec
                break; 

              case 5:  // -D
                dump_time_log_statistics(option_value);
                break;

              case 6: // -ui
                die = 1;
                driver_instance->run();
                break;
                
              case 7: // -rr
                die = 1;
                cmd_journal_rerecord(option_value);
                break;
            }
        }
    } genCatch(err) {
        die = true;
    } genEndtry;
    
    if(die) {
       delete_scratch_apps();
       driver_exit(0);
    }
}



//-----------------------------------------
//main
//
// Command_line Entry point
//----------------------------------------


static void second_pr(int argc, char **argv)
// debug routine to print the arguments for put process
{
    Initialize(second_pr);

    static genTmpfile tnm("secnd");
    FILE *out = tnm.open_buffered();

    for (int i = 0; i < argc; i++)
        OSapi_fprintf(out,"%d (%s)\n",i,argv[i]);

    tnm.close();
}

static void print_build_date(void){
    Initialize(print_build_date);

    static int has_been_called = 0;

    if (! has_been_called)
    {
        // why can't we make the build_date string more sophisticated?
        char * build_date_string = OSapi_strdup(build_date);
        char * month_day = build_date_string + 4;
        month_day[6] = 0;
        char * year = build_date_string + 24;
        year[4] = 0;
        char * time = build_date_string + 11;
        time[5] = 0;

        msg("DIScover Version: $1", normal_sev) << build_version << eom;
        msg("Built on: $1 $2 $3", normal_sev)
                << month_day << eoarg
                << year << eoarg
                << time << eom;
        has_been_called = 1;
    }
}

class vsession;
extern "C" void init_cockpit();
static bool serverOperation = false;

extern "C" bool isServer (void)
{
    return serverOperation;
}
#ifdef NEW_UI
// this function is intended to be the prototype for "unrecoverable"
// errors that happen when the pset_server is in "-server" mode
// and the communications channels haven't been set up yet.
static void ShowLMErrorMessage()
{
        vstr *pMsgText=vstrCreateSized(128);
        pMsgText=vstrAppendScribed(pMsgText, vcharScribeLiteral(
        "Discover was unable to verify your license.\nPlease check the license server,\nthe license file, and the value of LM_LICENSE_FILE,\nor call customer support"));
        vstr * pMsgTitle=vstrCreateSized(128);
        pMsgTitle=vstrAppendScribed(pMsgTitle, vcharScribeLiteral("Discover License Error"));
        vnotice * pMessage= new vnotice();
        pMessage->SetText(pMsgText);
        pMessage->SetModal(TRUE);
        pMessage->SetTitle(pMsgTitle);
        pMessage->Open();
        pMessage->Beep();
        pMessage->Process();
        //called to make purify et. al happier.
        vstrDestroy(pMsgText);
        vstrDestroy(pMsgTitle);
        pMsgText=NULL;
        pMsgTitle=NULL;
        delete pMessage;
        pMessage=NULL;
}
#endif

#ifdef _WIN32
char* realOSPATH_hashed(char*);
extern void* pfun_realOSPATH_hashed;
#endif

//----------------------------------------------------------------------------
extern "C" void msg_status(const char* fmt...);
extern "C" void msg_warn(const char* fmt...);
extern "C" void msg_error(const char* fmt...);

void status_message(msg_sev, char const *msg)
{
  msg_status(msg);
}

void popup_message_box(msg_sev sev, char const *msg)
{
  switch(sev) {
  case warning_sev:
    msg_warn(msg);
    break;
  default:
    msg_error(msg);
    break;
  }
}


// main
//------------------------------------------
// synopsis:
// The main routine for the aset executable (run from the discover script)
//
// description:
//   Checks for the special switches -verify_pdf, -batch, and
//   calls the appropriate functions.  If none of these are set, it 
//   calls main_main(), which is the default interactive discover.
//   main_main initializes UIMX and pops up a project window, then 
//   creates a driver and calls it.
//------------------------------------------
main(int argc, char *argv[], char *envp[])
{
//boris 02101998. Please keep it at the top of the function 'main'. Thank you.
    static int halt_flag;
    char* tmp;
    if (0 != (tmp = getenv("PSET_HALT"))) {
        int    sleeper = 0;
        double floater = 0.0;

        halt_flag = atoi(tmp);
        msg("\nPlease attach debugger to process $1 and set halt_flag to 0\n") << OSapi_getpid() << eom;
        msg("   You have $1 seconds before execution will resume...") << halt_flag << eom;
        while (halt_flag-- > 0) {
            for (sleeper=0; sleeper<1000000; sleeper++) {
                floater = 4.0 * 3.0;
            }
        }
        msg("   Resuming\n") << eom;
    }

#if defined(_WIN32) && defined(_DEBUG)

  char* pszDbgPoint = (char*)GetDiscoverRegistryDebug("driver_main");
  if ( pszDbgPoint && *pszDbgPoint && strcmp(pszDbgPoint,"0") )
  {
    if ( strcmp(pszDbgPoint, "-1") == 0 )
      _asm int 3;
    else
    {
           int  bAnswer;
     char szText[1024];
     sprintf( szText, "Discover debug point: driver_main. Process ID=%d\nWould you like to invoke hard break point?", OSapi_getpid() );
           if ( argc > 7 && argv[7] )
             bAnswer = MessageBox( 0, szText, argv[7], MB_YESNO | MB_TOPMOST );
           else
             bAnswer = MessageBox( 0, szText, "...", MB_YESNO | MB_TOPMOST );
       if ( bAnswer == IDYES )
          _asm int 3;
      }
    }
#endif

    set_main_options (argc, argv);


    //Initialize log presentation service name and message group id.
    //Do it as soon as possible so that generated messages can be routed
    //to the gui(if present).
    char *lp_service_name = get_main_option("-lpservice");
    if (lp_service_name) {
       char *msg_grp_id = get_main_option("-message_group");
       if (msg_grp_id)
         init_lp_service(lp_service_name, msg_grp_id);
    }

    pset_server_service_flag = (   check_main_option("-service")
                                || check_main_option("-server"));
    is_gui_flag = (   !check_main_option("-batch")
                   && !is_pset_server_in_server_mode());

    //if it is interactive session enable popup messages
    if (is_gui()) {
      register_ui_callbacks(popup_message_box, status_message);
    }

    //start license manager
    if ((is_gui() || is_pset_server_in_server_mode()) && _lc()) {
                _le();
#ifdef NEW_UI
                bool servermode= check_main_option("-server");
                if (servermode)
                {
                        vstartup(argc, argv);
                        char *locale_success = setlocale(LC_ALL, "C");
                        assert(locale_success);

                        ShowLMErrorMessage();
                }
#endif
      driver_exit (3);
    }
#ifndef NEW_UI
    bool isinterflag = (is_gui() && check_main_option("-envcheck")) ? 1 : 0;
    if (isinterflag && !do_prelim_check(argc, argv))
      driver_exit (4);
#endif

#ifdef _WIN32
  //if we link with /subsystem:console switch, we have to call this function
  //If /subsystem:windows is used, GALAXY calls it from WinMain()
  _vsysBeforeMSWMain( GetModuleHandle( NULL ), NULL, NULL, 0 );
  // Allocate console and set up stdout and stderr
    //AllocConsole();
  TclWinInit(GetModuleHandle(NULL));
  TkWinXInit(GetModuleHandle(NULL));
#endif
    OSapi_cout_and_cerr();
#ifdef _WIN32
    create_tmp_dir();
#endif
    dd_set_hook();

#ifdef _WIN32
    InitializeConfigCriticalSection();
    pfun_realOSPATH_hashed = (void*)realOSPATH_hashed;
#endif

    Initialize (main);

    // Galaxy needs to scan the argument list.
#ifdef NEW_UI
    vstartup (argc, argv);
#endif
    char *locale_success = setlocale(LC_ALL, "C");
    assert(locale_success);

#ifdef _WIN32
    Application::ShowTaskbarWindow(0);//hides taskbar window on NT, does nothing on UNIX
#endif

    // Remember the original argv[0] so that class customize can examine it.

    //main_argv_0 = OSapi_strdup(argv[0]);
    DIS_args = new Args(argc, argv);
    customize::instance = new customize;
    customize::set_write_flag(1);
    
    if (check_main_option("--version"))
    {
        print_build_date();
        driver_exit(0);
    }

#ifndef NEW_UI
    if (isinterflag) {
      if (!do_prefs_check()){
        driver_exit(6);
      }
      else 
       append_options(argc, argv);
    }
#ifndef _WIN32
    if (is_gui() && set_font_path() < 0)
        driver_exit (5);
#endif
#endif

    // Copy arguments because X destroys some of them
    new_argv = (char**)psetmalloc((argc + 1) * sizeof(char*));
    new_argc = argc;
    for (int i = 0; i < argc; i++)
        new_argv[i] = OSapi_strdup(argv[i]);

    char * tee_fname = get_main_option("-tee");
    if (tee_fname) {
        tee_file = fopen(tee_fname,"a");
    }

    if (is_gui())
      aset_log = new autolog;

#ifndef NEW_UI
    def_table = new gtRTL_cfg;
    // Init message logging.
    system_message_logger_init();
#endif

    DBG
    {
        second_pr(argc, argv);
    }

    builtin_mmgr = 0;

    init_rel_ptrs();

    //init_new_handler(4000000, 50000);   // Set up emergency memory handler.

    print_build_date();

    if (argc>1 && !is_gui())
    {
        if (!is_pset_server_in_server_mode()) { // -batch
#ifdef _WIN32
          BatchParams.argc = argc;
          BatchParams.argv = argv;
          BatchParams.envp = envp;
          BatchParams.nThreadId = GetCurrentThreadId();
          gnMainThreadId = BatchParams.nThreadId;
          _beginthread (batch_thread_proc, 0, NULL );
          MSG msg;
          while (GetMessage(&msg, NULL, 0, 0));
          driver_exit ((int)msg.lParam);
#else
            xref_main(argc,argv,envp);
            driver_exit (0);
#endif
        }
    }

#ifndef NEW_UI
// -msg_logger
    if(get_main_option("-msg_logger"))
        system_message_logger(1);

// -unit_scope
    genString unit_name = get_main_option("-unit_scope");
    if (unit_name.length())
    {
        int count = 0;
        int i;
        for (i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-unit_scope"))
            {
                for (int j = i+1; j < argc; j++)
                {
                    if (*argv[j] == '-')
                        break;
                    count++;
                }
                break;
            }
        }

        const char **log_name = (const char **)psetmalloc(sizeof(char *) * count);
        for (int j = 0; j < count; j++)
            log_name[j] = OSapi_strdup(argv[j+i+1]);

        create_one_scope_name(-2, "from_switch", count, log_name);
        
        for(i = 0; i < count; i++)
            free((char*)log_name[i]);
        psetfree(log_name);
    }

#endif
    
    genError* err;
    genTry {
      main_main(argc,argv,envp);
    } genCatch(err) {
      driver_exit(1);
    } genEndtry;
    // we will never come here

    return 0;
}

#ifdef _WIN32
unsigned long __stdcall output_thread_control(void *fd)
{
        char buf[512];
        int  read_fd = *((int *)fd);

        while(1){
                int nBytesRead;
                if((nBytesRead = _read(read_fd, buf, 512)) > 0)
                {
                   for(int i = 0; i < nBytesRead; i++)
                       putchar(buf[i]);
                }
        }
        return 0;
}


int redirect_output(void)
{
        int handles[2];
        unsigned long thread_id;
        if(_pipe(handles, 1024, _O_BINARY) == -1)
                return -1;
        SECURITY_ATTRIBUTES sec_attr;
        ZeroMemory( (void *)&sec_attr, sizeof( SECURITY_ATTRIBUTES ) );
        sec_attr.nLength              = sizeof(sec_attr);
        sec_attr.lpSecurityDescriptor = NULL;
        sec_attr.bInheritHandle       = TRUE;
        nPipeReadHandle = handles[0];
        CreateThread(&sec_attr, 0, output_thread_control, (void *)&nPipeReadHandle, 0, &thread_id);
        return handles[1];
}

#endif 


//------------------------------------------
// main_main
//
// Aset entry point
//------------------------------------------

//#define TXT(a) a 

#define PENDING driver_instance->process_pending_events

static void load_pdfs()
// load the pdf files used by main_main() and pdf_debugger
{
    Initialize(load_pdfs);

    genString pdf_file[1024];
    int num_pdf_files = get_option_genStrings ("-P", pdf_file, 1024);
    num_pdf_files    += get_option_genStrings ("-pdf", 
                                               pdf_file + num_pdf_files, 
                                               1024 - num_pdf_files);
    load_project_rtl(num_pdf_files, pdf_file);
}


#ifdef NEW_UI
//extern Interpreter *interpreter_instance;
static void trigger()
// This is called by an event that was posted into the future inside of main_main.  It
// is used to tell gala that the pset_server is up and running.  It has to wait until
// the system has processed for a while to do it, which is why it cannot just call this
// code directly.
{
  cout << "You are trying to connect back to client" << endl;
  char* clientName = get_main_option ("-client");
  cout << clientName << endl;

  Application* app = new Application();

  if (!(app->connect (0, clientName))) 
  {
        // Initializing interpreter and sourcing discover.dis.
        // We are unable to source it properly if no client connection is established.
    load_pdfs_complete();

    DISui_app = app;
    cout << "You have made the client connection" << endl;
    char buffer[1024];
    sprintf (buffer, "DISmain_started"); 
    vstr *result = rcall_dis_DISui_eval(app, (vchar*)&buffer[0]);
    if( result )
      vstrDestroy( result );
  } 
  else  {
    cout << "Connection failed!" << endl;
  }
}
#endif

void OpenDomainShell();
void InitializeSocketServer(const char* service);

#ifdef WIN32

static bool runningGalaUI(void)
{
    bool retval = false;

    char *service = get_main_option("-service");
    char *group   = get_main_option("-group");
    char *client  = get_main_option("-client");
    if ( service && group && client )
       retval = true;
    
    return retval;
}

#endif // WIN32


int main_main(int argc, char *argv[], char ** /* envp */ )
{
    Initialize (main_main);

    int i;
    genError* err;

    // save process id for programmatic debugging from master-aset

    char* fn = get_main_option ("-slave_id");
    if(fn) {
        FILE* file = fopen(fn, "w");
        if(file == NULL) {
            msg("STDERR: Cannot open '$1'.") << fn << eom;
            exit(1);
        } else {
            OSapi_fprintf(file, "%d\n", OSapi_getpid());
            fclose(file);
        }
    }

    // get shared memory address if specified on command line
    char* exec = get_main_option ("-name");
    if (!exec || strcmp (exec, "aset_slave"))
        __is_master_pset = 1; /* case master aset */

#ifdef NEW_UI
    char* clientName = get_main_option("-client");
    if (clientName) {
        cout << "You have found a client." << endl;
        callbackEvent* cbfn = new callbackEvent (trigger);
        cbfn->Post();
    }
#endif

#ifndef NEW_UI
    // Initialize the help system.
    // This should happen before any gt objects are created.
    gtBase::init_help_button(HelpUI_LongHelp);

    // Init hooks for command journal and auto test
    cmd_init_hooks();

    c_main (&argc, argv);       // Initialize UIM/X (and Xt)
#endif

    builtin_mmgr = new MemoryManager(NULL);

    // Argument may get eaten up by InterViews, copy them here
    if (argc != new_argc) {
      argc = new_argc;
        for (i = 0; i < new_argc; i++)
            argv[i] = OSapi_strdup(new_argv[i]);
    }
	
	//----------------------
	// issue # 44046 "pset_server hangs when no correct service name specified"
#ifdef NEW_UI
#ifdef _WIN32
	if(!(check_main_option("-server") || check_main_option("-batch"))) {
	      msg("Either \'-batch\' or \'-server\' option should be specified.\n") << eom;
    	  driver_exit (0);
    } else {
    	if(check_main_option("-server") && !check_main_option("-server_name")) {
	      msg("Server name should be specified.\n") << eom;
    	  driver_exit (0);
    	}
	}
#endif
#endif
	//----------------------

    if (_lo(LIC_DISCOVER)) {
#ifdef NEW_UI
        if (check_main_option("-server")&&!check_main_option("-batch"))
                {
                        ShowLMErrorMessage();
                        driver_exit((int) err);
                }
#endif
                SetClientHook (driver_client_hook);
        RTL_init();
        msg("DIScover: $1\n") << lm_errstring(int(err)) << eom;
        driver_exit (3);
    }
    genTry {
        driver_instance = new driver (argc, argv); PENDING();
        SetClientHook (driver_client_hook); PENDING();
        projNode::proj_init();          PENDING();
        RTL_init ();                    PENDING();
    } genCatch (err) {
        driver_exit((int)err);
    } genEndtry;
                                        PENDING();
    // Create system RTLs
    oodt_relation::create_defaults();   PENDING();

    server_name = get_main_option("-server_name");

    // Check for pmod_services required.
    char * pmod_server_name=get_main_option("-pmod_server");
    if(pmod_server_name) {
      service_init(argc, argv);
    }
    // create new scrapbook instance
    scrapbook_instance = new scrapbook(
        "DIScover Clipboard", unit_cut_paste); PENDING();

    if (!check_main_option("-core")) aset_set_signals(); PENDING();
    if(!get_main_option("-e") && !get_main_option("-ee")){
        OSapi_putenv("PSET_PARAPARTION=1");
    }


    //instanstiate progress bar
    mpb_init();
    mpb_info("Loading DIScover...");

    load_pdfs();
    
    steInitialize ();                   PENDING();

#ifdef NEW_UI
// Initialize application contexts
    const char* group = Application::getGroupName();
    if (check_main_option ("-server")) {
        serverOperation = true;
        
        if (!home_proj_selector()) {
            vnotice *notice = new vnotice;
            notice->SetText((const vchar *)"Home project is not specified.");
            notice->SetModal(vTRUE);
            notice->Process();
            driver_exit(0);
            return 0;
        }

        DISmain_app = new Application;
        if (DISmain_app->registerService (group, "DISmain", DISmainServer_wrap)) {
            OSapi_fprintf (stderr, "Cannot register service %s:DISmain; exiting.\n", group);
        }  else
            OSapi_fprintf (stderr, "Service  %s:DISmain available.\n", group);
        DISmainServerInit();
    #ifndef _WIN32
           ParaCancel *cancel = new ParaCancel(group);
    #endif
        
        
        DISdormant_app = new Application;
        if (DISdormant_app->registerService (group, "DISdormant", DISdormantServer_wrap)) {
            OSapi_fprintf (stderr, "Cannot register service %s:DISdormant; exiting.\n", group);
        }  else
            OSapi_fprintf (stderr, "Service  %s:DISdormant available.\n", group);
        DISdormantServerInit();
        

        DISrtlFilter_app = new Application;
        if (DISrtlFilter_app->registerService (group, "DISrtlFilter", DISrtlFilterServer_wrap)) {
            OSapi_fprintf (stderr, "Cannot register service %s:DISrtlFilter; exiting.\n", group);
        }  else
            OSapi_fprintf (stderr, "Service  %s:DISrtlFilter available.\n", group);
        DISrtlFilterServerInit();

        DIScockpit_app = new Application;
        if (DIScockpit_app->registerService (group, "DIScockpit", DIScockpitServer_wrap)) {
            OSapi_fprintf (stderr, "Cannot register service %s:DIScockpit; exiting.\n", group);
        }  else
            OSapi_fprintf (stderr, "Service  %s:DIScockpit available.\n", group);

        DISview_app = new Application;
        if (DISview_app->registerService (group, "DISview", DISviewServer_wrap)) {
            OSapi_fprintf (stderr, "Cannot register service %s:DISview; exiting.\n", group);
        }  else
            OSapi_fprintf (stderr, "Service  %s:DISview available.\n", group);
        DISviewServerInit();
    }

    InitializeSocketServer(server_name);
    g_global_interp = make_interpreter();
    DIS_stateGroup = new StateGroup(g_global_interp);

    DIS_stateGroup->defineState ((vchar*)"InProgress");
    DIS_stateGroup->defineState ((vchar*)"ValidSelection");

#endif // NEW_UI

    init_cockpit();

#ifndef NEW_UI
    if (!popup_very_first_window()) {
           driver_exit(-1);
           return 0;
    }
    load_pdfs_complete();
    if (!popup_first_windows()) {
          driver_exit(-1);
          return 0;
    }
#endif

    projList::full_list->copy(*projList::search_list);
    if ( projNode::get_home_proj() == 0 )
    {
      msg("") << eom;
      msg("Home project is not specified.\n") << eom;
      driver_exit (0);
      return 0;
    }

#ifndef NEW_UI
    mpb_refresh();
    pop_cursor();
#endif

#ifdef _WIN32

    // Cannot InitializeCLInterpreter and source discover.dis
    // in GALA UI until GALA is in the message loop.  Hence
    // the trigger() function above.

    if ( ! runningGalaUI() )
       {
         load_pdfs_complete();
       }
#endif

	if(is_gui()) {
		if(_los("mc")!=LIC_SUCCESS) {
#ifdef NEW_UI
			ShowLMErrorMessage();
#endif
			msg("DIScover: $1\n", error_sev) << "Unable to verify your developer license. Please check the license server, the license file, and the value of LM_LICENSE_FILE, or call customer support" << eom;
			driver_exit(3);
		}
	}
	aset_startup();

// start scripting    
    char * journal = get_main_option("-journal");
    if(journal)
       cmd_journal_log(journal);

//    OpenDomainShell();
// start interactive session
    driver_instance->run();

#ifdef NEW_UI
    delete DISui_app; 
        DISmain_app->unregisterService();
        delete DISmain_app;
    DISgpi_app->unregisterService();
        delete DISgpi_app;
    DISgroup_app->unregisterService();
        delete DISgroup_app;
        DISdormant_app->unregisterService();
        delete DISdormant_app;
    DISrtlFilter_app->unregisterService();
    delete DISrtlFilter_app;
    DIScockpit_app->unregisterService();
        delete DIScockpit_app;
    DISview_app->unregisterService();
        delete DISview_app;
#endif

	if(is_gui()) _lis("mc");
    driver_exit (0);
    return 0;
}

static void print_help ()
//  Print brief help for when the user types  discover -batch, but either omits -home or
//    gets one of the switches or arguments wrong
{
    msg("Usage: discover -batch [-home <project-name>] [-prefs <prefs-file>] [other options]", normal_sev) << eom;
    msg("-h display a brief help summary", normal_sev) << eom;
    msg("-home <project> use the named project", normal_sev) << eom;
    msg("-pdf <files> project definition file(s)", normal_sev) << eom;
    msg("-update <project> updates model for the specified projects", normal_sev) << eom;
    msg("-analysis do dependency analysis (first pass) thru all projects", normal_sev) << eom;
}

// =======================================
// expand project name
static bool local_check_proj_name_for_build(projNode *pn, const char *proj_name)
{
    Initialize(local_check_proj_name_for_build);

    if (check_proj_name_for_build(pn, proj_name))
        return true;

    msg("STDERR: Some modules exist above project $1 in the project hierarchy.") << proj_name << eom;
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

    char *pn_name = pn->get_ln();

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
            msg("STDERR: Project $1 does not exist.") << new_s.str() << eom;
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

void append_branch_name(char *proj_name, char *branch_name)
{
 //do not append branch_name if branch_name is "main"

 if (strcmp(branch_name, "main")) {
   strcat(proj_name, "@@");
   if (!strcmp(branch_name, "checkedout") || !strcmp(branch_name, "checkout"))
     strcat(proj_name, "co");
   else
     strcat(proj_name, branch_name);
 }
}
// =======================================

static bool
ignore_u(int argc,
         char **argv,
         int i)
{
  Initialize(ignore_u);

  for(int j = i + 1; j < argc; ++j) {
    if (!strcmp(argv[j], "-analsys") ||
        !strcmp(argv[j], "-update") ||
        !strcmp(argv[j], "-metric") ||
        !strcmp(argv[j], "-headers") ||
        !strcmp(argv[j], "-dfa_init") ||
        !strcmp(argv[j], "-dfa"))
      return true;
  }

  return false;
}

int xref_main(int argc, char **argv, char **envp)
// process any command line that has -batch in it
{
    Initialize(xref_main);

    int cli_script_flag = 0;
    char *load_files[1024];
    fileLanguage load_file_types [1024];
    int num_of_load_files = 0;
    int printing_xref = 0;
    int remove_old_modules = 0;
    int tidy_xref = 0;
    int save_files = 1;
    int save_includes = 1;
    int update_xref = 0;

    genString pmod_file[1024];
    int num_of_pmod_files = 0;
    genString pmod_command="";
    
    genString pdf_file[1024];
    int num_pdf_files = 0;

    char *visible_projects [1024];
    int num_of_visible_projects = 0;

    genString projects[1024];
    int num_of_projects = 0;

    genString xref_projname = NULL;
    char *remove_files[1024];
    int num_of_remove_files = 0;

    char *merge_xrefs[1024];
    int num_of_merge_xrefs = 0;
    
    char *report_file = NULL;

    fileLanguage file_type = FILE_LANGUAGE_C;

    enum option_type {
        ILLEGAL,
        TEE,
        PMOD,
        LOAD,
        XREF,
        REMOVE,
        PRINT,
        PREFS,
        SAVE_FILES,
        MERGE_XREFS,
        PROJECT_FILE,
        PROJECT_NAME,
        CLI_SCRIPT,
        VISIBLE_PROJECTS,
        ALL_FILES,
        SUB_SYSTEMS,
        FUNCTIONS,
        VARIABLES,
        CLASSES,
        STRUCTURES,
        UNIONS,
        ENUMS,
        TYPEDEFS,
        MACROS,
        REPORT_FILE,
        EXT_PROJ_LISTS,
        ROOT_GROUPS,
        WEIGHT_FILE_NAME,
        SERVER_NAME,
        LP_SERVICE
        };

    option_type option, last_option;

    const char * tmp = getenv("PSET_TIMER");
    char *timing = 0;
    if (tmp) {
        if (tmp[0] == '1' && tmp[1] == '\0') {
            tmp = "load_all_files#load_one_file#smt_prog_import#smt_to_ast#smtHeader::build_ast#smtHeader::smt_delayed_map#smt_cpp_main#cpp_open_file";
        }
        timing = new char [ strlen(tmp) + 1];
        strcpy (timing, tmp);
    }

    int do_simplify_headers = 0;
    int branch_flag = 0;
    int home_flag = 0;

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

    if (_lc()) {
        _le();
        driver_exit(1);
    }
    /* if (_vvc_()) {
        _mmc_();
        driver_exit(1);
        }*/
    if(_lo(LIC_ADMIN)){
        _lm(LIC_ADMIN);
        driver_exit(1);
    }

// pdf-debugger
    char *proj_name     = get_main_option("-pdf_prt");
    char *phy_n         = get_main_option("-pdf_fn");
    char *log_n         = get_main_option("-pdf_ln");
    char *sub_proj_name = get_main_option("-pdf_sub_proj_name");
    if (proj_name || phy_n || log_n)
    {
        buildxref_set_signals();
        load_pdfs_xref_main();
        pdf_debug(proj_name, phy_n, log_n, sub_proj_name);
        driver_exit (0);
    }

    // -save_proj save project structure
    if (check_main_option("-save_proj")) {
        proj_name = get_main_option("-save_proj");
        buildxref_set_signals();
        prj_load_pdfs_start();
        load_pdfs_xref_main();
        prj_load_pdfs_end();
        int res = 0;
        if (proj_name && proj_name[0]) {
            if (branch_flag) append_branch_name(proj_name, branch);
            res = proj_save (proj_name, 0);
          }
        else
            res = proj_save_all (0);

        driver_exit (0);
    }

    // -save_root save all ROOT projects and save PMOD projects
    //            only if the .prj file is outdated
    if (check_main_option("-save_root")) {
        proj_name = get_main_option("-save_root");
        buildxref_set_signals();
        prj_load_pdfs_start();
        load_pdfs_xref_main();
        prj_load_pdfs_end();
        int res = 0;
        if (proj_name && proj_name[0]) {
            if (branch_flag) append_branch_name(proj_name, branch);
            res = proj_save (proj_name, 1);
          }
        else
            res = proj_save_all ( 1 );

        driver_exit (0);
    }

    bool command_line_error = 0;    // no errors so far
    customize::set_write_flag(0);   // most options will not write to pmod

    // setup arguments in options structure
    set_main_options (argc, argv);


    last_option = option = LOAD;
    // u_ignored will be set when "-update", "-analysis", "-metric", or "-headers", "-dfa_init", or "-dfa" is found
    int u_ignored = 0;

    char *group_name[1024];
    int group_name_count = 0;

    char *ext_proj_name[1024];
    int ext_proj_name_count = 0;

    for (int i = 1; i < argc; i++) {
      char *swtch = argv[i];
      if (*swtch == '-') {
        if (!strcmp(swtch, "-batch")) {
          // we already knew this, so just skip the switch
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
              msg("Commandline switch: $1 illegal after $2", error_sev) << swtch
 << eoarg << pmod_command.str() << eom;
              option=last_option=ILLEGAL;
              continue;
            }
                
            pmod_command = swtch;
            is_model_build_flag = true;
            option = last_option = PMOD;
            continue;
          }
        }
        if (!strcmp(swtch, "-core") || !strcmp(swtch, "-debug") || 
            !strcmp(swtch, "-slave_id") || !strcmp(swtch, "-eee")) {
          // we separately checked for these, so just skip the switch
          option=last_option=ILLEGAL;
          continue;
        }
        if (!strcmp(swtch, "-remove")) {
          // remove all modules whose source files no longer exist
          is_model_build_flag = true;
          customize::set_write_flag(1);  // we will need write access to pmods
          remove_old_modules = 1;
          option=last_option=ILLEGAL;
          continue;
        }
        if (!strcmp(swtch, "-print")) {
          // print xref (only if dd code is debug version)
          is_model_build_flag = true;
          printing_xref = 1;
          option=last_option=ILLEGAL;
          continue;
        }
        if (!strcmp(swtch, "-pdf")) {
          option = last_option = PROJECT_FILE;
          continue;
        }
        if (!strcmp(swtch, "-prefs") || !strcmp(swtch, "-sysPrefs") ||
            !strcmp(swtch, "-noUserPrefs") || !strncmp(swtch, "-DIS_", 5) ||
            !strncmp(swtch, "-PSET.", 6) ) {
          // Preference switches are OK
          option=last_option=PREFS;
          continue;
        }
        // External Word Processor file processing
        if (!strcmp(swtch, "-doc")) {
          file_type = FILE_LANGUAGE_EXT;
          if (last_option != REMOVE)
            option = last_option = LOAD;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          continue;
        }
        if (!strcmp(swtch, "-analysis")) {
          u_ignored = 1;
          if(multiple_proj_flag) {
            multiple_proj_flag = 0;
            msg("Switch -update is ignored with -analysis", warning_sev) << eom;
          }

          update_xref     = 2;
          is_model_build_flag = true;
          analysis_flag = 1;
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
        if(!strcmp(swtch, "-source")){
            option = last_option = CLI_SCRIPT;
            continue;
        }
        if(!strcmp(swtch, "-simplify")) {
            do_simplify_headers = 1;
            continue;
        }
        if (!strcmp(swtch, "-subdivide_groups"))
        {
            option = last_option = ROOT_GROUPS; 
            continue;
        }
        if (!strcmp(swtch, "-weights"))
        {
            option = last_option = WEIGHT_FILE_NAME; 
            continue;
        }
        if (!strcmp(swtch, "-report")) {
          option = last_option = REPORT_FILE;
          continue;
        }

        if (!strcmp(swtch, "-update")) {
          u_ignored = 1;
          if (analysis_flag) {
            msg("Switch -update is ignored with -analysis", warning_sev) << eom;
            option=last_option = PROJECT_NAME;
            continue;
          }
          update_xref        = 3;
          multiple_proj_flag = 1;
          is_model_build_flag = true;
          xref_projname      = "/";
          option = last_option = PROJECT_NAME;
          customize::set_write_flag(1);    
          continue;
        }

        if (!strcmp(swtch, "-headers")) {
          u_ignored = 1;
          update_xref     = 4;
          is_model_build_flag = true;
          included_flag = 1;
          xref_projname   = "/";
          option = last_option = PROJECT_NAME;
          customize::set_write_flag(1);    
          continue;
        }

        if (!strcmp(swtch, "-metric")) {
          u_ignored = 1;
          update_xref     = 5;
          multiple_proj_flag = 1;
          is_model_build_flag = true;
          xref_projname   = "/";
          option = last_option = PROJECT_NAME;
          customize::set_write_flag(1);    
          continue;
        }
        
        if (!strcmp(swtch, "-dfa")) {
          u_ignored = 1;
          update_xref     = 6;
          multiple_proj_flag = 1;
          is_model_build_flag = true;
          xref_projname   = "/";
          option = last_option = PROJECT_NAME;
          customize::set_write_flag(1);    
          continue;
        }

        if (!strcmp(swtch, "-dfa_init")) {
          u_ignored = 1;
          update_xref     = 7;
          multiple_proj_flag = 1;
          is_model_build_flag = true;
          xref_projname   = "/";
          option = last_option = PROJECT_NAME;
          customize::set_write_flag(1);    
          continue;
        }

        if (!strcmp(swtch, "-dump_psets")) {
          u_ignored = 1;
          update_xref = 8;
          multiple_proj_flag = 1;
          is_model_build_flag = true;
          xref_projname   = "/";
          option = last_option = PROJECT_NAME;
          continue;
        }

        if(!strcmp(swtch, "-home")){
          option = XREF;
          continue;
        }

        if (!strcmp(swtch, "-first")) {
          update_xref = 2;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          option=last_option=ILLEGAL;
          continue;
        }

        if(!strcmp(swtch, "-server_name")) {
            option = last_option = SERVER_NAME;
            continue;
        }

        if (!strcmp(swtch, "-lpservice") || !strcmp(swtch, "-message_group")) {
            option = LP_SERVICE;
            continue;
        }
        
        // all other legal commandline switches are 1 character each
        if (strlen (swtch) > 2) {
          command_line_error = 1;
          msg("Error: Unrecognized commandline switch: $1\n") << swtch << eom;
          option=last_option=ILLEGAL;
          continue;
        }

        switch (*++swtch) {
        case 'e':
          continue;

        case 'l':
          option = last_option = LOAD;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          break;

        case 'm':
          option = last_option = MERGE_XREFS;
          customize::set_write_flag(1);    
          is_model_build_flag = true;
          break;
          
        case 'x':
          option = XREF;
          break;

        case 'c':
          file_type = FILE_LANGUAGE_C;
          if (last_option != REMOVE)
            option = last_option = LOAD;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          continue;

        case 'C':
          file_type = FILE_LANGUAGE_CPP;
          if (last_option != REMOVE)
            option = last_option = LOAD;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          continue;

        case 'P':
          option = last_option = PROJECT_FILE;
          break;

        case 'r':
          option = last_option = REMOVE;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          break;

        case 't':
          tidy_xref = 1;
          is_model_build_flag = true;
          customize::set_write_flag(1);
          option=last_option=ILLEGAL;
          continue;

        case 's':
          save_files = 1;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          option=last_option=ILLEGAL;
          continue;

        case 'u':
          if (u_ignored || ignore_u(argc, argv, i))
          {
              msg("Warning: Command line switch: -u will be ignored with -analysis, -update, -metric, -dfa_init, -dfa or -headers\n") << eom;
              continue;
          }
          update_xref = 1;
          is_model_build_flag = true;
          customize::set_write_flag(1);    
          option=last_option=ILLEGAL;
          continue;

        case 'v':
          // visible projects
          option = last_option = VISIBLE_PROJECTS;
          break;

        case 'h':
          // Do not bother printing help here if we already have errors;
          //   it will be printed at end of loop
          if (!command_line_error) {
            print_help ();
            driver_exit (0);
          }
                
        default:
          msg("Error: Unrecognized commandline switch: $1\n") << swtch-1 << eom;
          command_line_error = 1;
          break;
        }
      } else {
        switch (option) {

        case ILLEGAL:
          // this parameter follows a switch that does not take arguments
          msg("Unexpected argument : $1", error_sev) << swtch << eom;
          command_line_error = 1;
          break;

        case TEE:
          break;

        case PMOD:
          // this parameter follows a switch that processes an external pmod
          pmod_file[num_of_pmod_files++] = argv[i];
          break;
                
        case LOAD:
          // create xref for argv[++i]
          load_file_types[num_of_load_files] = file_type;
          load_files[num_of_load_files++] = argv[i];
          break;

        case MERGE_XREFS:
          merge_xrefs [num_of_merge_xrefs++] = argv[i];
          break;

        case REMOVE:  // -r switch
          remove_files[num_of_remove_files++] = argv[i];
          break;

        case XREF:  // -home switch
          {
            // create xref file
            char *p = argv[i];
            if (*p != '/')
              xref_projname.printf("/%s",p);
            else
              xref_projname = p;

            home_flag = 1;
            option = last_option;
            break;
            
          }

        case PROJECT_FILE:
          // specify pdf file
          if (num_pdf_files < 1024) {
            pdf_file[num_pdf_files++] = argv[i];
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

        case SERVER_NAME:
          server_name = OSapi_strdup(argv[i]);
          option      = ILLEGAL;
          break;
          
        case REPORT_FILE:
          report_file = argv[i];
          option = ILLEGAL;
          break;

        case CLI_SCRIPT:
          cli_script = argv[i];
          option     = ILLEGAL;
          cli_script_flag = 1;
          break;
                
        case PREFS:
          // Preference options are processed elsewhere.
          break;

        case VISIBLE_PROJECTS:
          visible_projects[num_of_visible_projects++] = argv[i];
          break;

        case ALL_FILES:
          
          all_file_string = argv[i];
          option=ILLEGAL;
          break;
              
        case SUB_SYSTEMS:
                
          sub_system_string = argv[i];
          option=ILLEGAL;
          break;

        case FUNCTIONS:
          
          function_string = argv[i];
          option=ILLEGAL;
          break;

        case VARIABLES:
                
          variable_string = argv[i];
          option=ILLEGAL;
          break;
          
        case CLASSES:
                
          class_string = argv[i];
          option=ILLEGAL;
          break;

        case STRUCTURES:
                
          struct_string = argv[i];
          option=ILLEGAL;
          break;

        case UNIONS:
          
          union_string = argv[i];
          option=ILLEGAL;
          break;

        case ENUMS:
                
          enum_string = argv[i];
          option=ILLEGAL;
          break;
                
        case TYPEDEFS:
          
          typedef_string = argv[i];
          option=ILLEGAL;
          break;
                
        case MACROS:
                
          macro_string = argv[i];
          option=ILLEGAL;
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

        case ROOT_GROUPS:

            Assert(group_name_count < 1024);
            group_name[group_name_count++] = argv[i];
            break;

        case LP_SERVICE: 
          //do nothing since its already been taken care of in main function.
          break;

        default:
          option = last_option;
          break;
        }
      }
    }

    if (!xref_projname.length() && !cli_script_flag) {
        command_line_error = 1;
        msg("Error: -home switch is required\n") << eom;
    }
    if (command_line_error) {
        msg("Command line error", error_sev) << eom;
        print_help ();
        driver_exit (1);
    }

    // initialize project stuff

    visible_projects[num_of_visible_projects] = NULL;

    builtin_mmgr = new MemoryManager(NULL);

    if (!get_main_option("-core")) buildxref_set_signals();
    if(!get_main_option("-e") && !get_main_option("-ee")){
        OSapi_putenv("PSET_PARAPARTION=1");
    }

    driver_instance = new driver;
    // create new scrapbook instance
#ifndef _WIN32
    scrapbook_instance = new scrapbook(
        (char*)gettext(TXT("DIScover Clipboard")), unit_cut_paste);
#else
    scrapbook_instance = new scrapbook(
        (char*)TXT("DIScover Clipboard"), unit_cut_paste);
#endif

    //append branch name to project names
    if (branch_flag && strcmp(branch, "main")) {
      for (int i=0; i<num_of_projects; i++) {
        projects[i] += "@@";
        if (!strcmp(branch, "checkedout") || !strcmp(branch, "checkout"))
           projects[i] += "co";
        else
           projects[i] += branch ;
      }
    }

   //append branch name to home project name
   if (home_flag && branch_flag && strcmp(branch, "main")) {
      xref_projname += "@@";
      if (!strcmp(branch, "checkedout") || !strcmp(branch, "checkout"))
        xref_projname += "co";
      else
        xref_projname += branch;
    }

    // parse all the pdfs, make proj & Xref
    load_project_rtl( num_pdf_files, pdf_file);  
    re_generate_pdf_tree_for_refresh_top_proj();

    steInitialize();
    
    if (timing)
       aset_timer_operation("start");

    if (do_simplify_headers) {
        if (multiple_proj_flag){
            for(int i = 0; i < num_of_projects; i++) {
                convert_project_wild_cards(projects[i].str());
                for (int j=0; j < proj_name_head.size(); j++){
                    proj_name_entry *p = proj_name_head[j];
                    msg("Simplifying #includes for project $1 ") << p->proj_name << eom;
                    ;
                    simplify_batch(p->proj_name);
                }
            }
        } 
        is_model_build_flag = false;
    }

    if (is_disbuild_update()) {
        int res = -1;
        if (!multiple_proj_flag)
            disbuild_update_usage_error ("No project name after -update option");
        else if (num_of_projects != 1)
            disbuild_update_usage_error ("More than one project name after -update option");
        else {
            convert_project_wild_cards(projects[0].str());
            proj_name_entry *p = proj_name_head[0];
            res = disbuild_update_project (p->proj_name, 0);
        }
        driver_exit (res);
    } else if (is_model_build()) {
      if (multiple_proj_flag)
      {
          for(int i = 0; i < num_of_projects; i++)
          {
              convert_project_wild_cards(projects[i].str());
              for (int j=0;j<proj_name_head.size();j++)
              {
                  proj_name_entry *p = proj_name_head[j];

                  msg("Updating project $1 ") << p->proj_name << eom;
                  ;
                  dd_build_xref(num_of_load_files, load_files,
                                load_file_types, num_of_remove_files,
                                remove_files, p->proj_name, save_files,
                                save_includes, update_xref,
                                visible_projects, report_file);
              }
          }
      } else if (analysis_flag) {
           // 0 - regular analysis, 1 - local, 2 - merge into local, 3 - summarize
          int analysis_mode = disbuild_analysis_mode ();
          if (analysis_mode == 3)
              disbuild_analysis_summarize ();
          else if (analysis_mode == 2)
              disbuild_analysis_merge ();
          else {
              if (num_of_projects) {
                  if (analysis_mode) disbuild_analysis_start (num_of_projects);
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
                  if (analysis_mode)
                      disbuild_analysis_pmod_projects ();
                  else {
                      /* Do first pass (dependency analyzis)*/
                      objArr sub_projects;
                      dd_get_pmod_subprojects(sub_projects);

                      for(int proj_index = 0; proj_index < sub_projects.size(); proj_index++)
                          {
                              projNode *sub_proj = (projNode *)sub_projects[proj_index];
                              msg("Dependency analysis for project $1") << sub_proj->get_ln() << eom; ;
                              dd_build_xref(num_of_load_files, load_files, load_file_types,
                                            num_of_remove_files, remove_files, sub_proj->get_ln(),
                                            save_files, save_includes, update_xref, visible_projects,
                                            report_file);
                          }
                  }
              }

              if (analysis_mode == 0) output_dependency_modules(report_file);
          }
      }

      if (included_flag)
      {
          if (num_of_projects)
          {
              for(int i = 0; i < num_of_projects; i++)
              {
                  convert_project_wild_cards(projects[i].str());
                  for (int j=0;j<proj_name_head.size();j++)
                  {
                      proj_name_entry *p = proj_name_head[j];

                      msg("process remaining header files for project $1 ") << p->proj_name << eom;
                      ;
                      dd_build_xref(num_of_load_files, load_files, load_file_types, num_of_remove_files, remove_files,
                                    p->proj_name, save_files, save_includes, update_xref, visible_projects,
                                    report_file);
                  }
              }
          }
          else
          {
              /* Do third pass */
              objArr sub_projects;
              dd_get_pmod_subprojects(sub_projects);
              for(int proj_index = 0; proj_index < sub_projects.size(); proj_index++)
              {
                  projNode *sub_proj = (projNode *)sub_projects[proj_index];

                  msg("process remaining header files for project $1 ") << sub_proj->get_ln() << eom;
                  ;
                  dd_build_xref(num_of_load_files, load_files, load_file_types, num_of_remove_files, remove_files,
                                sub_proj->get_ln(), save_files, save_includes, update_xref, visible_projects,
                                report_file);
              }
          }
      }

      if(!multiple_proj_flag && !analysis_flag && !included_flag) {

            projNode* pn = projNode::find_project(xref_projname);
            if (!pn)
            {
                msg("Project $1 does not exist.\n") << xref_projname.str() << eom;
                return 0;
            }
            if (!local_check_proj_name_for_build(pn, xref_projname))
                return 0;

            dd_build_xref (  num_of_load_files ,  load_files ,  load_file_types ,  num_of_remove_files ,  remove_files ,  xref_projname ,  save_files ,  save_includes ,  update_xref ,  visible_projects ,  report_file);

            if (remove_old_modules) {
                Remove_Old_Modules();    // remove any modules that no longer exist as source files
            }
            
            if (num_of_merge_xrefs)
                dd_merge_xref (num_of_merge_xrefs, merge_xrefs, xref_projname);
            
            if (num_of_pmod_files)
                do_pmod_command (num_of_pmod_files, pmod_file, pmod_command.str());
        
            if (printing_xref)
                dd_print_xref (xref_projname);
            
            if (tidy_xref)
                dd_tidy_xref (xref_projname);
        }
    }

    if (timing) {
        aset_timer_operation("stop");
        aset_timer_operation("dump");
        for (char * fun = strtok(timing,"#"); fun; fun = strtok(0,"#"))
            dump_time_log_statistics(fun);
    }

    if(cli_script_flag){
      // get project node from project name
      if (xref_projname.length()) {
        projNode *projnode = projNode::find_project(xref_projname);
        if (!projnode) {
          msg("Project $1 does not exist.\n") << xref_projname.str() << eom;
          driver_exit(0);
        }
        // set the projnode to be current project
        projNode::set_home_proj (projnode);
      } else {
        projNode* cproj = projNode::get_control_project();
        projNode::set_current_proj (cproj);      
      }
      cli_eval_file(cli_script, cout, 0);
    }

#if defined(_WIN32) && defined(_DEBUG)
                int halt_flag = 0;
    if ( GetDiscoverRegistryDebug("xref_main_exit") )
                {
                        halt_flag = 1;
                        if ( GetDiscoverRegistryDebug("-update") )
                        {
                                if ( GetDiscoverRegistryDebug(argv[7]) == 0 )
                                        halt_flag = 0;
                        }
                }

                if ( halt_flag )
            _asm int 3;
#endif

    driver_exit (0);
    return 0;
}

static FILE *debug_out = 0;

static genString save_home_proj;
static genString save_sys_proj;
static int dummy_sys_proj_flag = 0;
static genString save_pmod_dir;


#ifdef _WIN32
void batch_thread_proc( void * )
{
        shell_configurator_init();
  xref_main( BatchParams.argc, BatchParams.argv, BatchParams.envp);
        shell_configurator_destroy();
  PostThreadMessage( BatchParams.nThreadId, WM_QUIT, 0, 0 ); 
}


extern "C"  {

typedef struct LibraryList {
    HINSTANCE handle;
    struct LibraryList *nextPtr;
} LibraryList;

static LibraryList *libraryList = NULL; /* List of currently loaded DLL's.  */

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

/*stub out some SDK routines referenced in TK code*/
HIMC WINAPI ImmGetContext(HWND)
{
  return 0;
}

BOOL WINAPI ImmReleaseContext(HWND, HIMC)
{
  return 0;
}

LONG  WINAPI ImmGetCompositionStringW(HIMC, DWORD, LPVOID, DWORD)
{
  return 0;
}

}

#endif

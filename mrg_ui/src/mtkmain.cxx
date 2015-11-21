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
// mtkmain.cxx
//
//	main entry point
//
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <locale.h>
#endif

#include <stdlib.h>
#include <tcl.h>
#include <tk.h>
#include <stdio.h>
#undef DELETE // defined in windows.h; conflict with mrg_ui
#include "diffmanager.h"
#include "framework.h"
#include "mtkargs.h"
#include "mtkdiffmanager.h"

/*
 * Forward declarations for procedures defined later in this file:
 */

#ifdef _WIN32
static void		setargv(int *argcPtr, char ***argvPtr);
#endif
static void		Panic TCL_VARARGS(char *,format);

#define USE_PREFS 0

MtkArgs *startup_args      = NULL;

//============================================================================
static char const *getenv(Tcl_Interp *interp, char const *name)
{
    return Tcl_GetVar2(interp, "env", name, TCL_GLOBAL_ONLY);
}

//============================================================================
static bool is_readable(Tcl_Interp *interp, char *fname)
{
    genString cmd("file readable ");
    cmd += FrameWork::quoteForTclString(interp, fname);
    if (Tcl_Eval(interp, cmd) == TCL_OK) {
	char const *result = Tcl_GetStringResult(interp);
	int val;
	if (Tcl_GetBoolean(interp, result, &val) != TCL_OK) {
	    return false;
	}
	return val;
    }
    return false;
}

//============================================================================
static int script_file(Tcl_Interp *interp, const char *script, genString& fname)
{
  static char const *env_psethome = getenv(interp, "PSETHOME");
  static char const *env_home = getenv(interp, "HOME");
  static char const *env_fosterhome = getenv(interp, "DISCOVER_SCRIPTS");
#if USE_PREFS
  static char const *env_optscripts = customize::getStrPref("configuratorOptionalScriptsDir");
#endif
  int fFileFound = 0;
  if( env_home )
  {
    fname = env_home;
    fname += "/.";
    fname += script;
    fFileFound = is_readable(interp, fname);
  }
  if (!fFileFound && env_fosterhome) 
  {
    fname  = env_fosterhome;
    fname += "/";
    fname += script;
    fFileFound = is_readable(interp, fname);
  }

#if USE_PREFS
  if (!fFileFound && env_optscripts) 
  {
    fname  = env_optscripts;
    fname += "/";
    fname += script;
    fFileFound = (OSapi_access(fname, R_OK) == 0);
  }
#endif

  if (!fFileFound && env_psethome) 
  {
    fname  = env_psethome;
    fname += "/lib/";
    fname += script;
    fFileFound = is_readable(interp, fname);
  }
  if(!fFileFound)
      fname = NULL;
  else {
      // !!! Omitting this step may cause problems, but I'll give it
      // a try, as it's good not to bring in unnecessary machinery.
      // genString tmp = OSPATH(fname);
      // fname         = tmp;
  }
  return fFileFound;
}

//============================================================================
static int source_script(Tcl_Interp *interp, const char *script)
{
    genString fname;
    int nRet = TCL_ERROR;

    if (!script)
	return nRet;

    int fFileFound = script_file(interp, script, fname); 
    if( fFileFound ) {
	// dis_print_verbose("%sSourcing %s", indent(depth), (char*)fname);
	nRet = Tcl_EvalFile(interp, fname);
	if(nRet != TCL_OK){
	    char const *err_info = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	    if(err_info)
		fprintf(stderr, "%s\n", err_info);
	}
    } else {
	fprintf(stderr, "File %s does not exist in PSETHOME's lib directory.\n", script);
    }
    // mpb_refresh();
    return nRet;
}

//============================================================================
// The default Tcl_AppInit returns TCL_ERROR if something bad happens,
// and then Tk_Main merrily proceeds to enter an interpreter loop.
// Don't do that; just exit.
//
extern "C" int Tcl_AppInit(Tcl_Interp *interp)
{
    if (Tcl_Init(interp) == TCL_ERROR) {
        fprintf(stderr, "Tcl_Init failed.\n");
    	Panic(interp->result);
	// unreachable: Panic exits
    }
    if (Tk_Init(interp) == TCL_ERROR) {
        fprintf(stderr, "Tk_Init failed.\n");
    	Panic(interp->result);
	// unreachable: Panic exits
    }
    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

    FrameWork framework(interp);

    if(startup_args->isPresent("debug")) {
	framework.DisplayNotice("Attach.");
    }


    // Notes on script sourcing:
    // 	(1) Sourcing the scripts doesn't _do_ anything: it only defines procs
    //	(2) Perhaps the user-specific script should run _after_ mrg_ui.tcl?

    // Run a user-specific configuration script.
    Tcl_SetVar(interp, "tcl_rcFileName", "~/.dismrgrc", TCL_GLOBAL_ONLY);

    // General procedures required for mrg_ui:
    if (source_script(interp, "mrg_ui.tcl") != TCL_OK) {
	// Error already reported.
#ifdef _WIN32
        // Programs using WinMain don't print to terminal window.
	MessageBeep(MB_ICONEXCLAMATION);
	MessageBox(NULL, "Failure reading file %PSETHOME%\\lib\\mrg_ui.tcl.", "fatal error in mrg_ui",
		MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#endif
	exit(EXIT_FAILURE);
    }

    // Create the DiffManager to keep track of what needs to be done:
    DiffManager diff_manager;

    // Prevent the root window from appearing.
    framework.Tcl("wm withdraw .");

    // Now create a view on the diff file that will do all the UI work:
    MtkDiffManager dm(framework, diff_manager);
    dm.OpenDiffManager();

    // Rather than return and give a Tcl prompt,
    // wait for the main dialog to be dismissed and exit.
    dm.Wait();
    exit(EXIT_SUCCESS);

    // unreached:
    return TCL_OK;
}

//----------------------------------------------------------------------------------

void RunBatchMode(void)
{
    DiffManager diff_manager; // this will request all sources to merge
    diff_manager.BatchMerge();
}

int common_main(int argc, char **argv)
{
    static MtkValidArgs argspec[] = { {"debug", 0},
				      {"batch", 0},
				      {"data", 1},
				      {"trace", 0},
				      {NULL, 0} };
    startup_args = new MtkArgs(argc, argv, argspec);

    if (!startup_args->check(0)) {
#ifdef _WIN32
        // Programs using WinMain don't print to terminal window.
	MessageBox(NULL, "usage: mrg_ui [-batch] [-data filename]", "mrg_ui usage", MB_OK);
#else
	fprintf(stderr, "usage: mrg_ui [-batch] [-data filename]\n", argv[0]);
#endif
	return EXIT_FAILURE;
    }

    if(startup_args->isPresent("batch")){
	RunBatchMode();
        exit(EXIT_SUCCESS);
    } else {
	char *rargv[2];

#ifdef _WIN32
	/*
	 * Replace argv[0] with full pathname of executable, and forward
	 * slashes substituted for backslashes.
	 */

	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, sizeof(buffer));
	for (char *p = buffer; *p != '\0'; p++) {
	    if (*p == '\\') {
		*p = '/';
	    }
	}
	rargv[0] = buffer;
#else
        rargv[0] = argv[0];
#endif

	rargv[1] = NULL;
	Tk_Main(1, rargv, Tcl_AppInit);

	// This continuation is avoided, because it only arises after
	// the interpreter has prompted the user in the terminal window.
	// Users of mrg_ui should not deal with that, hence
	// * not reached *
	// But if this point should be reached, the interpreter will have
	// printed a prompt '%' with no newline, so fix things up with:
	printf("\n");
    }
    return EXIT_FAILURE;
}

#ifdef _WIN32
/*
 *----------------------------------------------------------------------
 *
 * WinMain --
 *
 *	Main entry point from Windows.
 *
 * Results:
 *	Returns false if initialization fails, otherwise it never
 *	returns. 
 *
 * Side effects:
 *	Just about anything, since from here we call arbitrary Tcl code.
 *
 *----------------------------------------------------------------------
 */

extern "C" int APIENTRY
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpszCmdLine,
        int nCmdShow)
{
    char **argv;
    int argc;

    Tcl_SetPanicProc(Panic);

    /*
     * Set up the default locale to be standard "C" locale so parsing
     * is performed correctly.
     */

    setlocale(LC_ALL, "C");

    setargv(&argc, &argv);

    return common_main(argc, argv);
}
#else
int main(int argc, char **argv)
{
    return common_main(argc, argv);
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * Panic --
 *
 *	Display a message and exit.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Exits the program.
 *
 *----------------------------------------------------------------------
 */

void
Panic TCL_VARARGS_DEF(char *,arg1)
{
#ifdef _WIN32
    va_list argList;
    char buf[1024];
    char *format;
    
    format = TCL_VARARGS_START(char *,arg1,argList);
    vsprintf(buf, format, argList);

    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, buf, "fatal error in mrg_ui",
	    MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#ifdef _MSC_VER
    _asm {
        int 3
    }
#endif
    ExitProcess(1);
#else
    fprintf(stderr, "fatal error in mrg_ui\n");
    exit(EXIT_FAILURE);
#endif
}

#ifdef _WIN32
/*
 *-------------------------------------------------------------------------
 *
 * setargv --
 *
 *	Parse the Windows command line string into argc/argv.  Done here
 *	because we don't trust the builtin argument parser in crt0.  
 *	Windows applications are responsible for breaking their command
 *	line into arguments.
 *
 *	2N backslashes + quote -> N backslashes + begin quoted string
 *	2N + 1 backslashes + quote -> literal
 *	N backslashes + non-quote -> literal
 *	quote + quote in a quoted string -> single quote
 *	quote + quote not in quoted string -> empty string
 *	quote -> begin quoted string
 *
 * Results:
 *	Fills argcPtr with the number of arguments and argvPtr with the
 *	array of arguments.
 *
 * Side effects:
 *	Memory allocated.
 *
 *--------------------------------------------------------------------------
 */

static void
setargv(int *argcPtr,		/* Filled with number of argument strings. */
        char ***argvPtr)	/* Filled with argument strings (malloc'd). */
{
    char *cmdLine, *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;
    
    cmdLine = GetCommandLine();

    /*
     * Precompute an overly pessimistic guess at the number of arguments
     * in the command line by counting non-space spans.
     */

    size = 2;
    for (p = cmdLine; *p != '\0'; p++) {
	if (isspace(*p)) {
	    size++;
	    while (isspace(*p)) {
		p++;
	    }
	    if (*p == '\0') {
		break;
	    }
	}
    }
    argSpace = (char *) ckalloc((unsigned) (size * sizeof(char *) 
	    + strlen(cmdLine) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = cmdLine;
    for (argc = 0; argc < size; argc++) {
	argv[argc] = arg = argSpace;
	while (isspace(*p)) {
	    p++;
	}
	if (*p == '\0') {
	    break;
	}

	inquote = 0;
	slashes = 0;
	while (1) {
	    copy = 1;
	    while (*p == '\\') {
		slashes++;
		p++;
	    }
	    if (*p == '"') {
		if ((slashes & 1) == 0) {
		    copy = 0;
		    if ((inquote) && (p[1] == '"')) {
			p++;
			copy = 1;
		    } else {
			inquote = !inquote;
		    }
                }
                slashes >>= 1;
            }

            while (slashes) {
		*arg = '\\';
		arg++;
		slashes--;
	    }

	    if ((*p == '\0') || (!inquote && isspace(*p))) {
		break;
	    }
	    if (copy != 0) {
		*arg = *p;
		arg++;
	    }
	    p++;
        }
	*arg = '\0';
	argSpace = arg + 1;
    }
    argv[argc] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;
}
#endif

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
/**********
*
* main.cxx - Galaxy client main modules.
*
**********/

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#include <nameServCalls.h>
#include <DisRegistry.h>
#endif

#include <vport.h>
#include vstartupHEADER
#include vstdlibHEADER

#ifdef WIN_RUNNER_TEST
#include <vtest.h>
#endif
//these three can be deleted when layer::prepare toshutdown is removed
#include <pdumem.h>
#include <pdupath.h>
#include <pdustring.h>


#include <ginterp.h>
#include <gglobalFuncs.h>
#include <gviewer.h>
#include <gviewerMenu.h>
#include <gviewerPopupMenuList.h>
#include <glayer.h>
#include <gdialogLayer.h>
#include <geditorLayer.h>
#include <gstate.h>
#include <tcpServer.h>
#include <xxinterface.h>

#include <tipwin.h>
#include <identify.h>

#ifndef WIN32
#include  <../include/WinHelp.h>
#endif

#include <../../communications/include/Application.h>

Tcl_Interp* g_global_interp; // Global interperter used for top-level applicaton.
int interp_trace = 0; // Flag for watching tcl evaluations
int interp_errTrace = 0; // Flag for watching tcl evaluations

//Global flag indicating that UI client is running
//It is used by MyDialog class to implement different behaviour
//for UI client and graphical views (now part of pset server) 
//when user selects 'Close' in a system menu
int fIsUIClient = 1;

Application* DISui_app = NULL; // My application service context
StateGroup* DIS_stateGroup;
static char integrator_exit_code[5]={27,0,0,27,0};

extern Server DISuiServer;
extern int rcall_dis_connect_app (Application*, vstr*);

extern Viewer *Viewer_vr;
extern Y2K_FLAG;

// Integrator Server TCP communication/connection.
tcpServer* IntegratorServer;

#ifdef _WIN32
typedef DWORD (WINAPI * UT32PROC)(LPVOID lpBuff, DWORD dwUserDefined,
	LPVOID *lpTranslationList);

typedef BOOL (WINAPI * PUTREGISTER)(HANDLE hModule, LPCSTR SixteenBitDLL,
	LPCSTR InitName, LPCSTR ProcName, UT32PROC* ThirtyTwoBitThunk,
	FARPROC UT32Callback, LPVOID Buff);

typedef VOID (WINAPI * PUTUNREGISTER)(HANDLE hModule);

extern "C" {
static PUTUNREGISTER UTUnRegister = NULL;
static HINSTANCE tclInstance;	/* Global library instance handle. */
static int tclPlatformId;	/* Running under NT, 95, or Win32s? */
}
#endif

/**********
*
* main - Entry point for Galaxclient.
*
**********/


//System system_evaluator;

void call_progress(int flag)
{
    static vcursor *oldCursor = NULL;
    static Viewer* vr;
    static stop = 0;

    if (flag == -1) {
	stop++;
	return;
    } else if (flag == -2) {
	stop--;
	if (stop < 0) stop = 0;
	return;
    }

    if (stop && flag) return;

    if (Viewer::GetFirstViewer()) {
    if (flag) {

        DIS_stateGroup->changeState((vchar*)"InProgress=1");
// Use Block instead of changing the cursor so that we get the system wait cursors.
// 
//        if (!oldCursor)
//            oldCursor =  Viewer::GetFirstViewer()->GetDialog()->GetCursor();
//       vresource res = vresourceGet (vapplication::GetCurrent()->
//                                                  GetResources(), vnameInternGlobalLiteral ("WaitCursor"));
//        vcursor* newCursor = new vcursor (res);
	
        //if ( Viewer::GetFirstViewer()->GetDialog() )
			//Viewer::GetFirstViewer()->GetDialog()->Block();
        vapplication *app = vapplication::GetCurrent();
        if( app )
        {
          vapplicationWindowIterator iterator;               
          iterator.Start(app);
          while (iterator.Next())
          {
            vwindow *aWindow = iterator.GetWindow();
            if( !aWindow->IsBlocked() )
              aWindow->Block();
          }
          iterator.Finish();
        }

    } else {
        DIS_stateGroup->changeState((vchar*)"InProgress=0");

 //       vcursor* curCursor =  Viewer::GetFirstViewer()->GetDialog()->GetCursor();

        //if ( Viewer::GetFirstViewer()->GetDialog() )
			//Viewer::GetFirstViewer()->GetDialog()->Unblock();
        vapplication *app = vapplication::GetCurrent();
        if( app )
        {
          vapplicationWindowIterator iterator;               
          iterator.Start(app);
          while (iterator.Next())
          {
            vwindow *aWindow = iterator.GetWindow();
            if( aWindow->IsBlocked() )
              aWindow->Unblock();
          }
          iterator.Finish();
        }
//           
    }
  }
}

int main(int argc, char *argv[])
{

    int status = vTRUE;


#if defined(_WIN32 ) && !defined(_DEBUG) 
    HANDLE hDevNull = CreateFile( "nul", GENERIC_WRITE, 
				  FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
				  OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 
    if( hDevNull != INVALID_HANDLE_VALUE )
	{
	    SetStdHandle( STD_OUTPUT_HANDLE, hDevNull );
	    SetStdHandle( STD_ERROR_HANDLE, hDevNull );
	}
#endif

    vdebugTraceEnter(main);
    vstartup(argc, argv);   // Delayed startup of classes.
    int fIntegratorOpen = 0;



#ifdef _WIN32
    WSADATA data;
    WSAStartup (MAKEWORD(1,1), &data);
#endif

// Init args and flags.
    DIS_args= new Args(argc, argv);

// Check for TCL args.
    if (DIS_args->findArg("-trace") )   interp_trace = 1;
    if (DIS_args->findArg("-errTrace") ) interp_errTrace = 1;

    if (DIS_args->findArg("-tcpTrace") )
	tcpServer::SetTcpTrace(vTRUE);
    else
	tcpServer::SetTcpTrace(vFALSE);

    char name[256];
    char menus[256];
    char pmenus[256];
    char qhelp[256];
    char vrFile[256];

    name[0] = '\0';

    strcpy (menus, "menus.dat");
    strcpy (qhelp, "qhelp.dat");
    strcpy (vrFile, "gala.vr");
    strcpy (pmenus, "popupmenus.dat");

    DIS_args->findValue("-name", name);
    if (name[0] != '\0') {
	const vchar* sep = vfsPath::GetSeparatorString();
	vstr* dir = vfsPath::GetAppDir()->CloneString();
	sprintf (menus, "%s%s..%slib%s%s.menus", (vchar*)dir, sep, sep, sep, name);
	sprintf (pmenus, "%s%s..%slib%s%s.pmenus", (vchar*)dir, sep, sep, sep, name);
	sprintf (qhelp, "%s%s..%slib%s%s.qhelp", (vchar*)dir, sep, sep, sep, name);
	sprintf (vrFile, "%s%s..%slib%s%s.vr", (vchar*)dir, sep, sep, sep, name);
	vstrDestroy (dir);
    }

// Check for menus.dat and qhelp.dat
    char override[256];

    override[0] = '\0';
    DIS_args->findValue("-menus", override);
    if (override[0] != '\0')
	strcpy (menus, override);
    ViewerMenuBar::SetMenuDataFile( (vchar*) menus);

    if(name && (strcmp(name, "y2k") == 0)||strcmp(name, "qar") == 0) 
	Y2K_FLAG = 1;
    else {
	Y2K_FLAG = 0;
	override[0] = '\0';
	DIS_args->findValue("-pmenus", override);
	if (override[0] != '\0')
	    strcpy (pmenus, override);
	ViewerPopupMenuList::SetPopupMenuFile ((vchar*) pmenus);
    }

    override[0] = '\0';
    DIS_args->findValue("-qhelp", override);
    if (override[0] != '\0')
	strcpy (qhelp, override);
    ViewerMenuBar::SetQHelpDataFile( (vchar*) qhelp);

    override[0] = '\0';
    DIS_args->findValue("-vr", override);
    if (override[0] != '\0')
	strcpy (vrFile, override);
    setResourceFile(vrFile);




// Create the global command interpreter for top-level window.
    g_global_interp = make_interpreter();

    DIS_stateGroup = new StateGroup(g_global_interp);

    DIS_stateGroup->defineState ((vchar*)"InProgress");
    DIS_stateGroup->defineState ((vchar*)"ValidSelection");
    DIS_stateGroup->defineState ((vchar*)"EXTRACTsw");

#ifdef WIN_RUNNER_TEST
    vtestStartup(); 
#endif

// See if this app needs a server connection.
    vbool needsServerConnection = vTRUE;
    if ( *name) {
	if (!strcmp(name, "admin") ||
	    !strcmp(name, "y2k") ||
	    !strcmp(name, "qar") ||
	    !strcmp(name, "ModelServer")||
	    !strcmp(name, "modelserver"))
	    needsServerConnection = vFALSE;
    }

    const char* group = 0;
    if (status) {
	group = Application::getGroupName();

	DISui_app = new Application;
    }

#ifndef WIN32
    
    // UNIX HERE (License checking in gala only done on UNIX)
    
    if (_lc()) {
	_le();
	gString cmd = "dis_confirm1 {Fatal License Error} {Quit}";
	cmd += " {DISCOVER is unable to access your DISCOVER license.\n";
	cmd += "Please check your license file and contact SET Support.}";
	gala_eval(g_global_interp, cmd);
	status = vFALSE;
    }

    if (status && Y2K_FLAG == 0) {
	if (_li(LIC_DISCOVER)) {
	    _lm(LIC_DISCOVER);
	    gString cmd = "dis_confirm1 {Fatal License Error} {Quit} ";
	    cmd += " {DISCOVER is unable to access your DISCOVER license.\n";
	    cmd += "Please check your license file and contact SET Support.}";
	    gala_eval(g_global_interp, cmd);
	    status = vFALSE;

	}
    }
    else if (status && Y2K_FLAG==1) {
      char sl[256];
      sl[0]='\0';
      DIS_args->findValue("-app", sl);
      if (strcmp(name, "qar")==0 || ( strcmp(name, "y2k")==0 && strcmp(sl, "EvaluatorUNIX")==0) )
	{
	  gString cmd = "dis_aset_eval_cmd {license VI1 co}";
	  int rv = gala_eval(g_global_interp, cmd);
	  if ( rv != 0 )
	    {
	      _lm(LIC_QAC);
	      status = vFALSE; 
	    }
	}
    }

#endif

    if (status) {
	DISui_app->set_interp (g_global_interp);


	char service[256];
	service[0] = '\0';

	DIS_args->findValue("-service", service);
	if (!*service) {
	    strcpy (service, "DISui");
	}

	if (DISui_app->registerService(group, service, &DISuiServer, &call_progress)) {
	    gString cmd = "dis_confirm1 {Fatal DISCOVER Communications Error} {Quit}";
	    cmd += " {DISCOVER is unable to communicate with the DISCOVER Server.\n";
	    cmd += "Please run Communications Server Reset and try again.}";
	    gala_eval(g_global_interp, cmd);
	    status = vFALSE;
	} 
    }


    char start_layer[256];
    if (status) {
	init_interpreter (DISui_app->interp());

	start_layer[0] = '\0';

// Check for app/layer args.
	DIS_args->findValue("-app", start_layer);



// Run the global startup code.
	eval_StringResource(g_global_interp, (vchar*)"startupCode");

// See if there already is a pset server to connect to.
	gala_eval(g_global_interp, "pset_server_connect");
    }



    // We must always connect to the integrator server 
    // or else it will not know that it should shut down.
    #ifdef _WIN32
    char appname[255];
    appname[0]=0;
    if((!DIS_args->findValue("-name", appname)) || (strcmp(appname, "admin")!=0) )  {
       BOOL noIntegration=FALSE;
       STARTUPINFO si;
       PROCESS_INFORMATION pi;
       char cmdport[256];
	   int integratorport;
	   int integratoaddr;
	   int i;
       cmdport[0] = '\0';
       if( (DIS_args->findValue("-cmdport", cmdport)) && (atoi(cmdport)!=0) ) {
	       IntegratorServer = new tcpServer;
	       if(IntegratorServer ) {
	          fIntegratorOpen = 1;
	          IntegratorServer->SetEvalMode();
	          IntegratorServer->Open(cmdport);
		   }
	   } else {
			// If no port available in the command-line we will ask nameserver to
			// give us integrator port and address.
		    char host[1024];
			char full[1050];
			char intname[1024];
			char protocol[2];
			gethostname(host,1024);
			strcpy(full,"Discover:Integrator:");
			strcat(full,host);
			GetDisRegistryStr("Integrator","alone",protocol,2,"0");
			if(protocol[0]=='1') {
			   GetDisRegistryStr("Integrator","Location",intname,1024,"C:\\Discover\\vc5int");
			   strcat(intname,"\\integratorserver.exe");
			   switch(NameServerGetService(full,integratorport,integratoaddr)) {
			       // We'v got it!
			       case 1 : break;
                   // Nameserver OK but we are unable to find integration service
				   case 0 : memset(&si,0,sizeof(STARTUPINFO));
	                        si.cb=sizeof(STARTUPINFO);
	                        if(::CreateProcess(NULL,intname,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi)==FALSE) {
							    noIntegration=TRUE;
							}
						    // Trying to connect to the integrator again - timeout 
						    // to give it a chance to register it's service.
						    for(i=0;i<200;i++) {
						       if(NameServerGetService(full,integratorport,integratoaddr)==1) break;
							}
						    if(i==200) noIntegration=TRUE;
						    break;
                   // No nameserver detected
				   case -1: // Trying to start nameserver
	                        memset(&si,0,sizeof(STARTUPINFO));
	                        si.cb=sizeof(STARTUPINFO);
		                    if(::CreateProcess(NULL,"nameserv",NULL,NULL,FALSE,DETACHED_PROCESS,NULL,NULL,&si,&pi)==FALSE) {
							    noIntegration=TRUE;
							}
						    // We are starting new copy of the integrator
						    if(noIntegration==FALSE) {
					           memset(&si,0,sizeof(STARTUPINFO));
	                           si.cb=sizeof(STARTUPINFO);
	                           if(::CreateProcess(NULL,intname,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi)==FALSE) {
							       noIntegration=TRUE;
							   }
						       // Trying to connect to the integrator again - timeout 
						       // to give it a chance to register it's service.
						       for(i=0;i<200;i++) {
						          if(NameServerGetService(full,integratorport,integratoaddr)==1) break;
							   }
						       if(i==200) noIntegration=TRUE;
							}
						    break;
			   }
			   if(noIntegration==FALSE) {
			      // Connecting to the integrator
                  IntegratorServer = new tcpServer;
	              if(IntegratorServer ) {
	                  fIntegratorOpen = 1;
	                  IntegratorServer->SetEvalMode();
			          sprintf(cmdport,"%u",integratorport);
	                  IntegratorServer->Open(cmdport);
				      IntegratorServer->WriteBuffer(5,integrator_exit_code);
				  }
			   }
			}
	   }
    }
    #endif

    if (status) {
// Create the viewer and get the initial (blank) view.
	Viewer* vr = new Viewer;
	View* v = vr->find_view (vr->GetViewCode());

// Create the application dialog layer and display it in the viewer.
	dialogLayer* mainLayer = NULL;

// choose the starting dialogLayer
	if (start_layer[0] == '\0')
	    strcpy(start_layer, "Browser");

// look for a pmod server
	if (DIS_args->findArg("-chooser")) {
	    mainLayer = new dialogLayer("ServiceChooser");
	    Tcl_SetVar(g_global_interp, "SwitchTo", start_layer, 
		       TCL_GLOBAL_ONLY);
	    Viewer_vr = vr;
	    vr->Disable_MenuToolBar();
	} else {
	    if (!DIS_args->findArg("-app")) {
// we want the splash screen, not the browser, if we are connecting
// to a pset server that is already running
		strcpy(start_layer, "SplashScreen");  
	    }
	    mainLayer = new dialogLayer(start_layer);
	    Viewer_vr = NULL;
	}

// Make sure that we have a UI connection (if we need it).
	if (needsServerConnection && Application::IsConnectionLost() ) {
	    gString cmd = "dis_confirm1 {Fatal DISCOVER Communications Error} {Quit}";
	    cmd += " {DISCOVER is unable to communicate with the communications server.\n";
	    cmd += "Please run Communications Server Reset and try again.}";
	    gala_eval(g_global_interp, cmd);
	    status = vFALSE;
	}

	if (mainLayer && v)
	    mainLayer->displayInView(v);
	else
	    delete mainLayer;
    }





    if (status)
	tipWindow::Startup(); 

// This is where all of the work is done.
// Handle events until user quits.
    if (status)
	vevent::Process();
    #ifdef _WIN32
       if( fIntegratorOpen ) {
	      IntegratorServer->WriteBuffer(5,integrator_exit_code);
	      IntegratorServer->Close();
	      delete IntegratorServer;
	   }
    #endif







#ifndef WIN32
    WinHelp (NULL, "AutoTRIAL.hlp", HELP_QUIT, 0);
#endif


// is removed from TCL exit code
//if (DISui_app) delete DISui_app;
DISui_app->unregisterService();
while (Viewer::GetFirstViewer())    // Delete all viewers.
   delete Viewer::GetFirstViewer();
if (DIS_stateGroup) delete DIS_stateGroup;
if (DIS_args) delete DIS_args;



#ifdef CLEANUP_ON
// Clean up.
// This is done to make the Galaxy memory checking happier.
    ViewerMenu::DestroyAllMenus();    // Delete all menus.

    if (DISui_app) delete DISui_app;
    if (mainLayer) delete mainLayer;   // causes it to crash.  I don't think it is actually needed -- jwr

    while (Viewer::GetFirstViewer())    // Delete all viewers.
	delete Viewer::GetFirstViewer();

    if (DIS_stateGroup) delete DIS_stateGroup;
    if (DIS_args) delete DIS_args;
    if (startupCode) delete startupCode;
#endif

    vdebugTraceLeave ();

#if defined(_WIN32 ) && !defined(_DEBUG) 
    if( hDevNull != INVALID_HANDLE_VALUE )
	{
	    SetStdHandle( STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE );
	    SetStdHandle( STD_ERROR_HANDLE, INVALID_HANDLE_VALUE );
	    CloseHandle( hDevNull );
	}
#endif

	//  Sending exit code to the integrator - it will exit if reference count = 0
    exit (status);

    return status;
}   /* end of main */

/**********     end of main.C     **********/


/********************************************************************
STOP.  This has no business being here--it is implemented ONLY to allow pset_server to compile.
It is here temporarily--it logically goes in layer.cxx
*/

//-----------------------------------------------------------------------------
// this evaluates a premade tcl file in the context of a layer tcl interpreter.
// basically it gives a layer the ability to intercept a shutdown request--
// if you wanted to save something, you could do that...

//return values: 0=ok to shutdown. 1= do not shutdown 2=abort (cancel button) -1=serious error (eg missing file)

int Layer::PrepareShutdown()
{  
	int nRet=0;
    const char * TCL_SHUTDOWN_FILE="$PSETHOME/bin/gui/shutdown.tcl";
    char *parsed_shutdown_file=NULL;
    parsed_shutdown_file=pdrealpath(TCL_SHUTDOWN_FILE);
    if (parsed_shutdown_file)
	{
		Tcl_Interp* interp = getInterp();
	    Tcl_EvalFile(interp, parsed_shutdown_file);
		if(!(strcmp(Tcl_GetStringResult(interp), "1")))
			nRet=1;
		else if(!(strcmp(Tcl_GetStringResult(interp), "2")))
			nRet=2;
		else if (atoi(Tcl_GetStringResult(interp))<0)
			nRet=-1;
		FREE_MEMORY(parsed_shutdown_file);
	}
    else
	nRet=-1;
    return nRet;
}





/*****     C stubs     **********/

extern "C" {

void aset_driver_exit (int code)
{
    exit (EXIT_FAILURE);
}

#ifdef _WIN32

typedef struct LibraryList {
    HINSTANCE handle;
    struct LibraryList *nextPtr;
} LibraryList;

static LibraryList *libraryList = NULL;	/* List of currently loaded DLL's.  */

HINSTANCE
TclWinGetTclInstance()
{
    return tclInstance;
}

int		
TclWinGetPlatformId()
{
    return tclPlatformId;
}

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

	if (UTRegister(tclInstance, buffer, NULL, "UTProc", &UTProc, NULL,
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

#endif

}

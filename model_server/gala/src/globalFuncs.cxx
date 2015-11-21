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
//**********
//
// globalFuncs.cxx - Gala app global functions.
//
//**********

#include <cstdarg>
namespace std {};
using namespace std;
#include <ggeneric.h>
#include <gglobalFuncs.h>
#include <gString.h>
#include <gviewer.h>
#include <ginterp.h>
#include <glayer.h>
#include <Application.h>


void call_progress(int started_flag);

static int wait = 1; 
static int wait_for_refresh = 0; 
static int wait_nesting = 100;

extern Application* DISui_app;


void show_interpreter_wait(int);

static const vchar * DICTIONARY_SEPERATORS = (vchar *) ":,";



vcursor* oldCursor;
vcursor* waitCursor;

static int Eval (Tcl_Interp* interp, char* cmd)
{
     if (interp_trace) printf ("Eval: %s\n", cmd);
     int c = Tcl_Eval (interp, cmd);
     if (interp_trace) printf ("(%d) %s\n", c, Tcl_GetStringResult(interp));
     return c;
}

void show_interpreter_wait_for_refresh(int flag)
{
    if (flag) {
  	DISui_app->StopCountAction(1);
	show_interpreter_wait(1);
	wait_for_refresh = 1;
    } else {
  	DISui_app->StopCountAction(0);
	wait_for_refresh = 0;
	show_interpreter_wait(0);
    }
}

void show_interpreter_wait (int flag)
{
    
    static int stop = 0;

    if (flag == -1) {
        call_progress(-1);
	return;
    } else if (flag == -2) {
        call_progress(-2);
	return;
    }

    if (flag)
	DISui_app->IncrementCount();
    else
	DISui_app->DecrementCount();
	
#if 0

    if (flag < 0) {
	wait = flag;
    }
    else for (Viewer* vr = Viewer::GetFirstViewer(); vr; vr = vr->next) {
	if (flag) {
	    if (wait) {
		if (wait_nesting == 0) {
		    oldCursor = vr->GetDialog()->GetCursor();
		    vresource res = vresourceGet (vapplication::GetCurrent()->
						  GetResources(), vnameInternGlobalLiteral ("WaitCursor"));
		    waitCursor = new vcursor (res);
		
		    if (vr->GetDialog() )
		        vr->GetDialog()->SetCursor(waitCursor);
		}
	    }
	} else {
	    if (wait) {
		if (wait_nesting == 1) {
		    if (vr->GetDialog() )
		        vr->GetDialog()->SetCursor(oldCursor);
		    delete waitCursor;
		}
	    }
	}
    }
    if (flag) wait_nesting++;
    else if (wait_nesting > 0) wait_nesting--;
#endif
}


int gala_eval_generic (Tcl_Interp* interp, char* execCmd, int show_wait)
{
    int result;	

    //if (wait_for_refresh) {
	//Tcl_SetResult (interp, "Waiting...", TCL_STATIC);
	//Viewer::GetFirstViewer()->DisplayMessage ((vchar*)"Waiting for response from server");
	//return 1;
    //}

    show_interpreter_wait(1);
    if (wait_for_refresh)  {
        result = Eval(interp, execCmd);
    } else {
        result = Eval(interp, execCmd);
    }
    show_interpreter_wait(0);
    
    return result;
}

int gala_eval(Tcl_Interp* interp, char* execCmd)
{
    static gString lastErr = (vchar*) "";
    int value = gala_eval_generic(interp, execCmd, TRUE);

    vchar* err = (vchar*) Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    if (interp_errTrace && err && *err ) {
        if (vcharCompare(err, (vchar*)lastErr) != 0) {
            if (interp_errTrace) dis_message((vchar*)"TCL ERROR:%s", (char*)err);
            lastErr = err;
        }
    }
    return (value);
}

int gala_global_eval(char* execCmd)
{
    Tcl_Interp* interp = g_global_interp;
    int value = TCL_ERROR;
    if( interp )
    {
      static gString lastErr = (vchar*) "";
      value = gala_eval_generic(interp, execCmd, TRUE);

      vchar* err = (vchar*) Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
      if (interp_errTrace && err && (err[0] != '\n') ) {
          if (vcharCompare(err, (vchar*)lastErr) != 0) {
              if (interp_errTrace) dis_message((vchar*)"TCL ERROR:%s", (char*)err);
              lastErr = err;
          }
      }
    }
    return (value);
}

int ReevalTopLayer(void) {
	layerStack* globalStack=Layer::getList();
	Layer * CurLayer=globalStack->top();
    if(CurLayer!=NULL) {
        gString dlgName;
		CurLayer->getResourceName(dlgName);

		gString procName = "DialogLayers.";
		procName+=dlgName;
		procName+=".Procedures";

		Tcl_Interp* interp = CurLayer->getInterp();
	    eval_StringResource(interp, procName);
		return vTRUE;
	}
	return vFALSE;
}



// Utility to evaluate a string resource using TCL.
// Will check $DIS_TCL_PATH/name.tcl before the resource file.
int eval_StringResource(Tcl_Interp* interp, vchar* resourceName) {
    vbool status = vTRUE;
    gString fileName;
    vchar* ptr = NULL;
    vfsPath* vpath;

    gString name = (vchar*)resourceName;
    name += (vchar*)".tcl";
    // replace all '/', ':', '*' and '\\' in name to '_'
    char *p = (char *)name;
    while(*p != '\0'){
	if(*p == '/' || *p == '\\' || *p == ':' || *p == '*')
	    *p = '_';
	p++;
    }
    vpath = vfsPath::BuildScribed(vfsPATH_TYPE_FULL,
				  vcharScribeLiteral("$GALA_SCRIPTS"),
				  vcharScribeLiteral(name), 
				  NULL);
    vpath->ExpandVariables();
    vexWITH_HANDLING {
	status = vpath->Access(vfsFILE_EXISTS | vfsCAN_READ);
    } vexON_EXCEPTION {
	status = vFALSE;
    } vexEND_HANDLING;

    if(status==vFALSE) {
	vpath = vfsPath::BuildScribed(vfsPATH_TYPE_FULL,
				      vcharScribeLiteral("$PSETHOME"),
				      vcharScribeLiteral("lib"),
				      vcharScribeLiteral("gala_scripts"),
				      vcharScribeLiteral(name), 
				      NULL);
	vpath->ExpandVariables();
	vexWITH_HANDLING {
	    status = vpath->Access(vfsFILE_EXISTS | vfsCAN_READ);
	} vexON_EXCEPTION {
	    status = vFALSE;
	} vexEND_HANDLING;
    }


    if (status) {
	if (interp_errTrace)
	    dis_message((vchar*)"Getting text from $DIS_TCL_PATH/%s.", (vchar*) name);
	vfsFile* vfile = vfsFile::Open (vpath, vfsOPEN_READ_ONLY);
	vfile->Seek (0, vfsSEEK_FROM_END);
	int size = (int) vfile->GetPos();
	ptr = new vchar[size+1];
	vfile->Seek(0, vfsSEEK_FROM_START);
	vfile->Read((vbyte*)ptr, 1, size);
	ptr[size] = 0;
	vfile->Close();

// Next try loading from the resource file.
    } else {
	vresource stringRes;
	vtext* procedures;
	if (getResource(resourceName, &stringRes)) {
	    status = vTRUE;    // We found it.
// Large text resources can cause a lot of warning messages.
// So we lower the verbosity while loading the resource.
	    int oldLevel = vdebugGetWarningLevel();
	    vdebugSetWarningLevel(vdebugLEVEL_MIN);
	    procedures = new vtext(stringRes);
	    vdebugSetWarningLevel(oldLevel);
	    long len = procedures->GetLength();
	    ptr = new vchar[len+1];
	    procedures->GetTextRange(0, len, ptr, len);
	    ptr[len] = 0;
	    delete procedures;
	} else {
	    dis_message((vchar*)"Unable to find '%s' string resource!", resourceName);
	    status = vFALSE;
	}
    }

    if (status) {
	status = gala_eval(interp, (char*)ptr);
	delete [] ptr;
    }
    delete vpath;
    return(status);
}


// Utility function to display text in the active viewer.
vbool dis_message (const vchar* msgFormat, ...)
{
    gInit(dis_message);

    vbool status = vFALSE;

    static gString msgString;

    va_list ap;
    va_start(ap, msgFormat);
    msgString.vsprintf(msgFormat, ap);

    Viewer* vr;
    View *v;
    Viewer::GetExecutingView (vr, v);
    if (vr && v) {
	    status = vTRUE;
	    vr->DisplayMessage(msgString);
    }
    
    return (status);
}




/**********     end of globalFunc.cxx     **********/


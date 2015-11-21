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
 /*********
 *
 * dis_cmds.cxx - Menu manipulation commands.
 *
 **********/

#include "cLibraryFunctions.h"
#include <vport.h>
#include <vchar.h>
#include <vstdio.h>
#include <vstdlib.h>
#include <vsignal.h>
#include <vfs.h>
#include <vport.h>


#include <galaxy_undefs.h>

#include <machdep.h>
#include <ggeneric.h>
#include <gglobalFuncs.h>
#include <gString.h>
#include <gcontrolObjects.h>

#include <gvpopen.h>

#include <tcpServer.h>
#include <gapl_menu.h>    // This has the function externs!

#include <gviewer.h>
#include <gview.h>
#include <gfileChooser.h>
#include <grtlClient.h>
#include <glayer.h>
#include <gdialogLayer.h>
#include <geditorLayer.h>
#include <gtoolbar.h> 
#include <ggenConfirmDialog.h>
#include <gAttributeEditor.h>
#include <gDrawingArea.h>
#include <tipwin.h>

#include vbuttonHEADER
#include vsessionHEADER
#include vdasservHEADER
#include vspinnerHEADER
#include vcommHEADER
#include vscrapHEADER
#include vmemHEADER
#include vstdioHEADER


#include <Application.h>
#include "DIS_main/interface.h"
#include "DIS_gpi/interface.h"

void remote_call_async (Application *, _vstrptr, _vstrptr);

#include <SelectionIterator.h>

#ifndef _WIN32
#include "WinHelp.h"
#else
#undef VOID
#include <windows.h>
#endif

class Viewer;
class View;
class Layer;

typedef void notifier_function(void);

extern tcpServer* IntegratorServer;

extern int SendToServer_sync(char *msg, gString& result);
extern int SendToServer_asyn(char *msg);

static Tcl_Interp * process_interpreter    = NULL;
static       vstr * process_end_script     = NULL;
static const vchar* HISTORY_MENU           = (vchar*)"HistoryMenu";
static const vchar* PROCEDURES             = (vchar*)".Procedures";
static char       * poll_timer_proc        = NULL;
static Tcl_Interp * poll_timer_interpreter = NULL;
static int          waiting_for_process    = 0;
gdPollTimer poll_timer;


Viewer *Viewer_vr = NULL;

const char* FOLLOW_PROJECT = "empty";
extern Application* DISui_app;



// Utility functions
//    These functions start with 'dis_' but do are NOT callable from TCL.



// Utility to find a item given its name and interpreter.
// NOTE - This function may change interp to the item's interpreter.
//        (I know, I should think of a better way.)
void* dis_findItem(Tcl_Interp*& interp, vchar* name)
{
   void* myItem = NULL;

    // Try getting the item from the interpreter's dialog.
    char* gdText = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);
    int gdialog_id = -1;
    if (gdText) 
		gdialog_id = atoi (gdText);
    GDialog* gd = NULL;
    if (gdialog_id > 0 )
        gd = GDialog::FindGDialog (gdialog_id);
    if (gd)
        myItem = (gdTextItem*)gd->FindItem(vnameInternGlobal(name) );
    // If we haven't found the item, try using the executing view.
    if (myItem == NULL) {
        Viewer* vr;
        View *v;
        Viewer::GetExecutingView (vr, v);
        if (vr && v) {
            myItem = (gdTextItem*)v->FindItem (name);
            // Switch over to the view's interpreter.
            interp = v->interp();
        }
    }

    return (myItem);
}    // End of dis_findItem method.



void gdPollTimer::ObserveTimer(void)
{
    static int inside = 0;

    if(inside)
	return;
    inside = 1;
    if(poll_timer_proc)
	gala_eval_generic(poll_timer_interpreter, poll_timer_proc, FALSE);
    inside = 0;
}




// TCL commands.
//    These functions are added to the TCL interpreter.


int dis_file (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    GDialog* gd = 0;
    Viewer* vr = 0;
    View* v = 0;
    gdTextItem* myItem;

    char* gdid = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);
    int gdialog_id = -1;
    if (gdid)
        gdialog_id = atoi (gdid);

	if (gdialog_id > 0 ) {
        gd = GDialog::FindGDialog (gdialog_id);
    }
    if (gd && (argc == 4))
        myItem = (gdTextItem*)gd->FindItem(vnameInternGlobal((const vchar*)argv[4]));
    else {
        Viewer::GetExecutingView (vr, v);
	    if (vr && v && (argc == 4)) {
	        myItem = (gdTextItem*)v->FindItem ((vchar*)argv[3]);
	    }
    }
   
    if (strcmp(argv[1], "-new") == 0) {
        if (strcmp(argv[2], "-c") == 0) {
        } else if (strcmp(argv[2], "-c") == 0) {
        } else if (strcmp(argv[2], "-cpp") == 0) {
        } else if (strcmp(argv[2], "-frame") == 0) {
        } else if (strcmp(argv[2], "-plain") == 0) {
        } else if (strcmp(argv[2], "-ste") == 0) {
	    }
    } else if (strcmp(argv[1], "-open") == 0) {
	    if (argc == 4 && v) {
            viewLayer* vl = viewLayer::findViewLayer ((vchar*)argv[3]);
            if (!vl) {
                vl = new viewLayer;
	            vl->setTitle((vchar*)argv[2]);
            }
	        if (v) vl->displayInView(v);
	        //myItem->ShowFile((vchar*)argv[2], 0);
        }
    } else if (strcmp(argv[1], "-close") == 0) {
    } else if (strcmp(argv[1], "-revert") == 0) {
    }

    
    return TCL_OK;
}

int dis_layer_close (ClientData, Tcl_Interp* interp, int argc, char* argv)
{ 
    if (argc != 2) TCL_EXECERR("wrong number of arguments")

    char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
    char* v  = Tcl_GetVar (interp, "v", TCL_GLOBAL_ONLY);
    if (v && vr) {
        int viewer_id = atoi (vr);
        int view_id = atoi (v);

        Viewer* vr = Viewer::find_viewer (viewer_id);
        if (!vr)
	    return TCL_ERROR;

        View* v = vr->find_view (view_id);
        if (!v)
	    return TCL_ERROR;

	Layer* l = v->GetLayer();
	if (l) delete l;
    }
    
    return TCL_OK;
}

//a mini-helper function for dis_layer

//returns 0 for ok, 1 for not ok
static int licenseCheck(const char * layer, Tcl_Interp* interp)
{
	int nRet=0;
	int must_check=0;
	gString lchkfunc;
	if (layer)
	{
		if (strcmp(layer, "EvaluatorNT")==0)
		{
			lchkfunc="license VI1 co";
			must_check=1;
		}
		//else if ....
	}
	if (must_check)
	{
		// dis_blah "yourfunction", which returns 0 if unsuccessful.

		//because of the nt/unix differences  <sigh>
#ifdef WIN32
		gString cmd="dis_client_access_eval \"";
#else
		gString cmd="dis_aset_eval_cmd \"";
#endif
		cmd+=lchkfunc;
		cmd+="\"";
		Tcl_Eval(interp, cmd);
		if (strcmp(Tcl_GetStringResult(interp),"0")==0)
		{
			nRet=1;
		}
	}
	return nRet;

}



int dis_layer (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{    
    if ((argc < 2) || (argc > 5))
	TCL_EXECERR("wrong number of arguments")

    Viewer* vr;
    View* v;

//    char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
//    char* v  = Tcl_GetVar (interp, "v", TCL_GLOBAL_ONLY);
    Viewer::GetExecutingView(vr, v);
    if (vr && v) {
		//lets start by cleanign this this up a bit
		//this is a check to enable licenseing on layer switches
		if (argc>3 && (strcmp(argv[1], "-open")==0) && (strcmp(argv[2], "DialogLayer")==0) )
		{
			if (licenseCheck(argv[3], interp)!=0)
				return TCL_OK;
		}

		
        if (strcmp(argv[1], "-close") == 0) {
	    Layer* l = v->GetLayer();
	    if (l) delete l;
	    v->refresh();
	    return TCL_OK;
        }
	
        if (strcmp(argv[2], "DialogLayer") == 0) 
        {
            Layer* l;
            gString nm;
            const layerStack* ls = Layer::getList();
 
    	    for (int i=0; i<ls->size(); i++) 
          {
            l = (*ls)[i];
            l->getResourceName(nm);
            if (vcharCompare((vchar*)nm, (vchar*)argv[3]) == 0) 
            {

		          /*
		          * If there is no view associated with the layer, then display
		          * the layer in the current view because the view that is associated
		          * with the layer is gone
	 	          */

		          View* lv = l->getView();
		          if (!lv)
              {
			          l->displayInView (v);
                return TCL_OK;
              }
		          else if (v->GetLayer() != l)
              {
                l->displayInView(v);
                return TCL_OK;
              } else
		return TCL_OK;
              //don't open a new dialog in any case, reuse an old one
              if (argc >= 5 && strcmp(argv[4], "-reuse") == 0)
                return TCL_OK;
	          }
          }
	    dialogLayer* dl = new dialogLayer(argv[3]);
            gString argList = (vchar*)"";
            for (int j=4; j<argc; j++) {
                argList += (vchar*)" ";
                argList += (vchar*)argv[j];
            }
            Tcl_SetVar(dl->getInterp(), "Args", (char*)(vchar*)argList, TCL_GLOBAL_ONLY);
	    dl->displayInView (v);
        } else if (strcmp(argv[2], "EditorLayer") == 0) {
            editorLayer* el = new editorLayer;
            el->displayInView (v);
        } else if (strcmp(argv[2], "viewLayer") == 0) {
	    viewLayer* vl = new viewLayer;
	    vl->setTitle((vchar*)argv[3]);
	    vl->displayInView(v);
	}
    } else {
	Tcl_SetResult (interp, "couldn't find v vr", TCL_STATIC);
	return TCL_ERROR;
    }

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_close  (ClientData, Tcl_Interp* interp, int argc, char* argv)
{
    if (argc != 2) {
	TCL_EXECERR("wrong number of arguments")
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
    char* v  = Tcl_GetVar (interp, "v", TCL_GLOBAL_ONLY);
    if (v && vr) {
        int viewer_id = atoi (vr);
        int view_id = atoi (v);

        Viewer* vr = Viewer::find_viewer (viewer_id);
        if (!vr)
	    return TCL_ERROR;

        View* v = vr->find_view (view_id);
        if (!v)
	    return TCL_ERROR;
    }
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------

int dis_view_manage (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{    
    if (argc != 2 && argc != 3)
	TCL_EXECERR("wrong number of arguments")

    if (strcmp (argv[1], "-open") == 0) {
        new Viewer;
    }

    char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
    char* v  = Tcl_GetVar (interp, "v", TCL_GLOBAL_ONLY);
    if (v && vr) {
        int viewer_id = atoi (vr);
        int view_id = atoi (v);

        Viewer* vr = Viewer::find_viewer (viewer_id);
        if (!vr)
	    return TCL_ERROR;

        View* v = vr->find_view (view_id);
        if (!v)
	    return TCL_ERROR;
	
	if (strcmp (argv[1], "-close") == 0) {
	    delete vr;
	    if (argc == 2 || strcmp (argv[2], "-noexit") != 0) {
		if (!Viewer::GetFirstViewer()) {
			tipWindow::ResetTimer();
			vevent::StopProcessing();
		}
	    }
	} 

	else if (strcmp (argv[1], "-split_horizontal") == 0) {
	    v->split_vertical(50);
	} 

	else if (strcmp (argv[1], "-split_vertical") == 0) {
	    v->split_horizontal(50);
	} 

	else if (strcmp (argv[1], "-add") == 0) {
	    v->addPane(50);
	} 

	else if (strcmp (argv[1], "-delete") == 0) {
	    v->remove();
        }

    } 

    else {
	TCL_EXECERR ("wrong number of arguments");
    }
    
    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_customize_toolbar (ClientData, Tcl_Interp* interp, int, char *[])
{
    char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
    if (vr) 
    {
	int viewer_id = atoi (vr);
	
	Viewer* vr = Viewer::find_viewer (viewer_id);
	if (!vr)
	    return TCL_ERROR;
	
	ToolBarDialog * newToolBarDialog = new ToolBarDialog(vloadableDONT_LOAD);
	newToolBarDialog->initialize (vr->GetToolBar());
	return TCL_OK;
    }
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------

int dis_build_menu (ClientData,
		    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (apl_append_separator);

    if (argc < 3) 
	TCL_EXECERR ("wrong number of argument")

    if (vcharCompare((vchar*)argv[1], HISTORY_MENU) == 0)  {
	char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
	if (vr) {
	    int viewer_id = atoi (vr);
	    Viewer* vr = Viewer::find_viewer (viewer_id);
	    if (!vr)
		return TCL_ERROR;
	    
	    vr->AddLayerItems((vchar*)argv[1], (vchar*)argv[2]);
	}
    }
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_SetViewVariables(ClientData,
			 Tcl_Interp* interp, int , char *[])
{
    Viewer* vr = NULL;
    View* v = NULL;
    
    Viewer::GetExecutingView(vr, v);

    if (vr && v) {
	char buffer[100];
	sprintf(buffer, "%d", vr->GetId());
	Tcl_SetVar(interp, "vr", buffer, TCL_GLOBAL_ONLY);
	sprintf(buffer, "%d", v->GetId());
	Tcl_SetVar(interp, "v", buffer, TCL_GLOBAL_ONLY);
	return TCL_OK;
    } else
	return TCL_ERROR;
}


//-----------------------------------------------------------------------------

int dis_view_cl (ClientData,
	      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (apl_append_separator);

    if (argc < 2) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    if (vcharCompare((vchar*)argv[1], (vchar*)"-close") == 0)  {
	char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
	char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
	if (vr && v) {
	    int viewer_id = atoi (vr);
	    int view_id = atoi (v);
	    Viewer* vr = Viewer::find_viewer (viewer_id);
	    if (!vr)
		return TCL_ERROR;
	    View* v = vr->find_view(view_id);
	    if (!v)
		return TCL_ERROR;

	    delete v;
	}
    } else if (vcharCompare((vchar*)argv[1], (vchar*)"-layer") == 0)  {
	char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
	char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
	if (vr && v) {
	    int viewer_id = atoi (vr);
	    int view_id = atoi (v);
	    Viewer* vr = Viewer::find_viewer (viewer_id);
	    if (!vr)
		return TCL_ERROR;
	    View* v = vr->find_view(view_id);
	    if (!v)
		return TCL_ERROR;

	    gString nm;
	    const layerStack* ls = Layer::getList();

	    int i = 0;
	    Layer* l;
	    if (ls->size()) {
		do  {
		    l = (*ls)[i];
		    l->getResourceName(nm);
		    i++;
		}  while ((vcharCompare((vchar*)nm, (vchar*)argv[2]) != 0) && (i < ls->size()));
		
		if (vcharCompare((vchar*)nm, (vchar*)argv[2]) == 0) {
		    l->displayInView(v);
		    return TCL_OK;
		}
	    }

	    viewLayer* vl = viewLayer::viewLayerList_->findViewLayer((vchar*)argv[2]);
	    if (vl) {
		vl->displayInView(v);
		v->MakeCurrent();
	    }
	}
    }
    return TCL_OK;
}


dis_viewer_new (ClientData, Tcl_Interp*, int , char* [])
{
    new Viewer;

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_launch_dialog (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{

    gInit (dis_launch_dialog);

    if (argc < 2) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }

    gString dialogName = "GenericDialogs.";
    dialogName += argv[1];

    vresource res;
    if (getResource((const vchar*)dialogName, &res) ) {
	   GDialog* myDialog = new GDialog(res);
	    
	   // Give the dialog box access to the view which created it.
	   char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
	   char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);

	   vwindow* display = vwindow::GetRoot();

	   vrect initialPos;
	   initialPos.SetX((display->GetWidth()-myDialog->GetWidth())/2);
	   initialPos.SetY((display->GetHeight()-myDialog->GetHeight())/2);
	   initialPos.SetWidth(myDialog->GetWidth());
	   initialPos.SetHeight(myDialog->GetHeight());
	   myDialog->SetRect(&initialPos);
	

	   myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
	   myDialog->SetInterpVariable((vchar*)"v",  (vchar*)v);
		int nBlock=0;
	   // See if the dialog is modal
	   int i = 2;
	   if (argc > 2  && (strcmp(argv[i],"-modal") == 0)) {
	      myDialog->SetModal (vTRUE);
	      i++;
		  // the following is added because modal dialogs should go into their own event loops...

		  if(argc>3 && (strcmp(argv[i], "-block") == 0)) {
			  nBlock++;}
	   }


       // Pass through any remaining arguments.
       gString argList;
       for (; i<argc; i++) {
          argList += " ";
          argList += argv[i];
       }
       myDialog->SetInterpVariable((vchar*)"Args", argList);

       myDialog->Open();

       // eval the procedures.
       gString procName = dialogName;
       procName += PROCEDURES;
       eval_StringResource(myDialog->GetInterpreter(), procName);
	   if (nBlock)
	   {
			myDialog->SetBlocking();
	   }
	    
	   vchar buffer[64];
	   sprintf((char*)buffer, "%d", myDialog->GetId());
	   Tcl_SetResult (interp,(char*) buffer, TCL_VOLATILE);

	   return TCL_OK;
    }

    if (interp_errTrace)
        dis_message((vchar*)"Error! dialog '%s' not found in dictionary.",
                    (const vchar*)dialogName);
    return TCL_ERROR;
}
//-----------------------------------------------------------------------------

int dis_launch_selBrowser (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {

    gInit (dis_launch_selBrowser);

    if (argc < 2) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }

    vresource res;
    if (getResource("GenericDialogs", &res))  {
	   if (getResource(res, "SelectionBrowser", &res)) {
	     GDialog* myDialog = new GDialog(res);
	    
	     // Give the dialog box access to the view which created it.
	     char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
	     char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);

	     vwindow* display = vwindow::GetRoot();
	    
	     vrect initialPos;
	     initialPos.SetX((display->GetWidth()-myDialog->GetWidth())/2);
	     initialPos.SetY((display->GetHeight()-myDialog->GetHeight())/2);
	     initialPos.SetWidth(myDialog->GetWidth());
	     initialPos.SetHeight(myDialog->GetHeight());
	     myDialog->SetRect(&initialPos);

	     myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
	     myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);

	     myDialog->SetInterpVariable((vchar*)"SBLabel", (vchar*)argv[1]);
	     myDialog->SetInterpVariable((vchar*)"SBOKFunc", (vchar*)argv[2]);

	     // See if the dialog is modal

	     if (argc > 3  && (strcmp(argv[3],"-modal") == 0)) {
		    myDialog->SetModal (vTRUE);
	     }

         // eval the procedures.
         gString procName = "GenericDialogs.SelectionBrowser.";
         procName += PROCEDURES;
         eval_StringResource(myDialog->GetInterpreter(), procName);
	    
	     char buffer[64];
	     sprintf((char*)buffer, "%d", myDialog->GetId());
	     Tcl_SetResult (interp,(char*) buffer, TCL_VOLATILE);

		 myDialog->Open();
	     return TCL_OK;
	  }
    }

    if (interp_errTrace)
        dis_message((vchar*)"Error: dialog '%s' not found in dictionary.",
                    argv[1]);

    return TCL_ERROR;
}


//-----------------------------------------------------------------------------

int dis_close_dialog (ClientData,
					  Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_close_dialog);
	
    if (argc < 1) {
		Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    char* gd = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);
    if (gd) {
        int gdialog_id = atoi (gd);
		
        GDialog* gd = GDialog::FindGDialog (gdialog_id);
        if (!gd)
			return TCL_ERROR;
		else  
		{
			char *dialog_close_callback = Tcl_GetVar (interp, "DialogCloseCallback", TCL_GLOBAL_ONLY);
			char* vr_id_str             = Tcl_GetVar (interp, "vr",  TCL_GLOBAL_ONLY);
			char* v_id_str              = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
			if(dialog_close_callback != NULL && vr_id_str != NULL && v_id_str != NULL)
			{
				int vr_id  = atoi(vr_id_str);
				Viewer* vr = Viewer::find_viewer (vr_id);
				if (vr)
				{
					int v_id = atoi(v_id_str);
					View* v  = vr->find_view(v_id);
					if(v)
						gala_eval_generic (v->interp(), dialog_close_callback, FALSE);
				}
			}

			gd->CloseDialog();
			gd->DeleteLater();
		}
    }
    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_setvar (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_dialog_setvar);

    if (argc > 5 || argc < 4) {
        TCL_EXECERR("Invalid number of arguments.")
    }

    // Use layer name to set the value.
    if (strcmp (argv[1], "-layerId") == 0) {
	Layer* layer = Layer::find_layer(atoi(argv[2]) );
	if (layer) {
           if (argc == 5 && argv[3] && argv[4])
              Tcl_SetVar (layer->getInterp(), argv[3], argv[4], TCL_GLOBAL_ONLY);
        } else
            TCL_EXECERR1("Unable to find layer id", argv[2]);

    // Use layer name to set the value.
    } else if (strcmp (argv[1], "-layer") == 0) {
	Layer* layer = Layer::find_layer (argv[2]);
	if (layer) {
           if (argc == 5 && argv[3] && argv[4])
              Tcl_SetVar(layer->getInterp(), argv[3], argv[4], TCL_GLOBAL_ONLY);
        } else
            TCL_EXECERR1("Unable to find layer name", argv[2]);
	    
    // Use dialog's (or view's) interpreter to set the value.
    } else  if (strcmp (argv[1], "-dialog") == 0) {
        int gdialog_id = atoi (argv[2]);

        GDialog* gd = GDialog::FindGDialog (gdialog_id);

        //
        //  If we didn't find a dialog, then check for a 
        //  viewer since they have unique IDs.
        //

        if (!gd) {
            int view_id = atoi (argv[2]);

	    char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
	    if (vr) {
	        int viewer_id = atoi (vr);
	        Viewer* vr = Viewer::find_viewer (viewer_id);
	        if (!vr) return TCL_ERROR;
	        View* v = vr->find_view(view_id);
	        if (!v) return TCL_ERROR;

                // Need to add braces to allow multi-word values.
                vstr* cmdStr = vstrCloneScribed(vcharScribeFormat(
                                                (vchar*)"{%s}", argv[4]) );
                v->SetInterpVariable((vchar*)argv[3], cmdStr);
                vstrDestroy(cmdStr);
            }
        }

        else {
	    gd->SetInterpVariable((vchar*)argv[3], (vchar*)argv[4]);
        }
	    
    // Use the given view's interpreter to set the value.
    } else if (strcmp (argv[1], "-view") == 0) {
        int view_id = atoi (argv[2]);

        Viewer* vr = Viewer::GetFirstViewer();
        if (vr) {
	    View* v = vr->find_view(view_id);
	    if (v)
                v->SetInterpVariable((vchar*)argv[3], (vchar*)argv[4]);
            else 
                TCL_EXECERR("Unable to find view.")
        } else
            TCL_EXECERR("Unable to find viewer.")
    }

    else if (strcmp (argv[1], "-global") == 0) {
	Tcl_SetVar (g_global_interp, argv[2], argv[3], TCL_GLOBAL_ONLY);
    }

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_getvar (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_dialog_getvar);

    vchar *returnValue;

    if (argc > 4 || argc < 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    // Use layer name to get the value.
    if (strcmp (argv[1], "-layerId") == 0) {
	Layer* layer = Layer::find_layer(atoi(argv[2]) );
	if (layer) {
            Tcl_GetVar(layer->getInterp(), argv[3], TCL_GLOBAL_ONLY);
        } else
            TCL_EXECERR1("Unable to find layer id", argv[2]);

    // Use layer name to get the value.
    } else if (strcmp (argv[1], "-layer") == 0) {
	Layer* layer = Layer::find_layer (argv[2]);
	if (layer) {
            Tcl_GetVar(layer->getInterp(), argv[3], TCL_GLOBAL_ONLY);
        } else
            TCL_EXECERR1("Unable to find layer name", argv[2]);
	    
    } else if (strcmp (argv[1], "-dialog") == 0) {
        int gdialog_id = atoi (argv[2]);

        GDialog* gd = GDialog::FindGDialog (gdialog_id);

        //
        //  If we didn't find a dialog, then check for a 
        //  viewer since they have unique IDs.
        //

        if (!gd) {
            int view_id = atoi (argv[2]);

	    Viewer* vr = Viewer::GetFirstViewer ();
	    View* v = vr->find_view(view_id);
            while (!v && (vr = vr->GetNextViewer ())) {
                v = vr->find_view(view_id);
            }

	    if (!v) return TCL_ERROR;

            returnValue = v->GetInterpVariable((vchar*)argv[3]);
        }

        else {
	    returnValue = gd->GetInterpVariable((vchar*)argv[3]);
        }
    }

    else if (strcmp (argv[1], "-view") == 0) {
        int view_id = atoi (argv[2]);

	Viewer* vr = Viewer::GetFirstViewer ();
	View* v = vr->find_view(view_id);
        while (!v && (vr = vr->GetNextViewer ())) {
            v = vr->find_view(view_id);
        }

	if (!v) return TCL_ERROR;

        returnValue = v->GetInterpVariable((vchar*)argv[3]);
    }

    else if (strcmp (argv[1], "-global") == 0) {
	returnValue = (vchar*)Tcl_GetVar(g_global_interp, argv[2], TCL_GLOBAL_ONLY);
    }

    Tcl_SetResult (interp, (char *)returnValue, TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdTextItemSetText(ClientData,
			  Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdTextItemSetText);
    
    if (argc < 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextItem *myItem = (gdTextItem *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("text item not found", argv[1]);
    char *txt = argv[2];
    myItem->SetText((vchar *)txt);
    Tcl_SetResult(interp, "", TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdTextItemGetText(ClientData,
			  Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdTextItemGetText);
    
    if (argc < 2) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextItem *myItem = (gdTextItem *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("text item not found", argv[1]);
    vtext *textobj = myItem->GetTextData();
    int len        = textobj->GetLength();
    char *txt      = (char *)malloc(len + 1);
    if(txt == NULL){
	Tcl_SetResult(interp, "out of memory", TCL_STATIC);
	return TCL_ERROR;
    }
    myItem->GetText((vchar *)txt, len + 1);
    txt[len] = 0;
    Tcl_SetResult(interp, txt, TCL_VOLATILE);
    free(txt);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdInitDrawing (ClientData,
		       Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdInitDrawing);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[2])

    myItem->SetClickCmd(argv[3]);
    if(argc > 4)
	myItem->SetMoveCmd(argv[4]);
    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdDrawingAddItem(ClientData,
			 Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingAddItem);

//  if (argc < 4) {
//	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
//	return TCL_ERROR;
//  }

    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1]);
    int number = myItem->AddItem(argv[2]);
    gString result;
    result.sprintf((vchar *)"%d", number);
    Tcl_SetResult(interp, (char *)result, TCL_VOLATILE);
    
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingRemoveItem(ClientData,
			    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingAddItem);

    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1]);
    myItem->RemoveItem(atoi(argv[2]));

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingClear(ClientData,
		       Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingAddItem);

    if (argc < 2) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1]);
    myItem->Clear();

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSetGrid(ClientData,
			 Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingSetGrid);

    if (argc < 5) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
      if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1])
    int w = atoi(argv[2]);
    int h = atoi(argv[3]);
    myItem->SetGrid(w, h, argv[4]);

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSetGridDimensions(ClientData,
				   Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingSetGridDimensions);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
      if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1])
    int rows = atoi(argv[2]);
    int cols = atoi(argv[3]);
    myItem->SetGridDimensions(rows, cols);

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSetIconTitle(ClientData,
			      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingSetIconTitle);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
      if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1])
    int index   = atoi(argv[2]);
    char *title = argv[3];
    myItem->SetIconTitle(index, title);

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSetIconTip(ClientData,
			    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingSetIconTip);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
      if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1])
    int index   = atoi(argv[2]);
    char *title = argv[3];
    myItem->SetIconTip(index, title);

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSetDragProhibited(ClientData,
				   Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingAddItem);

    if (argc != 3) {
      Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
      return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1])
    myItem->SetDragProhibited(atoi(argv[2]));

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSelectItem(ClientData,
			    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingSelectItem);

    if (argc != 4) {
      Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
      return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
        TCL_EXECERR1 ("drawing area not found", argv[1])
    myItem->SelectItem(atoi(argv[2]), atoi(argv[3]));

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdDrawingSetSingleSelection(ClientData,
				    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdDrawingSetSingleSelection);

    if (argc != 3) {
      Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
      return TCL_ERROR;
    }
    gdDrawingArea *myItem = (gdDrawingArea *)dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem) TCL_EXECERR1 ("drawing area not found", argv[1])
    myItem->SetSingleSelection(atoi(argv[2]));

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewSetData(ClientData,
			  Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetListRow);

    if (argc < 5) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    int row       = atoi(argv[2]);
    int col       = atoi(argv[3]);
    int read_only = 0;
    if(argc > 5)
	if(argv[5][0] == 'R' || argv[5][0] == 'r')
	    read_only = 1;
    char *data = NULL;
    if(argc > 6)
	data = argv[6];
    item->SetDataCell(row, col, argv[4], read_only, data);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewRemoveRow(ClientData,
			                Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetListRow);

    if (argc < 3) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item) TCL_EXECERR1 ("listview not found", argv[1]);
    int row = atoi(argv[2]);
    item->RemoveAllRow(row);
    return TCL_OK;
}

//-----------------------------------------------------------------------------
int dis_gdListViewFlush(ClientData,
			                Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetListRow);

    if (argc < 2) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item) TCL_EXECERR1 ("listview not found", argv[1]);
    item->Flush();
    return TCL_OK;
}


// Call only on existing cell, the arguments are :
// list,row,column, callback  <state, CheckTitle, UncheckTitle>
int dis_gdListViewAddCheckCell(ClientData,
							   Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdAddCheckCell);

    if (argc < 5) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item) TCL_EXECERR1 ("listview not found", argv[1]);
    int row       = atoi(argv[2]);
    int col       = atoi(argv[3]);

    char *data = NULL;
	data = argv[4];
	
	int state=1;
	if(argc>5) state = atoi(argv[5]);

    gdListCheck* check=item->SetCellCheck(row,col,data,state);
	if(check!=NULL) {
	   if(argc>6) {
	      gString titleOn=argv[6];
	      check->SetTitle(titleOn);
	   }
	   if(argc>7) {
	      gString titleOff=argv[7];
		  check->SetAlternateTitle(titleOff);
	   }
	}
	return TCL_OK;
}
//-----------------------------------------------------------------------------


// Call to modify images for the checkbutton, the arguments are :
//  list, row, col, On_Resource_Path, Off_Resource_Path
int dis_gdListViewChangeCheckIcons (ClientData,
							        Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdChangeCheckCell);

    if (argc < 6) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item) TCL_EXECERR1 ("listview not found", argv[1]);
    int row       = atoi(argv[2]);
    int col       = atoi(argv[3]);
  	ListViewCell *cell = item->GetCellInfo(row, col);
	if(cell!=NULL) 
		if(cell->check_button!=NULL)
	   cell->check_button->SetImageResources(argv[4], argv[5]);

	return TCL_OK;
}
//-----------------------------------------------------------------------------

//  Call to set and update checkbox status
int dis_gdListViewChangeCheckStatus (ClientData,
							        Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdChangeCheckStatus);

    if (argc != 5) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item) TCL_EXECERR1 ("listview not found", argv[1]);
    int row       = atoi(argv[2]);
    int col       = atoi(argv[3]);
  	ListViewCell *cell = item->GetCellInfo(row, col);
	if(cell!=NULL) {
		if(cell->check_button!=NULL) {
		   cell->check_button->SetValueAndRedraw(atoi(argv[4]));
	    }
		item->SetFocus(row,col);
	}
	return TCL_OK;
}
//-----------------------------------------------------------------------------

int dis_gdListViewGetCellData(ClientData,
			      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdListViewGetCellData)

    if (argc != 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    int row       = atoi(argv[2]);
    int col       = atoi(argv[3]);
    char *val     = (char *)item->GetCellData(row, col);
    if(val == NULL)
        val = "";
    Tcl_AppendResult(interp, val, NULL);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewGetCellValue(ClientData,
			       Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdListViewGetCellValue);

    if (argc != 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    int row       = atoi(argv[2]);
    int col       = atoi(argv[3]);
    char *val     = (char *)item->GetCellValue(row, col);
    if(val == NULL)
        val = "";
    Tcl_AppendResult(interp, val, NULL);
    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdListViewSetCellMenu(ClientData,
			      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetCellMenu);

    if (argc != 6) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    
    int row = atoi(argv[2]);
    int col = atoi(argv[3]);
    int  largc;
    char **largv;
    if (Tcl_SplitList(interp, argv[5], &largc, &largv) == TCL_ERROR) {
	return TCL_ERROR;
    }
    item->SetCellMenu(row, col, argv[4], largc, largv);
    ckfree((char *)largv);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

// this function is inserted callback function name into the gdListView
// this function will be called every time the editor changed the cell value
int dis_gdListViewSetChangeCallback(ClientData,
				    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetChangeCallback);

    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    item->SetChangeCallback(argv[2]);
    return TCL_OK;
}

// this function is inserted callback function name into the gdListView
// this function will be called every time the current row or column changed.
int dis_gdListViewSetCellChange (ClientData,
				    Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetChangeCallback);

    if (argc != 3) {
	   Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	   return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item) TCL_EXECERR1 ("listview not found", argv[1]);
    item->SetCellChangeCallback(argv[2]);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewSetDblClickCallback(ClientData,
				      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdSetDblClickCallback);

    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    item->SetDblClickCallback(argv[2]);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewUseInternalData(ClientData,
				  Tcl_Interp* interp, int argc, char* argv[])
{
    if (argc != 2) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdListView *item = (gdListView *)dis_findItem(interp, (vchar*) argv[1]);
    if(!item)
        TCL_EXECERR1 ("listview not found", argv[1]);
    item->CreateInternalList();
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdInitButton (ClientData,
		       Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdInitButton);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdButton* myItem = (gdButton*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem)
        TCL_EXECERR1 ("button not found", argv[2])
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdInitRuler (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdInitButton);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdRuler* myItem = (gdRuler*) dis_findItem(interp, (vchar*) argv[2]);
    if (!myItem) TCL_EXECERR1 ("Ruler not found", argv[2])

    gdListView* myView = (gdListView*) dis_findItem(interp, (vchar*) argv[3]);
    myItem->setListView (myView);

    myItem->SetTclCmd((vchar*)argv[4]);

    return TCL_OK;
  }

//-----------------------------------------------------------------------------

int dis_gdInitNotebookPage (ClientData,
		       Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdInitNotbookPage);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdNotebookPage* myItem = (gdNotebookPage*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem)
        TCL_EXECERR1 ("button not found", argv[2])
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
  }

//-----------------------------------------------------------------------------

int dis_activate_notebook_page (ClientData,
		       Tcl_Interp* interp, int argc, char* argv[])
{
  gInit (dis_activate_notebook_page);
  int nResult = TCL_ERROR;
  if (argc >= 3) 
  {
    vnotebook *book = vnotebook::CastDown((vdialogItem *)dis_findItem(interp, (vchar*) argv[1]));
    if( book ) 
    {
      vnotebookPage *thePage = book->FindPage (vnameInternGlobal((const vchar *)argv[2]));
      if (thePage)
      {
        thePage->SetCurrent();
        nResult = TCL_OK;
      }
    }
  }
  else if( argc==2 ) //only page tag is supplied
  {
    vnotebookPage *thePage = vnotebookPage::CastDown(
                  (vdialogItem *)dis_findItem(interp, (vchar*) argv[1]));
    if (thePage)
    {
      thePage->SetCurrent();
      nResult = TCL_OK;
    }
  }
  else
    Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
  return nResult;
}




int dis_gdInitSash (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    gInit (dis_gdInitSash);

    if (argc != 7) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdSash* myItem = NULL;
    char* gd = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);

    if (gd) {
        int gdialog_id = atoi (gd);
      
        if (gdialog_id > 0) {
	    GDialog* gd = GDialog::FindGDialog (gdialog_id);
	    if (!gd) return TCL_ERROR;
	
	    myItem = (gdSash*)gd->FindItem(vnameInternGlobal((const vchar*)argv[2]));
            if (strcmp (argv[3], "NULL") == 0) myItem->SetPrev (NULL);
            else myItem->SetPrev ((gdSash*)gd->FindItem(vnameInternGlobal((const vchar *)argv[3])));
            
            if (strcmp (argv[4], "NULL") == 0) myItem->SetNext (NULL);
            else myItem->SetNext ((gdSash*)gd->FindItem(vnameInternGlobal((const vchar *)argv[4])));

            myItem->SetLeft ((vdialogItem *)gd->FindItem (vnameInternGlobal((const vchar *)argv[5])));
            myItem->SetRight ((vdialogItem *)gd->FindItem (vnameInternGlobal((const vchar *)argv[6])));
        } 

        else {
	    Viewer* vr;
	    View *v;
	    Viewer::GetExecutingView (vr, v);
	    if (vr && v) {
	        myItem = (gdSash*)v->FindItem ((vchar*)argv[2]);
			if (myItem) {
                myItem->SetPrev ((gdSash*)v->FindItem((vchar*)argv[3]));
                myItem->SetNext ((gdSash*)v->FindItem((vchar*)argv[4]));

                myItem->SetLeft ((vdialogItem *)v->FindItem ((vchar*)argv[5]));
                myItem->SetRight ((vdialogItem *)v->FindItem ((vchar*)argv[6]));
			}
	    }
        }       
    }
    
    if (!myItem) TCL_EXECERR1 ("sash not found", argv[2])
 
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdInitSlider (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdInitSlider);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdSlider* myItem = (gdSlider*) dis_findItem(interp, (vchar*) argv[2]);

    if (!myItem) TCL_EXECERR1 ("button not found", argv[2])
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdInitListItem (ClientData,
		       Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdInitListItem);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdListItem* myItem = (gdListItem*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem) {
      Tcl_SetResult (interp, "List item not found", TCL_STATIC);
      return TCL_ERROR;
    }
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
  }


//-----------------------------------------------------------------------------

int dis_gdInitListView (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdInitListView);

    if (argc != 5) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdListView* myItem = (gdListView*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem) {
      Tcl_SetResult (interp, "List item not found", TCL_STATIC);
      return TCL_ERROR;
    }
    
    int columns = atoi (argv[3]);
    myItem->setColumns (columns);
    myItem->SetColumnWidth (vlistviewALL_COLUMNS, (int) (myItem->GetWidth() / columns));

    myItem->SetTclCmd((vchar*)argv[4]);

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewRefresh (ClientData, Tcl_Interp* interp, int, char *argv[]) {

    gdListView* myItem = NULL;

    myItem = (gdListView*) dis_findItem(interp, (vchar*)argv[1]);
    if ((myItem == NULL) && interp_errTrace)
	dis_message((vchar*)"@dis_gdListViewRefresh: Did NOT Find %s\n", argv[1]);
    if (myItem) {
        myItem->Inval(vwindowINVAL_IMMEDIATE);
                if(myItem->GetList()->GetRowCount()>0)
		   myItem->ScrollFocusIntoView();
	}

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_gdListViewAddPopupItem (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    gdListView* myItem = NULL;

    if(argc != 4){
	Tcl_AppendResult(interp, "usage: ", argv[0], " <list> <title> <callback>", NULL);
	return TCL_ERROR;
    }
    myItem = (gdListView*) dis_findItem(interp, (vchar*)argv[1]);
    if ((myItem == NULL) && interp_errTrace)
	dis_message((vchar*)"@dis_gdListViewAddPopupItem: Did NOT Find %s\n", argv[1]);
    if (myItem) {
	myItem->AddPopupItem(argv[2], argv[3]);
    }

    return TCL_OK;
}

//-----------------------------------------------------------------------------


int dis_gdInitProgressItem (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_gdInitProgressItem);

    if (argc < 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdProgressItem* myItem = (gdProgressItem*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem) {
	Tcl_SetResult (interp, "Progress item not found", TCL_STATIC);
	return TCL_ERROR;
    }
    
    if(argc > 3)
	myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
}

// dis_gdNumberSpinner_SetMax tag value
int dis_gdNumberSpinner_SetMax(ClientData, Tcl_Interp * interp, int argc, char * argv[])
{
	int nRet=TCL_ERROR;
	
	if (argc>=3)
	{
		
		gdNumberSpinner* myItem= (gdNumberSpinner *) dis_findItem(interp, (vchar*) argv[2]  );
		if (!myItem) TCL_EXECERR1("Item for dialog not found : ", argv[2]);
		nRet=TCL_OK;
		myItem->SetMax(atoi(argv[3]));
	}
	return nRet;
}
// dis_gdNumberSpinner_SetMax tag value
int dis_gdNumberSpinner_SetMin(ClientData, Tcl_Interp * interp, int argc, char * argv[])
{
	int nRet=TCL_ERROR;
	
	if (argc>=3)
	{
		
		gdNumberSpinner* myItem= (gdNumberSpinner *) dis_findItem(interp, (vchar*) argv[2]  );
		if (!myItem) TCL_EXECERR1("Item for dialog not found : ", argv[2]);
		nRet=TCL_OK;
		myItem->SetMin(atoi(argv[3]));
	}
	return nRet;
}


//----------------------------------------------------------------------------
//dis_gdInitNumberSpinner $gd tagname TclCallback

// Important warning:  do not put anything which could possibly call veventStopProcessing
// in the tcl callback--if you do the spinner will increment itself forever...


int dis_gdInitNumberSpinner(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
	gInit(dis_gdInitNumberSpinner);
	
	int nRet=TCL_ERROR;
	if (argc>=4)
	{
		nRet=TCL_OK;
		gdNumberSpinner* myItem= (gdNumberSpinner *) dis_findItem(interp, (vchar*) argv[2]  );
		
		if (!myItem) TCL_EXECERR1("Item for dialog not found : ", argv[2]);
		
		myItem->SetTclCmd((vchar*)argv[3]);
		/*
		
		  //have to save this here because for some reason setting min also changes value...
		  int nCur=myItem->GetValue();
		  //allow for overide of values stored in resource file
		  if (argc>=5)
		  {
		  int curMax=myItem->GetMax();
		  int curMin=myItem->GetMin();
		  int nMin=curMin;
		  //allows for a "" argument to just set the max
		  if ( argv[4] && strcmp(argv[4], "")  )
		  {
		  nMin=atoi(argv[4]);
		  }
		  
			int nMax= argc>=6? atoi(argv[5]): curMax;
			
			  if (nMin<nMax)
			  {
			  myItem->SetMin(nMin);
			  myItem->SetMax(nMax);
			  
				if (nCur<nMin|| nCur>nMax)
				{
				//vchar buffer[12];
				//sprintf((char *)buffer,"%d", nMin);
				//myItem->SetInterpVariable( (vchar *) myItem->GetTag(), (vchar *) (char *) buffer);
				myItem->SetValue(nMin);
				//myItem->Inval(vwindowINVAL_IMMEDIATE);
				}
				}
				
	}*/
		
	}
	return nRet;
	
}

//-----------------------------------------------------------------------------

int dis_gdInitToggle (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_close_dialog);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdToggle* myItem = (gdToggle*) dis_findItem(interp, (vchar*) argv[2]);

 

    myItem->SetTclCmd((vchar*)argv[3]);

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdInitTextItem (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdInitTextItem);

    if (argc < 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem) TCL_EXECERR1 ("TextItem not found", argv[2]);
    
    if (argc == 4) myItem->SetTclCmd((vchar*)argv[3]);
    if (argc == 5) myItem->SetUpdateCmd((vchar*)argv[4]);

    return TCL_OK;
}


//-----------------------------------------------------------------------------


int dis_gdInitExclusiveGroup (ClientData, Tcl_Interp* interp, int argc, char* argv[]) 
{
    gInit (dis_close_dialog);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdExclusiveGroup* myItem = (gdExclusiveGroup*) dis_findItem(interp, (vchar*) argv[2]);
    
    if (!myItem) {
      Tcl_SetResult (interp, "Exclusive Group item not found", TCL_STATIC);
      return TCL_ERROR;
    }
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
}




//-----------------------------------------------------------------------------

int dis_gdInitComboBox (ClientData,
			 Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_close_dialog);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdComboBox* myItem = (gdComboBox*) dis_findItem(interp, (vchar*) argv[2]); 
    
    if (!myItem) {
      Tcl_SetResult (interp, "Combobox item not found", TCL_STATIC);
      return TCL_ERROR;
    }
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdInitPopDownComboBox (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_close_dialog);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    gdPopDownComboBox* myItem = (gdPopDownComboBox*) dis_findItem(interp, (vchar*) argv[2]);

    if (!myItem) TCL_EXECERR1("Item for dialog not found : ", argv[2]);

    myItem->SetTclCmd((vchar*)argv[3]);

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdComboBoxSetMatch(ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdComboBoxSetMatch);

    if (argc != 3) {
	Tcl_AppendResult(interp, "usage: ", argv[0], " <item> <flag>", NULL);
	return TCL_ERROR;
    }

    gdPopDownComboBox* myItem = (gdPopDownComboBox*) dis_findItem(interp, (vchar*) argv[1]);

    if (!myItem) TCL_EXECERR1("Item for dialog not found : ", argv[1]);

    myItem->SetMatchBehavior(atoi(argv[2]));

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_gdInitLabel (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_gdInitLabel);

    if (argc < 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    char* gd = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);
    gdLabel* myItem = (gdLabel*) dis_findItem( interp, (vchar*)argv[2]);

    if (!myItem)
        TCL_EXECERR1 ("Label not found", argv[2])
    
    myItem->SetTclCmd((vchar*)argv[3]);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_ItemEnable (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
    gInit (dis_ItemEnable);
    
    if (argc != 3 && argc != 5) TCL_EXECERR ("wrong number of arguments")

    vdialogItem* myItem;
    const char* myValue = NULL;
	    
    if (strcmp (argv[1], "-layer") == 0) {
	Layer* layer = Layer::find_layer (argv[2]);
	if (layer && (argc == 5)) {
            View* v = layer->getView ();
	    if (v) {
	        myItem = (vdialogItem*)v->FindItem ((vchar*)argv[3]);
	        myValue = argv[4];
	    }
        } else
            return TCL_ERROR;

    } else {
        myItem = (vdialogItem*) dis_findItem(interp, (vchar*) argv[1]);
        if (myItem)
            myValue = argv[2];
    }
    
    if (!myItem)
        TCL_EXECERR1 ("Item not found", argv[2])
	    
    if ( *myValue == '0') myItem->Disable();
    else myItem->Enable();
    
    return TCL_OK;
}


int dis_ItemName (ClientData,
		      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_ItemEnable);
    
    if (argc != 3 && argc != 5)
        TCL_EXECERR ("wrong number of arguments")

    vdialogItem* myItem;
    const char* myValue = NULL;
	    
    if (strcmp (argv[1], "-layer") == 0) {
	Layer* layer = Layer::find_layer (argv[2]);
	if (layer && (argc == 5)) {
            View* v = layer->getView ();
	    if (v) {
	        myItem = (vdialogItem*)v->FindItem ((vchar*)argv[3]);
	        myValue = argv[4];
	    }
        } else
            return TCL_ERROR;

    } else {
        myItem = (vdialogItem*) dis_findItem(interp, (vchar*) argv[1]);
        if (myItem) myValue = argv[2];
    } 

    if (!myItem)
        TCL_EXECERR1 ("button not found", argv[2])
	    
    if (myItem->IsKindOf (&vbutton::Kind)) {
        vbutton* b = vbutton::CastDown(myItem);
        b->SetTitle((const vchar*)myValue);
        b->Inval(vwindowINVAL_IMMEDIATE);
    } 
    return TCL_OK;
}


int dis_print (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {

    gInit (dis_print);

    if (argc < 2) {
	Tcl_AppendResult (interp, argv[1], ": wrong number of arguments", NULL);
	return TCL_ERROR;
    }

    if (interp_errTrace) dis_message((vchar*)"tcl message: %s\n", argv[1]);
    return TCL_OK;
}

static int execute_in_directory(char *command, char *path)
{
    vstr    *dirEntry;
    vfsDir  *openDir;
    vfsPath vpath;

    vpath.Set ((vchar*)path);
    vexWITH_HANDLING {
	openDir = vfsDir::Open (&vpath);
    } vexON_EXCEPTION {
        openDir = NULL;
    } vexEND_HANDLING;
    if (openDir == (vfsDir *) NULL) 
	return -1;
    
    vpath.SetWorkingDir();
#ifndef WIN32
    vsystem((const char* )command);
#endif

    vstr *thisDir   = vstrCloneScribed (vcharScribeLiteral("."));
    vstr *parentDir = vstrCloneScribed (vcharScribeLiteral(".."));
    while ((dirEntry = openDir->Read()) != (vchar*) NULL) {
        if (vcharCompare (dirEntry, thisDir) == 0) {
            vstrDestroy (dirEntry);
            continue;
        } 
	if (vcharCompare (dirEntry, parentDir) == 0) {
            vstrDestroy (dirEntry);
            continue;
        }

	vfsInfo fileinfo;
	vfsPath vpath;
	int retValue = 0;

	vpath.Set ((vchar*)path);
	vpath.Append (dirEntry, NULL);
	vexWITH_HANDLING {
	    vpath.GetFileInfo (&fileinfo);
	} vexON_EXCEPTION {
	    retValue = 1;
	} vexEND_HANDLING;
	if (retValue != 0) 
	    continue;

	if (fileinfo.IsDirectory() == TRUE) {
	    vstr *fullPath = vpath.CloneString ();
	    execute_in_directory(command, (char *)fullPath);
	    vstrDestroy (fullPath);
	}

        vstrDestroy (dirEntry);
    }

    openDir->Close();
    vstrDestroy (parentDir);
    vstrDestroy (thisDir);
    
    return 1;
}

int dis_ExecuteInSubDirs(ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit(dis_ExecuteInSubDirs);
    
    if(argc != 3)
	TCL_EXECERR ("wrong number of arguments");
    char *root_dir = argv[1];
    char *command  = argv[2];

    vfsPath *working_dir = vfsPath::CloneWorkingDir();
    if(execute_in_directory(command, root_dir) < 0){
	working_dir->SetWorkingDir();
	delete working_dir;
	TCL_EXECERR("specified path invalid");
    }
    working_dir->SetWorkingDir();
    delete working_dir;
    return TCL_OK;
}    

int dis_KillSystemCommand (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_KillSystemCommand);

    if (argc != 2 && argc != 3)
	TCL_EXECERR ("wrong number of arguments");

    int pid = atoi(argv[1]);
    int sig = SIGKILL;
    if (argc == 3) sig = atoi(argv[2]);
    
#ifndef WIN32
    if (pid) vpkill (pid, sig);
#endif

    return TCL_OK;
}

void external_process_end_handler(void)
{
    fflush(stdout);
    if(process_end_script)
	gala_eval(process_interpreter, (char *)process_end_script);
}


void external_process_end_handler_wait(void)
{
    waiting_for_process--;
}


//
// Local Eval - evaluates the command in the specified LOCAL service
// Note:  This is a cludge to allow UI in the pset_server executable and still
//        call entry-point functions previously only allowed from the client.
//

int leval (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{

    if (argc != 3) TCL_EXECERR ("wrong number of arguments");

    vstr *result;
    result = server_eval ( 	(vstr*) argv[1], 
				(vstr *)argv[2]);

    Tcl_SetResult (interp, (char*)(vchar*)result, TCL_VOLATILE);

    return TCL_OK;

}


int reval_async (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{

    if (argc != 3 && argc != 4) TCL_EXECERR ("wrong number of arguments");

    if (argc == 3) 
	remote_call_async (Application::findApplication("DISmain"), 
				    (vstr*) argv[1], (vstr *)argv[2]);
    else if (argc == 4) 
	remote_call_async (Application::findApplication(argv[1]), 
				    (vstr*) argv[2], (vstr *)argv[3]);  // weird difference
  
    return TCL_OK;
}
 
  
int reval (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{

    if (argc != 3 && argc != 4) TCL_EXECERR ("wrong number of arguments");

    vstr *result;
    if (argc == 3) 
	result = rcall_server_eval (Application::findApplication("DISmain"), 
				    (vstr*) argv[1], (vstr *)argv[2]);
    else if (argc == 4) {

	    result = rcall_server_eval (Application::findApplication(argv[1]), 
		             	    	    (vstr*) argv[2], (vstr *)argv[3]);  
	}

    Tcl_SetResult (interp, (char*)(vchar*)result, TCL_VOLATILE);

    return TCL_OK;
}



int dis_SystemCommand (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_SystemCommand);
    
    if (argc < 3)
	TCL_EXECERR ("wrong number of arguments");
    
    notifier_function *handler;
    if(process_end_script)
	vstrDestroy(process_end_script);
    if(argc == 4){
	process_end_script  = vstrCloneScribed (vcharScribeSystem (argv[3])); 
	process_interpreter = interp;
	handler             = external_process_end_handler;
    } else {
		process_end_script  = NULL;
		process_interpreter = NULL;
		handler             = NULL;
    }

    FILE* file;
    if (argv[2]) 
	file = vpopen_tmp((vchar*)argv[1], (vchar*)">", (vchar*)argv[2], handler);
    else
        file = vpopen_tmp((vchar*)argv[1], (vchar*)"R", (vchar*)argv[2], handler);

    if (!file)
	    TCL_EXECERR1 ("unable to execute", argv[1])
    else {
	int pid = vp_find_pid (file);
	gString tmp;
	tmp.sprintf((vchar *)"%d", pid);
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
        return TCL_OK;
    }
}

int dis_SystemCommandWait (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_SystemCommand);
    
    if (argc != 3)
	    TCL_EXECERR ("wrong number of arguments");
    
    waiting_for_process++;

    FILE* file;
    if (argv[2]) 
	file = vpopen_tmp((vchar*)argv[1], (vchar*)">", (vchar*)argv[2], external_process_end_handler);
    else
        file = vpopen_tmp((vchar*)argv[1], (vchar*)"R", (vchar*)argv[2], external_process_end_handler);

    if (!file){
	waiting_for_process--;
	TCL_EXECERR1 ("unable to execute", argv[1]);
    } else {
#ifndef WIN32
        while (waiting_for_process) OSapi_sleep(1);
#endif
	gString tmp;
	tmp.sprintf((vchar *)"%d",  vp_find_pid (file));
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
        return TCL_OK;
    }
}


int dis_GalaLogFile(ClientData, Tcl_Interp* interp, int , char *[])
{
    gInit(dis_GalaLogFile);
#ifndef _WIN32
    Tcl_SetResult (interp, "gala.log", TCL_STATIC);
#else
    //Because galaxy application doesn't have stdout on Windows,
    //we will not redirect it
    Tcl_SetResult (interp, "nul", TCL_STATIC);
#endif
    return TCL_OK;
}


int dis_DevNull(ClientData, Tcl_Interp* interp, int , char *[])
{
    gInit(dis_DevNull);
    
#ifndef WIN32
    Tcl_SetResult (interp, "/dev/null", TCL_STATIC);
#else
    Tcl_SetResult (interp, "nul", TCL_STATIC);
#endif

    return TCL_OK;
}

    

int dis_ViewHighliteLine(ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_ViewHighliteLine);

    if ((argc < 3) || (argc > 4))
	TCL_EXECERR ("wrong number of arguments")

    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[1]);
    
    if (!myItem){
        TCL_EXECERR1 ("TextItem not found", argv[1]);
    } else
	myItem->HighliteLine(atoi(argv[2]));

    return TCL_OK;
}

int dis_AddViewFileScanner (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_AddViewFileScanner);

    if (argc != 4)
	    TCL_EXECERR ("wrong number of arguments")

    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[1]);
    
    if (!myItem) {
        TCL_EXECERR1 ("TextItem not found", argv[1])
    } else {
		myItem->AddScanner ((vchar*)argv[2], (vchar*)argv[3]);
	}
    return TCL_OK;
}

int dis_ViewFile (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_ViewFile);

    if (argc < 4)
	TCL_EXECERR ("wrong number of arguments")

    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[1]);
    
    if (!myItem)
        TCL_EXECERR1 ("TextItem not found", argv[1])
   else {
	if (argc == 4)
	    myItem->ShowFile((vchar*)argv[2], atoi(argv[3]));
        else
	    myItem->ShowFile((vchar*)argv[2], TRUE);
    }

    return TCL_OK;
}

int dis_BlockSignals(ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
 #ifndef WIN32
    gInit(dis_BlockSignals);
    static int blocked  = FALSE;
    static sigset_t old_mask;
    sigset_t mask;

    sigemptyset (&mask);
    sigaddset (&mask, SIGCHLD);
    
    if(argc < 2)
	TCL_EXECERR ("wrong number of arguments");
    int block = atoi(argv[1]);
    if(block){
	if(blocked)
	    return TCL_OK;
	sigprocmask(SIG_BLOCK, &mask, &old_mask);
	blocked  = TRUE;
    } else {
	if(!blocked)
	    return TCL_OK;
	blocked = FALSE;
	sigprocmask(SIG_SETMASK, &old_mask, NULL);
    }
	#endif
    return TCL_OK;
}

int dis_StopViewUpdating(ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit(dis_StopViewUpdating);

    if (argc < 2)
	TCL_EXECERR ("wrong number of arguments")

    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[1]);
    
    if (!myItem){
        TCL_EXECERR1 ("TextItem not found", argv[1]);
    }
    else
	myItem->StopUpdating();

    return TCL_OK;
}



int dis_gdListItemRefresh (ClientData, Tcl_Interp* interp, int, char *argv[]) {
    gdListItem* myItem = NULL;

    myItem = (gdListItem*)GDialog::FindListItem (argv[1]);
    if (myItem) { // Found it.
        // Find list item will iterate for others of the same name.
        while (myItem) {
            myItem->Inval(vwindowINVAL_IMMEDIATE); 
            myItem = (gdListItem*)GDialog::FindListItem (NULL);
        }

    // TLW - Added this step to TRY to get it to work with Dialog Layers.
    // NOTE - It only looks for a single instance of the given name.
    } else {    // Not in a GDialog, try using the view.
        myItem = (gdListItem*) dis_findItem(interp, (vchar*)argv[1]);
        if ((myItem == NULL) && interp_errTrace)
            dis_message((vchar*)"@dis_gdListItemRefresh: Did NOT Find %s\n",
                        argv[1]);
    }       

    if (myItem) 
        myItem->Inval(vwindowINVAL_IMMEDIATE); 

    return TCL_OK;
}


// Select a single entry.
int dis_gdListItemSelect (ClientData, Tcl_Interp* interp,
			  int argc, char** argv)
{
	if (argc < 3)
		TCL_EXECERR ("wrong number of arguments");

	gdListItem* myItem = (gdListItem*) dis_findItem(interp, (vchar*) argv[1]);
	if (!myItem)
		TCL_EXECERR ("List item not found");
    
	int index = atoi(argv[2]);
	if ((index >= 0) && (index < myItem->GetList()->GetRowCount()) )
		myItem->SelectItem(atoi(argv[2]));
	else
		TCL_EXECERR("index is out of bounds");

	return TCL_OK;
}



// Select all entries in the given selection.
int dis_gdListItemSelection (ClientData, Tcl_Interp* interp,
			     int argc, char** argv)
{
    if (argc < 3)
	TCL_EXECERR ("wrong number of arguments");

    gdListItem* myItem = (gdListItem*) dis_findItem(interp, (vchar*) argv[1]);
    if (!myItem)
	TCL_EXECERR ("List item not found");

    // Get the list and listSelector.
    vlist* list = myItem->GetList();
    vlistSelection* sel = myItem->GetSelection();

    // Unselect all current selections.
    list->UnselectAll(sel);
    
    // Tell list item to ignore selections until we are done.
    myItem->AllowSelection(vFALSE);

    // Select all items in the selection.
    SelectionIterator newSel((vchar*)argv[2]);
    while (newSel.Next()) 
        list->SelectRow(sel, newSel.GetNext() );
    
    // Tell list item to handle selections again.
    myItem->AllowSelection(vTRUE);

    return TCL_OK;
}
    

/*-------------------------------------------------------*/

int dis_gdListViewSelect (ClientData, Tcl_Interp* interp, int argc, char** argv) {

	if (argc < 3) TCL_EXECERR ("wrong number of arguments");

	gdListView* myItem = (gdListView*) dis_findItem(interp, (vchar*) argv[1]);
	if (!myItem) TCL_EXECERR ("List item not found");
    
	int index = atoi(argv[2]);
	vlist *l = myItem->GetList();
	if ((index >= 0) && (index < l->GetRowCount()) )
		l->SelectRow (myItem->GetSelection(), index);
	else
		TCL_EXECERR("index is out of bounds");

	return TCL_OK;
}
    
/*-------------------------------------------------------*/

int dis_gdDialogEval (ClientData, Tcl_Interp*, int, char *argv[]) {

    GDialog* myDialog = NULL;
    myDialog = GDialog::FindGDialog (argv[1]);
    gala_eval_generic (myDialog->GetInterpreter(), argv[2], FALSE);
    return (TCL_OK);
}


int dis_gdDialogIDEval (ClientData, Tcl_Interp* interp, int, char *argv[]) {

    int id = atoi (argv[1]);
    
    GDialog* gd = GDialog::FindGDialog (id);

    if (gd) {
	gala_eval_generic (gd->GetInterpreter(), argv[2], FALSE);
	Tcl_SetResult(interp, Tcl_GetStringResult(gd->GetInterpreter()), TCL_VOLATILE);
	return TCL_OK;
    }

    else {
	Viewer* vr = Viewer::GetFirstViewer ();
	View* v = vr->find_view(id);
        while (!v && (vr = vr->GetNextViewer ())) {
            v = vr->find_view(id);
        }

	if (!v) return TCL_ERROR;
	gala_eval_generic (v->interp(), argv[2], FALSE);
	Tcl_SetResult(interp, Tcl_GetStringResult(v->interp()), TCL_VOLATILE);
	return TCL_OK;
    }
}


int dis_gdDialogSetTitle (ClientData, Tcl_Interp* interp, int, char *argv[]) {
	char* idStr = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);
	if (!idStr) return TCL_ERROR;

	int id = atoi (idStr);
	if (!id) return TCL_ERROR;

	GDialog* gd = GDialog::FindGDialog (id);
	if (!gd) return TCL_ERROR;
	
	gd->SetTitle ((vchar*) argv[1]);
    return TCL_OK;
}

int dis_ViewText (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_ViewText);

    if (argc < 3)
	TCL_EXECERR ("wrong number of arguments")

    // Note - This call may change interp.
    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[1]);

    // If we don't have an item, it is an error.
    if (myItem == NULL )
        TCL_EXECERR1 ("button not found", argv[1])


    // Got the item, now do the processing.
    else if (strcmp (argv[2], "-append") == 0) {
        vstr *theText = myItem->GetTextAsString();
        theText = vstrAppend (theText, (vchar*)argv[3]);
        myItem->SetText (theText);
        // Set the value to generate TCL callbacks.
        Tcl_SetVar(interp, argv[1], (char *) theText, TCL_GLOBAL_ONLY);
		vstrDestroy (theText);
        
    }

    else if (strcmp (argv[2], "-overwrite") == 0) {
        myItem->SetText ((vchar*)argv[3]);
        // Set the value to generate TCL callbacks.
        Tcl_SetVar(interp, argv[1], argv[3], TCL_GLOBAL_ONLY);
    }

    else if (strcmp (argv[2], "-view") == 0) {
        myItem->SetText ((vchar*)argv[3]);
        // Set the value to generate TCL callbacks.
        Tcl_SetVar(interp, argv[1], argv[3], TCL_GLOBAL_ONLY);
    }

    else if (strcmp (argv[2], "-clear") == 0) {
        myItem->SetText ((vchar*)"");
        // Set the value to generate TCL callbacks.
        Tcl_SetVar(interp, argv[1], "", TCL_GLOBAL_ONLY);
    }
   
    else {
        TCL_EXECERR1 ("Bad argument", argv[1])
    }

    return TCL_OK;
}

int dis_GetText(ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_GetText);
 
    if (argc < 2)
        TCL_EXECERR ("wrong number of arguments")
 
    // Note - This call may change interp.
    gdTextItem* myItem = (gdTextItem*) dis_findItem(interp, (vchar*) argv[1]);
 
    // If we don't have an item, it is an error.
    if (myItem == NULL ) {
        TCL_EXECERR1 ("text not found", argv[1])  
 
    // Got the item, now do the processing.
    } else  {
         vstr*  tempStr = myItem->GetTextAsString();
         char*  myText = (char*) vcharExportAlloc(vstrScribe(tempStr) );
         Tcl_SetResult (interp, myText, TCL_VOLATILE);
         vcharExportFree(myText);
    }
 
    return TCL_OK;
}

int dis_DirMakeDirectory (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_EvalFile);

    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments");

    vfsPath vpath;
    vpath.Set ((vchar*)argv[1]);
    
    
    int ok;
    vexWITH_HANDLING {
#ifndef WIN32
	vpath.MakeDir (vfsOWNER_PERMISSIONS_MASK | vfsGROUP_READ | vfsGROUP_EXECUTE |
		       vfsOTHER_READ | vfsOTHER_EXECUTE);
#endif
        ok = 1;
    } vexON_EXCEPTION {
        ok = 0;
    } vexEND_HANDLING;


    if (ok)
	return TCL_OK;  
    else
      TCL_EXECERR1 ("unable to create directory", argv[1])
	
}

int dis_GetAppDirectory (ClientData, Tcl_Interp* interp, int, char *[]) {

    vstr* appfile = vfsPath::GetAppDir()->CloneString();
    gString tmp   = (char*)(vchar*)appfile;
    for (char *ptr = (char *)tmp; *ptr; ptr++)
	if ((*ptr == '\\') && (*(ptr+1) != '\\')) *ptr = '/';
    Tcl_SetResult (interp, (char *)tmp,TCL_VOLATILE);
    vstrDestroy(appfile);
    return TCL_OK;
}

int dis_GetStartupDirectory (ClientData, Tcl_Interp* interp, int, char *[]) {

    vstr* appfile = vfsPath::GetStartupDir()->CloneString();
    gString tmp   = (char*)(vchar*)appfile;
    for (char *ptr = (char *)tmp; *ptr; ptr++)
	if ((*ptr == '\\') && (*(ptr+1) != '\\')) *ptr = '/';
    Tcl_SetResult (interp, (char*)tmp, TCL_VOLATILE);
    vstrDestroy(appfile);
    return TCL_OK;
}

int dis_Message (ClientData, Tcl_Interp*, int argc, char* argv[]) {

    for (int i=1; i<argc; i++)
        dis_message((vchar *)"%s", argv[i]);
    
    return TCL_OK;
}

int error_Message (ClientData, Tcl_Interp*, int, char *argv[]) {
	dis_message ((vchar*)argv[1]);
	return TCL_OK;
}

int dis_EvalFile (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_EvalFile);

    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments");

    return Tcl_EvalFile (interp, argv[1]);
}

int dis_SetPollProc(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
#ifndef WIN32
    if (argc != 2)
	TCL_EXECERR ("wrong number of arguments");
    if(poll_timer_proc)
	free(poll_timer_proc);
    poll_timer_proc        = strdup(argv[1]);
    poll_timer_interpreter = interp;
    poll_timer.SetRecurrent();
//    poll_timer.SetPeriod(0, 7000000000);
    poll_timer.SetPeriod(3, 0);
    poll_timer.Start();
    Tcl_SetResult (interp, "1", TCL_VOLATILE);
#endif
    return TCL_OK;
}

int dis_StopPollProc(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 1)
	TCL_EXECERR ("wrong number of arguments");
    if(poll_timer_proc){
	free(poll_timer_proc);
	poll_timer_proc = NULL;
    }
    poll_timer.Stop();
    Tcl_SetResult (interp, "1", TCL_VOLATILE);

    return TCL_OK;
}

int dis_ProgressElapsedTime (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2)
	TCL_EXECERR ("wrong number of arguments");
    
    gdProgressItem* myItem = (gdProgressItem*) dis_findItem(interp, (vchar*) argv[1]);
    
    if (!myItem)
        TCL_EXECERR1 ("Progress Item not found", argv[1])

    char tempStr[100];
    long seconds = myItem->ElapsedTime();
    int hours = (int) (seconds / 3600);
    seconds = seconds - (hours * 3600);
    int mins = (int) (seconds / 60);
    seconds = seconds - (mins * 60);

    sprintf (tempStr, "%d:%02d:%02d", hours, mins, seconds);
    Tcl_SetResult (interp, tempStr, TCL_VOLATILE);

    return TCL_OK;
}


int dis_ProgressStartTime (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    gdProgressItem* myItem = (gdProgressItem*) dis_findItem(interp, (vchar*) argv[1]);

    if (!myItem)
        TCL_EXECERR1 ("Progress Item not found", argv[1])

    char tempStr[100];
    vscribe *myScribe = myItem->StartTime()->ScribeFormatted(vdateFormat::GetTerseTime());
    myScribe->Scribble (tempStr);
    tempStr[myScribe->GetLength()] = '\0';
    Tcl_SetResult (interp, tempStr, TCL_VOLATILE);

    return TCL_OK;
}


int dis_ProgressEstimatedRelative (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    gdProgressItem* myItem = (gdProgressItem*) dis_findItem(interp, (vchar*) argv[1]);

    if (!myItem)
        TCL_EXECERR1 ("Progress Item not found", argv[1])

    char tempStr[100];
    long seconds = myItem->ProjectedRelativeTime();
    int hours = (int) (seconds / 3600);
    seconds = seconds - (hours * 3600);
    int mins = (int) (seconds / 60);
    seconds = seconds - (mins * 60);

    sprintf (tempStr, "%d:%02d:%02d", hours, mins, seconds);
    Tcl_SetResult (interp, tempStr, TCL_VOLATILE);

    return TCL_OK;
}


int dis_ProgressEstimatedEnd (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    gdProgressItem* myItem = (gdProgressItem*) dis_findItem(interp, (vchar*) argv[1]);

    if (!myItem)
        TCL_EXECERR1 ("Progress Item not found", argv[1])

    char tempStr[100];
    vscribe *myScribe = myItem->ProjectedEndTime()->ScribeFormatted(vdateFormat::GetTerseTime());
    myScribe->Scribble (tempStr);
    tempStr[myScribe->GetLength()] = '\0';
    Tcl_SetResult (interp, tempStr, TCL_VOLATILE);

    return TCL_OK;
}

	
int dis_ProgressStart (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    gdProgressItem* myItem = (gdProgressItem*) dis_findItem(interp, (vchar*) argv[1]);

    if (!myItem)
        TCL_EXECERR1 ("Progress Item not found", argv[1])

    myItem->StartTime()->SetCurrent();

    return TCL_OK;
}

int dis_help (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc < 3 || argc > 4) 
	TCL_EXECERR ("wrong number of arguments")

    char* file = "DISCOVER_HELP.hlp";
    if (argc >= 4) file = argv[3];

    if (strcmp(argv[1],"-helponhelp") == 0)
	WinHelp (NULL, file, HELP_HELPONHELP, 0);
    else if (strcmp(argv[1], "-contents")==0)
	WinHelp (NULL, file, HELP_FINDER, 0);
    else
	WinHelp (NULL, file, HELP_CONTEXT, atoi(argv[2]));
    return TCL_OK;
}


int dis_eval (ClientData, Tcl_Interp*, int, char *[]) {
    return TCL_OK;
}


//--------------------------------------------------------------------------------
// dis_getTextStrRes - gets the value of a text str resource.
// 
// argv[1] is the name of the resource to get.
//
// returns TCL_OK or TCL_ERROR.
//--------------------------------------------------------------------------------

int dis_getTextStrRes (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {

    if (argc != 2) TCL_EXECERR ("wrong number of arguments")

    if (!argv[1] || !argv[0] ) TCL_EXECERR ("bad arguments")

    vresource res;
    if (!getResource (argv[1], &res)) TCL_EXECERR ("no resource found")

    vtext *string = new vtext (res);

    const int TEMPINCR = 1024;
    char *theString;
    size_t stringSize = TEMPINCR;
    theString = (char*)malloc (sizeof(char) * stringSize);
    theString[0] = '\0';

    char temp[TEMPINCR];

    for (long i = 1; i <= string->GetLineCount(); i++) {
        long charpos = string->GetLinePosition(i);
 
        int length = (int) (string->GetTextLine (NULL, (vchar*)temp, TEMPINCR, i));
       
        if (length + OSapi_strlen(theString) + 1 >= stringSize) {
            stringSize += TEMPINCR;
            theString = (char*)realloc (theString, stringSize);
        }

        strncat (theString, temp, length);

    }

    Tcl_SetResult (interp, theString, TCL_VOLATILE);
    delete string;
    free (theString);
    return TCL_OK;
}


//--------------------------------------------------------------------------------
// dis_getListStrRes - gets the value of a liststr resource (one column at a time).
// 
// argv[1] is the name of the resource to get.
// argv[2] is the column of the resource to get.
//
// returns TCL_OK or TCL_ERROR.
//--------------------------------------------------------------------------------

int dis_getListStrRes (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {

    if (argc != 3) TCL_EXECERR ("wrong number of arguments")

    if (!argv[1] || !argv[0] )
	TCL_EXECERR ("bad arguments")

    vresource res;
    if (!getResource (argv[1], &res))
	TCL_EXECERR ("no resource found")

    vliststr *liststr = new vliststr (res);

    int column = atoi (argv[2]);

    const int TEMPINCR = 1024;
    char *temp;
    size_t tempSize = TEMPINCR;
    temp = (char*)malloc (sizeof(char) * tempSize);
    temp[0] = '\0';

    for (int i = 0; i < liststr->GetRowCount(); i++) {
        char *newstring = (char *)liststr->GetCellValue (i, column);
        if (OSapi_strlen(newstring) + OSapi_strlen(temp) + 4 >= tempSize) {
            tempSize += TEMPINCR;
            temp = (char*)realloc (temp, tempSize);
        }

        if (i) strcat (temp, " ");
        strcat (temp, "{");
        strcat (temp, newstring);
        strcat (temp, "}");
    }

    Tcl_SetResult (interp, temp, TCL_VOLATILE);
    free (temp);
    return TCL_OK;
}

/*-------------------------------------------------------*/

int dis_putStringPreference(ClientData, Tcl_Interp* interp,
				int argc, char** argv)
{
    if (argc != 3) TCL_EXECERR("wrong number of arguments");

    putPreference(argv[1], argv[2]);

    return TCL_OK;
}

	
/*-------------------------------------------------------*/

int dis_getStringPreference(ClientData, Tcl_Interp* interp,
				int argc, char** argv)
{
    if (argc != 2) TCL_EXECERR("wrong number of arguments");

    vstr* value;
    if (getPreference(argv[1], &value) )
        Tcl_SetResult (interp, (char*)value, TCL_VOLATILE);
    else
        TCL_EXECERR1("Unable to find preference", argv[1]);

    return TCL_OK;
}

	


//--------------------------------------------------------------------------------
// dis_gdAddToCombo - Adds a value to a combo box
// 
// argv[1] is the gd
// argv[2] is the Combobox tag name
// argv[3] is the element to add to the list.
//
// returns TCL_OK or TCL_ERROR.
//--------------------------------------------------------------------------------

int dis_gdAddToCombo (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {

    if (argc != 4) TCL_EXECERR ("wrong number of arguments")

    gdComboBox* myItem = (gdComboBox*) dis_findItem(interp, (vchar*) argv[2]);

    if (myItem) {
        myItem->AppendText ((const vchar*)argv[3]);
    }

    return TCL_OK;
}


//--------------------------------------------------------------------------------
// dis_gdAddListToCombo - Adds a list of values to a combo box
// 
// argv[1] is the gd
// argv[2] is the Combobox tag name
// argv[3 thru argc - 1] are the elements to add to the list.
//
// Because I couldn't get it to work any other way, we also check if
// each arg is a list. Each list has all of its members added to the combo.
// If you want spaces in the combo label, you need to use AddItemToCombo above.
// NOTE - This routine doesn't handle braces or quotes!
//
// returns TCL_OK or TCL_ERROR.
//--------------------------------------------------------------------------------
int dis_gdAddListToCombo (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc < 4)
        TCL_EXECERR ("wrong number of arguments") 

    gdComboBox* myItem = (gdComboBox*) dis_findItem(interp, (vchar*) argv[2]);

    if (myItem) {
        // Get the current value.
        // We will check for it being empty later.
        char* currentValue = Tcl_GetVar(interp, argv[2], TCL_GLOBAL_ONLY);

        // Loop through the args, adding to the combo list.
        for (int i = 3; i < argc; i++) {
            // Copy the arg and add each word into the Combo.
            vstr* lStr = vstrCloneScribed(vcharScribeSystem(argv[i]) );
            vchar* item = vcharGetFirstToken(lStr, " ");
            while (item) {
                myItem->AppendText (item);
                item = vcharGetNextToken(" ");
            }
            vstrDestroy(lStr);
        }
        // If the current value is empty, set it to the first item.
        if ((currentValue == NULL) || (*currentValue == NULL) ) {
			if(myItem->GetValueCount()!=0) {
				myItem->SelectTextAt(0);
                Tcl_SetVar(interp, argv[2], (char*)myItem->GetTextAt(0), TCL_GLOBAL_ONLY);
			} else {
                Tcl_SetVar(interp, argv[2], (char*)"", TCL_GLOBAL_ONLY);
			}
        }
    }

    return TCL_OK;
}


//--------------------------------------------------------------------------------
// dis_gdClearCombo - Clears out a combobox.
// 
// argv[1] is the gd
// argv[2] is the Combobox tag name
//
// returns TCL_OK or TCL_ERROR.
//--------------------------------------------------------------------------------

int dis_gdClearCombo (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {

    if (argc != 3) TCL_EXECERR ("wrong number of arguments")

    gdComboBox* myItem = (gdComboBox*) dis_findItem(interp, (vchar*) argv[2]);

    if (myItem) {
        vliststr *theList = myItem->GetListstr();
        for (int i = theList->GetRowCount() - 1; i >= 0; i--) {
            myItem->DeleteTextAt (i);
	}
    }

    return TCL_OK;
}


//--------------------------------------------------------------------------------
// dis_gdComboSelect - Selects an item in a combo box (-1 for no selection)
// 
// argv[1] is the gd
// argv[2] is the Combobox tag name
// argv[3] is the index of the item to select (indicies start at zero).
//
// returns TCL_OK or TCL_ERROR.
//--------------------------------------------------------------------------------

int dis_gdComboSelect (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {

    if (argc != 4) TCL_EXECERR ("wrong number of arguments")

    gdComboBox* myItem = (gdComboBox*) dis_findItem(interp, (vchar*) argv[2]);

    if (myItem) {
        int index = atoi(argv[3]);
        if (index < myItem->GetTextCount() )
            myItem->SelectTextAt(index);
        else
            TCL_EXECERR("Combo selection index out of range.");
    }

    return TCL_OK;
}


//--------------------------------------------------------------------------------

int dis_gdNotebookPageEnable (ClientData, Tcl_Interp* interp, int argc, char *argv[]) {

    if (argc != 5) TCL_EXECERR ("wrong number of arguments")

    gdNotebook* myItem = (gdNotebook*) dis_findItem(interp, (vchar*) argv[2]);

    if (myItem) {
        vnotebookPage *thePage = myItem->FindPage (vnameInternGlobal((const vchar *)argv[3]));
        if (thePage) {
            vnotebookTab *theTab = thePage->GetTab();
            if ( *argv[4] == '0') theTab->Disable();
            else                  theTab->Enable();
        }
    }

    return TCL_OK;
}

int dis_service_connect_remote (ClientData, Tcl_Interp* interp, int argc, char *argv[]) 
{
   if (argc != 3) TCL_EXECERR ("wrong number of arguments")
 
    Application *app;
 
    app = Application::findApplication ((vchar*)argv[1]);
 
    if (app) {
	rcall_dis_connect_app (app, (vstr*)argv[2]);
        return TCL_OK;
    }

    return TCL_ERROR;
}


int dis_service_connect (ClientData, Tcl_Interp* interp, int argc, char *argv[]) 
{
    if (argc != 2) TCL_EXECERR ("wrong number of arguments")

    Application *app;

    app = Application::findApplication ((vchar*)argv[1]);

    if (app) {
	    Tcl_SetResult (interp, "Service already connected", TCL_STATIC);
	    return TCL_ERROR;
    }

    app = new Application;
    if (app->connect (Application::getGroupName(), argv[1])) {
	   Tcl_SetResult (interp, "Error connecting service", TCL_STATIC);	
	   return TCL_ERROR;
    }

    return TCL_OK;
}

int dis_service_disconnect (ClientData, Tcl_Interp* interp, int argc, char *argv[]) 
{
    if (argc != 2) TCL_EXECERR ("wrong number of arguments")
	Application *app;
    app = Application::findApplication ((vchar*)argv[1]);
    if (app) {
	    delete app;
	    return TCL_OK;
    }

    return TCL_ERROR;
}

int dis_query (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    static const char *pszYes = "1";
    static const char *pszNo = "0";
    const char *pszIsModule = pszNo;
    // Check for enough arguments.
    if (argc < 3)
	TCL_EXECERR ("Need at least 3 args for dis_query.");

    // First arg is the Query Type.
    gString queryType = (vchar*) argv[1];

    // Second arg is the "Ask" or "Catagories" selection list.
    gString selection = (vchar*) argv[2];

    // Third arg, if given, is the source RTL.
    rtlClient* sourceRTL = NULL;
    if (argc > 3) {
        sourceRTL = rtlClient::find (atoi(argv[3]));
    } else {
        // Get source RTL from the Browser.
        char* rtl = Tcl_GetVar (interp, "BROWSER_SourceRTL", TCL_GLOBAL_ONLY);
        if (rtl) sourceRTL = rtlClient::find (atoi(rtl));
    }
 
    // Fourth arg, if given, is the target RTL.
    rtlClient* targetRTL = NULL;
    if (argc > 4) {
        targetRTL = rtlClient::find (atoi(argv[4]));
    } else {
        // Get target RTL from the Browser.
        char* rtl = Tcl_GetVar (interp, "BROWSER_TargetRTL", TCL_GLOBAL_ONLY);
        if (rtl) targetRTL = rtlClient::find (atoi(rtl));
    }

    // If still no source RTL, try getting last RTL selected.
    if (sourceRTL == 0) {
        char* rtl = Tcl_GetVar (interp, "GLOBAL_LastRTL", TCL_GLOBAL_ONLY);
	sourceRTL = rtlClient::find (atoi(rtl));
    }

    // If still no source RTL, we have to stop.
    if (!sourceRTL) TCL_EXECERR ("no Source RTL specified");


    // Now, process the given query.
    if (queryType == (vchar*) "-categories") {
	sourceRTL->queryCategories ((vchar*)selection);
    } else if (queryType == (vchar*) "-toplevel") {
        sourceRTL->queryProjects ();
    } else if (queryType == (vchar*) "-ask") {
        if (sourceRTL && targetRTL)
            targetRTL->queryLinks (sourceRTL, (vchar*)selection);
    } else if (queryType == (vchar*) "-projects") {
	if (sourceRTL && targetRTL)
	{ 
	    if ( targetRTL->queryProjects (sourceRTL) )
	        pszIsModule = pszYes;
	}
    } else if (queryType == (vchar*) "-update") {
        if (sourceRTL)
            sourceRTL->update();            
    } else {
	if (sourceRTL && targetRTL)
            targetRTL->queryLinks (sourceRTL, (vchar*)selection);
    } 

    Tcl_SetResult (interp, (char *)pszIsModule, TCL_STATIC);
    return TCL_OK;
}

int dis_stop (ClientData, Tcl_Interp*, int, char *[]) {
    return TCL_OK;
}

// This is a hack to get the RTLs to work from the server when the gala code was
// linked in with the pset_server code.  This command is used to set the creator
// of the RTL to be Server.  This is so updates to the RTL will go to the server,
// and not back to the client through the DIS_ui channel.

int dis_rtl_set_caller (ClientData, Tcl_Interp* , int , char *argv[])
{

    rtlClient* rtl = rtlClient::find (atoi(argv[1]));
    if (rtl) {
	rtl->set_caller ((vchar*)argv[2]);
	return TCL_OK;
    }

    return TCL_ERROR;
}


int dis_rtl_select (ClientData, Tcl_Interp* , int argc, char *argv[])
{
    if (argc != 3) 
	return TCL_ERROR;

    rtlClient* rtl = rtlClient::find (atoi(argv[1]));
    if (rtl) {
	rtl->select ((const vchar*)argv[2]);
        return TCL_OK;
    }
    return TCL_ERROR;
}

int dis_rtl_clear (ClientData, Tcl_Interp *, int argc, char *argv[])
{
    if (argc != 2) return TCL_ERROR;

    rtlClient* rtl = rtlClient::find (atoi(argv[1]));
    if (rtl) {
	rtl->clear();
	rtl->update();
  	return TCL_OK;
    }
    return TCL_ERROR;
}

int dis_rtl_preserve_selection (ClientData, Tcl_Interp *, int argc, char *argv[])
{
    if (argc != 2) return TCL_ERROR;

    rtlClient* rtl = rtlClient::find (atoi(argv[1]));
    if (rtl) {
      rtl->preserveSelection();
      return TCL_OK;
    }
    return TCL_ERROR;
}

int dis_rtl_get_id (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_rtl_get_id);
 
    if (!(argc == 2 || argc == 3)) {
        Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
        return TCL_ERROR;
    }

    char *name;
    if (argc == 2) name = argv[1];
    else name = argv[2];
 
    rtlClient* myItem = (rtlClient*) dis_findItem(interp, (vchar*) name);
 
    if (!myItem)
        TCL_EXECERR1 ("Unable to find rtl Item", argv[2])

    gString tmp;
    if (argc == 2){
	tmp.sprintf((vchar *)"%d", myItem->getId());
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
    }
    if (argc == 3){
	tmp.sprintf((vchar *)"%d", myItem->getServerId());
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
    }
    return TCL_OK;
}

static int global_selection_arrived = 0;
static char *sel_file_name = NULL;
static char *sel_line_num  = NULL;
static char *sel_tab_size  = NULL;
static char *sel_text      = NULL;
static char *sel_end_pos   = NULL;

int dis_ide_get_selection(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
		if(argc != 1)
			TCL_EXECERR ("wrong number of arguments")
	
    if (!IntegratorServer)
			TCL_EXECERR ("No Integrator Server")

#ifdef _WIN32
    if (IntegratorServer->isOpen()) {
      global_selection_arrived = 0;
      int zoro = *(int*)"0R0Z";
      IntegratorServer->WriteInt(zoro);
      IntegratorServer->WriteBuffer(OSapi_strlen("get_selection")+1, "get_selection");
      DWORD start_time   = GetTickCount();
      DWORD current_time = start_time;
      /* Wait answer or 5 sek */
      while (global_selection_arrived == 0 && (DWORD)(current_time - start_time) < 5000){
	MSG msg;
	while ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
	  ::DispatchMessage(&msg);
	}
	current_time = GetTickCount();
      }
      if(global_selection_arrived == 0){
	MessageBox(NULL, "Unable to get proper selection from MSDEV. Possible installation problem.",
		   "DISCOVER", MB_OK);
	Tcl_SetResult(interp, "0 0 0 0 {0}", TCL_VOLATILE);
      } else 
	Tcl_AppendResult(interp, sel_file_name, " ", sel_line_num, " ", sel_tab_size, 
			 " ", sel_end_pos, " {", sel_text, "}", NULL);
    }
#endif
    return TCL_OK;
}

#define SET_SEL_STR(x,y) if(x) free(x); x = strdup(y);

int dis_IDE_current_selection(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if(argc < 7){
	SET_SEL_STR(sel_file_name, "0");
	SET_SEL_STR(sel_line_num, "0");
	SET_SEL_STR(sel_tab_size, "0");
	SET_SEL_STR(sel_end_pos, "0");
	SET_SEL_STR(sel_text, "0");
    } else {
	SET_SEL_STR(sel_file_name, argv[2]);
	SET_SEL_STR(sel_line_num, argv[3]);
	SET_SEL_STR(sel_tab_size, argv[4]);
	SET_SEL_STR(sel_end_pos, argv[5]);
	SET_SEL_STR(sel_text, argv[6]);
    }
    global_selection_arrived = 1;
    return TCL_OK;
}

int dis_ide_send_command (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc  != 2)
	TCL_EXECERR ("wrong number of arguments")
	
    if (!IntegratorServer)
	TCL_EXECERR ("No Integrator Server")

    if (IntegratorServer->isOpen()) {
        int zoro = *(int*)"0R0Z";
        IntegratorServer->WriteInt (zoro);
        IntegratorServer->WriteBuffer (OSapi_strlen(argv[1])+1, argv[1]);
    }

    return TCL_OK;
}

int dis_ide_start (ClientData, Tcl_Interp*, int, char *[]) {
    return 1;
}

int dis_viewer_pop_active (ClientData, Tcl_Interp*, int, char *[]) {
    Viewer::GetFirstViewer()->bring_to_top();
    return TCL_OK;
}

int dis_rtl_post_init (ClientData, Tcl_Interp*, int argc, char *argv[])
{
    rtlClient* rtl = rtlClient::find (atoi(argv[1]));

    if (argc == 3) rtl->setFilterButton ((vchar*)argv[2]);
    else rtl->setFilterButton ((vchar*)"rtlFilter");

    if (rtl) rtl->postLoadInit(NULL);

    return TCL_OK;
}

int dis_ItemHide (ClientData,
                      Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_ItemHide);
 
    if (argc != 3 && argc != 5)
        TCL_EXECERR ("wrong number of arguments")
 
    vdialogItem* myItem;
    const char* myValue = NULL;
       
    if (strcmp (argv[1], "-layer") == 0) {
        Layer* layer = Layer::find_layer (argv[2]);
        if (layer && (argc == 5)) {
            View* v = layer->getView ();
            if (v) {
                myItem = (vdialogItem*)v->FindItem ((vchar*)argv[3]);
                myValue = argv[4];
            }
        } else
            return TCL_ERROR;
 
    } else {
        myItem = (vdialogItem*) dis_findItem(interp, (vchar*) argv[1]
);
        if (myItem)
            myValue = argv[2];
    }
 
    if (!myItem)
        TCL_EXECERR1 ("Item not found", argv[2])
       
    if ( *myValue == '0') myItem->Show();
    else myItem->Hide();
 
    return TCL_OK;
}

#ifdef WIN32
#define getpid _getpid
extern "C" long getpid(); 
#endif

dis_puts (ClientData, Tcl_Interp* , int argc, char* argv[])
{
	for (int i=0; i<argc; i++) {
		fprintf (stderr, "%s ", argv[i]);
	}
	fprintf (stderr, "\n");

	return TCL_OK;
}

dis_GetPid (ClientData, Tcl_Interp* interp, int , char *[])
{
    gString tmp;
    tmp.sprintf((vchar *)"%d", getpid());
    Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
    return TCL_OK;
}

dis_GetSystemName (ClientData, Tcl_Interp* interp, int , char *[])
{
    char buf[256];
    OSapi_gethostname (buf, 100);
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
    return TCL_OK;
}

dis_GetMyServiceName (ClientData, Tcl_Interp* interp, int , char *[])
{
    Tcl_SetResult(interp, (char *)DISui_app->getName(), TCL_VOLATILE);
    return TCL_OK;
}

dis_GetMyGroupName (ClientData, Tcl_Interp* interp, int , char *[])
{
    Tcl_SetResult(interp, (char *)Application::getGroupName(), TCL_VOLATILE);
    return TCL_OK;
}

dis_StartService (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
	gString str;

    if (argc != 5)
		return TCL_ERROR;

	str = (vchar*)argv[1];

#ifdef WIN32
	str += (vchar*)".exe";
#endif

	str += (vchar*)" -group ";
	str += (vchar*)argv[2];
	str += (vchar*)" -service ";
	str += (vchar*)argv[3];
    str += (vchar*)" -client ";
	str += (vchar*)argv[4];

	for (vchar*ptr=str;*ptr;ptr++) {
#ifdef WIN32
		if (*ptr == '/') *ptr = '\\';
#else
		if (*ptr == '\\') *ptr = '/';
#endif
	}

	FILE* f = vpopen(str, (vchar*)"R");
	if (!f) {
		fprintf (stderr, "System Failure [%s]\n", (char*)(vchar*)str);
	    return TCL_ERROR;
	}
	
	gString tmp;
	tmp.sprintf((vchar *)"%u", vp_find_pid(f));
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
	return TCL_OK;
}

dis_StopService (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    return dis_puts (0, interp, argc, argv);
}

dis_getenv (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    int status = TCL_OK;

    if (argc != 2) {
        status = TCL_ERROR;
    } else {
        char* value = OSapi_getenv (argv[1]);
	if (value) {
	    Tcl_SetResult(interp, value, TCL_VOLATILE);
	}
    }
    return status;
}

dis_putenv (ClientData, Tcl_Interp* , int argc, char* argv[])
{
    if (argc == 3) {
	gString str;
	str.sprintf ((vchar*)"%s=%s", (vchar*)argv[1], (vchar*)argv[2]);
        OSapi_putenv (OSapi_strdup((char*)(vchar*)str));
        return TCL_OK;
    }
    return TCL_ERROR;
}

dis_DateTime (ClientData, Tcl_Interp* interp, int , char *[])
{
    vdate date;

    date.SetCurrent();

    gString str;
    str.sprintf ((vchar *)"%02d-%02d-%02d-%02d-%02d-%02d", date.GetYear()-1, date.GetMonth()-1, date.GetDay()-1, 
		 date.GetHour(), date.GetMinute(), date.GetSecond());
    Tcl_SetResult(interp, (char *)str, TCL_VOLATILE);
    return TCL_OK;
}

int dis_java_test (ClientData, Tcl_Interp* interp, int , char *[])
{
    printf("You are here.\n");
    vchar array[100];
    vchar* array2 = new vchar [200];
    sprintf((char*)array, "\nThe eagle has landed");
    int size = OSapi_strlen((char*)array) + 1;

    char num[4];
    num[0] = (size >> 24) & 255;    
    num[1] = (size >> 16) & 255;    
    num[2] = (size >>  8) & 255;    
    num[3] = (size >>  0) & 255;    

    sprintf((char*)array2, "%s\n", array);
    Tcl_SetResult (interp, (char*)array2, TCL_VOLATILE);
    return TCL_OK;
}


// Strips off control characters, which are used for the icon
int getTrueSize(vstr* string)
{
    int i = vcharLength(string);
    int length = i + 1;
    while (( i >= 0 ) && vcharIsControl(string[i-1])) {
	i--;
	length--;
    }

    return length;
}


char* getData(int serverId, int size)
{
    char idInAscii[100];
    sprintf(idInAscii, "\n%d\n%d\n", serverId, size);
    int idSize = OSapi_strlen(idInAscii);

    const char* ERROR_ELEMENT = "error";
    char** data = new char*[size];
    int* dataSize = new int[size];
    int totalDataSize = 0;
    int length;
    for (int i = 0; i < size; i++) {
	vstr* str = rcall_dis_rtl_get_item (Application::findApplication("DISmain"), 
					    serverId, i);

	length = 0;
        if (str) {
	    length = getTrueSize(str);
	    data[i] = new char[vcharLength(str)]; // still copy control codes
	    strcpy(data[i], (char*) str);
	}
	else {
	    length = OSapi_strlen(ERROR_ELEMENT) + 1;
	    data[i] = new char[length];
	    strcpy(data[i], ERROR_ELEMENT);
	}
	dataSize[i] = length;
	printf("%d\n", i);
	totalDataSize = totalDataSize + length + 1; /* extra space for carriage return */
    }

    char* dataToReturn = new char[totalDataSize + 20 + idSize];
    strcpy(dataToReturn, idInAscii);
    char* ptr = dataToReturn + idSize;

    for (i = 0; i < size; i++) {
	printf("%d\n", i);
	strcpy(ptr, data[i]);
	ptr = ptr + dataSize[i] - 1;
	delete [] data[i];
	*ptr = '\n';
	ptr++;
    }
    strcpy(ptr, "finish\n");
    delete [] data;
    delete [] dataSize;

    return dataToReturn;
}    


int dis_java_build (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    // args :
    // 1: selections in category list
    // 2: filter including
    // 3: filter excluding

    if (argc < 4) {
	Tcl_SetResult(interp, "Not enough arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    int serverId = rcall_dis_rtl_create(Application::findApplication("DISmain"), 0);

    if (strcmp(argv[1], FOLLOW_PROJECT) == 0)
	rcall_dis_rtl_query_top_proj (Application::findApplication("DISmain"), serverId);
    else
	rcall_dis_rtl_init (Application::findApplication("DISmain"),
			    serverId, (vstr*) argv[1]);

    if ( strcmp(argv[2], "none") != 0) {
	printf("Trying to filter...\n");
	char filterCommand[2048];
	sprintf(filterCommand, "reval \"DISrtlFilter\" [concat \" dis_filterApply\" %d %s %s %s %s", serverId,
		"" /* sort */, "" /* format */, argv[2] /* show */, "" /* hide */);
	
	Tcl_Eval(interp, filterCommand);
    }
    
    int size = rcall_dis_rtl_get_size(Application::findApplication("DISmain"), 
				      serverId);
    
    Tcl_SetResult (interp, getData(serverId, size), TCL_VOLATILE);
    return TCL_OK;
}


int dis_java_follow (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    //args:
    // 1: base query
    // 2: selections in base list
    // 3: selections in ask list
    // 4: filter including
    // 5: filter excluding

    if (argc < 6) {
	Tcl_SetResult(interp, "Not enough arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    int sourceId = atoi(argv[1]);
    int serverId = rcall_dis_rtl_create(Application::findApplication("DISmain"), 0);
    

    if (strcmp(argv[3], FOLLOW_PROJECT) == 0)
	rcall_dis_rtl_query_proj (Application::findApplication("DISmain"), sourceId,
				  atoi(argv[2]), serverId);	
    else {
	rcall_dis_rtl_select(Application::findApplication("DISmain"), sourceId, 
			     (vstr*) argv[2]);
	rcall_dis_rtl_query_links (Application::findApplication("DISmain"), sourceId,
				   (vstr*) argv[3], serverId);
    }
    
    int size = rcall_dis_rtl_get_size(Application::findApplication("DISmain"), 
				      serverId);
    
    Tcl_SetResult (interp, getData(serverId, size), TCL_VOLATILE);
    return TCL_OK;
}



int dis_java_destroy (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    //args:
    // ids to destroy

    for (int i = 1; i < argc; i++)
	rcall_dis_rtl_destroy(Application::findApplication("DISmain"), 
			      atoi(argv[i]));
    
    Tcl_SetResult (interp, "\nDestruction complete\n", TCL_STATIC);
    return TCL_OK;
}



    
int dis_build_service_list (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    //args: 
    // 1) Name attribute to match against
    // 2) Regular expression to match titles against

    if (argc < 3) {
	Tcl_SetResult(interp, "Not enough arguments", TCL_STATIC);
	return TCL_ERROR;
    }

    Application::buildServiceList((const vchar*) argv[1],
				  (const vchar*) argv[2]);
    return TCL_OK;
}



int dis_get_service_list_size(ClientData, Tcl_Interp* interp, int, char *[])
{
    char buffer[1024];
    sprintf((char*)buffer, "%d", Application::getServiceListSize());
    Tcl_SetResult (interp, buffer, TCL_VOLATILE);
    return TCL_OK;
}


int dis_get_service_list_entry(ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    // args:
    // 1: index
    // 2: attribute name
    if (argc < 3) {
	Tcl_SetResult(interp, "Not enough arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    
    Tcl_SetResult(interp, (char*) Application::getServiceListAttributeValue(atoi(argv[1]), (const vchar*) argv[2]), TCL_VOLATILE);
    return TCL_OK;
}


int dis_get_service_list_index(ClientData, Tcl_Interp* interp, 
			       int argc, char** argv)
{
    // args:
    // 1: attribute name
    // 2: attribute value

    if (argc < 3)
	TCL_EXECERR ("not enough arguments");
    
    char buffer[1024];
    sprintf(buffer, "%d", Application::getServiceListItemIndex((vchar*) argv[1], (vchar*) argv[2]));
    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
    return TCL_OK;
}



// Return a list of all entries in the given selection.
int dis_SelectionFromList (ClientData, Tcl_Interp* interp,
			     int argc, char** argv)
{
    if (argc < 3)
	TCL_EXECERR ("wrong number of arguments");

    // Get list of selected items.
    const vchar* sourceList = (vchar*) argv[1];
    gString items = (vchar*)"";

    SelectionIterator sel((vchar*)argv[2]);
    vbool first = vTRUE;
    while (sel.Next()) {
        int index = sel.GetNext();
        gString cmd;
        cmd.sprintf((vchar*)"lindex {%s} %d", sourceList, index);
        Tcl_Eval(interp, (char*)(vchar*)cmd);
        if (first)
            first = vFALSE;
        else
            items += (vchar*)" ";
        items += (vchar*)Tcl_GetStringResult(interp);
    }

    Tcl_SetResult (interp, (char*)(vchar*)items, TCL_VOLATILE);

    return TCL_OK;
}



int dis_file_read (ClientData, Tcl_Interp* interp, int argc, char ** argv)
{
    // Reads a file and returns the contents of the file

    if (argc != 2) TCL_EXECERR ("Wrong number of arguments");

    vfsFile *readFile = (vfsFile *)NULL;
    vbyte readBuffer[vfsIO_BUFFER_SIZE+1];
    vstr *returnString = vstrCreate();

    vexWITH_HANDLING {
        vchar *filepath = (vchar*)argv[1];
        vfsPath path;
        path.Set (filepath);
        path.ExpandVariables();
        path.MakeCanonical();

	readFile = vfsFile::Open (&path, vfsOPEN_READ_ONLY);
	long numBytes;
	while ((numBytes = readFile->Read(readBuffer, sizeof(vbyte), vfsIO_BUFFER_SIZE)) != 0) {
	    readBuffer[numBytes] = (vbyte)'\0';
	    returnString = vstrAppend (returnString, (vchar*)readBuffer);
	}
    } vexON_EXCEPTION {
    } vexEND_HANDLING;


    Tcl_SetResult (interp, (char *) returnString, TCL_VOLATILE);
    return TCL_OK;
}


int dis_file_write (ClientData, Tcl_Interp* interp, int argc, char ** argv)
{
    // Writes a filed to disk

    if (argc != 3) TCL_EXECERR ("Wrong number of arguments");

    vchar *filepath = (vchar*)argv[1];
    vchar *contents = (vchar*)argv[2];
    vfsPath path;
    path.Set (filepath);

    vfsFile *writeFile = (vfsFile *)NULL;

    vexWITH_HANDLING {
	writeFile = vfsFile::Open (&path, vfsOPEN_WRITE_TRUNCATE);
	int numBytes = vcharLength(contents);
	writeFile->Write (contents, sizeof(vbyte), numBytes);
    } vexON_EXCEPTION {
    } vexEND_HANDLING;                                                   

	if (writeFile) writeFile->Close ();

    return TCL_OK;
}



int dis_PasteViewerSelection (ClientData,
		       Tcl_Interp* interp, int , char *[])
{
  gInit (dis_PasteViewerSelection);
  
  int fWasPasted = 0;
  Application *app = Application::findApplication("DISmain");
  if( app && app->mySession() )
  {
    if( rcall_dis_viewer_selection_exists( app ) == 1 )
    {
      Viewer::GetFirstViewer()->bring_to_top();
      if( Tcl_Eval( interp, "dis_layer -open DialogLayer Browser -reuse" ) 
          == TCL_OK )
      {
        gdNotebook* myItem = (gdNotebook*) dis_findItem(interp, (vchar*)"BrowserNotebook" );

        if (myItem) 
        {
          vnotebookPage *thePage = 
            myItem->FindPage(vnameInternGlobal((const vchar *)"ModelBrowser"));
          if (thePage)
          {
            thePage->SetCurrent();
            rtlClient* pRTL = (rtlClient*) dis_findItem(interp, (vchar*)"ElementsList");
            if( pRTL )
            {
              fWasPasted = 1;
              rcall_dis_insert_selection_into_rtl( app, pRTL->getServerId() );
            }
          }
        }
      }
    }
  }

  if( !fWasPasted )
    Tcl_Eval( interp, "dis_Message [concat There is no symbol selected to query on]" );
  return TCL_OK;
}


int dis_ide_open_definition (ClientData,
		       Tcl_Interp* interp, int , char *[] )
{
  gInit (dis_ide_open_definition);
  int nRet = TCL_ERROR;

  Application *app = Application::findApplication("DISmain");
  if( app && app->mySession() )
  {
    rcall_dis_open_selected_in_view( app );
    nRet = TCL_OK;
  }
  else
    Tcl_Eval( interp, "dis_Message [concat There is no symbol selected to open definition for]" );
  return nRet;
}


int dis_TimerRet(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 1)
        TCL_EXECERR ("wrong number of arguments");
    Tcl_SetResult (interp, "1", TCL_VOLATILE);

    return TCL_OK;
}


dis_Enable_MenuToolBar(ClientData, Tcl_Interp* , int , char *[])
{
    if(Viewer_vr) 
       Viewer_vr->Enable_MenuToolBar();

    return TCL_OK;
}

int dis_TextItemSetModifiable(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_ERROR;
  if( argc >=3 )
  {
    int fIsModif = atoi( argv[2] );
    vtextitem* pItem = vtextitem::CastDown((vdialogItem *)dis_findItem(interp, (vchar*) argv[1]));
    if( pItem )
    {
      pItem->SetModifiable( fIsModif );
      nRet = TCL_OK;
    }
  }
  else
    TCL_EXECERR ("wrong number of arguments")  
  return nRet;
}

extern void InitAttributeEditor(gdAttributeEditor *ae);

dis_gdInitAttributeEditor(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_gdInitAttributeEditor);

    if (argc != 3) {
       Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
       return TCL_ERROR;
    }

    gdAttributeEditor* myItem = (gdAttributeEditor*) dis_findItem(interp, (vchar*)argv[2]);

    if (!myItem) {
       Tcl_SetResult (interp, "AttributeEditor item not found", TCL_STATIC);
       return TCL_ERROR;
    }

    InitAttributeEditor(myItem);
    return TCL_OK;
}

dis_gdAttributeEditorAddAttribute(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_gdInitAttributeEditor);

    if (argc != 5) {
       Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
       return TCL_ERROR;
    }
    gdAttributeEditor* myItem = (gdAttributeEditor*) dis_findItem(interp, (vchar*)argv[1]);
    if (!myItem) {
       Tcl_SetResult (interp, "AttributeEditor item not found", TCL_STATIC);
       return TCL_ERROR;
    }
    myItem->AddAttribute(argv[2], argv[3], argv[4]);
    return TCL_OK;
}

dis_gdAttributeEditorAddEnumAttributeValue(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_gdAttributeEditorAddEnumAttributeValue);

    if (argc != 4) {
       Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
       return TCL_ERROR;
    }
    gdAttributeEditor* myItem = (gdAttributeEditor*) dis_findItem(interp, (vchar*)argv[1]);
    if (!myItem) {
       Tcl_SetResult (interp, "AttributeEditor item not found", TCL_STATIC);
       return TCL_ERROR;
    }
    myItem->AddEnumAttributeValue(argv[2], argv[3]);
    return TCL_OK;
}

dis_gdAttributeEditorSetAttributeValue(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_gdAttributeEditorSetAttributeValue);

    if (argc != 4) {
       Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
       return TCL_ERROR;
    }
    gdAttributeEditor* myItem = (gdAttributeEditor*) dis_findItem(interp, (vchar*)argv[1]);
    if (!myItem) {
       Tcl_SetResult (interp, "AttributeEditor item not found", TCL_STATIC);
       return TCL_ERROR;
    }
    myItem->SetAttributeValue(argv[2], argv[3]);
    return TCL_OK;
}

dis_gdAttributeEditorReset(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_gdAttributeEditorSetAttributeValue);

    if (argc != 2) {
       Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
       return TCL_ERROR;
    }
    gdAttributeEditor* myItem = (gdAttributeEditor*) dis_findItem(interp, (vchar*)argv[1]);
    if (!myItem) {
       Tcl_SetResult (interp, "AttributeEditor item not found", TCL_STATIC);
       return TCL_ERROR;
    }
    myItem->Reset();
    return TCL_OK;
}

int dis_aset_eval_cmd(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if(argc != 2){
	Tcl_AppendResult(interp, "usage: ", argv[0], " <command>", NULL);
	return TCL_ERROR;
    }
    gString result;
    SendToServer_sync(argv[1], result);
    Tcl_AppendResult(interp, (char *)result, NULL);
    return TCL_OK;
}

int dis_aset_eval_cmd_async(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if(argc != 2){
	Tcl_AppendResult(interp, "usage: ", argv[0], " <command>", NULL);
	return TCL_ERROR;
    }
    SendToServer_asyn(argv[1]);
    return TCL_OK;
}
   
int dis_tempnam(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  int nRet = TCL_ERROR;
	char *pszDir = NULL;
	char *pszPrefix = NULL;
	if( argc > 1 && strlen(argv[1])>1   )  // so that you can have it go in the temp dir
										// use a {} or "" to make this work
		pszDir = argv[1];
	if( argc > 2 )
		pszPrefix = argv[2];
	char *pszName = OSapi_tempnam( pszDir, pszPrefix );
	if( pszName )
	{
		Tcl_SetResult( interp, OSPATH( pszName ), TCL_VOLATILE );
		nRet = TCL_OK;
		free( pszName );
	}
	return nRet;
}


int dis_unlink(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
	int nRet = TCL_OK;
	char pszResult[2];
	int nResult = 0;
	if( argc > 1 )
	{
		if( OSapi_unlink( argv[1] ) != -1 )
			nResult = 1;
	}
	sprintf( pszResult, "%d", nResult );
	Tcl_SetResult(interp, pszResult, TCL_VOLATILE );
	return nRet;
}

int dis_update_gdTextItem(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
	int nRet = TCL_OK;
	if( argc > 1 )
	{
		gdTextItem *item = gdTextItem::CastDown((vdialogItem *)dis_findItem(interp, (vchar*) argv[1]));
		if( item )
		{
			vstr *cont = item->GetTextAsString();
			if( cont )
			{
				item->SetInterpVariable( (vchar *)item->GetTag(), cont );
				vstrDestroy( cont );
			}
		}
	}
	return nRet;
}


// prequit()
// the dis_quit function was getting a bit too complicated
// so I broke it up into two parts.
// prequit tests (in a minimally annoying manner) to ensure that everything is saved or that the
// user doesn't care
// returns: -1 error 0 OK to quit 1 not ok to quit

static int prequit(Tcl_Interp * interp);
int prequit(Tcl_Interp * interp)
{
	int nRet=0;
	int nQuitStatus = 0; //tells what the status of the whole thing is.
	int nUnfinishedLayers =0; // number of layers that didn't quit properly but didn't cancel
	
	gInit (dis_quit);
	
	layerStack* myLayerStack=Layer::getList();
	Layer * pCurLayer=myLayerStack->top();
	
	//iterate through the layers asking them what they want to do:
	while (pCurLayer!=NULL)
	{
		nQuitStatus=pCurLayer->PrepareShutdown();
		if (nQuitStatus == 2)
		{	nRet=1;
			break;
		}
		else if (nQuitStatus==1)
			nUnfinishedLayers++;
		else if (nQuitStatus <0)
			nRet=-1;
		pCurLayer=Layer::findPreviousLayer(pCurLayer);
	}
	
	if (nUnfinishedLayers&&nRet!=1)
	{
		const char * MessageCmd = "dis_confirm2 \"Really Quit?\" Yes No \"You may have unsaved files or unfinished processes.\n Are you sure you want to quit?\"";
		Tcl_Eval(interp, (char *)MessageCmd);
		if (!strcmp(Tcl_GetStringResult(interp), "RightButton"))
			nRet=1;
	}

	return nRet;
}

// dis_quit: standard quit routine
// syntax:  dis_quit [NOSERVER]
// this function iterates over all layers on the layerstack and executes
// a function which basically asks them if they are ready to shut down.
// If all layers are ready to shut down, it does so, otherwise it asks if the user is "really sure"
// note that this requires the TwoButtonConfirm dialog in every vr file...
// NOSERVER refrains from shutting down servers--it is used if you don't want to shut down a pmodserver.

	
// dis_quit function now returns a value--it returns 0 if you can quit, or 1 if you can't

int dis_quit (ClientData, Tcl_Interp* interp , int argc , char *argv[]) 
{
	int nResult=prequit(interp);
	
	//the logic behind this is that a -1 denotes either a Tcl error or a layer that doesn't 
	// conform to my standard. So I try to conform to the previous behaviour.  
	if (nResult<0)
			nResult=0;
	if (nResult==0) 
	{
		vbool shutdown_server=vTRUE;
		if (argc>=2)
			shutdown_server=(vbool) strcmp(argv[1], "NOSERVER");
		
		if (shutdown_server)
		{
			Application *app = Application::findApplication("DISmain");
			if(	app && app->mySession() && !Application::IsConnectionLost() ) {
				int server_status;    // This is here to quiet down the HP compiler.
				server_status = rcall_dis_shutdown_server( app );
			}
		}
		
		// Now do the client side shutdown.
		
		// Run the shutdown tcl code.
		eval_StringResource(g_global_interp, (vchar*) "shutdownCode");
		
		
		// Delete all of the active viewers.
		while (Viewer::GetFirstViewer()) {
			delete Viewer::GetFirstViewer();
		}
		
		// Tell galaxy to stop running.
		tipWindow::ResetTimer();
		vevent::StopProcessing();
	}
	else
		nResult=1;

	char pszResult[3];
	sprintf(pszResult, "%d", nResult);
	Tcl_SetResult(interp, pszResult, TCL_STATIC);

	return TCL_OK;
}


int dis_current_filter_name(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 1) {
           Tcl_SetResult (interp, "Wrong number args", TCL_STATIC);
           return TCL_ERROR;
    }

    vchar *returnValue;

    returnValue = (vchar*)Tcl_GetVar(g_global_interp, "GLOBAL_CurrentFilterName", TCL_GLOBAL_ONLY);

    Tcl_SetResult (interp, (char *)returnValue, TCL_VOLATILE);
    return TCL_OK;
}

int dis_gdListItem_init_width(ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) {
           Tcl_SetResult (interp, "Wrong number args", TCL_STATIC);
           return TCL_ERROR;
    }

    gdListItem* myItem = (gdListItem*) dis_findItem(interp, (vchar*) argv[1]);

    if (!myItem) {
      Tcl_SetResult (interp, "List item not found", TCL_STATIC);
      return TCL_ERROR;
    }

    myItem->InitWidth();

    return TCL_OK;
}

//----------------------------------------------------------------------------------------------------
// <result>  <-  <name> <index>
//
// This command will set the line wich index passed into it as active line in spinner.
//-----------------------------------------------------------------------------------------------------
int  dis_gdTextSpinSelectByIndex(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);

    int line = atoi(argv[2]);
    if( line<0 || spinner->GetTextCount()<= line) {
	   Tcl_SetResult (interp, "passed line out of range", TCL_STATIC);
	   return TCL_ERROR;
    }
    spinner->SelectTextAt(line);
	spinner->MarkContentDirty();
    return TCL_OK;
}
//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
// <void>  <-  <name> <text>
//
// This command will set the line wich name passed into it as a active line in spinner.
//----------------------------------------------------------------------------------------------------
int dis_gdTextSpinSelectByText(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
    spinner->SelectText((unsigned char *)argv[2]);
	spinner->MarkContentDirty();
    return TCL_OK;
}
//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
// <void>  <-  <name> <text>
// This function will add the text line into text spinner. The function will add this line at the end
// of the text spinner list. If it is sorted text spinner, it will call sort() after adding.
//---------------------------------------------------------------------------------------------------
int dis_gdTextSpinAddText(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
    spinner->AppendText((unsigned char *)argv[2]);
	spinner->MarkListDirty();
    return TCL_OK;
}
//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
// <void>  <-  <name> <text> <index>
// 
// This function will change text at the spesified index
//---------------------------------------------------------------------------------------------------
int dis_gdTextSpinChangeText(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);

    int line = atoi(argv[3]);
    if( line<0 || spinner->GetTextCount()<= line) {
	Tcl_SetResult (interp, "passed line out of range", TCL_STATIC);
	return TCL_ERROR;
    }
    spinner->SetTextAt((unsigned char *)argv[2],line);
	spinner->MarkListDirty();
    return TCL_OK;
}
//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
// <void>  <-  <name>
//
// This function will clean all selected text spinner list
//----------------------------------------------------------------------------------------------------
int dis_gdTextSpinClear(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 2) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
    spinner->Clean();
	spinner->MarkListDirty();
    return TCL_OK;
}
//---------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------
// <void>  <-  <name> <text>
//
// This command will remove spesified text from the spinner.
//---------------------------------------------------------------------------------------------------
int dis_gdTextSpinDeleteText(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
    spinner->DeleteText((unsigned char *)argv[2]);
	spinner->MarkListDirty();
    return TCL_OK;
}
//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
// <void>  <-  <name> <index>
//
// This command will remove text spin line with the spesified index.
//----------------------------------------------------------------------------------------------------
int dis_gdTextSpinDeleteIndex(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
    if (argc != 3) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
    if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
    int line = atoi(argv[2]);
    if( line<0 || spinner->GetTextCount()<= line) {
	Tcl_SetResult (interp, "passed line out of range", TCL_STATIC);
     return TCL_ERROR;
   }
   spinner->DeleteTextAt(line);
   spinner->MarkListDirty();
   return TCL_OK;
}
//-----------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------
// <text>  <-  <name> 
//
// This function will return currently selected text string.
//-----------------------------------------------------------------------------------------------------
int dis_gdTextSpinGetSelectedText(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 2) {
	  Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
   Tcl_SetResult (interp, spinner->GetCurrentText(), TCL_STATIC);
   return TCL_OK;
}
//-----------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------
// <index> <-  <name> 
//
// This function will return the currently selected line index.
//-----------------------------------------------------------------------------------------------------
int dis_gdTextSpinGetSelectedIndex(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 2) {
	  Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
   int index = spinner->GetCurrentIndex();
   gString result_str;
   result_str.sprintf((vchar *)"%d", index);
   Tcl_SetResult (interp, (char *)result_str, TCL_VOLATILE);
   return TCL_OK;
}
//-----------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------
// <text>  <-  <name> <index>
//
// This function will return the text line located at the spesified index.
//-----------------------------------------------------------------------------------------------------
int dis_gdTextSpinGetTextAt(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 3) {
	  Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
   int line = atoi(argv[2]);
   if( line<0 || spinner->GetTextCount()<= line) {
	 Tcl_SetResult (interp, (char *)"passed line out of range", TCL_STATIC);
     return TCL_ERROR;
   }
   Tcl_SetResult (interp, (char *)spinner->GetTextAt(line), TCL_STATIC);
   return TCL_OK;
}
//------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------
// <text>  <-  <name> <text>
//
// This function will return the text the index of the spesified line
//-----------------------------------------------------------------------------------------------------
int dis_gdTextSpinGetIndexFromText(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 3) {
	  Tcl_SetResult (interp, (char *)"wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
   int index = spinner->FindTextIndex((unsigned char *)argv[2]);
   gString result_str;
   result_str.sprintf((vchar *)"%d", index);
   Tcl_SetResult (interp, (char *)result_str, TCL_VOLATILE);
   return TCL_OK;
}
//------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------
// <int>   <-  <name> 
//
// This command will return the amount of lines currently into the text spinner.
//------------------------------------------------------------------------------------------------------
int dis_gdTextSpinGetLinesAnount(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 2) {
	  Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
   int amount = spinner->GetTextCount();
   gString result_str;
   result_str.sprintf((vchar *)"%d", amount);
   Tcl_SetResult (interp, (char *)result_str, TCL_VOLATILE);
   return TCL_OK;
}
//-----------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------
// <void>  <-  <name>
//
// This command will sort lines in the spesified list.
//-----------------------------------------------------------------------------------------------------
int dis_gdTextSpinSort(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 2) {
	  Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ("Text spinner was not found", argv[1]);
   spinner->SortList();
   return TCL_OK;
}
//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
// <void>  <-  <name>
// This function will set the SelectionChanged callback to the spesified spinner.
//----------------------------------------------------------------------------------------------------
int dis_gdTextSpinSetChangeCallback(ClientData, Tcl_Interp* interp, int argc, char *argv[]) {
   if (argc != 3) {
	  Tcl_SetResult (interp, (char *)"wrong number of arguments", TCL_STATIC);
	  return TCL_ERROR;
   }
   gdTextSpinner *spinner = (gdTextSpinner *)dis_findItem(interp, (vchar*) argv[1]);
   if(!spinner)  TCL_EXECERR1 ((char *)"Text spinner was not found", argv[1]);
   spinner->SetChangeCallback((char *)argv[2]);
   return TCL_OK;
}
//---------------------------------------------------------------------------------------------------

//
// Returns the number of items in a given selection:
//
int dis_sel_size(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
  int ret = TCL_ERROR;

  gString usage;
  usage.sprintf((vchar *)"Usage: %s <selection>", argv[0]);

  if (argc == 2) {
    int nsel = 0;
    SelectionIterator it((vchar *)argv[1]);
    while (it.Next()) {
      ++nsel;
    }
    gString result;
    result.sprintf((vchar *)"%d", nsel);
    Tcl_SetResult (interp, (char *)result, TCL_VOLATILE);
    ret = TCL_OK;
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}

struct aset_notification {
    char       *f_name;
    Tcl_Interp *interp;
};

#ifndef _WIN32

static int read_server_output(int fd, gString& retval)
{
    char buffer[1024];
    int  len;
    int  total = 0;
    do {
	total += (len = read(fd, buffer, 1023));
	if(len <= 0)
	    break;
	if(len == 1023)
	    buffer[1023] = 0;
	retval += buffer;
    } while(len == 1023);
    return (total != 0);
}

static void notification_proc(veventFD *efd)
{
    aset_notification *an = (aset_notification *)efd->GetData();
    int fd                = efd->GetFileDescriptor();
    gString str;
    int success = read_server_output(fd, str);
    if(success == 0){
	   efd->Unregister();
	   tipWindow::ResetTimer();
	   vevent::StopProcessing();
	   return;
    }
    if(an->f_name){
	gString full_cmd = an->f_name;
	full_cmd        += " {";
	full_cmd        += str;
	full_cmd        += "}";
	gala_eval(an->interp, (char *)full_cmd);
    }
}

static void exception_proc(veventFD *efd)
{
    aset_notification *an = (aset_notification *)efd->GetData();
    free(an->f_name);
    delete an;
    int fd = efd->GetFileDescriptor();
    efd->Unregister();
	tipWindow::ResetTimer();
    vevent::StopProcessing();
}

int dis_register_aset_callback(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if(argc != 2) {
	Tcl_AppendResult(interp, argv[0], " wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    veventFD * efd = veventFD::Register(5);
    if(!efd) {
	Tcl_AppendResult(interp, argv[0], " failed to register.", NULL);
	return TCL_ERROR;
    }
    aset_notification *an = new aset_notification;
    an->f_name            = strdup(argv[1]);
    an->interp            = interp;
    efd->SetData((void*)an);
    efd->SetObserveReadProc(notification_proc);
    efd->SetObserveExceptProc(exception_proc);
    
    return TCL_OK;
}

#else

static struct aset_notification an;

void dis_server_notification(char *str)
{
    gString full_cmd = an.f_name;
    full_cmd        += " {";
    full_cmd        += str;
    full_cmd        += "}";
    gala_eval(an.interp, (char *)full_cmd);
}

int dis_register_aset_callback(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if(argc != 2) {
	Tcl_AppendResult(interp, argv[0], " wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    an.f_name            = strdup(argv[1]);
    an.interp            = interp;

    return TCL_OK;
}


#endif

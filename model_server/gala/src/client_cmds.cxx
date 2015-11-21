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

#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER
#include veventHEADER

#include <ggeneric.h>
#include <gdialog.h>
#include <gsash.h>
#include <gview.h>
#include <gviewer.h>
#include <gviewerMenu.h>
#include <ginterp.h>
#include <gString.h>
 
int Viewer::CMD_ChangeView(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	int viewer_id = atoi(argv[1]);
	int view_id = atoi(argv[2]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	if (vr) {
	    View* v = vr->find_view (view_id);
	    if (v) {
		return TCL_OK;
	    }
	}
    }

    return TCL_ERROR;
}

int Viewer::CMD_Close(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 2) {
	int viewer_id = atoi(argv[1]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	if (vr) {
	    delete vr;
	    if (!Viewer::GetFirstViewer()) vevent::StopProcessing();
	    return TCL_OK;
	}
    }

    return TCL_ERROR;
}

int Viewer::CMD_CloseView(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	int viewer_id = atoi(argv[1]);
	int view_id = atoi(argv[2]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	
	if (vr) {
	    View* v = vr->find_view (view_id);
	    if (v) {
		v->close();
		return TCL_OK;
	    }
	}
    }
    return TCL_ERROR;
}

int Viewer::CMD_Create (ClientData, Tcl_Interp* interp, int, char**)
{  
    Viewer* v = new Viewer;
    if (v) {
	gString tmp;
	tmp.sprintf((vchar *)"%d", v->id);
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
	return TCL_OK;
    } else 
	return TCL_ERROR;
    
}

int Viewer::CMD_OpenLayout(ClientData, Tcl_Interp*, int, char**)
{
    return TCL_ERROR;
}

int Viewer::CMD_Print (ClientData, Tcl_Interp*, int, char**)
{
    for (Viewer* v=GetFirstViewer(); v; v=v->next) {
	v->print();
    }
    return TCL_OK;
}

int Viewer::CMD_RemoveView(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	int viewer_id = atoi(argv[1]);
	int view_id = atoi(argv[2]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	
	if (vr) {
	    View* v = vr->find_view (view_id);
	    if (v) {
		v->remove();
		return TCL_OK;
	    }
	}
    }
    return TCL_ERROR;
}

int Viewer::CMD_SaveLayout(ClientData, Tcl_Interp*, int, char**)
{
    return TCL_ERROR;
}


int Viewer::CMD_Split(ClientData, Tcl_Interp*, int argc, char** argv)
{    
    if (argc == 4) {
	int viewer_id = atoi(argv[1]);
	int view_id = atoi(argv[2]);

        Viewer* vr = Viewer::find_viewer (viewer_id);

	if (vr) {
	    View* v = vr->find_view (view_id);
	    if (v) {
		if (strcmp(argv[3], "-horizontal") == 0)
		    v->split_horizontal();
		else if (strcmp(argv[3], "-vertical") == 0)
		    v->split_vertical();
		else return TCL_ERROR;
	    }
	}
    }
    return TCL_ERROR;
}


int Viewer::CMD_Add(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	int viewer_id = atoi(argv[1]);
	int view_id = atoi(argv[2]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	
	if (vr) {
	    View* v = vr->find_view (view_id);
	    if (v) {
		v->addPane();
		return TCL_OK;
	    }
	}
    }
    return TCL_ERROR;
}

/* Used for debugging:  showitem, hideitem */


int Viewer::CMD_ShowItem(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	int viewer_id = atoi(argv[1]);
	char* tagName = argv[2];
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	
	vr->GetMenuBar()->ShowItem ((vchar*)tagName);
    }
    return TCL_ERROR;
}


int Viewer::CMD_HideItem(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	int viewer_id = atoi(argv[1]);
	char* tagName = argv[2];
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	
	vr->GetMenuBar()->HideItem ((vchar*)tagName);
    }
    return TCL_ERROR;
}


int Viewer::CMD_ShowMenu(ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 2) {
	char* tagName = argv[1];
	char* viewer_id = Tcl_GetVar(interp, "vr", TCL_GLOBAL_ONLY);
        Viewer* vr = Viewer::find_viewer (atoi(viewer_id));
	
	vr->GetMenuBar()->ShowMenu ((vchar*)tagName);
    }
    return TCL_ERROR;
}


int Viewer::CMD_HideMenu(ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 2) {
	char* tagName = argv[1];
	char* viewer_id = Tcl_GetVar(interp, "vr", TCL_GLOBAL_ONLY);
        Viewer* vr = Viewer::find_viewer (atoi(viewer_id));
	
	vr->GetMenuBar()->HideMenu ((vchar*)tagName);
    }
    return TCL_ERROR;
}


int Viewer::CMD_SetToggle(ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 3) {
	char* tagName = argv[1];
	char* viewer_id = Tcl_GetVar(interp, "vr", TCL_GLOBAL_ONLY);
        Viewer* vr = Viewer::find_viewer (atoi(viewer_id));

	if (vcharCompare((vchar*)argv[2], "-on") == 0)
	    vr->GetMenuBar()->ActivateItem((vchar*)tagName);
	else if (vcharCompare((vchar*)argv[2], "-off") == 0)
	    vr->GetMenuBar()->DeactivateItem((vchar*)tagName);
	else
	    return TCL_ERROR;
	return TCL_OK;
    }
    else
	return TCL_ERROR;
}





/* Used for debugging:  showitem, hideitem */
int Viewer::CMD_EnableItem(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3){
		char* tagName = argv[2];
	    if (strcmp (argv[1], "-all") == 0) {
			for (Viewer* vr = Viewer::GetFirstViewer(); vr; vr=vr->next) {
				vr->GetMenuBar()->EnableItem ((vchar*)tagName);
			}
			return TCL_OK;
	    } else { 
	        int viewer_id = atoi(argv[1]);	
            Viewer* vr = Viewer::find_viewer (viewer_id);
	        vr->GetMenuBar()->EnableItem ((vchar*)tagName);
            return TCL_OK;
        }
	}
    return TCL_ERROR;
}


int Viewer::CMD_DisableItem(ClientData, Tcl_Interp*, int argc, char** argv)
{
    if (argc == 3) {
	char* tagName = argv[2];
        if (strcmp (argv[1], "-all") == 0) {
	    for (Viewer* vr = Viewer::GetFirstViewer(); vr; vr=vr->next) {
	        vr->GetMenuBar()->DisableItem ((vchar*)tagName);
	    }
	    return TCL_OK;
	} else { 
	    int viewer_id = atoi(argv[1]);	
            Viewer* vr = Viewer::find_viewer (viewer_id);
	    vr->GetMenuBar()->DisableItem ((vchar*)tagName);
            return TCL_OK;
        }
    }
    return TCL_ERROR;
}


int Viewer::CMD_EnableMenu(ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 2) {
	char* tagName = argv[1];
	char* viewer_id = Tcl_GetVar(interp, "vr", TCL_GLOBAL_ONLY);
        Viewer* vr = Viewer::find_viewer (atoi(viewer_id));
	
	vr->GetMenuBar()->EnableMenu ((vchar*)tagName);
        return TCL_OK;
    }
    return TCL_ERROR;
}


int Viewer::CMD_DisableMenu(ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 2) {
	char* tagName = argv[1];
	char* viewer_id = Tcl_GetVar(interp, "vr", TCL_GLOBAL_ONLY);
        Viewer* vr = Viewer::find_viewer (atoi(viewer_id));
	
	vr->GetMenuBar()->DisableMenu ((vchar*)tagName);
        return TCL_OK;
    }
    return TCL_ERROR;
}

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
// interp.cxx - Gala application TCL interpreter functions.
//
//**********

#include <string.h>
#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vcolorchsrHEADER
#include vcolorHEADER
#include vresourceHEADER
#include vmenubarHEADER
#include vmenuHEADER
#include vtextitemHEADER

#include <ggeneric.h>
#include <gprefs.h>
#include <ginterp.h>
#include <gview.h>
#include <gviewer.h>
#include <grtlClient.h>
#include <gpaneObjects.h>
#include <gprefs.h>
#include <gapl_menu.h>
#include <dis_browser.h>
#include <dis_confirm.h>
#include <glayer.h>

//void client_tcl_init (Tcl_Interp* interp);

extern Tcl_CmdProc dis_puts;
extern Tcl_CmdProc dis_GetPid;
extern Tcl_CmdProc dis_GetSystemName;
extern Tcl_CmdProc dis_GetMyServiceName;
extern Tcl_CmdProc dis_GetMyGroupName;
extern Tcl_CmdProc dis_StartService;
extern Tcl_CmdProc dis_StopService;
extern Tcl_CmdProc dis_AddViewFileScanner;
extern Tcl_CmdProc dis_DateTime;
extern Tcl_CmdProc dis_putenv;

extern void register_gala_specific_commands(Tcl_Interp* interp);
extern int eval_StringResource(Tcl_Interp* interp, vchar* resourceName);

int Tcl_InterpDeleteEvent::Handle(void) {
   if (GetClientData()) delete_interpreter((Tcl_Interp *)GetClientData());
   return TRUE;
}

int dis_IsNTCmd(ClientData, Tcl_Interp* interp, int argc, char**)
{
#ifdef _WIN32
    char *ret_str = "1";
#else
    char *ret_str = "0";
#endif
    Tcl_SetResult (interp, ret_str, TCL_STATIC);
    return TCL_OK;
}

int gala_sourceCmd(ClientData, Tcl_Interp* interp, int argc, char **argv)
{
    if(argc != 2){
	Tcl_SetResult (interp, "usage: gala_source file_name", TCL_STATIC);
	return TCL_ERROR;
    }
    char *fname = argv[1];
    int status  = eval_StringResource(interp, (vchar *)fname);
    return status;
}

g_prefs_colorchooser (ClientData, Tcl_Interp*, int argc, char**)
{
    if (argc == 1)
    {
	if (Viewer::GetCurrentColorChooser())
	    Viewer::GetCurrentColorChooser()->Raise();
	else
	{
	    ColorDialog * dialog = new ColorDialog (vloadableDONT_LOAD);
	    Viewer::SetCurrentColorChooser(dialog);
		dialog->initialize();
	}
	return TCL_OK;
    }
    return TCL_ERROR;
}
    
g_prefs_changescheme (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    if (argc == 1)
    {
	PaletteChooser::CastDown (Viewer::GetCurrentColorChooser()->FindItem (vnameInternGlobalLiteral ("PaletteChooser")))
	    ->LoadScheme();
	return TCL_OK;
    }
    return TCL_ERROR;
}


g_prefs_cancelscheme (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    if (argc == 1)
    {
	PaletteChooser::CastDown (Viewer::GetCurrentColorChooser()->FindItem (vnameInternGlobalLiteral ("PaletteChooser")))
	    ->CancelScheme();
	return TCL_OK;
    }
    return TCL_ERROR;
}

g_prefs_createscheme (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    if (argc == 2)
    {
	PaletteChooser::CastDown (Viewer::GetCurrentColorChooser()->FindItem (vnameInternGlobalLiteral ("PaletteChooser")))
	    ->CreateScheme((vname *)vnameInternGlobalLiteral (argv[1]));
	return TCL_OK;
    }
    else
	return TCL_ERROR;
}

g_file_print (ClientData, Tcl_Interp*, int argc, char* argv[])
{
     if (argc == 3) {	
	int viewer_id = atoi(argv[1]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	if (vr) {
     	    vr->print();
	    return TCL_OK;
	}
    }

    return TCL_ERROR;
}

g_viewer_close (ClientData, Tcl_Interp*, int argc, char* argv[])
{
     if (argc == 2) {	
	int viewer_id = atoi(argv[1]);
	
        Viewer* vr = Viewer::find_viewer (viewer_id);
	if (vr) {
	    delete vr;
	    return TCL_OK;
	}
    }

    return TCL_ERROR;
}

g_viewer_view_add (ClientData, Tcl_Interp*, int argc, char* argv[])
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

g_viewer_view_remove (ClientData, Tcl_Interp*, int argc, char* argv[])
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

g_viewer_view_split (ClientData, Tcl_Interp*, int argc, char* argv[])
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

g_make_browser (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    if (argc != 3)
	return TCL_ERROR;

    int viewer_id = atoi (argv[1]);
    int view_id = atoi (argv[2]);

    Viewer* vr = Viewer::find_viewer (viewer_id);
    if (!vr)
	return TCL_ERROR;

    View* v = vr->find_view (view_id);
    if (!v)
	return TCL_ERROR;

    // Create the browser.
//    browserLayer* bl = new browserLayer;
//    bl->displayInView (v);
    printf("g_make_browser not implemented.\n");

    return TCL_OK;
}

g_viewer_view_zoom (ClientData, Tcl_Interp*,
				int /* argc */, char* /* argv */ [])
{
     return TCL_ERROR;
}

g_list_init (ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 4) {
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

	    glistItem* list = (glistItem*)(v->FindItem((vchar*)argv[1]));
	    if (list) list->Initialize((vchar*)argv[2], (vchar*)argv[3],v);
	    return TCL_OK;
	}
    }

return TCL_ERROR;
}

 
g_button_init (ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc >= 3) {
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

	    gbutton* button = gbutton::CastDown(v->FindItem((vchar*)argv[1]));
	    if (button) button->Initialize((vchar*)argv[2], v);
	    return TCL_OK;
	}
    }

return TCL_ERROR;
}


static int g_list_select (ClientData,
			Tcl_Interp* interp, int argc, char** argv)
{
    if (argc == 3) {

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

	    View* vp = v->FindGroupParent ();
	    if (!vp) {
	        printf ("Can't find group parent.\n");
	        return TCL_ERROR;
	    }

	    gString lstr = (vchar*) argv[1];

	    if (lstr == (vchar*) "CategoriesList") {
	        rtlClient* el = rtlClient::CastDown
				(vp->FindItem ((vchar*) "ElementsList"));
		rtlClient* rl = rtlClient::CastDown
				(vp->FindItem ((vchar*) "ResultsList"));
	        if (!el)
	            return TCL_ERROR;
	        el->queryCategories ((vchar*) argv[2]);
		if (rl)
		    rl->clear ();
	    }
	    else if (lstr == (vchar*) "AskList") {
	        rtlClient* el = rtlClient::CastDown
				(vp->FindItem ((vchar*) "ElementsList"));
		rtlClient* rl = rtlClient::CastDown
				(vp->FindItem ((vchar*) "ResultsList"));
		if (!el || !rl)
		    return TCL_ERROR;
		rl->queryLinks (el, (vchar*) argv[2]);
	    }

	    return TCL_OK;
	}
    }

    return TCL_ERROR;
}



g_messageBox_init (ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc >= 3) {
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

	    gmessageBox* box = gmessageBox::CastDown((v->FindItem((vchar*)argv[1])));
	    if (box) box->Initialize((vchar*)argv[2], v);
	    return TCL_OK;
	}
    }

return TCL_ERROR;
}


g_textbox_get (ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc >= 2) {
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

	    vtextitem* box = vtextitem::CastDown((v->FindItem((vchar*)argv[1])));
	    if (box) 
		Tcl_SetResult (interp, (char*)box->GetTextAsString(), TCL_STATIC);
	    return TCL_OK;
	}
    }

return TCL_ERROR;
}


g_textbox_set (ClientData, Tcl_Interp* interp, int argc, char** argv)
{
    if (argc >= 3) {
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

	    vtextitem* box = vtextitem::CastDown((v->FindItem((vchar*)argv[1])));
	    if (box) 
		box->SetText((vchar*)argv[2]);
	    return TCL_OK;
	}
    }

return TCL_ERROR;
}


 
g_init_text (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    if (argc == 5) {
        Viewer* vr = Viewer::find_viewer (atoi(argv[1]));
        if (vr) {
            View* v = vr->find_view (atoi(argv[2]));
 
            if (v) {
		vtextitem* text = (vtextitem*)(v->FindItem((vchar*)argv[3]));
		if (text)  {
		    text->CreateTextData();
		    text->SetText((vchar*)argv[4]);
		}
		return TCL_OK;
            }
        }
    }
 
    return TCL_ERROR;
}


 
g_make_target (ClientData, Tcl_Interp* interp, int argc, char* [])
{
    if (argc == 1) {
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

	    v->target(vTRUE);
	    return TCL_OK;
	}
    }
    return TCL_ERROR;
}



Tcl_Interp* make_interpreter()
{
    Tcl_Interp *interp;
    
    interp = Tcl_CreateInterp();
    Tcl_Init (interp);

    //client_tcl_init (interp);

    rtlClient::addCommands (interp);
    init_interpreter (interp);
    return interp;
}

int dis_get_file_directory (ClientData, Tcl_Interp* interp, int argc, char** argv)
{
  char pszDir[ 512 ];
  if( argc > 1 )
  {
    char *pLastBackSlash = NULL;
    strcpy( pszDir, argv[1] );
    char *pLastSlash = strrchr( pszDir, '/' );
#ifdef _WIN32
    pLastBackSlash = strrchr( pszDir, '\\' );
#endif
    if( pLastBackSlash > pLastSlash )
      pLastSlash = pLastBackSlash;
    if( pLastSlash )
      pLastSlash[1] = '\0';
  }
  else
    strcpy( pszDir, "" );
  Tcl_SetResult( interp, pszDir, TCL_VOLATILE );
  return TCL_OK;
}

void init_interpreter (Tcl_Interp *interp) {

    Tcl_CreateCommand (interp, "dis_ViewerChangeView", Viewer::CMD_ChangeView, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerClose", Viewer::CMD_Close, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerCloseView", Viewer::CMD_CloseView, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerCreate", Viewer::CMD_Create, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerOpenLayout", Viewer::CMD_OpenLayout, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerRemoveView", Viewer::CMD_RemoveView, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerSaveLayout", Viewer::CMD_SaveLayout, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerSplit", Viewer::CMD_Split, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerPrint", Viewer::CMD_Print, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerAdd", Viewer::CMD_Add, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerShowItem", Viewer::CMD_ShowItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerHideItem", Viewer::CMD_HideItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerShowMenu", Viewer::CMD_ShowMenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerHideMenu", Viewer::CMD_HideMenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerEnableItem", Viewer::CMD_EnableItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerDisableItem", Viewer::CMD_DisableItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerEnableMenu", Viewer::CMD_EnableMenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerDisableMenu", Viewer::CMD_DisableMenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewerSetToggle", Viewer::CMD_SetToggle
, NULL, NULL);

    Tcl_CreateCommand (interp, "g_init_text", g_init_text, NULL, NULL);
    Tcl_CreateCommand (interp, "g_list_init", g_list_init, NULL, NULL);
    Tcl_CreateCommand (interp, "g_button_init", g_button_init, NULL, NULL);
    Tcl_CreateCommand (interp, "g_list_select", g_list_select, NULL, NULL);
    Tcl_CreateCommand (interp, "g_textbox_get", g_textbox_get, NULL, NULL);
    Tcl_CreateCommand (interp, "g_textbox_set", g_textbox_set, NULL, NULL);
    Tcl_CreateCommand (interp, "g_messageBox_init", g_messageBox_init, NULL, NULL);

    Tcl_CreateCommand (interp, "g_file_print", g_file_print, NULL, NULL);

    Tcl_CreateCommand (interp, "g_prefs_colorchooser", g_prefs_colorchooser, NULL, NULL);
    Tcl_CreateCommand (interp, "g_prefs_changescheme", g_prefs_changescheme, NULL, NULL);
    Tcl_CreateCommand (interp, "g_prefs_cancelscheme", g_prefs_cancelscheme, NULL, NULL);
    Tcl_CreateCommand (interp, "g_prefs_createscheme", g_prefs_createscheme, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_MenuCreate",             dis_MenuCreate, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAppendItem",         dis_MenuAppendItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAppendLabelItem",    dis_MenuAppendLabelItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAppendToggleItem",   dis_MenuAppendToggleItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAppendSubmenu",      dis_MenuAppendSubmenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAppendGroupSubmenu", dis_MenuAppendGroupSubmenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAppendSeparator",    dis_MenuAppendSeparator, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuCreateDynamic",      dis_MenuCreateDynamic, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuCreateGroup",        dis_MenuCreateGroup, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAddMenuArgument",    dis_MenuAddMenuArgument, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuAddItemArgument",    dis_MenuAddItemArgument, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuSetToggleItem",      dis_MenuSetToggleItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuItemEnable",         dis_MenuItemEnable, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_MenuItemDisable",        dis_MenuItemDisable, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_layer", dis_layer, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_file", dis_file, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_view_manage", dis_view_manage, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_customize_toolbar", dis_customize_toolbar, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_build_menu", dis_build_menu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_browser_get_askList", dis_browser_get_askList, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_browser_get_categoriesList", dis_browser_get_categoriesList, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_browser_update_categoriesList", dis_browser_update_categoriesList, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_browser_update_askList_for_categories", dis_browser_update_askList_for_categories, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_browser_convert_ask_selection", dis_browser_convert_ask_selection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_browser_convert_category_selection", dis_browser_convert_category_selection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_browser_update_askList_for_rtl", dis_browser_update_askList_for_rtl, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_SetViewVariables", dis_SetViewVariables, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_view", dis_view_cl, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_viewer_new", dis_viewer_new, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_launch_dialog", dis_launch_dialog, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_launch_selBrowser", dis_launch_selBrowser, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_close_dialog", dis_close_dialog, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_prompt", dis_prompt, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_prompt_file", dis_prompt_file, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_confirm1", dis_confirm1, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_confirm2", dis_confirm2, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_confirm3", dis_confirm3, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_confirm_list", dis_confirm_list, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_prompt_string1", dis_prompt_string1, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_prompt_string2", dis_prompt_string2, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_prompt_string3", dis_prompt_string3, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_prompt_bool1", dis_prompt_bool1, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_prompt_bool2", dis_prompt_bool2, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_prompt_bool3", dis_prompt_bool3, NULL, NULL);

	Tcl_CreateCommand (interp, "dis_prompt_int1", dis_prompt_int1, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_prompt_int2", dis_prompt_int2, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_prompt_int3", dis_prompt_int3, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_TimerRet", dis_TimerRet, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_Enable_MenuToolBar", dis_Enable_MenuToolBar, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_close_confirmDialog", dis_close_confirmDialog, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_setvar", dis_setvar, NULL, NULL);    
    Tcl_CreateCommand (interp, "dis_getvar", dis_getvar, NULL, NULL);    

    Tcl_CreateCommand (interp, "dis_gdInitButton", dis_gdInitButton, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitDrawing", dis_gdInitDrawing, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitNotebookPage", dis_gdInitNotebookPage, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitListItem", dis_gdInitListItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitListView", dis_gdInitListView, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_gdInitNumberSpinner", dis_gdInitNumberSpinner, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_gdNumberSpinner_SetMin", dis_gdNumberSpinner_SetMin, NULL, NULL);
	Tcl_CreateCommand (interp, "dis_gdNumberSpinner_SetMax", dis_gdNumberSpinner_SetMax, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_gdInitToggle", dis_gdInitToggle, NULL, NULL);   
    Tcl_CreateCommand (interp, "dis_gdInitTextItem", dis_gdInitTextItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitExclusiveGroup", dis_gdInitExclusiveGroup, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitComboBox", dis_gdInitComboBox, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitPopDownComboBox", dis_gdInitPopDownComboBox, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitLabel", dis_gdInitLabel, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitRuler", dis_gdInitRuler, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitProgressItem", dis_gdInitProgressItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitSlider", dis_gdInitSlider, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdInitSash", dis_gdInitSash, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_gdDrawingSetGrid", dis_gdDrawingSetGrid, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingSetGridDimensions", dis_gdDrawingSetGridDimensions, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingSetIconTitle", dis_gdDrawingSetIconTitle, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingSetIconTip", dis_gdDrawingSetIconTip, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingAddItem", dis_gdDrawingAddItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingRemoveItem", dis_gdDrawingRemoveItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingSetDragProhibited", dis_gdDrawingSetDragProhibited, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingSetSingleSelection", dis_gdDrawingSetSingleSelection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingClear", dis_gdDrawingClear, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDrawingSelectItem", dis_gdDrawingSelectItem, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_gdInitAttributeEditor", dis_gdInitAttributeEditor, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AttributeEditorAddAttribute", dis_gdAttributeEditorAddAttribute, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AttributeEditorAddEnumAttributeValue", dis_gdAttributeEditorAddEnumAttributeValue, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AttributeEditorSetAttributeValue", dis_gdAttributeEditorSetAttributeValue, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AttributeEditorReset", dis_gdAttributeEditorReset, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_current_filter_name", dis_current_filter_name, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_print", dis_print, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_quit", dis_quit, NULL, NULL);

    
    Tcl_CreateCommand (interp, "dis_ExecuteInSubDirs", dis_ExecuteInSubDirs, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirCreate", dis_DirCreate, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirExpandVariables", dis_DirExpandVariables, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirFindFileFromPath", dis_DirFindFileFromPath, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSize", dis_DirSize, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirItemName", dis_DirItemName, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirItemFullname", dis_DirItemFullname, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirItem", dis_DirItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirRefresh", dis_DirRefresh, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirIsDirectory", dis_DirIsDirectory, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirGetGlobalBeforeFlags", dis_DirGetGlobalBeforeFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetGlobalBeforeFlags", dis_DirSetGlobalBeforeFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirGetGlobalAfterFlags", dis_DirGetGlobalAfterFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetGlobalAfterFlags", dis_DirSetGlobalAfterFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirGetGlobalFilter", dis_DirGetGlobalFilter, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetGlobalFilter", dis_DirSetGlobalFilter, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetExcludeFlag", dis_DirSetExcludeFlag, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirGetExcludeFlag", dis_DirGetExcludeFlag, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirDetermineFlags", dis_DirDetermineFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirGetFlags", dis_DirGetFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetFlags", dis_DirSetFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirIsFlags", dis_DirIsFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirGetFilter", dis_DirGetFilter, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetFilter", dis_DirSetFilter, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSetCompilerFlags", dis_DirSetCompilerFlags, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirSave", dis_DirSave, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirRestore", dis_DirRestore, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirDestroy", dis_DirDestroy, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirDetermineFilter", dis_DirDetermineFilter, NULL, NULL);


    Tcl_CreateCommand (interp, "dis_SetPollProc", dis_SetPollProc, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_StopPollProc", dis_StopPollProc, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_CountPDFfiles", dis_CountPDFfiles, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GeneratePDF", dis_GeneratePDF, NULL, NULL);
    
    Tcl_CreateCommand (interp, "dis_ViewFile", dis_ViewFile, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AddViewFileScanner", dis_AddViewFileScanner, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_StopViewUpdating", dis_StopViewUpdating, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_BlockSignals", dis_BlockSignals, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetText", dis_GetText, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewText", dis_ViewText, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ViewHighliteLine", dis_ViewHighliteLine, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_EvalFile", dis_EvalFile, NULL, NULL);
    Tcl_CreateCommand (interp, "reval_async", reval_async, NULL, NULL);
    Tcl_CreateCommand (interp, "reval", reval, NULL, NULL);
    Tcl_CreateCommand (interp, "leval", leval, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_SystemCommand", dis_SystemCommand, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_SystemCommandWait", dis_SystemCommandWait, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_KillSystemCommand", dis_KillSystemCommand, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DevNull", dis_DevNull, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GalaLogFile", dis_GalaLogFile, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DirMakeDirectory", dis_DirMakeDirectory, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetAppDirectory", dis_GetAppDirectory, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetStartupDirectory", dis_GetStartupDirectory, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_Message", dis_Message, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ItemEnable", dis_ItemEnable, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ItemHide", dis_ItemHide, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ItemName", dis_ItemName, NULL, NULL);
    Tcl_CreateCommand (interp, "g_make_browser", g_make_browser, NULL, NULL);
    Tcl_CreateCommand (interp, "g_make_target", g_make_target, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ProgressStartTime", dis_ProgressStartTime, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ProgressElapsedTime", dis_ProgressElapsedTime, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ProgressEstimatedRelative", dis_ProgressEstimatedRelative, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ProgressEstimatedEnd", dis_ProgressEstimatedEnd, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ProgressStart", dis_ProgressStart, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_getListStrRes", dis_getListStrRes, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_getTextStrRes", dis_getTextStrRes, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_putStringPreference", dis_putStringPreference, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_getStringPreference", dis_getStringPreference, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_help", dis_help, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_eval", dis_eval, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextItemSetText", dis_gdTextItemSetText, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextItemGetText", dis_gdTextItemGetText, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdAddToCombo", dis_gdAddToCombo, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdAddListToCombo", dis_gdAddListToCombo, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdClearCombo", dis_gdClearCombo, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdComboSelect", dis_gdComboSelect, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdComboBoxSetMatch", dis_gdComboBoxSetMatch, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdNotebookPageEnable", dis_gdNotebookPageEnable, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_service_connect_remote", dis_service_connect_remote, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_service_connect", dis_service_connect, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_service_disconnect", dis_service_disconnect, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListItemRefresh", dis_gdListItemRefresh, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListItemSelect", dis_gdListItemSelect, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListItemSelection", dis_gdListItemSelection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListItem_init_width", dis_gdListItem_init_width, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewRemoveRow", dis_gdListViewRemoveRow, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewFlush",     dis_gdListViewFlush, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewSelect", dis_gdListViewSelect, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewRefresh", dis_gdListViewRefresh, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewAddPopupItem", dis_gdListViewAddPopupItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewUseInternalData", dis_gdListViewUseInternalData, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewSetCellMenu", dis_gdListViewSetCellMenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewSetData", dis_gdListViewSetData, NULL, NULL);


	Tcl_CreateCommand (interp, "dis_gdTextSpinSelectByIndex",     dis_gdTextSpinSelectByIndex,     NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinSelectByText",      dis_gdTextSpinSelectByText,      NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinAddText",           dis_gdTextSpinAddText,           NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinChangeText",        dis_gdTextSpinChangeText,        NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinClear",             dis_gdTextSpinClear,             NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinDeleteText",        dis_gdTextSpinDeleteText,        NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinDeleteIndex",       dis_gdTextSpinDeleteIndex,       NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinGetSelectedText",   dis_gdTextSpinGetSelectedText,   NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinGetSelectedIndex",  dis_gdTextSpinGetSelectedIndex,  NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinGetTextAt",         dis_gdTextSpinGetTextAt,         NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinGetLinesAnount",    dis_gdTextSpinGetLinesAnount,    NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinGetIndexFromText",  dis_gdTextSpinGetIndexFromText,  NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdTextSpinSetChangeCallback", dis_gdTextSpinSetChangeCallback, NULL, NULL);



    Tcl_CreateCommand (interp, "dis_gdListViewAddCheckCell",     dis_gdListViewAddCheckCell, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewChangeCheckIcons", dis_gdListViewChangeCheckIcons, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewChangeCheckStatus", dis_gdListViewChangeCheckStatus, NULL, NULL);


    Tcl_CreateCommand (interp, "dis_gdListViewSetChangeCallback", dis_gdListViewSetChangeCallback, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewSetCellChange",     dis_gdListViewSetCellChange, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewSetDblClickCallback", dis_gdListViewSetDblClickCallback, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewGetCellData", dis_gdListViewGetCellData, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdListViewGetCellValue", dis_gdListViewGetCellValue, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDialogEval", dis_gdDialogEval, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDialogIDEval", dis_gdDialogIDEval, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_gdDialogSetTitle", dis_gdDialogSetTitle, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_stop", dis_stop, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_rtl_get_id", dis_rtl_get_id, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_rtl_select", dis_rtl_select, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_rtl_set_caller", dis_rtl_set_caller, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_rtl_clear", dis_rtl_clear, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_rtl_preserve_selection", dis_rtl_preserve_selection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_query", dis_query, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ide_send_command", dis_ide_send_command, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ide_get_selection", dis_ide_get_selection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_IDE_current_selection", dis_IDE_current_selection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_viewer_pop_active", dis_viewer_pop_active, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_rtl_post_init", dis_rtl_post_init, NULL, NULL);
    Tcl_CreateCommand (interp, "error_Message", error_Message, NULL, NULL);	

    Tcl_CreateCommand (interp, "dis_aset_eval_cmd", dis_aset_eval_cmd, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_aset_eval_cmd_async", dis_aset_eval_cmd_async, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_java_build", dis_java_build, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_java_follow", dis_java_follow, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_java_destroy", dis_java_destroy, NULL, NULL);

    Tcl_CreateCommand (interp, "dis_build_service_list", dis_build_service_list, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_get_service_list_size", dis_get_service_list_size, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_get_service_list_entry", dis_get_service_list_entry, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_get_service_list_index", dis_get_service_list_index, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_SelectionFromList", dis_SelectionFromList, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_puts", dis_puts, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetPid", dis_GetPid, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetSystemName", dis_GetSystemName, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetMyServiceName", dis_GetMyServiceName, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_GetMyGroupName", dis_GetMyGroupName, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_StartService", dis_StartService, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_StopService", dis_StopService, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_file_write", dis_file_write, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_file_read", dis_file_read, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_DateTime", dis_DateTime, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_getenv", dis_getenv, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_putenv", dis_putenv, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_PasteViewerSelection", dis_PasteViewerSelection, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_ide_open_definition", dis_ide_open_definition, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_activate_notebook_page", dis_activate_notebook_page, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_TextItemSetModifiable", dis_TextItemSetModifiable, NULL, NULL);
		Tcl_CreateCommand (interp, "dis_tempnam", dis_tempnam, NULL, NULL);
		Tcl_CreateCommand (interp, "dis_unlink", dis_unlink, NULL, NULL);
		Tcl_CreateCommand (interp, "dis_update_gdTextItem", dis_update_gdTextItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_get_file_directory", dis_get_file_directory, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AccessAddMenu", dis_AccessAddMenu, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_AccessAddItem", dis_AccessAddItem, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_sel_size", dis_sel_size, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_register_aset_callback", dis_register_aset_callback, NULL, NULL);
    Tcl_CreateCommand (interp, "dis_isNT", dis_IsNTCmd, NULL, NULL);
    Tcl_CreateCommand (interp, "gala_source", gala_sourceCmd, NULL, NULL);
    register_gala_specific_commands( interp );
}

void delete_interpreter (Tcl_Interp* interp)
{

    Tcl_DeleteInterp (interp);
}

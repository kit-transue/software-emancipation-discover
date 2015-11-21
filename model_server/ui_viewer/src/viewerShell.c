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

/*******************************************************************************
	viewerShell.c
	(Generated from interface file viewerShell.if)
*******************************************************************************/

#include <stdio.h>
#include "UxLib.h"
#include "UxPaneW.h"
#include "UxTextF.h"
#include "UxFrame.h"
#include "UxCascB.h"
#include "UxTogB.h"
#include "UxSep.h"
#include "UxPushB.h"
#include "UxRowCol.h"
#include "UxForm.h"
#include "UxTopSh.h"

/*******************************************************************************
	Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include <varargs.h>
#include <systemMessages.h>
#include <perspective.h>
#include <representation.h>
#include <viewer_modes.h>
#include <viewerShell_manage.h>
#include <top_widgets.h>
#include <FileSelector.h>
#include <Panner.h>
#include <button_bar.h>
#include <help.h>
#include <xref_queries.h>
#include <paraDebug_vs_c.h>
#include <uimx_context.h>
#include <Xm/Protocols.h>
#define CONTEXT_PTR CONTEXT_PTR_NAME(ViewerShell)
#define CURRENT_CONTEXT CURRENT_CONTEXT_NAME(ViewerShell)
void    miniBrowser_initialize_slide_button();
void    miniBrowser_initialize_slide_separator();
char*	gettext();
void	popup_quick_search();
void    viewerShell_ExpandView();
void*	viewerShell_get_current_view();
void	viewerShell_panner_callback();
void	viewerShell_app_save();
void	viewerShell_app_save_as();
int	viewerShell_finalize();
/*void    viewerShell_manage_home_project();*/
void    viewerShell_manage();
void    viewerShell_open_Inheritance();
void    viewerShell_new_dir();
void    viewerShell_file_load();
void    viewerShell_file_unload();
void    viewerShell_file_update();
void    viewerShell_set_miniBrowser();
void*   gtShell_icon_pixmap();
void*	gtShell_icon_mask();
void    viewerShell_close_button_callback (); 
typedef struct
{
    void*	vshell;
    Widget	panner;
    Widget	filesel;
    int		interp_mode;
    int		debug_mode;
}
instance_data;
/* Link status flags. */
#define APPTREE_HAS_NEXT       (1<<0)
#define APPTREE_HAS_PREV       (1<<1)
#define APPTREE_IS_LINK_TARGET (1<<2)
#define APPTREE_IS_LINK_SOURCE (1<<3)
static char str_1_1[] = "1 : 1";
static char str_1_n[] = "1 : N";
static char str_n_1[] = "N : 1";
static char str_n_n[] = "N : N";
static char str_cl_svr[] = "cl/svr";
static char str_svr_cl[] = "svr/cl";
static char str_pr_pr[] = "pr/pr";

/*******************************************************************************
	The definition of the context structure:
	If you create multiple instances of your interface, the context
	structure ensures that your callbacks use the variables for the
	correct instance.

	For each swidget in the interface, each argument to the Interface
	function, and each variable in the Instance Specific section of the
	Declarations Editor, there is an entry in the context structure.
	and a #define.  The #define makes the variable name refer to the
	corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	swidget	UxViewerShell;
	swidget	Uxform1;
	swidget	Uxbrowser_icon;
	swidget	Uxmenubar;
	swidget	Uxfile_menu;
	swidget	Uxfile_open;
	swidget	Uxfile_expand;
	swidget	Uxfile_open_defn;
	swidget	Uxfile_open_flowchart;
	swidget	Uxfile_open_vert_hierarchy;
	swidget	Uxfile_open_erd;
	swidget	Uxfile_open_dc;
	swidget	Uxfile_open_inheritance;
	swidget	Uxfile_open_calltree;
	swidget	Uxfile_s01;
	swidget	Uxfile_load;
	swidget	Uxfile_unload;
	swidget	Uxfile_update;
	swidget	Uxfile_s1;
	swidget	Uxfile_save;
	swidget	Uxfile_save_btn_bar;
	swidget	Uxfile_s2;
	swidget	Uxfile_close_view;
	swidget	Uxfile_print;
	swidget	Uxfile_s3;
	swidget	Uxfile_close_window;
	swidget	Uxfile;
	swidget	Uxedit_menu;
	swidget	Uxedit_cut;
	swidget	Uxedit_copy;
	swidget	Uxedit_paste;
	swidget	Uxedit_s1;
	swidget	Uxedit_search;
	swidget	Uxedit_s2;
	swidget	Uxedit_change_editor;
	swidget	Uxedit_s3;
	swidget	Uxedit_clear_selection;
	swidget	Uxedit;
/*	swidget	Uxmanage_menu;
	swidget	Uxmanage_home_project;
	swidget	Uxmanage_s01;
	swidget	Uxmanage_get1;
	swidget	Uxmanage_get2;
	swidget	Uxmanage_put;
	swidget	Uxmanage_s1;
	swidget	Uxmanage_copy;
	swidget	Uxmanage_diff;
	swidget	Uxmanage_lock;
	swidget	Uxmanage_unlock;
	swidget	Uxmanage_s2;
	swidget	Uxmanage_unget;
	swidget	Uxmanage; */
	swidget	Uxview_menu;
	swidget	Uxview_options_menu;
	swidget	Uxview_options_mode_buttons;
	swidget	Uxview_options_buttonbar;
	swidget	Uxview_options_decorate;
	swidget	Uxview_options;
	swidget	Uxview_s0;
	swidget	Uxview_split;
	swidget	Uxviewer_remove;
	swidget	Uxview_s1;
	swidget	Uxview_collapse;
	swidget	Uxview_expand_level_menu;
	swidget	Uxview_expand_level_b1;
	swidget	Uxview_expand_level_b2;
	swidget	Uxview_expand_level_b3;
	swidget	Uxview_expand_level_b4;
	swidget	Uxview_expand_level_b5;
	swidget	Uxview_expand_level;
	swidget	Uxgra_fresh;
	swidget	Uxview_s2;
	swidget	Uxview_magnify;
	swidget	Uxview_shrink;
	swidget	Uxview_zoomtofit;
	swidget	Uxview_reset;
	swidget	Uxview_s3;
	swidget	Uxview_pref;
	swidget	Uxview;
	swidget	Uxhelp_menu;
	swidget	Uxhelp_TOC;
	swidget	Uxhelp_index;
	swidget	Uxhelp_s1;
	swidget	Uxhelp_viewer_menus;
	swidget	Uxhelp_on_viewers;
	swidget	Uxhelp_on_help;
	swidget	Uxhelp_s2;
	swidget	Uxhelp_release_notes;
	swidget	Uxhelp_getting_started;
	swidget	Uxhelp_graphics_conventions;
	swidget	Uxhelp;
	swidget	Uxcustom_buttons_form;
	swidget	Uxcustom_buttons_controls;
	swidget	Uxcustomize_mode;
	swidget	Uxremove_mode;
	swidget	Uxnormal_mode;
	swidget	Uxcustom_buttons_s1;
	swidget	Uxcustom_buttons_slot;
	swidget	Uxbuttons_form;
	swidget	Uxbuttons_s1;
	swidget	Uxbuttons_rowcol;
	swidget	Uxseparator2;
	swidget	UxmodeButtonSlot;
	swidget	Uxrelmode_1_1;
	swidget	Uxrelmode_1_many;
	swidget	Uxrelmode_many_1;
	swidget	Uxrelmode_many_many;
	swidget	Uxseparator1;
	swidget	Uxpanner_frame;
	swidget	Uxstatus_line;
	swidget	UxviewWindowForm;
	swidget	UxviewWindow;
	swidget	UxviewSepBut;
	swidget	UxviewListSep;
	swidget	UxviewList;
	instance_data	Uxinstance;
	void	*Uxvshell_obj;
	int	Uxcreate_minibrowser;
} _UxCViewerShell;

#define ViewerShell             UxViewerShellContext->UxViewerShell
#define form1                   UxViewerShellContext->Uxform1
#define browser_icon            UxViewerShellContext->Uxbrowser_icon
#define menubar                 UxViewerShellContext->Uxmenubar
#define file_menu               UxViewerShellContext->Uxfile_menu
#define file_open               UxViewerShellContext->Uxfile_open
#define file_expand             UxViewerShellContext->Uxfile_expand
#define file_open_defn          UxViewerShellContext->Uxfile_open_defn
#define file_open_flowchart     UxViewerShellContext->Uxfile_open_flowchart
#define file_open_vert_hierarchy UxViewerShellContext->Uxfile_open_vert_hierarchy
#define file_open_erd           UxViewerShellContext->Uxfile_open_erd
#define file_open_dc            UxViewerShellContext->Uxfile_open_dc
#define file_open_inheritance   UxViewerShellContext->Uxfile_open_inheritance
#define file_open_calltree      UxViewerShellContext->Uxfile_open_calltree
#define file_s01                UxViewerShellContext->Uxfile_s01
#define file_load               UxViewerShellContext->Uxfile_load
#define file_unload             UxViewerShellContext->Uxfile_unload
#define file_update             UxViewerShellContext->Uxfile_update
#define file_s1                 UxViewerShellContext->Uxfile_s1
#define file_save               UxViewerShellContext->Uxfile_save
#define file_save_btn_bar       UxViewerShellContext->Uxfile_save_btn_bar
#define file_s2                 UxViewerShellContext->Uxfile_s2
#define file_close_view         UxViewerShellContext->Uxfile_close_view
#define file_print              UxViewerShellContext->Uxfile_print
#define file_s3                 UxViewerShellContext->Uxfile_s3
#define file_close_window       UxViewerShellContext->Uxfile_close_window
#define file                    UxViewerShellContext->Uxfile
#define edit_menu               UxViewerShellContext->Uxedit_menu
#define edit_cut                UxViewerShellContext->Uxedit_cut
#define edit_copy               UxViewerShellContext->Uxedit_copy
#define edit_paste              UxViewerShellContext->Uxedit_paste
#define edit_s1                 UxViewerShellContext->Uxedit_s1
#define edit_search             UxViewerShellContext->Uxedit_search
#define edit_s2                 UxViewerShellContext->Uxedit_s2
#define edit_change_editor      UxViewerShellContext->Uxedit_change_editor
#define edit_s3                 UxViewerShellContext->Uxedit_s3
#define edit_clear_selection    UxViewerShellContext->Uxedit_clear_selection
#define edit                    UxViewerShellContext->Uxedit
/*
#define manage_menu             UxViewerShellContext->Uxmanage_menu
#define manage_home_project     UxViewerShellContext->Uxmanage_home_project
#define manage_s01              UxViewerShellContext->Uxmanage_s01
#define manage_get1             UxViewerShellContext->Uxmanage_get1
#define manage_get2             UxViewerShellContext->Uxmanage_get2
#define manage_put              UxViewerShellContext->Uxmanage_put
#define manage_s1               UxViewerShellContext->Uxmanage_s1
#define manage_copy             UxViewerShellContext->Uxmanage_copy
#define manage_diff             UxViewerShellContext->Uxmanage_diff
#define manage_lock             UxViewerShellContext->Uxmanage_lock
#define manage_unlock           UxViewerShellContext->Uxmanage_unlock
#define manage_s2               UxViewerShellContext->Uxmanage_s2
#define manage_unget            UxViewerShellContext->Uxmanage_unget
#define manage                  UxViewerShellContext->Uxmanage
*/
#define view_menu               UxViewerShellContext->Uxview_menu
#define view_options_menu       UxViewerShellContext->Uxview_options_menu
#define view_options_mode_buttons UxViewerShellContext->Uxview_options_mode_buttons
#define view_options_buttonbar  UxViewerShellContext->Uxview_options_buttonbar
#define view_options_decorate   UxViewerShellContext->Uxview_options_decorate
#define view_options            UxViewerShellContext->Uxview_options
#define view_s0                 UxViewerShellContext->Uxview_s0
#define view_split              UxViewerShellContext->Uxview_split
#define viewer_remove           UxViewerShellContext->Uxviewer_remove
#define view_s1                 UxViewerShellContext->Uxview_s1
#define view_collapse           UxViewerShellContext->Uxview_collapse
#define view_expand_level_menu  UxViewerShellContext->Uxview_expand_level_menu
#define view_expand_level_b1    UxViewerShellContext->Uxview_expand_level_b1
#define view_expand_level_b2    UxViewerShellContext->Uxview_expand_level_b2
#define view_expand_level_b3    UxViewerShellContext->Uxview_expand_level_b3
#define view_expand_level_b4    UxViewerShellContext->Uxview_expand_level_b4
#define view_expand_level_b5    UxViewerShellContext->Uxview_expand_level_b5
#define view_expand_level       UxViewerShellContext->Uxview_expand_level
#define gra_fresh               UxViewerShellContext->Uxgra_fresh
#define view_s2                 UxViewerShellContext->Uxview_s2
#define view_magnify            UxViewerShellContext->Uxview_magnify
#define view_shrink             UxViewerShellContext->Uxview_shrink
#define view_zoomtofit          UxViewerShellContext->Uxview_zoomtofit
#define view_reset              UxViewerShellContext->Uxview_reset
#define view_s3                 UxViewerShellContext->Uxview_s3
#define view_pref               UxViewerShellContext->Uxview_pref
#define view                    UxViewerShellContext->Uxview
#define help_menu               UxViewerShellContext->Uxhelp_menu
#define help_TOC                UxViewerShellContext->Uxhelp_TOC
#define help_index              UxViewerShellContext->Uxhelp_index
#define help_s1                 UxViewerShellContext->Uxhelp_s1
#define help_viewer_menus       UxViewerShellContext->Uxhelp_viewer_menus
#define help_on_viewers         UxViewerShellContext->Uxhelp_on_viewers
#define help_on_help            UxViewerShellContext->Uxhelp_on_help
#define help_s2                 UxViewerShellContext->Uxhelp_s2
#define help_release_notes      UxViewerShellContext->Uxhelp_release_notes
#define help_getting_started    UxViewerShellContext->Uxhelp_getting_started
#define help_graphics_conventions UxViewerShellContext->Uxhelp_graphics_conventions
#define help                    UxViewerShellContext->Uxhelp
#define custom_buttons_form     UxViewerShellContext->Uxcustom_buttons_form
#define custom_buttons_controls UxViewerShellContext->Uxcustom_buttons_controls
#define customize_mode          UxViewerShellContext->Uxcustomize_mode
#define remove_mode             UxViewerShellContext->Uxremove_mode
#define normal_mode             UxViewerShellContext->Uxnormal_mode
#define custom_buttons_s1       UxViewerShellContext->Uxcustom_buttons_s1
#define custom_buttons_slot     UxViewerShellContext->Uxcustom_buttons_slot
#define buttons_form            UxViewerShellContext->Uxbuttons_form
#define buttons_s1              UxViewerShellContext->Uxbuttons_s1
#define buttons_rowcol          UxViewerShellContext->Uxbuttons_rowcol
#define separator2              UxViewerShellContext->Uxseparator2
#define modeButtonSlot          UxViewerShellContext->UxmodeButtonSlot
#define relmode_1_1             UxViewerShellContext->Uxrelmode_1_1
#define relmode_1_many          UxViewerShellContext->Uxrelmode_1_many
#define relmode_many_1          UxViewerShellContext->Uxrelmode_many_1
#define relmode_many_many       UxViewerShellContext->Uxrelmode_many_many
#define separator1              UxViewerShellContext->Uxseparator1
#define panner_frame            UxViewerShellContext->Uxpanner_frame
#define status_line             UxViewerShellContext->Uxstatus_line
#define viewWindowForm          UxViewerShellContext->UxviewWindowForm
#define viewWindow              UxViewerShellContext->UxviewWindow
#define viewSepBut              UxViewerShellContext->UxviewSepBut
#define viewListSep             UxViewerShellContext->UxviewListSep
#define viewList                UxViewerShellContext->UxviewList
#define instance                UxViewerShellContext->Uxinstance
#define vshell_obj              UxViewerShellContext->Uxvshell_obj
#define create_minibrowser      UxViewerShellContext->Uxcreate_minibrowser

static _UxCViewerShell	*UxViewerShellContext;


/*******************************************************************************
	Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget	create_ViewerShell();

/*******************************************************************************
	Auxiliary code from the Declarations Editor:
*******************************************************************************/

/* aux code */
void vs_set_status_line_message( UxWidget,str )
	Widget UxWidget;
	char *str;
{
        PUSH_CONTEXT 
        XtVaSetValues(UxGetWidget(status_line),XmNvalue,str,NULL);
        POP_CONTEXT 
}


static void manage_widget_list(va_alist)
    va_dcl
/*
   Call either XtUnmanageChildren or XtManageChildren on a list of
   swidgets.  Converts NULL-terminated variadic list of swidget pointers to
   array of Widgets.  All (s)widgets in the argument list must have the
   same parent.
*/
{
    va_list pvar;
    int flag;
    swidget* sw_ptr;
    Widget w_array[32];
    Widget* w_array_ptr = w_array;
    void (*func)();

    va_start(pvar);

    flag = va_arg(pvar, int);
    func = (flag ? XtManageChildren : XtUnmanageChildren);
    while((sw_ptr = va_arg(pvar, swidget*)) != NULL)
	*w_array_ptr++ = UxGetWidget(*sw_ptr);
    (*func)(w_array, w_array_ptr - w_array);

    va_end(pvar);
}

void vs_special_buttons(UxWidget, mode)
    Widget UxWidget;
    enum vsSpecialButtons mode;
/*
   Enable exactly one set of special buttons, or hide them all.
*/
{
    PUSH_CONTEXT

XtUnmanageChild(UxGetWidget(form1));

    manage_widget_list(
	(mode != vsSpecialButtonsNone),
	&buttons_rowcol,
	&buttons_s1,
	NULL);

    XtManageChild(UxGetWidget(form1));

    POP_CONTEXT
}


/* function to set the correct mode button for a given mode*/
static void set_mode_button(mode)
    int mode;
{
    /* use XmToggleButtonSetState instead of UxPutSet,*/
    /* because the former will also unset the other */
    /* toggle buttons in the radio box */
    switch (mode)
    {
      case VE_1_1_relation:
	XmToggleButtonSetState(UxGetWidget(relmode_1_1), True, True);
	break;

      case VE_1_many_relation:
	XmToggleButtonSetState(UxGetWidget(relmode_1_many), True, True);
	break;

      case VE_many_1_relation:
	XmToggleButtonSetState(UxGetWidget(relmode_many_1), True, True);
	break;

      case VE_many_many_relation:
	XmToggleButtonSetState(UxGetWidget(relmode_many_many), True, True);
	break;
    }
}

/* function to set correct editor toggle button value if enabled */
static void vs_enable_editor_button (UxWidget, sensitive, state)
    Widget UxWidget;
    int sensitive;
    int state;
{
    PUSH_CONTEXT

    XtSetSensitive(UxGetWidget(edit_change_editor), sensitive);
    XmToggleButtonSetState(UxGetWidget(edit_change_editor), state, False);

    POP_CONTEXT
}

void vs_enable_decorate_source(UxWidget, sensitive, state)
    Widget UxWidget;
    int sensitive;
    int state;
{
    PUSH_CONTEXT

    XtSetSensitive(UxGetWidget(view_options_decorate), sensitive);
    XmToggleButtonSetState(UxGetWidget(view_options_decorate), state, False);

    POP_CONTEXT
}

/*Guy: I changed a few settings in this function*/
/* Set up the sensitivities of menu buttons based on view_type. */
/* If "enable" is true, we are enabling buttons for "type", */
/* otherwise we are disabling buttons previously enabled for "type". */

static void sensitize_type(type, enable)
    int type;
    int enable;
{
    switch(type)
    {
      case view_STE:


        XtSetSensitive(UxGetWidget(view_magnify),	   !enable);
        XtSetSensitive(UxGetWidget(view_shrink),	   !enable);
        XtSetSensitive(UxGetWidget(view_zoomtofit),	   !enable);
        XtSetSensitive(UxGetWidget(view_reset),		   !enable);

        break;

      case view_SMT:

        XtSetSensitive(UxGetWidget(file_open),		enable);


        XtSetSensitive(UxGetWidget(view_options_decorate), enable);
        XtSetSensitive(UxGetWidget(view_magnify),	   enable);
        XtSetSensitive(UxGetWidget(view_shrink),	   enable);
        XtSetSensitive(UxGetWidget(view_zoomtofit),	   enable);
        XtSetSensitive(UxGetWidget(view_reset),		   enable);

        break;

      case view_CallTree:

        XtSetSensitive(UxGetWidget(file_open),		enable);

        XtSetSensitive(UxGetWidget(edit_cut),   	!enable);
        XtSetSensitive(UxGetWidget(edit_copy),   	!enable);
        XtSetSensitive(UxGetWidget(edit_paste),  	!enable);

        break;

      case view_ERD:
      case view_DC:

	if (enable) {
	   XtVaSetValues(UxGetWidget(relmode_1_1),XtVaTypedArg,
	   XmNlabelString,XmRString,str_1_1,strlen(str_1_1)+1,NULL);
	   XtVaSetValues(UxGetWidget(relmode_1_many),XtVaTypedArg,
	   XmNlabelString,XmRString,str_1_n,strlen(str_1_n)+1,NULL);
	   XtVaSetValues(UxGetWidget(relmode_many_1),XtVaTypedArg,
	   XmNlabelString,XmRString,str_n_1,strlen(str_n_1)+1,NULL);
	   XtVaSetValues(UxGetWidget(relmode_many_many),XtVaTypedArg,
	   XmNlabelString,XmRString,str_n_n,strlen(str_n_n)+1,NULL);
        }
	manage_widget_list(enable, &relmode_1_1, &relmode_1_many,
			   &relmode_many_1, &relmode_many_many, NULL);
/* FALL THROUGH */

      case view_Class:
        XtSetSensitive(UxGetWidget(file_open),		enable);


        XtSetSensitive(UxGetWidget(view_options_decorate), !enable);

        break;

      case view_SubsysMap:

	if (enable) {
	   XtVaSetValues(UxGetWidget(relmode_1_1),XtVaTypedArg,
	   XmNlabelString,XmRString,str_cl_svr,strlen(str_cl_svr)+1,NULL);
	   XtVaSetValues(UxGetWidget(relmode_1_many),XtVaTypedArg,
	   XmNlabelString,XmRString,str_svr_cl,strlen(str_svr_cl)+1,NULL);
	   XtVaSetValues(UxGetWidget(relmode_many_1),XtVaTypedArg,
	   XmNlabelString,XmRString,str_pr_pr,strlen(str_pr_pr)+1,NULL);
        }
	manage_widget_list(enable, &relmode_1_1, &relmode_1_many,
			   &relmode_many_1, NULL);
/* FALL THROUGH */

      case view_SubsysBrowser:
        XtSetSensitive(UxGetWidget(file_open),		enable);


        XtSetSensitive(UxGetWidget(view_options_decorate), !enable);

        break;

      case view_Raw:

	XtSetSensitive(UxGetWidget(edit_cut), 	        !enable);
	XtSetSensitive(UxGetWidget(edit_copy), 	        !enable);
	XtSetSensitive(UxGetWidget(edit_paste),	        !enable);
	XtSetSensitive(UxGetWidget(edit_clear_selection),!enable);
	XtSetSensitive(UxGetWidget(edit_search),         !enable);

        XtSetSensitive(UxGetWidget(view_magnify),	   !enable);
        XtSetSensitive(UxGetWidget(view_shrink),	   !enable);
        XtSetSensitive(UxGetWidget(view_zoomtofit),	   !enable);
        XtSetSensitive(UxGetWidget(view_reset),		   !enable);

        break;

      case view_DGraph: 
        XtSetSensitive(UxGetWidget(file_save),          !enable);
        XtSetSensitive(UxGetWidget(file_save_btn_bar),  !enable);
        XtSetSensitive(UxGetWidget(file_close_view),     enable);
        XtSetSensitive(UxGetWidget(file_print),          enable);

        XtSetSensitive(UxGetWidget(edit_cut),           !enable);
        XtSetSensitive(UxGetWidget(edit_copy),          !enable);
        XtSetSensitive(UxGetWidget(edit_paste),         !enable);
        XtSetSensitive(UxGetWidget(edit_search),         enable);
        break; 

      case view_Last:

	if (enable) {
	    XtSetSensitive(UxGetWidget(file_open), false);
	}

        XtSetSensitive(UxGetWidget(file_save),		   !enable);
        XtSetSensitive(UxGetWidget(file_save_btn_bar),	   !enable);
        XtSetSensitive(UxGetWidget(file_close_view),	   !enable);
        XtSetSensitive(UxGetWidget(file_print),		   !enable);

        XtSetSensitive(UxGetWidget(edit_cut),   	!enable);
        XtSetSensitive(UxGetWidget(edit_copy),   	!enable);
        XtSetSensitive(UxGetWidget(edit_paste),  	!enable);
        XtSetSensitive(UxGetWidget(edit_search),   	!enable);

	break;
    }
}

void vs_set_buttons(UxWidget, vs_new_type, vs_old_type)
    Widget UxWidget;
    int vs_new_type;
    int vs_old_type;
{
    PUSH_CONTEXT

    /* set the correct toggle button for the mode the viewerShell */
    /* is currently in */
    set_mode_button(viewerShell_get_mode(instance.vshell));

    sensitize_type(vs_old_type, 0);
    sensitize_type(vs_new_type, 1);

    POP_CONTEXT
}

int vs_get_buttonbar(UxWidget)
    Widget UxWidget;
{
    int state;

    PUSH_CONTEXT

    state = XmToggleButtonGetState(UxGetWidget(view_options_buttonbar));

    POP_CONTEXT

    return state;
}

void vs_set_buttonbar(UxWidget, visible)
    Widget UxWidget;
    int visible;
{
    PUSH_CONTEXT

    if(visible)
    {
	XtManageChild(UxGetWidget(custom_buttons_form));
	XtVaSetValues(UxGetWidget(buttons_form),
		XmNtopWidget, UxGetWidget(custom_buttons_form),
		NULL);
    }
    else
    {
	XtVaSetValues(UxGetWidget(buttons_form),
		XmNtopWidget, UxGetWidget(menubar),
		NULL);
	XtUnmanageChild(UxGetWidget(custom_buttons_form));
    }

    manage_widget_list(visible,
		       &custom_buttons_controls,
		       &custom_buttons_slot,
		       &custom_buttons_s1,
		       NULL);

    /* If button-bar is gone, return to edit mode to avoid confusion. */
    if(!visible)
	XmToggleButtonSetState(UxGetWidget(normal_mode), True, True);

    XmToggleButtonSetState(UxGetWidget(view_options_buttonbar), visible, False);

    POP_CONTEXT
}

/* Hide or show the mode buttons. */
void vs_display_mode_buttons(UxWidget, visible)
    Widget UxWidget;
    int visible;
{
    PUSH_CONTEXT

    if(visible)
    {
	set_mode_button(viewerShell_get_mode(instance.vshell));

	XtVaSetValues(UxGetWidget(separator1),
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, UxGetWidget(modeButtonSlot),
	    NULL);
    }
    else
    {
	XtVaSetValues(UxGetWidget(separator1),
	    XmNleftAttachment, XmATTACH_FORM,
	    NULL);
    }
    manage_widget_list(visible, &modeButtonSlot, &panner_frame, NULL);

    POP_CONTEXT
}

void vs_display_mini_browser(UxWidget, visible)
    Widget UxWidget;
    int visible;
{
    PUSH_CONTEXT

    if(visible)
    {

        XtVaSetValues(UxGetWidget(viewWindow), XmNrightAttachment, XmATTACH_WIDGET,
                      XmNrightWidget, UxGetWidget(viewListSep),
	              XmNrightOffset, 150,
	              NULL);
    }
    else
    {
	XtVaSetValues(UxGetWidget(viewWindow), XmNrightAttachment, XmATTACH_FORM,
	              XmNrightOffset, 0,
	              NULL);
    }
    viewerShell_set_miniBrowser(instance.vshell, visible);

    POP_CONTEXT
}

void vs_enable_manager(UxWidget, state)
    Widget UxWidget;
    int state;
{
    PUSH_CONTEXT
/*
XtSetSensitive(UxGetWidget(manage_get1),     state);
XtSetSensitive(UxGetWidget(manage_get2),     state);
XtSetSensitive(UxGetWidget(manage_put),      state);
XtSetSensitive(UxGetWidget(manage_copy),     state);
XtSetSensitive(UxGetWidget(manage_diff),     state);
XtSetSensitive(UxGetWidget(manage_lock),     state);
XtSetSensitive(UxGetWidget(manage_unlock),   state);
XtSetSensitive(UxGetWidget(manage_unget),    state);
*/
    POP_CONTEXT
}

Widget vs_status_line_color(UxWidget, color_name)
    Widget UxWidget;
    char *color_name;
{
    int   color_size = strlen(color_name) + 1;
    Widget w = NULL;

    PUSH_CONTEXT

    w = UxGetWidget(status_line);
    XtVaSetValues(w,
	XtVaTypedArg, XmNbackground, XmRString, color_name, color_size,
	NULL);
    XClearWindow(UxDisplay, XtWindow(w));

    POP_CONTEXT

    return w;
}

void vs_destroy (UxWidget)
    Widget UxWidget;
{
    PUSH_CONTEXT;
    UxDestroyInterface(ViewerShell);
    POP_CONTEXT;

}

/*******************************************************************************
	The following are callback functions.
*******************************************************************************/

static void	destroyCB_ViewerShell( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	rem_top_widget(UxWidget);
	viewerShell_delete(instance.vshell);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	createCB_ViewerShell( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	add_top_widget(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	focusCB_form1( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	viewerShell_set_current_vs(instance.vshell);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_openSelected(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_expand( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_ExpandView(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_defn( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_openDefinition(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_flowchart( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_open_FlowChart(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_vert_hierarchy( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_open_VertHierarchy(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_erd( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_open_ERD(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_dc( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_open_DC(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_inheritance( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_open_Inheritance(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_open_calltree( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_open_call_tree(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_load( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_file_load(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_unload( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_file_unload(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_update( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_file_update(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_save( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_app_save(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_save_btn_bar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_save_buttonbars(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_close_view( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_close_current_view(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_print( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{ 
	void *cur_view = viewerShell_get_current_view(instance.vshell);
	if (cur_view)
	  {
	  BB_BEGIN(instance.vshell)
	  popup_Print(cur_view);
	  BB_END
	  }
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_file_close_window( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_close_viewer_shell(UxGetWidget(ViewerShell),instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	mapCB_edit_menu( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	int sensitive = False;
	int state = False;
	int read_only = 0;
	int edit_flag = edit_menu_can_edit (instance.vshell, &sensitive, &state, &read_only);
	   if (edit_flag == 0) {
		XtSetSensitive(UxGetWidget(edit_cut), 	        False);
		XtSetSensitive(UxGetWidget(edit_copy), 	        False);
		XtSetSensitive(UxGetWidget(edit_paste),	        False);
		XtSetSensitive(UxGetWidget(edit_clear_selection),False);
		XtSetSensitive(UxGetWidget(edit_search),         False);
	        XtSetSensitive(UxGetWidget(edit_change_editor), False);
	        XmToggleButtonSetState(UxGetWidget(edit_change_editor), False, False);
	  } else {
	        XtSetSensitive(UxGetWidget(edit_change_editor), sensitive);
	        XmToggleButtonSetState(UxGetWidget(edit_change_editor), state, False);
	        if (read_only == -1) {
	            XtSetSensitive(UxGetWidget(edit_cut),                False);
	            XtSetSensitive(UxGetWidget(edit_copy),               False);
	            XtSetSensitive(UxGetWidget(edit_paste),              False);
	            return;
	         }
	         read_only = (read_only) ? False : True;
	         XtSetSensitive(UxGetWidget(edit_cut),           read_only);
	         XtSetSensitive(UxGetWidget(edit_paste),         read_only);
	  }
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_edit_cut( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_cut(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_edit_copy( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_copy(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_edit_paste( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_paste(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_edit_search( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	void *cur_view = viewerShell_get_current_view(instance.vshell);
	if (cur_view)
	  {
	  BB_BEGIN(instance.vshell)
	  popup_quick_search(); 
	  BB_END
	  }
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_edit_change_editor( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	void* cur_view = viewerShell_get_current_view(instance.vshell);
	if (cur_view)
	  {
	  int set = ((XmToggleButtonCallbackStruct*)UxCallbackArg)->set;
	  BB_BEGIN(instance.vshell)
	  ste_interface_change_editor(cur_view, set);
	  BB_END
	  }
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_edit_clear_selection( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	dr_clear_selection();
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

/*
static void	mapCB_manage_menu( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	int new_manage_mode = can_manage_file(instance.vshell);
	XtSetSensitive(UxGetWidget(manage_get1),     new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_get2),     new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_put),      new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_copy),     new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_diff),     new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_lock),     new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_unlock),   new_manage_mode);
	XtSetSensitive(UxGetWidget(manage_unget),    new_manage_mode);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_home_project( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage_home_project(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_get1( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_GET);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_get2( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_GET2);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_put( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_PUT);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_copy( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_COPY);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_diff( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_DIFF);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_lock( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_LOCK);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_unlock( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_UNLOCK);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_manage_unget( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manage(instance.vshell, MANAGE_UNGET);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}
*/
static void	mapCB_view_menu( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	int collapse_flag = can_collapse_explode(instance.vshell);
	int flowchart_flag = is_flowchart_view(instance.vshell);
	int gra_fresh_flag = can_refresh_view(instance.vshell);
	XtSetSensitive(UxGetWidget(view_split), can_add_viewer(instance.vshell));
	XtSetSensitive(UxGetWidget(viewer_remove), can_remove_viewer(instance.vshell));
	viewerShell_enable_decorate_source(instance.vshell);
	XtSetSensitive(UxGetWidget(view_collapse),collapse_flag && !flowchart_flag);
	XtSetSensitive(UxGetWidget(view_expand_level),collapse_flag && !flowchart_flag);
	XtSetSensitive(UxGetWidget(view_magnify),collapse_flag);
	XtSetSensitive(UxGetWidget(gra_fresh),gra_fresh_flag);
	XtSetSensitive(UxGetWidget(view_shrink),collapse_flag);
	XtSetSensitive(UxGetWidget(view_zoomtofit),collapse_flag);
	XtSetSensitive(UxGetWidget(view_reset),collapse_flag);
	XtSetSensitive(UxGetWidget(view_pref),collapse_flag);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_view_options_mode_buttons( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	int set = ((XmToggleButtonCallbackStruct*)UxCallbackArg)->set;
	vs_display_mode_buttons(UxWidget, set);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_view_options_buttonbar( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	vs_set_buttonbar(UxGetWidget(ViewerShell), ((XmToggleButtonCallbackStruct*)UxCallbackArg)->set);
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_view_options_decorate( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_decorate_source(instance.vshell, ((XmToggleButtonCallbackStruct*)UxCallbackArg)->set);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_split( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	vsAddViewer(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_viewer_remove( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	vsRemoveViewer (instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_collapse( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_collapse_current(instance.vshell, -1);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_expand_level_b1( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	viewerShell_explode_current(instance.vshell, 1);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_expand_level_b2( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	viewerShell_explode_current(instance.vshell, 2);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_expand_level_b3( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	viewerShell_explode_current(instance.vshell, 3);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_expand_level_b4( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	viewerShell_explode_current(instance.vshell, 4);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_expand_level_b5( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	viewerShell_explode_current(instance.vshell, 5);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_gra_fresh( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_refresh_gra_view(instance.vshell);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_magnify( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manipulate_perspective(instance.vshell, VIEW_ZOOM_XY, 1);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_shrink( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manipulate_perspective(instance.vshell, VIEW_ZOOM_XY, -1);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_zoomtofit( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manipulate_perspective(instance.vshell, VIEW_ZOOM_TO_FIT);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_reset( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	viewerShell_manipulate_perspective(instance.vshell, VIEW_RESET);
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_view_pref( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	BB_BEGIN(instance.vshell)
	popup_PROJ_view_pref();
	BB_END
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_TOC( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_index( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_viewer_menus( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_on_viewers( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_on_help( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_release_notes( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_getting_started( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	activateCB_help_graphics_conventions( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	HelpUI_HelpWidget_CB(UxWidget);
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_customize_mode( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_is_customizing(instance.vshell, VC_customize);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_remove_mode( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_is_customizing(instance.vshell, VC_remove);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_normal_mode( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_is_customizing(instance.vshell, VC_normal);
	}
	UxViewerShellContext = UxSaveCtx;
}


static void	valueChangedCB_relmode_1_1( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_set_mode(instance.vshell, VE_1_1_relation);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_relmode_1_many( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_set_mode(instance.vshell, VE_1_many_relation);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_relmode_many_1( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_set_mode(instance.vshell, VE_many_1_relation);
	}
	UxViewerShellContext = UxSaveCtx;
}

static void	valueChangedCB_relmode_many_many( UxWidget, UxClientData, UxCallbackArg )
	Widget		UxWidget;
	XtPointer	UxClientData, UxCallbackArg;
{
	_UxCViewerShell         *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxViewerShellContext;
	UxViewerShellContext = UxContext =
			(_UxCViewerShell *) UxGetContext( UxThisWidget );
	{
	if (((XmToggleButtonCallbackStruct*)UxCallbackArg)->set)
	   viewerShell_set_mode(instance.vshell, VE_many_many_relation);
	}
	UxViewerShellContext = UxSaveCtx;
}

/*******************************************************************************
	The 'init_' function sets the private properties for all the
	swidgets to the values specified in the Property Table.
	Some properties need to be set after the X widgets have been
	created and the setting of these properties is done in the
	'build_' function after the UxCreateWidget call.
*******************************************************************************/
extern int isUpdateAvailable();
static void	_Uxinit_ViewerShell()
{
	UxPutTitle( ViewerShell, gettext(TXT("ParaSET Viewer")) );
	UxPutIconName( ViewerShell, gettext(TXT("Viewer")) );

	UxPutVerticalSpacing( form1, 0 );
	UxPutMarginWidth( form1, 0 );
	UxPutMarginHeight( form1, 0 );
	UxPutHorizontalSpacing( form1, 0 );
	UxPutUnitType( form1, "100th_millimeters" );

	UxPutHeight( browser_icon, 910 );
	UxPutWidth( browser_icon, 820 );
	UxPutResizePolicy( browser_icon, "resize_none" );

	UxPutMenuAccelerator( menubar, "<KeyUp>F10" );
	UxPutRowColumnType( menubar, "menu_bar" );

	UxPutRowColumnType( file_menu, "menu_pulldown" );

	UxPutLabelString( file_open, gettext(TXT("Reduce View")) );

	UxPutLabelString( file_expand, gettext(TXT("Expand View")) );

	UxPutLabelString( file_open_defn, gettext(TXT("Open Definition")) );

	UxPutLabelString( file_open_flowchart, gettext(TXT("Open Flowchart")) );

	UxPutLabelString( file_open_vert_hierarchy, gettext(TXT("Open Outline")) );

	UxPutLabelString( file_open_erd, gettext(TXT("Open ERD")) );

	UxPutLabelString( file_open_dc, gettext(TXT("Open Data Chart")) );

	UxPutLabelString( file_open_inheritance, gettext(TXT("Open Inheritance")) );

	UxPutLabelString( file_open_calltree, gettext(TXT("Open Call Tree")) );

	UxPutLabelString( file_load, gettext(TXT("Load")) );

	UxPutLabelString( file_unload, gettext(TXT("Unload")) );

	if(isUpdateAvailable())	  UxPutLabelString( file_update, gettext(TXT("Update")) );

	UxPutLabelString( file_save, gettext(TXT("Save")) );

	UxPutLabelString( file_save_btn_bar, gettext(TXT("Save Custom Buttons")) );

	UxPutLabelString( file_close_view, gettext(TXT("Close Current View")) );

	UxPutLabelString( file_print, gettext(TXT("Print...")) );

	UxPutLabelString( file_close_window, gettext(TXT("Close Window")) );

	UxPutLabelString( file, gettext(TXT("File")) );

	UxPutRowColumnType( edit_menu, "menu_pulldown" );

	UxPutLabelString( edit_cut, gettext(TXT("Cut")) );

	UxPutLabelString( edit_copy, gettext(TXT("Copy")) );

	UxPutLabelString( edit_paste, gettext(TXT("Paste")) );

	UxPutLabelString( edit_search, gettext(TXT("Entity Search...")) );

	UxPutSet( edit_change_editor, "false" );
	UxPutLabelString( edit_change_editor, gettext(TXT("VI editor mode")) );

	UxPutLabelString( edit_clear_selection, gettext(TXT("Clear Selection")) );

	UxPutLabelString( edit, gettext(TXT("Edit")) );

/*	UxPutRowColumnType( manage_menu, "menu_pulldown" ); 

	UxPutLabelString( manage_home_project, gettext(TXT("Home Project")) );

	UxPutLabelString( manage_get1, gettext(TXT("Get")) );

	UxPutLabelString( manage_get2, gettext(TXT("Get with Options...")) );

	UxPutLabelString( manage_put, gettext(TXT("Put...")) );

	UxPutLabelString( manage_copy, gettext(TXT("Copy")) );

	UxPutLabelString( manage_diff, gettext(TXT("Diff")) );

	UxPutLabelString( manage_lock, gettext(TXT("Lock")) );

	UxPutLabelString( manage_unlock, gettext(TXT("Unlock...")) );

	UxPutLabelString( manage_unget, gettext(TXT("Unget")) );

	UxPutLabelString( manage, gettext(TXT("Manage")) );
*/
	UxPutRowColumnType( view_menu, "menu_pulldown" );

	UxPutRowColumnType( view_options_menu, "menu_pulldown" );

	UxPutSet( view_options_mode_buttons, "true" );
	UxPutLabelString( view_options_mode_buttons, gettext(TXT("Mode Buttons")) );

	UxPutLabelString( view_options_buttonbar, gettext(TXT("Custom Buttons")) );

	UxPutLabelString( view_options_decorate, gettext(TXT("Decorate Source")) );

	UxPutLabelString( view_options, gettext(TXT("Options")) );

	UxPutLabelString( view_split, gettext(TXT("Split Viewer")) );

	UxPutLabelString( viewer_remove, gettext(TXT("Remove Viewer")) );

	UxPutLabelString( view_collapse, gettext(TXT("Collapse")) );

	UxPutEntryAlignment( view_expand_level_menu, "alignment_end" );
	UxPutRowColumnType( view_expand_level_menu, "menu_pulldown" );

	UxPutLabelString( view_expand_level_b1, gettext(TXT("1st Level")) );

	UxPutLabelString( view_expand_level_b2, gettext(TXT("2nd Level")) );

	UxPutLabelString( view_expand_level_b3, gettext(TXT("3rd Level")) );

	UxPutLabelString( view_expand_level_b4, gettext(TXT("4th Level")) );

	UxPutLabelString( view_expand_level_b5, gettext(TXT("5th Level")) );

	UxPutLabelString( view_expand_level, gettext(TXT("Display")) );

	UxPutLabelString( gra_fresh, gettext(TXT("Refresh View")) );

	UxPutLabelString( view_magnify, gettext(TXT("Zoom In")) );

	UxPutLabelString( view_shrink, gettext(TXT("Zoom Out")) );

	UxPutLabelString( view_zoomtofit, gettext(TXT("Zoom To Fit")) );

	UxPutLabelString( view_reset, gettext(TXT("Reset Zoom")) );

	UxPutLabelString( view_pref, gettext(TXT("View Preferences...")) );

	UxPutLabelString( view, gettext(TXT("View")) );

	UxPutRowColumnType( help_menu, "menu_pulldown" );

	UxPutLabelString( help_TOC, gettext(TXT("Contents")) );

	UxPutLabelString( help_index, gettext(TXT("Index")) );

	UxPutLabelString( help_viewer_menus, gettext(TXT("On Viewer Menus")) );

	UxPutLabelString( help_on_viewers, gettext(TXT("On Viewers")) );

	UxPutLabelString( help_on_help, gettext(TXT("On Help")) );

	UxPutLabelString( help_release_notes, gettext(TXT("Release Notes")) );

	UxPutLabelString( help_getting_started, gettext(TXT("Getting Started")) );

	UxPutLabelString( help_graphics_conventions, gettext(TXT("Graphics Conventions")) );

	UxPutLabelString( help, gettext(TXT("Help")) );

	UxPutPacking( custom_buttons_controls, "pack_tight" );
	UxPutRadioBehavior( custom_buttons_controls, "true" );
	UxPutSpacing( custom_buttons_controls, 0 );
	UxPutMarginWidth( custom_buttons_controls, 0 );
	UxPutMarginHeight( custom_buttons_controls, 0 );
	UxPutAdjustLast( custom_buttons_controls, "false" );
	UxPutOrientation( custom_buttons_controls, "horizontal" );

	UxPutSpacing( customize_mode, 0 );
	UxPutIndicatorType( customize_mode, "one_of_many" );
	UxPutIndicatorSize( customize_mode, 0 );
	UxPutMarginWidth( customize_mode, 60 );
	UxPutMarginTop( customize_mode, 0 );
	UxPutMarginHeight( customize_mode, 60 );
	UxPutLabelString( customize_mode, gettext(TXT("Add")) );
	UxPutShadowThickness( customize_mode, 60 );
	UxPutIndicatorOn( customize_mode, "false" );

	UxPutSpacing( remove_mode, 0 );
	UxPutIndicatorType( remove_mode, "one_of_many" );
	UxPutIndicatorSize( remove_mode, 0 );
	UxPutMarginWidth( remove_mode, 60 );
	UxPutMarginTop( remove_mode, 0 );
	UxPutMarginHeight( remove_mode, 60 );
	UxPutLabelString( remove_mode, gettext(TXT("Remove")) );
	UxPutShadowThickness( remove_mode, 60 );
	UxPutIndicatorOn( remove_mode, "false" );

	UxPutSet( normal_mode, "true" );
	UxPutSpacing( normal_mode, 0 );
	UxPutIndicatorType( normal_mode, "one_of_many" );
	UxPutIndicatorSize( normal_mode, 0 );
	UxPutMarginWidth( normal_mode, 60 );
	UxPutMarginTop( normal_mode, 0 );
	UxPutMarginHeight( normal_mode, 60 );
	UxPutShadowThickness( normal_mode, 60 );
	UxPutLabelString( normal_mode, gettext(TXT("Normal")) );
	UxPutIndicatorOn( normal_mode, "false" );

	UxPutWidth( custom_buttons_s1, 120 );
	UxPutOrientation( custom_buttons_s1, "vertical" );

	UxPutOrientation( buttons_s1, "horizontal" );

	UxPutSpacing( buttons_rowcol, 0 );
	UxPutMarginWidth( buttons_rowcol, 0 );
	UxPutMarginHeight( buttons_rowcol, 0 );
	UxPutAdjustLast( buttons_rowcol, "false" );
	UxPutOrientation( buttons_rowcol, "horizontal" );

	UxPutResizeWidth( modeButtonSlot, "false" );
	UxPutWidth( modeButtonSlot, 1260 );
	UxPutEntryAlignment( modeButtonSlot, "alignment_center" );
	UxPutAdjustMargin( modeButtonSlot, "false" );
	UxPutAdjustLast( modeButtonSlot, "false" );
	UxPutPacking( modeButtonSlot, "pack_column" );
	UxPutRadioBehavior( modeButtonSlot, "true" );
	UxPutMarginWidth( modeButtonSlot, 30 );
	UxPutMarginHeight( modeButtonSlot, 0 );

	UxPutRecomputeSize( relmode_1_1, "false" );
	UxPutWidth( relmode_1_1, 1200 );
	UxPutHeight( relmode_1_1, 1200 );
	UxPutHighlightThickness( relmode_1_1, 0 );
	UxPutLabelString( relmode_1_1, str_1_1 );
	UxPutShadowThickness( relmode_1_1, 90 );
	UxPutLabelType( relmode_1_1, "string" );
	UxPutIndicatorOn( relmode_1_1, "false" );

	UxPutRecomputeSize( relmode_1_many, "false" );
	UxPutWidth( relmode_1_many, 1200 );
	UxPutHeight( relmode_1_many, 1200 );
	UxPutHighlightThickness( relmode_1_many, 0 );
	UxPutLabelString( relmode_1_many, str_1_n );
	UxPutShadowThickness( relmode_1_many, 90 );
	UxPutLabelType( relmode_1_many, "string" );
	UxPutIndicatorOn( relmode_1_many, "false" );

	UxPutRecomputeSize( relmode_many_1, "false" );
	UxPutWidth( relmode_many_1, 1200 );
	UxPutHeight( relmode_many_1, 1200 );
	UxPutHighlightThickness( relmode_many_1, 0 );
	UxPutLabelString( relmode_many_1, str_n_1 );
	UxPutShadowThickness( relmode_many_1, 90 );
	UxPutLabelType( relmode_many_1, "string" );
	UxPutIndicatorOn( relmode_many_1, "false" );

	UxPutRecomputeSize( relmode_many_many, "false" );
	UxPutWidth( relmode_many_many, 1200 );
	UxPutHeight( relmode_many_many, 1200 );
	UxPutHighlightThickness( relmode_many_many, 0 );
	UxPutLabelString( relmode_many_many, str_n_n );
	UxPutShadowThickness( relmode_many_many, 90 );
	UxPutLabelType( relmode_many_many, "string" );
	UxPutIndicatorOn( relmode_many_many, "false" );

	UxPutOrientation( separator1, "vertical" );

	UxPutShadowThickness( panner_frame, 60 );
	UxPutNavigationType( panner_frame, "none" );
	UxPutShadowType( panner_frame, "shadow_in" );

	UxPutWidth( status_line, 16000 );
	UxPutCursorPositionVisible( status_line, "false" );
	UxPutEditable( status_line, "false" );
	UxPutSensitive( status_line, "true" );

	UxPutRubberPositioning( viewWindowForm, "true" );
	UxPutMarginWidth( viewWindowForm, 0 );
	UxPutMarginHeight( viewWindowForm, 0 );

	UxPutMarginWidth( viewWindow, 0 );
	UxPutMarginHeight( viewWindow, 0 );
	UxPutHeight( viewWindow, 10000 );

	UxPutFillOnArm( viewSepBut, "false" );
	UxPutTraversalOn( viewSepBut, "false" );
	UxPutHeight( viewSepBut, 280 );
	UxPutWidth( viewSepBut, 280 );

	UxPutOrientation( viewListSep, "vertical" );

	UxPutMarginWidth( viewList, 0 );
	UxPutMarginHeight( viewList, 0 );
	UxPutWidth( viewList, 6000 );

}

/*******************************************************************************
	The 'build_' function creates all the swidgets and X widgets,
	and sets their properties to the values specified in the
	Property Editor.
*******************************************************************************/

static swidget	_Uxbuild_ViewerShell()
{
	/* Create the swidgets */

	ViewerShell = UxCreateTopLevelShell( "ViewerShell", NO_PARENT );
	UxPutContext( ViewerShell, UxViewerShellContext );

	form1 = UxCreateForm( "form1", ViewerShell );
	browser_icon = UxCreateForm( "browser_icon", form1 );
	menubar = UxCreateRowColumn( "menubar", form1 );
	file_menu = UxCreateRowColumn( "file_menu", menubar );
	file_open = UxCreatePushButton( "file_open", file_menu );
	file_expand = UxCreatePushButton( "file_expand", file_menu );
	file_open_defn = UxCreatePushButton( "file_open_defn", file_menu );
	file_open_flowchart = UxCreatePushButton( "file_open_flowchart", file_menu );
	file_open_vert_hierarchy = UxCreatePushButton( "file_open_vert_hierarchy", file_menu );
	file_open_erd = UxCreatePushButton( "file_open_erd", file_menu );
	file_open_dc = UxCreatePushButton( "file_open_dc", file_menu );
	file_open_inheritance = UxCreatePushButton( "file_open_inheritance", file_menu );
	file_open_calltree = UxCreatePushButton( "file_open_calltree", file_menu );
	file_s01 = UxCreateSeparator( "file_s01", file_menu );
	file_load = UxCreatePushButton( "file_load", file_menu );
	file_unload = UxCreatePushButton( "file_unload", file_menu );
	file_update = UxCreatePushButton( "file_update", file_menu );
	file_s1 = UxCreateSeparator( "file_s1", file_menu );
	file_save = UxCreatePushButton( "file_save", file_menu );
	file_save_btn_bar = UxCreatePushButton( "file_save_btn_bar", file_menu );
	file_s2 = UxCreateSeparator( "file_s2", file_menu );
	file_close_view = UxCreatePushButton( "file_close_view", file_menu );
	file_print = UxCreatePushButton( "file_print", file_menu );
	file_s3 = UxCreateSeparator( "file_s3", file_menu );
	file_close_window = UxCreatePushButton( "file_close_window", file_menu );
	file = UxCreateCascadeButton( "file", menubar );
	edit_menu = UxCreateRowColumn( "edit_menu", menubar );
	edit_cut = UxCreatePushButton( "edit_cut", edit_menu );
	edit_copy = UxCreatePushButton( "edit_copy", edit_menu );
	edit_paste = UxCreatePushButton( "edit_paste", edit_menu );
	edit_s1 = UxCreateSeparator( "edit_s1", edit_menu );
	edit_search = UxCreatePushButton( "edit_search", edit_menu );
	edit_s2 = UxCreateSeparator( "edit_s2", edit_menu );
	edit_change_editor = UxCreateToggleButton( "edit_change_editor", edit_menu );
	edit_s3 = UxCreateSeparator( "edit_s3", edit_menu );
	edit_clear_selection = UxCreatePushButton( "edit_clear_selection", edit_menu );
	edit = UxCreateCascadeButton( "edit", menubar );
/*	
	manage_menu = UxCreateRowColumn( "manage_menu", menubar );
	manage_home_project = UxCreatePushButton( "manage_home_project", manage_menu );
	manage_s01 = UxCreateSeparator( "manage_s01", manage_menu );
	manage_get1 = UxCreatePushButton( "manage_get1", manage_menu );
	manage_get2 = UxCreatePushButton( "manage_get2", manage_menu );
	manage_put = UxCreatePushButton( "manage_put", manage_menu );
	manage_s1 = UxCreateSeparator( "manage_s1", manage_menu );
	manage_copy = UxCreatePushButton( "manage_copy", manage_menu );
	manage_diff = UxCreatePushButton( "manage_diff", manage_menu );
	manage_lock = UxCreatePushButton( "manage_lock", manage_menu );
	manage_unlock = UxCreatePushButton( "manage_unlock", manage_menu );
	manage_s2 = UxCreateSeparator( "manage_s2", manage_menu );
	manage_unget = UxCreatePushButton( "manage_unget", manage_menu );
	manage = UxCreateCascadeButton( "manage", menubar );
*/	
	view_menu = UxCreateRowColumn( "view_menu", menubar );
	view_options_menu = UxCreateRowColumn( "view_options_menu", view_menu );
	view_options_mode_buttons = UxCreateToggleButton( "view_options_mode_buttons", view_options_menu );
	view_options_buttonbar = UxCreateToggleButton( "view_options_buttonbar", view_options_menu );
	view_options_decorate = UxCreateToggleButton( "view_options_decorate", view_options_menu );
	view_options = UxCreateCascadeButton( "view_options", view_menu );
	view_s0 = UxCreateSeparator( "view_s0", view_menu );
	view_split = UxCreatePushButton( "view_split", view_menu );
	viewer_remove = UxCreatePushButton( "viewer_remove", view_menu );
	view_s1 = UxCreateSeparator( "view_s1", view_menu );
	view_collapse = UxCreatePushButton( "view_collapse", view_menu );
	view_expand_level_menu = UxCreateRowColumn( "view_expand_level_menu", view_menu );
	view_expand_level_b1 = UxCreatePushButton( "view_expand_level_b1", view_expand_level_menu );
	view_expand_level_b2 = UxCreatePushButton( "view_expand_level_b2", view_expand_level_menu );
	view_expand_level_b3 = UxCreatePushButton( "view_expand_level_b3", view_expand_level_menu );
	view_expand_level_b4 = UxCreatePushButton( "view_expand_level_b4", view_expand_level_menu );
	view_expand_level_b5 = UxCreatePushButton( "view_expand_level_b5", view_expand_level_menu );
	view_expand_level = UxCreateCascadeButton( "view_expand_level", view_menu );
	gra_fresh = UxCreatePushButton( "gra_fresh", view_menu );
	view_s2 = UxCreateSeparator( "view_s2", view_menu );
	view_magnify = UxCreatePushButton( "view_magnify", view_menu );
	view_shrink = UxCreatePushButton( "view_shrink", view_menu );
	view_zoomtofit = UxCreatePushButton( "view_zoomtofit", view_menu );
	view_reset = UxCreatePushButton( "view_reset", view_menu );
	view_s3 = UxCreateSeparator( "view_s3", view_menu );
	view_pref = UxCreatePushButton( "view_pref", view_menu );
	view = UxCreateCascadeButton( "view", menubar );
	help_menu = UxCreateRowColumn( "help_menu", menubar );
	help_TOC = UxCreatePushButton( "help_TOC", help_menu );
	help_index = UxCreatePushButton( "help_index", help_menu );
	help_s1 = UxCreateSeparator( "help_s1", help_menu );
	help_viewer_menus = UxCreatePushButton( "help_viewer_menus", help_menu );
	help_on_viewers = UxCreatePushButton( "help_on_viewers", help_menu );
	help_on_help = UxCreatePushButton( "help_on_help", help_menu );
	help_s2 = UxCreateSeparator( "help_s2", help_menu );
	help_release_notes = UxCreatePushButton( "help_release_notes", help_menu );
	help_getting_started = UxCreatePushButton( "help_getting_started", help_menu );
	help_graphics_conventions = UxCreatePushButton( "help_graphics_conventions", help_menu );
	help = UxCreateCascadeButton( "help", menubar );
	custom_buttons_form = UxCreateForm( "custom_buttons_form", form1 );
	custom_buttons_controls = UxCreateRowColumn( "custom_buttons_controls", custom_buttons_form );
	customize_mode = UxCreateToggleButton( "customize_mode", custom_buttons_controls );
	remove_mode = UxCreateToggleButton( "remove_mode", custom_buttons_controls );
	normal_mode = UxCreateToggleButton( "normal_mode", custom_buttons_controls );
	custom_buttons_s1 = UxCreateSeparator( "custom_buttons_s1", custom_buttons_form );
	custom_buttons_slot = UxCreateForm( "custom_buttons_slot", custom_buttons_form );
	buttons_form = UxCreateForm( "buttons_form", form1 );
	buttons_s1 = UxCreateSeparator( "buttons_s1", buttons_form );
	buttons_rowcol = UxCreateRowColumn( "buttons_rowcol", buttons_form );
	separator2 = UxCreateSeparator( "separator2", form1 );
	modeButtonSlot = UxCreateRowColumn( "modeButtonSlot", form1 );
	relmode_1_1 = UxCreateToggleButton( "relmode_1_1", modeButtonSlot );
	relmode_1_many = UxCreateToggleButton( "relmode_1_many", modeButtonSlot );
	relmode_many_1 = UxCreateToggleButton( "relmode_many_1", modeButtonSlot );
	relmode_many_many = UxCreateToggleButton( "relmode_many_many", modeButtonSlot );
	separator1 = UxCreateSeparator( "separator1", form1 );
	panner_frame = UxCreateFrame( "panner_frame", form1 );
	status_line = UxCreateTextField( "status_line", form1 );
	viewWindowForm = UxCreateForm( "viewWindowForm", form1 );
	viewWindow = UxCreatePanedWindow( "viewWindow", viewWindowForm );
	viewSepBut = UxCreatePushButton( "viewSepBut", viewWindowForm );
	viewListSep = UxCreateSeparator( "viewListSep", viewWindowForm );
	viewList = UxCreateForm( "viewList", viewWindowForm );

	_Uxinit_ViewerShell();

	/* Create the X widgets */

	UxCreateWidget( ViewerShell );
	createCB_ViewerShell( UxGetWidget( ViewerShell ),
			(XtPointer) UxViewerShellContext, (XtPointer) NULL );

	UxCreateWidget( form1 );
	UxPutTopAttachment( browser_icon, "attach_form" );
	UxPutRightOffset( browser_icon, 25 );
	UxPutRightAttachment( browser_icon, "attach_form" );
	UxCreateWidget( browser_icon );

	UxPutTopOffset( menubar, 0 );
	UxPutTopAttachment( menubar, "attach_form" );
	UxPutRightOffset( menubar, 845 );
	UxPutRightAttachment( menubar, "attach_form" );
	UxPutLeftOffset( menubar, 0 );
	UxPutLeftAttachment( menubar, "attach_form" );
	UxCreateWidget( menubar );

	UxCreateWidget( file_menu );
	UxCreateWidget( file_open );
	UxCreateWidget( file_expand );
	UxCreateWidget( file_open_defn );
	UxCreateWidget( file_open_flowchart );
	UxCreateWidget( file_open_vert_hierarchy );
	UxCreateWidget( file_open_erd );
	UxCreateWidget( file_open_dc );
	UxCreateWidget( file_open_inheritance );
	UxCreateWidget( file_open_calltree );
	UxCreateWidget( file_s01 );
	UxCreateWidget( file_load );
	UxCreateWidget( file_unload );
	if(isUpdateAvailable())	  UxCreateWidget( file_update );
	UxCreateWidget( file_s1 );
	UxCreateWidget( file_save );
	UxCreateWidget( file_save_btn_bar );
	UxCreateWidget( file_s2 );
	UxCreateWidget( file_close_view );
	UxCreateWidget( file_print );
	UxCreateWidget( file_s3 );
	UxCreateWidget( file_close_window );
	UxPutSubMenuId( file, "file_menu" );
	UxCreateWidget( file );

	UxCreateWidget( edit_menu );
	UxCreateWidget( edit_cut );
	UxCreateWidget( edit_copy );
	UxCreateWidget( edit_paste );
	UxCreateWidget( edit_s1 );
	UxCreateWidget( edit_search );
	UxCreateWidget( edit_s2 );
	UxCreateWidget( edit_change_editor );
	UxCreateWidget( edit_s3 );
	UxCreateWidget( edit_clear_selection );
	UxPutSubMenuId( edit, "edit_menu" );
	UxCreateWidget( edit );
/*
	UxCreateWidget( manage_menu );
	UxCreateWidget( manage_home_project );
	UxCreateWidget( manage_s01 );
	UxCreateWidget( manage_get1 );
	UxCreateWidget( manage_get2 );
	UxCreateWidget( manage_put );
	UxCreateWidget( manage_s1 );
	UxCreateWidget( manage_copy );
	UxCreateWidget( manage_diff );
	UxCreateWidget( manage_lock );
	UxCreateWidget( manage_unlock );
	UxCreateWidget( manage_s2 );
	UxCreateWidget( manage_unget );
	UxPutSubMenuId( manage, "manage_menu" );
	UxCreateWidget( manage );
*/
	UxCreateWidget( view_menu );
	UxCreateWidget( view_options_menu );
	UxCreateWidget( view_options_mode_buttons );
	UxCreateWidget( view_options_buttonbar );
	UxCreateWidget( view_options_decorate );
	UxPutSubMenuId( view_options, "view_options_menu" );
	UxCreateWidget( view_options );

	UxCreateWidget( view_s0 );
	UxCreateWidget( view_split );
	UxCreateWidget( viewer_remove );
	UxCreateWidget( view_s1 );
	UxCreateWidget( view_collapse );
	UxCreateWidget( view_expand_level_menu );
	UxCreateWidget( view_expand_level_b1 );
	UxCreateWidget( view_expand_level_b2 );
	UxCreateWidget( view_expand_level_b3 );
	UxCreateWidget( view_expand_level_b4 );
	UxCreateWidget( view_expand_level_b5 );
	UxPutSubMenuId( view_expand_level, "view_expand_level_menu" );
	UxCreateWidget( view_expand_level );

	UxCreateWidget( gra_fresh );
	UxCreateWidget( view_s2 );
	UxCreateWidget( view_magnify );
	UxCreateWidget( view_shrink );
	UxCreateWidget( view_zoomtofit );
	UxCreateWidget( view_reset );
	UxCreateWidget( view_s3 );
	UxCreateWidget( view_pref );
	UxPutSubMenuId( view, "view_menu" );
	UxCreateWidget( view );

	UxCreateWidget( help_menu );
	UxCreateWidget( help_TOC );
/*
        // we do not have a context sensitive help for now
 
	UxCreateWidget( help_index );
	UxCreateWidget( help_s1 );
	UxCreateWidget( help_viewer_menus );
	UxCreateWidget( help_on_viewers );
	UxCreateWidget( help_on_help );
	UxCreateWidget( help_s2 );
	UxCreateWidget( help_release_notes );
	UxCreateWidget( help_getting_started );
	UxCreateWidget( help_graphics_conventions );
*/
	UxPutSubMenuId( help, "help_menu" );
	UxCreateWidget( help );

	UxPutTopWidget( custom_buttons_form, "menubar" );
	UxPutTopOffset( custom_buttons_form, 0 );
	UxPutTopAttachment( custom_buttons_form, "attach_widget" );
	UxPutRightOffset( custom_buttons_form, 30 );
	UxPutRightAttachment( custom_buttons_form, "attach_form" );
	UxPutLeftOffset( custom_buttons_form, 30 );
	UxPutLeftAttachment( custom_buttons_form, "attach_form" );
	UxCreateWidget( custom_buttons_form );

	UxPutTopOffset( custom_buttons_controls, 0 );
	UxPutTopAttachment( custom_buttons_controls, "attach_form" );
	UxPutRightOffset( custom_buttons_controls, 0 );
	UxPutRightAttachment( custom_buttons_controls, "attach_form" );
	UxCreateWidget( custom_buttons_controls );

	UxCreateWidget( customize_mode );
	UxCreateWidget( remove_mode );
	UxCreateWidget( normal_mode );
	UxPutBottomOffset( custom_buttons_s1, 0 );
	UxPutBottomAttachment( custom_buttons_s1, "attach_form" );
	UxPutTopAttachment( custom_buttons_s1, "attach_form" );
	UxPutRightWidget( custom_buttons_s1, "custom_buttons_controls" );
	UxPutRightOffset( custom_buttons_s1, 0 );
	UxPutRightAttachment( custom_buttons_s1, "attach_widget" );
	UxCreateWidget( custom_buttons_s1 );

	UxPutBottomOffset( custom_buttons_slot, 0 );
	UxPutBottomAttachment( custom_buttons_slot, "attach_form" );
	UxPutRightWidget( custom_buttons_slot, "custom_buttons_s1" );
	UxPutRightOffset( custom_buttons_slot, 0 );
	UxPutRightAttachment( custom_buttons_slot, "attach_widget" );
	UxPutTopOffset( custom_buttons_slot, 0 );
	UxPutTopAttachment( custom_buttons_slot, "attach_form" );
	UxPutLeftOffset( custom_buttons_slot, 0 );
	UxPutLeftAttachment( custom_buttons_slot, "attach_form" );
	UxCreateWidget( custom_buttons_slot );

	UxPutRightOffset( buttons_form, 30 );
	UxPutRightAttachment( buttons_form, "attach_form" );
	UxPutTopWidget( buttons_form, "custom_buttons_form" );
	UxPutTopOffset( buttons_form, 0 );
	UxPutTopAttachment( buttons_form, "attach_widget" );
	UxPutLeftOffset( buttons_form, 30 );
	UxPutLeftAttachment( buttons_form, "attach_form" );
	UxCreateWidget( buttons_form );

	UxPutTopOffset( buttons_s1, 0 );
	UxPutTopAttachment( buttons_s1, "attach_form" );
	UxPutRightOffset( buttons_s1, 0 );
	UxPutRightAttachment( buttons_s1, "attach_form" );
	UxPutLeftOffset( buttons_s1, 0 );
	UxPutLeftAttachment( buttons_s1, "attach_form" );
	UxCreateWidget( buttons_s1 );

	UxPutTopWidget( buttons_rowcol, "buttons_s1" );
	UxPutRightOffset( buttons_rowcol, 0 );
	UxPutRightAttachment( buttons_rowcol, "attach_form" );
	UxPutTopOffset( buttons_rowcol, 0 );
	UxPutTopAttachment( buttons_rowcol, "attach_widget" );
	UxPutLeftOffset( buttons_rowcol, 0 );
	UxPutLeftAttachment( buttons_rowcol, "attach_form" );
	UxPutBottomOffset( buttons_rowcol, 0 );
	UxPutBottomAttachment( buttons_rowcol, "attach_form" );
	UxCreateWidget( buttons_rowcol );

	UxPutTopWidget( separator2, "buttons_form" );
	UxPutTopOffset( separator2, 0 );
	UxPutTopAttachment( separator2, "attach_widget" );
	UxPutRightOffset( separator2, 0 );
	UxPutRightAttachment( separator2, "attach_form" );
	UxPutLeftOffset( separator2, 0 );
	UxPutLeftAttachment( separator2, "attach_form" );
	UxCreateWidget( separator2 );

	UxPutBottomOffset( modeButtonSlot, 1400 );
	UxPutBottomAttachment( modeButtonSlot, "attach_form" );
	UxPutTopWidget( modeButtonSlot, "separator2" );
	UxPutTopOffset( modeButtonSlot, 30 );
	UxPutTopAttachment( modeButtonSlot, "attach_widget" );
	UxPutLeftOffset( modeButtonSlot, 30 );
	UxPutLeftAttachment( modeButtonSlot, "attach_form" );
	UxCreateWidget( modeButtonSlot );

	UxCreateWidget( relmode_1_1 );
	UxCreateWidget( relmode_1_many );
	UxCreateWidget( relmode_many_1 );
	UxCreateWidget( relmode_many_many );
	UxPutTopWidget( separator1, "separator2" );
	UxPutTopOffset( separator1, 0 );
	UxPutTopAttachment( separator1, "attach_widget" );
	UxPutLeftOffset( separator1, 0 );
	UxPutBottomOffset( separator1, 0 );
	UxPutLeftWidget( separator1, "modeButtonSlot" );
	UxPutLeftAttachment( separator1, "attach_widget" );
	UxPutBottomAttachment( separator1, "attach_form" );
	UxCreateWidget( separator1 );

	UxPutRightWidget( panner_frame, "separator1" );
	UxPutRightOffset( panner_frame, 0 );
	UxPutRightAttachment( panner_frame, "attach_widget" );
	UxPutLeftOffset( panner_frame, 30 );
	UxPutLeftAttachment( panner_frame, "attach_form" );
	UxPutBottomOffset( panner_frame, 30 );
	UxPutBottomAttachment( panner_frame, "attach_form" );
	UxCreateWidget( panner_frame );

	UxPutBottomOffset( status_line, 120 );
	UxPutBottomAttachment( status_line, "attach_form" );
	UxPutRightOffset( status_line, 120 );
	UxPutRightAttachment( status_line, "attach_form" );
	UxPutLeftWidget( status_line, "separator1" );
	UxPutLeftOffset( status_line, 120 );
	UxPutLeftAttachment( status_line, "attach_widget" );
	UxCreateWidget( status_line );

	UxPutTopWidget( viewWindowForm, "separator2" );
	UxPutTopOffset( viewWindowForm, 0 );
	UxPutTopAttachment( viewWindowForm, "attach_widget" );
	UxPutBottomWidget( viewWindowForm, "status_line" );
	UxPutBottomOffset( viewWindowForm, 120 );
	UxPutBottomAttachment( viewWindowForm, "attach_widget" );
	UxPutRightOffset( viewWindowForm, 30 );
	UxPutRightAttachment( viewWindowForm, "attach_form" );
	UxPutLeftWidget( viewWindowForm, "separator1" );
	UxPutLeftOffset( viewWindowForm, 0 );
	UxPutLeftAttachment( viewWindowForm, "attach_widget" );
	UxCreateWidget( viewWindowForm );

	UxPutTopOffset( viewWindow, 0 );
	UxPutTopAttachment( viewWindow, "attach_form" );
	UxPutBottomOffset( viewWindow, 0 );
	UxPutBottomAttachment( viewWindow, "attach_form" );
	UxPutLeftOffset( viewWindow, 0 );
	UxPutLeftAttachment( viewWindow, "attach_form" );
	UxCreateWidget( viewWindow );

	UxPutTopOffset( viewSepBut, 0 );
	UxPutTopAttachment( viewSepBut, "attach_form" );
	UxPutLeftOffset( viewSepBut, 16000 );
	UxPutLeftAttachment( viewSepBut, "attach_form" );
	UxCreateWidget( viewSepBut );

	UxPutBottomOffset( viewListSep, 0 );
	UxPutBottomAttachment( viewListSep, "attach_form" );
	UxPutTopWidget( viewListSep, "viewSepBut" );
	UxPutTopOffset( viewListSep, 0 );
	UxPutTopAttachment( viewListSep, "attach_widget" );
	UxPutLeftOffset( viewListSep, 16120 );
	UxPutLeftAttachment( viewListSep, "attach_form" );
	UxCreateWidget( viewListSep );

	UxPutTopOffset( viewList, 0 );
	UxPutTopAttachment( viewList, "attach_form" );
	UxPutBottomOffset( viewList, 0 );
	UxPutBottomAttachment( viewList, "attach_form" );
	UxPutRightOffset( viewList, 0 );
	UxPutRightAttachment( viewList, "attach_form" );
	UxPutLeftWidget( viewList, "viewListSep" );
	UxPutLeftOffset( viewList, 150 );
	UxPutLeftAttachment( viewList, "attach_widget" );
	UxCreateWidget( viewList );


	UxAddCallback( ViewerShell, XmNdestroyCallback,
			destroyCB_ViewerShell,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( form1, XmNfocusCallback,
			focusCB_form1,
			(XtPointer) UxViewerShellContext );

	UxPutMenuHelpWidget( menubar, "help" );

	UxAddCallback( file_open, XmNactivateCallback,
			activateCB_file_open,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_expand, XmNactivateCallback,
			activateCB_file_expand,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_defn, XmNactivateCallback,
			activateCB_file_open_defn,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_flowchart, XmNactivateCallback,
			activateCB_file_open_flowchart,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_vert_hierarchy, XmNactivateCallback,
			activateCB_file_open_vert_hierarchy,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_erd, XmNactivateCallback,
			activateCB_file_open_erd,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_dc, XmNactivateCallback,
			activateCB_file_open_dc,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_inheritance, XmNactivateCallback,
			activateCB_file_open_inheritance,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_open_calltree, XmNactivateCallback,
			activateCB_file_open_calltree,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_load, XmNactivateCallback,
			activateCB_file_load,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_unload, XmNactivateCallback,
			activateCB_file_unload,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_update, XmNactivateCallback,
			activateCB_file_update,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_save, XmNactivateCallback,
			activateCB_file_save,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_save_btn_bar, XmNactivateCallback,
			activateCB_file_save_btn_bar,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_close_view, XmNactivateCallback,
			activateCB_file_close_view,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_print, XmNactivateCallback,
			activateCB_file_print,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( file_close_window, XmNactivateCallback,
			activateCB_file_close_window,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_menu, XmNmapCallback,
			mapCB_edit_menu,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_cut, XmNactivateCallback,
			activateCB_edit_cut,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_copy, XmNactivateCallback,
			activateCB_edit_copy,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_paste, XmNactivateCallback,
			activateCB_edit_paste,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_search, XmNactivateCallback,
			activateCB_edit_search,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_change_editor, XmNvalueChangedCallback,
			valueChangedCB_edit_change_editor,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( edit_clear_selection, XmNactivateCallback,
			activateCB_edit_clear_selection,
			(XtPointer) UxViewerShellContext );

/*	UxAddCallback( manage_menu, XmNmapCallback,
			mapCB_manage_menu,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_home_project, XmNactivateCallback,
			activateCB_manage_home_project,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_get1, XmNactivateCallback,
			activateCB_manage_get1,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_get2, XmNactivateCallback,
			activateCB_manage_get2,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_put, XmNactivateCallback,
			activateCB_manage_put,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_copy, XmNactivateCallback,
			activateCB_manage_copy,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_diff, XmNactivateCallback,
			activateCB_manage_diff,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_lock, XmNactivateCallback,
			activateCB_manage_lock,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_unlock, XmNactivateCallback,
			activateCB_manage_unlock,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( manage_unget, XmNactivateCallback,
			activateCB_manage_unget,
			(XtPointer) UxViewerShellContext );
*/
	UxAddCallback( view_menu, XmNmapCallback,
			mapCB_view_menu,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_options_mode_buttons, XmNvalueChangedCallback,
			valueChangedCB_view_options_mode_buttons,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_options_buttonbar, XmNvalueChangedCallback,
			valueChangedCB_view_options_buttonbar,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_options_decorate, XmNvalueChangedCallback,
			valueChangedCB_view_options_decorate,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_split, XmNactivateCallback,
			activateCB_view_split,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( viewer_remove, XmNactivateCallback,
			activateCB_viewer_remove,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_collapse, XmNactivateCallback,
			activateCB_view_collapse,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_expand_level_b1, XmNactivateCallback,
			activateCB_view_expand_level_b1,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_expand_level_b2, XmNactivateCallback,
			activateCB_view_expand_level_b2,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_expand_level_b3, XmNactivateCallback,
			activateCB_view_expand_level_b3,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_expand_level_b4, XmNactivateCallback,
			activateCB_view_expand_level_b4,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_expand_level_b5, XmNactivateCallback,
			activateCB_view_expand_level_b5,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( gra_fresh, XmNactivateCallback,
			activateCB_gra_fresh,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_magnify, XmNactivateCallback,
			activateCB_view_magnify,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_shrink, XmNactivateCallback,
			activateCB_view_shrink,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_zoomtofit, XmNactivateCallback,
			activateCB_view_zoomtofit,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_reset, XmNactivateCallback,
			activateCB_view_reset,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( view_pref, XmNactivateCallback,
			activateCB_view_pref,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_TOC, XmNactivateCallback,
			activateCB_help_TOC,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_index, XmNactivateCallback,
			activateCB_help_index,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_viewer_menus, XmNactivateCallback,
			activateCB_help_viewer_menus,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_on_viewers, XmNactivateCallback,
			activateCB_help_on_viewers,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_on_help, XmNactivateCallback,
			activateCB_help_on_help,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_release_notes, XmNactivateCallback,
			activateCB_help_release_notes,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_getting_started, XmNactivateCallback,
			activateCB_help_getting_started,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( help_graphics_conventions, XmNactivateCallback,
			activateCB_help_graphics_conventions,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( customize_mode, XmNvalueChangedCallback,
			valueChangedCB_customize_mode,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( remove_mode, XmNvalueChangedCallback,
			valueChangedCB_remove_mode,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( normal_mode, XmNvalueChangedCallback,
			valueChangedCB_normal_mode,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( relmode_1_1, XmNvalueChangedCallback,
			valueChangedCB_relmode_1_1,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( relmode_1_many, XmNvalueChangedCallback,
			valueChangedCB_relmode_1_many,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( relmode_many_1, XmNvalueChangedCallback,
			valueChangedCB_relmode_many_1,
			(XtPointer) UxViewerShellContext );

	UxAddCallback( relmode_many_many, XmNvalueChangedCallback,
			valueChangedCB_relmode_many_many,
			(XtPointer) UxViewerShellContext );


	/* Finally, call UxRealizeInterface to create the X windows
	   for the widgets created above. */

	UxRealizeInterface( ViewerShell );

	return ( ViewerShell );
}

/*******************************************************************************
	The following function includes the code that was entered
	in the 'Initial Code' and 'Final Code' sections of the
	Declarations Editor. This function is called from the
	'Interface function' below.
*******************************************************************************/

static Widget	_Ux_create_ViewerShell( _Uxvshell_obj, _Uxcreate_minibrowser )
	void	*_Uxvshell_obj;
	int	_Uxcreate_minibrowser;
{
	swidget                 rtrn;
	_UxCViewerShell         *UxContext;

	UxViewerShellContext = UxContext =
		(_UxCViewerShell *) UxMalloc( sizeof(_UxCViewerShell) );

	vshell_obj = _Uxvshell_obj;
	create_minibrowser = _Uxcreate_minibrowser;
	rtrn = _Uxbuild_ViewerShell();

	/* final code */
	if (gtShell_icon_pixmap())
		XtVaSetValues(UxGetWidget(ViewerShell), XmNiconPixmap, gtShell_icon_pixmap(),
				XmNiconMask, gtShell_icon_mask(), NULL);
	/*if (_ffd_()) */
	/*    XtUnmanageChild(UxGetWidget(file_open_erd));*/
	/*if (_fff_()) */
	/*    XtUnmanageChild(UxGetWidget(debug)); */
	XSetWindowBackgroundPixmap(UxDisplay, XtWindow(UxGetWidget(ViewerShell)), None);
	instance.vshell = vshell_obj;
	instance.filesel = NULL;
	instance.panner = create_Panner(UxGetWidget(panner_frame),
				viewerShell_panner_callback, instance.vshell);
	instance.interp_mode = 0;
	XtVaSetValues(instance.panner, XmNwidth, 40, XmNheight, 40, NULL);
	XtManageChild(instance.panner);
	Panner_perspective(instance.panner, 0, 0, 100, 100, 30, 30, 40, 40);
	XtVaSetValues(UxGetWidget(viewWindow), XmNrightAttachment, XmATTACH_WIDGET,
	              XmNrightWidget, UxGetWidget(viewListSep),
		      XmNrightOffset, 150,
		      NULL);
	/* set viewerShell's pointers */
	viewerShell_initWidgetSlots(instance.vshell,
		UxGetWidget(custom_buttons_slot),
		UxGetWidget(viewWindow),
		UxGetWidget(menubar),
		UxGetWidget(viewList), /*UxGetWidget(debug_stdout_window),*/
		UxGetWidget(browser_icon),
		instance.panner);
	manage_widget_list(False, &relmode_1_1, &relmode_1_many,
			   &relmode_many_1, &relmode_many_many, NULL);
	if(create_minibrowser){
		miniBrowser_initialize_slide_button(UxGetWidget(viewSepBut), UxGetWidget(viewListSep));
	    	miniBrowser_initialize_slide_separator(UxGetWidget(viewListSep));
	}else{
		vs_display_mini_browser(UxGetWidget(rtrn), 0);
		XtVaSetValues(UxGetWidget(menubar), XmNrightAttachment, XmATTACH_FORM, 0);
		XtUnmanageChild(UxGetWidget(browser_icon));
	}
	XtVaSetValues(UxGetWidget(rtrn), XmNdeleteResponse, XmDO_NOTHING, 0); 
	{
	Atom WM_DELETE_WINDOW;
	WM_DELETE_WINDOW = XmInternAtom(XtDisplay(UxGetWidget(rtrn)), "WM_DELETE_WINDOW", False); 
	XmAddWMProtocolCallback (UxGetWidget(rtrn), WM_DELETE_WINDOW, viewerShell_close_button_callback, UxGetWidget(rtrn)); 
	}
	vs_special_buttons(UxGetWidget(rtrn), vsSpecialButtonsNone);
	vs_set_buttonbar(UxGetWidget(rtrn), False);
	XmToggleButtonSetState(UxGetWidget(view_options_mode_buttons), False, True);
	UxPopupInterface(rtrn, 0);
	return UxGetWidget(rtrn);
}

/*******************************************************************************
	The following is the 'Interface function' which is the
	external entry point for creating this interface.
	This function should be called from your application or from
	a callback function.
*******************************************************************************/

Widget	create_ViewerShell( _Uxvshell_obj, _Uxcreate_minibrowser )
	void	*_Uxvshell_obj;
	int	_Uxcreate_minibrowser;
{
	Widget			_Uxrtrn;

	_Uxrtrn = _Ux_create_ViewerShell( _Uxvshell_obj, _Uxcreate_minibrowser );

	return ( _Uxrtrn );
}

/*******************************************************************************
	END OF FILE
*******************************************************************************/


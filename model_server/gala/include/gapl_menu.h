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
//-----------------------------------------------------------------------------
// gapl_menu.h
//
// Menu manipulation commands.
//-----------------------------------------------------------------------------

#ifndef _apl_menu_h
#define _apl_menu_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef _interp_h
#include "ginterp.h"
#endif

class ViewerMenuBar;
class ViewerPopupMenuList;


// TCL Command Utilities.
void * dis_findItem(Tcl_Interp*& interp, vchar* name);

// TCL Command Procedures.
extern Tcl_CmdProc dis_DirCreate;
extern Tcl_CmdProc dis_DirExpandVariables;
extern Tcl_CmdProc dis_DirDestroy;
extern Tcl_CmdProc dis_DirDetermineFilter;
extern Tcl_CmdProc dis_DirDetermineFlags;
extern Tcl_CmdProc dis_DirFind;
extern Tcl_CmdProc dis_DirFindFileFromPath;
extern Tcl_CmdProc dis_DirGetFilter;
extern Tcl_CmdProc dis_DirGetFlags;
extern Tcl_CmdProc dis_DirGetGlobalFilter;
extern Tcl_CmdProc dis_DirGetGlobalBeforeFlags;
extern Tcl_CmdProc dis_DirGetGlobalAfterFlags;
extern Tcl_CmdProc dis_DirIsDirectory;
extern Tcl_CmdProc dis_DirItem;
extern Tcl_CmdProc dis_DirItemFullname;
extern Tcl_CmdProc dis_DirItemName;
extern Tcl_CmdProc dis_DirRefresh;
extern Tcl_CmdProc dis_DirRestore;
extern Tcl_CmdProc dis_DirSave;
extern Tcl_CmdProc dis_DirSetFilter;
extern Tcl_CmdProc dis_DirSetFlags;
extern Tcl_CmdProc dis_DirIsFlags;
extern Tcl_CmdProc dis_DirSetCompilerFlags;
extern Tcl_CmdProc dis_DirSetGlobalFilter;
extern Tcl_CmdProc dis_DirSetGlobalBeforeFlags;
extern Tcl_CmdProc dis_DirSetGlobalAfterFlags;
extern Tcl_CmdProc dis_DirSetExcludeFlag;
extern Tcl_CmdProc dis_DirGetExcludeFlag;
extern Tcl_CmdProc dis_DirSize;
extern Tcl_CmdProc dis_ExecuteInSubDirs;
extern Tcl_CmdProc dis_EvalFile;
extern Tcl_CmdProc dis_SetPollProc;
extern Tcl_CmdProc dis_StopPollProc;
extern Tcl_CmdProc dis_CountPDFfiles;
extern Tcl_CmdProc dis_GeneratePDF;
extern Tcl_CmdProc dis_GetAppDirectory;
extern Tcl_CmdProc dis_GetStartupDirectory;
extern Tcl_CmdProc dis_ItemName;
extern Tcl_CmdProc dis_ItemEnable;
extern Tcl_CmdProc dis_ItemHide;
extern Tcl_CmdProc dis_KillSystemCommand;
extern Tcl_CmdProc dis_DirMakeDirectory;
extern Tcl_CmdProc dis_MenuAddItemArgument;
extern Tcl_CmdProc dis_MenuAddMenuArgument;
extern Tcl_CmdProc dis_MenuAppendGroupSubmenu;
extern Tcl_CmdProc dis_MenuAppendItem;
extern Tcl_CmdProc dis_MenuAppendLabelItem;
extern Tcl_CmdProc dis_MenuAppendSeparator;
extern Tcl_CmdProc dis_MenuAppendSubmenu;
extern Tcl_CmdProc dis_MenuAppendToggleItem;
extern Tcl_CmdProc dis_MenuCreate;
extern Tcl_CmdProc dis_MenuCreateDynamic;
extern Tcl_CmdProc dis_MenuCreateGroup;
extern Tcl_CmdProc dis_MenuSetToggleItem;
extern Tcl_CmdProc dis_MenuItemEnable;
extern Tcl_CmdProc dis_MenuItemDisable;
extern Tcl_CmdProc dis_Message;
extern Tcl_CmdProc dis_TimerRet;
extern Tcl_CmdProc reval_async;
extern Tcl_CmdProc reval;
extern Tcl_CmdProc leval;
extern Tcl_CmdProc dis_SystemCommand;
extern Tcl_CmdProc dis_SystemCommandWait;
extern Tcl_CmdProc dis_GalaLogFile;
extern Tcl_CmdProc dis_DevNull;
extern Tcl_CmdProc dis_ViewFile;
extern Tcl_CmdProc dis_StopViewUpdating;
extern Tcl_CmdProc dis_BlockSignals;
extern Tcl_CmdProc dis_ViewText;
extern Tcl_CmdProc dis_GetText;
extern Tcl_CmdProc dis_ViewHighliteLine;
extern Tcl_CmdProc dis_build_menu;
extern Tcl_CmdProc dis_SetViewVariables;
extern Tcl_CmdProc dis_close_dialog;
extern Tcl_CmdProc dis_customize_toolbar;
extern Tcl_CmdProc dis_setvar;
extern Tcl_CmdProc dis_getvar;
extern Tcl_CmdProc dis_file;
extern Tcl_CmdProc dis_gdInitButton;
extern Tcl_CmdProc dis_gdInitNotebookPage;
extern Tcl_CmdProc dis_gdInitComboBox;
extern Tcl_CmdProc dis_gdInitExclusiveGroup;
extern Tcl_CmdProc dis_gdInitLabel;
extern Tcl_CmdProc dis_gdInitRuler;
extern Tcl_CmdProc dis_gdInitListItem;
extern Tcl_CmdProc dis_gdInitListView;
extern Tcl_CmdProc dis_gdInitOptionMenu;
extern Tcl_CmdProc dis_gdInitPopDownComboBox;
extern Tcl_CmdProc dis_gdInitTextItem;
extern Tcl_CmdProc dis_gdInitNumberSpinner;
extern Tcl_CmdProc dis_gdNumberSpinner_SetMax;
extern Tcl_CmdProc dis_gdNumberSpinner_SetMin;
extern Tcl_CmdProc dis_gdInitToggle;
extern Tcl_CmdProc dis_gdInitProgressItem;
extern Tcl_CmdProc dis_gdInitSlider;
extern Tcl_CmdProc dis_gdInitSash;
extern Tcl_CmdProc dis_gdInitSpreadSheet;
extern Tcl_CmdProc dis_save_filter;
extern Tcl_CmdProc dis_load_filter;
extern Tcl_CmdProc dis_modifiedflag;
extern Tcl_CmdProc dis_closefilter;
extern Tcl_CmdProc dis_current_filter_name;
extern Tcl_CmdProc dis_launch_dialog;
extern Tcl_CmdProc dis_launch_selBrowser;
extern Tcl_CmdProc dis_layer;
extern Tcl_CmdProc dis_print;
extern Tcl_CmdProc dis_quit;
extern Tcl_CmdProc dis_view_cl;
extern Tcl_CmdProc dis_view_manage;
extern Tcl_CmdProc dis_viewer_new;
extern Tcl_CmdProc dis_ProgressStartTime;
extern Tcl_CmdProc dis_ProgressElapsedTime;
extern Tcl_CmdProc dis_ProgressEstimatedRelative;
extern Tcl_CmdProc dis_ProgressEstimatedEnd;
extern Tcl_CmdProc dis_ProgressStart;
extern Tcl_CmdProc dis_eval;
extern Tcl_CmdProc dis_help;
extern Tcl_CmdProc dis_Enable_MenuToolBar;

extern Tcl_CmdProc dis_gdInitAttributeEditor;
extern Tcl_CmdProc dis_gdAttributeEditorAddAttribute;
extern Tcl_CmdProc dis_gdAttributeEditorSetAttributeValue;
extern Tcl_CmdProc dis_gdAttributeEditorReset;
extern Tcl_CmdProc dis_gdAttributeEditorAddEnumAttributeValue;

extern Tcl_CmdProc dis_gdTextItemSetText;
extern Tcl_CmdProc dis_gdTextItemGetText;

extern Tcl_CmdProc dis_gdInitDrawing;
extern Tcl_CmdProc dis_gdDrawingAddItem;
extern Tcl_CmdProc dis_gdDrawingRemoveItem;
extern Tcl_CmdProc dis_gdDrawingSetGrid;
extern Tcl_CmdProc dis_gdDrawingSetGridDimensions;
extern Tcl_CmdProc dis_gdDrawingSetIconTitle;
extern Tcl_CmdProc dis_gdDrawingSetIconTip;
extern Tcl_CmdProc dis_gdDrawingSetDragProhibited;
extern Tcl_CmdProc dis_gdDrawingSetSingleSelection;
extern Tcl_CmdProc dis_gdDrawingClear;
extern Tcl_CmdProc dis_gdDrawingSelectItem;

extern Tcl_CmdProc dis_gpi_connect;
extern Tcl_CmdProc dis_gpi_disconnect;
extern Tcl_CmdProc dis_gpi_status;
extern Tcl_CmdProc dis_gpi_getMessages;
extern Tcl_CmdProc dis_gpi_clearMessages;
extern Tcl_CmdProc dis_gpi_countClients;
extern Tcl_CmdProc dis_gpi_isActive;
extern Tcl_CmdProc dis_gpi_getClientStr;
extern Tcl_CmdProc dis_gpi_init;

extern Tcl_CmdProc dis_getListStrRes;
extern Tcl_CmdProc dis_getTextStrRes;
extern Tcl_CmdProc dis_putStringPreference;
extern Tcl_CmdProc dis_getStringPreference;
extern Tcl_CmdProc dis_gdAddToCombo;
extern Tcl_CmdProc dis_gdAddListToCombo;
extern Tcl_CmdProc dis_gdClearCombo;
extern Tcl_CmdProc dis_gdComboSelect;
extern Tcl_CmdProc dis_gdComboBoxSetMatch;
extern Tcl_CmdProc dis_gdNotebookPageEnable;
extern Tcl_CmdProc dis_service_connect;
extern Tcl_CmdProc dis_service_disconnect;
extern Tcl_CmdProc dis_service_connect_remote;
extern Tcl_CmdProc dis_service_disconnect_remote;
extern Tcl_CmdProc dis_gdListItemRefresh; 
extern Tcl_CmdProc dis_gdListItemSelect; 
extern Tcl_CmdProc dis_gdListItemSelection; 
extern Tcl_CmdProc dis_gdListItem_init_width;

extern Tcl_CmdProc dis_gdListViewRemoveRow; 
extern Tcl_CmdProc dis_gdListViewFlush; 

extern Tcl_CmdProc dis_gdListViewSelect; 
extern Tcl_CmdProc dis_gdListViewRefresh; 
extern Tcl_CmdProc dis_gdListViewChangeCheckStatus;

extern Tcl_CmdProc dis_gdListViewAddPopupItem;
extern Tcl_CmdProc dis_gdListViewUseInternalData;
extern Tcl_CmdProc dis_gdListViewSetData;

extern Tcl_CmdProc dis_gdListViewAddCheckCell;
extern Tcl_CmdProc dis_gdListViewChangeCheckIcons;


extern Tcl_CmdProc dis_gdListViewGetCellData;
extern Tcl_CmdProc dis_gdListViewGetCellValue;
extern Tcl_CmdProc dis_gdListViewSetCellMenu;
extern Tcl_CmdProc dis_gdListViewSetChangeCallback;
extern Tcl_CmdProc dis_gdListViewSetCellChange;
extern Tcl_CmdProc dis_gdListViewSetDblClickCallback;

extern Tcl_CmdProc dis_gdDialogEval;
extern Tcl_CmdProc dis_gdDialogIDEval;
extern Tcl_CmdProc dis_gdDialogSetTitle;
extern Tcl_CmdProc dis_query;
extern Tcl_CmdProc dis_stop;
extern Tcl_CmdProc dis_rtl_get_id;
extern Tcl_CmdProc dis_rtl_select;
extern Tcl_CmdProc dis_rtl_set_caller;
extern Tcl_CmdProc dis_rtl_clear;
extern Tcl_CmdProc dis_rtl_preserve_selection;
extern Tcl_CmdProc dis_ide_send_command;
extern Tcl_CmdProc dis_ide_get_selection;
extern Tcl_CmdProc dis_IDE_current_selection;
extern Tcl_CmdProc dis_ide_start;
extern Tcl_CmdProc dis_viewer_pop_active;
extern Tcl_CmdProc dis_rtl_post_init;
extern Tcl_CmdProc dis_aset_eval_cmd;
extern Tcl_CmdProc dis_aset_eval_cmd_async;
extern Tcl_CmdProc error_Message;


extern Tcl_CmdProc dis_vdasserv;
extern Tcl_CmdProc dis_remove_directory;


extern Tcl_CmdProc dis_java_build;
extern Tcl_CmdProc dis_java_follow;
extern Tcl_CmdProc dis_java_destroy;

extern Tcl_CmdProc dis_build_service_list;
extern Tcl_CmdProc dis_get_service_list_size;
extern Tcl_CmdProc dis_get_service_list_entry;
extern Tcl_CmdProc dis_get_service_list_index;
extern Tcl_CmdProc dis_SelectionFromList; 
extern Tcl_CmdProc dis_file_write;
extern Tcl_CmdProc dis_file_read;
extern Tcl_CmdProc dis_getenv;
extern Tcl_CmdProc dis_PasteViewerSelection;
extern Tcl_CmdProc dis_ide_open_definition;
extern Tcl_CmdProc dis_activate_notebook_page;
extern Tcl_CmdProc dis_CreateBatch;
extern Tcl_CmdProc dis_CancelBatch;
extern Tcl_CmdProc dis_TextItemSetModifiable;
extern Tcl_CmdProc dis_OpenSettings;
extern Tcl_CmdProc dis_CloseSettings;
extern Tcl_CmdProc dis_ReadSettingsVar;
extern Tcl_CmdProc dis_WriteSettingsVar;
extern Tcl_CmdProc dis_FlushSettings;
extern Tcl_CmdProc dis_tempnam;
extern Tcl_CmdProc dis_unlink;
extern Tcl_CmdProc dis_update_gdTextItem;
extern Tcl_CmdProc dis_AccessAddMenu;
extern Tcl_CmdProc dis_AccessAddItem;
extern Tcl_CmdProc dis_sel_size;
extern Tcl_CmdProc dis_register_aset_callback;
extern vbool aplMenuLoad (ViewerMenuBar*, const char*);
extern vbool aplMenuLoad (ViewerPopupMenuList*, const char*);

// gdTextSpinner command definision
extern Tcl_CmdProc  dis_gdTextSpinSelectByIndex;    // <void>  <-  <name> <index>
extern Tcl_CmdProc  dis_gdTextSpinSelectByText;     // <void>  <-  <name> <text>
extern Tcl_CmdProc  dis_gdTextSpinAddText;          // <void>  <-  <name> <text>
extern Tcl_CmdProc  dis_gdTextSpinChangeText;       // <void>  <-  <name> <text> <index>
extern Tcl_CmdProc  dis_gdTextSpinClear;            // <void>  <-  <name>
extern Tcl_CmdProc  dis_gdTextSpinDeleteText;       // <void>  <-  <name> <text>
extern Tcl_CmdProc  dis_gdTextSpinDeleteIndex;      // <void>  <-  <name> <index>
extern Tcl_CmdProc  dis_gdTextSpinGetSelectedText;  // <text>  <-  <name> 
extern Tcl_CmdProc  dis_gdTextSpinGetSelectedIndex; // <index> <-  <name> 
extern Tcl_CmdProc  dis_gdTextSpinGetTextAt;        // <text>  <-  <name> <index>
extern Tcl_CmdProc  dis_gdTextSpinGetLinesAnount;   // <int>   <-  <name>
extern Tcl_CmdProc  dis_gdTextSpinGetIndexFromText; // <index>  <-  <name> <text>
extern Tcl_CmdProc  dis_gdTextSpinSort;             // <void>  <-  <name>
extern Tcl_CmdProc  dis_gdTextSpinSetChangeCallback;// <void>  <-  <name>

#endif // _apl_menu_h

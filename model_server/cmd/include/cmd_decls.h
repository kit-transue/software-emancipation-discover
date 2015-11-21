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
extern "C" {
typedef struct app app;
typedef struct app *appPtr;
typedef struct appTree appTree;
typedef struct appTree *appTreePtr;
typedef int appType;


typedef struct _WidgetRec* Widget;
typedef struct viewer viewer;

/* declarations missing from all those header files */
appTreePtr C_node (appType,char*, ...);
appPtr C_hdr (appType,char*);
void cmd_convert_window_command (int,char*,int,int);
void cmd_convert_window_report (int,char*,int,int);
void cmd_epoch_macro (unsigned char*);
void cmd_epoch_window_command (int,int,int,int);
void cmd_epoch_window_report (int,int,int,int);
void cmd_find_file (int,int,int,int,char*);
int cmd_report_node_def (appTree*);
void cmd_set_script_version (int);
int cmd_set_validation_group (char*);
void cmd_set_validation_level (int);
void cmd_switch_to_buffer (int);
void dbg_prompt (char*);
void dr_clear_selection ();
int dr_copy ();
int dr_cut ();
int dr_paste ();
void em_set_speed (int);
void gra_begin_connection (int,int,int,double,double,char*,char*);
void gra_collapse_expand (int,int,int,double,double,char*,char*);
void gra_drop (int,int,int,double,double,char*,char*);
void gra_end_connection (int,int,int,double,double,char*,char*);
viewer *gra_focus_viewer();
void gra_modal_click (int,int,int,double,double,char*,char*);
void gra_open_node (int,int,int,double,double,char*,char*);
void gra_pickup (int,int,int,double,double,char*,char*);
void gra_select (int,int,int,double,double,char*,char*);
void gra_symbol_menu (int,int,int,double,double,char*,char*);
int pdcmd_execute (char*,char*);
void ste_cmd_mouse ();
int ste_cmd_save (app*);
int ste_cursor (appTree*,int,int);
int ste_do_replace_remove (appTree*,appTree*,char*,int,int,appTree*,appTree*);
int ste_insert_text (appTree*,char*,int);
int ste_raw_cursor (int,int);
int ste_region (appTree*,int,int,appTree*,int,int,int);
int ste_replace_text (appTree*,char*);
int synchronize_now (char*);
Widget ui_addselect (char*,char*);
Widget ui_button_event (char*,int,int);
Widget ui_list_action (char*);
Widget ui_push_button (char*);
Widget ui_replace_text (char*,char*,int);
Widget ui_select_gadget (char*,char*);
Widget ui_select_label (char*,char*,int);
Widget ui_select_list (char*,char*);
Widget ui_select_top (char*,int);
viewer* ui_select_viewer (int);
void ui_validate_match_count (int);
void ui_validate_namedInt (char*,int);
void ui_validate_namedString (char*,char*);
void ui_validate_obj (char*,char*,char*);
void ui_viewer_validate (int,char*,int);
int vsystem_from_script (const char*);
}

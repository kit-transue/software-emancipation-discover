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
class Application;
#include "Application.h"
typedef void async;
typedef char* _charptr;
typedef vstr* _vstrptr;
async rcall_gc_rtl_refresh (Application* app, int id);
async gc_rtl_refresh (int id);
int rcall_gc_create_layer (Application* app, int type);
int gc_create_layer (int type);
async rcall_gc_close_layer (Application* app, int id);
async gc_close_layer (int id);
async rcall_gc_set_layer_name (Application* app, int layer, _vstrptr name);
async gc_set_layer_name (int layer, _vstrptr name);
async rcall_gc_set_layer (Application* app, int id);
async gc_set_layer (int id);
async rcall_gc_display_message (Application* app, _vstrptr msg);
async gc_display_message (_vstrptr msg);
async rcall_gc_create_view_layer (Application* app, int client_id, _vstrptr name);
async gc_create_view_layer (int client_id, _vstrptr name);
async rcall_gc_open_file (Application* app, _vstrptr name);
async gc_open_file (_vstrptr name);
async rcall_gc_close_file (Application* app, _vstrptr name);
async gc_close_file (_vstrptr name);
async rcall_access_shell_set_result (Application* app, int id, _charptr pszTag, _charptr pszResult);
async access_shell_set_result (int id, _charptr pszTag, _charptr pszResult);
int rcall_send_to_global_client_eval (Application* app, _vstrptr str);
int send_to_global_client_eval (_vstrptr str);
async rcall_send_to_global_client_eval_async (Application* app, _vstrptr str);
async send_to_global_client_eval_async (_vstrptr str);
int rcall_send_notification_string_to_client (Application* app, _vstrptr str);
int send_notification_string_to_client (_vstrptr str);
int rcall_get_rtl_info (Application* app, int& last_sel, int& elements, int& results, dyn_vstrptr& scope);
int get_rtl_info (int& last_sel, int& elements, int& results, dyn_vstrptr& scope);
int rcall_get_selection_rtl (Application* app);
int get_selection_rtl ();
int rcall_get_activate_elements_rtl (Application* app);
int get_activate_elements_rtl ();
int rcall_dis_display_string_OK_cancel (Application* app, _charptr title, _charptr prompt_str, _charptr err_msg, dyn_vstrptr& result);
int dis_display_string_OK_cancel (_charptr title, _charptr prompt_str, _charptr err_msg, dyn_vstrptr& result);
_charptr rcall_dis_choose_file_simple (Application* app, _charptr title);
_charptr dis_choose_file_simple (_charptr title);
static _vstrptr rcall_dis_servereval (Application* app, _vstrptr service, _vstrptr str);
static _vstrptr dis_servereval (_vstrptr service, _vstrptr str);
static async rcall_dis_servereval_async (Application* app, _vstrptr service, _vstrptr str);
static async dis_servereval_async (_vstrptr service, _vstrptr str);
_vstrptr rcall_dis_DISui_eval (Application* app, _vstrptr str);
_vstrptr dis_DISui_eval (_vstrptr str);
async rcall_dis_DISui_eval_async (Application* app, _vstrptr str);
async dis_DISui_eval_async (_vstrptr str);
void DISuiServerInit();

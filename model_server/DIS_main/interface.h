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
int rcall_dis_connect_app (Application* app, _vstrptr name);
int dis_connect_app (_vstrptr name);
int rcall_dis_rtl_create (Application* app, int a0);
int dis_rtl_create (int a0);
async rcall_dis_rtl_set_caller (Application* app, int id, _vstrptr caller);
async dis_rtl_set_caller (int id, _vstrptr caller);
async rcall_dis_rtl_init (Application* app, int id, _vstrptr spec);
async dis_rtl_init (int id, _vstrptr spec);
async rcall_dis_rtl_clear (Application* app, int id);
async dis_rtl_clear (int id);
async rcall_dis_rtl_select (Application* app, int id, _vstrptr spec);
async dis_rtl_select (int id, _vstrptr spec);
async rcall_dis_rtl_query_links (Application* app, int src_id, _vstrptr spec, int trg_id);
async dis_rtl_query_links (int src_id, _vstrptr spec, int trg_id);
async rcall_dis_rtl_query_proj (Application* app, int src_id, int pos, int trg_id);
async dis_rtl_query_proj (int src_id, int pos, int trg_id);
async rcall_dis_rtl_query_top_proj (Application* app, int trg_id);
async dis_rtl_query_top_proj (int trg_id);
async rcall_dis_rtl_query_parent_proj (Application* app, int trg_id);
async dis_rtl_query_parent_proj (int trg_id);
_vstrptr rcall_dis_rtl_get_title (Application* app, int id);
_vstrptr dis_rtl_get_title (int id);
_vstrptr rcall_dis_rtl_get_item (Application* app, int id, int idx);
_vstrptr dis_rtl_get_item (int id, int idx);
int rcall_dis_rtl_get_unfilteredSize (Application* app, int id);
int dis_rtl_get_unfilteredSize (int id);
int rcall_dis_rtl_isFiltered (Application* app, int id);
int dis_rtl_isFiltered (int id);
int rcall_dis_rtl_get_size (Application* app, int id);
int dis_rtl_get_size (int id);
int rcall_dis_rtl_get_type (Application* app, int id);
int dis_rtl_get_type (int id);
int rcall_dis_rtl_destroy (Application* app, int id);
int dis_rtl_destroy (int id);
int rcall_dis_rtl_edit_copy (Application* app, int source, int target);
int dis_rtl_edit_copy (int source, int target);
int rcall_dis_rtl_add_to_exist_xref (Application* app, int source_id, int target_id);
int dis_rtl_add_to_exist_xref (int source_id, int target_id);
int rcall_dis_rtl_add_to_exist (Application* app, int source_id, int target_id);
int dis_rtl_add_to_exist (int source_id, int target_id);
int rcall_dis_rtl_remove_selected (Application* app, int source_id);
int dis_rtl_remove_selected (int source_id);
int rcall_dis_rtl_edit_paste (Application* app, int source, int target);
int dis_rtl_edit_paste (int source, int target);
int rcall_dis_layer_create (Application* app, int a0, int a1, int a2);
int dis_layer_create (int a0, int a1, int a2);
int rcall_dis_layer_destroy (Application* app, int a0);
int dis_layer_destroy (int a0);
async rcall_dis_set_layer (Application* app, int a0);
async dis_set_layer (int a0);
int rcall_dis_open_selection (Application* app, int rtl_id, int rep_type);
int dis_open_selection (int rtl_id, int rep_type);
_vstrptr rcall_server_eval (Application* app, _vstrptr server, _vstrptr str);
_vstrptr server_eval (_vstrptr server, _vstrptr str);
async rcall_server_eval_async (Application* app, _vstrptr server, _vstrptr str);
async server_eval_async (_vstrptr server, _vstrptr str);
async rcall_dis_show_file (Application* app, _vstrptr name);
async dis_show_file (_vstrptr name);
int rcall_dis_shutdown_server (Application* app);
int dis_shutdown_server ();
int rcall_dis_viewer_selection_exists (Application* app);
int dis_viewer_selection_exists ();
async rcall_dis_insert_selection_into_rtl (Application* app, int rtl_id);
async dis_insert_selection_into_rtl (int rtl_id);
async rcall_dis_open_selected_in_view (Application* app);
async dis_open_selected_in_view ();
int rcall_dis_create_package_input (Application* app, int fUseScope, int rtl_id, _charptr pszScope, _charptr pszFile);
int dis_create_package_input (int fUseScope, int rtl_id, _charptr pszScope, _charptr pszFile);
int rcall_dis_rtl_processKey (Application* app, int id, _vstrptr thekey, int mod, int startat);
int dis_rtl_processKey (int id, _vstrptr thekey, int mod, int startat);
async rcall_cli_process_shell_request (Application* app, _charptr in, int id, _charptr tag);
async cli_process_shell_request (_charptr in, int id, _charptr tag);
int rcall_dis_access_eval (Application* app, _charptr cmd, _charptr& out);
int dis_access_eval (_charptr cmd, _charptr& out);
async rcall_dis_access_eval_async (Application* app, _charptr str);
async dis_access_eval_async (_charptr str);
int rcall_dis_is_module (Application* app, int id, int pos);
int dis_is_module (int id, int pos);
static _vstrptr rcall_dis_servereval (Application* app, _vstrptr service, _vstrptr str);
static _vstrptr dis_servereval (_vstrptr service, _vstrptr str);
static async rcall_dis_servereval_async (Application* app, _vstrptr service, _vstrptr str);
static async dis_servereval_async (_vstrptr service, _vstrptr str);
_vstrptr rcall_dis_DISmain_eval (Application* app, _vstrptr str);
_vstrptr dis_DISmain_eval (_vstrptr str);
async rcall_dis_DISmain_eval_async (Application* app, _vstrptr str);
async dis_DISmain_eval_async (_vstrptr str);
void DISmainServerInit();

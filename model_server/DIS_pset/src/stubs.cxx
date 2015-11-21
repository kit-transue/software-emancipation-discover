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
#include <cLibraryFunctions.h>
#include "../../DIS_main/interface.h"
#include "../../DIS_ui/interface.h"

extern Application* DISui_app;

#include <vport.h>
#include vstrHEADER
#include "tcl.h"

typedef unsigned char * _vstrptr;

static void pset_stub (char* msg, int silent=0) {
    static int squelch = -1;
    char* silenceVal;
    if (squelch == -1) {
    	silenceVal = OSapi_getenv("PSET_SILENCE_STUBS");
    	if(silenceVal!=NULL)
			squelch = (silenceVal[0]!='0');
     	else
			squelch = 1;
    }
    if(!silent && !squelch) OSapi_fprintf (stderr, "%s called, but is not defined\n", msg);
//    OSapi_fprintf (stderr, "%s called, but is not defined\n", msg);
}      

class Application;

void remote_call_async (Application * app, _vstrptr server, _vstrptr str) {

    rcall_dis_DISui_eval_async (DISui_app, str);

}


void rcall_dis_rtl_set_caller (Application*, int id, _vstrptr caller)
{
    dis_rtl_set_caller (id, vstrClone (caller));
}

int rcall_dis_rtl_create (Application*, int a0)
{
    return dis_rtl_create (a0);
}

void  rcall_dis_rtl_init (Application*, int id, _vstrptr spec)
{
    dis_rtl_init (id, vstrClone (spec));
}

void rcall_dis_rtl_clear (Application*, int id)
{
    dis_rtl_clear (id);
}

void rcall_dis_rtl_select (Application*, int id, _vstrptr spec)
{
    dis_rtl_select (id, vstrClone (spec));
}

void rcall_dis_rtl_query_links (Application*, int src_id, _vstrptr spec, int trg_id)
{
    dis_rtl_query_links (src_id, vstrClone (spec), trg_id);
}

void rcall_dis_rtl_query_proj (Application*, int src_id, int pos, int trg_id)
{
    dis_rtl_query_proj (src_id, pos, trg_id);
}

void rcall_dis_rtl_query_top_proj (Application*, int trg_id)
{
    dis_rtl_query_top_proj (trg_id);
}

_vstrptr rcall_dis_rtl_get_title (Application*, int id)
{
    return vstrClone (dis_rtl_get_title (id));
}

int rcall_dis_rtl_get_unfilteredSize (Application*, int id)
{
    return dis_rtl_get_unfilteredSize (id);
}

int rcall_dis_rtl_get_type (Application*, int id)
{
    return dis_rtl_get_type (id);
}

int rcall_dis_rtl_destroy (Application*, int id)
{
    return dis_rtl_destroy (id);
}

int rcall_dis_rtl_processKey (Application*, int id, _vstrptr theKey, int mod, int startat)
{
    return dis_rtl_processKey (id, vstrClone (theKey), mod, startat);
}

void rcall_dis_rtl_query_parent_proj (Application*, int trg_id)
{
    dis_rtl_query_parent_proj (trg_id);
}

_vstrptr rcall_dis_rtl_get_item (Application*, int id, int idx)
{
    return vstrClone (dis_rtl_get_item (id, idx));
}

int rcall_dis_rtl_edit_copy(Application*, int, int)
{
    pset_stub("rcall_dis_rtl_edit_copy()");
    return 0;
}

int rcall_dis_rtl_edit_paste(Application*, int, int)
{
    pset_stub("rcall_dis_rtl_edit_paste()");
    return 0;
}

int rcall_dis_rtl_get_size (Application*, int id)
{
    return dis_rtl_get_size (id);
}

int rcall_dis_shutdown_server (Application*)
{
    pset_stub("rcall_dis_shutdown_server (Application*)");
    return 1;
}

_vstrptr rcall_server_eval (Application* app, _vstrptr, _vstrptr)
{
    pset_stub("rcall_server_eval (Application* app, _vstrptr, _vstrptr)");
    return (_vstrptr)0;
}

void rcall_server_eval_async (Application*, _vstrptr, _vstrptr)
{
    pset_stub("rcall_server_eval_async (Application*, _vstrptr, _vstrptr)");
}

int rcall_dis_connect_app (Application*, _vstrptr)
{
    pset_stub("call_dis_connect_app (Application*, _vstrptr)");
    return 0;
}

int rcall_dis_open_selection (Application*, int, int)
{
    pset_stub("rcall_dis_open_selection (Application*, int, int)");
    return 0;
}

int rcall_dis_layer_create (Application*, int, int, int)
{
    pset_stub ("rcall_dis_layer_create (Application*, int, int, int)");
    return 0;
}

void rcall_dis_set_layer (Application*, int)
{
    pset_stub("rcall_dis_set_layer (Application*, int)");
}

//////////////////////////

void call_progress(int)
{
    pset_stub("call_progress(int)");
}

extern "C" void vpkill(int, int)
{
}

const char * glob_to_regexp(const char*, int);

const unsigned char * glob_to_regexp(const unsigned char* p, int n)
{
    return (const unsigned char*)glob_to_regexp((const char*)p, n);
}

class tcpServer;

tcpServer * IntegratorServer;

extern "C" FILE* vpopen(const char* sh_command, const char* type);
typedef void notifier_function(void);

extern "C" FILE* vpopen_tmp(const char* sh_command, const char* type, const char* /* ofile */, notifier_function* /* handler */)
{
    return vpopen(sh_command, type);
}


int rcall_dis_viewer_selection_exists (Application* app)
{
  pset_stub("rcall_dis_viewer_selection_exists (Application* app)");
  return 0;
}

void rcall_dis_insert_selection_into_rtl (Application* app, int rtl_id)
{
  pset_stub("void rcall_dis_insert_selection_into_rtl (Application* app, int rtl_id)");	
}

void rcall_dis_open_selected_in_view (Application* app)
{
  pset_stub("rcall_dis_open_selected_in_view (Application* app)");
}

void register_gala_specific_commands(Tcl_Interp* interp)
{
}

int rcall_dis_is_module (Application* app, int, int)
{
  pset_stub("rcall_dis_is_module (Application* app, int, int)");
  return 0;
}

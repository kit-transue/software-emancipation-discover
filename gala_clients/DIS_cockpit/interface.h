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
void rcall_send_to_cockpit_unco (Application* app);
void send_to_cockpit_unco ();
void rcall_send_to_cockpit_ci (Application* app);
void send_to_cockpit_ci ();
void rcall_send_to_cockpit_co (Application* app);
void send_to_cockpit_co ();
void rcall_send_to_cockpit_update (Application* app);
void send_to_cockpit_update ();
void rcall_init_cockpit (Application* app);
void init_cockpit ();
void rcall_connect_to_cockpit (Application* app);
void connect_to_cockpit ();
void rcall_send_to_cockpit_delete (Application* app);
void send_to_cockpit_delete ();
static _vstrptr rcall_dis_servereval (Application* app, _vstrptr service, _vstrptr str);
static _vstrptr dis_servereval (_vstrptr service, _vstrptr str);
static async rcall_dis_servereval_async (Application* app, _vstrptr service, _vstrptr str);
static async dis_servereval_async (_vstrptr service, _vstrptr str);
_vstrptr rcall_dis_DIScockpit_eval (Application* app, _vstrptr str);
_vstrptr dis_DIScockpit_eval (_vstrptr str);
async rcall_dis_DIScockpit_eval_async (Application* app, _vstrptr str);
async dis_DIScockpit_eval_async (_vstrptr str);
void DIScockpitServerInit();

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
#ifndef _LayerServer_h
#define _LayerServer_h

#include <wchar.h>

#include <psetmem.h>

#include <viewerShell.h>
#include "gArray.h"

class steScreen;
class view;
class dialog;
class LayerServer;

#ifndef viewPtr
typedef view* viewPtr;
typedef steScreen* steScreenPtr;
#endif

struct viewTable {
    view* view_;
    steScreen* screen;
    int layer;
};

gArray (viewTable);

class LayerServer
{
  public:
    
    LayerServer (int id, int window, int type);
    LayerServer ();
    ~LayerServer ();

    int make_current();

    static LayerServer* find_layer_server (int id);
    static LayerServer* find_layer_server (view*);
    static LayerServer* find_layer_server (steScreen*);
    static steScreen* get_current_screen ();
    static steScreen* get_screen_of_view (view*);
    static view* get_current_view_of_screen (steScreen*);
    static void register_view (view*, steScreen*);
    static void unregister_view (view*);

    dialog* dlg();
    steScreen* screen();
    int id();
    int window();
    view* get_view();
  private:
    LayerServer* next;

    int id_;
    int type_;
    int window_;
    steScreen* screen_;
    dialog* dialog_;
    int uid;
  
    static int uniq_id;
    static LayerServer* layers;
    static gArrayOf (viewTable) view_table;
};

#endif





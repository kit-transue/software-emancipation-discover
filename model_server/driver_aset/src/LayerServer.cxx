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
#include "machdep.h"
#include <sys/stat.h>
#include <msg.h>
#include <LayerServer.h>
#include <objOper.h>
#include <dialog.h>
#include <steScreen.h>
#include <viewerShell.h>
#include <view_creation.h>

int LayerServer::uniq_id;
LayerServer* LayerServer::layers;
genArrOf(viewTable) LayerServer::view_table;

extern steScreen* gala_current_screen;
extern int gala_current_screen_layer;
extern dialog* gala_current_screen_dialog;

LayerServer::LayerServer (int id, int window, int type)
{
    Initialize(LayerServer::LayerServer (int id, int window, int type));
    this->id_ = id;
    this->next = layers;
    layers = this;
    this->type_ = type;
    this->window_ = window;
    this->screen_ = 0;
    this->dialog_ = 0;
    uid = uniq_id++;

    if (type == 1) {

        gala_current_screen_dialog = dialog_ = new dialog("gala_dialog");
        gala_current_screen = screen_ = db_new(steScreen,((steBuffer *)NULL, (long)window));
        if (!screen_->get_screen()) {
	    DBG {
		msg("Delete an empty screen.\n") << eom;
	    }
            obj_delete (screen_);
	    screen_ = 0;
        } 
        dialog_->create_window(0, (void*)window);
    }
}

LayerServer::~LayerServer()
{
    if (layers != this) {
	LayerServer *ls;
        for (ls=layers; ls->next == this; ls=ls->next);
        ls->next = ls->next->next;
    } else
        layers = layers->next;

    delete screen_;
}

steScreen* LayerServer::screen()
{
    return screen_;
}

int LayerServer::id()
{
     return id_;
}

int LayerServer::window()
{
    return window_;
}

dialog* LayerServer::dlg()
{
    return dialog_;
}

LayerServer* LayerServer::find_layer_server (int id)
{
    LayerServer* servers = layers; 

    while (servers) {
	if (servers->id_ == id) {
	    return servers;
        }
        servers = servers->next;
    }

    return 0;
}

steScreenPtr LayerServer::get_current_screen()
{
    steScreenPtr scr;

    if (gala_current_screen)
	return gala_current_screen;

    viewer *vwr = view_target_viewer();
    if (vwr)
	scr = vwr->get_screen();
    
    return scr;
}

steScreenPtr LayerServer::get_screen_of_view (viewPtr view)
{   
    for (int i=0; i < view_table.size(); i++)
    {
	if (view_table[i].view_ == view)
	    return view_table[i].screen;
    }
    return viewer::get_screen_of_view (view);
}

viewPtr LayerServer::get_current_view_of_screen(steScreen* screen)
{
   for (int i=0; i < view_table.size(); i++)
    {
	if (view_table[i].screen == screen)
	    return view_table[i].view_;
    }
    return viewer::get_current_view_of_screen (screen);
}

LayerServer* LayerServer::find_layer_server (viewPtr view)
{
    for (int i=0; i < view_table.size(); i++)
    {
	if (view_table[i].view_ == view)
	    return find_layer_server(view_table[i].layer);
    }
    return NULL;
}

LayerServer* LayerServer::find_layer_server (steScreen* screen) 
{
    LayerServer* servers = layers; 

    while (servers) {
	if (servers->screen_ == screen) {
	    return servers;
        }
        servers = servers->next;
    }

    return 0;
}

void LayerServer::unregister_view (viewPtr viewptr)
{
}

void LayerServer::register_view (viewPtr viewptr, steScreenPtr screen)
{
    for (int i=0; i < view_table.size(); i++)
    {
	if (view_table[i].view_ == viewptr) {
	    view_table[i].screen = screen;
	    return;
	}
    }

    viewTable* entry = view_table.grow();
    entry->layer = gala_current_screen_layer;
    entry->screen = screen;
    entry->view_ = viewptr;
    
    return;
}

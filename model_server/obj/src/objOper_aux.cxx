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
#include <msg.h>
#include <app.h>
//#include <ldr.h>

#include "objOperate.h"     
#include "RTL.h"     
#include "RTL_externs.h"     

//////////////////////////////////////////////////////////////////////
// Begin app

void proj_add_to_loaded(appPtr module);
void proj_remove_from_loaded(appPtr module);     

objArr* app::app_list;
objArr* app::new_app_list;

objArr* app::get_app_list()
{
  if (! app_list)
    app_list = new objArr;
  return app_list;
}

objArr* app::get_new_app_list()
{
  if (! new_app_list)
      new_app_list = new objArr;
  return new_app_list;
}

void app::append_to_list()
{
  Initialize(app::append_to_list);

  proj_add_to_loaded(this);

  static int inited = 0;

  if (! app_list)
    app_list = new objArr;
  app_list->insert_last(this);

  if (inited && app::new_app_list == 0)
	return;

  inited = 1;
  objArr* newlist = app::get_new_app_list();
  newlist->insert_last(this);

}

void app::db_after_restore(){}  // new db interface

app::app() : Hierarchical((char const *)0)
{
  Initialize(app::app);
    DBG{
       msg("args: ()\n") << eom;
    }

    append_to_list();
}

app::app(char const *title)
     : Hierarchical(title)
{
  Initialize(app::app);
    DBG{
       msg("args: title $1\n") << title << eom;
    }
    set_filename (title);
    append_to_list();
}

app::app(char const *file_name, appType type, smtLanguage language)
     : Hierarchical(file_name)
{
  Initialize(app::app);

    set_filename (file_name, type, language);
    append_to_list();
}

app::app(char const *title, char const *file_name, appType type, smtLanguage language)
     : Hierarchical(title)
{
    Initialize(app::app);

    set_filename (file_name, type, language);
    append_to_list();
}

app::app(char const *title, char const *file_name)
       : Hierarchical(title)
{
  Initialize(app::app);
    DBG{
       msg("args: title $1 fn $2\n") << title << eoarg << file_name << eom;
    }
    set_filename (file_name);
    append_to_list();
}

extern objSet sh_set;

app::~app()
{
    app_list->remove(this);
    new_app_list->remove(this);
    sh_set.remove (this);
    proj_remove_from_loaded(this);
}

void app::description(genString& str)
{
    str = "app";
}

void app::hilite_obj(objHiliter*, objHiliter*) {}

void app::focus_obj(objFocuser*, objFocuser*) {}

void app::assign_obj(objAssigner*, objAssigner*) {}

// End app
//////////////////////////////////////////////////////////////////////
// Begin commonTree

int commonTree::get_extype() const {
    return -1;
}

int commonTree::get_node_type() {
    return -1;
}

void commonTree::set_node_type(steNodeType) {
    msg("\nset_node_type - Bad call to non-steTree.\n") << eom;
}

commonTree::commonTree(const commonTree&oo) : objTree(oo),
    type(oo.type),
    extype(oo.extype),
    language(oo.language),
    raw_flag(oo.raw_flag),
    read_only(oo.read_only),
    smod_vision(oo.smod_vision),
    reserv(oo.reserv)
{}

// End commonTree
//////////////////////////////////////////////////////////////////////
// Begin Hierarchical

void Hierarchical::open(objTreePtr) {
    Initialize(Hierarchical::open);
    Error(NOT_IMPLEMENTED_YET);
}

void Hierarchical::hilite_obj(objHiliter*, objHiliter*) {}

void Hierarchical::focus_obj(objFocuser*, objFocuser*) {}

void Hierarchical::assign_obj(objAssigner*, objAssigner*) {}

objTreePtr Hierarchical::get_root() const {
    return objTreePtr(get_relation(tree_of_header, (Hierarchical *)this));
}

void Hierarchical::put_root(objTreePtr  the_root) {
    put_relation(tree_of_header, this, the_root);
}

void Hierarchical:: notify_softassoc_insert
                             (objInserter *, objInserter *) {}
    
void Hierarchical:: notify_softassoc_remove
                             (objRemover *, objRemover *) {}
    
void Hierarchical:: notify_softassoc_merge
                             (objMerger *, objMerger *) {}
    
void Hierarchical:: notify_softassoc_split
                             (objSplitter *, objSplitter *) {}

Hierarchical::Hierarchical(const Hierarchical& oo) : commonTree(oo),
   title(oo.title),
   filename(oo.filename),
   modified(oo.modified),
  imported(oo.imported),
  valid_filename(oo.valid_filename),
  src_modified(oo.src_modified),
  reserve_space(oo.reserve_space),
  date(oo.date),
  ph_name(oo.ph_name)
{}

// End Hierarchical
//////////////////////////////////////////////////////////////////////
// Begin appTree

//ldrPtr appTree::make_ref_ldr (){ return NULL;} // dead code. Do not use. -jef

appTree* appTree::app_to_ste() { return NULL;}
appTree * appTree::get_world () {return this;}

// End appTree
//////////////////////////////////////////////////////////////////////
// Begin random crap

objInserter:: objInserter( objOperType t, objTreePtr x, objTreePtr y, 
        void *d, void const *p, objInserter *prev) :
    type(t), src_obj(x), targ_obj(y), data(d), pt(p), prev_ins(prev) {}

int objInserter::apply(RelationalPtr) { return 0; }

void const *objInserter::get_data(){ return data; }
objTree* objInserter::get_target() { return targ_obj; }
objOperType objInserter::get_op_type() { return type; }

int objRemover::apply(RelationalPtr)  { return 0; }
int objHiliter::apply(RelationalPtr) const { return 0; }
int objFocuser::apply(RelationalPtr) const { return 0; }
int objAssigner::apply(RelationalPtr) const { return 0; }
int objMerger::apply(RelationalPtr)  { return 0; }
int objSplitter::apply(RelationalPtr) { return 0; }



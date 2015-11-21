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
// Implementation of scrapbook class
//------------------------------------------------
//
//
// Implement scrapbook functionality
//

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cstdio>
#include <cassert>
#endif /* ISO_CPP_HEADERS */


#include "msg.h"
#include "Relational.h"
#include "clipboard.h"
#include "objOperate.h"
#include "ste_create_region.h"
#include "steRegion.h"
#include "scrapbook.h"
#include "driver.h"
#include "steDocument.h"
#include "objArr.h"
#include "objSet.h"
#include "scrapbook-report.h"
#include "smt.h"

scrapbookPtr            scrapbook_instance;

init_relational (scrapbook, app);

init_relation (scrapbook_to_append_to, 1, NULL, scrapbook_to_append_from, 1, NULL);

scrapbook::scrapbook(char *name, CLIPBOARD_BEHAVIOR b): app (name)
{
    set_behavior (b);

    appTreePtr root = db_new (clipboard, ());

    this->put_root(root);
    this->set_type (App_SCRAPBOOK);

    if (name && *name)
	scrapbook_report_create (this);
}

scrapbook::scrapbook(CLIPBOARD_BEHAVIOR b)
{
    set_behavior (b);

    appTreePtr root = db_new (clipboard, ());

    this->put_root(root);
    this->set_type (App_SCRAPBOOK);
}

scrapbook::scrapbook (const scrapbook &)
{
    set_behavior (behavior);
}

scrapbook::~scrapbook()
{
    scrapbook_report_delete (this);
}

void scrapbook::remove_obj (objRemover * ro, objRemover * nr)
{
    Initialize (scrapbook::remove_obj);

    // get src object
    appTreePtr src_obj = checked_cast(appTree, ro->src_obj);

    if (src_obj) {
	// remove it
	src_obj->remove_from_tree ();
	obj_delete (src_obj);
	nr->src_obj = src_obj;
    } else
	nr->src_obj = NULL;

    Return
}

void scrapbook::insert_obj (objInserter *oi, objInserter *ni)
{
    Initialize (scrapbook::insert_obj);

    appTreePtr src_obj = checked_cast(appTree,oi->src_obj);
    appTreePtr targ_obj = checked_cast(appTree,oi->targ_obj);

    *ni = *oi;

    // insert appropriately
    if (targ_obj) {

	switch (oi->type) {
	  case FIRST:
	    targ_obj->put_first (src_obj);
	    break;

	  case AFTER:
	    targ_obj->put_after (src_obj);
	    break;

	  case REPLACE:
	    if (src_obj && src_obj != targ_obj) {
		targ_obj->put_after(src_obj);
		obj_remove(this, targ_obj);
	    }
	    break;

	  default:
	    ni->type = NULLOP;
	    break;
	}
    } else
	ni->type = NULLOP;

    Return
}

// inserts clipboard at appropriate place
void scrapbook::insert_clipboard (clipboardPtr clip_board)
{
    Initialize (scrapbook::insert_clipboard);

    appTreePtr root = checked_cast(appTree, this->get_root());
    if (behavior == unit_cut_paste)
	obj_insert ( this, FIRST, clip_board, root, NULL);
    else {
	// get to the last object
	appTreePtr last;
	for (last = checked_cast(appTree, root->get_first());
	     last && last->get_next();
	     last = checked_cast(appTree, last->get_next()));

	if (last)
	    obj_insert ( this, AFTER, clip_board, last, NULL);
	else
	    obj_insert ( this, FIRST, clip_board, root, NULL);
    }

    Return
}

// copy objects to the scrapbook as one object
void scrapbook::copy(objArr &objs_to_copy, bool shallow)
{
	msg("scrapbook::copy() called!\n") << eom;
	assert(0);
}

extern void ste_cut_region(steRegionPtr);
extern void ste_confused_regions(steRegionPtr, steRegionPtr);

void scrapbook::cut (objArr &objs_to_cut, bool shallow)
{
    Initialize(scrapbook::cut);

    if (objs_to_cut.size() > 0) {

	start_transaction();
	start_smt_editing();

	copy (objs_to_cut, shallow);

	Obj *el;
        steRegionPtr cur_reg = NULL;
        steRegionPtr next_reg = NULL;
        commonTreePtr nd;
        int smt_flag = 0;
// Reverce order
	ForEach (el, objs_to_cut) {
            nd = checked_cast(commonTree, el);
            smt_flag = ( is_smtTree(nd) ) ? 1 : 0;
            if ( smt_flag == 0 )
              driver_instance->remove_object ( nd );
            else if (cur_reg){
              next_reg = checked_cast(steRegion,get_relation(region_of_start, nd));
              if ( next_reg ){
                 if ( cur_reg == next_reg )
                    ste_cut_region(next_reg);
                 else
                    ste_confused_regions(cur_reg, next_reg);

                   next_reg = cur_reg = NULL;
                }     
            }else{
               cur_reg = checked_cast(steRegion,get_relation(region_of_end,nd));
               if ( cur_reg == NULL )
                  driver_instance->remove_object ( nd );
               else{
                  next_reg = checked_cast(steRegion,get_relation(region_of_start, nd));
                  if ( next_reg ){
                      if ( next_reg == cur_reg )
                          ste_cut_region(next_reg);
                      else
                          ste_confused_regions(cur_reg, next_reg);
                      next_reg = cur_reg = NULL;
                  }
               }
            }
	}
	
        if ( cur_reg || next_reg ){
           ste_confused_regions(cur_reg,next_reg);
           next_reg = cur_reg = NULL;
        }
	commit_smt_editing();
	end_transaction();
    }

    return;
}


void scrapbook::reference(objArr& objs_to_refer)
{
    Initialize(scrapbook::reference);
    if (objs_to_refer.size() > 0) {

	start_transaction();
        {
	    clipboardPtr clip_board = db_new (clipboard, ());

	    insert_clipboard (clip_board);

	    Obj *el;
	    ForEach (el, objs_to_refer)
	    {
		clip_board->reference (checked_cast(appTree, el));
	    }

	}
	end_transaction();
    }

    return;
}

void scrapbook::reference_graphics(objArr&/*objs_to_refer*/)
{
    Initialize(scrapbook::reference_graphics);
    return;	// References temporary turned off
}

clipboardPtr scrapbook::current_clipboard()
{
    Initialize (scrapbook::current_clipboard);

    clipboardPtr clip_board = NULL;

    // get root of scrapbook
    appTreePtr root = checked_cast(appTree, this->get_root());

    for (appTreePtr next = checked_cast(appTree, root->get_first());
	 next;
	 next = checked_cast(appTree, next->get_next())) {

	if (is_clipboard (next)) {
	    clip_board = checked_cast(clipboard, next);

	    if (behavior == unit_cut_paste)
		break;
	}
    }

    ReturnValue (clip_board);
}
    
objArr *scrapbook::objects_to_paste(appPtr dest_header)
{
    Initialize(scrapbook::objects_to_paste);

    objArr *r_val = NULL;

    clipboardPtr clip_board = current_clipboard();

    if (clip_board) {
	r_val = new objArr;

	clip_board->fill_objects (dest_header, *r_val);

	if (behavior == sequential_cut_paste) {
	    obj_remove (this, clip_board);
	}
    }

    ReturnValue (r_val);
}

void scrapbook::set_behavior (CLIPBOARD_BEHAVIOR b)
{
    Initialize (scrapbook::set_behavior);

    behavior = b;

    if (this != scrapbook_instance) {
	if (b == sequential_cut_paste) {
	    // set up relation between this scrapbook and clipboard
	    put_relation (scrapbook_to_append_from, this, scrapbook_instance);
	} else {
	    // remove relation
	    this->rem_rel (scrapbook_to_append_from);
	}
    }

    Return
}

void scrapbook::send_string(ostream& stream) const
{
    Initialize (scrapbook::send_string);

    appTreePtr root = checked_cast(appTree, this->get_root());

    if (root)
	root->send_string(stream);

    Return
}


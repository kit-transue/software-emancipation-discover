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
// ldrOODT.h.C
//------------------------------------------
// synopsis:
//
// Implementation of ldrOODT class
//------------------------------------------

#include "cLibraryFunctions.h"
#include "machdep.h"
#include <ldrOODT.h>
#include <smt.h>
#include <RTL.h>
#include <viewerShell.h>
#include <rebuildControl.h>
#include <errorBrowser.h>
#include <view.h>

init_relational(ldrOODT,ldrHierarchy);
int els_reparse_modules(symbolArr&,symbolArr&);

objArr ldrOODT::OODT_headers;

//------------------------------------------
// ldrOODT::ldrOODT()
//------------------------------------------

ldrOODT::ldrOODT() {
   Initialize(ldrOODT::ldrOODT);
   
   RTLPtr h = db_new(RTL, (NULL));
   pending = checked_cast(RTLNode, h->get_root());
   OODT_headers.insert_last(this);
}

//------------------------------------------
// ldrOODT::~ldrOODT()
//------------------------------------------

ldrOODT::~ldrOODT() {
   Initialize(ldrOODT::ldrOODT);
   
   RTLPtr h = checked_cast(RTL, pending->get_header());
   obj_delete(h);
   pending = NULL;
   OODT_headers.remove(this);
}

//------------------------------------------
// ldrOODT::add_pending(smtHeaderPtr)
//------------------------------------------

void ldrOODT::add_pending(smtHeaderPtr sh) {
   Initialize(ldrOODT::add_pending);
   
   pending->rtl_insert(sh, true);
   Obj* view_hdrs = get_relation(view_of_ldr, this);
   Obj* view_hdr;
   ForEach(view_hdr, *view_hdrs) {
      viewPtr vh = checked_cast(view, view_hdr);
      viewerShell* vs = viewerShell::viewerShell_of_view(vh);
      if (vs) {
         viewer* v = vs->viewer_of_view(vh);
         if (v) {
            v->enable_reparse(true);
         }
      }
   }
}

//------------------------------------------
// ldrOODT::has_pending()
//------------------------------------------

bool ldrOODT::has_pending() {
   Initialize(ldrOODT::has_pending);
   
   return pending->rtl_contents().size() > 0;
}

//------------------------------------------
// ldrOODT::refresh_pending()
//------------------------------------------

void ldrOODT::refresh_pending() {
   Initialize(ldrOODT::refresh_pending);
   
   symbolArr hdrs = pending->rtl_contents();
   Obj* hdr;
   ForEachS(hdr, hdrs) {
      smtHeaderPtr sh = checked_cast(smtHeader, hdr);
      if (!sh->arn) {
         pending->rtl_remove(sh);
      }
   }
   
   Obj* view_hdrs = get_relation(view_of_ldr, this);
   Obj* view_hdr;
   ForEach(view_hdr, *view_hdrs) {
      viewPtr vh = checked_cast(view, view_hdr);
      viewerShell* vs = viewerShell::viewerShell_of_view(vh);
      if (vs) {
         viewer* v = vs->viewer_of_view(vh);
         if (v) {
            v->enable_reparse(pending->rtl_contents().size() > 0);
         }
      }
   }
}

//------------------------------------------
// ldrOODT::parse_pending()
//------------------------------------------

void ldrOODT::parse_pending() {
  Initialize(ldrOODT::parse_pending);
   
  errorBrowserClear();
  start_transaction() {
    symbolArr files = pending->rtl_contents();
    symbolArr rest;
    els_reparse_modules(files,rest);
  } end_transaction();
  errorBrowser_show_errors();  // It will automatically pop up if there is an errors.
}

// ++++++ ------ ++++++++
objSet ldrOODT::modifiedObjectsByPropagation;
 
objSet* ldrOODT::get_modifiedOBP()
{
        return &modifiedObjectsByPropagation;
}
 
// to be inserted in all "rebuild_root"s (OODT rebuild_root is a pure virtual) 
int ldrOODT::doNotRefreshPropagated(ldrOODT *object)
{
        Initialize(ldrOODT::doNotRefreshPropagated);
        if (!getPROP_REFRESH_VIEW()) {
                objSet *mOBP = get_modifiedOBP();
                mOBP->insert(object);
                return 1;
        }
        return 0;
}

//  -------------------------------- 
//  refreshModifiedOBP()
//  rebuild_root() after propagation
//  -------------------------------- 
void ldrOODT::refreshModifiedOBP()
{
    Initialize(ldrOODT::refreshModifiedOBP);
    objSet  *mOBP = get_modifiedOBP();
    Obj     *ob;
    ldrOODT *rh;
    ForEach(ob, *mOBP) {
        rh = checked_cast(ldrOODT, ob);
        rh->rebuild_root();
    }
    mOBP->remove_all();
}

//------------------------------------------
// [static] ldrOODT::refresh_after_get
//------------------------------------------

void ldrOODT::refresh_after_get(const symbolArr& before, const symbolArr& after) {
   Initialize(ldrOODT::refresh_after_get);

   for (size_t i = 0; i < OODT_headers.size(); i++) {
      checked_cast(ldrOODT, OODT_headers[i])->do_refresh(before, after);
   }
}


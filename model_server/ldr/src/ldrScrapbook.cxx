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
// Implementation of ldrScrapbook class
//------------------------------------------------

#include "genError.h"
#include "ldrClipboard.h"
#include "ldrScrapbook.h"
#include "objOperate.h"

init_relational (ldrScrapbook, ldrHierarchy);

ldrScrapbook::ldrScrapbook()
{
}

ldrScrapbook::ldrScrapbook(scrapbookPtr scrap_book, clipboardPtr clip_board)
{
    Initialize(ldrScrapbook::ldrScrapbook);

    if (!scrap_book) return;

    if (!clip_board) clip_board = checked_cast(clipboard, scrap_book->get_root());

    if (!clip_board) return;

    ldrClipboardPtr root = extract_clipboard (scrap_book, clip_board);

    this->put_root(root);
}

ldrScrapbook::ldrScrapbook(const ldrScrapbook &oo)
{
}

ldrScrapbook::~ldrScrapbook()
{
}

void ldrScrapbook::build_selection(const ldrSelection& olds, OperPoint& app_point)
{
    Initialize (ldrScrapbook::build_selection);

    app_point.node = olds.ldr_symbol->get_appTree();

    app_point.type = FIRST;

    Return
}

ldrClipboardPtr ldrScrapbook::extract_clipboard (scrapbookPtr head,
						 clipboardPtr root)
{
    Initialize (ldrScrapbook::extract_clipboard);

    ldrClipboardPtr l_node = NULL;

    if (root && head) {

	l_node = db_new (ldrClipboard, (root));

	ldrClipboardPtr prev = NULL;
	for (clipboardPtr next = checked_cast(clipboard, root->get_first());
	     next; next = checked_cast(clipboard, next->get_next())) {

	    ldrClipboardPtr new_node = extract_clipboard (head, next);

	    if (new_node)
		if (prev)
		    prev->put_after(new_node);
		else
		    l_node->put_first (new_node);
	    

	    prev = new_node;
	}
    }

    ReturnValue (l_node);
}

void ldrScrapbook::insert_obj(objInserter *oi, objInserter *ni)
{
    Initialize (ldrScrapbook::insert_obj);

    appTreePtr targ_obj = checked_cast(appTree, oi->targ_obj);
    appTreePtr src_obj  = checked_cast(appTree, oi->src_obj);
    ldrClipboardPtr root = checked_cast(ldrClipboard,this->get_root());

    ni->type = NULLOP;

    if (targ_obj && is_clipboard(targ_obj) ) {

	ldrNodePtr targ_ldr = checked_cast(ldrNode, find_ldr (targ_obj));

	if (targ_ldr) {
	    
	    // Extract src ldr
	    ldrNodePtr src_ldr = NULL;
	    if (src_obj && is_clipboard(src_obj)) {
		src_ldr = checked_cast(ldrNode, find_ldr (src_obj));
		if (!src_ldr)
		    src_ldr = extract_clipboard
			( checked_cast(scrapbook, src_obj->get_header()),
			  checked_cast(clipboard, src_obj ));
	    }

	    switch (oi->type) {
	      case FIRST:
		if (targ_ldr == root) {
		    targ_ldr->put_first(src_ldr);
		    ni->type = oi->type;
		    ni->targ_obj = targ_ldr;
		    ni->src_obj = src_ldr;
		}

		break;

	      case AFTER:
		targ_ldr->put_after (src_ldr);
		ni->type = oi->type;
		ni->targ_obj = targ_ldr;
		ni->src_obj = src_ldr;
		break;

	      case REPLACE:
		if (targ_ldr != src_ldr) {
		    targ_ldr->put_after (src_ldr);
		    targ_ldr->remove_from_tree ();
		}
		ni->type = oi->type;
                ni->targ_obj = targ_ldr;
                ni->src_obj = src_ldr;
                break;

	      default: break;
	    }
	}
    }

    Return
}

void ldrScrapbook::remove_obj(objRemover *ro, objRemover *nr)
{
    Initialize (ldrScrapbook::remove_obj);

    appTreePtr src_obj = checked_cast(appTree, ro->src_obj);

    nr->src_obj = NULL;

    if (src_obj) {

	// find ldr for target obj
	ldrNodePtr src_ldr = checked_cast(ldrNode, find_ldr (src_obj));

	if (src_ldr) {
	    src_ldr->remove_from_tree();

	    nr->src_obj = src_ldr;
	}
    }

    Return
}

void ldrScrapbook::send_string(ostream& stream) const
{
    Initialize (ldrScrapbook::send_string);

    ldrClipboardPtr root = checked_cast(ldrClipboard, this->get_root());

    if (root)
	root->send_string(stream);

    Return
}

/*
   START-LOG-------------------------------------------

   $Log: ldrScrapbook.cxx  $
   Revision 1.5 1998/08/25 13:45:02EDT pero 
   removed or; port to HP
Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:58  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/

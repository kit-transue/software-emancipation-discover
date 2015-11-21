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
// Implementation of clipboard class
//
// Log: see the bottom
//
#include "genError.h"
#include "objOperate.h"
#include "deepCopier.h"
#include "reference.h"
#include "clipboard.h"
#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
#endif /* ISO_CPP_HEADERS */
#include "smt.h"
#include "steRegion.h"
#include "objArr_Int.h"
#include "steRefNode.h"
#include "steTextNode.h"
#include "ste_interface.h"


init_relational (clipboard, appTree);


clipboard::clipboard()
{
}

clipboard::clipboard(const clipboard &)
{
}

clipboard::~clipboard()
{
}

commonTreePtr clipboard::shallow_copy (commonTreePtr app_node)
{
    Initialize (clipboard::shallow_copy);

    steTextNodePtr r_val = NULL;

    if (app_node) {

	ostrstream ostr;
	app_node->send_string(ostr);
	ostr << ends;
	char * str = ostr.str();

	if (str) {
	    // create regular text node
	    r_val = db_new(steTextNode, (str));
	    r_val->set_node_type(steREG);

	    // free up string
	    delete str;
	}
    }

    ReturnValue (r_val);
}

commonTreePtr clipboard::deep_copy ( Obj *app_node)
  {
  Initialize (clipboard::deep_copy);

    commonTreePtr answer;
    Obj *r_val = NULL;
    objArr_Int reg_int;

    deepCopier cp;
    commonTree* st;
    if ( app_node->collectionp() ){  // smt region
       // Region mapping 
       Obj *bor_cur;
       int bor_ind = -1;
       int start_index = -1;
       steRegionPtr cur_reg;
       appTreePtr ap_n;
       ForEach(bor_cur, *app_node){
          bor_ind++;
          ap_n = checked_cast(appTree,bor_cur);
          cur_reg = checked_cast(steRegion,
                get_relation(region_of_start,ap_n));
          if ( cur_reg )
             start_index = bor_ind;

          if ( start_index == -1 )
             continue;

          cur_reg = checked_cast(steRegion,
             get_relation(region_of_end,ap_n));
          if ( cur_reg ){
             reg_int.insert_last( start_index );
             reg_int.insert_last( cur_reg->start_offset );
             reg_int.insert_last( bor_ind );
             reg_int.insert_last( cur_reg->end_offset );
             start_index = -1;
          }
       }
       Obj*el;
       int start_char = 10000000;
       smtTree* cur;
       ForEach(el, *app_node){
          cur = checked_cast(smtTree,el);
          if (cur->tbeg < start_char){
             st = cur;
             start_char = cur->tbeg;
          }
	  else if(cur->tbeg== start_char && st->subtree_of(cur) )
	  {
             st = cur;
             start_char = cur->tbeg;
	  }
       }
    } else
       st = checked_cast(commonTree, app_node);

    cp.put_starter_obj (st);
    cp.put_header_obj (this->get_header());
    r_val = obj_copy_deep (*app_node, &cp, &reg_int);

    answer = checked_cast(commonTree,r_val);

    if (is_smtTree (answer)){
      smtHeaderPtr hd = checked_cast(smtHeader, answer->get_header());
      // For SMT it's vary unreliable to copy subtree this way
      // At least "tidy_tree" needed
      hd->tidy_tree();

      answer = hd;
    }
    return answer;
}

commonTreePtr clipboard::deep_copy (commonTreePtr app_node)
  {
    return   deep_copy ( (Obj *)app_node );
  }

void clipboard::add_object (commonTreePtr app_node)
{
    Initialize (clipboard::add_object);

    if (app_node) {
	if (is_clipboard (app_node)) {

	    // insert contents of clipboard in reverse order
	    commonTreePtr next = checked_cast(commonTree, app_node->get_first());
	    if (next) {

		// get to the end
		while (next->get_next())
		    next = checked_cast(commonTree, next->get_next());

		// insert
		do {
		    this->put_first(next);
		} while (next = checked_cast(commonTree, next->get_prev()));
	    }

	    obj_delete (app_node);

	} else
	    this->put_first(app_node);
    }

    Return
}

void clipboard::copy (Obj *app_node, bool shallow)
{
  Initialize (clipboard::copy);
    commonTreePtr new_app_node = (shallow) 
       ? shallow_copy ( (checked_cast(commonTree,&app_node)) )
       : deep_copy (app_node);

    if (new_app_node)
       add_object (new_app_node);
}


void clipboard::copy (commonTreePtr app_node, bool shallow)
{
    copy( (Obj *)app_node, shallow );
}

void clipboard::fill_objects (appPtr dest_header, objArr &paste_objects)
{
    Initialize (clipboard::fill_objects);

    deepCopier cp;
    cp.put_header_obj (dest_header);
    start_transaction() {
	for (commonTreePtr node = checked_cast(commonTree, this->get_first());
	     node;
	     node = checked_cast(commonTree, node->get_next())) {

	    if (is_Hierarchical(node))
              cp.put_starter_obj(checked_cast(Hierarchical,node)->get_root());
	    else
	      cp.put_starter_obj (node);

	    commonTreePtr orig_obj = checked_cast(commonTree, cp.get_starter_obj());
	    commonTreePtr new_obj =
                checked_cast(commonTree, obj_copy_deep (*((Obj *)orig_obj), &cp));
	    if (new_obj)
		paste_objects.insert_last (new_obj);
	}
    } end_transaction();

    Return
}

void clipboard::reference (commonTreePtr obj)
{
    Initialize (clipboard::reference);

    appTreePtr ref;

    ref = make_reference(checked_cast(appTree, obj));

    if (ref)
	add_object (ref);

    reference_put_root(ref, checked_cast(appTree, obj));

    return;
}

void clipboard::send_string(ostream& stream) const
{
    Initialize (clipboard::send_string);

    for (commonTreePtr node = checked_cast(commonTree, this->get_first());
	 node;
	 node = checked_cast(commonTree, node->get_next())) {
	if (is_Hierarchical (node)) {
	    appTreePtr root = checked_cast(appTree,node->get_root());
	    if (root) root->send_string(stream);
	} else
	    node->send_string (stream);
    }
}

appPtr clipboard::create_deep_copy (objArr& nodes, objArr_Int& regions, int do_regions)
{
    Initialize(clipboard::create_deep_copy);
    smtHeader *h = NULL;
    if ( !(nodes.size() && regions.size()) )
	return NULL;
    
    deepCopier cp;
    cp.put_starter_obj (checked_cast(smtTree,nodes[0]));
    cp.put_header_obj (NULL);
    
    // It is assumed that we are getting smt tree where first level
    // children correspond to "regions" from regions Int array
    smtTree *rt = checked_cast(smtTree,obj_copy_deep (nodes, &cp, &regions, do_regions));
    if (rt)
	h = checked_cast(smtHeader,rt->get_header());
    
    return h;
}

/*
   START-LOG-------------------------------------------

   $Log: clipboard.cxx  $
   Revision 1.6 2000/07/10 23:02:35EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.14  1994/05/05  20:04:37  boris
Bug track: 6794
Fixed "slow merge" problem, and #6977 asyncronous save

Revision 1.2.1.13  1993/05/26  12:14:10  aharlap
fixed some copy/paste problems

Revision 1.2.1.12  1993/01/11  17:51:07  jon
References

Revision 1.2.1.11  1993/01/05  15:36:24  jon
Delayed applying the root_of_reference relation to the node
added to the clipboard until after the node is actually on
the clipboard (at which point it actually has a header).

Revision 1.2.1.10  1992/12/17  14:27:02  jon
Fixed up code relating to references.

Revision 1.2.1.9  1992/12/02  23:04:31  smit
fix cut/paste problem.

Revision 1.2.1.8  1992/12/01  19:40:04  boris
Changed copy first argument from Relational& to Obj& to fix CUT

Revision 1.2.1.7  1992/11/24  21:52:00  wmm
Fix clipboard init_relational (said was derived from Relational,
actually derived from appTree).

Revision 1.2.1.6  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.5  1992/10/29  20:25:43  boris
Added smt region notion

Revision 1.2.1.4  92/10/16  14:13:20  boris
Added region mapping

Revision 1.2.1.3  92/10/15  14:42:58  mg
cut/paste modification

Revision 1.2.1.2  92/10/09  18:42:33  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/

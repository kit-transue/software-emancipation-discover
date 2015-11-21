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
// objOp.C
//
#include <transaction.h>
#include <objRelation.h>
#include <app.h>
#include <objArr.h>

#include <objOperate.h>
RelClass(ddElement);
RelClass(projModule);

// Values of obj_id field:
//   > 0 - Normal value (by default - 1 - assigned by Obj constructor)
//   0 -   immediatly before "delete". Assigned in obj_real_delete before
//	   "delete" statement
//   -1 -  after destructor of "Relational"
//   -2 -  after destructor of "Obj"
//   -3 -  after dispose
//   -4 -  inside obj_real_delete - inserted to "pend" array
//   -5 -  inside obj_real_delete - inserted to "proc" array

void obj_delete(Obj *node, objDeleter *od)
{
 Initialize(obj_delete);
  Assert(node);    
  obj_delete(*(Relational *)node, od);
 return;
}

// old style interface
void obj_delete(Relational &root, objDeleter *)
{
    Initialize(obj_delete);
    if ( !(root.get_id() > 0 || root.get_id() == -3) )
	return;

    start_transaction(){

      // boris: tried to prevent removing of a ddElement from tree 
      // and to fix crash in direct ddElement deleting
	if (is_ddElement(&root) || is_projModule(&root)) {
	  obj_dispose( &root );
	  return;
	}

	appPtr apl = NULL;
	appTree *atptr = NULL;

	if (is_appTree(&root)) {
	    atptr = appTreePtr(&root);
	    apl = atptr->get_header();
	}

	if(apl) 
	    obj_remove(apl, atptr);
	else 
	    obj_dispose(&root);

    } end_transaction();

    return;
}

static void insert_if_not_there(Obj * objs, objArr& arr){
  static char * _my_name = "insert_if_not_there";

     Obj * staff;
     ForEach(staff, *objs){
        int id = staff->get_id();
        if(id==-4 || id==-5)  continue;
	Assert(id == -3 || id > 0);	// Must be existent or disposed
        arr.insert_last(staff);
        staff->put_id(-4);
     }
  return;
}


// notify along relations from node wthich do not have type (D,M) flag
void obj_notify(int flag, Relational *node)
{
      relArr& tmp_arr = node->get_arr();
      int sz = tmp_arr.size();
      for(int ii=0; ii<sz; ++ii){
         Relation * rel = tmp_arr[ii];
         RelType  * rel_type = rel->get_rel_type();
         if (rel_type->get_flags() & flag)       // skip flag links
             continue;
         else if((rel_type == tree_of_header) || (rel_type == header_of_tree))
             continue;

         Obj * notif_set = rel->get_members();
         Obj * curr;
         ForEach(curr, *notif_set) {
	   if (curr->get_id() >= 0)
	     ((Relational *)curr)->notify(flag, node, /*od*/0, rel_type);
	 }
      }
}

// obj_real_delete function based on new propagation mechanism


extern objArr * pend_ptr;
extern objArr * proc_ptr;

void obj_delete_or_unload(Obj *root, int delete_flag)
{
 Initialize(obj_delete_or_unload);

  objArr pend, proc;

  pend_ptr = &pend;
  proc_ptr = &proc;

// Collect set of objects to delete

  insert_if_not_there(root, pend);

  Relational *node;
  objArr prop;

  int del_id = delete_flag ? 0 : -1;
  int flag   = delete_flag ? relationMode::D : relationMode::UD;
  while(node = (Relational*) pend.pop()) {

    node->propagate(flag, &prop);                 // get propagation set
    insert_if_not_there(&prop, pend);  

    prop.removeAll();

// propagate along pointers

    if(node->descr()){
      for(RelType *rt=node->descr()->rel; rt; rt=rt->next){
         if(rt->get_flags() & relationMode::D){
             Obj * els = node->get_raw_ptr(rt);
             if(els) insert_if_not_there(els, pend);
         }
      }
    }

    proc.insert_last(node);

    node->put_id(-5);
  }

//Delete objects in proc:

  int no_proc = proc.size();
  int iii;
  int no_noh = 0, no_h = 0;
  // first remove non-headers
  for(iii=no_proc-1; iii>=0; --iii) {
      Relational * node2 = (Relational *) proc[iii];

      obj_notify(relationMode::D, node2);
      if(delete_flag){
          node2->call_rel_hooks(-1);
      }
      // deletes from obj_to_dispose if root is not obj_to_dispose
      obj_remove_from_dispose( root, node2 );

      if(! is_app(node2)){
        node2->put_id(del_id);
        delete node2;		// Obj_id will be -2 after all destructors
        proc[iii] = NULL;
        ++no_noh;
      } else {
        ++no_h;
      }
  }
  // now remove gheaders
  for(iii=no_proc-1; iii>=0; --iii) {
      Relational * node2 = (Relational *) proc[iii];
      if(node2){
        node2->put_id(del_id);
        delete node2;
      }
  }
  pend_ptr = NULL;
  proc_ptr = NULL;
}


void obj_real_delete(Obj *root)
{
 Initialize(obj_real_delete);
 obj_delete_or_unload(root, 1);
}

void dd_delete_dd_or_xref_node_views(Obj*);

void obj_unload(Obj *root)
{
 Initialize(obj_unload);
 dd_delete_dd_or_xref_node_views(root);
 Relational *rr = (Relational *)root;
#if 0 // I'm unsure of this:
 if (is_app(rr))
     gpi_close_out((app *)rr);
#endif
 obj_delete_or_unload(root, 0);
}
 
/*
   START-LOG-------------------------------------------

   $Log: objOp.cxx  $
   Revision 1.7 2002/03/04 17:24:21EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.17  1994/03/05  14:42:40  trung
Bug track: 0
fixes for prop to subsys, notify rtl, restoring subsys

Revision 1.2.1.16  1993/12/03  17:09:34  himanshu
Bug track: new code
added call to gpi_close_out  for sending a msg to an external app for closing a
file, just unloaded

Revision 1.2.1.15  1993/08/22  02:20:50  aharlap
fixed bug #4099

Revision 1.2.1.14  1993/07/08  18:12:44  mg
first delete non-headers

Revision 1.2.1.13  1993/05/19  22:05:48  boris
Changed Assert at the beginning of obj_delete to return

Revision 1.2.1.12  1993/04/02  23:26:56  boris
Introduced special case for ddElement obj_delete() that puts it directly
in obj_to_dispose array without removing from tree;

Revision 1.2.1.11  1993/04/01  17:04:20  aharlap
part of bug # 3071

Revision 1.2.1.10  1993/03/31  23:55:24  boris
fixed bug #3054 in propagate

Revision 1.2.1.9  1993/03/29  19:28:31  mg
#define U     32

Revision 1.2.1.8  1993/02/24  01:03:30  trung
change call to call_rem_hooks to call_hooks

Revision 1.2.1.7  1993/02/22  14:32:20  mg
obj_delete_or_unload

Revision 1.2.1.6  1993/01/13  22:04:44  boris
Added preliminary remove relation hook call before real node delete

Revision 1.2.1.5  1992/11/24  20:18:38  smit
Fix bad check for casting.

Revision 1.2.1.4  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.3  1992/10/14  15:40:40  boris
Fixes bug with not removing trees of a obj_deleted header

Revision 1.2.1.2  92/10/09  18:55:22  boris
Fix comments

   Revision 1.2.1.1  92/10/07  20:50:52  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:50:51  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:19  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:56  smit
   Initial revision

   Revision 1.19  92/09/23  18:16:14  mg
   *** empty log message ***
   
   Revision 1.18  92/09/10  18:15:06  mg
   propagate args
   
   Revision 1.17  92/09/01  18:46:54  mg
   *** empty log message ***
   
   Revision 1.16  92/08/11  11:44:35  mg
   notify
   
   Revision 1.15  92/07/09  19:19:14  mfurman
   SMT split/merge support
   
   Revision 1.14  92/04/03  15:24:25  mfurman
   *** empty log message ***
   
   Revision 1.13  92/04/03  14:46:56  mfurman
   Hook for illegal delete
   
   Revision 1.12  92/02/19  17:37:35  mg
   *** empty log message ***
   
   Revision 1.11  92/02/19  14:24:22  mg
   put_id(0) before delete
  
   Revision 1.10  92/02/14  13:20:42  mg
   delete_immediatly
  
   Revision 1.9  92/02/14  11:38:47  mg
   use arrays instead of sets in obj_real_delete
  
   Revision 1.8  92/01/09  17:45:56  mfurman
   obj_real_delete added
  
   Revision 1.7  92/01/02  11:07:59  mg
   *** empty log message ***
  
   Revision 1.6  91/12/26  15:53:38  mg
   clean up
  
   Revision 1.5  91/10/21  20:29:59  sergey
   Change obj_delete definition.
  
   Revision 1.4  91/10/09  09:30:13  sergey
   *** empty log message ***
  
   Revision 1.2  91/10/08  11:52:37  sergey
   Added notify; minor cleanup.
  
   History:   09/24/91     M.Furman         Initial coding.
  
   Log : see the bottom of file

   END-LOG---------------------------------------------

*/

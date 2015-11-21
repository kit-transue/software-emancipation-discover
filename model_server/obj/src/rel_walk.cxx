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
// REL_WALK.C
//
// synopsis : Walk along the relation tree.
//
//
// Description: Walks thru all nodes of the relational tree starting from the 
//              root. For each relation type finds all member objects and tests
//              if they satisfy given conditions (accept() ). The objects that
//              passed the test are put into the set with their walk_ptr.
//              This call optionally can pass the data along the tree and
//              change it according to the current relation type in
//              trnsf_data(). Actual node modification is done either in
//              accept() call or outside the rel_walk by apply() call on
//              all nodes extracted into the set.
//

#include "objCollection.h"
#include "objRelation.h"
#include "objPropagate.h"
#include "objOper.h"
#include "smtScdTypes.h"
static void  collect_rel(objRelWalkSet *, int, objSet &, objSet &, 
                         int level = ALL_LEVELS);

 void rel_walk(RelationalPtr root, const int &flag, objPropagator *walk_ptr,
                   objSet &propagate_set, int level, int )
 {

    Obj           *el;
    objSet        pend_set;
    objRelWalkSet *root_el;


//-------- Executable Code ---------------------------------------------------

    Initialize(rel_walk);


    root_el = db_new (objRelWalkSet, (walk_ptr, root));
    pend_set.insert(root_el);

    while( pend_set.size() != 0 )
    {
	ForEach(el, pend_set) 
	    { if (el) break; }   // get the next node of the set.
                                 // the "if(el) break; break;" eliminates a compiler
                                 // warning "Exit before end of for" on sun5.

       pend_set.remove(el);               // remove it from pending set 
       if ( propagate_set.includes( checked_cast(objRelWalkSet,el)) )
            continue;                     // already processed

//     get nodes related to current one (one relation tree level) :
//       -  all accept tests are done inside of obj_rel_walk 
//       -  related nodes are put into pend_set in accepted
//       -  current root node is put into propagate_set if accepted

       collect_rel(checked_cast(objRelWalkSet,el), flag, pend_set, propagate_set, level);
    }
 }





//  collect_rel  - collects nodes directly related to given root node
//                 using test rules specified in accept() calls;
//                 put accepted nodes into pend_set and the root node
//                 into propagate_set for processing by apply() call.


static void  collect_rel( objRelWalkSet *curr_el, int flag, objSet &pend_set, 
                          objSet &propagate_set, int level)
{

    Obj*         members;
    Obj*         notif_set;
    objRelWalkSet  *curr_set_el;
    objPropagator  *walk_ptr; 
    objPropagator  *curr_walk_ptr, *new_walk_ptr;
    RelationalPtr  root;


//-------- Executable Code ---------------------------------------------------

    Initialize(collect_rel);

    walk_ptr = curr_el->walk_ptr;
    root     = curr_el->targ_obj;

//  check if node really should be modified; use walker accept() 

    walk_ptr = walk_ptr->accept(root, level);
    if (walk_ptr == NULL)
          Return;

    propagate_set.insert(curr_el); 

//  collect direct relations
 
    relArr& tmp_arr   = root->get_arr();
    int sz = tmp_arr.size();
    for(int ii=0; ii<sz; ++ii){
        Relation * curr_rel = tmp_arr[ii];
        if ( curr_rel == NULL )
             continue;

        RelType * rel_type = curr_rel->get_rel_type();

//      test the entire relation type (flag, etc.)

        curr_walk_ptr = rel_type->accept(walk_ptr, flag);

        if (curr_walk_ptr == NULL){

//          relation is rejected, but the nodes stil should 
//          be notified about delete/modify update

            if ( (walk_ptr->get_notify_flag()==AFTER_ACCEPT)
                                                  && (flag == relationMode::D || flag == relationMode::M) ){
                 notif_set = curr_rel->get_members(); 
                 Obj * curr;
                 ForEach(curr, *notif_set)
                    ((Relational *)curr)->notify(flag, root, 
                                        walk_ptr, rel_type);
            }
            continue;             
        }
        members = curr_rel->get_members(); 

//      save only the objects that are not collected already

//        members -= propagate_set;

//      perform member class accept test

        Obj *el1;
        ForEach(el1, *members){
	    objRelWalkSet tmp_set_el (curr_walk_ptr, (RelationalPtr )el1);
            if(propagate_set.includes(&tmp_set_el)){
		continue;
            }
            new_walk_ptr = 
               ((Relational *)el1)->accept(curr_walk_ptr, rel_type);
            if (new_walk_ptr == NULL)
                continue;

            curr_set_el = db_new (objRelWalkSet, 
                               (new_walk_ptr, (RelationalPtr )el1));
            pend_set.insert(curr_set_el);
        }        
    }
    Return;
 }

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
#include "transaction.h"

#include "objRelation.h"
#include "objOper.h"
#include <objArr.h>

#include "SoftAssoc.h"  // To notify when delete.
#include <psetmem.h>
#include <objOperate.h>
#include <symbolPtr.h>
#include <symbolArr.h>

RelClass(ddElement);
RelClass(projModule);

struct memory_chunk{
    char   *memory;
    struct memory_chunk *next;
};

struct memory_info{
    struct memory_chunk *last_chunk;
    char   *memory_ptr;
};

struct memory_chunk memory_list;
struct memory_chunk *last_chunk;

void db_clear_incremental ();

static  int POOL_SIZE = 100000;

static char* initial_transaction_memory_pool = (char *)psetmalloc(POOL_SIZE);
static char* transaction_memory_pool         = initial_transaction_memory_pool;
static char* transaction_memory_ptr          = initial_transaction_memory_pool;
static char* transaction_memory_limit        = initial_transaction_memory_pool + POOL_SIZE-1;

const int MAX_TRANSAC_LEVEL = 100;
static struct memory_info memory_arr[MAX_TRANSAC_LEVEL];

int    transac_level = 0;   // each nested call adds 1 to it
static objArr obj_to_dispose;   // set of objects to delete

struct memory_chunk *allocate_memory_chunk(struct memory_chunk *current)
{
    Initialize(allocate_memory_chunk);
    
    current->next = (struct memory_chunk *)malloc(sizeof(struct memory_chunk));
    Assert(current->next);
    current->next->memory = (char *)psetmalloc(POOL_SIZE);
    current->next->next   = NULL;
    return current->next;
}

void add_new_memory_chunk(void)
{
    last_chunk               = allocate_memory_chunk(last_chunk);
    transaction_memory_pool  = last_chunk->memory;
    transaction_memory_ptr   = transaction_memory_pool;
    transaction_memory_limit = transaction_memory_pool + POOL_SIZE-1;
}

void dispose_memory_chunks(struct memory_info *m)
{
    transaction_memory_pool  = m->last_chunk->memory;
    transaction_memory_ptr   = m->memory_ptr;
    transaction_memory_limit = transaction_memory_pool + POOL_SIZE-1;
    // Now dispose additional memory chunks (if was allocated)
    struct memory_chunk *temp = m->last_chunk->next;
    while(temp){
	psetfree(temp->memory);
	temp = temp->next;
    }
    m->last_chunk->next = NULL;
    last_chunk          = m->last_chunk;
}

void obj_transaction_start(){
   Initialize(obj_transaction_start);
        if (transac_level == 0){
            Assert(obj_to_dispose.empty());
            transaction_memory_ptr   = initial_transaction_memory_pool;
	    transaction_memory_limit = initial_transaction_memory_pool + POOL_SIZE-1;
	    memory_list.memory       = initial_transaction_memory_pool;
	    memory_list.next         = NULL;
	    last_chunk               = &memory_list;
        } else {
            Assert(transac_level < MAX_TRANSAC_LEVEL);
        }
        memory_arr[transac_level].last_chunk = last_chunk;
        memory_arr[transac_level].memory_ptr = transaction_memory_ptr;
        transac_level ++;
   return;
}

void xref_notifier_apply();

void propagate_modified_headers();

void obj_transaction_end(){
    Initialize(obj_transaction_end);

   if (transac_level > 1){
          transac_level --;
	  dispose_memory_chunks(&memory_arr[transac_level]);
          return;
   }
             
    if (obj_to_dispose.empty()) {
       xref_notifier_apply();
       propagate_modified_headers();
    }

   while(! obj_to_dispose.empty() ){

     Obj *el;
     objArr disp_arr;

//   do not delete non-header-less obj

     ForEach(el, obj_to_dispose){
        if(is_commonTree((Relational*)el)) {
           commonTree* node = checked_cast(commonTree,el);
           if(! is_Hierarchical(node)){
              if( node->get_header() && !(is_ddElement(node) || is_projModule(node)) )
                 continue;
           }
        }
        disp_arr.insert_last(el);
     }
     obj_to_dispose.removeAll();
     if (! disp_arr.empty() ) obj_real_delete(&disp_arr);
     xref_notifier_apply();	// notify after deletion
     propagate_modified_headers();
  }
  db_clear_incremental ();
  transac_level --;

} 

void obj_transaction_abort(){
      
       if (transac_level == 0) return;
       transac_level = 1;
       obj_transaction_end();
}

void  obj_dispose(Obj *ob){
    char * _my_name = "obj_dispose";
        int id = ob->get_id();
        Assert(id != -1);    // already deleted;
        Assert(id != -4);    // already pending;
        Assert(id != -5);    // already processed by real_delete;
        if(id==-3) return;
        ob->put_id(-3);
        obj_to_dispose.insert_last(ob);
        if (is_appTree((Relational *) ob)) notify_softassoc(checked_cast(appTree,ob), relationMode::D);
}

objArr * pend_ptr =NULL;
objArr * proc_ptr =NULL;

void  obj_remove_from_dispose(Obj *root, Obj *ob){
    if ( root != &obj_to_dispose )
         obj_to_dispose.remove(ob);
}

void *  transactional::operator new(size_t size)
{
   char* _my_name = "transactional::operator new";

   Assert(transac_level > 0);

   char* ob      = transaction_memory_ptr;
   int rem       =  size % sizeof(int);
   if(rem) size += (sizeof(int) - rem);
   transaction_memory_ptr += size;
   if(transaction_memory_ptr > transaction_memory_limit){
       add_new_memory_chunk();
       ob  = transaction_memory_ptr;
       rem =  size % sizeof(int);
       if(rem) 
	   size += (sizeof(int) - rem);
       transaction_memory_ptr += size;
   }
   return ob;
}
/*
   START-LOG-------------------------------------------

   $Log: transaction.cxx  $
   Revision 1.8 2002/03/04 17:24:37EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.13  1994/03/28  22:35:21  azaparov
Bug track: 6688
Fixed memory pool limitations

Revision 1.2.1.12  1994/01/13  03:06:29  builder
*** empty log message ***

Revision 1.2.1.11  1994/01/13  01:09:55  kws
Use psetmalloc

Revision 1.2.1.10  1993/12/22  14:03:01  aharlap
added db_incremental

Revision 1.2.1.9  1993/12/21  22:49:22  wmm
Bug track: 5672
Fix bug 5672 by: 1) collecting all propagations of modified files in an RTL in Hierarchical and
doing the propagation at end_transaction time; 2) removing the relation between the ddHeader
and OODT view symbolTrees.

Revision 1.2.1.8  1993/07/07  21:48:21  mg
int transac_level = 0;

Revision 1.2.1.7  1993/04/14  11:03:39  wmm
Performance improvement: do xref_notifier_apply() after the dispose,
in case the dispose results in xref modifications; also, fix bug
introduced in last bug fix that causes some notifications to be
skipped (when there are pending notifications but no pending
disposals).

Revision 1.2.1.6  1993/04/09  13:59:47  wmm
*Part* of bug fix for 3108 (do not close yet): move xref_notifier_apply()
into while (!obj_to_dispose.empty()) loop.

Revision 1.2.1.5  1993/04/02  23:26:56  boris
Fixed double delete while obj_unload();
Added case in obj_dispose() for ddElements that to delete them
even if they have header

Revision 1.2.1.4  1992/12/28  16:37:19  mg
xref_notifier_apply.

Revision 1.2.1.3  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:55:40  boris
Fix comments


   END-LOG---------------------------------------------

*/

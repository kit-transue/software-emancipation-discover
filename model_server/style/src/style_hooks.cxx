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
//                  style_hooks.C
//	Application-layer style/category functions
//
#include <cLibraryFunctions.h>
#include <msg.h>
#include <objOper.h>
#include <ste_interface.h>
#include <style_hooks.h>
#include <ddict.h>
#include <steSlot.h>
#include <ste_style_handler.h>
#include <steHeader.h>

//
// 		Overview
//
//    The links are, at present, just relations. There's no underlying
// link object (except to the extent that there's an underlying relation
// object) and the relational mechanism takes care of all the hard work.
// Links may go from any apptree to any other apptree, though linking to
// anything other than steText or smtTree is probably inadvisable. 
// 

//
// initialize relations - a link goes from node to ddcat or ddsty.
//

init_relation(ddcats_of_node,MANY,NULL,nodes_of_ddcat,MANY,NULL);
init_relation(ddsty_of_node,1,relationMode::C,nodes_of_ddsty,MANY,NULL);

//
// Function to assign / deassign style/category
//

static int style_modify(appTree *nd, char *nm, boolean categ, int op) {
 Initialize(style_modify);
  if ( !nd )
      return -1;

  if( categ ){
     if ( op >= 0 )          // 0 - restore, 1- assign
        ste_interface_category_assign(nd, nm);
     else
        ste_interface_category_deassign(nd, nm);
  }else{
    if ( op >= 0 )
        ste_interface_style_assign(nd, nm);
     else
        ste_interface_style_deassign(nd, nm);
  }
  return 0;
}

//
// The following are "hook" functions attached to the ddsty_of_node and
// ddcats_of_node relations.
// Its job is to assign and deassign styles and categories when the relation
// is added and removed. 
//

static void style_hook(int op, Relational *tg, Relational *sc, RelType *)
{
  Initialize(style_hook);
#ifdef DEBUG
  OSapi_printf("style hook: diagnostic mode\n");
  OSapi_printf("Source%s: %s\n", is_rmtNode(sc) ? "[rmt]" : "", sc->get_name());
  OSapi_printf("Target%s: %s\n", is_rmtNode(tg) ? "[rmt]" : "", tg->get_name());
  OSapi_printf("Operation: %s\n", op>0 ? "add" : "remove");
#endif
  char * nm = 0;
  appTreePtr nd = 0;

    if (is_ddElement(tg)){
      nm = tg->get_name();
      nd = checked_cast(appTree,sc);
    }

    if ( is_ddElement(sc) ){
      nm = sc->get_name();
      nd = checked_cast(appTree,tg);
    }

    if ( style_modify(nd, nm, 0, op) )
	msg("Warning: style could not be installed") << eom;
#ifdef DEBUG
      else OSapi_printf("Style hook o'key.\n");
#endif
}

static void style_cat_hook(int op, Relational *tg, Relational *sc, RelType *)
{
  Initialize(style_cat_hook);
#ifdef DEBUG
  OSapi_printf("style_cat_hook: diagnostic mode\n");
  OSapi_printf("Source%s: %s\n", is_rmtNode(sc) ? "[rmt]" : "", sc->get_name());
  OSapi_printf("Target%s: %s\n", is_rmtNode(tg) ? "[rmt]" : "", tg->get_name());
  OSapi_printf("Operation: %s\n", op>0 ? "add" : "remove");
#endif
  char * nm = 0;
  appTreePtr nd = 0;

    if (is_ddElement(tg)){
      nm = tg->get_name();
      nd = checked_cast(appTree,sc);
    }

    if ( is_ddElement(sc) ){
      nm = sc->get_name();
      nd = checked_cast(appTree,tg);
    }

    if ( style_modify(nd, nm, 1, op) )
	msg("Warning: category could not be installed") << eom;
#ifdef DEBUG
      else OSapi_printf("Category hook o'key.\n");
#endif
}

ddElementPtr get_dd( appTreePtr an, char *nm, ddKind my_k ){
 Initialize(get_dd);
    ddElementPtr de = 0;
    RelType *rt = ( my_k == DD_CATEGORY ) ? ddcats_of_node : ddsty_of_node;

    Obj *slot_set, *el1;
    ddElementPtr cur_de;
    slot_set = get_relation(rt, an);
    if ( slot_set && slot_set->collectionp()){
        ForEach( el1, *slot_set ){
            cur_de = checked_cast(ddElement,el1);
            if ( strcmp(nm,cur_de->get_name()) == 0 ){
               de = cur_de;
               break;
            }
        }
    }else
        de = checked_cast(ddElement,slot_set);

    return de;   
}

void style_manual_put( appTreePtr an, char *nm , boolean categ )
{
  Initialize(style_manual_put);
     ddKind my_k = ( categ ) ? DD_CATEGORY : DD_STYLE;
     if (  !nm || !an )
         return;

     if ( get_dd( an, nm, my_k ) ){
        steHeader(Message)
        << "style_manual_put: already assigned."
        << steEnd;
        return;
     }

     RelType *rt = ( categ ) ? ddcats_of_node : ddsty_of_node ;
     ddElementPtr de = db_new(ddElement,(nm, an, 0, my_k, rt));
}

void style_manual_remove( appTreePtr an, char *nm , boolean categ )
{
  Initialize(style_manual_remove);
     ddKind my_k = ( categ ) ? DD_CATEGORY : DD_STYLE;
     RelType *rt = ( categ ) ? ddcats_of_node : ddsty_of_node;

     if ( !an )
         return;

   
     if ( nm == 0 ){
        Obj *slot_set, *el1;
        slot_set = get_relation(rt, an);

        if ( slot_set == 0 )
           return;

        Relational *rel;
        if ( slot_set->collectionp() ){
            ForEach( el1, *slot_set ){
               rel = (Relational *)el1;
               rem_relation(rt, an, rel);
            }
        }else{
            rel = (Relational *)slot_set;
            rem_relation(rt, an, rel);
        }
        return;
     }

     ddElementPtr de = get_dd( an, nm, my_k );
     if ( de == NULL ){
        steHeader(Message)
        << "style_manual_remove: not assigned."
        << steEnd;
        return;
     }

     rem_relation(rt, an, de);
}

int style_manual_find_remove( appTreePtr an, char *nm , boolean categ )
{
  Initialize(style_manual_remove);
     ddKind my_k = ( categ ) ? DD_CATEGORY : DD_STYLE;
     RelType *rt = ( categ ) ? ddcats_of_node : ddsty_of_node;

     if ( !an )
         return 0;
   
     int res = 0;
     Obj *slot_set, *el1;
     Relational *rel;

     for ( ; an ; an = checked_cast(appTree,an->get_parent()) )
         if ( (slot_set = get_relation(rt, an)) )
            if ( nm ){
               ddElementPtr de = get_dd( an, nm, my_k );
               if (de) {
                  res = 1;
                  rem_relation(rt, an, de);
                  break;         
               }else
                  continue;              
            }else{
               ForEach (el1, *slot_set) {
                  rel = (Relational *)el1;
                  rem_relation(rt, an, rel);
               }
               res = 1;
               break;
           }
     return res;
}

//
// This is indeed a kludge: a class with nothing but a constructor. We
// declare a global instance of it, so the hook will get called at startup
// time before anybody tries to do any hypertext operations. (We don't 
// necessarily ever get called anywhere else before somebody tries to 
// add relations; if you restore a document with links, the save subsystem
// restores the relations automatically. If you want to see the categories,
// the hook has to be installed by then.)
//
// There must be some better way of doing this. (Especially considering that
// it depends on the relation being initialized before the kludge, which C++
// doesn't guarantee.)
//

class style_hooks {
  public:
    style_hooks();
};

static style_hooks style_cludge;

style_hooks::style_hooks() {
  obj_rel_put_hook(ddsty_of_node, style_hook);
  obj_rel_put_hook(ddcats_of_node, style_cat_hook);
}

/*
   START-LOG-------------------------------------------

   $Log: style_hooks.cxx  $
   Revision 1.4 2002/03/04 17:26:02EST ktrans 
   Change M/C/D/W/S macros to enums
// Revision 1.5  1993/05/17  15:36:27  aharlap
// put copy flag to ddsty_of_node relation
// bug # 3441
//
// Revision 1.4  1993/04/20  16:02:24  boris
// Fixed style save/restore
//
// Revision 1.3  1993/04/19  20:30:54  boris
// added style_manual_find_remove() function
//
// Revision 1.2  1993/02/05  20:28:10  boris
// Fixed bug #2397 with style/category restore
//
// Revision 1.1  1993/01/18  00:00:12  boris
// Initial revision
//

   END-LOG---------------------------------------------

*/

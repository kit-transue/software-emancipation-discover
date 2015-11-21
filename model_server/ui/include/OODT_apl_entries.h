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
#ifndef _OODT_apl_entries_h
#define _OODT_apl_entries_h

// OODT_apl_entries.h
//------------------------------------------
// synopsis:
// 
// Declarations for entries defined in ui/OODT/OODT_apl.C
//------------------------------------------

// include files

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _ui_globals_h
#include <ui-globals.h>
#endif

#ifndef _objRelation_h
#include <objRelation.h>
#endif

//#ifndef steView_h
//#include <steView.h>
//#endif

//#ifndef __word_list
//#include <word_list.h>
//#endif

//#ifndef __alias_list
//#include <alias_list.h>
//#endif

#ifndef __objRTL_h
#include <objRTL.h>
#endif

#ifndef _dialog_h
#include <dialog.h>
#endif

#ifndef _ddict_h
#include <ddict.h>
#endif

//#ifndef _viewNode_h
//#include <viewNode.h>
//#endif

#ifndef __viewGraHeader_h
#include <viewGraHeader.h>
#endif

#ifndef _oodt_class_member_sorter_h
#include <oodt_class_member_sorter.h>
#endif

#ifndef __ldrNode_h_
#include <ldrNode.h>
#endif

#ifndef __oodt_relation
#include <oodt_relation.h>
#endif

#ifndef _representation_h
#include <representation.h>
#endif
      
#ifndef _symbolArr_h
#include <symbolArr.h>
#endif      
      
#ifndef _ddSelector_h
#include <ddSelector.h>
#endif      
      
#ifndef _groupHdr_h
#include <groupHdr.h>
#endif      

extern "C" {
// C++ functions called from UIMX code

RTLNodePtr apl_OODT_get_all_of_type(ddKind);

RTLNodePtr apl_OODT_get_all_of_type_from_all_proj(ddKind);

void apl_OODT_get_all_comp_gen_dtors(symbolArr&);

//void apl_OODT_setup_class_selection(viewClassNodePtr, viewPtr);

//void apl_OODT_set_disp_selection(viewClass_mixin*, viewGraHeaderPtr,
//      member_attributes, char*);

void apl_OODT_set_disp_selection2(viewGraHeaderPtr, member_attributes, char*);

void apl_OODT_reset_sorting_order();

void apl_OODT_apply_sort(viewGraHeaderPtr vh, oodt_sorter_types sorter_type,
      int order[]);

void apl_OODT_init_sel_list(viewSymbolNode*);

void apl_OODT_explode_entities(viewGraHeaderPtr vh, oodt_explode_selections flags);

void apl_OODT_define_relation(oodt_relationPtr, const char*, const char*,
      rel_cardinality, bool, const char*, const char*, const char*,
      const char*, const char*, const char*);

void apl_OODT_assign_relation_card(const char*, const char*, const rel_cardinality);

void apl_OODT_set_subsys_selection(viewGraHeaderPtr vh, ddSelector sel);

void apl_OODT_set_builtin_rel_filter(viewPtr, int);

int apl_OODT_get_builtin_rel_filter(viewPtr);

void apl_OODT_set_DC_builtin_rel_filter(viewPtr, int);

void apl_OODT_toggle_erd_attribs(viewPtr, int);

void apl_OODT_toggle_erd_ops(viewPtr, int);

//void apl_OODT_set_relation_filter( viewClass_mixin* vcm, viewGraHeader* vh, char* text,bool inherited_rels);

void apl_OODT_update_after_rel_mod(oodt_relationPtr, const symbolPtr& org_src,
				   const symbolPtr& org_trg);

} /* extern "C" */

/*
   START-LOG-------------------------------------------

   $Log: OODT_apl_entries.h  $
   Revision 1.18 2000/07/10 23:12:00EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.15  1994/07/19  16:18:36  jerry
// Bug track: N/A
// Added return value to apl_OODT_create_subsubsys.
//
// Revision 1.14  1994/05/10  14:18:49  azaparov
// Added arguments handling during creating new functions/methods
//
// Revision 1.13  1993/06/14  20:08:57  wmm
// Fix bug 3237.
//
// Revision 1.12  1993/05/27  20:17:16  glenn
// Protect nested includes using protect_includes script.
//
// Revision 1.11  1993/04/01  16:49:15  wmm
// Fix bug 3074 (mark one rel as "main" and always present "main"
// for editing, regardless of which is selected).
//
// Revision 1.10  1993/03/03  16:51:42  wmm
// Make RTL enquiries available to other code (was static).
//
// Revision 1.9  1993/01/18  17:34:13  wmm
// Support logical/physical paths.
//
// Revision 1.8  1993/01/13  20:04:04  wmm
// Add "rename_subsystem" entry.
//
// Revision 1.7  1993/01/11  19:04:21  wmm
// Support single-step creation and addition to subsystem.
//
// Revision 1.6  1993/01/11  00:16:57  wmm
// Remove obsolete declaration.
//
// Revision 1.5  1993/01/09  14:55:46  wmm
// Add header file arguments to apl_OODT_create_func_in_subsys.
//
// Revision 1.4  1993/01/06  20:26:46  wmm
// Change apl_OODT_create_subsystem to return a pointer to the newly-
// created subsystem, so that all subsystem creation can be done
// using that routine (allowing check for duplicate subsystem names,
// for instance).
//
// Revision 1.3  1993/01/04  21:20:49  wmm
// Support single-step addition of new subsystem to existing subsystem.
//
// Revision 1.2  1992/12/28  19:33:48  wmm
// Support new subsystem implementation.
//
// Revision 1.1  1992/12/17  21:36:09  wmm
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _OODT_apl_entries_h


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
// PROJ_apl.C
//------------------------------------------
// synopsis:
// Project (desktop) application functions
//
// description:
// These are the functions called by the
// Project menus.
//------------------------------------------

// include files

#include "machdep.h"
#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#include <signal.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#include <csignal>
#endif /* ISO_CPP_HEADERS */

#include "time_log.h"
#include "genError.h"
#include <setprocess.h>
#include "objOper.h"
#include "objRTL.h"
#include "obj_print.h"
#include "ste.h"
#include "steDocument.h"
#include "smt.h"
#include "driver.h"
#include "all-stubs.h"
#include "RTL.h"
#include "machdep.h"
#include "ste_array_handler.h"

#include <miniBrowser.h>
#include <steEpochList.h>
// classes referenced

class steRegion;


// external functions

appTree* ste_get_hilited_region_node( steRegion* ) ;
void spd_fill_selection_array (symbolArr&);
int get_region_array_size();

//
// *** FUNCTIONS START HERE ***
//

extern "C" void fill_selected_objects (symbolArr *selection)
// fills selection with all selected nodes
// handles ste regions differently than driver::fill_array_with_selected_appnodes    
{
    Initialize(fill_selected_objects);

    spd_fill_selection_array(*selection);
    
    // get all selections from emacs list views
    steEpochList::fill_selected_nodes(NULL, *selection);
    miniBrowser::fill_selected_nodes(*selection);
    
    int ste_size = get_region_array_size();
    for (int ii=0; ii<ste_size; ++ii) {
        steRegion * oo = checked_cast(steRegion, get_region_array(ii));
        Obj *tn = ste_get_hilited_region_node( oo );
        selection->insert_last (tn);
    }
}

// local calls extracted because of GNU compiler complaints

static void local_PROJ_Debug_Print_Selected_Node ()
{
    symbolArr  selection;

    fill_selected_objects (&selection);

    if(selection.size()){
	symbolPtr el;
	ForEachS (el, selection)
		node_prtS (el);
    } else 
	node_repeat();
}


static void local_PROJ_Debug_Print_Selected_Tree ()
{
    Initialize(local_PROJ_Debug_Print_Selected_Tree);
    symbolArr  selection;

    fill_selected_objects (&selection);

    if(selection.size()){
	symbolPtr el;
	ForEachS (el, selection){
	    tree_prt ((objTree*)ObjPtr (el));
	}
    }
    else
	tree_repeat(cout);      
}

extern "C" {

void apl_PROJ_Debug_Follow (int num)
{
    follow (cout, num);
}

void apl_PROJ_Debug_Break ()
{

#ifndef _WIN32
    kill (OSapi_getpid(), SIGINT);
#else
    _asm int 3;
#endif
}

void apl_PROJ_Debug_Print_Selected_Node ()
{
   local_PROJ_Debug_Print_Selected_Node ();
}

void apl_PROJ_Debug_Print_Same_Node ()
{
    node_repeat ();
}

void apl_PROJ_Debug_Print_Selected_Tree ()
{
    local_PROJ_Debug_Print_Selected_Tree ();
}

} /* end extern "C" */

/*
   START-LOG-------------------------------------------

   $Log: PROJ_apl.C  $
   Revision 1.11 2001/03/21 09:04:22EST Scott Turner (sturner) 
   fixing bug 20040, "Where declared" on an interace's method
   should return the interface.
   At the same time, fixed so that aset's DEBUG menu's follow command
   (which suffered a regression) will work again.
Revision 1.2.1.14  1994/07/27  22:51:41  bakshi
Bug track: n/a
epoch list project

Revision 1.2.1.13  1994/07/20  20:54:16  mg
Bug track: NA
obsoleted playground

Revision 1.2.1.12  1994/02/09  00:59:57  builder
Port

Revision 1.2.1.11  1993/12/06  19:12:30  mg
Bug track: 4466
obsoleted assoc

Revision 1.2.1.10  1993/05/13  17:58:03  mg
*** empty log message ***

Revision 1.2.1.9  1993/04/18  01:34:23  wmm
Allow metering to be dumped for a subtree of the call tree.

Revision 1.2.1.8  1993/03/28  02:29:27  davea
removed call to symbolPtr::get_obj()

Revision 1.2.1.7  1993/02/08  16:19:48  glenn
Remove full_search, journal, and other unused functions.

Revision 1.2.1.6  1993/01/03  20:26:17  aharlap
changed objArr to symbolArr for fill_selected_objects and
spd_fill_selection_array

Revision 1.2.1.5  1992/12/18  14:10:26  boris
Fixed wrong pointer conversion

Revision 1.2.1.4  1992/12/17  15:41:38  jon
*** empty log message ***

Revision 1.2.1.3  1992/11/23  20:30:37  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  20:01:18  smit
*** empty log message ***

   END-LOG---------------------------------------------
*/

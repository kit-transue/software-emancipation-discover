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
#ifndef _CONSTR_H
#define _CONSTR_H

///////////////////////////////////////////////////////////////////////////////
// global declarations for the constraint layout
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// types of constraints with corresponding default priorities;
enum constr_type { USER, LAYOUT_TEMP, LAYOUT_PERM, STABILITY };
#define USER_PRI 0
#define LAYOUT_TEMP_PRI -3
#define LAYOUT_PERM_PRI -2
#define STABILITY_PRI -1

#define ONLY_STABILITY -1

///////////////////////////////////////////////////////////////////////////////
// modes constraints can have in the constraint manager
enum mode { DO,          // add a new constraint
	    UNDO,        // delete an old constraint
	    QUERY,       // ask if constraint is satisfied
	    CONSISTENT,  // constraint is consistent
	    WAKEUP,      // constraint is activated (test if it is consistent)
	    SLEEP,       // constraint is deactivated (test if without it the 
		 	 // other constraints are consistent
            INCONSISTENT // constraint is not consistent
};
	    
///////////////////////////////////////////////////////////////////////////////
// textual description of constraints
extern char* modedescription[];  

///////////////////////////////////////////////////////////////////////////////
// create a dummy node in graph with specified name
extern node* create_constr_dummy(graph*,char*); 
      
#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void error (char*, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#define sorry printf("sorry, not implemented\n")
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#include "edgeSet.h"
#include "interval.h"
#include "constr_node.h"
#include "constr_box.h"
#include "constr_net.h"
#include "queue.h"
#include "constr_queue.h"
#include "constr_manager.h"
#include "constr_desc.h"
#include "constr_3D.h"

extern dimension H_direction;
extern dimension V_direction;

extern int debug;

extern void space(ostream &,int);

#endif

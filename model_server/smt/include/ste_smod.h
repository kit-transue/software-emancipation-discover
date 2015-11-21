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
///////////////////////// ste_smod.h ///////////////////////////
//
// smod handler
//
#ifndef _ste_smod_h_
#define _ste_smod_h_

//#include <ldrSelection.h>
#include <objOperate.h>
#include <genString.h>
#include <smt.h>

// Let's introduce a notion of a smtTree node SMOD visibility.
// It is vital to show to User in "Full" manner just pieces that
// have signes of User attention. There are two kinds of User attention:
//     1) automatic,  (Temporary Visibility), for instance:
//         -  places that User currently changes (between reparses);
//         -  nodes that have manualy category/style assigned
//         -  hard associated tokens;
//     2) constant,   sign of a manual User request to make node or region
//        constantly visible despite of a Reparse status.   
// 
// Following enumeration describes types of a constant visibility.
// Value of a 2-bits field smod_visibility (introduced on commonTree) is a
// current node visibility status. These values are result of User activity
// and are constand during a ParaSET session.
// Value of a 1-bit temp_vis field (introduced on smtTree) is a current
// node temporary visibility status. Temp_vis fields are filled right before
// lde short extraction and are used only while ldr tree get extracted.
// 

enum SmodVisibility{
  SmodZero  = 0, // - no User attention,ldr tree is depended on Temp.Visibility
  SmodFull,      // - Full User Attention, causes Full ldr build
  SmodLocal,     // - The node gets extracted, all children on Temp.Visibility
  SmodPartial,   // - The same like Local, plus Header extraction (if any);
  SmodRegion,    // - special value of a ldrShortNode
  SmodChange    // - value of a smtTree node that corresponding to SmodFull
                 //   but gets reset to SmodZero if last Parsing is succesfull
};

// There is substantial difference between Zero and Local extraction.
// Local one preserves ldrNode - smtNode correspondence for the a specified 
// smtNode.
// Zero one preserves ldrNode - smtNode correspondence only for root of tree
// and tokens.

class smodRegion : public objRegion {
  public:
  SmodVisibility visibility;

  smodRegion( OperPoint *pt1, OperPoint *pt2, SmodVisibility  vis);
  smodRegion( appTreePtr t1,int off1, appTreePtr t2,int off2, SmodVisibility);
};

extern void smod_rebuild_node(smtTreePtr, SmodVisibility);
extern void smod_rebuild_node(smtTreePtr, SmodVisibility, int do_tidy);

extern void smod_rebuild_region( OperPoint *, OperPoint *, SmodVisibility);

extern void smod_rebuild_region( smtTreePtr start_node, int start_offset,
                     smtTreePtr end_node, int end_offset, SmodVisibility vis);

extern void smod_set_visibility( void *reg );
smtTreePtr smod_leaf_offset( smtTreePtr nd, int offset, int& cur_off );
SmodVisibility smod_vision_convert( int );
smtTreePtr smod_get_to_rebuild( smtTreePtr );
void smod_put_vision( objArr&, SmodVisibility = SmodChange, int force = 0 );
int smod_narrow (smtTreePtr& st, int of1, smtTreePtr& en, int of2, genString& txt);
extern smtTreePtr smt_get_token_by_offset (smtTreePtr st, int off, int& new_off, 
					   int end_flag = 0, int st_offset = 0);

#endif


/*
   START-LOG-------------------------------------------

   $Log: ste_smod.h  $
   Revision 1.4 2000/01/03 17:37:31EST ktrans 
   remove smod_test
// Revision 1.3  1994/05/05  20:08:03  boris
// Bug track: 6794
// Fixed "slow merge" problem, and #6977 asyncronous save
//
// Revision 1.2  1993/08/03  23:43:34  boris
// added REPLACE_REGION functionality
//
// Revision 1.1  1993/03/26  04:51:05  boris
// Initial revision
//

   END-LOG---------------------------------------------
*/

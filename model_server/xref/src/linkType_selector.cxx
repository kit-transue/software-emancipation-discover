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
// /aset/dd/src/linkType_selector.C
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Mon Jul 24 16:32:41 1995:	Created by William M. Miller
//
//------------------------------------------
#include "genError.h"


#include <linkType_selector.h>

#ifndef _ddKind_h
#include <ddKind.h>
#endif

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif

#ifndef _linkTypes_h
#include <linkTypes.h>
#endif


//****************************************
// Implementations for class linkType_selector
//****************************************



//========================================
// linkType_selector::linkType_selector
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Mon Jul 24 16:35:47 1995:	Created by William M. Miller
//========================================

linkType_selector::linkType_selector(): num_types(4),
      types_set(0), results(new symbolArr[4]),
      has_only_local_links(true) {
   Initialize(linkType_selector::linkType_selector);

   for (size_t i = 0; i < NUM_OF_LINKS; i++) {
      lt_wanted[i] = 0xff;
   }      
}


//========================================
// linkType_selector::~linkType_selector
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Mon Jul 24 16:36:29 1995:	Created by William M. Miller
//========================================

linkType_selector::~linkType_selector () {
   Initialize(linkType_selector::~linkType_selector);

   delete [] results;
}


//========================================
// linkType_selector::add_link_type
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Mon Jul 24 16:37:23 1995:	Created by William M. Miller
//========================================

void linkType_selector::add_link_type (linkType lt) {
   Initialize(linkType_selector::add_link_type);

   lt_wanted[lt] = types_set;
   if (types_set >= num_types) {
      num_types *= 2;
      symbolArr* new_res = new symbolArr[num_types];
      for (size_t i = 0; i < types_set; i++) {
	 new_res[i] = results[i];
      }
      delete [] results;
      results = new_res;
   }
   types_set++;
   if (!linkTypes::local_search[lt]) {
      has_only_local_links = false;
   }
}

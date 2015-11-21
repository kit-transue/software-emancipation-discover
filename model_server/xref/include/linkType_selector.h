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
// /aset/dd/include/linkType_selector.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Mon Jul 24 16:32:41 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _linkType_selector_h
#define _linkType_selector_h

#ifndef ISO_CPP_HEADERS
#include <stddef.h>      
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif      

#ifndef _ddKind_h
#include <ddKind.h>
#endif

//========================================
// Class linkType_selector
//	Created Mon Jul 24 16:32:41 1995 by William M. Miller
//----------------------------------------
// Description:
//
//========================================

class linkType_selector{
public:
	linkType_selector();
	~linkType_selector();
	symbolArr& operator[](linkType lt);
	void add_link_type(linkType lt);
	bool wants(linkType lt);
	bool local_only();
private:
	size_t num_types;
	size_t types_set;
	unsigned char lt_wanted[NUM_OF_LINKS];
	symbolArr* results;
	symbolArr scratch_array;
	bool has_only_local_links;
};

//========================================
// linkType_selector::wants
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Mon Jul 24 16:37:34 1995:	Created by William M. Miller
//========================================

inline bool linkType_selector::wants (linkType lt) {
   bool result;
   if (lt >= 0 && lt < NUM_OF_LINKS) {
      result = lt_wanted[lt] != 0xff;
   }
   else result = false;
   return result;
}


//========================================
// linkType_selector::operator[]
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Mon Jul 24 16:37:01 1995:	Created by William M. Miller
//========================================

inline symbolArr& linkType_selector::operator[] (linkType lt) {
   symbolArr* result;
   if (lt >= 0 && lt < NUM_OF_LINKS) {
      result = (lt_wanted[lt] != 0xff) ? &results[lt_wanted[lt]] : &scratch_array;
   }
   else result = &scratch_array;
   return *result;
}


//========================================
// linkType_selector::local_only
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Aug 28 10:26:29 1996:	Created by William M. Miller
//========================================

inline bool linkType_selector::local_only() {
   return has_only_local_links;
}



#endif

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
// /aset/subsystem/include/bit_ref.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Wed Jul 19 08:23:41 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _bit_ref_h
#define _bit_ref_h
      
#ifndef ISO_CPP_HEADERS
#include <stddef.h>      
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#endif /* ISO_CPP_HEADERS */


//========================================
// Class bit_ref
//	Created Wed Jul 19 08:23:41 1995 by William M. Miller
//----------------------------------------
// Description:
//
// Used as the result of bit_array::operator[](), allows subscripted expressions
// to be assigned to and used as values.      
//========================================

class bit_ref {
public:
	bit_ref(unsigned char& ch, size_t ofs);
	operator bool()const;
	const bit_ref& operator=(bool val);
	const bit_ref& operator |=(bool val);
	const bit_ref& operator &=(bool val);
private:
	unsigned char& data;
	size_t offset;
};


//****************************************
// Implementations for class bit_ref
//****************************************


//========================================
// bit_ref::bit_ref
//----------------------------------------
// Description:
//
// Constructor; takes a reference to a character and an offset within the character
// and makes this bit_ref point to the indicated bit.
//----------------------------------------
// History:
// 
// Wed Jul 19 08:33:32 1995:	Created by William M. Miller
//========================================

inline bit_ref::bit_ref (unsigned char& ch, size_t ofs): data(ch), offset(ofs) { }


//========================================
// bit_ref::operator bool
//----------------------------------------
// Description:
//
// Returns the value of the referenced bit.
//----------------------------------------
// History:
// 
// Wed Jul 19 08:34:06 1995:	Created by William M. Miller
//========================================

inline bit_ref::operator bool ()const {
   return (data & (0x80 >> offset)) != 0;
}


//========================================
// bit_ref::operator=
//----------------------------------------
// Description:
//
// Assigns the specified value to the referenced bit.  (Normalizes argument
// to allow for old (non-builtin) definitions of "bool".)
//----------------------------------------
// History:
// 
// Wed Jul 19 08:34:46 1995:	Created by William M. Miller
//========================================

inline const bit_ref& bit_ref::operator= (bool val) {
   data = (data & ~(0x80 >> offset)) | ((val != false) << (7 - offset));
   return *this;
}


//========================================
// bit_ref::operator |=
//----------------------------------------
// Description:
//
// "ors" the (normalized) specified value into the referenced bit.
//----------------------------------------
// History:
// 
// Wed Jul 19 08:34:58 1995:	Created by William M. Miller
//========================================

inline const bit_ref& bit_ref::operator |= (bool val) {
   data |= ((val != false) << (7 - offset));
   return *this;
}


//========================================
// bit_ref::operator &=
//----------------------------------------
// Description:
//
// "ands" the (normalized) specified value into the referenced bit.
//----------------------------------------
// History:
// 
// Wed Jul 19 08:35:08 1995:	Created by William M. Miller
//========================================

inline const bit_ref& bit_ref::operator &= (bool val) {
   data &= ((val != false) << (7 - offset));
   return *this;
}


#endif

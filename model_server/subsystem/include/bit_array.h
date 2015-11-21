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
// /aset/subsystem/include/bit_array.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Wed Jul 19 08:24:44 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _bit_array_h
#define _bit_array_h

#ifndef _bit_ref_h
#include <bit_ref.h>
#endif      

//========================================
// Class bit_array
//	Created Wed Jul 19 08:24:44 1995 by William M. Miller
//----------------------------------------
// Description:
//
// Represents a 0-origin array of bits.
//========================================

class bit_array{
public:
	bit_array(size_t sz);
	~bit_array();
	const bit_ref operator[](size_t idx) const;
	bit_ref operator[](size_t idx);
private:
	unsigned char* data;
	size_t size;
};


//========================================
// bit_array::operator[] const
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Jul 19 09:13:46 1995:	Created by William M. Miller
//========================================

inline const bit_ref bit_array::operator[] (size_t idx) const {
   return bit_ref(data[idx / 8], idx % 8);
}


//========================================
// bit_array::operator[]
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Jul 19 09:13:46 1995:	Created by William M. Miller
//========================================

inline bit_ref bit_array::operator[] (size_t idx) {
   return bit_ref(data[idx / 8], idx % 8);
}



#endif

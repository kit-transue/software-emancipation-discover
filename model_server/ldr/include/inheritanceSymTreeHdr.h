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
// /aset/ldr/include/inheritanceSymTreeHdr.h
//------------------------------------------
// description:
//
// Contains the declaration of class inheritanceSymTreeHdr.
//------------------------------------------
// History:
//
// Thu Apr 20 09:42:01 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _inheritanceSymTreeHdr_h
#define _inheritanceSymTreeHdr_h


//========================================
// Class inheritanceSymTreeHdr
//	Created Thu Apr 20 09:42:01 1995 by William M. Miller
//----------------------------------------
// Description:
//
// This class is used instead of symbolTreeHeader as the app header
// for inheritance views (ldrBrowserHierarchy).  The sole reason for
// its existence is to check for deletion of the root class and to
// delete the view if that happens; otherwise, all processing is
// delegated to symbolTreeHeader.
//========================================


#include "symbolTreeHeader.h"

class inheritanceSymTreeHdr : public symbolTreeHeader {
public:
	define_relational(inheritanceSymTreeHdr,symbolTreeHeader);
	inheritanceSymTreeHdr(char* name);
	virtual void xref_notify(const symbolArr& modified,
				 const symbolArr& added,
				 const symbolArr& deleted);
};
generate_descriptor(inheritanceSymTreeHdr,symbolTreeHeader);

#endif

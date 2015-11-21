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
// /aset/subsystem/include/groupTree.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Tue Apr 11 16:41:46 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _groupTree_h
#define _groupTree_h


//========================================
// Class groupTree
//	Created Tue Apr 11 16:41:46 1995 by William M. Miller
//----------------------------------------
// Description:
//
// This is the appTree root for groups and subsystems.       
//========================================


#ifndef _RTL_h
#include <RTL.h>
#endif
      
#ifndef _groupHdr_h      
#include <groupHdr.h>
#endif      

#ifndef _db_h
#include <db.h>
#endif      

class groupTree : public RTLNode{
public:
   define_relational(groupTree,RTLNode);
   copy_member(groupTree);
   
	groupTree( groupHdr*,bool doing_restore);
	static void db_save(app*, appTree*, db_app&, db_buffer&);
	static groupTree* db_restore(app*, appTree*, db_app&, db_buffer&);
	appPtr get_main_header();
	groupTree(const groupTree&);
};
generate_descriptor(groupTree,RTLNode);

#endif


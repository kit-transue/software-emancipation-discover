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
// subsys.h.C
//------------------------------------------
// synopsis:
// 
// Implementation of subsystem header and root classes.
//------------------------------------------

// INCLUDE FILES

#include <subsys.h>
#include <objOperate.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <objRelation.h>
#include <ddict.h>
#include <xref.h>
#include <driver.h>
#include <RTL_Names.h>
#include <RTL_apl_extern.h>

#include <xref.h>
#include <dd_or_xref_node.h>
#include <systemMessages.h>
#include <genString.h>
#include <db.h>
#include <shell_calls.h>

#include <proj.h>
#include <gtPushButton.h>

#ifndef _path_h
#include <path.h>
#endif

#include <machdep.h>
#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#ifndef _groupTree_h
#include <groupTree.h>
#endif

// RELATIONAL MACRO STUFF

projModulePtr app_get_mod(appPtr);
init_relation(subsys_of_symtree,1,NULL,symtree_of_subsys,MANY,relationMode::DM);


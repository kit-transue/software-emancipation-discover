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
// subsys_header
//------------------------------------------
// synopsis:
// App header for OODT subsystems
//
// description:
// Provides insert/remove services for OODT subsystems
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef __subsys_header
#define __subsys_header

// include files

#include "objOper.h"

class subsys_header: public app {
public:
   define_relational(subsys_header, app);
#if 0
   copy_member(subsys_header);
#else
   virtual RelationalPtr rel_copy () const;
#endif
   subsys_header();
   subsys_header(const subsys_header& other);
   void insert_obj(objInserter* oi, objInserter* ni);
   void remove_obj(objRemover *, objRemover* nr);
};

generate_descriptor(subsys_header, app);

define_relation(ddHeader_of_subsys_header, subsys_header_of_ddHeader);

#endif

/*
    START-LOG-------------------------------

    $Log: subsys_header.h  $
    Revision 1.2 1998/08/10 18:21:36EDT pero 
    port to VC 5.0: removal of typename, or, etc.
 * Revision 1.2.1.3  1993/07/13  11:31:52  builder
 * uninlined copy_member
 *
 * Revision 1.2.1.2  1992/10/09  19:57:18  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/


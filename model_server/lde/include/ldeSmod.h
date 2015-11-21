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
///////////////////////////////// ldeSmod.h /////////////////////////
//
// Smod Short Extractor class
//
#ifndef _ldeSmod_
#define _ldeSmod_

#include <objOper.h>
#include <smt.h>
#include <steDisplayNode.h>
#include <ldrSmodNode.h>

enum LdeSmodStatus{
  LdeExtractStart = 0, 
  LdeRegionStart,
  LdeRegionAppend,
  LdeWait
};

class ldeSmod{
 public:
   smtTreePtr        smt_root;     // smt root to extract
   ldrSmodNodePtr    cur_region;   // appending region 
   LdeSmodStatus     status;       // Exstractor status

   ldeSmod( smtTreePtr root = 0 );
   steDisplayNodePtr extract( smtTreePtr, steNodeType );
   steDisplayNodePtr extract_region( smtTreePtr );
   steDisplayNodePtr extract_node( smtTreePtr, steNodeType );
};

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldeSmod.h  $
   Revision 1.2 1995/07/27 20:23:04EDT rajan 
   Port
// Revision 1.1  1993/03/26  04:28:16  boris
// Initial revision
//

   END-LOG---------------------------------------------
*/

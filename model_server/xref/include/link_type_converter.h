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
#ifndef _link_type_convertor_h
#define _link_type_convertor_h
#include <general.h>
#include <dd-forward.h>
#define ForEachLink(t,e,s) for(link_iter li(s,t); e = li.get_next(t);)

#define MAX_LINK_TYPE 128
// Centralize the initialization of certain constant tables,
//     connected with link types
class  link_type_converter
{
friend class xrefSymbol;
  public:
    link_type_converter();
    ~link_type_converter(){};
    static bool is_belong_to_file(int linktype)  { return belong_to_file[linktype]; }
    static bool is_forward_link(int linktype)  { return forward_link[linktype]; }
    static bool is_reverse_link(int linktype)  { return reverse_link[linktype]; }
  private:
    static char belong_to_file[MAX_LINK_TYPE];
    static char forward_link[MAX_LINK_TYPE];
    static char reverse_link[MAX_LINK_TYPE];
};

#endif

/*
   START-LOG-------------------------------------------

   $Log: link_type_converter.h  $
   Revision 1.2 2000/04/04 10:04:45EDT sschmidt 
   Port to HP 10.  Merging from /main/hp10port to mainline.
// Initial revision
//

   END-LOG---------------------------------------------
*/



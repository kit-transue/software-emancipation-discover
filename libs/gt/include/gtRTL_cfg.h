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
#ifndef _gtRTL_cfg_h
#define _gtRTL_cfg_h

// gtRTL_cfg.h
//------------------------------------------
// synopsis:
// 
// 
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _objArr_h
#include <objArr.h>
#endif


class gtRTL_spec : public Obj
{
  public:
    gtRTL_spec(char *, char *, char *, char *, char *);
    genString name;
    genString sort;
    genString format;
    genString filter;
    genString hide;
};


class gtRTL_cfg
{
  public:
    objArr spec;

    gtRTL_cfg();
    void save_entries();

    void add_entry(gtRTL_spec *sp) { spec.insert_last(sp); };
    void remove_entry(gtRTL_spec *sp) { spec.remove(sp); };
    void rename_entry(gtRTL_spec *sp, genString& name) { sp->name = name; };
    void change_entry(gtRTL_spec *old, gtRTL_spec *new_e)
	{ remove_entry(old); add_entry(new_e); };
    gtRTL_spec *get_entry(gtPushButton *);
};

extern gtRTL_cfg *def_table;


/*
   START-LOG-------------------------------------------

   $Log: gtRTL_cfg.h  $
   Revision 1.2 1996/06/13 09:06:57EDT bing 
   Bug track: n/a
   Changed def_table into a pointer.
// Revision 1.1  1993/04/30  18:51:49  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _gtRTL_cfg_h

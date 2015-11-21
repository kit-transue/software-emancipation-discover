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
#ifndef _Search_h
#define _Search_h

//
// Search.h
//

#include <objArr.h>
#include <RTL.h>

typedef enum {
    SRCH_INEXACT  = 0x0001,
    SRCH_WORD     = 0x0002,
    SRCH_PAT_OFF  = 0x0004,
    SRCH_KEYWORD  = 0x0008,
    SRCH_VDECL    = 0x0010,
    SRCH_FDECL    = 0x0020,
    SRCH_PDECL    = 0x0040,
    SRCH_CDECL    = 0x0080,
    SRCH_EDECL    = 0x0100,
    SRCH_FIELD    = 0x0200,
    SRCH_MACRO    = 0x0400,
    SRCH_CONST    = 0x0800,
    SRCH_STRING   = 0x1000,
    SRCH_COMMENT  = 0x2000
} Constraints;


typedef struct
{
    int start;
    int end;
    int word_start;
    int word_end;
    int char_start;
    int char_end;
    // check for membership in rtl to avoid deletion problems
    Obj* start_node;
    Obj* end_node;
    // check for membership in rtl to avoid deletion problems
    Obj* hit_bank_node;
    commonTreePtr constrained_node; // node with constraints if end > start
    int node_type;
} aHit;

genArr(aHit);

struct Domain
{
    objArr nodes;
    char*  text;
    int    text_len;
    int*   text_marks;
};

class Search
{
public:
    Search(viewPtr view_header, char* pattern, int consts);
    ~Search ();
   
    short is_ste_view;
    short is_ste_tree;
    short is_smt_tree;

    viewPtr         curr_view;
    HierarchicalPtr search_header;
    commonTreePtr   search_root;
    char* pattern;
    int constraints;

    objArr constraint_nodes;
    struct Domain domain;
    int num_matches;
    genArrOf(aHit) hits;
    RTLPtr hit_bank;  // We use this rtl so we can tell if nodes get deleted

    int matches();
    int select(int index);

    int node_is_constrained(commonTree*) const;
protected:
    void setupPattern(char *, int);
    void setupHeaderAndRoot();
    void seekHits();
    void ste_seekHits();
    void addHit(int hit_start, int hit_end, int start_node_num = 0, 
        int hit_type = 1, commonTreePtr constrained_node=NULL);
//    void ste_addHit(int hit_start, int hit_end, appTreePtr start_node);
    int get_hit(int index, aHit& hit);
    int select_hit(aHit& hit, int add_selection);
};

/*
   START-LOG-------------------------------------------

   $Log: Search.h  $
   Revision 1.4 1995/07/27 20:37:32EDT rajan 
   Port
// Revision 1.1  1993/12/11  16:51:52  jon
// Initial revision
//

   END-LOG---------------------------------------------

*/

#endif // _Search_h

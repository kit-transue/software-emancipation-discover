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
#ifndef _cmd_internal_h
#define _cmd_internal_h

#include <cmd.h>

#ifndef _genError_h
#include <genError.h>
#endif
#ifndef _objCollection_h
#include <objCollection.h>
#endif
#ifndef _genString_h
#include <genString.h>
#endif
#ifndef _objOper_h
#include <objOper.h>
#endif
#ifndef _objArr_h
#include <objArr.h>
#endif
class objSet;

typedef int (*obj_type_function)(Relational*);
int obj_tree_get_nodes(objTree*node,  objArr& arr, obj_type_function fun=0);

class cmdJournal;

extern cmdJournal * cmd_cur_journal;

void cmdJournal_create
   (char const *fn, char const *cn, char const *pdf, char const *bug, char const *sub, char const *proj, int ech);

void cmdJournal_finish();


// Class for memory representation of node or header placement.
class objPlacement : public Obj
    {
 public:
    appType tree_type;           // Type of object tree
    char *tree_name;         // Name of object tree
    objArr     tree_path;    // Numeric representation of path from root
    };

void cmd_string_toc(ostream &, char *);
char *cmd_string_toc(char *);
void cmd_string_fromc(ostream &, char *);
char *cmd_string_fromc(char *);

void cmd_gen_real_write(const char *fmt,  va_list args);

#include <cmdf.h>

/*
   START-LOG-------------------------------------------

   $Log: cmd_internal.h  $
   Revision 1.4 2000/07/07 08:19:33EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.1  1994/07/20  20:12:49  mg
 * Initial revision
 *

*/

#endif

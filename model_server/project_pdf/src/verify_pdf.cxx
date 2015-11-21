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
// verify_pdf.C
//------------------------------------------
// synopsis:
//
// Verify the real file systems against pdf
//   1. display any file that is not recognized by parser
//   2. display any two files that map into the same pset
//   3. display any two dirs that map into the same pmod
//   4. display any file which only maps into one rule (no mapping to a home project)
//   5. display any top project which does not contain rtl, relation, and sub-system
//
// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#ifndef _proj_h
#include <proj.h>
#endif

#include "machdep.h"

#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef __psetmem_h
#include <psetmem.h>
#endif

#ifndef _path_h
#include <path.h>
#endif

#ifndef _fileCache_h
#include <fileCache.h>
#endif











typedef struct
{
    projMap *pm;
    int     used_count;
} rule_entry;

genArr(rule_entry);

static genArrOf(rule_entry) rule_data;

void add_one_rule(projMap *pm)
{
    Initialize(add_one_rule);

    for (int i=0;i<rule_data.size();i++)
    {
        rule_entry *p = rule_data[i];
        if (p->pm == pm)
        {
           p->used_count++;
           break;
        }
    }
}

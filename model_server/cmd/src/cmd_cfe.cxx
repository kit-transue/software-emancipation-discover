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
// file cmd_cfe.C
//
// Group of functions for "execute" Command Journal.
//
// This functions called immedeately from Command Journal text 
// using "C" Interpreter 
//
//
// History:  08/07/91    M.Furman   Initial coding for types appTree*,
//                                  objOperType, appTree
//           08/13/91    M.Furman   Reconstruction according to
//                                  capturing preliminary spec and
//                                  addition the new functions
//           08/15/91    M.Furman   Function C_rel_type added
//           09/19/91    M.Furman   clean up

#include "objOper.h"
#include "genError.h"
#include "cmd.h"
#ifndef _objArr_h
#include "objArr.h"
#endif
#include "msg.h"

// Make "objOperType" (may be it isn't needed)
extern "C" objOperType C_oper_type(char *t)
    {
    Initialize(C_oper_type);
    if(strcmp(t, "NULLOP"       ) == 0)
       ReturnValue(NULLOP       );
    if(strcmp(t, "FIRST"        ) == 0)
       ReturnValue(FIRST        );
    if(strcmp(t, "AFTER"        ) == 0)
       ReturnValue(AFTER        );
    if(strcmp(t, "REPLACE"      ) == 0)
       ReturnValue(REPLACE      );
    if(strcmp(t, "REPLACE_RIGHT") == 0)
       ReturnValue(REPLACE_RIGHT);
    ReturnValue(NULLOP);
    }

// Make "app *" value (find header)
extern "C" appPtr C_hdr(appType type, char *name)
    {
    Initialize(C_hdr);
    ReturnValue(checked_cast(app,app::get_header(type, name)));
    }

// Make "appTree *" value (find app node)
// returns NULL, if node isn't exist
extern "C" appTreePtr C_node(appType type, char *rname, ...)
{
    app *hdr;
    appTree *p;
    va_list args;
    int a, i;

    Initialize(C_node);
    hdr = checked_cast(app,app::get_header(type, rname));
    if(hdr == NULL)
    {
	ReturnValue(NULL);
    }
    p = checked_cast(appTree,hdr->get_root());    // p = root node address
    va_start(args, rname);
    for ( ; p && (a = va_arg(args, int)) != 0; ++i )
    {
	p = checked_cast(appTree,p->get_first());   // p = first son
	for(i = 1; p && i < a; i++)
	{
	    p = checked_cast(appTree,p->get_next());  // p = next brother
	}
    }
    if (!p)
    {
	msg("Could not find C_node", catastrophe_sev) << eom;
    }
    ReturnValue(p);
}

void cmd_gra_popup_end()
{
//   tracking_on = 0;
}

// Make "objArr of appTree" value (make selection array)
extern "C" objArr *C_sel(appTree *obj, ...)
    {
    va_list args;
    appTree *p = obj;
    objArr *sel = new objArr;

    Initialize(C_sel);
    va_start(args, obj);
    do
      {
      sel->insert_last((Obj *)p);
      }
      while ((p = va_arg(args, appTree *)) != NULL);
    ReturnValue(sel);
    }

// Make pointer for relation type descriptor from identifier
extern "C" RelType *C_rel_type(char *id)
    {
    Initialize(C_rel_type);
    ReturnValue(/*find_rel_type(id)*/NULL);
    }



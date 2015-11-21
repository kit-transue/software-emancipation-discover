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
// file view_clean.C
//
//  Temporary function to delete all relations between ldr and app
//  called from dialog.h.C
//
//     9/18/91    M.Furman     initial revision

#include "Object.h"
#include "graSymbol.h"
#include "representation.h"
#include "OperPoint.h"
#include "ldrHeader.h"
//#include "ldrAstHierarchy.h"
//#include "ldrStdHierarchy.h"
//#include "ldrAstFlowchart.h"
#include "viewNode.h"
#include "viewGraHeader.h"
#include "viewSelection.h"
#include "viewFn.h"
#include "steView.h"
#include "ste.h"
#include "dialog.h"
#include "ui.h"
#include "graTools.h"
#include "steScreen.h"

void t_remrel(objTree *, RelType *, RelType *r);


void temp_view_clean(view *v)
  {
  Initialize(temp_view_clean);

  objTree *p1;
  ldr *pldr;
  app *papp;
  Relational  *pbuf, *pscr;

  pldr = v->get_ldrHeader();
  papp = checked_cast(app,get_relation(app_of_ldr, pldr));
  pbuf = (Relational *)get_relation(view_buffer, v);
  pscr = (Relational *)get_relation(view_screen, v);

  if(papp != NULL)
    {
    rem_relation(app_of_ldr, pldr, papp);
    rem_relation(ldr_of_app, papp, pldr);
    }
  rem_relation(document_view, papp, v);
  rem_relation(view_document, v, papp);
  rem_relation(buffer_view, pbuf, v);
  rem_relation(view_buffer, v,    pbuf);
  rem_relation(screen_view, pscr, v);
  rem_relation(view_screen, v,    pscr);
  
  p1 = pldr->get_root();
  t_remrel(p1, apptree_of_ldrtree, ldrtree_of_apptree);
  }


void t_remrel(objTree *t, RelType *r, RelType *r1)
  {
  Initialize(t_remrel);

  objTree *t1;

  t1 = checked_cast(objTree,get_relation(r, t));
  if(t1)
    {
    rem_relation(r, t, t1); 
    rem_relation(r1, t1, t);
    }
  for(t1 = t->get_first(); t1; t1 = t1->get_next())
     t_remrel(t1, r, r1);
  }

/*
   START-LOG-------------------------------------------

   $Log: view_clean.cxx  $
   Revision 1.3 1999/06/18 16:28:25EDT ktrans 
   remove unused local variable
   Reviewed by: boris
Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:44:27  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/

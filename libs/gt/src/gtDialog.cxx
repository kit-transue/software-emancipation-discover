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
#include <gtDialog.h>

void clear_modal_dialog(gtDialog* x);

gtBase* gtDialog::dflt_parent = 0;

gtDialog::gtDialog() {}
gtDialog::~gtDialog() 
{ 
    clear_modal_dialog(this);
    if (dflt_parent==this) dflt_parent = 0; 
}

void gtDialog::popdown() 
{
  clear_modal_dialog(this);

  if (dflt_parent==this)
    dflt_parent = 0; 
  gtShell::popdown(); 
}

/*
   START-LOG-------------------------------------------

   $Log: gtDialog.C  $
   Revision 1.2 1995/09/29 11:44:59EDT kws 
   FIx crash in error popups
Revision 1.4  1993/12/14  22:43:19  andrea
Testing

Revision 1.3  1993/08/16  18:36:15  swu
bug fix for 4368

Revision 1.2  1993/08/04  15:47:40  swu
part of the fix for bug 4119

Revision 1.1  1992/10/10  21:53:11  builder
Initial revision



   END-LOG---------------------------------------------

*/

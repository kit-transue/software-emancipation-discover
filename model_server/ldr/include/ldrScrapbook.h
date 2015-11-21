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
#ifndef _ldrScrapbook_h_
#define _ldrScrapbook_h_

#include "OperPoint.h"
#include "clipboard.h"
#include "scrapbook.h"
#include "ldrClipboard.h"
#include "ldrHierarchy.h"

RelClass (ldrScrapbook);
class ldrScrapbook : public ldrHierarchy {
  public:
    ldrScrapbook();
    ldrScrapbook(scrapbookPtr, clipboardPtr = NULL);
    ldrScrapbook (const ldrScrapbook&);
    ~ldrScrapbook();

    virtual void build_selection(const ldrSelection&, OperPoint&);
    ldrClipboardPtr extract_clipboard (scrapbookPtr, clipboardPtr);

    virtual void insert_obj(objInserter *, objInserter *);
    virtual void remove_obj(objRemover *, objRemover *);

    virtual void send_string(ostream& stream) const;

    copy_member (ldrScrapbook);
    define_relational (ldrScrapbook, ldrHierarchy);
};

generate_descriptor(ldrScrapbook, ldrHierarchy);

#endif

/*
   START-LOG--------------------------------------

   $Log: ldrScrapbook.h  $
   Revision 1.3 1997/05/21 12:11:39EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
 * Revision 1.2  1993/03/18  20:28:05  aharlap
 * cleanup for paraset compatibility
 *
 * Revision 1.1  1992/10/10  21:30:26  builder
 * Initial revision
 *

   END-LOG----------------------------------------

*/


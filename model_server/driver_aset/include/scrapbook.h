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
#ifndef _scrapbook_h_
#define _scrapbook_h_

#include "objArr.h"
#include "objOper.h"
#include "clipboard.h"

RelClass (scrapbook);

typedef enum {
    unit_cut_paste,
    sequential_cut_paste
} CLIPBOARD_BEHAVIOR;

class scrapbook: public app
{
  private:
    CLIPBOARD_BEHAVIOR behavior;
    

  protected:
    clipboardPtr current_clipboard();

  public:

    define_relational(scrapbook, app);

    // clipboard functionality
    void           insert_clipboard (clipboardPtr);

    scrapbook (char *, CLIPBOARD_BEHAVIOR = unit_cut_paste);
    scrapbook (CLIPBOARD_BEHAVIOR = unit_cut_paste);
    scrapbook (const scrapbook &);
    ~scrapbook();

    void copy(objArr &objs_to_copy, bool shallow);
    void cut(objArr &objs_to_cut, bool shallow);
    void reference(objArr &objs_to_refer);
    void reference_graphics (objArr &);
    objArr *objects_to_paste(appPtr dest_header);

    void set_behavior (CLIPBOARD_BEHAVIOR);

    virtual void insert_obj(objInserter *oi, objInserter *ni);
    virtual void remove_obj(objRemover *, objRemover * nr);
    virtual void send_string(ostream& stream) const;

    copy_member(scrapbook);
};

generate_descriptor (scrapbook, app);

define_relation (scrapbook_to_append_to, scrapbook_to_append_from);

extern scrapbookPtr scrapbook_instance;

/*
   START-LOG-------------------------------------------

   $Log: scrapbook.h  $
   Revision 1.3 1998/08/10 18:21:26EDT pero 
   port to VC 5.0: removal of typename, or, etc.
 * Revision 1.2.1.2  1992/10/09  18:53:18  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif

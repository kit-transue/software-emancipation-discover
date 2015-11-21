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
#ifndef _gtStringEdXm_h
#define _gtStringEdXm_h

// gtStringEdXm.h
//------------------------------------------
// synopsis:
// Class description for the gtStringEd object in OSF Motif
//------------------------------------------

#include <gtStringEd.h>
#include <gtCallbackMgrXm.h>


class gtStringEditorXm : public gtStringEditor, public gtCallbackMgrXm
{
  public:
    gtStringEditorXm(gtBase* parent, const char* name, const char* contents);
    ~gtStringEditorXm();

    virtual long get_last_position();
    virtual void set_selection(long first, long last);
    virtual void set_callback(gtStringCB, void*);
    virtual void editable(int ed);
    virtual char *text();
    virtual void text(const char *);
    virtual void read_only();
    virtual void columns(int c);
    virtual void show_cursor(int);
    virtual void hide_unfocused_cursor();
    virtual void activate_callback(gtStringCB, void*);
    virtual void focus_callback(gtStringCB, void*);
    virtual void unfocus_callback(gtStringCB, void*);
    virtual void changed_callback(gtStringCB, void *);
};


/*
   START-LOG-------------------------------------------

   $Log: gtStringEdXm.h  $
   Revision 1.1 1993/02/23 23:57:41EST builder 
   made from unix file
 * Revision 1.2.1.4  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.3  1993/01/21  14:58:28  jon
 * Added member function changed_callback
 *
 * Revision 1.2.1.2  1992/10/09  18:44:23  jon
 * RCS History Marker Fixup
 *
   END-LOG---------------------------------------------
*/

#endif // _gtStringEdXm_h

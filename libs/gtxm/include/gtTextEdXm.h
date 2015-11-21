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
#ifndef _gtTextEdXm_h
#define _gtTextEdXm_h

// gtTextEdXm.h
//------------------------------------------
// synopsis:
// Class description for the gtTextEd object in OSF Motif
//------------------------------------------

#include <gtTextEd.h>
#include <gtCallbackMgrXm.h>


class gtTextEditorXm : public gtTextEditor, public gtCallbackMgrXm
{
  public:
    gtTextEditorXm(gtBase* parent, const char* name, const char* contents, int wrap_lines);
    ~gtTextEditorXm();

    virtual gtBase* container();

    virtual void num_rows(int);
    virtual void num_columns(int);
    virtual void read_only_mode();
    virtual void normal_mode();

    virtual long get_last_position();
    virtual char* text();
    virtual void insert(long pos, const char*);
    virtual void set_selection(long first, long last);
    virtual void text(const char*);
    virtual void show_position(long);
    virtual void word_wrap(int);
    virtual void append(const char *);
    virtual long current_pos();
    virtual void changed_callback(gtTextCB, void*);
    virtual long get_top_pos();
    virtual void set_top_pos(long);
    virtual void vertical_slider_position(int& value, int& size, int& maximum, int& minimum);

    virtual int  get_background_color();
    virtual void set_background_color(int color);

  private:
    Widget	txt;
    gtRep	txt_rep;
};


/*
   START-LOG-------------------------------------------

   $Log: gtTextEdXm.h  $
   Revision 1.2 1995/04/19 08:29:15EDT so 
   Bug track: n/a
   fix bug 9597
 * Revision 1.2.1.5  1993/11/22  15:57:31  jon
 * Added member functions get_top_pos(), set_top_pos(long), and
 * vertical_slider_position(). Also added word_wrap parameter to ctr.
 *
 * Revision 1.2.1.4  1993/05/24  17:11:35  glenn
 * Add changed_callback().
 * Add txt_rep.
 * Add destructor.
 * Add second base class gtCallbackMgrXm.
 *
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.2  1992/10/09  18:44:26  jon
 * RCS History Marker Fixup
 *
   END-LOG---------------------------------------------
*/

#endif // _gtTextEdXm_h

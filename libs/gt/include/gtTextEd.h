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
#ifndef _gtTextEd_h 
#define _gtTextEd_h

// gtTextEd.h
//------------------------------------------
// synopsis:
// class description of generic TextEditor (gtTextEditor)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif


class gtTextEditor : public gtPrimitive
{
  public:
    // Creates a gtTextEditor appropriate to the toolkit.
    // Note that gtTextEditors are multi-line, where gtStringEditors are
    // for single-line work.
    // Pre: Must have a valid gt object parent passed to it.
    // Post: The returned pointer will point to a valid gtTextEditor.
    static gtTextEditor* create(
	gtBase *parent, const char* name, const char* contents = 0);

    static gtTextEditor* create(
	gtBase *parent, const char* name, const char* contents, int wrap_lines);

    ~gtTextEditor();

    // Sets the number of rows of text that will go inside the TextEditor.
    // Pre: The gtTextEditor must have already been created for a specific
    // toolkit.
    // Post: The gtTextEditor will appear with that number of rows.
    virtual void num_rows(int rows) = 0;
    virtual void num_columns(int columns) = 0;

    // determine whether or not the text is changeable by the user
    virtual void read_only_mode() = 0;
    virtual void normal_mode() = 0;

    // gets the last position of the editor
    virtual long get_last_position() = 0;

    // specifies or returns the text in the object
    virtual char* text() = 0;
    virtual void text(const char*) = 0;

    // inserts text into the objet at location pos
    virtual void insert(long pos, const char*) = 0;

    // sets the selection of the text
    virtual void set_selection(long first, long last) = 0;

    // scrolls the text to show position pos
    virtual void show_position(long) = 0;

    // determines whether or not the text editor exhibits wraparound
    // properties
    virtual void word_wrap(int) = 0;

    // appends some text to the end of the editor's contents, and
    // scrolls to the end of the text editor.
    virtual void append(const char *) = 0;

    // returns the current insertion position
    virtual long current_pos() = 0;

    virtual void changed_callback(gtTextCB callback, void* client_data) = 0;

    // returns the position of the top most character in the window
    virtual long get_top_pos() = 0;

    // sets the top most character position in the window
    virtual void set_top_pos(long) = 0;
    
    // Read the position of the vertical slider for the scrolled window
    virtual void vertical_slider_position(int& value, int& size, int& maximum, int& minimum) = 0;
    
    // Sets the color of edit field.
    virtual int  get_background_color() = 0;
    virtual void set_background_color(int color) = 0;

  protected:
    gtTextEditor();
};


/*
   START-LOG-------------------------------------------

// $Log: gtTextEd.h  $
// Revision 1.2 1995/04/19 08:28:27EDT so 
// Bug track: n/a
// fix bug 9597
 * Revision 1.2.1.5  1993/12/19  22:12:18  builder
 * *** empty log message ***
 *
 * Revision 1.2.1.4  1993/11/22  15:38:48  jon
 * Added member functions get_top_pos(), set_top_pos(), and
 * vertical_slider_position()
 *
 * Revision 1.2.1.3  1993/05/24  17:10:46  glenn
 * Add changed_callback().
 *
 * Revision 1.2.1.2  1992/10/09  18:10:48  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:07  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:06  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:00  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:26  smit
 * Initial revision
 * 
// Revision 1.9  92/09/11  11:12:39  builder
// Make destructor public.
// 
// Revision 1.8  92/09/11  04:07:54  builder
// Add ctor and dtor.
// 
// Revision 1.7  92/08/06  18:15:30  rfermier
// added word_wrap, append, current_pos
// 
// Revision 1.6  92/08/06  02:38:18  builder
// Remove NULL
// 
// Revision 1.5  92/07/30  23:45:28  glenn
// GT method name changes
// 
// Revision 1.4  92/07/24  11:07:08  glenn
// Use const char* when appropriate.
// 
// Revision 1.3  92/07/22  13:41:55  rfermier
// added normal_mode
// 
// Revision 1.2  92/07/14  16:15:01  rfermier
// Added container fix and num_columns
// 
// Revision 1.1  92/07/08  20:39:22  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtTextEd_h

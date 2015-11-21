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
#ifndef _gtBaseXm_h
#define _gtBaseXm_h

// gtBaseXm.h
//------------------------------------------
// synopsis:
// Class definition of OSF Motif implemented gtRep
//
//------------------------------------------

#include <x11_intrinsic.h>
#if 0 /* XXX: is Motif really required?  Should it be? */
#include <Xm/Xm.h>
#endif
#include <gtBase.h>


// Patchpoints for things that do not use gt (like uimx). -jef
extern "C" void    set_widget_help_context(Widget, const char *);
extern const char* get_widget_help_context(Widget);
extern char*       widget_help_pathname(Widget, int);
extern char*       widget_pathname(Widget);

class gtRep
// the true representation of the GT object.  Definition of
// class gtRep is specific to the particular toolkit being used.
{
  public:
    static void ref(gtRep*);
    static void unref(gtRep*&);

    gtRep() : widg(NULL), ref_count(0) {}
    gtRep(Widget w) : widg(w), ref_count(0) {}

    Widget 	widget() { return widg; }
    void	widget(Widget w) { widg = w; }

    void	print_path();
    char*	pathname();

    void        help_context_name(const char *);
    const char* help_context_name();
    char*       help_pathname(int implicit_name = 0);

  private:
    Widget 	widg;
    int		ref_count;
};


class gtReasonXm
{
  public:
    gtReasonXm() : r_code(gtNoMatch) {}
    gtReasonXm(gtReason r) : r_code(r) {}
    gtReasonXm(int XmReasonConstant);

    gtReason    reason();
    const char* reason_cb();
    int         reason_code();

  private:
    gtReason r_code;
};


typedef Time gtTime;


inline int horzPixels(int hundredths_of_inches)
{
    return int((hundredths_of_inches * 0.254 * gtBase::ppmm_width) + 0.5);
}

inline int vertPixels(int hundredths_of_inches)
{
    return int((hundredths_of_inches * 0.254 * gtBase::ppmm_height) + 0.5);
}


/*
   START-LOG-------------------------------------------

   $Log: gtBaseXm.h  $
   Revision 1.3 1994/07/27 12:16:44EDT jethran 
   comment is in RCS log
 * Revision 1.2.1.7  1994/07/26  23:52:47  jethran
 * removed quickhelp stuff as part of overall removal of quickhelp
 *
 * Revision 1.2.1.6  1993/05/12  23:28:55  jon
 * Removed auto test tool and command journaling code and installed modal
 * patchpoints instead.
 *
 * Revision 1.2.1.5  1993/01/26  05:13:40  glenn
 * Remove class gtWindow.
 *
 * Revision 1.2.1.4  1993/01/25  21:05:05  jon
 * Added member functions help_context_name, help_pathname, and quickhelp_text
 * Added extern patchpoints for interfacing raw widgets to the help system
 *
 * Revision 1.2.1.3  1992/12/30  19:23:16  glenn
 * Add horzPixels, vertPixels inline functions.
 *
 * Revision 1.2.1.2  1992/10/09  18:43:36  jon
 * RCS History Marker Fixup
 *
   END-LOG---------------------------------------------
*/

#endif // _gtBaseXm_h

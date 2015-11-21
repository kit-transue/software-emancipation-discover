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
#ifndef _gtPrim_h
#define _gtPrim_h

// gtPrim.h
//------------------------------------------
// synopsis
// Primitive GT object class definitions
//------------------------------------------

#ifndef _gtBase_h
#include <gtBase.h>
#endif

class gtPrimitive : public gtBase
{	
  public:
    ~gtPrimitive();

    // Removes the object from the management of its parent.
    void unmanage();

    // unmanage followed by manage, to trigger default management.
    void remanage();

    // Attach one side of the object to its surroundings.  If a non-NULL
    // attachee is specified, it will be attached to that object, otherwise
    // to the edge of the form.  The offset is the distance between the two
    // objects being attached.  If the opposite arg is true, it will be
    // attached opposite the normal position.

    void attach(
	gtCardinalDir, gtBase* attachee = 0, int offset = 0, int opposite = 0);

    // Attach all sides of an object with offsets of zero.
    // Passing "this" indicates no attachment in the given direction.
    void attach_tblr(gtBase* T=0, gtBase* B=0, gtBase* L=0, gtBase* R=0);
    void attach_tb(gtBase* T=0, gtBase* B=0);
    void attach_lr(gtBase* L=0, gtBase* R=0);

    // Attaches one side of the object based on a percentage of the
    // width or height of the form.
    void attach_pos(gtCardinalDir, int percent);

    // Change the attachment offset on the specified side.
    void attach_offset(gtCardinalDir, int offset);
    
    // Removes all attachments on the specified side of the object.
    void detach(gtCardinalDir);

    // Convenience functions for attach functions.

    // Attach one side of the object to the opposite side of another object.
    void attach_top(gtBase *base) { attach(gtDirUp, base); }
    void attach_bottom(gtBase *base) { attach(gtDirDown, base); }
    void attach_left(gtBase *base) { attach(gtDirLeft, base); }
    void attach_right(gtBase *base) { attach(gtDirRight, base); }

    // Align one side of the object with the same side of another object.
    void attach_opp_top(gtBase *base) { attach(gtDirUp, base, 0, 1); }
    void attach_opp_bottom(gtBase *base) { attach(gtDirDown, base, 0, 1); }
    void attach_opp_left(gtBase *base) { attach(gtDirLeft, base, 0, 1); }
    void attach_opp_right(gtBase *base) { attach(gtDirRight, base, 0, 1); }

    // Connect the object to the side of the form.
    void attach_side_top() { attach(gtDirUp); }
    void attach_side_bottom() { attach(gtDirDown); }
    void attach_side_left() { attach(gtDirLeft); }
    void attach_side_right() { attach(gtDirRight); }

    void attach_pos_top(int pos) { attach_pos(gtDirUp, pos); }
    void attach_pos_bottom(int pos) { attach_pos(gtDirDown, pos); }
    void attach_pos_left(int pos) { attach_pos(gtDirLeft, pos); }
    void attach_pos_right(int pos) { attach_pos(gtDirRight, pos); }

    void attach_offset_top(int off) { attach_offset(gtDirUp, off); }
    void attach_offset_bottom(int off) { attach_offset(gtDirDown, off); }
    void attach_offset_left(int off) { attach_offset(gtDirLeft, off); }
    void attach_offset_right(int off) { attach_offset(gtDirRight, off); }

    void detach_top() { detach(gtDirUp); }
    void detach_bottom() { detach(gtDirDown); }
    void detach_left() { detach(gtDirLeft); }
    void detach_right() { detach(gtDirRight); }

    // Set constraints used by a gtPanedWindow.
    // -1 => set to default; 0 => leave unchanged; >0 => 100ths of inches.
    void min_max_height(int min = -1, int max = -1);

    // Adjusts the sensitivity of the primitive object.
    void set_sensitive(int true_if_sensitive);

    // Sets an accelerator for the primitive.
    void accelerator(const char* acc, const char* acc_text);
    void mnemonic(const char* mne);

    // Controls keyboard navigation patterns.
    // For XtIntrinsics, this is tab groups.
    void add_key_navig_group();
    void rem_key_navig_group();

    // Obsolete function -- good place to insert a new virtual function.
    virtual void construct();

    // Store & retrieve additional information to be piggybacked onto an
    // object.
    void* userData();
    void  userData(void*);

    // like manage and unmanage, but apply to a whole list at once.
    static void manage_list(gtPrimitive** plist, int num);
    static void unmanage_list(gtPrimitive** plist, int num);

    // install an accelerator from the object to the target primitive.
    void install_accelerator(gtPrimitive* targ);

    // install a set of translations onto the object
    void translations(void*);

    // specify and retrieve the foreground color of the object
    unsigned long foreground();
    void foreground(unsigned long);
    void foreground(const char*);

    // specify and retrieve the background color of the object
    unsigned long background();
    void background(unsigned long);
    void background(const char*);

    // set the font
    void font(const char*);

    // Set the text alignment on objects that understand it.
    void alignment(gtAlign);

  protected:
    gtPrimitive() {}
};


/*
   START-LOG-------------------------------------------

// $Log: gtPrim.h  $
// Revision 1.2 2000/07/07 08:10:18EDT sschmidt 
// Port to SUNpro 5 compiler
 * Revision 1.2.1.5  1993/01/26  05:09:53  glenn
 * Remove xset, yset, x_set_to_obj, y_set_to_obj, trigger_count.
 * Remove window.
 * Simplify ctor.
 *
 * Revision 1.2.1.4  1993/01/22  17:17:22  glenn
 * Add attach_tb, attach_lr, attach_tblr, alignment.
 *
 * Revision 1.2.1.3  1993/01/20  03:43:13  glenn
 * Change default arguments to min_max_height.
 *
// Revision 1.17  92/09/11  11:12:22  builder
// Make destructor public.
// 
// Revision 1.16  92/09/11  08:17:55  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.15  92/09/11  04:07:38  builder
// Add ctor and dtor.
// 
// Revision 1.14  92/08/18  12:23:20  rfermier
// added foreground functions
// 
// Revision 1.13  92/08/06  18:22:23  rfermier
// pixel measurements & conversions, better trigger_callbacks
// 
// Revision 1.12  92/08/03  03:06:36  builder
// Remove reference to NULL.
// 
// Revision 1.11  92/08/02  11:31:26  glenn
// Remove width() and height() (now in gtBase).
// Add min_max_height.
// Remove attach_side_opp_* inline functions.
// Work on comments about attach functions.
// 
// Revision 1.10  92/07/31  08:36:36  builder
// Remove map and unmap.
// 
// Revision 1.9  92/07/30  23:44:53  glenn
// GT method name changes
// 
// Revision 1.8  92/07/28  20:33:45  rfermier
// Fix stupid attach_pos_right typo bug
// 
// Revision 1.7  92/07/28  17:27:51  rfermier
// new attachment functions
// 
// Revision 1.6  92/07/28  11:51:57  rfermier
// added font function
// 
// Revision 1.5  92/07/22  13:48:59  rfermier
// fixed mnemonics, added background functions
// 
// Revision 1.4  92/07/14  16:13:53  rfermier
// Added remanage function
// 
// Revision 1.3  92/07/13  15:25:27  rfermier
// Added translations
// 
// Revision 1.2  92/07/09  18:01:06  rfermier
// *** empty log message ***
// 
// Revision 1.1  92/07/08  20:39:10  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif

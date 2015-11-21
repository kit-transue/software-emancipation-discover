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
#ifndef _gtBase_h
#define _gtBase_h

// gtBase.h
//------------------------------------------
// synopsis
// Generic Toolkit Library class definitions and member functions
// Generic object definitions go in this file, not bound to any
// particular toolkit
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

class gtBase
{
  public:
    static double ppmm_height;
    static double ppmm_width;



    // Take over control of the windowing hierarchy until the function
    // (*stop) returns true.  cd is a client-data structure which will be
    // sent as an argument to cond.

    void take_control_top(int (*stop)(void* cd), void* cd);

    static void take_control(int (*stop)(void* cd), void* cd);
    static void epoch_take_control(int (*stop)(void* cd), void* cd);

    // Register functions necessary for command recording and playback
    // inside the modal event loop.  event_playback() is called at the
    // top of the event loop, and if it returns a non-zero value then
    // the rest of the event loop is skipped.  event_record_pre() and
    // event_record_post() are called with the current event just before
    // and just after (repsectively) the event is dispatched.

    static void register_modal_patchpoints(
	int (*event_playback)(int (*stop)(void* cd), void* cd),
	void (*event_record_pre)(XEvent&, int),
	void (*event_record_post)(XEvent&, int));

    // Dispatch registered handlers for an event
    static void dispatch (gtEventPtr);

    // Initialize the help callback function.
    static void init_help_button(void (*)(gtBase*));

    // Indirect function for help-button callbacks.  This function should be
    // initialized with a call to gtBase::init_help_button().
    static void help_button_callback(gtPushButton*, gtEvent*, void*, gtReason);

    // Sets the context help path name.
    void help_context_name(const char *);

    // Returns the context help path name.
    const char *help_context_name();

    // Return a string containing the full pathname for context sensitive
    // help. If the implicit_name parameter is true, then the name returned
    // is pasted together from the first explicit name found while working
    // up the widget tree with widget names appended for all intermediate
    // widgets that do not have an explicit name.  If implicit_name is
    // false, then just the first explicit name found is returned.
    // Value is a pointer to static storage.
    char* help_pathname(int implicit_name = 0);

    // Convert between pixels and hundredths of an inch, or vice versa.
    // Note that the answer is always rounded up to the nearest unit.
    static int pix_to_hund_horiz(int);
    static int hund_to_pix_horiz(int);
    static int pix_to_hund_vert(int);
    static int hund_to_pix_vert(int);



    void* user_destroy_data;
    void (*user_destroy_func)(gtBase*, void*);



    // Virtual destructor allows derived classes first crack at suicide
    virtual ~gtBase();

    // Access the true representation of the GT object.  Definition of
    // class gtRep is specific to the particular toolkit being used.
    virtual gtRep* rep();

    // Returns the GT object into which children of this object should
    // insert themselves.
    virtual gtBase* container();



    // Manage (or unmanage) this object inside its parent.
    void manage();
    void unmanage();
    int  is_managed();

    // Make this object and its children visible (or invisible).
    void map();
    void unmap();
    int  is_mapped();
    int  is_visible();

    // Set the cursor used in the GT object.
    void cursor(gtCursor = gtDefaultCursor);

    // Processes pending exposure events immediately for the GT object.
    void update_display();

    // Flush the display.
    void flush();

    // Rings the bell at volume% of max.
    void bell(int volume);

    // Moves the object to coordinates x, y, or gets current x or y coord.
    // Returns the coordinates in pixels, NOT hundredths of an inch.
    void pos(int x, int y);
    int px();
    int py();

    // Changes the size of the object to height and width.
    // NOTE unusual argument order, use -1 to leave either dimension unchanged.
    void size(int height, int width);
    void width(int);		// Get and set width, returns in pixels
    int  pwidth();
    void height(int);		// Get and set height, returns in pixels
    int  pheight();

    // Change the thickness of the highlight border surrounding the object.
    void border(int);

    // Change the margins, i.e. spacing from edges of object to its contents.
    void margins(int horz, int vert);

    // Initialize pixels per millimeter (ppmm) constants.
    void calc_ppmm();

    // Registers a procedure to handle a particular event.
    void event_handler(long mask, int, gtEventHandler, void *);

    // Remove next event that matches mask without waiting. 
    int check_mask_event(long mask, gtEventPtr);

    // Remove next event that matches mask and window without waiting.
    int check_window_event(long mask, gtEventPtr);

    // Returns whether or not the window of the event is the same window as
    // the object.
    int my_window(gtEventPtr);

    // Returns a pointer to the window of the object.
    void* get_window();

    // Set the user-specified destruction callback.
    void user_destroy(void (*)(gtBase*, void*), void*);

    // Internal function: Use the specified rep object.
    void attach_rep(gtRep*);

    // Internal function: release resources associated with the rep object.
    void release_rep();

    // Return a string containing the full pathname of the widget,
    // or comparable information.  Value is a pointer to static storage.
    char* pathname();

    // Return a string containing the name of the widget.
    const char* name();

    static void flush_output_callback(void (*)(void*), void*);
    static void oneshot(void (*)(void*), void*);
    static void flush_oneshot();
    static void flush_output();

  protected:
    // This function is invoked after the rep is destroyed.
    static void destroy_callback(void*, gtBase*, void*);

    static int (*modal_event_playback)(int (*stop)(void* cd), void* cd);
    static void (*modal_event_record_pre)(XEvent &, int);
    static void (*modal_event_record_post)(XEvent &, int);

    gtRep* r;


    // Protected constructor prevents instantiation of raw gtBase object.
    gtBase();

    // Called by all GT objects at the end of their constructors.  Sets up
    // destroy callbacks and other initialization that must be done after
    // the Widget has been created.
    void destroy_init();
};


/*
   START-LOG-------------------------------------------

 $Log: gtBase.h  $
 Revision 1.3 1994/07/27 12:16:32EDT jethran 
 comment is in RCS log
 * Revision 1.2.1.15  1994/07/26  23:52:02  jethran
 * removed quickhelp stuff as part of overall removal of quickhelp
 *
 * Revision 1.2.1.14  1994/03/18  19:28:04  mg
 * Bug track: 1
 * oneshot implementation
 *
 * Revision 1.2.1.13  1993/11/30  16:38:35  aharlap
 * Bug track: # 5252
 * call bring_to_top from take_control for Expose
 *
 * Revision 1.2.1.12  1993/09/10  20:54:36  boris
 * Fixed double click crash on OK in Rename dialog box
 *
 * Revision 1.2.1.11  1993/05/20  16:31:39  jon
 * Put in flag to cmd_handle_event_pre/post to indicate modal
 *
 * Revision 1.2.1.10  1993/05/12  23:27:07  jon
 * Added member function gtBase::register_modal_patchpoints() to seperate
 * auto test tool and command journaling from the gt library
 *
 * Revision 1.2.1.9  1993/02/04  21:38:11  glenn
 * Add flush_output_callback, flush_output.
 *
 * Revision 1.2.1.8  1993/01/26  05:06:20  glenn
 * Remove realize, unrealize, is_realized.
 *
 * Revision 1.2.1.7  1993/01/25  21:03:52  jon
 * Added member functions help_context_name, help_pathname, and quickhelp_text
 *
 * Revision 1.2.1.6  1993/01/22  17:17:22  glenn
 * Add border, margins.
 *
 * Revision 1.2.1.5  1993/01/20  03:43:13  glenn
 * Add name.
 *
 * Revision 1.2.1.4  1992/10/23  17:40:12  glenn
 * Add cursor.
 *
 * Revision 1.2.1.3  92/10/19  14:17:04  glenn
 * Add is_managed, is_mapped, is_visible, is_realized.
 * 
 * Revision 1.2.1.2  92/10/09  18:09:54  jon
 * RCS history fixup
 * 
 * Revision 1.2.1.1  92/10/07  20:33:03  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:02  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:47  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:17  smit
 * Initial revision
 * 
// Revision 1.21  92/09/22  19:10:36  glenn
// Simplify destroy callbacks and support better user_destroy.
// 
// Revision 1.20  92/09/20  21:03:48  glenn
// Add quick help and long help.
// 
// Revision 1.19  92/09/16  13:44:45  glenn
// Add pathname.
// 
// Revision 1.18  92/09/11  11:11:49  builder
// Make destructor public.
// 
// Revision 1.17  92/09/11  00:34:59  glenn
// Add attach_rep, release_rep.
// Remove release_widget, disable_destroy_callback.
// 
// Revision 1.16  92/08/18  12:21:33  rfermier
// added clear_window
// 
// Revision 1.15  92/08/06  18:09:36  rfermier
// reason codes, pixel sizings & conversions
// 
// Revision 1.14  92/08/02  11:31:01  glenn
// Add width() and height() to fetch dimensions.
// 
// Revision 1.13  92/07/30  23:44:29  glenn
// GT method name changes
// 
// Revision 1.12  92/07/30  09:19:54  rfermier
// added get_window
// 
// Revision 1.11  92/07/29  15:54:48  rfermier
// better gtCardinalDir notation
// 
// Revision 1.10  92/07/28  17:27:04  rfermier
// Moved cardinalDir to gtBase
// 
// Revision 1.9  92/07/28  13:51:21  glenn
// Add gtDefaultAction to gtReason
// 
// Revision 1.8  92/07/22  15:47:08  glenn
// Fix declaration of gettext.
// 
// Revision 1.7  92/07/22  13:47:40  rfermier
// added x, y, event_handler
// 
// Revision 1.6  92/07/16  14:54:26  rfermier
//  Added client_data parameter to take_control
// 
// Revision 1.5  92/07/16  13:54:40  rfermier
// added flush, bell, check_window_event, check_mask_event, dispatch,
// and my_window
// 
// Revision 1.4  92/07/16  10:38:03  rfermier
// Added take_control
// 
// Revision 1.3  92/07/13  15:23:36  rfermier
// Help callback reasons
// 
// Revision 1.2  92/07/09  18:00:03  rfermier
// more documentation; resolution-independant code
// 
// Revision 1.1  92/07/08  20:38:48  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif

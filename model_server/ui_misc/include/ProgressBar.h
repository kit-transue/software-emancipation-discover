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
/*******************************************************
 *
 * FILE: ProgressBar.h
 *
 * Interface file for ProgressBar and related classes.
 *
 *******************************************************/

#ifndef _ProgressBar_h
#define _ProgressBar_h

#include <x11_intrinsic.h>
#include <Xm/Xm.h>

//
// CLASS: ProgressBar
//
// Display and manage a dialog box containing a graphical progress bar.
//

class ProgressBar
{

  public:
  
    ProgressBar(Widget, int, int=0);
    ~ProgressBar();

    void Increment(int = 1);                    // Increment the progress bar
    void SetMessage(const char *);              // Set the info label
    void Refresh();                             // Redraw, process events, and popup
    void Hide();                                // Temporarily hide the dialog
    void Finish();                              // Increment until maximum is reached
    void Popup();                               // Pop-up the dialog

  private:

    // Graphics routines:

    void DestroyWindow();
    void BuildDialog(Widget);                   // Create the dialog box
    void InitDialog(Widget);                    // Load resources
    void DrawBar();                             // Draw the progress bar
    int LoadXRes(Widget);
    int LoadGCs();
    int LoadColors();
    void UnloadXRes();
    void UnloadGCs();
    void UnloadColors();
    Widget TopShell(Widget);
    static int NormalizeVal(int, int, int);

    // Callbacks:

    static void ExposeCB(Widget, XtPointer, XtPointer);

    // Widgets and X resources:

    Widget dialog;
    Widget splash_frame;
    Widget splash_subform;
    Widget splash_label;
    Widget canvas;
    Widget msg_label;
    Widget pix_label;
    Widget num_canvas;
    Widget parent;
    GC gc_bar;
    GC gc_num;
    Pixel col_bar;
    Pixel col_num;
    static const char *fg_bar;
    static const char *fg_num;

    // Progress data:

    int tick;                   // The current position
    int limit;                  // The number of positions
    const char *default_msg;    // Default info text
    static const int dval_max;  // Maximum length of dynamic counter
    int is_splash;		// Is this the splash screen?

};

#endif /* !_ProgressBar_h */

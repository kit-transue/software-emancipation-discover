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
#ifndef _printDialog_h
#define _printDialog_h

#ifndef _WIN32
#include <dis_iosfwd.h>
#endif

#ifndef _gt_h
#include <gt.h>
#endif
#ifndef _genString_h
#include <genString.h>
#endif
#ifndef _objOper_h
#include <objOper.h>
#endif
#include <Builtin_DC_Relations.h>
#ifndef _objArr_h
#include <objArr.h>
#endif      

class printDialog {

  public:

    float scale ();
    int   landscape ();
    static printDialog *instance();
    void page_size (int &width, int &height, int &margin);  // returns size in pixels
    printDialog (class gtBase *, viewPtr);
    gtDialogTemplate *ui() { return dlg; }
    void  pages ();

  private:

    static printDialog *cur_dialog;
    int paper_width_in_pixels;
    int paper_height_in_pixels;
    float dots_per_unit;

    ~printDialog();
    void build_interface (class gtBase *, viewPtr);

    class gtDialogTemplate *dlg;
    class gtStringEditor *filename;
    class gtStringEditor *printername;
    class gtToggleButton *file_toggle;
    class gtToggleButton *printer_toggle;
    class gtToggleButton *ls_toggle;
    class gtStringEditor *s_text;
    class gtStringEditor *rows;
    class gtStringEditor *columns;
    class gtLabel *s_label;
    class gtStringEditor *paper_width;
    class gtStringEditor *paper_height;
    class gtOptionMenu   *units_menu;
    class gtOptionMenu   *std_paper_size_menu;
    class gtList         *print_list;

    static void ok_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void apply_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void file_CB (gtToggleButton*, gtEventPtr, void*, gtReason);
    static void printer_CB (gtToggleButton*, gtEventPtr, void*, gtReason);
    static void ls_toggle_CB (gtToggleButton*, gtEventPtr, void*, gtReason);
    static void fit_to_page_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void compute_pages_CB(class gtPushButton*, gtEventPtr, void*, gtReason);
    static void user_defined_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void legal_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void ledger_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void letter_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void A4_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void centimeters_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void inches_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void ps_pixels_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void scale_CB(gtStringEditor *, gtEventPtr, void*, gtReason);
    static void paper_size_CB(gtStringEditor *, gtEventPtr, void*, gtReason);

    void compute_paper_size();
    char  *printer();
    char  *file();

    float  scale_for_fit_to_page ();
    int    postscript_print (class view *v);
    int    postscript_print (ostream &, class commonTree *);
    void   fill_print_list(gtList *);
    int    postscript_print (appPtr, int append_flag = 0);
    void   copy_old_values(printDialog *old_dialog);
    int    help_mode;
    objArr printItems;
};


/*

   $Log: Print.h  $
   Revision 1.6 2000/07/07 08:17:54EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.8  1994/02/01  19:15:06  azaparov
 * Bug track: 6202
 * Fixed bug 6202.
 *
 * Revision 1.2.1.7  1993/12/07  00:13:30  azaparov
 * Bug track: 5271
 * Fixed bug 5271
 *
 * Revision 1.2.1.6  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.5  1993/01/05  00:30:01  smit
 * add new callback.
 *
 * Revision 1.2.1.4  1992/12/23  23:13:56  smit
 * Add support for Paper sizes.
 *
 * Revision 1.2.1.3  1992/12/23  03:13:57  smit
 * *** empty log message ***
 *

*/

#endif // _printDialog_h

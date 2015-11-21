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
#ifndef _gtRTL_util_h
#define _gtRTL_util_h

// gtRTL_util.h
//------------------------------------------
// synopsis:
// 
// Dialog box summoned from button at bottom of gtRTL
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

//guy:

#ifndef _genString_h
#include <genString.h>
#endif

#define Object XtObject
  #include <X11/Intrinsic.h>
#undef Object

class gtRTL;
class gtRTL_util;

struct format_info
{
    int field_width;
    genString letters;
    int index;
    gtCascadeButton *format_field;
    gtStringEditor *width_edit;
    gtHorzBox *format_box;
    gtRTL_util *me;
};


struct sort_info
{
    genString letters;
    int forward;
    int index;
    gtCascadeButton *sort_field;
    gtOptionMenu *sort_direction;
    gtForm * sort_box;
    gtRTL_util *me;
};


struct cfg_callback
{
    class gtRTL_spec *spec; 
    class gtRTL_util *thisp;
};


class gtRTL_util
{
    gtDialogTemplate *shell;

#define N_FORMAT_BOXES  15
#define N_SORT_BOXES    15

    gtVertBox *format_box;
    format_info info[N_FORMAT_BOXES];

    gtVertBox *sort_box;
    sort_info s_info[N_SORT_BOXES];

    gtStringEditor *regexp;
    gtStringEditor *hide_regexp;
    gtStringEditor *cli_exp;
    gtStringEditor *form_exp;

    gtOptionMenu *filter_menu;

    cfg_callback *c_data;
  
    gtRTL *gt_rtl;

    //Guy:  predefined_filter_nameP contains the name of the predefined filter selected.
    genString predefined_filter_name;
    
    
  public:
    gtRTL_util(gtRTL*);
    ~gtRTL_util();

    void popup();

    static void OK_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Apply_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Cancel_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Config_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Delete_CB(Widget w,XtPointer client_data,XtPointer call_data);

    static void sort_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void no_sort_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void sort_ascending_CB(gtOptionMenu *,gtEvent *,void *,gtReason);
    static void sort_descending_CB(gtOptionMenu *,gtEvent *,void *,gtReason);

    static void format_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void no_format_CB(gtPushButton *,gtEvent *,void *cd,gtReason);

    static void create_filter(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void save_filter(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void rename_filter(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void delete_filter(gtPushButton *,gtEvent *,void *cd,gtReason);

  protected:
    void parse_specs(const char *, const char *, const char *, genString&, genString&, genString&);
    void reset_specs(char *,char *,char *,char *);
    void rehistory_menus();
    void compute_specs (genString&, genString&,genString&);
    void apply();

  public:
    gtForm *create_sort_menu (sort_info *info);
    gtHorzBox *create_format_menu (format_info *);

};


/*
   START-LOG-------------------------------------------

   $Log: gtRTL_util.h  $
   Revision 1.5 2002/04/24 14:03:31EDT andrey 
   Added handler for deleting 'Filter List' dialog box by clicking 'X' in the title bar.
// Revision 1.3  1994/08/04  14:12:56  farber
// Bug track: 7717
// insert appropriate information into the viewlist header
//
// Revision 1.2  1993/11/12  20:21:16  andrea
// Bug track: n/a
// I added functionality for displaying line numbers of objects
//
// Revision 1.1  1993/04/30  18:51:49  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _gtRTL_util_h

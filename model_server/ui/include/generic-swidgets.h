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
#ifndef __generic_swidgets_h__
#define __generic_swidgets_h__

/* There are fairly extensive comments in generic-swidgets.c, found in as/ui/misc. */

#define PUSHBUTTON 0
#define LABEL 1
#define TEXT 2
#define NUMBER 3

#define UI_SW_ABORT -1
#define UI_SW_IGNORE 0
#define UI_SW_DESTROY 1
#define UI_SW_USE 2

/* generic-swidgets.c defines this because things have to be treated */
/* slightly differently. */

#if defined(ibm) || defined (__GNUG__)
#ifdef __STDC__
#undef __STDC__
#endif
#endif

#ifdef __STDC__

typedef void * swpanel;

void ui_sw_add(swpanel p, int position);
void ui_sw_addend(swpanel p);
void ui_sw_delete(swpanel p, int position);
void ui_sw_deletefocused(swpanel p);
void ui_sw_deleteall(swpanel p);
void ui_sw_dispose(swpanel p);
char *ui_sw_gettext(swpanel p, int down, int across);
swidget *ui_sw_getswidget (swpanel p, int down, int across);
void ui_sw_focus (swpanel p, int down, int across);
char *ui_sw_gettextfocused(swpanel p, int across);
char *ui_sw_getoldtext(swpanel p, int down, int across);
char *ui_sw_getoldtextfocused(swpanel p, int across);
void ui_sw_puttext(swpanel p, int down, int across, char *text);
void ui_sw_puttextfocused(swpanel p, int across, char *text);
swpanel ui_sw_create(void *clientdata, swidget parent, int topmargin, 
                     int separation, 
                     int minbbsize, int height, int acrossnum, ...);
struct ui_list *ui_sw_to_ui_list(swpanel p, int across, 
                                 int (*checkfunc)(void*,int,int,char*,char*));
void ui_sw_search(swpanel p, int across, char *text,
                  int (*checkfunc)(void*,int,int,char*,char*));
void ui_sw_activateall(swpanel p, int across);
void ui_sw_activatefocused(swpanel p, int across);
void ui_sw_activate(swpanel p, int down, int across);
void ui_sw_add_callbacks(swpanel p, 
                         void (*emptyCallback)(void *), void (*fillCallback)(void *));
int ui_sw_row_from_swidget (swpanel p, swidget *swidget);
#else

#ifdef __cplusplus

typedef void *swpanel;

extern "C" {
   void ui_sw_add(swpanel p, int position);
   void ui_sw_addend(swpanel p);
   void ui_sw_delete(swpanel p, int position);
   void ui_sw_deletefocused(swpanel p);
   void ui_sw_focus (swpanel p, int down, int across);
   void ui_sw_deleteall(swpanel p);
   void ui_sw_dispose(swpanel p);
   char *ui_sw_gettext(swpanel p, int down, int across);
   char *ui_sw_gettextfocused(swpanel p, int across);
   void *ui_sw_getswidget (swpanel p, int down, int across);
   int  ui_sw_getcount (swpanel p);
   char *ui_sw_getoldtext(swpanel p, int down, int across);
   char *ui_sw_getoldtextfocused(swpanel p, int across);
   void ui_sw_puttext(swpanel p, int down, int across, char *text);
   void ui_sw_puttextfocused(swpanel p, int across, char *text);
   void ui_sw_search(swpanel p, int across, char *text,
                  int (*checkfunc)(void*,int,int,char*,char*));
   void ui_sw_activateall(swpanel p, int across);
   void ui_sw_activatefocused(swpanel p, int across);
   void ui_sw_activate(swpanel p, int down, int across);
   void ui_sw_add_callbacks(swpanel p, 
                         void (*emptyCallback)(void *), void (*fillCallback)(void *));
    int ui_sw_row_from_swidget (swpanel p, void *swidget);
}
#else

/* Would really be a typedef if K&R compiler liked it */
#define swpanel void *

void ui_sw_add(), ui_sw_addend(), ui_sw_delete(), ui_sw_deletefocused();
void ui_sw_deleteall(), ui_sw_dispose();
char *ui_sw_gettext();
char *ui_sw_gettextfocused();
char *ui_sw_getoldtext();
char *ui_sw_getoldtextfocused();
void ui_sw_puttext();
void ui_sw_puttextfocused();
void *ui_sw_create();
struct ui_list *ui_sw_to_ui_list();
void ui_sw_activateall();
void ui_sw_activatefocused();
void ui_sw_activate();
void ui_sw_add_callbacks();

#endif
#endif

/*
 * ui_sw_create(): declare a dynamic list of swidgets
 * The ... part of the function call consists of one or more of any of the following
 * sequences. The number of such sequences must correspond to the acrossnum parameter.
 *
 * Argument sequences:    PUSHBUTTON, x, width, "label", istab, function
 *                        TEXT,       x, width, "text",  istab, function
 *                        LABEL,      x, width, "label"
 *                        NUMBER,     x, width
 *
 * The prototype for a callback function passed in the "function" parameter is as
 * follows: 
 *   void samplecallback(void *clientdata, int down, int across, char *text, char *oldtext);
 *
 * Note that the empty and fill callbacks have a different argument list:
 *   void samplefillcallback(void *clientdata);
 *
 * The activate callback passed to ui_sw_create() is called when the widget in question
 * is activated, or when the application calls one of the ui_sw_activate...() functions.
 * The fill callback is called when the first row is placed in an empty panel; the empty
 * callback is called when the last row is removed from a panel. The empty and fill
 * callbacks are set using ui_sw_add_callbacks().
 *
 * ui_sw_dispose() deletes the panel and all its associated storage. Since there's a fair
 * amount of such storage, be sure to dispose of panels when done with them.
 *
 * Warnings: 
 *
 * UIMX context structures should be swapped appropriately for the callbacks.
 * The clientdata pointer can be used to hold either a UIMX context pointer or some other
 * pointer containing whatever information is necessary for the interface to operate.
 * Note that context-swapping should NOT be done in the functions that ui_sw_to_ui_list()
 * and ui_sw_search() call - at least under normal circumstances.
 *
 * The ui_list structure returned by ui_sw_to_ui_list() is a pointer allocated by
 * ui_make_empty list(). You should call ui_free_list() when you're finished with it.
 */

#endif

/*
    START-LOG-------------------------------

    $Log: generic-swidgets.h  $
    Revision 1.1 1993/05/27 16:23:18EDT builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  19:56:53  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/


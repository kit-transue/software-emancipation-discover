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
#ifndef _ui_globals_h
#define _ui_globals_h

#ifndef _prototypes_h
#include <prototypes.h>
#endif

BEGIN_C_DECL

#ifdef __cplusplus
#  ifndef _swidget_h
#  include <swidget.h>
#  endif
#else
#  ifndef _UXLIB_INCLUDED
#  include <UxLib.h>
#  endif
#endif

#ifndef WIN32
#ifndef X_H
#include <X11/X.h>
#endif

/* typedef unsigned long XID; */
/* typedef XID Window; */

#ifndef _XtIntrinsic_h
/* -- there is a conflict between X11: Object, and nihcl: Object -- */
#ifndef _XtObject_h
#define _XtObject_h
#endif
#include <X11/Intrinsic.h>
#endif
#else
typedef struct _WidgetRec *Widget;
#endif /* WIN32 */

#ifndef _ui_struct_h
#include <ui-struct.h>
#endif


/* This is for the file-selector and db-selector. */

struct ui_selector_info
{
    struct ui_list * (*search)(); /* search(char *mask); */
    char *itemname;
    char *path;
    char *name;
    char *mask;
    char *title;
    void (*call) PROTO((char *));		/* call(char *filename) */
    char *extension;
#ifdef DEBUG
    unsigned crc;
#endif
};


struct ui_RTL_info
{
    char *label3;
    char *label2;
    char *label1;
    char *label0;
    
/* Avoid overlap with token-pasting macros in C++ file <generic.h> */
#undef name3
#undef name2
    
    char *name3;
    char *name2;
    char *name1;
    char *name0;
    
    int  int3;
    int  int2;
    int  int1;
    int  int0;
    
    void *void3;
    void *void2;
    void *void1;
    void *void0;
    
    char *title;		/* Title of the window */
    void *parent;		/* swidget ptr */
    void *dialog;		/* dialog ptr */
    
    void (*call) PROTO((struct ui_RTL_info*));
    char (*verify) PROTO((struct ui_RTL_info*));
};

struct ui_list* ui_make_empty_list PROTO(());
void  ui_free_list PROTO((struct ui_list*));
void  ui_clear_list PROTO((struct ui_list*));
int   ui_len_of_the_list PROTO((struct ui_list*));
void  ui_list_set_size PROTO((struct ui_list*, int new_size));
void  ui_list_add PROTO((struct ui_list*, char*));
void  ui_list_add_at PROTO((struct ui_list*, char*, int));
void  ui_list_no_nul_add_at PROTO((struct ui_list*, char*, size_t, int));
void  ui_list_add_with_icon PROTO((struct ui_list*, char*, size_t, char, int));
void  ui_list_remove_at PROTO((struct ui_list*, int));
char* ui_get_list_element PROTO((struct ui_list*, int));
void  ui_output_list PROTO((struct ui_list*));
void  ui_sort_list PROTO((struct ui_list*));
void  ui_sort_list_caseless PROTO((struct ui_list*));
void  ui_list_to_widget PROTO((struct ui_list*, Widget widget));
void  ui_list_to_widget_with_charset PROTO((struct ui_list*, Widget widget, char *charset));
void  ui_list_to_uimx PROTO((swidget sw, struct ui_list*));
void  ui_uimx_to_list PROTO((char*, struct ui_list*));
void  ui_table_to_list PROTO((char**, struct ui_list*, int));

END_C_DECL

/*
    START-LOG-------------------------------

   $Log: ui-globals.h  $
   Revision 1.3 1999/12/03 16:56:42EST sschmidt 
   
 * Revision 1.2.1.4  1994/02/08  18:45:47  builder
 * Port
 *
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:24  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif /* _ui_globals_h */

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
/*
  param-list.h --- 11.7.91 17:01:40 --- mark simpson

  See file param-list.c for information upon the functions.
*/

/* to give a visual cue of the parametric swidget lists, make the */
/* backdrop this color */
#define PARAM_LIST_COLOR "white"

#define SPACE 10
#define ROW_HEIGHT 25

#define getY(n) (SPACE + ((n) * (ROW_HEIGHT + SPACE)))
#define Init(list) \
  list = (swidget *) malloc (1 * sizeof (swidget));
#define Resize(list, size) \
  list = (swidget *) realloc (list, size * sizeof (swidget));

extern int findFocus();
extern resizeList();
extern destroyField();
extern swidget createTextField();
extern swidget createLabel();
extern void printList();

/*
    START-LOG-------------------------------

    $Log: param-list.h  $
    Revision 1.1 1993/05/27 16:23:23EDT builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  19:57:11  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/


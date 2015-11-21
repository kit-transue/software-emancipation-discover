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
#ifndef _ui_struct_h
#define _ui_struct_h

#ifndef __UI_CRC
#  define __UI_CRC
#  ifdef DEBUG
    
#    define CheckStruct(ss) {     \
       char *z = (char*)&(ss);    \
       int i,x;                   \
       for (i=x=0; i<sizeof(ss)-sizeof(unsigned); x+=z[i++]); \
       if (x!=(ss).crc) {         \
         fprintf(stderr, "Structure overwrite detected, file %s, line %d\n",\
	   __FILE__, __LINE__);   \
         exit(1);                 \
       }                          \
     }
#    define CalcCheck(s)  {       \
       char *z = (char*)&(s);     \
       int i,x;                   \
       for (i=x=0; i<sizeof(s)-sizeof(unsigned); x+=z[i++]); \
       (s).crc=x;                 \
     }
#  else /* DEBUG */
#    define CheckStruct(s)
#    define CalcCheck(s)
#  endif /* DEBUG */
#endif


/* These are for SPD get-type-list routine */

#define CHAR     0
#define INT      1
#define REAL     2
#define COMPLEX  3
#define ARRAY    4
#define RECORD   5
#define ALL      6

/* Reference types for data-define */

typedef enum {REF_VAL=1, REF_REF=2, REF_POINT=3} referenceType;

/*
#define REF_VAL   1
#define REF_REF   2
#define REF_POINT 3
*/

typedef enum {IS_STE, IS_SPD} windowtypes;

typedef enum {SEARCH_PREVIOUS, SEARCH_NEXT, SEARCH_FROM_TOP, SEARCH_FROM_BOTTOM} search_direction_type;

typedef enum {SEARCH_CHAR, SEARCH_WORD} search_domain_type;
/*
 * Structure used by a variety of user-interface functions.
 *
 */



struct ui_list {
  char **strs;    /* array of strings */
  int num;        /* current size of array */
  int maxnum;     /* max allocated size of array */
};


/*
 * Structure used by STE category functions
 *
 */

struct ui_category {
  char *name;
  char *old_name;
  char *style_name;
  char *gl_flag;         /* User or System defined */
};

/*
 *
 * Structure used by STE style functiosn
 */

struct ui_style {
  char *name;
  char *old_name; /* old name in a case of changing of name */
  int type; /* 0=doc 1=title 2=paragraph 3=character */
  int level; /* -1=unspecified */
  float page_height;
  float page_width;
  int landscape; /* 0=>portrait */
  float top_margin;
  float bottom_margin;
  int doc_units; /* 0=inches 1=points 2=millimeters */
  float left_margin;
  float right_margin;
  int relative_to; /* 0=page 1=parent 2=sibling */
  int justification; /* -1=unspecified 0 = ragged 1=justified 2=centered */
  int para_units; /* 0=inches 1=points 2=millimeters */
  float first_line_indent;
  float space_before;
  float space_between_lines;
  int enum_type; /* -1=unspecified 0=none 1=bullets 2=simple numbers
                    3=cumulative numbers 4=roman caps 5=roman lc
                    6=alpha caps 7=alpha lc */
  char terminator;
  char separator;
  char* font_name;
  float font_size;
  int attributes; /* -1=unspecified 0=none 1=italic 2=bold 4=underline
                     8=shaded 16=reverse */
  char *background;
  char *foreground;
  char *curs_bg;
  char *curs_fg;
};

/*
 *
 * Structure used by STE format functions
 */

struct ui_format {
  char *name;
  int line_spacing;  /* 10 = single, 15 = 1.5, 20 = double spaced */
  float left_margin;  /* not checked for range */
  float right_margin; /* not checked for range */
  int is_enumerated;
  int has_bullets;
};


#endif /* _ui_struct_h */

/*
    START-LOG-------------------------------

    $Log: ui-struct.h  $
    Revision 1.1 1993/05/27 16:23:31EDT builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  19:57:26  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/


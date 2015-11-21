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
/////////////////////////   FILE steStyle.h   ///////////////////////////
//
// -- Contains Class Declaration
//
#ifndef steStyle_h
#define steStyle_h
#ifndef _objTree_h
#include "objTree.h"
#endif
RelClass(steStyle);

//#ifndef _steBufferHeap_h
//#include "steBufferHeap.h"
//#endif
#ifndef _objOper_h
#include "objOper.h"
#endif

// The following declaration enumerates all the supported formatting
// controls in a style:

enum ste_format_control {
   FIRST_FORMAT_CONTROL,

   // Document-level controls

   PAGE_HEIGHT = FIRST_FORMAT_CONTROL,
   PAGE_WIDTH,
   LANDSCAPE,		// false => portrait
   TOP_MARGIN,
   BOTTOM_MARGIN,	// relative to bottom edge

   // Structure-level controls

   STRUCTURE_CONTROLS,	// first structure control
   SPACE_BEFORE = STRUCTURE_CONTROLS,
   SPACE_BETWEEN_LINES,
   LEFT_MARGIN,
   RIGHT_MARGIN,	// relative to right edge
   FIRST_LINE_INDENT,	// relative to left margin
   ENUMERATION,
   JUSTIFIED_OR_CENTERED,

   // Character-level controls

   CHARACTER_CONTROLS,	// first character control
   FONT = CHARACTER_CONTROLS,	// value(FONT) returns size of font
   CHAR_ATTRIBUTES,	// same as stePS_attributes

   // End of controls

   NUM_FORMAT_CONTROLS
};

// The following declaration enumerates the types of styles supported:

enum ste_style_type {
   DOCUMENT_STYLE,
   PARAGRAPH_LEVEL_STYLE,
   TITLE_LEVEL_STYLE,
   PARAGRAPH_STYLE,
   TITLE_STYLE,
   CHARACTER_STYLE,
//
   TABLE_STYLES,
   TABLE_NAME_STYLE = TABLE_STYLES,
   LEFT_CORNER_STYLE,
   ZERO_COLUMN_STYLE,
   ZERO_ROW_STYLE,
   DEFAULT_ITEM_STYLE, 
//
   DOCUMENT_TITLE_STYLE,
   FULL_COMMENT_STYLE,
   INLINE_COMMENT_STYLE
};

// The following declaration enumerates the units in which measurements
// may be specified (not all units apply to all measurements):

enum ste_spacing_units {
   INCHES,
#ifndef _WIN32
   POINTS,
#else
   __POINTS,
#endif
   MILLIMETERS
};

// The following declaration enumerates the bases relative to which
// left and right margins may be specified:

enum ste_spacing_base {
   PAGE_RELATIVE,
   PARENT_RELATIVE,
   SIBLING_RELATIVE
};

// The following declaration enumerates the enumeration styles available:

enum ste_enumeration_style {
   NO_ENUMERATION,
   BULLETS,
   SIMPLE_NUMBERS,
   CUMULATIVE_NUMBERS,
   CAPITAL_ROMAN,
   LOWERCASE_ROMAN,
   CAPITAL_ALPHABETIC,
   LOWERCASE_ALPHABETIC
};

// The following declaration enumerates the possibilities for justification:

enum ste_justification {
   RAGGED_RIGHT,
   JUSTIFIED,
   CENTERED
};

#include "objOper.h"

//----------------------------------------------------------------

define_relation(style_of_node,node_of_style);

class steStyle : public Relational {
public :

  virtual void print(ostream&, int) const;
  define_relational(steStyle,Relational);
  declare_copy_member(steStyle);

  steStyle (const char * name, char* encoding = NULL);
  steStyle (const steStyle& oo);
  ~steStyle();

  int operator==(const steStyle&) const;

  void value(ste_format_control, float);
  float value(ste_format_control) const;
  float point_value(ste_format_control) const;

  void spacing(ste_format_control, ste_spacing_units);
  ste_spacing_units spacing(ste_format_control) const;

  void lr_margin_base(ste_spacing_base);
  ste_spacing_base lr_margin_base() const;

  void specified(ste_format_control, bool);
  bool specified(ste_format_control) const;

  void style_type(ste_style_type);
  ste_style_type style_type() const;

  void level(int);
  int level() const;

  void separator_char(char);
  char separator_char() const;

  void terminator_char(char);
  char terminator_char() const;

  void font(const char*);
  const char* font() const;

  void fg_color(const char*);
  const char* fg_color() const;

  void bg_color(const char*);
  const char* bg_color() const;

  void cursor_fg_color(const char*);
  const char* cursor_fg_color() const;

  void cursor_bg_color(const char*);
  const char* cursor_bg_color() const;

  void name(const char*);
  const char* name() const;

  char const *get_name() const;

  static char *gettok(char *);
  char* encode(char* buffer, size_t buffer_len) const;

  static steStyle* make_absolute_style_for(commonTreePtr);

  void incorp(steStylePtr, commonTreePtr);
  void set_defaults();
  int icon_index;
private:
  void compose_style(commonTreePtr, int&);
  void incorporate(steStylePtr, commonTreePtr);

  float values[NUM_FORMAT_CONTROLS];
  ste_spacing_units spacing_units[NUM_FORMAT_CONTROLS];
  ste_spacing_base margin_base;
  unsigned long specified_controls;
//assert(bitcount(unsigned long) >= NUM_FORMAT_CONTROLS)
  ste_style_type type;
  int lvl;
  char separator;
  char terminator;
  char font_name[64];
  char fg_clr[32];
  char bg_clr[32];
  char cursor_fg_clr[32];
  char cursor_bg_clr[32];
  char* style_name;
};

  generate_descriptor(steStyle,Relational);
#endif

/*
   START-LOG-------------------------------------------

   $Log: steStyle.h  $
   Revision 1.2 1996/04/30 13:22:43EDT kws 
   
 * Revision 1.5  1993/12/08  19:54:19  trung
 * Bug track: 0
 * change header
 *
 * Revision 1.4  1993/07/14  01:57:37  boris
 * uninline rel_copy()
 *
 * Revision 1.3  1993/06/05  00:47:31  boris
 * Added icon_index to steStyle
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.4  1992/11/25  23:25:49  boris
 *  Fixed changed Default Style printing crash
 *
 * Revision 1.2.1.3  1992/11/25  15:01:59  boris
 * Added Raw/Smod print facilities
 *
 * Revision 1.2.1.2  1992/10/09  20:01:15  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/

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
////////////////////////   FILE steStyle_h.C   ///////////////////////////
//
// -- Contains  related Class Definition                  
//
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <math.h>
#include <steStyle.h>
#include "steView.h"
#include "genError.h"
#include "stePostScript.h"
#include "ste_get.h"
#include "steHeader.h"
#include "ste_style_handler.h"
#include "steAppStyle.h"
#include "ste_table_handler.h"
#include "steTableNode.h"
#include "ste_commonTree_functions.h"
#include "ste_format_handler.h"

  init_relational(steStyle,Relational);
  init_relation(node_of_style,MANY,NULL,style_of_node,1,relationMode::C);
// ENCODING KEYS

static const char* encoding_key[NUM_FORMAT_CONTROLS] = {
	"PH",		// PAGE_HEIGHT
	"PW",		// PAGE_WIDTH
	"LS",		// LANDSCAPE
	"TM",		// TOP_MARGIN
	"BM",		// BOTTOM_MARGIN
	"IP",		// SPACE_BEFORE ("InterParagraph")
	"IL",		// SPACE_BETWEEN_LINES ("InterLine")
	"LM",		// LEFT_MARGIN
	"RM",		// RIGHT_MARGIN
	"FL",		// FIRST_LINE_INDENT
	"EN",		// ENUMERATION
	"JC",		// JUSTIFIED_OR_CENTERED
	"FS",		// FONT ("Font Size")
	"AT"		// CHAR_ATTRIBUTES
};

// DEFAULT VALUES

static const float default_value[NUM_FORMAT_CONTROLS] = {
	11.0,		// PAGE_HEIGHT
	8.5,		// PAGE_WIDTH
	false,		// LANDSCAPE
	1.0,		// TOP_MARGIN
	1.0,		// BOTTOM_MARGIN
	0.2,		// SPACE_BEFORE
	0.02,		// SPACE_BETWEEN_LINES
	0.75,		// LEFT_MARGIN
	1.0,		// RIGHT_MARGIN
	0.0,		// FIRST_LINE_INDENT
	NO_ENUMERATION,	// ENUMERATION
	JUSTIFIED,	// JUSTIFIED_OR_CENTERED
	14.0,		// FONT
	stePS_plain	// CHAR_ATTRIBUTES
};

// NOTE: LANDSCAPE, ENUMERATION, and JUSTIFIED_OR_CENTERED are given in
// points to prevent conversion to other units, since they are actually
// integral enumeration values rather than measurements.

#ifndef _WIN32
static const ste_spacing_units default_spacing[NUM_FORMAT_CONTROLS] = {
	INCHES,		// PAGE_HEIGHT
	INCHES,		// PAGE_WIDTH
	POINTS,		// LANDSCAPE
	INCHES,		// TOP_MARGIN
	INCHES,		// BOTTOM_MARGIN
	INCHES,		// SPACE_BEFORE
	INCHES,		// SPACE_BETWEEN_LINES
	INCHES,		// LEFT_MARGIN
	INCHES,		// RIGHT_MARGIN
	INCHES,		// FIRST_LINE_INDENT
	POINTS,		// ENUMERATION
	POINTS,		// JUSTIFIED_OR_CENTERED
	POINTS,		// FONT
	POINTS		// CHAR_ATTRIBUTES
};
#else /*_WIN32*/
static const ste_spacing_units default_spacing[NUM_FORMAT_CONTROLS] = {
        INCHES,         // PAGE_HEIGHT
        INCHES,         // PAGE_WIDTH
        __POINTS,         // LANDSCAPE
        INCHES,         // TOP_MARGIN
        INCHES,         // BOTTOM_MARGIN
        INCHES,         // SPACE_BEFORE
        INCHES,         // SPACE_BETWEEN_LINES
        INCHES,         // LEFT_MARGIN
        INCHES,         // RIGHT_MARGIN
        INCHES,         // FIRST_LINE_INDENT
        __POINTS,         // ENUMERATION
        __POINTS,         // JUSTIFIED_OR_CENTERED
        __POINTS,         // FONT
        __POINTS          // CHAR_ATTRIBUTES
};
#endif /*_WIN32*/

static const char default_separator = '.';
static const char default_terminator = '.';
static const char default_font_name[] = "";
static const char default_fg_color[] = "";
static const char default_bg_color[] = "";
static const char default_cursor_fg_color[] = "";
static const char default_cursor_bg_color[] = "";
/*
static const char default_font_name[] = "times"; // Times-Roman
static const char default_fg_color[] = "black";
static const char default_bg_color[] = "white";
static const char default_cursor_fg_color[] = "white";
static const char default_cursor_bg_color[] = "black";
*/
// The following routine is like "strtok" from the standard C library
// except that it allows for empty tokens and always assumes a delimiter
// of ",".

char* steStyle::gettok(char* start) {
   Initialize(gettok);
   static char* lasttime = NULL;
   char* tok = (start) ? start : lasttime;
   if (tok) {
      if (*tok) {
	 char *p;
         for (p = tok; *p && *p != ','; p++)
            ;
         if (*p)
            *p++ = 0;
         lasttime = p;
      }
      else lasttime = tok = NULL;
   }
   return (tok);
}

/********************************************
 * steStyle::steStyle (const char *, char*) *
 ********************************************/

steStyle::steStyle(const char* name, char* encoding) :
    icon_index(-1)
{
 Initialize(steStyle::steStyle);
   specified_controls = 0;
   if (encoding) {

// Read style from encoded form

      char* tok = gettok(encoding);
      style_name = new char[strlen(tok) + 1];
      strcpy(style_name, tok);
      tok = gettok(NULL);
      type = ste_style_type(atoi(tok));
      tok = gettok(NULL);
      lvl = atoi(tok);
      tok = gettok(NULL);
      margin_base = ste_spacing_base(atoi(tok));
      tok += strlen(tok) + 1;
      separator = tok[0];
      terminator = tok[1];
      tok = gettok(tok + 2);
      strcpy(font_name, tok);
      tok = gettok(NULL);
      strcpy(fg_clr, tok);
      tok = gettok(NULL);
      strcpy(bg_clr, tok);
      tok = gettok(NULL);
      strcpy(cursor_fg_clr, tok);
      tok = gettok(NULL);
      strcpy(cursor_bg_clr, tok);
      specified_controls = 0L;    // default none seen
      while ((tok = gettok(NULL)) != NULL) {
         for (int i = FIRST_FORMAT_CONTROL; i < NUM_FORMAT_CONTROLS; i++) {
            if (memcmp(tok, encoding_key[i], 2) == 0) {
               ste_format_control c = ste_format_control(i);
               value(c, atof(tok + 2));
               tok = gettok(NULL);
               spacing(c, ste_spacing_units(atoi(tok)));
               break;
            }
         }
      }
   }
   else {

// create default document format (type and values will be reset later
// if necessary)

      margin_base = PAGE_RELATIVE;

      style_name = new char[strlen(name) + 1];
      strcpy(style_name, name);
      type = DOCUMENT_STYLE;
      lvl = 0;
      separator = default_separator;
      terminator = default_terminator;

      strcpy(font_name, default_font_name);
      strcpy(fg_clr, default_fg_color);
      strcpy(bg_clr, default_bg_color);
      strcpy(cursor_fg_clr, default_cursor_fg_color);
      strcpy(cursor_bg_clr, default_cursor_bg_color);
//clean controls
      for (int i = FIRST_FORMAT_CONTROL; i < NUM_FORMAT_CONTROLS; i++) {
           ste_format_control c = ste_format_control(i);
           value(c, 0);
           spacing(c, default_spacing[ c ]);
           specified(c, false);
      }
   }
}

/****************************************
 * steStyle::steStyle (const steStyle&) *
 ****************************************/

steStyle::steStyle(const steStyle& other) {
   *this = other;
   style_name = new char[strlen(other.style_name) + 1];
   strcpy(style_name, other.style_name);
}

/*************************
 * steStyle::~steStyle() *
 *************************/

steStyle::~steStyle() {
   delete style_name;
}

/*****************************************
 * steStyle::operator==(const steStyle&) *
 *****************************************/

int steStyle::operator==(const steStyle& other) const {
   Initialize(steStyle::operator==);
   if (icon_index !=  other.icon_index)
      return 0;

   static float epsilon = .0001;
   if (strcmp(style_name, other.style_name) != 0)
      return (0);
   if (type != other.type)
      return (0);
   for (int i = FIRST_FORMAT_CONTROL; i < NUM_FORMAT_CONTROLS; i++) {
      ste_format_control c = ste_format_control(i);
      if (specified(c) != other.specified(c))
         return (0);
      if (specified(c) && (fabs(values[i] - other.values[i]) > epsilon ||
            spacing_units[i] != other.spacing_units[i]))
         return (0);
   }
   if (margin_base != other.margin_base)
      return (0);
   if (type == PARAGRAPH_LEVEL_STYLE || type == TITLE_LEVEL_STYLE) {
      if (lvl != other.lvl)
         return (0);
      if (separator != other.separator || terminator != other.terminator)
         return (0);
   }
   if (strcmp(font_name, other.font_name) != 0)
      return (0);
   if (strcmp(fg_clr, other.fg_clr) != 0)
      return (0);
   if (strcmp(cursor_fg_clr, other.cursor_fg_clr) != 0)
      return (0);
   if (strcmp(cursor_bg_clr, other.cursor_bg_clr) != 0)
      return (0);
   return (1);
}   

/**********************************************
 * steStyle::value(ste_format_control, float) *
 **********************************************/

void steStyle::value(ste_format_control c, float v) {
   if (c < NUM_FORMAT_CONTROLS) {
      values[c] = v;
      specified(c, true);
   }
   return;
}

// Values are always stored internally as points; the spacing units are
// just to make sure that a user who specifies a value in inches will see
// inches if he/she edits the value at a later time.  Conversions from
// specified units to points and vice versa are applied when the units
// are specified and when the value is read, respectively; i.e., it is
// necessary to set a value by a call to value first, then to spacing.

static float unit_xlat[] = {
   72.0,              // INCHES
   1.0,               // POINTS
   2.835              // MILLIMETERS
};

/***************************************
 * steStyle::value(ste_format_control) *
 ***************************************/

float steStyle::value(ste_format_control c) const {
   if (c == LANDSCAPE || c == ENUMERATION || c == JUSTIFIED_OR_CENTERED ||
         c == CHAR_ATTRIBUTES) {
      return (values[c]);
   }
   else return (values[c] / unit_xlat[spacing_units[c]]);
}

/*********************************************
 * steStyle::point_value(ste_format_control) *
 *********************************************/

float steStyle::point_value(ste_format_control c) const {
   Initialize(steStyle::point_value);
   return (values[c]);
}

/************************************************************
 * steStyle::spacing(ste_format_control, ste_spacing_units) *
 ************************************************************/

void steStyle::spacing(ste_format_control c, ste_spacing_units u) {
   Initialize(steStyle::spacing);
   if (c < NUM_FORMAT_CONTROLS  && c >= 0) {
      spacing_units[c] = u;
      if (c != LANDSCAPE && c != ENUMERATION && c != JUSTIFIED_OR_CENTERED &&
            c != CHAR_ATTRIBUTES)
         values[c] *= unit_xlat[u];
   }
   return;
}


/**********************************************
 * steStyle::lr_margin_base(ste_spacing_base) *
 **********************************************/

void steStyle::lr_margin_base(ste_spacing_base b) {
   Initialize(steStyle::lr_margin_base);
   margin_base = b;
   return;
}


/*************************************************
 * steStyle::specified(ste_format_control, bool) *
 *************************************************/

void steStyle::specified(ste_format_control c, bool v) {
   typedef unsigned long ulong;
   ulong mask = ~(1UL << c);
   specified_controls = (specified_controls & mask) | (ulong(v != 0) << c);
   return;
}

/*******************************************
 * steStyle::specified(ste_format_control) *
 *******************************************/

bool steStyle::specified(ste_format_control c) const {
   return ((specified_controls & (1UL << c)) != 0UL);
}

// Setting the style type also restricts the "specified" flags to the
// appropriate range of controls

/****************************************
 * steStyle::style_type(ste_style_type) *
 ****************************************/

void steStyle::style_type(ste_style_type t) {
   type = t;
   ste_format_control first_valid = (t == CHARACTER_STYLE) ?
         CHARACTER_CONTROLS : (t > DOCUMENT_STYLE) ? STRUCTURE_CONTROLS :
         FIRST_FORMAT_CONTROL;
   for (int i = FIRST_FORMAT_CONTROL; i < first_valid; i++) {
      ste_format_control c = ste_format_control(i);
      specified(c, false);
   }
   return;
}

/**************************
 * steStyle::style_type() *
 **************************/

ste_style_type steStyle::style_type() const {
   return (type);
}

/************************
 * steStyle::level(int) *
 ************************/

void steStyle::level(int l) {
   Initialize(steStyle::level);
   lvl = l;
   return;
}

/*********************
 * steStyle::level() *
 *********************/

int steStyle::level() const {
   Initialize(steStyle::level);
   return (lvl);
}


/*******************************
 * steStyle::font(const char*) *
 *******************************/

void steStyle::font(const char* nm) {
   Initialize(steStyle::font);
   strcpy(font_name, nm);
   return;
}

/********************
 * steStyle::font() *
 ********************/

const char* steStyle::font() const {
   Initialize(steStyle::font);
   return (font_name);
}

/***********************************
 * steStyle::fg_color(const char*) *
 ***********************************/

void steStyle::fg_color(const char* c) {
   Initialize(steStyle::fg_color);
   strcpy(fg_clr, c);
   return;
}

/************************
 * steStyle::fg_color() *
 ************************/

const char* steStyle::fg_color() const {
   Initialize(steStyle::fg_color);
   return (fg_clr);
}

/***********************************
 * steStyle::bg_color(const char*) *
 ***********************************/

void steStyle::bg_color(const char* c) {
   Initialize(steStyle::bg_color);
   strcpy(bg_clr, c);
   return;
}

/************************
 * steStyle::bg_color() *
 ************************/

const char* steStyle::bg_color() const {
   Initialize(steStyle::bg_color);
   return (bg_clr);
}

/******************************************
 * steStyle::cursor_fg_color(const char*) *
 ******************************************/

void steStyle::cursor_fg_color(const char* c) {
   Initialize(steStyle::cursor_fg_color);
   strcpy(cursor_fg_clr, c);
   return;
}


/******************************************
 * steStyle::cursor_bg_color(const char*) *
 ******************************************/

void steStyle::cursor_bg_color(const char* c) {
   Initialize(steStyle::cursor_bg_color);
   strcpy(cursor_bg_clr, c);
   return;
}


/*******************************
 * steStyle::name(const char*) *
 *******************************/

void steStyle::name(const char* nm) {
   Initialize(steStyle::name);
   char* p = new char[strlen(nm) + 1];
   strcpy(p, nm);
   delete style_name;
   style_name = p;
   return;
}
/******************
* steStyle::get_name();
******************/
char *steStyle::get_name() const {
 Initialize(steStyle::get_name);
#if 0
   char *my_str = new char [strlen(style_name) + 1];
   strcpy(my_str, style_name);
#endif
 return (char *)name();
}

/********************
 * steStyle::name() *
 ********************/

const char* steStyle::name() const {
   Initialize(steStyle::name);
   return (style_name);
}

// Write the style in a form usable by the constructor


steAppStylePtr st_table = NULL;

/*****************************************************
 * steStyle::make_absolute_style_for(commonTreePtr) *
 *****************************************************/

steStyle* steStyle::make_absolute_style_for(commonTreePtr tn) {
   Initialize(steStyle::make_absolute_style_for);
   steStyle* absolute = new steStyle("", NULL); // not persistent => no db_new
   int level = -1;
   st_table = get_node_style_table(checked_cast(commonTree,tn));
   absolute->compose_style(tn, level);
   if( ste_get_node_format_mode(tn) ){   // switch to default Page Format
       absolute->value(LEFT_MARGIN,72.0);
       absolute->specified(RIGHT_MARGIN,false);
       absolute->value(FIRST_LINE_INDENT,0.0);
       absolute->value(SPACE_BEFORE, (absolute->point_value(FONT))*(-1));
       absolute->value(JUSTIFIED_OR_CENTERED,RAGGED_RIGHT);
   }
   return (absolute);
}

// The style values to be applied at a given level are computed by
// recursively walking the tree of text nodes back to the root and then
// applying the appropriate partial styles as the recursion unwinds.

/******************************************************************
 * steStyle::compose_style(commonTreePtr, int&) *
 ******************************************************************/

void steStyle::compose_style(commonTreePtr tn, int& targ_level) {
   Initialize(steStyle::compose_style);

   int type1;
   extern steViewPtr ste_current_view;
   commonTreePtr parent = checked_cast(commonTree, tn->get_parent());
   steNodeType tn_type = (steNodeType) tn->get_node_type();
   steSlotPtr sl = get_node_style( tn );
   steStylePtr own_st = NULL;
   if ( sl )
       own_st = checked_cast(steStyle,sl->get_slot_obj());

   if (tn_type == steSTR)
      targ_level++;
   else if (tn_type == steHEAD)
      type = TITLE_LEVEL_STYLE;
   else if (tn_type == stePAR)
      type = PARAGRAPH_LEVEL_STYLE;

   int my_level = targ_level;
   if ( parent )
      compose_style(parent, targ_level);
   else {
      steStylePtr def_st = st_table->get_default_style();
      incorporate(def_st, tn);
   }
   if ( ste_is_splice_node_style( tn ) )
      return;
   if (tn_type == steSTR) {
      steStylePtr str_st = st_table->get_style(targ_level - my_level, type);
      incorporate(str_st, tn);
   }else{
        type1 = ste_get_table_item_type( tn );
        if ( type1 != 0 ){
           steStylePtr str_st = st_table->get_style( 1, (ste_style_type)type1);
           incorporate(str_st, tn);
// hardcoded first_indent to tabs
            if ( type1 == ZERO_ROW_STYLE || type1 == DEFAULT_ITEM_STYLE){
                 int tab_indent = ste_table_get_column_index( tn );
                 value(FIRST_LINE_INDENT, -(18.0*tab_indent) );
            }
//hardcoded spacing before
           if ( type1 == ZERO_COLUMN_STYLE ){
                steColumnNodePtr cc;
                if ( is_steTextNode( tn ) )
                     cc = checked_cast(steColumnNode,tn);
                else
                     cc = checked_cast(steColumnNode,(checked_cast(ldrTree,tn))->get_appTree());
                
                steTableNodePtr ttt = cc->get_table();  
                int lines = ttt->get_spacing();                
                value(SPACE_BEFORE, (point_value(FONT))*lines);
           }
        }
        type1 = ste_get_smt_title_type( tn );
        if ( type1 != 0 ){
             int style_in = 1;   //default for not opened SMT so far hardcoded
             if ( ste_current_view && ste_current_view->style_index > 0 )
                  style_in = ste_current_view->style_index;
             switch ( style_in ){
             case STE_INDENTED :
                  value(SPACE_BEFORE, 0.0);
                  value(FIRST_LINE_INDENT, 18.0);
                  break;
             case STE_EXDENTED :
                  value(SPACE_BEFORE, 0.0);
                  value(FIRST_LINE_INDENT, 0.0);
                  break;
             case STE_PASTED   :
                  if ( type1 == 1 ){ // start brace
                      value(LEFT_MARGIN, 72.0);
                      value(FIRST_LINE_INDENT, 0.0);
                      value(SPACE_BEFORE, (point_value(FONT))*(-1));
                  }else{
                      value(SPACE_BEFORE, 0.0);
                      value(FIRST_LINE_INDENT, 0.0);
                  }
                  break;
             default : break;
             }
        }
        type1 = ste_get_smt_clause_style( tn );
        if (type1 != 0)
            value(SPACE_BEFORE, 0.0);

        type1 = ste_get_smt_comment_type( tn );
        if ( type1 > 0 ){
           steStylePtr com_st = st_table->get_style( 1, (ste_style_type)type1);
           incorporate(com_st, tn);
           if ( type1 == INLINE_COMMENT_STYLE )
                value(SPACE_BEFORE, (point_value(FONT))*(-1));
        }
   }
   if (own_st)
      incorporate(own_st, tn);

   if ( is_first_structured_title( tn ) )
         value(SPACE_BEFORE, (point_value(FONT))*(-1));
   return;
}

void steStyle::set_defaults(){
 Initialize(steStyle::set_defaults);
      for (int i = FIRST_FORMAT_CONTROL; i < NUM_FORMAT_CONTROLS; i++) {
         ste_format_control c = ste_format_control(i);
         value(c, default_value[c]);
         spacing(c, default_spacing[c]);
      }
}

void steStyle::incorp(steStylePtr style, commonTreePtr tn) {
   incorporate(style, tn);
}

// Styles found in the recursive walk above are applied to the resulting
// style on a value-by-value basis.

/******************************************************
 * steStyle::incorporate(steStylePtr, commonTreePtr) *
 ******************************************************/

void steStyle::incorporate(steStylePtr style, commonTreePtr tn) {
   Initialize(steStyle::incorporate);
   if (!style)
      return;

   for (int i = FIRST_FORMAT_CONTROL; i < NUM_FORMAT_CONTROLS; i++) {
      ste_format_control c = (ste_format_control) i;
      if (c != LEFT_MARGIN && c != RIGHT_MARGIN && style->specified(c)) {
         if ( c == CHAR_ATTRIBUTES && specified(c) )
            values[i] = (int) values[i] | (int) style->values[i];
         else
            values[i] = style->values[i];
      
         spacing_units[i] = style->spacing_units[i];
         specified(c, true);
      }
   }
   if (style->specified(ENUMERATION)) {
      separator = style->separator;
      terminator = style->terminator;
   }

   if (*style->font_name)
      strcpy(font_name, style->font_name);
   if (*style->fg_clr)
      strcpy(fg_clr, style->fg_clr);
   if (*style->bg_clr)
      strcpy(bg_clr, style->bg_clr);
   if (*style->cursor_fg_clr)
      strcpy(cursor_fg_clr, style->cursor_fg_clr);
   if (*style->cursor_bg_clr)
      strcpy(cursor_bg_clr, style->cursor_bg_clr);

// If the left and right margins are sibling-relative, we must loop until
// we find the appropriate sibling.

   for (;;) {
      if (style->specified(LEFT_MARGIN)) {
	 switch (style->margin_base) {
	 case PAGE_RELATIVE:
	    values[LEFT_MARGIN] = style->values[LEFT_MARGIN];
	    values[RIGHT_MARGIN] = style->values[RIGHT_MARGIN];
            spacing_units[LEFT_MARGIN] = style->spacing_units[LEFT_MARGIN];
            spacing_units[RIGHT_MARGIN] = style->spacing_units[RIGHT_MARGIN];
            specified(LEFT_MARGIN, true);
            specified(RIGHT_MARGIN, true);
	    break;

	 case PARENT_RELATIVE:
	    values[LEFT_MARGIN] += style->values[LEFT_MARGIN];
	    values[RIGHT_MARGIN] += style->values[RIGHT_MARGIN];
            specified(LEFT_MARGIN, true);
            specified(RIGHT_MARGIN, true);
	    break;

	 case SIBLING_RELATIVE:
	    tn = checked_cast(commonTree, tn->get_prev());
	    while (tn) {
	       style = checked_cast(steStyle, 
                    get_node_character_style(checked_cast(commonTree, tn)));
	       if (style && style->specified(LEFT_MARGIN))
		  break;
               tn = checked_cast(commonTree, tn->get_prev());
	    }
	    if (tn)
	       continue;
	    break;
	 }
	 break;
      }
      break;
   }
   return;
}

/*
   START-LOG-------------------------------------------

   $Log: styStyle.cxx  $
   Revision 1.8 2002/03/04 17:25:57EST ktrans 
   Change M/C/D/W/S macros to enums
 * Revision 1.8  1994/08/05  19:10:17  boris
 * Bug track: 8020, 7962
 * Fixed memory problems on big (huge) report generation
 *
 * Revision 1.7  1993/06/05  00:48:06  boris
 * Added icon support
 *
 * Revision 1.6  1993/04/18  15:24:05  boris
 * Fixed crash in stySTyle constructor
 *
 * Revision 1.5  1993/04/17  20:38:06  smit
 * Fix purify problem.
 *
 * Revision 1.4  1993/02/04  18:49:55  boris
 * fixed bug #2372 (memory leak)
 *
 * Revision 1.3  1993/01/04  21:07:46  jon
 * Fixed empty styStyle constructor.
 *
 * Revision 1.2  1992/12/18  19:05:09  glenn
 * Transferred from STE
 *
Revision 1.2.1.7  1992/12/01  22:53:04  boris
Fixed CHAR_ATTRIBUTES overlapping on printing

Revision 1.2.1.6  1992/11/25  23:23:32  boris
Fixed changed Default Style printing crash

Revision 1.2.1.5  1992/11/25  14:59:54  boris
Added Raw/Smode print facilities

Revision 1.2.1.4  1992/11/22  03:15:45  builder
typesafe casts.

Revision 1.2.1.3  1992/11/20  19:36:32  boris
Added Decorate Source facilities

Revision 1.2.1.2  1992/10/09  19:48:37  boris
Fix comments



   END-LOG---------------------------------------------

*/

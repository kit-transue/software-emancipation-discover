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
#ifndef _gra_ivSymbol_h
#define _gra_ivSymbol_h

//------------------------------------------
// graSymbol
//------------------------------------------
// synopsis:
// Header file for gra parametric symbols
//------------------------------------------

#ifndef _general_h
#include <general.h>
#endif

#ifndef picture_h
#include <InterViews/enter-scope.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>
#endif

#ifndef _graEnums_h
#include <graEnums.h>
#endif

#include <graInterface.h>

#ifndef _XtIntrinsic_h
typedef struct _WidgetRec *Widget;
#endif

class iv3(TextBuffer);
class iv3(Rubberband);
class Line;
class rubberGraphic;

class Obj;
class graWindow;
class viewSymbolNode;
class viewConnectionNode;
class viewSelection;
class viewSymbolNode;


//------------------------------------------
// graAttachableSymbol
//------------------------------------------

class graAttachableSymbol : public Picture
{
  public:
    graAttachableSymbol();
    ~graAttachableSymbol();

    void get_attachment_point(graAttachmentPoint, int& x, int& y);

  protected:
    int x[graNumAttachmentPoints];
    int y[graNumAttachmentPoints];

    void clear_attachment_points();

friend class graWorld;
};


//------------------------------------------
// graParametricSymbol
//------------------------------------------

class graParametricSymbol : public graAttachableSymbol
{
  public:
    graParametricSymbol(int symbol, int layers);
    graParametricSymbol(int symbol, int layers, char*,
			graTextPositions = graTextModeUnknown);
    ~graParametricSymbol();

    void get_symbol_size(float& cx, float& cy, float& w, float& h,
			 bool include_text = true );

    iv3(Rubberband)* get_rubband();

    virtual void set_layers(int);
    virtual void add_layers(int);
    virtual void rem_layers(int);
    int get_layers();

    virtual void find_insertion( 
	graWindow* window,
	Graphic** selections, int num_selected, viewSelection*);

    viewSymbolNode* get_viewnode() { return viewnode; }
    void set_viewnode(viewSymbolNode *vn) { viewnode = vn; }

    void set_position(float cx, float cy);
    void set_scale(float sx, float sy);

    int  get_text_width();
    void set_text_width(int);

    virtual void open_node(graWindow* window);	// opens up symbol node

    int  has_text_focus() { return text_focus; }
    void set_text_focus(int p_focus);

    int  get_char_marker() { return char_marker; }
    void set_char_marker(int pos);

    int  handle_text_input(unsigned char *string, int len,
			   int control, int meta, int shift, int shiftlock);

    void replace_text(char*);

    int  locate_text_position(int x, int y);

    void shift_cursor(int pos);

    int  report_text_update();

    Obj* get_modified_token() { return modified_token; }

    void hilite_text(int start_pos, int end_pos);
    void unhilite_text(int pos);
    void unhilite_all();
    void hilite_modification_text();
    void set_popup_item_sensitive(char*, int);
    void add_bottom_line();
    void remove_bottom_line();

    enum { FALSE_BOTTOM_HEIGHT = 8 };

    graParametricSymbol* create_false_bottom();
    void remove_false_bottom(graParametricSymbol*);

    virtual void do_highlight(iv3(Canvas)*, rubberGraphic*);
    virtual void undo_highlight(iv3(Canvas)*);

    NodePopupInfo *generate_popup();

  protected:
    void shift_cursor_left ();
    void shift_cursor_right ();

    Obj* get_token_to_be_modified();	// Inquires SMT to get token
    void set_modified_token();		// Sets modification housekeeping
    void reset_modified_token();	// resets modification housekeeping
    void set_modification_start_end_pos(Obj* modified_token);

  private:
    int			symbol;
    Picture*		param_overlay[GRA_MAX_MODES];
    graSymbolModes	layers;
    graTextPositions	text_mode;
    int 		text_width, text_height;
    viewSymbolNode*	viewnode;
    int  		text_focus;
    int			char_marker;	// insertion position
    char*		buffer;		// Buffer space holder for editbuffer
    iv3(TextBuffer)*	editbuffer;	// edit buffer for the text

    Obj*	modified_token;		// token that was modified
    int		modification_start;	// start of modified string
    int		modification_end;	// end of modified string

    Widget	popup_menu;		// popup menu widget

    Graphic*	bottom_line;
    Graphic*	false_bottom1;
    Graphic*	false_bottom2;

    void	create_label_box(int mode_no);

    inline graAttachableSymbol* get_pictel(int layer);
};


//------------------------------------------
// graConnectionSymbol
//------------------------------------------

class graConnectionSymbol : public graParametricSymbol
{
  public:
    graConnectionSymbol(viewConnectionNode&);

    virtual void find_insertion( 
	graWindow* window,
	Graphic**selections, int num_selected, viewSelection*);
    virtual void set_layers(int);
    virtual void add_layers(int);
    virtual void rem_layers(int);
    virtual void regenerate(viewConnectionNode&);

  protected:
    int num_segments;

    struct _segments
    {
	Line* line;
	viewSymbolNode* viewnode;
    }
    *segments;

    graConnectionSymbol();
    ~graConnectionSymbol();
};


//------------------------------------------
// graFlexConn
//------------------------------------------

enum deco_type
{
    NOTHING,
    SOLID_DOT,
    HOLLOW_DOT,
    SINGLE_ARROW,
    DOUBLE_ARROW,
    SINGLE_BAR,
    DOUBLE_BAR,
    TRIANGLE,
    SEMICIRCLE,
    DIAMOND,
    CHICKENFOOT,
    CIRCLE_BAR,
    BAR_CHICKENFOOT,
    CIRCLE_CHICKENFOOT
};


class logical_segment;

class graFlexConn: public graConnectionSymbol
{
  public:
    graFlexConn(viewConnectionNode*);
    ~graFlexConn();

    virtual void find_insertion(graWindow*, Graphic**, int, viewSelection*);
    virtual void regenerate(viewConnectionNode&);
    virtual void do_highlight(iv3(Canvas)*, rubberGraphic*);
    virtual void undo_highlight(iv3(Canvas)*);

    void add_vertex(int seg_num, int x, int y);
    void reinit_seg(int seg_num);
    void set_decoration(int seg_num, deco_type);
    void set_label(int seg_num, const char*);
    void get_label_size(int seg_num, float& x, float& y);
    void set_label_loc(int seg_num, bool centered, bool above_or_right);

  private:
    int num_log_segs;
    logical_segment* log_segs;
    bool is_highlighted;

    void add_decoration(logical_segment&, PColor*, PColor*, bool, bool);
    void add_label(logical_segment&);
};


/*
//------------------------------------------
$Log: gra_ivSymbol.h  $
Revision 1.1 1996/10/17 14:05:44EDT azaparov 

 * Revision 1.2.1.4  1993/04/26  18:00:57  glenn
 * Add dtor for graAttachableSymbol and graConnectionSymbol.
 *
// Revision 1.42  92/09/23  15:24:26  jon
// Added member function get_layers
// 
// Revision 1.41  92/08/26  09:22:38  jon
// Added method generate_popup and removed method context_popup
// Bug #639
// 
// Revision 1.40  92/07/29  23:47:17  jon
// InterViews scope fix
// 
// Revision 1.39  92/07/15  08:53:46  wmm
// Add false_bottom tracking so set_layers can avoid deleting them.
// 
// Revision 1.38  92/06/25  20:26:59  aharlap
// moved enum outside of class for GNU
// 
// Revision 1.37  92/06/11  15:21:41  jont
// changed bool has_bottom_line to Graphic* bottom_line,
// added remove_bottom_line method
// 
// Revision 1.36  92/06/05  11:10:14  wmm
// Fix crash switching to modified ERD by making false bottoms part of the
// picture with which they are associated.
// 
// Revision 1.35  92/05/08  08:23:09  wmm
// Add graFlexConn::is_highlighted to allow for efficiency when highlighting
// is repeatedly turned on from graWorld::flush_repair().
// 
// Revision 1.34  92/04/17  11:23:19  wmm
// Enable drawing arrows on ERD relation connectors.
// 
// Revision 1.33  92/04/16  08:29:37  glenn
// Do not include generic-popup-menu.h or rubberGraphic.h.
// Add forward decl of rubberGraphic and minimal decl of Widget.
// 
// Revision 1.32  92/04/15  12:41:48  jont
// included generic-popup-menu.h
// 
// Revision 1.31  92/04/15  10:06:22  wmm
// Fix problem in which graFlexConn highlighting still occasionally resulted in
// use of the rubber band box.
// 
// Revision 1.30  92/04/15  08:01:38  wmm
// Change popup_menu from void* to Widget.
// 
// Revision 1.29  92/04/09  18:49:51  wmm
// Add virtual function allowing symbols to perform highlighting themselves
// instead of relying on the graWindow::highlight_box function.
// 
// Revision 1.28  92/03/27  19:45:32  wmm
// Redo false bottoms for ERD symbols.
// 
// Revision 1.27  92/03/27  11:08:24  glenn
// Move enums into graEnums.h
// Update include flag macros.
// 
// Revision 1.26  92/03/26  17:00:08  wmm
// Support ERDs.
// 
// Revision 1.24  92/02/28  17:01:45  smit
// changed declaration of a member function.
// 
//Revision 1.23  92/02/26  15:14:23  wmm
//Oh, well.  Leave nil defined (glenn).
//
//Revision 1.22  92/02/26  12:38:19  glenn
//Undefine nil after InterViews include files.
//
//Revision 1.21  92/02/25  21:23:49  builder
//fix bugs with include files.
//
//Revision 1.20  92/02/25  20:59:04  glenn
//Remove include files that are only needed in the .C file.
//
//Revision 1.19  92/02/24  12:36:33  jont
//popup item sensitive
//
//Revision 1.18  92/02/06  16:46:52  jont
// *** empty log message ***
//
//Revision 1.17  92/01/30  09:30:15  smit
//Change declaration
//
//Revision 1.16  92/01/24  16:45:41  glenn
//Define get_pictel as an inline function (replaces a macro).
//
//Revision 1.15  92/01/16  21:52:50  smit
//Add hilited text stuff.
//
//Revision 1.14  92/01/10  18:33:20  smit
//Make hilited and selected layer at the top.
//
//Revision 1.13  92/01/07  18:18:03  smit
// *** empty log message ***
//
//Revision 1.12  92/01/06  13:47:08  smit
//Support token editing.
//
//Revision 1.11  92/01/02  18:16:57  smit
//Added editing of text in diagram view.
//
//Revision 1.10  91/12/31  12:39:58  smit
//Added two new methods.
//
//Revision 1.9  91/12/26  18:22:44  smit
//Added bunch of things for text input
//
//Revision 1.8  91/12/24  17:22:51  smit
//Added new variables and methods
//
//Revision 1.7  91/12/17  15:42:56  smit
//Added new virtual function.
//
//Revision 1.6  91/12/13  13:27:39  wmm
//Add ability to get and set text width for multi-connection view routing.
//
//Revision 1.5  91/12/11  15:14:02  wmm
//Add parameter controlling whether the label text is included in the size of a
//symbol (needed for flowchart and OODT routers).
//
//Revision 1.4  91/12/09  10:29:10  wmm
//Add support for new ldr connection types (parametric attachment points),
//symbolic layer number constants.
//
//Revision 1.3  91/12/04  10:10:17  wmm
//Support OODT graphic symbols.
//
//Revision 1.2  91/10/25  14:57:32  smit
//Make text in graphics view change color when selected.
//
//Revision 1.1  91/09/01  19:32:14  kws
//Initial revision
//
//------------------------------------------
*/

#endif // _graSymbol_h

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
#ifndef _gra_galaSymbol_h
#define _gra_galaSymbol_h

//------------------------------------------
// gra_galaSymbol
//------------------------------------------
// synopsis:
// Header file for gra parametric symbols
//------------------------------------------

#include <general.h>
#include <graEnums.h>
#include <graInterface.h>

class Obj;
class graWindow;
class viewSymbolNode;
class viewConnectionNode;
class viewSelection;
class viewSymbolNode;

class vdomain;
class vdomainObject;
class vrect;
class vrectLong;
class vfont;

enum graDrawCommands {
    gra_moveto,
    gra_lineto,
    gra_stroke,
    gra_fill,
    gra_color,
    gra_rect,
    gra_circle,
    gra_ellipse,
    gra_linewidth,
    gra_image,
    gra_end
};

enum graSymbolType {
    graAttachableSymbolType,
    graParametricSymbolType,
    graConnectionSymbolType,
    graFlexConnType
};

class graDrawingCommands {
  public:
    graDrawingCommands();
    
    void SetDrawingCmds(int *cmds);
    void Execute(int x, int y, float sx, float sy);
    void GetBounds(vrectLong &rect, float sx, float sy);
    int  ExecutePointNearCheck(int x, int y, float sx, float sy, int px, int py, int distance);
    int  GetCurrentCmdIndex();


    void SetColors(int r, int g, int b);
    void AddLine(int x1, int y1, int x2, int y2);
    void AddCircle(int x, int y, int r);
    void AddMoveto(int x, int y);
    void AddLineto(int x, int y);
    void AddRect(int x, int y, int w, int h);
    void AddEllipse(int x, int y, int w, int h);
    void AddLineWidth(int lw);
    void AddStroke(void);
    void AddFill(void);
    void AddCommands(graDrawingCommands *cmds);
    void AddImage(int index);
    void ClearCommands(void);
    void SetHighlited(int highlite);

  private:
    void do_moveto(int &index);
    void do_lineto(int &index);
    void do_color(int &index);
    void do_stroke(int &index);
    void do_fill(int &index);
    void do_circle(int &index);
    void do_ellipse(int &index);
    void do_linewidth(int &index);
    void do_image(int &index);

    void do_movetoxy(int &index);
    void do_linetoxy(int &index);
    void do_colorxy(int &index);
    void do_strokexy(int &index);
    void do_fillxy(int &index);
    void do_circlexy(int &index);
    void do_ellipsexy(int &index);
    void do_linewidthxy(int &index);
    void do_imagexy(int &index);

    int  CheckPoint(int px, int py, int distance);
    void UpdateBounds(int x, int y, int w, int h);
    void grow(int len);
    
    int   *drawing_commands;
    int   cmd_size;
    int   last_cmd_pos;
    float scalex;
    float scaley;
    int   xoffset;
    int   yoffset;
    int   bounds_rect_x, bounds_rect_y;
    int   bounds_rect_w, bounds_rect_h;
    int   x1, y1, x2, y2;
    int   highlited;
};


//------------------------------------------
// graAttachableSymbol
//------------------------------------------

class graAttachableSymbol : public graDrawingCommands
{
  public:
    graAttachableSymbol();
    ~graAttachableSymbol();

    void    get_attachment_point(graAttachmentPoint, int& x, int& y);
    virtual graSymbolType SymbolType(void);
    

    int x[graNumAttachmentPoints];
    int y[graNumAttachmentPoints];
    
  protected:
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

    virtual void set_layers(int);
    virtual void add_layers(int);
    virtual void rem_layers(int);
    virtual void do_highlight(void);
    virtual void undo_highlight(void);
    int get_layers();

    viewSymbolNode* get_viewnode() { return viewnode; }
    void set_viewnode(viewSymbolNode *vn) { viewnode = vn; }

    void set_scale(float sx, float sy);

    int  get_text_width();
    void set_text_width(int);

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

    NodePopupInfo *generate_popup();

    virtual void          find_insertion(int gra_cmd_index, viewSelection* selection);
    virtual void          set_position(float cx, float cy);
    virtual void          Draw(int& cx, int& cy, int& w, int& h, bool include_text);
    virtual void          GetPosition(int &x, int &y, int &w, int &h);
    virtual void          MoveDX(int dx);
    virtual void          MoveDY(int dy);
    virtual graSymbolType SymbolType(void);
    virtual int           PointNear(int px, int py, int distance);
    virtual void          SetDomainAndObject(vdomain *domain, vdomainObject *obj);
    virtual void          DrawSelected(int& cx, int& cy, int& w, int& h, bool include_text);
    virtual void          SetObjectBounds(vrectLong *rect);

    void          SetFont(vfont *f);
    void          DrawTextRight(vrectLong& bounds);
    void          DrawTextPositioned(vrectLong& bounds);
    void          DrawTextInside(vrectLong& bounds);
    void          DrawDecoration(graLayerNo layer, int x, int y);
    void          DrawTextBox(int xoffset, int yoffset);
    void          GetTextBoxBounds(vrectLong &rect);
    int           TextBoxContainsPoint(int px, int py);
    int           DecorationPointTextRight(int px, int py);
    int           DecorationPointTextPositioned(int px, int py);
    int           DecorationPointTextInside(int px, int py);
    int           DecorationPoint(int px, int py);
    vdomainObject *GetDomainObject();


  protected:
    void shift_cursor_left ();
    void shift_cursor_right ();

    Obj* get_token_to_be_modified();	// Inquires SMT to get token
    void set_modified_token();		// Sets modification housekeeping
    void reset_modified_token();	// resets modification housekeeping
    void set_modification_start_end_pos(Obj* modified_token);

  protected:
    int			symbol;
    graDrawingCommands  *param_overlay[GRA_MAX_MODES];
    graParametricSymbol *false_bottom1;
    graParametricSymbol *false_bottom2;
    vdomainObject       *domain_object;
    vdomain             *domain;
    graSymbolModes	layers;
    graTextPositions	text_mode;
    int 		text_width, text_height;
    viewSymbolNode*	viewnode;
    int  		text_focus;
    int			char_marker;	// insertion position
    char*		buffer;		// Buffer space holder for editbuffer

    Obj*	modified_token;		// token that was modified
    int		modification_start;	// start of modified string
    int		modification_end;	// end of modified string
    int         x, y;
    int         labelbox_x, labelbox_y;
    int         labelbox_w, labelbox_h;
    float       scalex, scaley;
    vfont       *font;
    char        *text;

    void	create_label_box(int mode_no);

    inline graAttachableSymbol* get_pictel(int layer);
};


//------------------------------------------
// graConnectionSymbol
//------------------------------------------

class GraphPath { 
  public:
    GraphPath(int i1, int i2) { cmd_start = i1; cmd_end = i2; };
    int cmd_start;
    int cmd_end;
};

class graConnectionSymbol : public graParametricSymbol
{
  public:
    graConnectionSymbol(viewConnectionNode&);

    virtual void set_layers(int);
    virtual void add_layers(int);
    virtual void rem_layers(int);
    virtual void regenerate(viewConnectionNode&);

    virtual void          find_insertion(int gra_cmd_index, viewSelection* selection);
    virtual void          set_position(float cx, float cy);
    virtual void          Draw(int& cx, int& cy, int& w, int& h, bool include_text);
    virtual void          GetPosition(int &x, int &y, int &w, int &h);
    virtual void          MoveDX(int dx);
    virtual void          MoveDY(int dy);
    virtual graSymbolType SymbolType(void);
    virtual void          SetDomainAndObject(vdomain *domain, vdomainObject *obj);

    int num_segments;

    struct _segments
    {
	GraphPath     * path;
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

    virtual void          Draw(int& cx, int& cy, int& w, int& h, bool include_text);
    virtual void          GetPosition(int &x, int &y, int &w, int &h);
    virtual void          MoveDX(int dx);
    virtual void          MoveDY(int dy);
    virtual graSymbolType SymbolType(void);
    virtual void          regenerate(viewConnectionNode&);
    virtual void          do_highlight(void);
    virtual void          undo_highlight(void);

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

    void add_label(logical_segment&);
    void add_decoration(logical_segment&, int r, int g, int b, bool, bool);
};


#endif // _gra_ivSymbol_h

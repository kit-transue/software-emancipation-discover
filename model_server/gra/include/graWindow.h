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
#ifndef _graWindow_h
#define _graWindow_h

#if !defined(NEW_UI)

// graWindow
//------------------------------------------
// synopsis:
// Graphical view window
//------------------------------------------

#include <InterViews/enter-scope.h>
#include <InterViews/scene.h>
#include <InterViews/Graphic/grblock.h>
#include <InterViews/leave-scope.h>
#include "ivfix.h"

#include <general.h>

class iv3(Tray);
class iv3(MonoScene);
class iv3(Scene);
class iv3(Sensor);
class Picture;
class Damage;
class graParametricSymbol;
class viewGraHeader;

typedef union _XEvent* XEventPtr;

enum graWindow_Mode		// formerly just called "Mode".
{
    in_select_modal,
    in_pan_modal,
    in_zoom_modal,

    // Used by ERD and OODT for drag'n'drop relation definition.
    in_1_1_relation,
    in_1_many_relation,
    in_many_1_relation,
    in_many_many_relation
};


//------------------------------------------
// graScene
//------------------------------------------

class graWindow;

class graScene : public iv3(MonoScene)
{
  public:
    enum sliderStyle
    {
	SLIDER_NOTSHOWN,
	SLIDER_TOPLEFT,
	SLIDER_TOPRIGHT,
	SLIDER_BOTTOMLEFT,
	SLIDER_BOTTOMRIGHT
    };

    graScene(viewGraHeader*, Picture*, iv3(Sensor)*);
    graWindow* get_window() { return window; }

    void slider_style(sliderStyle);
    int slider_style() { return slider_style_; }

  private:
    graWindow* window;
    iv3(Tray)* tray;
    iv3(Scene)* slider_scene;
    sliderStyle slider_style_;
};

//------------------------------------------
// graWindow
//------------------------------------------

typedef struct _WidgetRec* Widget;

class rubberGraphic;

class graWindow : public GraphicBlock
{
  public:
    static void flush_all(void*);

    graWindow(graScene*, viewGraHeader*, Picture*, iv3(Sensor)*);
    ~graWindow();

    virtual void Handle( iv3(Event)&  );
    virtual void Update();
    virtual void Draw();
    virtual void Redraw(iv2(Coord), iv2(Coord), iv2(Coord), iv2(Coord));
    virtual void Resize();

    boolean size(int & width, int & height);

    void refresh();

    void  set_magnification(float);
    void  damage_area( Graphic* graphic );
    void  repair_damage();
    void  flush_repair ();
    void  repaint_window();
    void  insert_symbol(Picture*);
    void  remove_symbol(Picture*);
    void  dialog_scrollbars();
    void* get_view_header_window();
    void  set_widget(unsigned long x_window);
    int getymax() {return ymax;}
    void  set_mode (graWindow_Mode);
    graWindow_Mode  get_mode ();
    int   slider_style() { return scene->slider_style(); }
    void  slider_style(int s) { scene->slider_style(graScene::sliderStyle(s)); }

    void set_text_focus(graParametricSymbol*);
    void remove_text_focus();
    graParametricSymbol* get_text_focus () { return focus_symbol; }
    graScene* get_scene() { return scene;}

    graParametricSymbol* find_symbol(XEventPtr);

    void highlight(graParametricSymbol*);
    void unhighlight(graParametricSymbol*);
    void unhighlight();
    void recenter(int x, int y);
    void auto_scroll(int x, int y);
    void text_input(const char*, int);

    // X Intrinsics Actions

    void focus_in(XEventPtr, char**, int);
    void focus_out(XEventPtr, char**, int);
    void set_highlight(XEventPtr, char**, int);
    void clear_highlight(XEventPtr, char**, int);
    void open_node(XEventPtr, char**, int);
    void collapse_expand(XEventPtr, char**, int);
    void pickup(XEventPtr, char**, int);
    void carry(XEventPtr, char**, int);
    void drop(XEventPtr, char**, int);
    void zoom_rectangle(XEventPtr, char**, int);
    void recenter(XEventPtr, char**, int);
    void text_input(XEventPtr, char**, int);
    void select(XEventPtr, char**, int);
    void symbol_menu(XEventPtr, char**, int);
    void modal_click(XEventPtr, char**, int);
    void begin_connection(XEventPtr, char**, int);
    void reset_Drawn();
    bool Drawn() const;
    void end_connection(XEventPtr, char**, int);
 Widget get_widget() { return widget;}
  private:
    static rubberGraphic* xor_box;

    Picture* picture;
    Damage* damage;
    graParametricSymbol* focus_symbol;
    graParametricSymbol* highlight_symbol;
    viewGraHeader* view_header;
    graScene* scene;
    Widget widget;
    graWindow_Mode   modal_value;
    bool did_Draw;

    void highlight_box(graParametricSymbol*);
};

extern int gra_type();

#elif defined(NEW_UI)

class graWindow 
{
  public:
    static void flush_all(void*);
    void   refresh();
    void   damage_area(void *) {} ; // do nothing now.
};

#endif    // !defined(NEW_UI)

#endif


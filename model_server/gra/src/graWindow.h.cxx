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
// graWindow
//------------------------------------------
// synopsis:
// Implements a graphical view window
//
//------------------------------------------

// INCLUDE FILES

#include <Object.h>
#include <genError.h>

#include <InterViews/enter-scope.h>
#include <IV-X11/xevent.h>
#include <InterViews/event.h>
#include <InterViews/shape.h>
#include <InterViews/perspective.h>
#include <InterViews/canvas.h>
#include <InterViews/Graphic/damage.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/lines.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>

#include <driver.h>
#include <dialog.h>
#include <graWindow.h>
#include <graSymbol.h>
#include <viewNode.h>
#include <viewGraHeader.h>
#include <button.h>
#include <graTools.h>
#include <graAction.h>
#include <rubberGraphic.h>


// EXTERNAL FUNCTIONS

void gra_init_actions();
static int comp_perspective( iv3(Perspective) * ps1, iv3(Perspective) * ps2);

//------------------------------------------
// Constructor graScene
//
// This window is an InterViews object that coordinates the layout of a
// graWindow and a Slider.  The Slider is used to alter the Perspective of
// the graWindow.
//------------------------------------------

graScene::graScene( viewGraHeader *view_header, Picture *pic, iv3(Sensor) *sens )
{
    Propagate(false);
    window = new graWindow(this, view_header, pic, sens);
    Insert(window);

#if 0
    const int SLIDER_SIZE = round(inch * 0.8);
    const int FRAME_THICKNESS = 2;

    slider_style_ = SLIDER_NOTSHOWN;

    // Create a slider, but not for text-only windows.
    if(view_header->get_type() != Rep_ListText)
    {
	iv3(Slider)* slider = new iv3(Slider)(window);
	slider->GetShape()->Rect(SLIDER_SIZE, SLIDER_SIZE);

	slider_scene = new iv3(Frame)(slider, FRAME_THICKNESS);
	slider_scene->Propagate(false);
	slider_scene->GetShape()->Rect(SLIDER_SIZE, SLIDER_SIZE);
	slider_scene->GetShape()->Rigid();

	tray = new iv3(Tray);
	tray->iv3(HBox)(tray, window, tray);
	tray->iv3(VBox)(tray, window, tray);

	slider_style(SLIDER_BOTTOMRIGHT);

	Insert(tray);
    }
    else
    {
	slider_scene = tray = NULL;
	Insert(window);
    }
#endif
}

void graScene::slider_style(sliderStyle /*style*/)
{
#if 0
    Initialize(graScene::slider_style);

    if(slider_scene && tray && (slider_style_ != style))
    {
	if(slider_style_ != SLIDER_NOTSHOWN)
	    tray->Remove(slider_scene);

	switch(slider_style_ = style)
	{
	  case SLIDER_NOTSHOWN:
	    return;

	  case SLIDER_TOPLEFT:
	    tray->iv3(Align)(Top, slider_scene, window);
	    tray->iv3(Align)(Left, slider_scene, window);
	    break;

	  case SLIDER_TOPRIGHT:
	    tray->iv3(Align)(Top, slider_scene, window);
	    tray->iv3(Align)(Right, slider_scene, window);
	    break;

	  case SLIDER_BOTTOMLEFT:
	    tray->iv3(Align)(Bottom, slider_scene, window);
	    tray->iv3(Align)(Left, slider_scene, window);
	    break;

	  case SLIDER_BOTTOMRIGHT:
	    tray->iv3(Align)(Bottom, slider_scene, window);
	    tray->iv3(Align)(Right, slider_scene, window);
	    break;
	}
	tray->Change(slider_scene);
    }
#endif
}

//------------------------------------------
// Constructor graWindow
//
// This constructor creates a logical window which to paint graphics into
// This window can be inserted into the world and realized on the screen.
// This window is based in the interviews graphics block.  Therefore a
// perspective can be maintained. Also a damage system is implimented here
// to remember damaged areas for repainting.
//------------------------------------------

static const int MARGIN = 5;
static objArr_Int graWindows;

graWindow::graWindow(graScene* gs, viewGraHeader* v, Picture* p, iv3(Sensor)* s)
: GraphicBlock(s, p, MARGIN,
	       (v->get_type() == Rep_MultiConnection ||
		v->get_type() == Rep_OODT_Inheritance ||
		v->get_type() == Rep_OODT_Relations ||
		v->get_type() == Rep_OODT_Scope) ? iv2(Center) : TopLeft)
{
    scene = gs;
    view_header = v;
    picture = p;

    focus_symbol = NULL;
    highlight_symbol = NULL;
    widget = NULL;
    modal_value = in_select_modal;

    damage = new Damage(canvas, new iv3(Painter), picture);

    // Set up canvas type to be double-buffered.
    SetCanvasType(CanvasSaveContents);

    perspective->Attach(this);

    gra_init_actions();

    graWindows.insert_last(int(this));
}

// destructor
graWindow::~graWindow()
{
    perspective->Detach(this);
    delete damage;

    gra_handle_window_delete(this);
    graWindows.remove(int(this));
}

void graWindow::Draw()
{
    Initialize(graWindow::Draw);

    GraphicBlock::Draw();
    if(highlight_symbol)
    {
	xor_box->SetGraphic(highlight_symbol);
	xor_box->Redraw();
    }
    did_Draw = true;
    return;
}


//------------------------------------------
// fix_window
//
// Force the perspective to keep the viewer
// movement bounded by the graphic.  This
// is especially needed in resizing.
//------------------------------------------

static void fix_window (graWindow *window, iv3(Perspective) *ps)
{
    Initialize(fix_window);

    iv3(Perspective) new_ps = *ps;

    // If possible, modify new_ps so that it shows all of the graphic.
    if (ps->curwidth < ps->width) {
	if( ps->curx < ps->x0 )
	    new_ps.curx = ps->x0;
	else if((ps->curx + ps->curwidth) > (ps->x0 + ps->width))
	    new_ps.curx = ps->x0 + ps->width - ps->curwidth;
    } else if(ps->curx != ps->x0)
	new_ps.curx = ps->x0;

    if( ps->curheight < ps->height ) {
	if( ps->cury < ps->y0 )
	    new_ps.cury = ps->y0;
	else if((ps->cury + ps->curheight) > (ps->y0 + ps->height))
	    new_ps.cury = ps->y0 + ps->height - ps->curheight;
    } else if((ps->cury + ps->curheight) != (ps->y0 + ps->height))
	new_ps.cury = ps->y0 + ps->height - ps->curheight;

    // If ay changes were made, register them.
    if( comp_perspective ( &new_ps, ps) != 0 )
	window->Adjust(new_ps);
}

//------------------------------------------
// Update
//
// This function is called by the Perspective object whenever it is
// modified by some other object.  Interactors must respond to changes in
// the Perspective to which they have Attached themselves.  graWindow
// propogates changes in slider size and style to the Motif scrollbars.
//------------------------------------------

void graWindow::Update()
{
    Initialize(graWindow::Update);

    // do nothing for ERD and DATA CHART
    int vt = gra_type();
    if (!(vt == Rep_ERD || vt == Rep_DataChart || vt == Rep_SubsystemMap))
      fix_window (this, perspective);

    dialog_scrollbars();
}

void graWindow::refresh()
{
    Initialize(graWindow::refresh);

    fix_window (this, perspective);

    dialog_scrollbars();
}

/*_______________________________________________________________________
!
! Member:	graWindow::dialog_scrollbars
!
! Arguments:	int
!
! Returns:	void
!
! Synopsis:	Sends information about the perspective to the dialog so it will
!		show up in any scrollbars or panners that may be attached.
!
!		class Perspective
!    		IntCoord x0, y0;		origin of the graphic
!    		int width, height;		total size of the graphic
!    		IntCoord curx, cury;		current view position
!    		int curwidth, curheight;	current view size
!    		int sx, sy, lx, ly;		small and large scrolling increments
!
!		Calculates the scroll_bars' size as the percentage of the
!		graphic shown in the view, roughly (max - min) * (view size / graphic size).
!		The scroll value can be any value between the control's
!		(min + control's size) and max.
!
! Warning:	This routine must measure the scroll bar's position
!		by it's right edge as opposed to the left, our error.
!	        There should be some rounding error induced in the
!		scroll -> view relation since we calculate the scroll
!		bar's position from the view if the thumb box is moved.
!
_______________________________________________________________________*/
void graWindow::dialog_scrollbars()
{
    Initialize(graWindow::dialog_scrollbars);

    dialogPtr dlg = driver_instance->find_dialog_for_view(view_header);
    if(dlg == NULL)
	return;

    const int ticks = 100;
    const int view_num = dlg->get_view_num(view_header);
    dlg->set_vert_scrollbar_range(view_num, 0, ticks);

    iv3(Perspective)* ps = GetPerspective();

    int horiz_size = ticks;
    if((ps->width) && (ps->width - ps->curwidth > 0))
	horiz_size = round(double(ticks) * double(ps->curwidth)
			   / double(ps->width));
    if(horiz_size > ticks)
	horiz_size = ticks;

    int vert_size = ticks;
    if((ps->height) && (ps->height - ps->curheight > 0))
	vert_size = round(double(ticks) * double(ps->curheight)
			  / double(ps->height));
    if(vert_size > ticks)
	vert_size = ticks;

    int horiz_val = 0;
    if ((ps->width - ps->curwidth) > 0)
	horiz_val = round(double(horiz_size) + (double(ps->curx - ps->x0)
			   * double(ticks - horiz_size))
			  / double(ps->width - ps->curwidth));
    if(horiz_val > ticks)
	horiz_val = ticks;

    int vert_val = ticks;
    if (((ps->height - ps->curheight) > 0) && (ticks - vert_size != 0))
	vert_val = round(double(vert_size) + ((double(1) - double(ps->cury - ps->y0)
 			 / double(ps->height - ps->curheight)))
			 * double(ticks - vert_size));

    if(vert_val > ticks)
	vert_val = ticks;

    dlg->set_horiz_slider_size(view_num, horiz_size);
    dlg->set_vert_slider_size(view_num, vert_size);
    dlg->set_horiz_scrollbar_value(view_num, horiz_val);
    dlg->set_vert_scrollbar_value(view_num, vert_val);
}

//------------------------------------------
// set_magnification
//
// Change the perspective so that the graphic is displayed with
// the given magnification, and adjust the horizontal and vertical
// panning to conform to the alignment if the entire width and/or
// height will fit into the window.
//
// This code was adapted from GraphicBlock::SetMagnification and
// GraphicBlock::Align.
//------------------------------------------

void graWindow::set_magnification(float m)
{
    Initialize(graWindow::set_magnification);

    register iv3(Perspective)* p = perspective;

    float factor = min(100.0, double(m)) / mag;

    if (graphic != NULL && factor != 1.0)
    {
	// First handle the zoomOBing.

	iv2(Coord) halfw = p->curwidth / 2;
	iv2(Coord) halfh = p->curheight / 2;
	iv2(Coord) cx = p->curx + halfw;
	iv2(Coord) cy = p->cury + halfh;
	graphic->Scale(factor, factor, float(halfw), float(halfh));

	x0 = round((x0 - halfw)*factor + halfw);
	y0 = round((y0 - halfh)*factor + halfh);
	p->width = round(p->width * factor);
	p->height = round(p->height * factor);
	p->curx = round(float(cx) * factor) - halfw;
	p->cury = round(float(cy) * factor) - halfh;

	// Now adjust the panning independantly for each dimension.

	int dx = 0, dy = 0;

	if(p->width <= p->curwidth)
	{
	    switch (align) {
	      case BottomLeft:
	      case CenterLeft:
	      case TopLeft:
		dx = -p->curx;
		break;
	      case BottomCenter:
	      case iv2(Center):
	      case TopCenter:
		dx = (p->width - p->curwidth) / 2 - p->curx;
		break;
	      case BottomRight:
	      case CenterRight:
	      case TopRight:
		dx = p->width - p->curwidth - p->curx;
		break;
	    }
	}
	if(p->height <= p->curheight)
	{
	    switch (align) {
	      case BottomLeft:
	      case BottomCenter:
	      case BottomRight:
		dy = -p->cury;
		break;
	      case CenterLeft:
	      case iv2(Center):
	      case CenterRight:
		dy = (p->height - p->curheight) / 2 - p->cury;
		break;
	      case TopLeft:
	      case TopCenter:
	      case TopRight:
		dy = p->height - p->curheight - p->cury;
		break;
	    }
	}
#if 0
	if(dx || dy)
	{
	    p->curx += dx;
	    p->cury += dy;

	    iv2(Coord) l, b, dummy1, dummy2;
	    GetGraphicBox(l, b, dummy1, dummy2);
	    l = pad - l - p->curx;
	    b = pad - b - p->cury;
	    graphic->Translate(l, b);
	    x0 += l;
	    y0 += b;
	}
#endif
	reset_Drawn();
	UpdatePerspective();
	if (!Drawn())
	   Draw();
    }
    mag *= factor;
}

//------------------------------------------
// damage_area
//
// Tell the damage handler to damage the graphic area defined by the
// graphic symbol
//------------------------------------------

void graWindow::damage_area( Graphic *graphic )
{
    Initialize (graWindow::damage_area);

    if(canvas)
	damage->Incur(graphic);
}

void graWindow::flush_repair()
{
    Initialize (graWindow::flush_repair);

    if(damage->Incurred())
    {
	if(canvas)
	{
	    if(highlight_symbol)
		xor_box->Erase();

	    reset_Drawn();
	    UpdatePerspective();

	    if (!Drawn()) {
		damage->SetCanvas(canvas);
		damage->Repair();
	    }
	    else damage->Reset();

	    if(highlight_symbol)
		highlight_symbol->do_highlight(canvas, xor_box);
	}
	else
	    damage->Reset();
    }
}

//------------------------------------------
// flush_all
//
// Flush graphics all at once (from main event loop)
// to avoid annoying incremental updates.
//------------------------------------------

void graWindow::flush_all(void*)
{
    Initialize (graWindow::flush_all);

    for(int i = 0; i < graWindows.size(); i++)
    {
        graWindow* w = (graWindow*)graWindows[i];
	w->flush_repair();
    }
}


//------------------------------------------
// repair_damage
//
// No-op: see flush_all().
//------------------------------------------

void graWindow::repair_damage()
{}


//------------------------------------------
// repaint_window
//
// Force a repaint on the entire graphics canvas
//------------------------------------------

void graWindow::repaint_window()
{
    Initialize (graWindow::repaint_window);

    if(canvas)
	damage_area(graphic);
}

//------------------------------------------
// insert_symbol
//
// Insert a symbol into the graphics window
//------------------------------------------

void graWindow::insert_symbol( Picture *pic )
{
    Initialize (graWindow::insert_symbol);

    picture->Append(pic);
    damage_area(pic);
}

//------------------------------------------
// remove_symbol
//
// Remove a symbol from the graphics window
//------------------------------------------

void graWindow::remove_symbol( Picture *pic )
{
    Initialize (graWindow::remove_symbol);

    damage_area (pic);
    picture->Remove (pic);
}

// Return the window that contains the top-most InterViews window.
// The window returned _should_ belong to a widget.
void* graWindow::get_view_header_window()
{
    Initialize(graWindow::get_view_header_window);
    dialogPtr dlg = driver_instance->find_dialog_for_view(view_header);
    if(dlg)
	return dlg->window_of_view(view_header);
    else
	return NULL;
}

// Augment virtual function of GraphicBlock to allow re-routing of views.
void graWindow::Resize()
{
    Initialize (graWindow::Resize);

    if(highlight_symbol)		// Cancel any highlighting.
    {
	highlight_symbol = NULL;
	xor_box->SetCanvas(NULL);
    }

    GraphicBlock::Resize();

    if (view_header)
	view_header->resize();

    // execute any pending focus calls
    gra_focus_pending();
}

// returns true if canvas size is returned
boolean graWindow::size (int& width, int& height)
{
    Initialize(graWindow::size);

    iv3(Canvas)* canvas = GetCanvas();

    if(canvas) {

	width = canvas->pwidth();
	height = canvas->pheight();

	return true;

    } else {

	iv3(Shape)* shape = GetShape();
	if(shape) {
	    width = shape->width;
	    height = shape->height;
	} else {
	    width = 0;
	    height = 0;
	}

	return false;
    }
}


//------------------------------------------
// auto_scroll
//
// Change the Perspective while dragging,
// so that the given coordinates are visible.
//------------------------------------------

void graWindow::auto_scroll(int x, int y)
{
    Initialize(graWindow::auto_scroll);

    iv3(Perspective)& p = *perspective;

    int horz = 0, vert = 0;

    if(x < 0)
	horz = -1;
    else if(x > p.curwidth)
	horz = 1;

    if(y < 0)
	vert = -1;
    else if(y > p.curheight)
	vert = 1;

    if(horz || vert)
    {
	iv3(Perspective) s(p);

	if(horz)
	    s.curx += x - (s.curwidth * (2 + horz) / 4);

	if(vert)
	    s.cury += y - (s.curheight * (2 + vert) / 4);

	if(highlight_symbol)
	    xor_box->Erase();

	Adjust(s);

	if(highlight_symbol)
	    highlight_symbol->do_highlight(canvas, xor_box);
    }
}

//------------------------------------------
// graWindow::reset_Drawn()
//------------------------------------------

void graWindow::reset_Drawn()
{
   Initialize(graWindow::reset_Drawn);

   did_Draw = false;
}

//------------------------------------------
// graWindow::Drawn() const
//------------------------------------------

bool graWindow::Drawn() const
{
   Initialize(graWindow::Drawn);

   return did_Draw;
}


// local structures comparison; returns 0 if equal; 1 if not

static int comp_perspective( iv3(Perspective) * ps1, iv3(Perspective) * ps2){
      if ( ps1->x0 != ps2->x0 )
           return 1;

      else if ( ps1->y0 != ps2->y0 )
           return 1;

      else if ( ps1->width != ps2->width )
           return 1;

      else if ( ps1->height != ps2->height )
           return 1;

      else if ( ps1->curx != ps2->curx )
           return 1;

      else if ( ps1->cury != ps2->cury )
           return 1;

      else if ( ps1->curwidth != ps2->curwidth )
           return 1;

      else if ( ps1->curheight != ps2->curheight )
           return 1;

      else if ( ps1->sx != ps2->sx )
           return 1;

      else if ( ps1->sy != ps2->sy )
           return 1;

      else if ( ps1->lx != ps2->lx )
           return 1;

      else if ( ps1->ly != ps2->ly )
           return 1;

      else
           return 0;
}


/*
$Log: graWindow.h.C  $
Revision 1.4 1997/06/25 17:50:20EDT azaparov 
Fixed scrolling problem for large view
Revision 1.2.1.7  1993/04/26  18:04:52  glenn
Plug memory leaks related to Damage and Perspective instances
owned by graWindow.

Revision 1.2.1.6  1993/04/01  15:22:37  sergey
Corrected bad structures comparison. Part of bug #2963.

Revision 1.2.1.5  1993/02/04  21:38:40  glenn
Pass void* to graWindow::flush_all().  This function is now called
by gtBase::flush_output and should not be called directly anymore.

Revision 1.2.1.4  1992/11/18  20:39:12  oak
Made sure that the graphical views reset
the scroll bar bounds each time they are viewed.

Revision 1.2.1.3  1992/11/17  20:34:15  oak
Changed perspective -> scroll bar equations.
Fixes bug #1933.

// Revision 1.53  92/09/29  09:59:52  smit
// fix for scrollbars.
//
// Revision 1.52  92/09/04  06:46:25  wmm
// Plug one last (?) hole that allowed double-refreshes.
//
// Revision 1.51  92/09/03  11:51:01  wmm
// Do a better job of the preceding fix (zooming sometimes resulted in no
// screen update at all).
//
// Revision 1.50  92/09/03  09:21:12  wmm
// Fix bug 603 (multiple refresh of ERD): change graWindow::set_magnification()
// to use UpdatePerspective() instead of Perspective::Update() and Draw().
//
// Revision 1.49  92/08/20  12:06:33  boris
// Added Epoch - aSET Hang support
//
// Revision 1.48  92/07/30  11:36:10  jon
// Cleanup of InterViews scope fix
//
// Revision 1.47  92/07/29  23:46:21  jon
// InterViews scope fix
//
// Revision 1.46  92/07/09  14:52:03  glenn
// Add auto_scroll.
//
// Revision 1.45  92/06/26  17:39:47  smit
// flush rtls here.
//
// Revision 1.44  92/06/25  19:48:57  smit
// Make graphics window double buffered.
//
// Revision 1.43  92/06/23  15:51:53  wmm
// Add support for new OODT browser view types.
//
// Revision 1.42  92/06/22  10:51:31  boris
// Put ste_end_global_transaction() in flushAll()
//
// Revision 1.41  92/06/07  11:00:44  smit
// Change include files to use IV 3.0.1
//
// Revision 1.40  92/05/28  20:49:07  glenn
// Simplify graScene by commenting out all Tray code and include files.
// Stub out slider_style functions.
// Add function dialog_scrollbars and call from Update.
// Remove get_scrollbar_value and get_slider_size.
//
// Revision 1.39  92/05/22  13:42:09  wmm
// Make MultiConnection views (OODT browser, subsystem browser) place graphic
// in the center of the window instead of at the upper left corner.
//
// Revision 1.38  92/05/13  18:54:14  jont
// call gra_focus_pending in resize method
//
// Revision 1.37  92/05/08  09:19:28  glenn
// Check for damage incurred before repairing it to prevent unnecessary
// highlighting and de-highlighting.
//
// Revision 1.36  92/05/08  08:25:08  wmm
// Fix graWorld::flush_repair to do symbol-specific highlighting rather than
// assuming that all highlighting is done via the xor_box.
//
// Revision 1.35  92/05/06  17:47:33  smit
// Added static function to flush_all damages.
// Added a function to flush damage on a window.
// Made repair_damage method dummy method.
//
// Revision 1.34  92/05/04  11:10:29  glenn
// Suppress normal highlighting activity during Resize.  This should fix
// the problem with highlight crashes during "Maximize".
//
// Revision 1.33  92/04/16  08:23:55  glenn
// Fix problems with include files.
//
// Revision 1.32  92/04/13  15:00:09  smit
// Do not route for window independent routers during resize.
//
// Revision 1.31  92/03/30  17:08:42  smit
// set modal value per window.
//
// Revision 1.30  92/03/24  19:42:35  smit
// Update perspective in repair damage call.
//
// Revision 1.29  92/03/23  19:02:48  smit
// Fix reversal of parameters.
//
// Revision 1.28  92/03/19  12:09:01  smit
// Call Resize method of Graphic Block before doing our own resixe.
//
// Revision 1.27  92/03/19  09:17:46  smit
// Added Resize and size methods.
//
// Revision 1.26  92/03/04  13:48:29  glenn
// Limit zooming to 100x in set_magnification.
//
// Revision 1.25  92/03/03  18:33:40  glenn
// Move RubberGraphic class into its own file.
// Call gra_init_actions in graWindow ctor.
// Rename hilite_box to xor_box.
// Spell hilite as "highlight" in other symbol names.
// Move Redraw and Handle into graAction.h.C
//
// Revision 1.24  92/02/25  21:01:28  glenn
// Add some of the include files that were removed from graWindow.h
// Use NULL instead of nil.
//
// Revision 1.23  92/02/13  12:41:32  smit
// undraw the cursor when removing text focus.
//
// Revision 1.22  92/02/10  11:28:38  glenn
// Convert KeyEvents that only involve shift-keys into MotionEvents.
// Remove redundant code in DownEvent interpretation.
//
// Revision 1.21  92/02/04  10:34:48  glenn
// Implement get_view_header_window.
//
// Revision 1.20  92/01/26  17:17:45  glenn
// Add configurable slider.
// Clear and redraw hilite during repair_damage.
//
// Revision 1.19  92/01/24  10:02:36  glenn
// Make hilite_box two pixels thick.
// Rename RubberSymbol to RubberGraphic.
// Change Draw to use correct Rubband protocol.
// Add SetGraphic to class RubberGraphic.
// Set hilite_symbol to NULL in clear_hilite.
//
// Revision 1.18  92/01/23  19:08:26  glenn
// Define class RubberSymbol used for fast hiliting.
// Override Draw, Redraw to refresh the hilite box.
// Add set_hilite, clear_hilite.
//
// Revision 1.17  92/01/22  18:59:06  glenn
// Make the slider size smaller, and resolution-independant.
// Keep the slider the same size by making the surrounding Frame rigid.
// Do not create a Tray for text-only windows.
//
// Revision 1.16  92/01/21  16:49:05  glenn
// Implement set_magnification, for zooming with automatic recentering.
//
//Revision 1.15  92/01/20  14:39:30  glenn
//Check for NULL canvas before incurring or repairing damage.
//
//Revision 1.14  92/01/18  15:30:04  glenn
//Call damage->SetCanvas in repair_damage instead of damage_area.
//
//Revision 1.13  92/01/17  19:53:03  smit
//Donot show slider in ListText view.
//
//Revision 1.12  92/01/13  15:55:33  glenn
//Call Propagate(false) in graScene ctor to clamp off InterViews
//Change message propagation.
//Call UpdatePerspective in repaint_window to adapt to the Graphic
//object's new size.
//Incorporate (x0,y0) in calculation of Motif scrollbar values.
//
//Revision 1.11  92/01/09  15:57:51  glenn
//Attach the graWorld to its own Perspective so that its Update method
//gets called.
//Fix calculation of the vert_val for the Motif scrollbars.
//
//Revision 1.10  92/01/08  19:19:55  glenn
//Add ctor for class graScene, which pieces together an InterViews
//Tray object to hold the graWindow and a Slider.
//Add viewGraHeaderPtr to ctor args for class graWindow and remember
//it for use in Update.
//Remove initialization of redundant sensor and perspective objects.
//Add Update function for graWindow, which should propogate changes 
//in the perspective to the Motif scrollbars.
//
//Revision 1.9  91/12/26  18:24:52  smit
//Added text input handling stuff.
//
//Revision 1.8  91/12/24  15:14:42  smit
//Add yet another desirable feature to scrollbar code.
//
//Revision 1.7  91/12/23  14:38:47  smit
// *** empty log message ***
//
//Revision 1.6  91/12/23  13:45:52  smit
//Fix bug in translating coordinate system from lower left to upper right.
//
//Revision 1.5  91/12/20  19:28:13  smit
//Added stuff for scrollbars.
//
//Revision 1.4  91/11/13  14:14:56  smit
//Undo the previous change, since symbols were screwed up.
//
//Revision 1.3  91/11/13  13:51:03  smit
//Use resources for bakground and foreground color of interviews window.
//
//Revision 1.2  91/10/10  18:13:25  smit
//Make the graphics come up on the upperleft corner at 10 pixels offset.
//

*/

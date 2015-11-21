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
/************************************************************
 *
 * FILE: ProgressBar.C
 *
 * Implementation file for ProgressBar and related classes.
 *
 ************************************************************/

#include <x11_intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/MwmUtil.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>

#include <cLibraryFunctions.h>
#include <machdep.h>
#include <messages.h>

#include <gtNoParent.h>
#include <gtBaseXm.h>

#include <ProgressBar.h>

#include <lib/xpm.h>
#include <splash.xpm>

//
// Utility function:
//

int AllocXpm(Pixmap *pix, Widget w, int npix, char ***pd, Pixmap *mask)
{
  int ret = 0;

  XpmAttributes attrib;
  attrib.valuemask = 0;
  attrib.valuemask |= XpmCloseness;
  attrib.valuemask |= XpmReturnInfos;
  attrib.valuemask |= XpmReturnAllocPixels;
  attrib.valuemask |= XpmReturnExtensions;
  attrib.closeness = 200000;

  for (int i = 0; i < npix; i++) {
    if (mask)
      mask += i;
    ret = XpmCreatePixmapFromData(XtDisplay(w), 
				  XRootWindowOfScreen(XtScreen(w)),
                                  pd[i], pix + i,
                                  mask, &attrib);
    if (ret != XpmSuccess)
      break;
  }

  return ret;
}

//
// Static data:
//

const int ProgressBar::dval_max = 4;    // Max length of counter val

const char *ProgressBar::fg_bar = "blue";
const char *ProgressBar::fg_num = "black";

//
// ------------------- CLASS: ProgressBar -------------------
//

ProgressBar::ProgressBar(Widget w, int _limit, int _is_splash)
  : tick(0), 
    limit(_limit),
    default_msg((const char *)"Please wait..."),
    dialog(0),
    is_splash(_is_splash)
    
{
  if (w == 0)
    w = gtNoParent::app_shell()->container()->rep()->widget();
  InitDialog(w);
}

ProgressBar::~ProgressBar()
{
  DestroyWindow();
  UnloadXRes();
}

void ProgressBar::DestroyWindow()
{
  if (XtIsManaged(dialog)) {
    XtPopdown(XtParent(dialog));
    XtUnmanageChild(dialog);
  }
}

void ProgressBar::Hide()
{
  XtPopdown(XtParent(dialog));
}

void ProgressBar::Popup()
{
  XtPopup(XtParent(dialog), XtGrabNone);
}

void ProgressBar::Refresh()
{
  XFlush(XtDisplay(dialog));
  XmUpdateDisplay(dialog);

  Window win = XtWindow(dialog);
  Display *dpy = XtDisplay(dialog);
  XEvent event;

  while (XCheckMaskEvent(dpy, 0xffffffff, &event))
  {
      XtDispatchEvent(&event);
  }
}

// Specify the widget layout of the dialog box:

void ProgressBar::BuildDialog(Widget w)
{
  static const int xyoff = 15;         // Default horizontal and vertical offset
  static const Dimension xbar = 350;   // Width of bar canvas
  static const Dimension ybar = 24;    // Height of bar canvas
  static const Dimension xnum = 40;
  static const Dimension ynum = 24;

  // Width of dialog (theoretically we shouldn't need this, because the dialog
  // should auto-size itself in response to the size requests of its children.
  // However, our broken version of Motif doesn't support this, so we need to
  // explicitly give the width):
  
  static const Dimension xdlg = xbar + 200;

  //
  // Build the widgets and arrange them:
  //

  dialog = XmCreateFormDialog(w, "progress_bar", NULL, 0);
  XtVaSetValues(dialog, XmNnoResize, True, XmNwidth, xdlg, NULL);
  XtVaSetValues(XtParent(dialog),
		XmNtitle, "DIScover",
		XmNallowShellResize, True,
		XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		NULL);

  if (is_splash) {
    splash_frame = XtVaCreateWidget("splash_frame",
				    xmFrameWidgetClass, dialog,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNshadowType, XmSHADOW_IN,
				    NULL);

    splash_subform = XtVaCreateWidget("splash_subform",
				      xmFormWidgetClass, splash_frame,
				      NULL);

    char **pd[] = { splash };
    Pixmap xpm[1];
    Pixmap mask[1];

    AllocXpm(xpm, dialog, 1, pd, mask);

    splash_label = XtVaCreateWidget("splash_label",
				    xmLabelWidgetClass, splash_subform,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNlabelType, XmPIXMAP,
				    XmNlabelPixmap, xpm[0],
				    XmNbackground, WhitePixelOfScreen(XtScreen(parent)),
				    XmNforeground, BlackPixelOfScreen(XtScreen(parent)),
				    NULL);

    msg_label = XtVaCreateWidget("msg_label",
			         xmLabelWidgetClass, dialog,
			         XmNleftAttachment, XmATTACH_FORM,
			         XmNtopAttachment, XmATTACH_WIDGET,
			         XmNtopWidget, splash_frame,
			         XmNleftOffset, xyoff,
			         XmNtopOffset, xyoff,
			         NULL);

  }
  else {
    msg_label = XtVaCreateWidget("msg_label",
			         xmLabelWidgetClass, dialog,
			         XmNtopAttachment, XmATTACH_FORM,
			         XmNleftAttachment, XmATTACH_FORM,
			         XmNleftOffset, xyoff,
			         XmNtopOffset, xyoff,
			         NULL);
  }

  Widget num_frame = XtVaCreateWidget("num_frame",
				      xmFrameWidgetClass, dialog,
				      XmNshadowType, XmSHADOW_IN,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, msg_label,
				      XmNrightOffset, xyoff,
				      XmNbottomOffset, xyoff,
				      XmNtopOffset, xyoff,
				      NULL);

  num_canvas = XmCreateDrawingArea(num_frame, "num_canvas", NULL, 0);
  XtVaSetValues(num_canvas,
		XmNbackground, WhitePixelOfScreen(XtScreen(w)),
		NULL);

  Widget bar_frame = XtVaCreateWidget("bar_frame",
				      xmFrameWidgetClass, dialog,
				      XmNshadowType, XmSHADOW_IN,
				      XmNbottomAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_WIDGET,
				      XmNrightWidget, num_frame,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightOffset, 5,
				      XmNleftOffset, xyoff,              
				      XmNbottomOffset, xyoff,
				      XmNtopAttachment, XmATTACH_WIDGET,
				      XmNtopWidget, msg_label,
				      XmNtopOffset, xyoff,
				      NULL);

  canvas = XmCreateDrawingArea(bar_frame, "canvas", NULL, 0);
  XtVaSetValues(canvas,
		XmNbackground, WhitePixelOfScreen(XtScreen(w)),
		NULL);

  XtManageChild(dialog);
  if (is_splash) {
    XtManageChild(splash_frame);
    XtManageChild(splash_subform);
    XtManageChild(splash_label);
  }
  XtManageChild(msg_label);
  XtManageChild(num_frame);
  XtManageChild(num_canvas);
  XtManageChild(bar_frame);
  XtManageChild(canvas);

  // Hack to compensate for inconsistencies in our X support (specify the size
  // after the lower frame has been managed):

  XtVaSetValues(bar_frame, XmNwidth, xbar, XmNheight, ybar, NULL);
  XtVaSetValues(num_frame, XmNwidth, xnum, XmNheight, ynum, NULL);

  SetMessage(default_msg);
  DrawBar();

  XtAddCallback(canvas, XmNexposeCallback, ExposeCB, (XtPointer)this);
}

// Allocate X resources:

void ProgressBar::InitDialog(Widget w)
{
  LoadXRes(w);
  BuildDialog(w);
  Popup();

  Refresh();
}

int ProgressBar::LoadXRes(Widget w)
{
  int ret;

  parent = w;

  if (LoadColors() && LoadGCs())
    ret = 1;
  else
    ret = 0;

  return ret;
}

void ProgressBar::UnloadXRes()
{
  UnloadColors();
  UnloadGCs();
}

int ProgressBar::LoadColors()
{
  int ret = 1;

  Display *dpy = XtDisplay(parent);
  Colormap cmap = 0;                             // assignment to get rid of warning
  XtVaGetValues(parent, XtNcolormap, &cmap, NULL);
  XColor alloc_color;
  XColor exact_color;

  if (XAllocNamedColor(dpy, cmap, fg_bar, &alloc_color, &exact_color))
    col_bar = alloc_color.pixel;
  else
    col_bar = BlackPixelOfScreen(XtScreen(parent));

  if (XAllocNamedColor(dpy, cmap, fg_num, &alloc_color, &exact_color))
    col_num = alloc_color.pixel;
  else
    col_num = BlackPixelOfScreen(XtScreen(parent));

  return ret;
}

void ProgressBar::UnloadColors()
{
  Colormap cmap;
  XtVaGetValues(dialog, XtNcolormap, &cmap, NULL);
  XFreeColors(XtDisplay(parent), cmap, &col_bar, 1, 0);
  XFreeColors(XtDisplay(parent), cmap, &col_num, 1, 0);
}

int ProgressBar::LoadGCs()
{
  int ret = 0;

  unsigned long mask = 0;
  XGCValues gcv;

  mask |= GCForeground;

  // Create the bar gc:

  gcv.foreground = col_bar;
  gc_bar =  XtGetGC(parent, mask, &gcv);

  // Create the num gc:

  gcv.foreground = col_num;
  gc_num = XtGetGC(parent, mask, &gcv);

  if (gc_bar && gc_num)
    ret = 1;

  return ret;  
}

void ProgressBar::UnloadGCs()
{
  if (gc_bar) {
    XtReleaseGC(parent, gc_bar);
    gc_bar = NULL;
  }
  if (gc_num) {
    XtReleaseGC(parent, gc_num);
    gc_num = NULL;
  }
}

// Set the current info message (e.g.: "Loading..."):

void ProgressBar::SetMessage(const char *msg)
{
  XmString _msg = XmStringCreateLtoR((char *)gettext((char *)msg), XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues(msg_label, XmNlabelString, _msg, NULL);
  XmStringFree(_msg);

  Refresh();
}

// Draw the graphics:

void ProgressBar::DrawBar()
{
  //
  // Clear the window (lazy, but fine for our purposes):
  //

  XClearWindow(XtDisplay(canvas), XtWindow(canvas));
  XClearWindow(XtDisplay(num_canvas), XtWindow(num_canvas));

  //
  // Positioning and graphics data:
  //

  static Dimension w = 0;
  static Dimension h = 0;
  static Dimension nw = 0;
  static Dimension nh = 0;
  static XmFontList fontlist = NULL;
  static char dval[dval_max + 1];

  if (w == 0) {
    XtVaGetValues(canvas, XmNwidth, &w, XmNheight, &h, NULL);
    XtVaGetValues(num_canvas, XmNwidth, &nw, XmNheight, &nh, NULL);
    XtVaGetValues(msg_label, XmNfontList, &fontlist, NULL);
  }

  //
  // Determine % of progress bar
  //
  int ratio = tick ? (((double)100) / (((double)limit) / ((double)tick))) : 0;

  //
  // Draw the rectangle:
  //
  XFillRectangle(XtDisplay(canvas), XtWindow(canvas), gc_bar, 0, 0, (((double)(w * ratio))/100.0), h);

  //
  // Draw the status indicator:
  //
  OSapi_sprintf(dval, "%d%s", ratio, "%");
  XmString str = XmStringCreateLtoR((char *)gettext(dval), XmSTRING_DEFAULT_CHARSET);

  Dimension font_width = XmStringWidth(fontlist, str);
  Dimension font_height = XmStringHeight(fontlist, str);

  XmStringDraw(XtDisplay(num_canvas), XtWindow(num_canvas),
	       fontlist,
	       str,
	       gc_num,
	       (nw / 2) - (font_width / 2),
	       (nh / 2) - (font_height / 2),
	       font_width,
	       XmALIGNMENT_CENTER,
	       XmSTRING_DIRECTION_L_TO_R,
	       NULL);

  XmStringFree(str);
}

// Move the progress bar:

void ProgressBar::Increment(int n)
{
  if (n > 0) {
    tick += n;
    tick = NormalizeVal(tick, 0, limit);
    DrawBar();
  }

  Refresh();
}

int ProgressBar::NormalizeVal(int val, int min, int max)
{
  if ((val < min) || (val > max)) {
    int len = (max - min) + 1;
    int div = val / len;
    int rem = (val ? val : 0 - val) - (len * div);
    if (val < min)
      val = max - rem;
    else
      val = min + rem;
  }      

  return val;
}

void ProgressBar::Finish()
{
  while (tick < limit)
    Increment();
}

//
// Callbacks for drawing area:
//

void ProgressBar::ExposeCB(Widget, XtPointer client_data, XtPointer)
{
  ProgressBar *pb = (ProgressBar *)client_data;
  pb->DrawBar();
}

// Convenience routine to get the root of a widget tree:

Widget ProgressBar::TopShell(Widget w)
{
  while (w && !XtIsWMShell(w))
    w = XtParent(w);

  return w;
}

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
// gtBitmapXm.h.C
//------------------------------------------
// synopsis:
// 
// 1-24 bit deep image, with optional
// transparant areas.
//
//------------------------------------------

// INCLUDE FILES

//++X11
#include <X11/Xlib.h>
//--X11

//++MOTIF
#include <Xm/Xm.h>
#include <Xm/Label.h>
//--MOTIF

//++XPM
#include <lib/xpm.h>
//--XPM

//++GT
#include <gtLabel.h>
#include <gtPixmap.h>
#include <gtPixmapXm.h>
//--GT

gtPixmap *gtPixmap::create(gtBase *parent, const char *name, const char *xpm_filename, int w, int h)
{
  return new gtPixmapXm(parent, name, xpm_filename, w, h);
}

gtPixmap *gtPixmap::create(gtBase *parent, const char *name, const char **xpm_data, int w, int h)
{
  return new gtPixmapXm(parent, name, xpm_data, w, h);
}


gtPixmapXm::gtPixmapXm(gtBase *parent, const char *name, const char *xpm_filename, int w, int h)
{
  init_pixmaps(parent, true, xpm_filename, 0);
  place_in_label(parent, w, h);
}

gtPixmapXm::gtPixmapXm(gtBase *parent, const char *name, const char **xpm_data, int w, int h)
{
  init_pixmaps(parent, false, 0, xpm_data);
  place_in_label(parent, w, h);
}

int gtPixmapXm::init_pixmaps(gtBase *parent, bool from_file, char const *xpm_filename, char const **xpm_data)
{
  int retval = -1;

  image = 0;
  mask  = 0;
  attributes.valuemask = 0;
  
  if ( parent && ( xpm_filename || xpm_data ) )
    {
      // get Display and Screen
      int n = 0;
      Arg args[10];
      r->widget(XmCreateLabel(parent->container()->rep()->widget(), (char *)(name()), args, n));
      Display *display = XtDisplay(r->widget());
      
      Screen  *screen  = 0;
      XtSetArg(args[n], XmNscreen, &screen), n++;
      XtGetValues(r->widget(), args, n);

      // get the XPM

      if ( from_file )
	retval = XpmReadFileToPixmap(display, RootWindow(display, DefaultScreen(display)), (char *)xpm_filename, &image, &mask, &attributes);
      else
	retval = XpmCreatePixmapFromData(display, RootWindow(display, DefaultScreen(display)), (char **)xpm_data, &image, &mask, &attributes);

      if ( retval != XpmSuccess )
	{
	  image = 0;
	  mask  = 0;
	  attributes.valuemask = 0;
	  
	  XtAppContext context = XtDisplayToApplicationContext(display);
	  XtAppError(context, XpmGetErrorString(retval));

	  retval = -1;
	}
      else
	{
	  retval = 0;
	}
    }
  
  return retval;
}

int gtPixmapXm::place_in_label(gtBase *parent, int w, int h)
{
  int retval = -1;

  if ( image )
    {
      int n = 0;
      Arg args[10];
      
      // create label container for pixmap
      XtSetArg(args[n], XmNlabelType, XmPIXMAP), n++;
      XtSetArg(args[n], XmNmarginWidth, 0), n++;
      XtSetArg(args[n], XmNmarginHeight, 0), n++;
      XtSetArg(args[n], XmNwidth, w), n++;
      XtSetArg(args[n], XmNheight, h), n++;
      XtSetArg(args[n], XmNrecomputeSize, False), n++;
      r->widget(XmCreateLabel(parent->container()->rep()->widget(), (char *)(name()), args, n));
      
      // put the image into the label
      n = 0;
      XtSetArg(args[n], XmNlabelPixmap, image), n++;
      XtSetValues(r->widget(), args, n);

      retval = 0;
    }

  return retval;
}

gtPixmapXm::~gtPixmapXm()
{
  if ( image )
    {
      // Get display and colormap
      Display  *display  = XtDisplay(r->widget());
      Colormap  colormap = 0;
      Arg       args[1];
      XtSetArg(args[0], XmNcolormap, &colormap);
      XtGetValues(r->widget(), args, 1);
      
      // free pixmaps
      XFreePixmap(display, image);
      if ( mask ) XFreePixmap(display, mask);
      
      // free colors
      if ( attributes.alloc_pixels && (attributes.nalloc_pixels > 0) )
	XFreeColors(display, colormap, attributes.alloc_pixels, attributes.nalloc_pixels, 0);

      // free attributes
      XpmFreeAttributes(&attributes);
    }
}


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
#include "display.H"

static unsigned int count_args(va_list& args)
{
  unsigned int count = 0;
  while(va_arg(args, String))
  {
    va_arg(args, XtArgVal);
    ++count;
  }
  return count;
}

static Arg* get_args(va_list& args, unsigned int& count, String name0 = 0, XtArgVal value0 = 0)
{
  if(name0)
    ++count;

  Arg* array = new Arg[count];
  unsigned int n = 0;

  if(name0)
  {
    XtSetArg(array[n], name0, value0); ++n;
  }

  while(n < count)
  {
    String name = va_arg(args, String);
    XtArgVal value = va_arg(args, XtArgVal);
    XtSetArg(array[n], name, value); ++n;
  }
  return array;
}

UIObject::UIObject(char* name, UIObject& parent, WidgetClass widgetClass ...):
  application(name)
{
  va_list args;

  va_start(args, widgetClass);
  unsigned int count = count_args(args);
  va_end(args);

  va_start(args, widgetClass);
  Arg* xt_args = get_args(args, count);
  va_end(args);

  window = XtCreateManagedWidget(application::name, widgetClass, parent.window,
                                 xt_args, count);
  delete [] xt_args;
}


UIObject::UIObject(char* name, XtAppContext& context, int& argc, char** argv):
                 application(name)
{
# if XtSpecificationRelease == 5
  window = XtAppInitialize(&context, name, NULL,(Cardinal) 0, (int *) &argc, argv, NULL, NULL, 0);
# endif
# if XtSpecificationRelease == 4
  window = XtAppInitialize(&context, name, NULL,(Cardinal) 0, (Cardinal *) &argc, argv, NULL, NULL, 0);
# endif
}

void UIObject::allocColor(char* name, XColor* clr)
{
  Display* dpy = XtDisplay(window);
  int screen   = DefaultScreen(dpy);
  Colormap cmap = DefaultColormap(dpy, screen);
  XParseColor(dpy, cmap, name, clr);
  XAllocColor(dpy, cmap, clr);
}
  
void UIObject::getColors(Pixel bg, Pixel* newFg, Pixel* newTop, Pixel* newBottom, Pixel* newSelect)
{
  Display* dpy = XtDisplay(window);
  Screen* screen = XtScreen(window);
  Colormap cmap;
  XtVaGetValues(window, XmNcolormap, &cmap, NULL);
  XmGetColors(screen, cmap, bg, newFg, newTop, newBottom, newSelect);
}

Widget UIObject::getWidget()
{
  return window;
}

void UIObject::setValues(String name, XtArgVal value ... )
{
  va_list args;

  va_start(args, value);
  unsigned int count = count_args(args);
  va_end(args);

  va_start(args, value);
  Arg* xt_args = get_args(args, count, name, value);
  va_end(args);

  XtSetValues(window, xt_args, count);

  delete [] xt_args;
}

UIObject::~UIObject() { XtDestroyWidget(window); }

void UIObject::realize()  { XtRealizeWidget(window); }
void UIObject::map() { XtMapWidget(window); }

Screen* UIObject::getScreen()  { return (XtScreen(window)); }
Display* UIObject::getDisplay() { return (XtDisplay(window)); }






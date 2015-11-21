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

/*******************************************************************************
	Panner.c
	(Generated from interface file Panner.if)
*******************************************************************************/

#include <stdio.h>
#include "UxLib.h"
#include "UxDrawnB.h"
#include "UxForm.h"

/*******************************************************************************
	Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "perspective.h"
#include "uimx_context.h"
#define CONTEXT_PTR CONTEXT_PTR_NAME(Panner)
#define CURRENT_CONTEXT CURRENT_CONTEXT_NAME(Panner)
#define AllButtonsMask \
	(Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)
#define AllModsMask \
	(ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)

/*******************************************************************************
	The definition of the context structure:
	If you create multiple instances of your interface, the context
	structure ensures that your callbacks use the variables for the
	correct instance.

	For each swidget in the interface, each argument to the Interface
	function, and each variable in the Instance Specific section of the
	Declarations Editor, there is an entry in the context structure.
	and a #define.  The #define makes the variable name refer to the
	corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	swidget	UxPanner;
	swidget	Uxthumb;
	int	Uxpanner_x0;
	int	Uxpanner_y0;
	int	Uxpanner_width;
	int	Uxpanner_height;
	int	Uxpanner_curx;
	int	Uxpanner_cury;
	int	Uxpanner_curwidth;
	int	Uxpanner_curheight;
	Widget	Uxparent_w;
	void	(*Uxcallback)();
	void	*Uxcallback_data;
} _UxCPanner;

#define Panner                  UxPannerContext->UxPanner
#define thumb                   UxPannerContext->Uxthumb
#define panner_x0               UxPannerContext->Uxpanner_x0
#define panner_y0               UxPannerContext->Uxpanner_y0
#define panner_width            UxPannerContext->Uxpanner_width
#define panner_height           UxPannerContext->Uxpanner_height
#define panner_curx             UxPannerContext->Uxpanner_curx
#define panner_cury             UxPannerContext->Uxpanner_cury
#define panner_curwidth         UxPannerContext->Uxpanner_curwidth
#define panner_curheight        UxPannerContext->Uxpanner_curheight
#define parent_w                UxPannerContext->Uxparent_w
#define callback                UxPannerContext->Uxcallback
#define callback_data           UxPannerContext->Uxcallback_data

static _UxCPanner	*UxPannerContext;


/*******************************************************************************
	The following are translation tables.
*******************************************************************************/

static char	*Panner_back_xlate = "#replace\n\
<BtnDown>:Panner_back_input()\n";

static char	*Panner_thumb_xlate = "#replace\n\
<BtnDown>:Panner_thumb_input()\n\
<BtnMotion>:Panner_thumb_input()\n\
<BtnUp>:Panner_thumb_input()\n";

/*******************************************************************************
	Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget	create_Panner();

/*******************************************************************************
	Auxiliary code from the Declarations Editor:
*******************************************************************************/

static int round(a)
    double a;
{
    return (a > 0.0 ? (int)(a + 0.5) : -(int)(-a + 0.5));
}

#define MINSIZE 10
#define MAXSIZE 100

#define SPREAD(a,b) \
    if(b - a < MINSIZE) { a = (a + b - MINSIZE) / 2; b = a + MINSIZE; }

#define CLIP(a,b) \
    if(a < 0) a = 0; \
    if(b < MINSIZE / 2) b = MINSIZE / 2; \
    if(a > MAXSIZE - MINSIZE / 2) a = MAXSIZE - MINSIZE / 2; \
    if(b > MAXSIZE) b = MAXSIZE;

static void position_thumb()
{
    int left   = round((panner_curx - panner_x0) * MAXSIZE /
		       (double)panner_width);
    int bottom = round((panner_cury - panner_y0) * MAXSIZE /
		       (double)panner_height);
    int right  = round((panner_curx + panner_curwidth - panner_x0) * MAXSIZE /
		       (double)panner_width);
    int top    = round((panner_cury + panner_curheight - panner_y0) * MAXSIZE /
		       (double)panner_height);

    SPREAD(left,right)
    SPREAD(bottom,top)
    CLIP(left,right)
    CLIP(bottom,top)

    XtVaSetValues(UxGetWidget(thumb),
	XmNleftPosition,   left,
	XmNbottomPosition, MAXSIZE - bottom,
	XmNrightPosition,  right,
	XmNtopPosition,    MAXSIZE - top,
	NULL);

    XFlush(UxDisplay);
}

void Panner_perspective(UxWidget,
			x0, y0, width, height,
			curx, cury, curwidth, curheight)
    Widget UxWidget;
    int x0, y0, width, height;
    int curx, cury, curwidth, curheight;
{
    PUSH_CONTEXT

    panner_x0 = x0;
    panner_y0 = y0;
    panner_width = width;
    panner_height = height;
    panner_curx = curx;
    panner_cury = cury;
    panner_curwidth = curwidth;
    panner_curheight = curheight;

    position_thumb();

    POP_CONTEXT
}

void Panner_values(UxWidget,
		   x0, y0, width, height,
		   curx, cury, curwidth, curheight)
    Widget UxWidget;
    int *x0, *y0, *width, *height;
    int *curx, *cury, *curwidth, *curheight;
{
    PUSH_CONTEXT

    *x0 = panner_x0;
    *y0 = panner_y0;
    *width = panner_width;
    *height = panner_height;
    *curx = panner_curx;
    *cury = panner_cury;
    *curwidth = panner_curwidth;
    *curheight = panner_curheight;

    POP_CONTEXT
}

/*******************************************************************************
	The following are Action functions.
*******************************************************************************/

static void	action_Panner_thumb_input( UxWidget, UxEvent, UxParams, p_UxNumParams )
	Widget	UxWidget;
	XEvent	*UxEvent;
	String	*UxParams;
	int	*p_UxNumParams;
{
	int			UxNumParams = *p_UxNumParams;
	_UxCPanner              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxPannerContext;
	UxPannerContext = UxContext =
			(_UxCPanner *) UxGetContext( UxThisWidget );
	{
	    if(UxNumParams == 0)
	    {
		static Bool dragging;
		static Dimension width, height; /* unsigned! */
		static Position offset_x, offset_y;
		if(!dragging)
		{
		    if(UxEvent->type == ButtonPress)
		    {			/* Start dragging. */
			dragging = True;
			XtVaGetValues(UxGetWidget(Panner),
				      XmNwidth, &width,
				      XmNheight, &height,
				      NULL);
			offset_x = -UxEvent->xbutton.x;
			offset_y = -UxEvent->xbutton.y;
		    }
		}
		else
		{
		    Position thumb_left, thumb_top;
		    int curx, cury;
		    XtVaGetValues(UxGetWidget(thumb),
				  XmNx, &thumb_left, XmNy, &thumb_top, NULL);
		    thumb_left += UxEvent->xbutton.x + offset_x;
		    thumb_top  += UxEvent->xbutton.y + offset_y;
		    curx = panner_x0 +
			   round((thumb_left * panner_width) / (double)width);
		    cury = panner_y0 + panner_height - panner_curheight -
			   round((thumb_top * panner_height) / (double)height);
		    if((UxEvent->type == MotionNotify) &&
		       (UxEvent->xmotion.state & AllButtonsMask) == 0)
		    {			/* All buttons released, stop dragging. */
			dragging = False;
		    }
		    else if((UxEvent->type == MotionNotify) &&
			    (UxEvent->xmotion.state & AllModsMask))
		    {			/* Modifier held down, suspend callbacks. */
			panner_curx = curx;
			panner_cury = cury;
			position_thumb();
		    }
		    else
		    {			/* Normal drag. */
			(*callback)(UxWidget, callback_data, VIEW_PAN_XY, curx, cury);
			if(UxEvent->type == ButtonRelease)
			    dragging = False;
		    }
		}
	    }
	}
	UxPannerContext = UxSaveCtx;
}

static void	action_Panner_back_input( UxWidget, UxEvent, UxParams, p_UxNumParams )
	Widget	UxWidget;
	XEvent	*UxEvent;
	String	*UxParams;
	int	*p_UxNumParams;
{
	int			UxNumParams = *p_UxNumParams;
	_UxCPanner              *UxSaveCtx, *UxContext;
	swidget			UxThisWidget;

	UxThisWidget = UxWidgetToSwidget( UxWidget );
	UxSaveCtx = UxPannerContext;
	UxPannerContext = UxContext =
			(_UxCPanner *) UxGetContext( UxThisWidget );
	{
	    if(UxNumParams == 0)
	    {
		if(UxEvent->type == ButtonPress || UxEvent->type == KeyPress)
		{
		    Position thumb_left, thumb_top;
		    Dimension thumb_width, thumb_height;
		    int region, h, v;
		    XtVaGetValues(UxGetWidget(thumb),
				  XmNx, &thumb_left,
				  XmNy, &thumb_top,
				  XmNwidth, &thumb_width,
				  XmNheight, &thumb_height,
				  NULL);
		    region = 0;
		    if(UxEvent->xbutton.x < thumb_left)
			region |= 1;
		    else if(UxEvent->xbutton.x > thumb_left + thumb_width)
			region |= 2;
		    if(UxEvent->xbutton.y < thumb_top)
			region |= 4;
		    else if(UxEvent->xbutton.y > thumb_top + thumb_height)
			region |= 8;
		    h = panner_curwidth * 9 / 10;
		    if(h < 1) h = 1;
		    v = panner_curheight * 9 / 10;
		    if(v < 1) v = 1;
		    switch(region)
		    {
		      case  0: h =  0; v =  0; break;	/* inside thumb  */
		      case  1: h = -h; v =  0; break;	/* left center   */
		      case  2:         v =  0; break;	/* right center  */
		      case  4: h =  0;         break;	/* top center    */
		      case  5: h = -h;         break;	/* top left      */
		      case  6:                 break;	/* top right     */
		      case  8: h =  0; v = -v; break;	/* bottom center */
		      case  9: h = -h; v = -v; break;	/* bottom left   */
		      case 10:         v = -v; break;	/* bottom right  */
		    }
		    (*callback)(UxWidget, callback_data,
			VIEW_PAN_XY, panner_curx + h, panner_cury + v);
		}
	    }
	}
	UxPannerContext = UxSaveCtx;
}

/*******************************************************************************
	The following are callback functions.
*******************************************************************************/

/*******************************************************************************
	The 'init_' function sets the private properties for all the
	swidgets to the values specified in the Property Table.
	Some properties need to be set after the X widgets have been
	created and the setting of these properties is done in the
	'build_' function after the UxCreateWidget call.
*******************************************************************************/

static void	_Uxinit_Panner()
{
	UxPutTranslations( Panner, Panner_back_xlate );
	UxPutAutoUnmanage( Panner, "false" );
	UxPutNavigationType( Panner, "none" );
	UxPutMarginWidth( Panner, 0 );
	UxPutMarginHeight( Panner, 0 );
	UxPutShadowThickness( Panner, 0 );
	UxPutResizePolicy( Panner, "resize_none" );
	UxPutUnitType( Panner, "pixels" );

	UxPutUnitType( thumb, "pixels" );
	UxPutTranslations( thumb, Panner_thumb_xlate );
	UxPutBackground( thumb, "white" );
	UxPutHighlightThickness( thumb, 0 );
	UxPutShadowType( thumb, "shadow_out" );

}

/*******************************************************************************
	The 'build_' function creates all the swidgets and X widgets,
	and sets their properties to the values specified in the
	Property Editor.
*******************************************************************************/

static swidget	_Uxbuild_Panner()
{
	/* Create the swidgets */

	Panner = UxCreateForm( "Panner", (parent_w ? UxWidgetToSwidget(parent_w) : NO_PARENT) );
	UxPutContext( Panner, UxPannerContext );
	UxPutDefaultShell( Panner, "topLevelShell" );

	thumb = UxCreateDrawnButton( "thumb", Panner );

	_Uxinit_Panner();

	/* Create the X widgets */

	UxCreateWidget( Panner );
	UxPutTopPosition( thumb, 0 );
	UxPutRightPosition( thumb, 100 );
	UxPutTopAttachment( thumb, "attach_position" );
	UxPutRightAttachment( thumb, "attach_position" );
	UxPutLeftPosition( thumb, 0 );
	UxPutLeftAttachment( thumb, "attach_position" );
	UxPutBottomPosition( thumb, 100 );
	UxPutBottomAttachment( thumb, "attach_position" );
	UxCreateWidget( thumb );



	/* Finally, call UxRealizeInterface to create the X windows
	   for the widgets created above. */

	UxRealizeInterface( Panner );

	return ( Panner );
}

/*******************************************************************************
	The following function includes the code that was entered
	in the 'Initial Code' and 'Final Code' sections of the
	Declarations Editor. This function is called from the
	'Interface function' below.
*******************************************************************************/

static Widget	_Ux_create_Panner( _Uxparent_w, _Uxcallback, _Uxcallback_data )
	Widget	_Uxparent_w;
	void	(*_Uxcallback)();
	void	*_Uxcallback_data;
{
	swidget                 rtrn;
	_UxCPanner              *UxContext;

	UxPannerContext = UxContext =
		(_UxCPanner *) UxMalloc( sizeof(_UxCPanner) );

	parent_w = _Uxparent_w;
	callback = _Uxcallback;
	callback_data = _Uxcallback_data;
	rtrn = _Uxbuild_Panner();

	panner_x0 = panner_y0 = panner_curx = panner_cury = 0;
	panner_width = panner_height = panner_curwidth = panner_curheight = 100;
	position_thumb();
	return UxGetWidget(rtrn);
}

/*******************************************************************************
	The following is the 'Interface function' which is the
	external entry point for creating this interface.
	This function should be called from your application or from
	a callback function.
*******************************************************************************/

Widget	create_Panner( _Uxparent_w, _Uxcallback, _Uxcallback_data )
	Widget	_Uxparent_w;
	void	(*_Uxcallback)();
	void	*_Uxcallback_data;
{
	Widget			_Uxrtrn;
	static int		_Uxinit = 0;

	if ( ! _Uxinit )
	{
		static XtActionsRec	_Uxactions[] = {
				{ "Panner_thumb_input", action_Panner_thumb_input },
				{ "Panner_back_input", action_Panner_back_input }
				};

		XtAppAddActions( UxAppContext,
				_Uxactions,
				XtNumber(_Uxactions) );

		_Uxinit = 1;
	}

	_Uxrtrn = _Ux_create_Panner( _Uxparent_w, _Uxcallback, _Uxcallback_data );

	return ( _Uxrtrn );
}

/*******************************************************************************
	END OF FILE
*******************************************************************************/


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
	buttonbar_rowcol.c
	(Generated from interface file buttonbar_rowcol.if)
*******************************************************************************/

#include <stdio.h>
#include "UxLib.h"
#include "UxRowCol.h"

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
	swidget	Uxbuttonbar_rowcol;
	swidget	Uxparent;
	Widget	Uxparent_w;
} _UxCbuttonbar_rowcol;

#define buttonbar_rowcol        UxButtonbar_rowcolContext->Uxbuttonbar_rowcol
#define parent                  UxButtonbar_rowcolContext->Uxparent
#define parent_w                UxButtonbar_rowcolContext->Uxparent_w

static _UxCbuttonbar_rowcol	*UxButtonbar_rowcolContext;


/*******************************************************************************
	Forward declarations of functions that are defined later in this file.
*******************************************************************************/

Widget	create_buttonbar_rowcol();

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

static void	_Uxinit_buttonbar_rowcol()
{
	UxPutSpacing( buttonbar_rowcol, 30 );
	UxPutMarginWidth( buttonbar_rowcol, 30 );
	UxPutMarginHeight( buttonbar_rowcol, 30 );
	UxPutPacking( buttonbar_rowcol, "pack_tight" );
	UxPutTraversalOn( buttonbar_rowcol, "true" );
	UxPutAdjustMargin( buttonbar_rowcol, "false" );
	UxPutAdjustLast( buttonbar_rowcol, "false" );
	UxPutResizeWidth( buttonbar_rowcol, "true" );
	UxPutResizeHeight( buttonbar_rowcol, "true" );
	UxPutOrientation( buttonbar_rowcol, "horizontal" );
	UxPutUnitType( buttonbar_rowcol, "100th_millimeters" );

}

/*******************************************************************************
	The 'build_' function creates all the swidgets and X widgets,
	and sets their properties to the values specified in the
	Property Editor.
*******************************************************************************/

static swidget	_Uxbuild_buttonbar_rowcol()
{
	/* Create the swidgets */

	buttonbar_rowcol = UxCreateRowColumn( "buttonbar_rowcol", parent );
	UxPutContext( buttonbar_rowcol, UxButtonbar_rowcolContext );
	UxPutDefaultShell( buttonbar_rowcol, "transientShell" );


	_Uxinit_buttonbar_rowcol();

	/* Create the X widgets */

	UxCreateWidget( buttonbar_rowcol );


	/* Finally, call UxRealizeInterface to create the X windows
	   for the widgets created above. */

	UxRealizeInterface( buttonbar_rowcol );

	return ( buttonbar_rowcol );
}

/*******************************************************************************
	The following function includes the code that was entered
	in the 'Initial Code' and 'Final Code' sections of the
	Declarations Editor. This function is called from the
	'Interface function' below.
*******************************************************************************/

static Widget	_Ux_create_buttonbar_rowcol( _Uxparent_w )
	Widget	_Uxparent_w;
{
	swidget                 rtrn;
	_UxCbuttonbar_rowcol    *UxContext;

	UxButtonbar_rowcolContext = UxContext =
		(_UxCbuttonbar_rowcol *) UxMalloc( sizeof(_UxCbuttonbar_rowcol) );

	parent_w = _Uxparent_w;
	{
		parent = UxWidgetToSwidget(parent_w);
		rtrn = _Uxbuild_buttonbar_rowcol();

		XtVaSetValues(UxGetWidget(buttonbar_rowcol),
			XmNleftAttachment, XmATTACH_FORM,
			XmNleftOffset, 0,
			XmNrightAttachment, XmATTACH_FORM,
			XmNrightOffset, 0,
			XmNtopAttachment, XmATTACH_FORM,
			XmNtopOffset, 0,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNbottomOffset, 0,
			0);
		return UxGetWidget(buttonbar_rowcol);
	}
}

/*******************************************************************************
	The following is the 'Interface function' which is the
	external entry point for creating this interface.
	This function should be called from your application or from
	a callback function.
*******************************************************************************/

Widget	create_buttonbar_rowcol( _Uxparent_w )
	Widget	_Uxparent_w;
{
	Widget			_Uxrtrn;

	_Uxrtrn = _Ux_create_buttonbar_rowcol( _Uxparent_w );

	return ( _Uxrtrn );
}

/*******************************************************************************
	END OF FILE
*******************************************************************************/


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
	PROJECT.c
	(Generated from interface file PROJECT.if)
*******************************************************************************/

#include <stdio.h>
#include "UxLib.h"
#include "UxRowCol.h"
#include "UxApplSh.h"

/*******************************************************************************
	Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include <Xm/MwmUtil.h>
void apl_PROJ_Really_Quit();

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
	swidget	UxPROJECT;
	swidget	UxrowColumn;
} _UxCPROJECT;

#define PROJECT                 UxPROJECTContext->UxPROJECT
#define rowColumn               UxPROJECTContext->UxrowColumn

static _UxCPROJECT	*UxPROJECTContext;


/*******************************************************************************
	Forward declarations of functions that are defined later in this file.
*******************************************************************************/

swidget	popup_PROJECT();

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

static void	_Uxinit_PROJECT()
{
	UxPutMappedWhenManaged( PROJECT, "false" );
	UxPutGeometry( PROJECT, "-0+0" );
	UxPutTitle( PROJECT, "aset" );

	UxPutPacking( rowColumn, "pack_column" );
	UxPutEntryAlignment( rowColumn, "alignment_center" );
	UxPutAdjustLast( rowColumn, "false" );
	UxPutUnitType( rowColumn, "100th_millimeters" );

}

/*******************************************************************************
	The 'build_' function creates all the swidgets and X widgets,
	and sets their properties to the values specified in the
	Property Editor.
*******************************************************************************/

static swidget	_Uxbuild_PROJECT()
{
	/* Create the swidgets */

	PROJECT = UxCreateApplicationShell( "PROJECT", NO_PARENT );
	UxPutContext( PROJECT, UxPROJECTContext );

	rowColumn = UxCreateRowColumn( "rowColumn", PROJECT );

	_Uxinit_PROJECT();

	/* Create the X widgets */

	UxCreateWidget( PROJECT );
	UxCreateWidget( rowColumn );

	UxPutMwmDecorations( PROJECT, MWM_DECOR_BORDER|MWM_DECOR_MENU );


	/* Finally, call UxRealizeInterface to create the X windows
	   for the widgets created above. */

	UxRealizeInterface( PROJECT );

	return ( PROJECT );
}

/*******************************************************************************
	The following function includes the code that was entered
	in the 'Initial Code' and 'Final Code' sections of the
	Declarations Editor. This function is called from the
	'Interface function' below.
*******************************************************************************/

static swidget	_Ux_popup_PROJECT()
{
	swidget                 rtrn;
	_UxCPROJECT             *UxContext;

	UxPROJECTContext = UxContext =
		(_UxCPROJECT *) UxMalloc( sizeof(_UxCPROJECT) );

	rtrn = _Uxbuild_PROJECT();

	return(rtrn);
}

/*******************************************************************************
	The following is the 'Interface function' which is the
	external entry point for creating this interface.
	This function should be called from your application or from
	a callback function.
*******************************************************************************/

swidget	popup_PROJECT()
{
	swidget			_Uxrtrn;

	_Uxrtrn = _Ux_popup_PROJECT();

	return ( _Uxrtrn );
}

/*******************************************************************************
	END OF FILE
*******************************************************************************/


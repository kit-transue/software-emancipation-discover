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
#ifndef _graTools_h
#define _graTools_h

#define TOOLZoomFactor		1.1
#define TOOLPickAperture	3

class viewGraHeader;
class graParametricSymbol;

void gra_focus_pending();

void gra_manipulate_perspective(graWindow*, int op, int, int, int, int);

void gra_handle_window_delete(graWindow*);
void gra_handle_symbol_delete(graWindow*, graParametricSymbol*);


#endif // _graTools_h

/*
// graTools
//------------------------------------------
// synopsis:
// Declare utility routines for graWindow's and their symbols.
//
// description:
// ...
//------------------------------------------
// $Log: graTools.h  $
// Revision 1.2 1996/10/14 16:29:24EDT azaparov 
// 
 * Revision 1.2.1.2  1992/10/09  18:52:31  builder
 * fixed rcs heaqer
 *
 * Revision 1.2.1.1  92/10/07  20:42:08  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:42:07  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:31  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:56:10  smit
 * Initial revision
 * 
// Revision 1.7  92/05/13  18:55:45  jont
// declared gra_focus_pending()
// 
// Revision 1.6  92/03/04  13:51:20  glenn
// Remove dead function declarations.
// 
// Revision 1.5  92/03/03  18:39:15  glenn
// Change declaration of gra_handle_symbol_delete.
// 
//Revision 1.4  92/02/03  14:47:06  glenn
//Replace include files with forward class declarations.
//
//Revision 1.3  91/12/26  18:31:08  smit
//Added declaration for new functions.
//
//Revision 1.2  91/09/26  11:26:01  kws
//Convert perspective manipulation args from float to int
//
//Revision 1.1  91/09/03  09:08:06  kws
//Initial revision
//
//Revision 1.1  91/09/01  19:44:16  kws
//Initial revision
//
//Revision 1.1  91/09/01  19:32:14  kws
//Initial revision
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/

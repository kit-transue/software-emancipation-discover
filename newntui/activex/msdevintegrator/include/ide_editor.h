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
///////////////////////////////////////////////////////////////////////////
//
//	IDE_editor.h
//
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
#ifndef _IDE_EDITOR_H
#define _IDE_EDITOR_H

enum IDEVERSION
{
	NoIDE,
	MSDEV2x,
	MSDEV4x,
	MSDEV5x,
	MSDEV6x
};

void  dis_integration_load_file (char *fname );
void  dis_integration_open_file (char *fname, int line );
void  dis_integration_make_selection ( char* fname, int line, int col, char* tok=0 );
void  dis_integration_make_selection_internal( int line, int col, int len  );
int   dis_integration_read_selection ( int get_lineoffset = 0 /* ==0 if lineoffset does not needed */ );
int   dis_integration_getsel_tabsize ();
int   dis_integration_get_tabsize ();
int   dis_integration_getsel_lineoffset ();
int   dis_integration_getsel_line ();
int   dis_integration_getsel_col ();
char* dis_integration_getsel_token ();
char* dis_integration_getsel_fname ();


enum IDEVERSION	dis_integration_get_IDE();

#endif  //#ifndef _IDE_EDITOR_H



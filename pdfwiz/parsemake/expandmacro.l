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
%{
// expandmacro.l
// 23.oct.97 kit transue
// preprocessor to expand macros prior to parsing of makefile

// developed using -+ option of flex 2.5.4 (c++ support may change in later revs)

#include <iostream>
#include "variabletable.h"
static bool expansionmade = false;  // flag to see if we should try expanding again
%}

%option noyywrap

%x MACRO
%%

$\(		{ BEGIN MACRO; expansionmade = true; }
$.		{ expansionmade = true;
			macro_node const &expanded = global_symbols->lookup(yytext + 1);
			if (&expanded != macro_node::null_macro)
				*(yyout) << expanded.value;
		}

<MACRO>\)	{ BEGIN INITIAL; }
<MACRO>[^\)]+	{ macro_node const &expanded = global_symbols->lookup(yytext);
			if (&expanded != macro_node::null_macro)
				*(yyout) << expanded.value;
		}
<<EOF>>		{ int ret = expansionmade ? 2 : 1;
			expansionmade = false;
			return ret;
		}
%%

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
#ifdef WIN32
typedef unsigned int size_t;
#endif

void sym_define_new();
void sym_set_text_mode();
void sym_define_layer();
void sym_moveto();
void sym_lineto();
void sym_ellipse();
void sym_color();
void sym_stroke();
void sym_fill();
void sym_image();
void sym_linewidth();
void sym_attachment();
void sym_set_text();

%}

%union {
  struct constant {long value; int unsignedp;} integer;
  struct name {char *address; int length;} name;
  int voidval;
  char *sval;
  char* node;
}

%start SYMS

%token ATTACH TEXT_MODE MOVETO LINETO ELLIPSE COLOR LINE_WIDTH IMAGE STROKE FILL TEXT NAME NUMBER
%type <name> NAME
%type <integer> NUMBER
%type <name> ATTACHMENT_POS 
%%

SYMS : |
       SYMDEF SYMS ;

SYMDEF : SYMBOL_NAME '{' GRA_DEFS '}' ;

SYMBOL_NAME : NAME
		{ sym_define_new((char *)$<name>1.address);
		  /* printf("Symbol name %s", (char *)$<name>1.address); */
		  free($<name>1.address);
		}
	      ;	

GRA_DEFS: TEXT_MODE_SPEC TEXT_SPEC ATTACHMENTS LAYERS ;

TEXT_SPEC: |
           TEXT NAME
             { sym_set_text($<name>2.address);
	       free($<name>2.address);
	     }

LAYERS :  |
	  LAYER LAYERS
	  ;

LAYER : LAYER_NAME '{' GRAPHIC_COMMANDS '}' ;

LAYER_NAME : NAME 
               { sym_define_layer((char *)$<name>1.address);
	         free($<name>1.address);
	       }

GRAPHIC_COMMANDS :  |
		    GRAPHIC_COMMAND GRAPHIC_COMMANDS
		    ;

GRAPHIC_COMMAND : MOVETO NUMBER ',' NUMBER  
		    { sym_moveto($<integer>2.value, $<integer>4.value); }
                 | LINETO NUMBER ',' NUMBER
		    { sym_lineto($<integer>2.value, $<integer>4.value); }
	         | ELLIPSE NUMBER ',' NUMBER ',' NUMBER ',' NUMBER
		    { sym_ellipse($<integer>2.value, $<integer>4.value, $<integer>6.value, $<integer>8.value); }
                 | COLOR NUMBER ',' NUMBER ',' NUMBER
		    { sym_color($<integer>2.value, $<integer>4.value, $<integer>6.value); }
		 | STROKE
		    { sym_stroke(); }
		 | FILL
		    { sym_fill(); }
		 | IMAGE NAME
		    { sym_image((char *)$<name>2.address);
		      free($<name>2.address);
		    }
		 | LINE_WIDTH NUMBER
		    { sym_linewidth($<integer>2.value); }
	         ;

TEXT_MODE_SPEC : TEXT_MODE NAME
                   { sym_set_text_mode((char *)$<name>2.address);
                     free($<name>2.address);
	           }

ATTACHMENTS : |
	      ATTACHMENT ATTACHMENTS
              ;

ATTACHMENT : ATTACH ATTACHMENT_POS NUMBER ',' NUMBER
               { sym_attachment((char *)$<name>2.address, $<integer>3.value, $<integer>5.value); 
	         free($<name>2.address);
	       }

ATTACHMENT_POS : NAME ;

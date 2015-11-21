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
#include <string.h>
#include <math.h>
char quote_buffer[65535];
int buffer_index = 0;
int lineno = 1;
#define RETURN(xx) return xx;

/* default yywrap function - always treat EOF as an EOF */
#define yywrap() 1

/* stuff that should be defined in header files */
#include "cmd_decls.h"
#include "cmd_interp.h"

#ifndef _general_h
#include <general.h>
#endif
    
#include "cmd_regression.yacc.h"

#undef YY_INPUT
int regression_yyInput(char *buf, int max_size);
#define YY_INPUT(b,r,ms) (r=regression_yyInput(b,ms))
int yyparse(void);
%}

%x COMMENT QUOTE

%%
[\t ]+		/* ignore whitespace */ ;
\n		++lineno;
0		{ RETURN(ZERO); }
[+-]?[0-9]*\.[0-9]+	{
			yylval.double_const = atof(yytext);
			RETURN(DOUBLE);
		}
[+-]?[0-9]+	{
			yylval.int_const = atoi(yytext);
			RETURN(INT);
		}
"/*"		{ BEGIN COMMENT; }
<COMMENT>\n	{ ++lineno; }
<COMMENT>"*/"	{ BEGIN 0; }

\"		{ BEGIN QUOTE; }
<QUOTE>\"	{
			BEGIN 0;
			quote_buffer[buffer_index] = 0;
			yylval.char_star_const = strdup(quote_buffer);
			buffer_index = 0;
			RETURN(STRING);
		}
<QUOTE>\\\n	++lineno; /* ignore esc-newline continuation */ ;
<QUOTE>\\n	{ quote_buffer[buffer_index++] = '\n'; }
<QUOTE>\\t	{ quote_buffer[buffer_index++] = '\t'; }
<QUOTE>\\.	{ quote_buffer[buffer_index++] = yytext[1]; }
<QUOTE>\n	{ ++lineno; quote_buffer[buffer_index++] = yytext[0]; }
<QUOTE>.	{ quote_buffer[buffer_index++] = yytext[0]; }


cmd_convert_window_command	{ RETURN(CMD_CONVERT_WINDOW_COMMAND); }
cmd_convert_window_report	{ RETURN(CMD_CONVERT_WINDOW_REPORT); }
cmd_epoch_macro	{ RETURN(CMD_EPOCH_MACRO); }
cmd_find_file	{ RETURN(CMD_FIND_FILE); }
cmd_report_node_def	{ RETURN(CMD_REPORT_NODE_DEF); }
cmd_set_script_version	{ RETURN(CMD_SET_SCRIPT_VERSION); }
cmd_set_validation_group	{ RETURN(CMD_SET_VALIDATION_GROUP); }
cmd_set_validation_level	{ RETURN(CMD_SET_VALIDATION_LEVEL); }
em_set_speed	{ RETURN(EM_SET_SPEED); }
gra_begin_connection	{ RETURN(GRA_BEGIN_CONNECTION); }
gra_collapse_expand	{ RETURN(GRA_COLLAPSE_EXPAND); }
gra_drop	{ RETURN(GRA_DROP); }
gra_end_connection	{ RETURN(GRA_END_CONNECTION); }
gra_modal_click	{ RETURN(GRA_MODAL_CLICK); }
gra_open_node	{ RETURN(GRA_OPEN_NODE); }
gra_pickup	{ RETURN(GRA_PICKUP); }
gra_select	{ RETURN(GRA_SELECT); }
gra_symbol_menu	{ RETURN(GRA_SYMBOL_MENU); }
ste_cmd_mouse	{ RETURN(STE_CMD_MOUSE); }
ste_cursor	{ RETURN(STE_CURSOR); }
ste_do_replace_remove	{ RETURN(STE_DO_REPLACE_REMOVE); }
ste_insert_text	{ RETURN(STE_INSERT_TEXT); }
ste_region	{ RETURN(STE_REGION); }
synchronize_now	{ RETURN(SYNCHRONIZE_NOW); }
ui_addselect	{ RETURN(UI_ADDSELECT); }
ui_button_event	{ RETURN(UI_BUTTON_EVENT); }
ui_list_action	{ RETURN(UI_LIST_ACTION); }
ui_push_button	{ RETURN(UI_PUSH_BUTTON); }
ui_replace_text	{ RETURN(UI_REPLACE_TEXT); }
ui_select_gadget	{ RETURN(UI_SELECT_GADGET); }
ui_select_label	{ RETURN(UI_SELECT_LABEL); }
ui_select_list	{ RETURN(UI_SELECT_LIST); }
ui_select_top	{ RETURN(UI_SELECT_TOP); }
ui_select_viewer	{ RETURN(UI_SELECT_VIEWER); }
ui_validate_match_count	{ RETURN(UI_VALIDATE_MATCH_COUNT); }
ui_validate_namedInt	{ RETURN(UI_VALIDATE_NAMEDINT); }
ui_validate_namedString	{ RETURN(UI_VALIDATE_NAMEDSTRING); }
ui_validate_obj	{ RETURN(UI_VALIDATE_OBJ); }
ui_viewer_validate	{ RETURN(UI_VIEWER_VALIDATE); }
vsystem_from_script	{ RETURN(VSYSTEM_FROM_SCRIPT); }
C_node		{ RETURN(C_NODE); }
dbg_prompt	{ RETURN(DBG_PROMPT); }
if		{ RETURN(IF); }
[A-Za-z_]+	{ fprintf(stderr, "catastrophe: Unknown function: %s on line %d\n", yytext, lineno); exit(1); }
.	{ RETURN(yytext[0]); }
%%

void yyerror(char const *s)
{
	fprintf(stderr, "catastrophe: Regression parse error: %s\n", s);
	fprintf(stderr, "Line number: %d\n", lineno);
	fflush(stderr);
	exit(1);
}

static char * string_buffer;
static char * string_buffer_readhead;
static char * string_buffer_end;

int regression_yyInput(char *buf, int max_size)
{
	int n = string_buffer_end - string_buffer_readhead;
	if (n > max_size) n = max_size;
	if (n > 0) {
		memcpy(buf, string_buffer_readhead, n);
		string_buffer_readhead += n;
	}
	return n;
}
	


extern "C"
void execute_regression(char *s)
{
	string_buffer_readhead = string_buffer = strdup(s);
	string_buffer_end = string_buffer + strlen(s);

	printf(" ___cmd_journal");
	yyparse();
	free(string_buffer);
	printf("\n");
	fflush(stdout);
}

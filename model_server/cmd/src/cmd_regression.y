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

#if 0
#include <app.h>
#else
#include "cmd_decls.h"
#endif

#include "thunks.h"

#ifndef __cplusplus 	/* yacc files aren't compiled as C++ */
  #define const  
#endif


#ifndef _general_h
#include <general.h>
#endif

#include <stdlib.h>

/* standard yacc stuff; should be moved elsewhere */
void yyerror (const char* msg);
int yylex();


%}


%union {
	Widget_thunk *Widget_type;
	viewer_star_thunk *viewer_star_type;
	char_star_thunk *char_star_type;
	double_thunk *double_type;
	appTree_star_thunk *appTree_star_type;
	thunk_base *void_type;
	stmt_list_thunk *stmt_list_type;
	int_thunk *int_type;
	int_thunk_list *int_list_type;

	char *char_star_const;
	int int_const;
	double double_const;
}

%token CMD_CONVERT_WINDOW_COMMAND
%token CMD_CONVERT_WINDOW_REPORT
%token CMD_EPOCH_MACRO
%token CMD_EPOCH_WINDOW_COMMAND
%token CMD_EPOCH_WINDOW_REPORT
%token CMD_FIND_FILE
%token CMD_REPORT_NODE_DEF
%token CMD_SET_SCRIPT_VERSION
%token CMD_SET_VALIDATION_GROUP
%token CMD_SET_VALIDATION_LEVEL
%token CMD_SWITCH_TO_BUFFER
%token DR_CLEAR_SELECTION
%token DR_COPY
%token DR_CUT
%token DR_PASTE
%token EM_SET_SPEED
%token GRA_BEGIN_CONNECTION
%token GRA_COLLAPSE_EXPAND
%token GRA_DROP
%token GRA_END_CONNECTION
%token GRA_FOCUS_VIEWER
%token GRA_MODAL_CLICK
%token GRA_OPEN_NODE
%token GRA_PICKUP
%token GRA_SELECT
%token GRA_SYMBOL_MENU
%token PDCMD_EXECUTE
%token STE_CMD_MOUSE
%token STE_CMD_SAVE
%token STE_CURSOR
%token STE_DO_REPLACE_REMOVE
%token STE_INSERT_TEXT
%token STE_RAW_CURSOR
%token STE_REGION
%token STE_REPLACE_TEXT
%token SYNCHRONIZE_NOW
%token UI_ADDSELECT
%token UI_BUTTON_EVENT
%token UI_LIST_ACTION
%token UI_PUSH_BUTTON
%token UI_REPLACE_TEXT
%token UI_SELECT_GADGET
%token UI_SELECT_LABEL
%token UI_SELECT_LIST
%token UI_SELECT_TOP
%token UI_SELECT_VIEWER
%token UI_VALIDATE_MATCH_COUNT
%token UI_VALIDATE_NAMEDINT
%token UI_VALIDATE_NAMEDSTRING
%token UI_VALIDATE_OBJ
%token UI_VIEWER_VALIDATE
%token VSYSTEM_FROM_SCRIPT
%token IF
%token ZERO
%token C_NODE
%token C_HDR
%token DBG_PROMPT
%token <char_star_const> STRING
%token <int_const> INT
%token <double_const> DOUBLE


%type <Widget_type> Widget_expr
%type <viewer_star_type> viewer_star_expr
%type <char_star_type> char_star_expr
%type <char_star_type> opt_char_star_expr
%type <void_type> void_expr
%type <void_type> stmt
%type <stmt_list_type> stmt_list

%type <double_type> double_expr
%type <appTree_star_type> appTree_star_expr
%type <int_type> int_expr
%type <int_list_type> int_list


%%

start : '{' stmt_list '}'
	  {
		cmd_stmt_list_ptr = $2;
	  }
	;

stmt_list :
	  stmt { $$ = new stmt_list_thunk($1); }
	| stmt stmt_list
	  {
		$2->push_thunk($1);
		$$ = $2;
	  }
	;

stmt :
	  Widget_expr ';' { $$ = $1; }
	| viewer_star_expr ';' { $$ = $1; }
	| char_star_expr ';' { $$ = $1; }
	| void_expr ';' { $$ = $1; }
	| double_expr ';' { $$ = $1; }
	| appTree_star_expr ';' { $$ = $1; }
	| int_expr ';' { $$ = $1; }
	| '{' stmt_list '}' { $$ = $2; }
	| IF '(' int_expr ')' stmt { $$ = new if_thunk($3, $5); }
	;

char_star_expr :
	  STRING { $$ = new char_star_thunk($1); }
	  | ZERO { $$ = new char_star_thunk(0); }
	;

Widget_expr :
	  UI_ADDSELECT '(' char_star_expr ',' char_star_expr ')'
	  {
	 	$$ = new ui_addselect_thunk($3, $5);
	  }
	| UI_BUTTON_EVENT '(' char_star_expr ',' int_expr ',' int_expr ')'
	  {
		$$ = new ui_button_event_thunk($3, $5, $7);
	  }
	| UI_LIST_ACTION '(' char_star_expr ')'
	  {
		$$ = new ui_list_action_thunk($3);
	  }
	| UI_PUSH_BUTTON '(' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new ui_push_button_thunk($3);
	  }
	| UI_REPLACE_TEXT '(' char_star_expr ',' char_star_expr ',' int_expr ')'
	  {
		$$ = new ui_replace_text_thunk($3, $5, $7);
	  }
	| UI_SELECT_GADGET '(' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new ui_select_gadget_thunk($3, $5);
	  }
	| UI_SELECT_LABEL '(' char_star_expr ',' char_star_expr ',' int_expr ')'
	  {
		$$ = new ui_select_label_thunk($3, $5, $7);
	  }
	| UI_SELECT_LIST '(' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new ui_select_list_thunk($3, $5);
	  }
	| UI_SELECT_TOP '(' char_star_expr ',' int_expr ')'
	  {
		$$ = new ui_select_top_thunk($3, $5);
	  }
	;

viewer_star_expr :
	  UI_SELECT_VIEWER '(' int_expr ')'
	  {
		$$ = new ui_select_viewer_thunk($3);
	  }
	;

void_expr :
	  CMD_CONVERT_WINDOW_COMMAND '(' int_expr ',' char_star_expr ',' int_expr ',' int_expr ')'
	  {
		$$ = new cmd_convert_window_command_thunk($3, $5, $7, $9);
	  }
	| CMD_CONVERT_WINDOW_REPORT '(' int_expr ',' char_star_expr ',' int_expr ',' int_expr ')'
	  {
		$$ = new cmd_convert_window_report_thunk($3, $5, $7, $9);
	  }
	| CMD_EPOCH_MACRO '(' char_star_expr ')'
	  {
		$$ = new cmd_epoch_macro_thunk($3);
	  }
	| CMD_FIND_FILE '(' int_expr ',' int_expr ',' int_expr ',' int_expr ',' char_star_expr ')'
	  {
		$$ = new cmd_find_file_thunk($3, $5, $7, $9, $11);
	  }
	| CMD_SET_SCRIPT_VERSION '(' int_expr ')'
	  {
		$$ = new cmd_set_script_version_thunk($3);
	  }
	| CMD_SET_VALIDATION_LEVEL '(' int_expr ')'
	  {
		$$ = new cmd_set_validation_level_thunk($3);
	  }
	| EM_SET_SPEED '(' int_expr ')'
	  {
		$$ = new em_set_speed_thunk($3);
	  }
	| GRA_BEGIN_CONNECTION '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr opt_char_star_expr ')'
	  {
		$$ = new gra_begin_connection_thunk($3, $5, $7, $9, $11, $13, $14);
	  }
	| GRA_COLLAPSE_EXPAND '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new gra_collapse_expand_thunk($3, $5, $7, $9, $11, $13, $15);
	  }
	| GRA_DROP '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr opt_char_star_expr ')'
	  {
		$$ = new gra_drop_thunk($3, $5, $7, $9, $11, $13, $14);
	  }
	| GRA_END_CONNECTION '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr opt_char_star_expr ')'
	  {
		$$ = new gra_end_connection_thunk($3, $5, $7, $9, $11, $13, $14);
	  }
	| GRA_MODAL_CLICK '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr opt_char_star_expr ')'
	  {
		$$ = new gra_modal_click_thunk($3, $5, $7, $9, $11, $13, $14);
	  }
	| GRA_OPEN_NODE '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new gra_open_node_thunk($3, $5, $7, $9, $11, $13, $15);
	  }
	| GRA_PICKUP '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr opt_char_star_expr ')'
	  {
		$$ = new gra_pickup_thunk($3, $5, $7, $9, $11, $13, $14);
	  }
	| GRA_SELECT '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new gra_select_thunk($3, $5, $7, $9, $11, $13, $15);
	  }
	| GRA_SYMBOL_MENU '(' int_expr ',' int_expr ',' int_expr ',' double_expr ',' double_expr ',' char_star_expr opt_char_star_expr ')'
	  {
		$$ = new gra_symbol_menu_thunk($3, $5, $7, $9, $11, $13, $14);
	  }
	| STE_CMD_MOUSE '('  ')'
	  {
		$$ = new ste_cmd_mouse_thunk;
	  }
	| UI_VALIDATE_MATCH_COUNT '(' int_expr ')'
	  {
		$$ = new ui_validate_match_count_thunk($3);
	  }
	| UI_VALIDATE_NAMEDINT '(' char_star_expr ',' int_expr ')'
	  {
		$$ = new ui_validate_namedInt_thunk($3, $5);
	  }
	| UI_VALIDATE_NAMEDSTRING '(' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new ui_validate_namedString_thunk($3, $5);
	  }
	| UI_VALIDATE_OBJ '(' char_star_expr ',' char_star_expr ',' char_star_expr ')'
	  {
		$$ = new ui_validate_obj_thunk($3, $5, $7);
	  }
	| UI_VIEWER_VALIDATE '(' int_expr ',' char_star_expr ',' int_expr ')'
	  {
		$$ = new ui_viewer_validate_thunk($3, $5, $7);
	  }
	| DBG_PROMPT '(' char_star_expr ')'
	  {
		/* $$ = new dbg_prompt_thunk($3); */
	  }
	;

double_expr :
	  DOUBLE { $$ = new double_thunk($1); }
	  | INT { $$ = new double_thunk($1); }
	  | ZERO { $$ = new double_thunk(0); }
	;

int_expr :
	  CMD_REPORT_NODE_DEF '(' appTree_star_expr ')'
	    {
		$$ = new cmd_report_node_def_thunk($3);
	  }
	| CMD_SET_VALIDATION_GROUP '(' char_star_expr ')'
	  {
		$$ = new cmd_set_validation_group_thunk($3);
	  }
	| STE_CURSOR '(' appTree_star_expr ',' int_expr ',' int_expr ')'
	  {
		$$ = new ste_cursor_thunk($3, $5, $7);
	  }
	| STE_DO_REPLACE_REMOVE '(' appTree_star_expr ',' appTree_star_expr ',' char_star_expr ',' int_expr ',' int_expr ',' appTree_star_expr ',' appTree_star_expr ')'
	  {
		$$ = new ste_do_replace_remove_thunk($3, $5, $7, $9, $11, $13, $15);
	  }
	| STE_INSERT_TEXT '(' appTree_star_expr ',' char_star_expr ',' int_expr ')'
	  {
		$$ = new ste_insert_text_thunk($3, $5, $7);
	  }
	| STE_REGION '(' appTree_star_expr ',' int_expr ',' int_expr ',' appTree_star_expr ',' int_expr ',' int_expr ',' int_expr ')'
	  {
		$$ = new ste_region_thunk($3, $5, $7, $9, $11, $13, $15);
	  }
	| SYNCHRONIZE_NOW '(' char_star_expr ')'
	  {
		$$ = new synchronize_now_thunk($3);
	  }
	| VSYSTEM_FROM_SCRIPT '(' char_star_expr ')'
	  {
		$$ = new vsystem_from_script_thunk($3);
	  }
	| INT { $$ = new int_thunk($1); }
	| ZERO { $$ = new int_thunk(0); }
	;

appTree_star_expr :
	C_NODE '(' int_expr ',' char_star_expr ',' int_list ')'
	  {
		$$ = new C_node_thunk($3, $5, $7);
	  }
	| ZERO { $$ = new appTree_star_thunk(0); }
	;

int_list :
	  int_expr { $$ = new int_thunk_list($1); }
	| int_expr ',' int_list
	  {
		$3->push($1);
		$$ = $3;
	  }
	;

opt_char_star_expr :
	  { $$ = new char_star_thunk(0); }
	| ',' char_star_expr { $$ = $2; }
	;
%%

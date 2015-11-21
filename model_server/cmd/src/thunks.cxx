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
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstdio>
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include "thunks.h"
#include "msg.h"

#define DBG(x) 
#define DBG_START
#define DBG_END
#define DBGFINISH msg("done", normal_sev) << eom;

C_node_thunk::C_node_thunk(int_thunk * a1, char_star_thunk * a2, int_thunk_list *list) :
	arg1(a1),
	arg2(a2)
{
	int i = 0;
	for (; i < (sizeof(var_int_args)/sizeof(var_int_args[0])); ++i) {
		if (list->empty()) {
			var_int_args[i] = new int_thunk(0);
		} else {
			var_int_args[i] = list->top();
			list->pop();
		}
	}
	if (!list->empty()) {
		cerr << "catastrophe: Regression parse error: C_node called with more than " << i << " arguments!" << endl;
		exit(1);
	}
}


C_node_thunk::~C_node_thunk()
{
	delete arg1;
	delete arg2;
}

void
C_node_thunk::eval()
{
	DBG(C_nod);
	arg1->eval();
	arg2->eval();
	int const var_arr_size = sizeof(var_int_args)/sizeof(var_int_args[0]);
	for (int i = 0; i < var_arr_size; ++i) {
		var_int_args[i]->eval();
	}
	DBG_START
		cerr << " -- EXEC  C_node (" << arg1->result() << ',' << arg2->result();
		for (int j = 0; j < var_arr_size; ++j) {
			cerr << "," << var_int_args[j]->result();
			if (var_int_args[j]->result() == 0) {
				break;
			}
		}
		cerr << ") ; ... " << endl;
	DBG_END
	res = C_node(arg1->result(), arg2->result(),
		var_int_args[0]->result(),
		var_int_args[1]->result(),
		var_int_args[2]->result(),
		var_int_args[3]->result(),
		var_int_args[4]->result(),
		var_int_args[5]->result(),
		var_int_args[6]->result(),
		var_int_args[7]->result(),
		var_int_args[8]->result()
		);
	DBGFINISH
}


cmd_convert_window_command_thunk::cmd_convert_window_command_thunk(int_thunk * a1, char_star_thunk * a2, int_thunk * a3, int_thunk * a4) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4)
{}

cmd_convert_window_command_thunk::~cmd_convert_window_command_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
}

void
cmd_convert_window_command_thunk::eval()
{
	DBG(cmd_convert_window_command)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	DBG_START
		cerr << " -- EXEC  cmd_convert_window_command (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ") ; ... " << endl;
	DBG_END
	cmd_convert_window_command(arg1->result(), arg2->result(), arg3->result(), arg4->result());
	DBGFINISH
}


cmd_convert_window_report_thunk::cmd_convert_window_report_thunk(int_thunk * a1, char_star_thunk * a2, int_thunk * a3, int_thunk * a4) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4)
{}

cmd_convert_window_report_thunk::~cmd_convert_window_report_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
}

void
cmd_convert_window_report_thunk::eval()
{
	DBG(cmd_convert_window_report)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	DBG_START
		cerr << " -- EXEC  cmd_convert_window_report (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ") ; ... " << endl;
	DBG_END
	cmd_convert_window_report(arg1->result(), arg2->result(), arg3->result(), arg4->result());
	DBGFINISH
}


cmd_epoch_macro_thunk::cmd_epoch_macro_thunk(char_star_thunk * a1) :
	arg1(a1)
{}

cmd_epoch_macro_thunk::~cmd_epoch_macro_thunk()
{
	delete arg1;
}

void
cmd_epoch_macro_thunk::eval()
{
	DBG(cmd_epoch_macro)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  cmd_epoch_macro (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	cmd_epoch_macro((unsigned char *)arg1->result());
	DBGFINISH
}


cmd_find_file_thunk::cmd_find_file_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, int_thunk * a4, char_star_thunk * a5) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5)
{}

cmd_find_file_thunk::~cmd_find_file_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
}

void
cmd_find_file_thunk::eval()
{
	DBG(cmd_find_file)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	DBG_START
		cerr << " -- EXEC  cmd_find_file (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ") ; ... " << endl;
	DBG_END
	cmd_find_file(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result());
	DBGFINISH
}


cmd_report_node_def_thunk::cmd_report_node_def_thunk(appTree_star_thunk * a1) :
	arg1(a1)
{}

cmd_report_node_def_thunk::~cmd_report_node_def_thunk()
{
	delete arg1;
}

void
cmd_report_node_def_thunk::eval()
{
	DBG(cmd_report_node_def)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  cmd_report_node_def (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	cmd_report_node_def(arg1->result());
	DBGFINISH
}


cmd_set_script_version_thunk::cmd_set_script_version_thunk(int_thunk * a1) :
	arg1(a1)
{}

cmd_set_script_version_thunk::~cmd_set_script_version_thunk()
{
	delete arg1;
}

void
cmd_set_script_version_thunk::eval()
{
	DBG(cmd_set_script_version)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  cmd_set_script_version (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	cmd_set_script_version(arg1->result());
	DBGFINISH
}


cmd_set_validation_group_thunk::cmd_set_validation_group_thunk(char_star_thunk * a1) :
	arg1(a1)
{}

cmd_set_validation_group_thunk::~cmd_set_validation_group_thunk()
{
	delete arg1;
}

void
cmd_set_validation_group_thunk::eval()
{
	DBG(cmd_set_validation_group)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  cmd_set_validation_group (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	cmd_set_validation_group(arg1->result());
	DBGFINISH
}


cmd_set_validation_level_thunk::cmd_set_validation_level_thunk(int_thunk * a1) :
	arg1(a1)
{}

cmd_set_validation_level_thunk::~cmd_set_validation_level_thunk()
{
	delete arg1;
}

void
cmd_set_validation_level_thunk::eval()
{
	DBG(cmd_set_validation_level)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  cmd_set_validation_level (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	cmd_set_validation_level(arg1->result());
	DBGFINISH
}


em_set_speed_thunk::em_set_speed_thunk(int_thunk * a1) :
	arg1(a1)
{}

em_set_speed_thunk::~em_set_speed_thunk()
{
	delete arg1;
}

void
em_set_speed_thunk::eval()
{
	DBG(em_set_speed)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  em_set_speed (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	em_set_speed(arg1->result());
	DBGFINISH
}


gra_begin_connection_thunk::gra_begin_connection_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_begin_connection_thunk::~gra_begin_connection_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_begin_connection_thunk::eval()
{
	DBG(gra_begin_connection)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_begin_connection (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_begin_connection(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_collapse_expand_thunk::gra_collapse_expand_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_collapse_expand_thunk::~gra_collapse_expand_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_collapse_expand_thunk::eval()
{
	DBG(gra_collapse_expand)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_collapse_expand (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	gra_collapse_expand(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_drop_thunk::gra_drop_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_drop_thunk::~gra_drop_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_drop_thunk::eval()
{
	DBG(gra_drop)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_drop (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_drop(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_end_connection_thunk::gra_end_connection_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_end_connection_thunk::~gra_end_connection_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_end_connection_thunk::eval()
{
	DBG(gra_end_connection)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_end_connection (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_end_connection(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_modal_click_thunk::gra_modal_click_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_modal_click_thunk::~gra_modal_click_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_modal_click_thunk::eval()
{
	DBG(gra_modal_click)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_modal_click (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_modal_click(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_open_node_thunk::gra_open_node_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_open_node_thunk::~gra_open_node_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_open_node_thunk::eval()
{
	DBG(gra_open_node)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_open_node (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_open_node(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_pickup_thunk::gra_pickup_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_pickup_thunk::~gra_pickup_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_pickup_thunk::eval()
{
	DBG(gra_pickup)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_pickup (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_pickup(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_select_thunk::gra_select_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_select_thunk::~gra_select_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_select_thunk::eval()
{
	DBG(gra_select)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_select (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_select(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


gra_symbol_menu_thunk::gra_symbol_menu_thunk(int_thunk * a1, int_thunk * a2, int_thunk * a3, double_thunk * a4, double_thunk * a5, char_star_thunk * a6, char_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

gra_symbol_menu_thunk::~gra_symbol_menu_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
gra_symbol_menu_thunk::eval()
{
	DBG(gra_symbol_menu)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  gra_symbol_menu (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	gra_symbol_menu(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


ste_cmd_mouse_thunk::ste_cmd_mouse_thunk()
{}

ste_cmd_mouse_thunk::~ste_cmd_mouse_thunk()
{
}

void
ste_cmd_mouse_thunk::eval()
{
	DBG(ste_cmd_mouse)
	DBG_START
		cerr << " -- EXEC  ste_cmd_mouse (" << ") ; ... " << endl;
	DBG_END
	ste_cmd_mouse();
	DBGFINISH
}


ste_cursor_thunk::ste_cursor_thunk(appTree_star_thunk * a1, int_thunk * a2, int_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ste_cursor_thunk::~ste_cursor_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
}

void
ste_cursor_thunk::eval()
{
	DBG(ste_cursor)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ste_cursor (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ste_cursor(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


ste_do_replace_remove_thunk::ste_do_replace_remove_thunk(appTree_star_thunk * a1, appTree_star_thunk * a2, char_star_thunk * a3, int_thunk * a4, int_thunk * a5, appTree_star_thunk * a6, appTree_star_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

ste_do_replace_remove_thunk::~ste_do_replace_remove_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
ste_do_replace_remove_thunk::eval()
{
	DBG(ste_do_replace_remove)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  ste_do_replace_remove (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	ste_do_replace_remove(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


ste_insert_text_thunk::ste_insert_text_thunk(appTree_star_thunk * a1, char_star_thunk * a2, int_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ste_insert_text_thunk::~ste_insert_text_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
}

void
ste_insert_text_thunk::eval()
{
	DBG(ste_insert_text)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ste_insert_text (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ste_insert_text(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


ste_region_thunk::ste_region_thunk(appTree_star_thunk * a1, int_thunk * a2, int_thunk * a3, appTree_star_thunk * a4, int_thunk * a5, int_thunk * a6, int_thunk * a7) :
	arg1(a1),
	arg2(a2),
	arg3(a3),
	arg4(a4),
	arg5(a5),
	arg6(a6),
	arg7(a7)
{}

ste_region_thunk::~ste_region_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
	delete arg4;
	delete arg5;
	delete arg6;
	delete arg7;
}

void
ste_region_thunk::eval()
{
	DBG(ste_region)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	arg4->eval();
	arg5->eval();
	arg6->eval();
	arg7->eval();
	DBG_START
		cerr << " -- EXEC  ste_region (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ',' << arg4->result() << ',' << arg5->result() << ',' << arg6->result() << ',' << arg7->result() << ") ; ... " << endl;
	DBG_END
	ste_region(arg1->result(), arg2->result(), arg3->result(), arg4->result(), arg5->result(), arg6->result(), arg7->result());
	DBGFINISH
}


synchronize_now_thunk::synchronize_now_thunk(char_star_thunk * a1) :
	arg1(a1)
{}

synchronize_now_thunk::~synchronize_now_thunk()
{
	delete arg1;
}

void
synchronize_now_thunk::eval()
{
	DBG(syncronize_now)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  synchronize_now (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	synchronize_now(arg1->result());
	DBGFINISH
}


ui_addselect_thunk::ui_addselect_thunk(char_star_thunk * a1, char_star_thunk * a2) :
	arg1(a1),
	arg2(a2)
{}

ui_addselect_thunk::~ui_addselect_thunk()
{
	delete arg1;
	delete arg2;
}

void
ui_addselect_thunk::eval()
{
	DBG(ui_add_select)
	arg1->eval();
	arg2->eval();
	DBG_START
		cerr << " -- EXEC  ui_addselect (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	ui_addselect(arg1->result(), arg2->result());
	DBGFINISH
}


ui_button_event_thunk::ui_button_event_thunk(char_star_thunk * a1, int_thunk * a2, int_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ui_button_event_thunk::~ui_button_event_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
}

void
ui_button_event_thunk::eval()
{
	DBG(ui_button_event)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ui_button_event (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ui_button_event(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


ui_list_action_thunk::ui_list_action_thunk(char_star_thunk * a1) :
	arg1(a1)
{}

ui_list_action_thunk::~ui_list_action_thunk()
{
	delete arg1;
}

void
ui_list_action_thunk::eval()
{
	DBG(ui_list_action)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  ui_list_action (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	ui_list_action(arg1->result());
	DBGFINISH
}


ui_push_button_thunk::ui_push_button_thunk(char_star_thunk * a1) :
	arg1(a1)
{}

ui_push_button_thunk::~ui_push_button_thunk()
{
	delete arg1;
}

void
ui_push_button_thunk::eval()
{
	DBG(ui_push_button)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  ui_push_button (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	ui_push_button(arg1->result());
	DBGFINISH
}


ui_replace_text_thunk::ui_replace_text_thunk(char_star_thunk * a1, char_star_thunk * a2, int_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ui_replace_text_thunk::~ui_replace_text_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
}

void
ui_replace_text_thunk::eval()
{
	DBG(ui_replace_text)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ui_replace_text (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ui_replace_text(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


ui_select_gadget_thunk::ui_select_gadget_thunk(char_star_thunk * a1, char_star_thunk * a2) :
	arg1(a1),
	arg2(a2)
{}

ui_select_gadget_thunk::~ui_select_gadget_thunk()
{
	delete arg1;
	delete arg2;
}

void
ui_select_gadget_thunk::eval()
{
	DBG(ui_select_gadget)
	arg1->eval();
	arg2->eval();
	DBG_START
		cerr << " -- EXEC  ui_select_gadget (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	ui_select_gadget(arg1->result(), arg2->result());
	DBGFINISH
}


ui_select_label_thunk::ui_select_label_thunk(char_star_thunk * a1, char_star_thunk * a2, int_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ui_select_label_thunk::~ui_select_label_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;

}

void
ui_select_label_thunk::eval()
{
	DBG(ui_select_label)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ui_select_label (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ui_select_label(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


ui_select_list_thunk::ui_select_list_thunk(char_star_thunk * a1, char_star_thunk * a2) :
	arg1(a1),
	arg2(a2)
{}

ui_select_list_thunk::~ui_select_list_thunk()
{
	delete arg1;
	delete arg2;
}

void
ui_select_list_thunk::eval()
{
	DBG(ui_select_list)
	arg1->eval();
	arg2->eval();
	DBG_START
		cerr << " -- EXEC  ui_select_list (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	ui_select_list(arg1->result(), arg2->result());
	DBGFINISH
}


ui_select_top_thunk::ui_select_top_thunk(char_star_thunk * a1, int_thunk * a2) :
	arg1(a1),
	arg2(a2)
{}

ui_select_top_thunk::~ui_select_top_thunk()
{
	delete arg1;
	delete arg2;
}

void
ui_select_top_thunk::eval()
{
	DBG(ui_select_top)
	arg1->eval();
	arg2->eval();
	DBG_START
		cerr << " -- EXEC  ui_select_top (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	ui_select_top(arg1->result(), arg2->result());
	DBGFINISH
}


ui_select_viewer_thunk::ui_select_viewer_thunk(int_thunk * a1) :
	arg1(a1)
{}

ui_select_viewer_thunk::~ui_select_viewer_thunk()
{
	delete arg1;
}

void
ui_select_viewer_thunk::eval()
{
	DBG(ui_select_viewer)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  ui_select_viewer (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	ui_select_viewer(arg1->result());
	DBGFINISH
}


ui_validate_match_count_thunk::ui_validate_match_count_thunk(int_thunk * a1) :
	arg1(a1)
{}

ui_validate_match_count_thunk::~ui_validate_match_count_thunk()
{
	delete arg1;
}

void
ui_validate_match_count_thunk::eval()
{
	DBG(ui_validate_match_count)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  ui_validate_match_count (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	ui_validate_match_count(arg1->result());
	DBGFINISH
}


ui_validate_namedInt_thunk::ui_validate_namedInt_thunk(char_star_thunk * a1, int_thunk * a2) :
	arg1(a1),
	arg2(a2)
{}

ui_validate_namedInt_thunk::~ui_validate_namedInt_thunk()
{
	delete arg1;
	delete arg2;
}

void
ui_validate_namedInt_thunk::eval()
{
	DBG(ui_validate_namedInt)
	arg1->eval();
	arg2->eval();
	DBG_START
		cerr << " -- EXEC  ui_validate_namedInt (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	ui_validate_namedInt(arg1->result(), arg2->result());
	DBGFINISH
}


ui_validate_namedString_thunk::ui_validate_namedString_thunk(char_star_thunk * a1, char_star_thunk * a2) :
	arg1(a1),
	arg2(a2)
{}

ui_validate_namedString_thunk::~ui_validate_namedString_thunk()
{
	delete arg1;
	delete arg2;
}

void
ui_validate_namedString_thunk::eval()
{
	DBG(ui_validate_namedString)
	arg1->eval();
	arg2->eval();
	DBG_START
		cerr << " -- EXEC  ui_validate_namedString (" << arg1->result() << ',' << arg2->result() << ") ; ... " << endl;
	DBG_END
	ui_validate_namedString(arg1->result(), arg2->result());
	DBGFINISH
}


ui_validate_obj_thunk::ui_validate_obj_thunk(char_star_thunk * a1, char_star_thunk * a2, char_star_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ui_validate_obj_thunk::~ui_validate_obj_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
}

void
ui_validate_obj_thunk::eval()
{
	DBG(ui_validate_obj)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ui_validate_obj (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ui_validate_obj(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


ui_viewer_validate_thunk::ui_viewer_validate_thunk(int_thunk * a1, char_star_thunk * a2, int_thunk * a3) :
	arg1(a1),
	arg2(a2),
	arg3(a3)
{}

ui_viewer_validate_thunk::~ui_viewer_validate_thunk()
{
	delete arg1;
	delete arg2;
	delete arg3;
}

void
ui_viewer_validate_thunk::eval()
{
	DBG(ui_viewer_validate)
	arg1->eval();
	arg2->eval();
	arg3->eval();
	DBG_START
		cerr << " -- EXEC  ui_viewer_validate (" << arg1->result() << ',' << arg2->result() << ',' << arg3->result() << ") ; ... " << endl;
	DBG_END
	ui_viewer_validate(arg1->result(), arg2->result(), arg3->result());
	DBGFINISH
}


vsystem_from_script_thunk::vsystem_from_script_thunk(char_star_thunk *a1) :
	arg1(a1)
{
}

vsystem_from_script_thunk::~vsystem_from_script_thunk()
{
	delete arg1;
}

void
vsystem_from_script_thunk::eval()
{
	DBG(vsystem_from_script)
	arg1->eval();
	DBG_START
		cerr << " -- EXEC  vsystem_from_script (" << arg1->result() << ") ; ... " << endl;
	DBG_END
	res = vsystem_from_script(arg1->result());
	DBGFINISH
}

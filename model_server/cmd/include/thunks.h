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
#include "cmd_decls.h" 
#include "cmd_interp.h"


class C_node_thunk : public appTree_star_thunk
{
public:
	C_node_thunk();
	C_node_thunk(int_thunk *, char_star_thunk *, int_thunk_list*);
	virtual ~C_node_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	char_star_thunk * arg2;

	/* C_node is called with a variable number of arguments */
	/* Hopefully, these will be enough! */
	int_thunk * var_int_args[9];
};


class cmd_convert_window_command_thunk : public void_thunk
{
public:
	cmd_convert_window_command_thunk(int_thunk *, char_star_thunk *, int_thunk *, int_thunk *);
	virtual ~cmd_convert_window_command_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	char_star_thunk * arg2;
	int_thunk * arg3;
	int_thunk * arg4;
};


class cmd_convert_window_report_thunk : public void_thunk
{
public:
	cmd_convert_window_report_thunk(int_thunk *, char_star_thunk *, int_thunk *, int_thunk *);
	virtual ~cmd_convert_window_report_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	char_star_thunk * arg2;
	int_thunk * arg3;
	int_thunk * arg4;
};


class cmd_epoch_macro_thunk : public void_thunk
{
public:
	cmd_epoch_macro_thunk(char_star_thunk *);
	virtual ~cmd_epoch_macro_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
};


class cmd_find_file_thunk : public void_thunk
{
public:
	cmd_find_file_thunk(int_thunk *, int_thunk *, int_thunk *, int_thunk *, char_star_thunk *);
	virtual ~cmd_find_file_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	int_thunk * arg4;
	char_star_thunk * arg5;
};


class cmd_report_node_def_thunk : public int_thunk
{
public:
	cmd_report_node_def_thunk(appTree_star_thunk *);
	virtual ~cmd_report_node_def_thunk();
	virtual void eval();
private:
	appTree_star_thunk * arg1;
};


class cmd_set_script_version_thunk : public void_thunk
{
public:
	cmd_set_script_version_thunk(int_thunk *);
	virtual ~cmd_set_script_version_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
};


class cmd_set_validation_group_thunk : public int_thunk
{
public:
	cmd_set_validation_group_thunk(char_star_thunk *);
	virtual ~cmd_set_validation_group_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
};


class cmd_set_validation_level_thunk : public void_thunk
{
public:
	cmd_set_validation_level_thunk(int_thunk *);
	virtual ~cmd_set_validation_level_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
};


class em_set_speed_thunk : public void_thunk
{
public:
	em_set_speed_thunk(int_thunk *);
	virtual ~em_set_speed_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
};


class gra_begin_connection_thunk : public void_thunk
{
public:
	gra_begin_connection_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_begin_connection_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_collapse_expand_thunk : public void_thunk
{
public:
	gra_collapse_expand_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_collapse_expand_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_drop_thunk : public void_thunk
{
public:
	gra_drop_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_drop_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_end_connection_thunk : public void_thunk
{
public:
	gra_end_connection_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_end_connection_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_modal_click_thunk : public void_thunk
{
public:
	gra_modal_click_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_modal_click_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_open_node_thunk : public void_thunk
{
public:
	gra_open_node_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_open_node_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_pickup_thunk : public void_thunk
{
public:
	gra_pickup_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_pickup_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_select_thunk : public void_thunk
{
public:
	gra_select_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_select_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class gra_symbol_menu_thunk : public void_thunk
{
public:
	gra_symbol_menu_thunk(int_thunk *, int_thunk *, int_thunk *, double_thunk *, double_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~gra_symbol_menu_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	double_thunk * arg4;
	double_thunk * arg5;
	char_star_thunk * arg6;
	char_star_thunk * arg7;
};


class ste_cmd_mouse_thunk : public void_thunk
{
public:
	ste_cmd_mouse_thunk();
	virtual ~ste_cmd_mouse_thunk();
	virtual void eval();
private:
};


class ste_cursor_thunk : public int_thunk
{
public:
	ste_cursor_thunk(appTree_star_thunk *, int_thunk *, int_thunk *);
	virtual ~ste_cursor_thunk();
	virtual void eval();
private:
	appTree_star_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
};


class ste_do_replace_remove_thunk : public int_thunk
{
public:
	ste_do_replace_remove_thunk(appTree_star_thunk *, appTree_star_thunk *, char_star_thunk *, int_thunk *, int_thunk *, appTree_star_thunk *, appTree_star_thunk *);
	virtual ~ste_do_replace_remove_thunk();
	virtual void eval();
private:
	appTree_star_thunk * arg1;
	appTree_star_thunk * arg2;
	char_star_thunk * arg3;
	int_thunk * arg4;
	int_thunk * arg5;
	appTree_star_thunk * arg6;
	appTree_star_thunk * arg7;
};


class ste_insert_text_thunk : public int_thunk
{
public:
	ste_insert_text_thunk(appTree_star_thunk *, char_star_thunk *, int_thunk *);
	virtual ~ste_insert_text_thunk();
	virtual void eval();
private:
	appTree_star_thunk * arg1;
	char_star_thunk * arg2;
	int_thunk * arg3;
};


class ste_region_thunk : public int_thunk
{
public:
	ste_region_thunk(appTree_star_thunk *, int_thunk *, int_thunk *, appTree_star_thunk *, int_thunk *, int_thunk *, int_thunk *);
	virtual ~ste_region_thunk();
	virtual void eval();
private:
	appTree_star_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
	appTree_star_thunk * arg4;
	int_thunk * arg5;
	int_thunk * arg6;
	int_thunk * arg7;
};


class synchronize_now_thunk : public int_thunk
{
public:
	synchronize_now_thunk(char_star_thunk *);
	virtual ~synchronize_now_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
};


class ui_addselect_thunk : public Widget_thunk
{
public:
	ui_addselect_thunk(char_star_thunk *, char_star_thunk *);
	virtual ~ui_addselect_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
};


class ui_button_event_thunk : public Widget_thunk
{
public:
	ui_button_event_thunk(char_star_thunk *, int_thunk *, int_thunk *);
	virtual ~ui_button_event_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	int_thunk * arg2;
	int_thunk * arg3;
};


class ui_list_action_thunk : public Widget_thunk
{
public:
	ui_list_action_thunk(char_star_thunk *);
	virtual ~ui_list_action_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
};


class ui_push_button_thunk : public Widget_thunk
{
public:
	ui_push_button_thunk(char_star_thunk *);
	virtual ~ui_push_button_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
};


class ui_replace_text_thunk : public Widget_thunk
{
public:
	ui_replace_text_thunk(char_star_thunk *, char_star_thunk *, int_thunk *);
	virtual ~ui_replace_text_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
	int_thunk * arg3;
};


class ui_select_gadget_thunk : public Widget_thunk
{
public:
	ui_select_gadget_thunk(char_star_thunk *, char_star_thunk *);
	virtual ~ui_select_gadget_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
};


class ui_select_label_thunk : public Widget_thunk
{
public:
	ui_select_label_thunk(char_star_thunk *, char_star_thunk *, int_thunk *);
	virtual ~ui_select_label_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
	int_thunk * arg3;
};


class ui_select_list_thunk : public Widget_thunk
{
public:
	ui_select_list_thunk(char_star_thunk *, char_star_thunk *);
	virtual ~ui_select_list_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
};


class ui_select_top_thunk : public Widget_thunk
{
public:
	ui_select_top_thunk(char_star_thunk *, int_thunk *);
	virtual ~ui_select_top_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	int_thunk * arg2;
};


class ui_select_viewer_thunk : public viewer_star_thunk
{
public:
	ui_select_viewer_thunk(int_thunk *);
	virtual ~ui_select_viewer_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
};


class ui_validate_match_count_thunk : public void_thunk
{
public:
	ui_validate_match_count_thunk(int_thunk *);
	virtual ~ui_validate_match_count_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
};


class ui_validate_namedInt_thunk : public void_thunk
{
public:
	ui_validate_namedInt_thunk(char_star_thunk *, int_thunk *);
	virtual ~ui_validate_namedInt_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	int_thunk * arg2;
};


class ui_validate_namedString_thunk : public void_thunk
{
public:
	ui_validate_namedString_thunk(char_star_thunk *, char_star_thunk *);
	virtual ~ui_validate_namedString_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
};


class ui_validate_obj_thunk : public void_thunk
{
public:
	ui_validate_obj_thunk(char_star_thunk *, char_star_thunk *, char_star_thunk *);
	virtual ~ui_validate_obj_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
	char_star_thunk * arg2;
	char_star_thunk * arg3;
};


class ui_viewer_validate_thunk : public void_thunk
{
public:
	ui_viewer_validate_thunk(int_thunk *, char_star_thunk *, int_thunk *);
	virtual ~ui_viewer_validate_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	char_star_thunk * arg2;
	int_thunk * arg3;
};


class vsystem_from_script_thunk : public int_thunk
{
public:
	vsystem_from_script_thunk(char_star_thunk *);
	virtual ~vsystem_from_script_thunk();
	virtual void eval();
private:
	char_star_thunk * arg1;
};



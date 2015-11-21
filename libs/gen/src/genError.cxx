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
#include <msg.h>
#include <systemMessages.h>
#include <genError.h>
#include <objRelation.h>
#include <messages.h>

void gtPushButton_next_help_context(const char* context);

void logger_start_transaction();
void logger_end_transaction();

static int conv_error_level = 1; // See gen_conv_error_handler_new(), below.

int ok_to_delete_old_pmod = 1;

struct msg_node			// See genError::print(), below.
{
    int code;
    const char *msg;
};

static msg_node msg_table[] =
{
    { ERR_INPUT,		"ERR_INPUT" },
    { ERR_FAIL,			"ERR_FAIL" },
    { ERR_ALLOC,		"ERR_ALLOC" },
    { NOT_IMPLEMENTED_YET,	"NOT_IMPLEMENTED_YET" },
    { ERR_ASSERT,		"ERR_ASSERT" },
    { EPOCH_TIME_OUT,		"DISCOVER <-> Epoch timeout expired" },
};

static const int msg_table_size = sizeof(msg_table) / sizeof(msg_node);


void genError::print()
{
    logger_start_transaction();
    for(int i = 0; i < msg_table_size; ++i)
    {
	if(msg_table[i].code == code)
	{
	    msg("diagnostic: code $1 ($2)", error_sev) << code << eoarg << msg_table[i].msg << eom;
            logger_end_transaction();
	    return;
	}
    }
    msg("diagnostic: code $1", error_sev) << code << eom;
    logger_end_transaction();
}


static void tell_the_user_the_bad_news()
{
    static int error_dialog_popped_once = 0;

    // Error dialog should be popped once only
    if(!error_dialog_popped_once)
    {
        error_dialog_popped_once = 1;
	msg("DISCOVER has failed an internal consistency check.\nYou may want to exit and restart DISCOVER.", catastrophe_sev) << eom;
    }
}

void gen_error_handler(
    const char* name, int code, const char* filename, int linenum)
{
    logger_start_transaction();
    genError *_te = new genError(code);

    if(name == NULL)
	name = "";

    msg("DISCOVER ERROR: info follows.", catastrophe_sev) << eom; 
    _te->print(); 
    if(filename)
	msg("Location: $1:$2", catastrophe_sev) << filename << eoarg << linenum << eom;

    tell_the_user_the_bad_news();

    logger_end_transaction();

    // in case of ERR_ASSERT, disable overwrite of old pmod - ljb.
    if (code == ERR_ASSERT)
	ok_to_delete_old_pmod = 0;
    
    throw_error(int(_te));
}


void gen_conv_error_handler_new(
    const char * type, const char* name, int line_no, void* obj)
{
    if(conv_error_level)
    {
	msg("$1@$2: conversion to $3 ", catastrophe_sev) << name << eoarg << line_no << eoarg << type << eom;
	obj_prt_obj(obj);

	if(conv_error_level > 1)
	{
	    conv_error_level = 0;
	    node_prt((Obj*)obj);
	    conv_error_level = 2;
	}
    }
}

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
/* classes for built-in types and conversions */

#include "cmd_interp.h"

stmt_list_thunk::stmt_list_thunk(thunk_base *stmt) :
	head(0),
	next_stmt(0)
{
	push_thunk(stmt);
}

stmt_list_thunk::~stmt_list_thunk()
{
	thunk_node *next;
	while (head) {
		next = head->next;
		delete head->data;
		delete head;
		head = next;
	}
}

void
stmt_list_thunk::eval()
{
	while (has_next_stmt()) {
		exec_next_stmt();
	}
}

void
stmt_list_thunk::exec_next_stmt()
{
	if (next_stmt) {
		thunk_base * t = next_stmt->data;
		next_stmt = next_stmt->next;
		t->eval();
	}
}

int
stmt_list_thunk::has_next_stmt() const
{
	return next_stmt != 0;
}

void
stmt_list_thunk::abort()
{
	next_stmt = 0;
}

void
stmt_list_thunk::push_thunk(thunk_base *t)
{
	// really could move some of this into a thunk_node constructor
	thunk_node *new_node = new thunk_node;
	new_node->data = t;
	new_node->next = head;
	head = new_node;

	// This is the strange side-effect:
	next_stmt = head;
}


stmt_list_thunk *cmd_stmt_list_ptr = 0;

char_star_thunk::char_star_thunk()
{
}

char_star_thunk::char_star_thunk(char * a1) :
	res(a1)
{}

void
char_star_thunk::eval()
{
}

char *
char_star_thunk::result()
{
	return res;
}


if_thunk::if_thunk(int_thunk *a1, thunk_base *a2) :
	arg1(a1),
	arg2(a2)
{
}

if_thunk::~if_thunk()
{
	delete arg1;
	delete arg2;
}

void
if_thunk::eval()
{
	arg1->eval();
	if (arg1->result()) {
		arg2->eval();
	}
}

double_thunk::double_thunk(double d) :
	res(d)
{
}

void
double_thunk::eval()
{
}

int_thunk::int_thunk()
{
}

int_thunk::int_thunk(int i) :
	res(i)
{
}

void
int_thunk::eval()
{
}


int_thunk_list::int_thunk_list(int_thunk *it) :
	head(new list_node(0,it))
{
}

int_thunk_list::~int_thunk_list()
{
	while (!empty()) {
		pop();
	}
}

void
int_thunk_list::push(int_thunk *it)
{
	list_node *newnode = new list_node(head,it);
	head = newnode;
}

void
int_thunk_list::pop()
{
	if (!empty()) {  // could be an assert
		list_node *tmp = head->next;
		delete head;
		head = tmp;
	}
}

int
int_thunk_list::empty() const
{
	return (head == 0);
}

int_thunk*
int_thunk_list::top() const
{
	return head->data;
}

appTree_star_thunk::appTree_star_thunk()
{
}

appTree_star_thunk::appTree_star_thunk(appTree *atp) :
	res(atp)
{
}

void
appTree_star_thunk::eval()
{
}

appTree *
appTree_star_thunk::result()
{
	return res;
}

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

class thunk_base
{
public:
	virtual void eval() = 0;
};

class stmt_list_thunk : public thunk_base
{
public:
	stmt_list_thunk(thunk_base *);
	~stmt_list_thunk();
	virtual void eval();
	void exec_next_stmt();
	void abort();
	int has_next_stmt() const;
	void push_thunk(thunk_base *);
private:
	struct thunk_node
	{
		thunk_node *next;
		thunk_base *data;
	};
	thunk_node *head;
	thunk_node *next_stmt;
};

extern stmt_list_thunk *cmd_stmt_list_ptr;



class int_thunk : public thunk_base
{
public:
	int_thunk();
	int_thunk(int);
	virtual void eval();
	virtual int result() {return res;}
protected:
	int res;
};

class int_thunk_list
{
public:
	int_thunk_list(int_thunk *);
	virtual ~int_thunk_list();
	void push(int_thunk *);
	void pop();
	int empty() const;
	int_thunk *top() const;
private:
	struct list_node
	{
		list_node(list_node *a, int_thunk *b) : next(a), data(b) {}
		list_node *next;
		int_thunk *data;
	};
	list_node *head;
};

extern stmt_list_thunk *cmd_stmt_list_ptr;

class if_thunk : public thunk_base
{
public:
	if_thunk(int_thunk *, thunk_base *);
	virtual ~if_thunk();
	virtual void eval();
private:
	int_thunk * arg1;
	thunk_base * arg2;
};


class double_thunk : public thunk_base
{
public:
	double_thunk(double);
	virtual void eval();
	virtual double result() {return res;}
protected:
	double res;
};

class void_thunk : public thunk_base
{
};

class char_star_thunk : public thunk_base
{
public:
	char_star_thunk();
	char_star_thunk(char *);
	void eval();
	char * result();
protected:
	char * res;
};

class appType_thunk : public thunk_base
{
public:
	appType result();
protected:
	appType res;
};

class appTree_star_thunk : public thunk_base
{
public:
	appTree_star_thunk();
	appTree_star_thunk(appTree *);
	virtual void eval();

	appTree * result();
protected:
	appTree * res;
};

class viewer_star_thunk : public thunk_base
{
public:
	viewer * result();
protected:
	viewer * res;
};


class Widget_thunk : public thunk_base
{
public:
	Widget result();
protected:
	Widget res;
};

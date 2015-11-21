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
// TODO: made from a copy in /paraset/src/options_filter.
// split to use different messaging output and setech_utils.
// MUST RECONCILE VERSION IN /paraset/src/options_filter


#include <cctype>
#include <iostream>
#include "message.h"
#include "setech_utils.h"

#define OPTIONS_FILE "/lib/exec_options.dat"

#include "option_filter.h"

enum read_states {
	SKIPPING_SPACES,
	READING_EXEC_NAME,
	READING_FIRST_CHAR,
	READING_OPTION,
	SKIPPING_WORD,
	START_READING_ARGUMENT
};

class Option {
public:
    Option(char *opt, int args);
    virtual ~Option();

    char   *option;
    int    num_args;
    Option *next;
};

class ExecOptions {
public:
    ExecOptions(char *name);
    virtual ~ExecOptions();
    
    int                MatchOption(char *buf);
    void               AddOption(char *opt, int num_args);
    static ExecOptions *FindExecutable(char *name);
    
    char        *exe_name;
    Option      *options;
    ExecOptions *next;
};

ExecOptions *execs = NULL;

//------------------------------------------------------------------------------

Option::Option(char *opt, int args) :
	num_args(args),
	next(NULL)
{
    option   = strdup(opt);
}

Option::~Option()
{
    if (option)
    	free(option);
    if (next)
    	delete next;
}

//------------------------------------------------------------------------------

ExecOptions::ExecOptions(char *name) :
	options(NULL),
	next(NULL)
{
    exe_name = strdup(name);
}

ExecOptions::~ExecOptions()
{
    if (exe_name)
	free(exe_name);
    if (options)
    	delete options;
    if (next)
    	delete next;
}

void ExecOptions::AddOption(char *opt, int num_args)
{
    Option *o = new Option(opt, num_args);
    if(options == NULL){
	options = o;
	return;
    }
    Option *cur = options;
    while(cur->next != NULL)
	cur = cur->next;
    cur->next = o;
}

int ExecOptions::MatchOption(char *buf)
{
    Option *cur = options;
    while(cur != NULL){
	if(strcmp(cur->option, buf) == 0)
	    return cur->num_args;
	cur = cur->next;
    }
    return -1;
}

ExecOptions *ExecOptions::FindExecutable(char *name)
{
    ExecOptions *cur = execs;
    while(cur != NULL){
	if(strcmp(cur->exe_name, name) == 0)
	    return cur;
	cur = cur->next;
    }
    return NULL;
}

//------------------------------------------------------------------------------

void ReadSingleOption(ExecOptions *exec, char *buf)
{
    char opt[1024];

    char *p = buf;
    while(*p != 0 && isspace(*p))
	p++;
    char *p1 = opt;
    while(*p != 0 && !isspace(*p))
	*p1++ = *p++;
    *p1 = 0;
    int num_args = 0;
    if(*p != 0){
	while(*p != 0 && isspace(*p))
	    p++;
	if(*p != 0){
	    num_args = atoi(p);
	}
    }
    exec->AddOption(opt, num_args);
}

void AddExecToList(ExecOptions *exec)
{
    if(execs == NULL){
	execs = exec;
	return;
    }
    ExecOptions *cur = execs;
    while(cur->next != NULL)
	cur = cur->next;
    cur->next = exec;
}

ExecOptions *DefineNewExecutable(char *buf)
{
    char exe_name[1024];

    char *p = &buf[1];
    while(*p != 0 && isspace(*p))
	p++;
    char *p1 = exe_name;
    while(*p != 0 && !isspace(*p))
	*p1++ = *p++;
    *p1 = 0;
    ExecOptions *ret_val = new ExecOptions(exe_name);
    return ret_val;
}

int ReadOptionFile(void)
{
    char const *pset_home = setech_utils::psethome();
    int len           = strlen(pset_home) + strlen(OPTIONS_FILE) + 1;
    char *option_file = (char *)malloc(len);
    strcpy(option_file, pset_home);
    char *p = option_file + strlen(pset_home);
    strcpy(p, OPTIONS_FILE);

    char buffer[1024];
    FILE *f = fopen(option_file, "r");
    if(f == NULL) {
	msgid("Unable to read options description file") << option_file << eom;
    	free(option_file);
	return -1;
    }
    free(option_file);
    ExecOptions *current_exec = NULL;
    while(fgets(buffer, 1024, f) != NULL){
	if(buffer[0] == ':'){
	    current_exec = DefineNewExecutable(buffer);
	    AddExecToList(current_exec);
	} else {
	    if(current_exec == NULL){
		msgid("No executable defined in options file") << eom;
		return -1;
	    }
	    ReadSingleOption(current_exec, buffer);
	}
    }
    fclose(f);
    return 0;
}

//------------------------------------------------------------------------------

char *ReadExecName(istream &input, ostream &output)
{
    char buffer[2048];
    char *p = buffer;

    enum read_states state = SKIPPING_SPACES;

    while(input.good()){
	int ch = input.get();
	if(ch < 0)
	    break;
	output.put((char)ch);
	if(state == SKIPPING_SPACES){
	    if(!isspace(ch))
		state = READING_EXEC_NAME;
	} 
	if(state == READING_EXEC_NAME){
	    if(isspace(ch)){
		*p = 0;
		break;
	    }
	    if(isupper(ch))
		*p++ = tolower(ch);
	    else
		*p++ = ch;
	}
    }
    // Remove all path (if present) from executable name
    p = strrchr(buffer, '\\');
    if(p == NULL)
	p = buffer;
    else
	p++;
    char *ret_val = strdup(p);
    return ret_val;
}

void ReadOptions(ExecOptions *exec, istream &input, ostream &output)
{
    char buffer[2048];
    char *p;

    int state     = SKIPPING_SPACES;
    int sub_state = 0;  // used to indicate that word is enclosed in '"'
    int num_arg   = 0;
    while(input.good()){
	int ch = input.get();
	if(ch < 0)
	    break;
	if(state == SKIPPING_SPACES){
	    if(!isspace(ch))
		state = READING_FIRST_CHAR;
	    else
		output.put((char)ch);
	} 
	if(state == READING_FIRST_CHAR){
	    if(ch == '/' || ch == '-'){
		output.put((char)'-');
		state     = READING_OPTION;
		buffer[0] = 0;
		p         = buffer;
		continue;
	    } else {
		state = SKIPPING_WORD;
	    }
	}
	if(state == SKIPPING_WORD){
	    output.put((char)ch);
	    if(isspace(ch))
		if(sub_state == 0)
		    state = SKIPPING_SPACES;
	    if(ch == '"')
		if(sub_state == 0)
		    sub_state = 1;
		else
		    sub_state = 0;
	}
	if(state == READING_OPTION){
	    if(isspace(ch)){
		*p      = 0;
		num_arg = exec->MatchOption(buffer);
		if(num_arg <= 0){
		    output.put((char)ch);
		    state = SKIPPING_SPACES;
		}
		if(num_arg > 0)
		    state = START_READING_ARGUMENT;
	    } else {
		output.put((char)ch);
		*p++    = ch;
		*p      = 0;
		num_arg = exec->MatchOption(buffer);
		if(num_arg == 0)
		    state = SKIPPING_WORD;
		if(num_arg > 0)
		    state = START_READING_ARGUMENT;
	    }
	    continue;
	}
	if(state == START_READING_ARGUMENT){
	    if(isspace(ch))
		continue;
	    if(ch == '/' || ch == '-'){  // do not forget to output space
		output.put((char)' ');
		output.put((char)'-');
		state     = READING_OPTION;
		buffer[0] = 0;
		p         = buffer;
		continue;
	    }
	    output.put((char)ch);
	    if(ch == '"')
		if(sub_state == 0)
		    sub_state = 1;
		else
		    sub_state = 0;
	    state = SKIPPING_WORD;
	}
    }
}

void TranslateOptions(istream &input, ostream &output)
{
    char *exec_name   = ReadExecName(input, output);
    ExecOptions *exec = ExecOptions::FindExecutable(exec_name);
    free(exec_name);
    if(exec == NULL)
	return;
    ReadOptions(exec, input, output);
}

//------------------------------------------------------------------------------
int option_filter_main(istream &input, ostream &output)
{
    int result = 0;
    if(ReadOptionFile() < 0){
	// should really do lazy init here, so it's done only once
	result = 255;
    }
    else {
    	TranslateOptions(input, output);
    }
    if (execs) {
    	delete execs;
	execs = NULL;
    }
    return result;
}

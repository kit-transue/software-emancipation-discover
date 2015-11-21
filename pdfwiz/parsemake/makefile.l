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
// makefile.l
// 29.oct.97 kit transue
// parse standard format of makefiles

// developed using -+ option of flex 2.5.4 (c++ support may change in later revs)

// NOTE: running flex with -f option results in broken scanner.  Don't know why.

#include <iostream>
#include <string>
#include <stack>
#include <cassert>
#include "make_toplevel.h"
#include "make_target.h"
#include "dependpath.h"
#include "variabletable.h"
#include "macro_expander.h"
#include "mscondexpr.h"
#include "message.h"

//#pragma warning(disable:4786)
#pragma warning(disable:4503)


// gotta get rid of these globals!
string lhs;  // left side of assignment/dependency
string actionstring;	// building action across inline files

class conditional_state_mgr
{
public:
	enum stateval {NOT_YET_TRUE, TRUE, PAST_TRUE};
	void push()	{ statestack.push(state); }
	void pop()	{ state = statestack.top(); statestack.pop(); }
	bool empty()	{ return statestack.empty(); }
#ifdef MSNOTBROKEN
	stateval state;
#else
	int state;
#endif
	conditional_state_mgr() : state(TRUE) {}

private:
#ifdef MSNOTBROKEN
	stack<stateval> statestack;
#else
	stack<int> statestack;
#endif
} statemgr;

conditional_state_mgr state;
static int inlinefiles = 0;

%}

%option noyywrap

%x ACTION ASSIGNMENT DEPENDENCY COMMENT SQUELCHED EVALUATE EVALUATEDEF EVALUATENDEF SQUELCHEOL ECHOEOL INLINEFILE

include		^include[ \t]
suffixrule	^\.
action		^[ \t]
dependency	:
assignment	=
comment		^#
extension	^!
leftside	^[^ !#\t:=\n\.]([^#:=\n]|\\.)*
restofline	[^\n]*

%%

{include}{restofline}\n {
                        makefile::globalmakefile->add_includefile(string(yytext));
			BEGIN INITIAL;
			return 2;
		}

{dependency}	{
			lhs += '\n';
			makefile::globalmakefile->add_target(lhs);
			BEGIN DEPENDENCY;
		}

<DEPENDENCY>{restofline}\n	{
			// parse dependencies
			makefile::globalmakefile->add_dependents(yytext);

			BEGIN INITIAL;
			return 1;
		}

{assignment}	{ BEGIN ASSIGNMENT; }

{action}	{ BEGIN ACTION; }


<*>{extension}IF	{
			statemgr.push();
			statemgr.state = (statemgr.state == conditional_state_mgr::TRUE)
				? conditional_state_mgr::NOT_YET_TRUE
				: conditional_state_mgr::PAST_TRUE;
			if (statemgr.state == conditional_state_mgr::NOT_YET_TRUE) {
				BEGIN EVALUATE;
			} else {
				BEGIN SQUELCHEOL;
			}
		}

<*>{extension}ELSEIF	{
			switch(statemgr.state) {
				case conditional_state_mgr::NOT_YET_TRUE:
					BEGIN EVALUATE;
					break;
				case conditional_state_mgr::TRUE:
					statemgr.state = conditional_state_mgr::PAST_TRUE;
					BEGIN SQUELCHEOL;
					break;
				case conditional_state_mgr::PAST_TRUE:
					break;
			}
		}

<*>{extension}ELSE	{
			switch(statemgr.state) {
				case conditional_state_mgr::NOT_YET_TRUE:
					statemgr.state = conditional_state_mgr::TRUE;
					BEGIN COMMENT;
					break;
				case conditional_state_mgr::TRUE:
					statemgr.state = conditional_state_mgr::PAST_TRUE;
					BEGIN SQUELCHEOL;
					break;
				case conditional_state_mgr::PAST_TRUE:
					BEGIN SQUELCHEOL;
					break;
			}
		}

<*>{extension}ENDIF	{
			if (statemgr.empty())
				msgid("Extra !ENDIF encountered in makefile") << eom;
			else
				statemgr.pop();
			if (statemgr.state == conditional_state_mgr::TRUE) {
				BEGIN COMMENT;
			}
			else {
				BEGIN SQUELCHEOL;
			}
		}

<EVALUATE>{restofline}\n	{
			// prepare text for parsing
			// expand macros
			string s(global_macroexpander->expand_macros_in_string(yytext, makefile::globalmakefile->symbols));

			// parse!
			if (msCondExprParser.evaluate_line(s)) {
				statemgr.state = conditional_state_mgr::TRUE;
				BEGIN INITIAL;
			} else {
				BEGIN SQUELCHED;
			}
			return 1;
		}

{extension}MESSAGE	{ //
			BEGIN ECHOEOL;
		}

{extension}INCLUDE{restofline}\n {
                        makefile::globalmakefile->add_includefile(string(yytext));
			return 2;
			
		}

{extension}ERROR	{ //
			msgid("ERROR statement encountered; continuing") << eom;
			BEGIN ECHOEOL;
		}

<*>{extension}IFDEF	{
			statemgr.push();
			statemgr.state = (statemgr.state == conditional_state_mgr::TRUE)
				? conditional_state_mgr::NOT_YET_TRUE
				: conditional_state_mgr::PAST_TRUE;
			if (statemgr.state == conditional_state_mgr::NOT_YET_TRUE) {
				BEGIN EVALUATEDEF;
			} else {
				BEGIN SQUELCHEOL;
			}
		}

<*>{extension}IFNDEF	{
			statemgr.push();
			statemgr.state = (statemgr.state == conditional_state_mgr::TRUE)
				? conditional_state_mgr::NOT_YET_TRUE
				: conditional_state_mgr::PAST_TRUE;
			if (statemgr.state == conditional_state_mgr::NOT_YET_TRUE) {
				BEGIN EVALUATENDEF;
			} else {
				BEGIN SQUELCHEOL;
			}
		}

<*>{extension}ELSEIFDEF	{
			switch(statemgr.state) {
				case conditional_state_mgr::NOT_YET_TRUE:
					BEGIN EVALUATEDEF;
					break;
				case conditional_state_mgr::TRUE:
					statemgr.state = conditional_state_mgr::PAST_TRUE;
					BEGIN SQUELCHEOL;
					break;
				case conditional_state_mgr::PAST_TRUE:
					break;
			}
		}

<*>{extension}ELSEIFNDEF	{
			switch(statemgr.state) {
				case conditional_state_mgr::NOT_YET_TRUE:
					BEGIN EVALUATENDEF;
					break;
				case conditional_state_mgr::TRUE:
					statemgr.state = conditional_state_mgr::PAST_TRUE;
					BEGIN SQUELCHEOL;
					break;
				case conditional_state_mgr::PAST_TRUE:
					break;
			}
		}



<EVALUATEDEF>{restofline}\n	{
			// prepare text for parsing
			// expand macros
			string s(global_macroexpander->expand_macros_in_string(yytext, makefile::globalmakefile->symbols));
			string::size_type start = s.find_first_not_of(" \t\n");
			string::size_type end = s.find_last_not_of(" \t\n");
			string macroname(s, start, end - start);

			// parse!
			if (&(makefile::globalmakefile->symbols->lookup(macroname)) != macro_node::null_macro) {
				statemgr.state = conditional_state_mgr::TRUE;
				BEGIN INITIAL;
			} else {
				BEGIN SQUELCHED;
			}
			return 1;
		}


<EVALUATENDEF>{restofline}\n	{
			// prepare text for parsing
			// expand macros
			string s(global_macroexpander->expand_macros_in_string(yytext, makefile::globalmakefile->symbols));
			string::size_type start = s.find_first_not_of(" \t\n");
			string::size_type end = s.find_last_not_of(" \t\n");
			string macroname(s, start, end - start);

			// parse!
			if (&(makefile::globalmakefile->symbols->lookup(macroname)) == macro_node::null_macro) {
				statemgr.state = conditional_state_mgr::TRUE;
				BEGIN INITIAL;
			} else {
				BEGIN SQUELCHED;
			}
			return 1;
		}


{extension}[A-Za-z]+	{ // catchall rule
			msgid("Unknown !extension in makefile:") << yytext << eom;
			BEGIN ECHOEOL;
		}

<SQUELCHED>[^ \t\n]*	{ // first word in line inside conditional block
			BEGIN SQUELCHEOL;
		}

<SQUELCHED>\n		{
			return 1;
		}

<SQUELCHED>[ \t]	{}

<SQUELCHEOL>{restofline}\n	{
			BEGIN SQUELCHED;
			return 1;
		}


{comment}	{
			BEGIN COMMENT;
		}

{leftside}	{
			lhs = yytext;
		}

<ASSIGNMENT>{restofline}	{
			string s(yytext);
			global_macroexpander->expand_one_macro_in_string(s, lhs, makefile::globalmakefile->symbols);
			makefile::globalmakefile->symbols->insert (lhs, macro_node(s, lineno()));
			BEGIN INITIAL;
		}
<ASSIGNMENT>\n		{
			makefile::globalmakefile->symbols->insert (lhs, macro_node("", lineno()));
			BEGIN INITIAL;
			return 1;
		}

<ACTION>\<\<[^ \t\n\"]*	{
			++inlinefiles;
		}
<ACTION>([^\<\n]*(\<[^\<\n])?)*	{
			// nasty expressions weeds out inline files
                        // expand macros later
			actionstring += yytext;
		}

<ACTION>\n	{  // action finalized, inline files removed
			if (inlinefiles == 0) {
                        	makefile::globalmakefile->add_action(actionstring);
				actionstring.erase();
				BEGIN INITIAL;
			} else {
				BEGIN INLINEFILE;
			}
			return 1;
		}

<INLINEFILE>\<\<{restofline}	{  // end of inline file
			if (--inlinefiles == 0) {
				// multiple inline files may be specified

				// add action:
                        	makefile::globalmakefile->add_action(actionstring);
				actionstring.erase();
				BEGIN INITIAL;
			}
		}

<INLINEFILE>{restofline}	{ // contents of the file
			actionstring += yytext;
		}

<INLINEFILE>\n	{
			return 1;
		}

{suffixrule}{restofline}	{
			makefile::globalmakefile->add_suffixrule(string(yytext));
		}

<COMMENT>{restofline}?\n	{
			BEGIN INITIAL;
			return 1;
		}

<ECHOEOL>{restofline}?\n	{
			ECHO;
			BEGIN INITIAL;
			return 1;
		}

\n		{ 
			return 1;
		}

" "		{
		}

.		{
			assert(0);
		}
%%

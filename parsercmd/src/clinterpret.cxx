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

#ifdef _WIN32
#pragma warning (disable:4786)
#endif

#include <string>
#include <ctype.h>
#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif

#include "clinterpret.h"
#include "configpar.h"
#include "msg.h"

namespace std {};
using namespace std;

static bool evaluate_switch_expr(string, set<string> &, vector<string> &);

// ===========================================================================
OptionInterpreter::OptionInterpreter(bool no_arg, 
				     bool immediate, 
				     bool separate,
				     bool swallows,
				     list<string> translation,
				     string name,
				     string *intro,
				     pair<string,bool> *sw,
				     CommandLineInterpreter &env,
				     TranslatedOutput &block)
  : takes_no_arg(no_arg),
    takes_immediate_arg(immediate),
    takes_separate_arg(separate),
    swallower(swallows),
    translation(translation),
    option_name(name),
    option_intro(intro),
    swch(sw),
    env(env),
    block(block)
{
}

// ===========================================================================
// % means: insert the option argument from input
// %_% means: synonym for % (because %%0%% is ambiguous).
// %% means: insert %
// %n% means: insert the compiler argument from environment,
//    e.g. sunccpp's installation directory
//
void OptionInterpreter::interpret(string *arg) const {
    list<string> &oargs = block.translation;
    list<string>::const_iterator iter;
    for (iter = translation.begin(); iter != translation.end(); ++iter) {
#ifdef ISO_CPP_HEADERS
	ostringstream ostrm;
#ifdef sun5
	// Hack around an ugly bug: failure if the first string written
	// to the stream is larger than 128 characters.
	ostrm.write(" ", 1);
#endif
#else
	ostrstream ostrm;
#endif
	int len = iter->length();
	bool in_escape = false;  // into a % escape sequence
	string escape_val;
	for (int i = 0; i < len; i += 1) {
	    char ch = (*iter)[i];
	    if (in_escape) {
		if (isdigit(ch)) {
		    escape_val.append(1, ch);
		}
		else if (ch == '_' && escape_val.length() == 0) {
		    escape_val += ch;
		}
		else {
		    if (ch == '%') {
			if (escape_val.length() == 0) {
			    ostrm << '%';
			}
			else if (escape_val == "_") {
			    // Insert the argument, because %_% is synonymous with %.
			    if (arg != NULL) {
				ostrm << *arg;
			    }
			}
			else {
			    // end of numeric escape
			    int argnum = atoi(escape_val.c_str());
			    const string *argp = env.arg(argnum);
			    if (argp != NULL) {
				ostrm << *argp;
			    }
			    else {
				msg("Compiler configuration file refers to nonexistent argument %$1%.", error_sev)
				    << argnum << eoarg << eom;
			    }
			}
		    }
		    else {
			// oops, not an escape, insert the argument and the present character
			if (arg != NULL) {
			    ostrm << *arg;
			}
			ostrm << escape_val << ch;
		    }
		    escape_val = "";
		    in_escape = false;
		}
	    }
	    else if (ch == '%') {
		in_escape = true;
	    }
	    else {
		ostrm << ch;
	    }
	}
	if (in_escape) {
	    // not an escape, insert the argument
	    if (arg != NULL) {
		ostrm << *arg;
	    }
	    ostrm << escape_val;
	}
#ifndef ISO_CPP_HEADERS
	ostrm << ends;
#endif
#ifndef sun5
	oargs.push_back(ostrm.str());
#else
	// See above re: hack.
	string arg_str = ostrm.str();
	oargs.push_back(arg_str.c_str()+1);
#endif
    }
}

// ===========================================================================
bool OptionInterpreter::has_translation() const {
    return !translation.empty();
}

// ===========================================================================
// Note that the switch of an option is recorded in the TranslatedOutput
// block at this stage, but it does not affect the set of switches until
// the switches are interpreted at the time of assembling output.
void OptionInterpreter::flip_switch() const {
    if (swch != NULL) {
	block.swch_activations.push_back(*swch);
    }
}

// ===========================================================================
const string &OptionInterpreter::diagnostic_name() const
{
    return option_name;
}

// ===========================================================================
const string &OptionInterpreter::name() const
{
    return option_name;
}

// ===========================================================================
const string *OptionInterpreter::intro() const
{
    return option_intro;
}

// ===========================================================================
SuffixInterpreter::SuffixInterpreter(string name,
				     bool fold,
				     TranslatedOutput &block)
    : name(name), fold(fold), block(block)
{
}

// ===========================================================================
NameSelector::NameSelector(string intro)
  : name_prefix(intro)
{
}

// ===========================================================================
void NameSelector::note_named_option(OptionInterpreter &opt)
{
    named_options.insert(pair<const string, OptionInterpreter *>
			     (opt.name(), &opt));
}

// ===========================================================================
const string &NameSelector::nameintro() const
{
    return name_prefix;
}

// ===========================================================================
const OptionInterpreter *NameSelector::named_option(const string &name) const
{
    an_option_ptr_map::const_iterator iter = named_options.find(name);
    if (iter != named_options.end()) {
	return (*iter).second;
    }
    return NULL;
}

// ===========================================================================
NameSelectorEquivalent::NameSelectorEquivalent(string intro, NameSelector &equiv)
  : name_prefix(intro), eq(equiv)
{}

// ===========================================================================
//
CommandLineInterpreter::CommandLineInterpreter() 
  : flag_prefix(new string("/")),
    envargs(NULL),
    unrec_receiver(NULL),
    unrec_msg(msg_sev(unspecified_sev)),
    default_selector(NULL)
{
}

// ===========================================================================
const string *CommandLineInterpreter::flagintro() const
{
    return flag_prefix;
}

// ===========================================================================
const OptionInterpreter *CommandLineInterpreter::flag_option(char ch) const
{
    string flag_string;
    flag_string.append(1, ch);
    an_option_ptr_map::const_iterator iter = flags.find(flag_string);
    if (iter != flags.end()) {
	return (*iter).second;
    }
    return NULL;
}

// ===========================================================================
vector<NameSelector *> &CommandLineInterpreter::name_selectors()
{
    return selectors;
}

// ===========================================================================
vector<NameSelectorEquivalent *> &CommandLineInterpreter::name_equivs()
{
    return equivs;
}

// ===========================================================================
// Clears out the members of a CommandLineInterpreter, from any accumulated
// command line translation.  Preserves config file information.
//
void CommandLineInterpreter::reset() {
    a_list_of_blocks::iterator iter;
    for (iter = blocks.begin(); iter != blocks.end(); ++iter) {
	OutputBlock *b = *iter;
	b->clear();
    }
    enabled_switches.clear();
}

// ===========================================================================
static string substitute_env(const string &s, vector<string> &args)
{
#ifdef ISO_CPP_HEADERS
    ostringstream result;
#else
    ostrstream result;
#endif
    int len = s.length();
    bool in_escape = false;  // into a % escape sequence
    string escape_val;
    for (int i = 0; i < len; i += 1) {
	char ch = s.at(i);
	if (in_escape) {
	    if (isdigit(ch)) {
		escape_val.append(1, ch);
	    }
	    else {
		if (ch == '%' && escape_val.length() > 0) {
		    // end of numeric escape
		    int argnum = atoi(escape_val.c_str());
		    if (argnum < args.size()) {
		        result << args[argnum];
		    }
		    else {
			msg("Compiler configuration file refers to nonexistent argument %$1%.", error_sev)
			    << argnum << eoarg << eom;
		    }
		}
		else {
		    // oops, not an escape, do the %
		    result << '%' << escape_val;
		}
		escape_val = "";
		in_escape = false;
	    }
	}
	else if (ch == '%') {
	    in_escape = true;
	}
	else {
	    result << ch;
	}
    }
    if (in_escape) {
	// not an escape, do the %
	result << '%' << escape_val;
    }
#ifndef ISO_CPP_HEADERS
    result << ends;
#endif
    return result.str();
}

// ===========================================================================
// Returns true if config file is read successfully.
// In case of failure, a diagnostic message is allocated and stored into
// 'diagnostic'.
//
bool CommandLineInterpreter::configure(const char *config_file,
                                       vector<string> &envargs_param,
                                       string *&diagnostic,
				       msg_sev unrec_arg) {

    // Clear out the members from any data from a config file.
    // We're not careful about freeing memory here, because it
    // is only supposed to happen once.
    name_intros.clear();
    delete flag_prefix;
    flag_prefix = NULL;
    flags.clear();
    suffixes.clear();
    blocks.clear();
    unrec_receiver = NULL;
    unrec_msg = msg_sev(unspecified_sev);

    envargs = &envargs_param;
    bool result = parse_config_file(config_file, diagnostic, 
				    name_intros, name_intro_equivs,
				    flag_prefix,
                                    blocks,
				    unrec_msg,
				    *this);
    if (unrec_arg != unspecified_sev) {
        unrec_msg = unrec_arg;
    }

    // Create a NameSelector for each name_intro string.
    // To be populated below, where configure is called for each OutputBlock.
    selectors.clear();
    default_selector = NULL;
    a_list_of_cstrs::iterator siter;
    for (siter = name_intros.begin(); siter != name_intros.end(); ++siter) {
	const char *sp = *siter;
	if (sp != NULL) {
	    selectors.push_back(new NameSelector(sp));
	    // If it's the first, then it's the default.
	    if (siter == name_intros.begin()) {
    	    	default_selector = selectors.front();
	    }
	}
    }
    // Create a NameSelectorEquivalent for each name_equiv string.
    equivs.clear();
    for (siter = name_intro_equivs.begin(); siter != name_intro_equivs.end(); ++siter) {
	const char *sp = *siter;
	if (sp != NULL && default_selector != NULL) {
	    equivs.push_back(new NameSelectorEquivalent(sp, *default_selector));
	}
    }

    // Different kinds of blocks must be accounted for differently in
    // the interpreter's state. Call a virtual function so that a
    // translation block can activate its option interpreters in the
    // interpreter, etc.
    //
    a_list_of_blocks::iterator iter;
    for (iter = blocks.begin(); iter != blocks.end(); ++iter) {
	OutputBlock *b = *iter;
	if (b != NULL) {
	    b->configure(*this, envargs_param);
	}
    }
    return result;
}

// ===========================================================================
void lower(const string &str, string &low)
{
    low = "";
    string::size_type len = str.length();
    const char *strp = str.c_str();
    for (string::size_type i = 0; i < len; i += 1) {
	low.append(1, (char)tolower(strp[i]));
    }
}

// ===========================================================================
// Provides the string translation of the given suffix, putting it
// into the translation block associated with the suffix interpreter.
bool CommandLineInterpreter::suffixString(const string &suffix, string &result) const
{
    bool have_result = false;
    string low;
    lower(suffix, low);
    a_suffix_ptr_map::const_iterator it;
    const SuffixInterpreter *result_interpreter = NULL;
    for (it = suffixes.begin(); it != suffixes.end(); ++it) {
	const SuffixInterpreter &sval = *((*it).second);
	bool got_result = false;
	if (sval.fold) {
	    // fold case
	    string lw;
	    lower((*it).first, lw);
	    if (low == lw) {
		got_result = true;
	    }
	}
	else {
	    if (suffix == (*it).first) {
		got_result = true;
	    }
	}
	if (got_result) {
	    if (envargs != NULL) {
		result = substitute_env(sval.name, *envargs);
	    }
	    else {
		result = sval.name;
	    }
	    have_result = true;
            result_interpreter = &sval;
	}
    }
    if (result_interpreter != NULL) {
	TranslatedOutput &block = result_interpreter->block;
	block.translation.push_back(result);
    }
    return have_result;
}

// ===========================================================================
const string *CommandLineInterpreter::arg(int argnum)
{
    static string empty;
    return envargs != NULL && 0 <= argnum && argnum < envargs->size()
           ? &((*envargs)[argnum]) : NULL;
}

// ===========================================================================
void CommandLineInterpreter::note_named_option(OptionInterpreter &opt)
{
    NameSelector *sel = NULL;
    if (opt.intro() == NULL) {
        // There's no explicit intro string. Use the default.
	sel = default_selector;
    }
    else {
        // Verify that the intro string is in the declared list.
	bool ok = false;
        a_list_of_cstrs::iterator iter;
	for (iter = name_intros.begin(); iter != name_intros.end(); ++iter) {
	    if (*(opt.intro()) == *iter) {
		ok = true;
	    }
	}
	if (ok) {
	    // Find the associated selector.
	    string introstr = *opt.intro();
	    a_selector_list::iterator iter;
	    for (iter = selectors.begin(); iter != selectors.end(); ++iter) {
		if (introstr == (*iter)->nameintro()) {
		    sel = *iter;
		}
	    }
	}
    }
    // Pass it along to the associated set of names.
    if (sel != NULL) {
	sel->note_named_option(opt);
    }
}

// ===========================================================================
void CommandLineInterpreter::note_flag(OptionInterpreter &opt)
{
    flags.insert(pair<const string, OptionInterpreter *>
			 (opt.name(), &opt));
}

// ===========================================================================
void CommandLineInterpreter::note_suffix(pair<const string, SuffixInterpreter> &ref)
{
    suffixes.insert(pair<const string, SuffixInterpreter *>
		    (ref.first, &ref.second));
}

// ===========================================================================
void CommandLineInterpreter::note_receiver(TranslatedOutput *p)
{
    unrec_receiver = p;
}

// ===========================================================================
vector<OutputBlock *> &CommandLineInterpreter::block_sequence()
{
    return blocks;
}

// ===========================================================================
TranslatedOutput *CommandLineInterpreter::unrecognized_receiver()
{
    return unrec_receiver;
}

// ===========================================================================
msg_sev CommandLineInterpreter::unrecognized_msg()
{
    return unrec_msg;
}

// ===========================================================================
void CommandLineInterpreter::set_switch(string swch, bool enable) {
    if (enable) {
	enabled_switches.insert(swch);
    }
    else {
	enabled_switches.erase(swch);
    }
}

// ===========================================================================
bool CommandLineInterpreter::evaluate_switch_expr(string expr) {
    static vector<string> no_args;
    bool enabled = ::evaluate_switch_expr(expr, enabled_switches,
				          envargs != NULL ? *envargs : no_args);
    return enabled;
}

// ===========================================================================
void TranslatedOutput::clear()
{
    translation.clear();
}

// ===========================================================================
TranslatedOutput::TranslatedOutput()
    : forward_unrecognized(false)
{
}

// ===========================================================================
list<string> TranslatedOutput::get(CommandLineInterpreter &interp)
{
    list<pair<string,bool> >::iterator iter;
    for (iter = swch_activations.begin(); iter != swch_activations.end(); ++iter) {
	interp.set_switch(iter->first, iter->second);
    }
    return translation;
}

// ===========================================================================
void TranslatedOutput::configure(CommandLineInterpreter &interp, vector<string> &)
{
    an_option_list::iterator liter;
    for (liter = flags.begin(); liter != flags.end(); ++liter) {
        interp.note_flag(**liter);
    }
    for (liter = named_options.begin(); liter != named_options.end(); ++liter) {
        interp.note_named_option(**liter);
    }
    a_suffix_map::iterator siter;
    for (siter = suffixes.begin(); siter != suffixes.end(); ++siter) {
        interp.note_suffix(*siter);
    }
    if (forward_unrecognized) {
	interp.note_receiver(this);
    }
}

// ===========================================================================
void FixedOutput::clear()
{
}

// ===========================================================================
list<string> FixedOutput::get(CommandLineInterpreter &interp)
{
    if (condition == NULL || interp.evaluate_switch_expr(*condition)) {
	if (swch != NULL) {
	    interp.set_switch(swch->first, swch->second);
	}
	return value;
    }
    else {
	return list<string>();
    }
}

// ===========================================================================
FixedOutput::FixedOutput(const list<string> &v, bool do_substitute_environment,
			 const string *cond, const pair<string,bool> *sw)
    : value(v), do_environment(do_substitute_environment),
      condition(cond), swch(sw)
{
}

// ===========================================================================
FixedOutput::~FixedOutput()
{
    delete condition;
    delete swch;
}

// ===========================================================================
void FixedOutput::configure(CommandLineInterpreter &interp, vector<string> &envargs)
{
    if (do_environment) {
	list<string>::iterator iter;
	for (iter = value.begin(); iter != value.end(); ++iter) {
	    (*iter) = substitute_env(*iter, envargs);
	}
    }
}

// ===========================================================================
// Parsing switch expressions, as in
// <switch when="oldfor|&quot;%0%&quot;==&quot;5&quot;"/>
// ===========================================================================
class SwitchExprParser {
public:
    SwitchExprParser(string, set<string> &, vector<string> &);
    ~SwitchExprParser();
    bool parse();
private:
    bool expr(bool &);
    bool term(bool &);
    bool str(string &);
    bool match(const char *);
    char peek(); // returns 0 at end
    bool a_char(char &);

    set<string> &enabled;
    vector<string> &args;
    const char *text;
    int cur;
    bool is_error;
};

// ===========================================================================
SwitchExprParser::SwitchExprParser(string e, set<string> &enabled, vector<string> &args)
  : enabled(enabled), cur(0), is_error(false), args(args)
{
    text = new char[e.length() + 1];
    strcpy((char *)text, e.c_str());
}

// ===========================================================================
SwitchExprParser::~SwitchExprParser() {
    delete [] (char *)text;
}

// ===========================================================================
bool SwitchExprParser::parse() {
    bool result;
    if (!expr(result) || peek() != '\0') {
	msg("Compiler configuration file has error in switch expression $1.", error_sev) << text << eoarg << eom;
    }
    return result;
}

// ===========================================================================
// expr ::= term | '!' term | string '==' string | term '&' and_term | term '|' or_term
//
bool SwitchExprParser::expr (bool &result) {
    switch (peek()) {
    case '"':
	{
	    string left;
	    string right;
	    if (str(left) && match("==") && str(right)) {
		result = (left == right);
		return true;
	    }
	    return false;
	}
	break;
    case '!':
	{
	    match("!");
	    bool v;
	    if (term(v)) {
		result = !v;
		return true;
	    }
	    return false;
	}
	break;
    default:
	{
	    if (!term(result)) {
		return false;
	    }
	    switch(peek()) {
	    case '&':
		{
		    while (match("&")) {
			bool v;
			if (!term(v)) {
			    return false;
			}
			result = result && v;
		    }
		    return true;
		}
		break;
	    case '|':
		{
		    while (match("|")) {
			bool v;
			if (!term(v)) {
			    return false;
			}
			result = result || v;
		    }
		    return true;
		}
		break;
	    default:
		return true;
	    }
	}
	break;
    }
}

// ===========================================================================
// term ::= '(' expr ')' | switch
//
bool SwitchExprParser::term(bool &result) {
    if (peek() == '(') {
	match("(");
        return (expr(result) && match(")"));
    }
    else if (isalpha(peek())) {
	// Switch names are alphabetic [a-zA-Z]*.
	string sw;
	while (isalpha(peek())) {
	    char ch;
	    sw += (a_char(ch), ch);
	}
	result = (enabled.find(sw) != enabled.end());
	return true;
    }
    else {
	return false;
    }
}

// ===========================================================================
// Strings use C syntax. They begin and end with double-quotes.
// But within a string %n% is expanded using parameters, and %% represents
// a single percent, as it does elsewhere in parsercmd.
// Also note that the double-quotes must be escaped in the XML attribute using &quot;,
// which is a pain, but it's part of XML and it's better to use " than ' because in
// several languages the single quotes suppress expansion.
//
bool SwitchExprParser::str(string &result) {
    if (!match("\"")) {
	return false;
    }
    string str;
    bool escaping = false;
    for (;;) {
	if (peek() == '\0') {
	    return false;
	}
	char ch;
	a_char(ch);
	if (escaping) {
	    str += ch;
	    escaping = false;
	}
	else if (ch == '"') {
	    break;
	}
	else if (ch == '\\') {
	    escaping = true;
	}
	else {
	    str += ch;
	}
    }
    // We have the C-string-like contents.
    // Interpret the %'s.
    result = substitute_env(str, args);
    return true;
}

// ===========================================================================
bool SwitchExprParser::match(const char *p) {
    while (*p != '\0') {
	if (peek() != *p) {
	    return false;
	}
	char ch;
	a_char(ch);
	p += 1;
    }
    return true;
}

// ===========================================================================
char SwitchExprParser::peek() {
    return text[cur];
}

// ===========================================================================
bool SwitchExprParser::a_char(char &ch) {
    return (ch = text[cur++]) != '\0';
}

// ===========================================================================
bool evaluate_switch_expr(string expr, set<string> &enabled_switches,
				vector<string> &args) {
    SwitchExprParser parser(expr, enabled_switches, args);
    return parser.parse();
}

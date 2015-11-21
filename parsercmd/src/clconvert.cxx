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

#include "clconvert.h"
#include "configpar.h"

using namespace std;

// ===========================================================================
// The position within iargs is tracked by iargp.
// Upon advancing iargp to the next arg, istrm is set up with the
// contents of the argument.

// ===========================================================================
//
CommandLineConverter::CommandLineConverter(
	CommandLineInterpreter &clinterp,
	list<string> &i, list<string> &o)
  : interp(&clinterp), 
    iargs(i),
    istrm(NULL),
    oargs(o),
    diagnostic(NULL),
    fault(false)
{
    // Prepare state of input to parse.
    iargp = iargs.begin();
    fetch_arg();

    // Preallocate data structures for option_token.
    num_nameintros = clinterp.name_selectors().size() + clinterp.name_equivs().size();
    nameintros = new string[num_nameintros];
    maybe_name = new bool[num_nameintros];
    is_name = new bool[num_nameintros];
    intro_len = new int[num_nameintros];
    name_selectors = new NameSelector *[num_nameintros];
    CommandLineInterpreter::a_selector_list::iterator iter;
    int n = 0;
    for (iter = clinterp.name_selectors().begin();
         iter != clinterp.name_selectors().end(); ++iter, n += 1) {
        nameintros[n] = (*iter)->nameintro();
    	intro_len[n] = nameintros[n].length();
        name_selectors[n] = *iter;
    }
    CommandLineInterpreter::an_equiv_list::iterator eiter;
    for (eiter = clinterp.name_equivs().begin();
         eiter != clinterp.name_equivs().end(); ++eiter, n += 1) {
        nameintros[n] = (*eiter)->nameintro();
    	intro_len[n] = nameintros[n].length();
	name_selectors[n] = &(*eiter)->selector();
    }
}

// ===========================================================================
//
CommandLineConverter::~CommandLineConverter()
{
    delete diagnostic;
    delete [] nameintros;
    delete [] maybe_name;
    delete [] is_name;
    delete [] intro_len;
    delete [] name_selectors;
    delete istrm;
}

// ===========================================================================
// Parses a complete command line from iargs, converting it into oargs.
//
bool CommandLineConverter::convert(string &diag) {
    fault = false;
    bool success = command_line();
    if (diagnostic == NULL && fault) {
	makeDiagnostic("parsing failure");
    }
    if (diagnostic != NULL) {
	diag = *diagnostic;
    }
    return success;
}

// ===========================================================================
void CommandLineConverter::makeDiagnostic(const string &diag) {
    if (diagnostic == NULL) {
	diagnostic = new string(diag);
    }
}

// ===========================================================================
// This function will finalize the string stream, appending 'ends' if needed.
//
#ifdef ISO_CPP_HEADERS
void CommandLineConverter::makeDiagnostic(ostringstream &diag) {
#else
void CommandLineConverter::makeDiagnostic(ostrstream &diag) {
    diag << ends;
#endif
    makeDiagnostic(diag.str());
}

// ===========================================================================
// The following functions are parse-oriented. Each will return true/false
// depending on whether parsing the desired construct succeeds or fails.
// Successful parsing advances the current position within the source file
// to the end of the construct.
//
// ===========================================================================
// Parses a complete command line from iargs, converting it into oargs.
//
bool CommandLineConverter::command_line() {
    if (fault) return false;
    interp->reset();

    // fo*e
    string filenm;
    if (argument(filenm)) {
	// file name
	// The suffix may affect the result.
	string suffix;
	bool continuable = false;
	const char *p = filenm.c_str();
	while (*p != '\0') {
	    if (isalnum(*p)) {
		if (!continuable) {
		    suffix = "";
		}
		suffix.append(1, *p);
		continuable = true;
	    }
	    else {
		continuable = false;
	    }
	    p += 1;
	}
	string suffix_string;
	if (interp->suffixString(suffix, suffix_string)) {
	}
	if (!whitespace()) {
	    return false;
	}
    }
    for (;;) {
	string arg;
	if (complete_option() && whitespace()) {
	    // OK
	    // (*ostrm) << ' ';
	    // have_text = true;
	}
	else if (argument(arg) && whitespace()) {
	    unrecognized_option(arg);
	}
	else {
	    break;
	}
    }
    bool ok = eof();
    if (ok) {
	// Assemble the output from the sequence of blocks.
	CommandLineInterpreter::a_list_of_blocks::iterator iter;
	CommandLineInterpreter::a_list_of_blocks &blocks = interp->block_sequence();

	for (iter = blocks.begin(); iter != blocks.end(); ++iter) {
	    list<string> block_content = (*iter)->get(*interp);
	    oargs.splice(oargs.end(), block_content);
	}
    }
    return ok;
}

// ===========================================================================
// Parses one option with its arguments.  Translates the option to oargs.
//
bool CommandLineConverter::complete_option() {
    if (fault) return false;
    queue<const OptionInterpreter *> options;
    if (!option_token(options)) {
	return false;
    }
    string arg;
    bool have_arg = false;
    bool need_arg = false;
    if (options.size() == 1) {
	const OptionInterpreter *ointerp = options.front();
	if (ointerp->swallow_remaining()) {
	    iargp = iargs.end();
	    delete istrm;
	    istrm = NULL;
	}
	if (ointerp->separate_arg()) {
	    whitespace();
	}
	if (at_argument()) {
	    if (ointerp->immediate_arg() || ointerp->separate_arg()) {
		have_arg = argument(arg);
	    }
	    else if (ointerp->no_arg()) {
	    }
	    else {
#ifdef ISO_CPP_HEADERS
		ostringstream d;
#else
		ostrstream d;
#endif
		d << "option " << ointerp->diagnostic_name() << " does not specify its argument handling";
		makeDiagnostic(d);
		fault = true;
	    }
	}
	else {
	    if (ointerp->no_arg() || ointerp->swallow_remaining()) {
	    }
	    else {
                if (interp->unrecognized_msg() <= unknown_sev) {
		    msg("Option $1 does not support no argument.", interp->unrecognized_msg())
			    << ointerp->diagnostic_name() << eoarg << eom;
		}
		need_arg = true;
	    }
	}
	if (!fault && (have_arg || !need_arg)) {
	    interpret(ointerp, have_arg ? &arg : NULL);
	}
    }
    else {
	while (!options.empty()) {
	    const OptionInterpreter *ointerp = options.front();
	    options.pop();
	    if (ointerp->no_arg()) {
		interpret(ointerp, NULL);
	    }
	    else {
#ifdef ISO_CPP_HEADERS
		ostringstream d;
#else
		ostrstream d;
#endif
		d << "flag " << ointerp->diagnostic_name() << " expects an an argument and occurs with other flags.";
		makeDiagnostic(d);
		fault = true;
	    }
	}
    }
    return !fault;
}

// ===========================================================================
void CommandLineConverter::interpret(const OptionInterpreter *ointerp, string *arg) {
    if (ointerp->has_translation()) {
	ointerp->interpret(arg);
    }
    ointerp->flip_switch();
}

// ===========================================================================
// Parses an option token, e.g. /Tc, -w, -I, --bool
// Returns true if successful, in which case 'opt' is set to one or more
// translation actions.  Multiple actions indicate flag options which do not
// deal with arguments.  A single action may involve arguments.
//
bool CommandLineConverter::option_token(queue<const OptionInterpreter *> &opt) {
    if (fault) return false;
    const string *flagintro = interp->flagintro();
    bool maybe_flag = flagintro != NULL;
    int n;
    for (n = 0; n < num_nameintros; n += 1) {
	maybe_name[n] = true;
    }
    bool is_flag = false; // true if flagintro detected
    int is_name_eg; // -1, or an example nameintro if any detected
    for (n = 0; n < num_nameintros; n += 1) {
        is_name[n] = false; // true if nameintros[n] detected
    }
    int i = 0;
    int fil = maybe_flag ? (int)flagintro->length() : -1;
    // nameintro and flagintro are the character sequences which introduce
    // named options and flag options respectively.  For example, 
    // *flagintro == "-" and *nameintro == "--" is possible, or else
    // *nameintro == "/" and flagintro == NULL would be possible.
    for (;;) {
	// Test whether we have a name or a flag.
        for (n = 0; n < num_nameintros; n += 1) {
	    if (maybe_name[n] && intro_len[n] == i) {
		is_name[n] = true;
	    }
	}
	if (maybe_flag && fil == i) {
	    is_flag = true;
	}
        // Determine whether further examination of the option is needed.
	bool need_more_look = maybe_flag && !is_flag;
        for (n = 0; n < num_nameintros; n += 1) {
	    if (maybe_name[n] && !is_name[n]) {
		need_more_look = true;
	    }
	}
	// We're trying just to read characters until a nameintro or flagintro
	// is recognized. The tricky case occurs if one is the prefix of the other.
	// In that case, we might have to read a character to check on the longer
	// possibility, and then back up.
	is_name_eg = -1;
	for (n = 0; n < num_nameintros; n += 1) {
	    if (is_name[n]) {
		is_name_eg = n;
	    }
	}
    	if (!need_more_look && (is_flag || is_name_eg >= 0)) {
	    break;
	}
	// Look at the next character.
	if (istrm == NULL) {
	    return false;
	}
	char ch;
	istrm->get(ch);
	if (istrm->eof()) {
	    if (is_flag || is_name_eg >= 0) {
		// We have a valid prefix.
		break;
	    }
	    if (i != 0) {
		// We can't back up to restore the parse state to where we started.
		fault = true;
	    }
	    return false;
	}
	// Determine whether, given the character ch, it is still possible 
	// to find the name prefix or the flag prefix.
	bool some_maybe_name = false;
	for (n = 0; n < num_nameintros; n += 1) {
	    maybe_name[n] = maybe_name[n] && i < intro_len[n] && (nameintros[n].at(i) == ch);
	    if (maybe_name[n]) {
		some_maybe_name = true;
	    }
	}
	maybe_flag = maybe_flag && i < fil && (flagintro->at(i) == ch);
	i += 1;
	if (!(maybe_flag || some_maybe_name)) {
	    // Neither is possible; read no more characters.
	    istrm->putback(ch);
	    i -= 1;
	    if (is_name_eg >= 0 || is_flag) {
		// We have backed up to a valid prefix.
		break;
	    }
	    if (i != 0) {
		// We can't back up to restore the parse state to where we started.
		fault = true;
	    }
	    return false;
	}
	for (n = 0; n < num_nameintros; n += 1) {
	    is_name[n] = false;
	}
	is_flag = false;
    }
    // We have found the name prefix, or the flag prefix, or both.
    string optname;
    bool flag_found = false;
    bool non_flag_found = false;
    // Advance through the option name. At each character, check
    //   1. If a name option is complete, done.
    //   2. If at the end of the option name, done.
    //   3. If the character is a flag option, record it.
    for (;;) {
        for (n = 0; n < num_nameintros; n += 1) {
	    if (is_name[n]) {
		const OptionInterpreter *optp = name_selectors[n]->named_option(optname);
		if (optp && optp->immediate_arg() && at_argument()) {
		    opt = queue<const OptionInterpreter *>(); // Clear out flag options.
		    opt.push(optp);
		    return true;
		}
		if (optp && (optp->separate_arg() || optp->no_arg() || optp->swallow_remaining())
			 && at_whitespace()) {
		    opt = queue<const OptionInterpreter *>(); // Clear out flag options.
		    opt.push(optp);
		    return true;
		}
	    }
	}
	if (at_whitespace()) {
	    if (flag_found && !non_flag_found) {
		return true;
	    }
	    string opt = is_flag ? *flagintro : nameintros[is_name_eg];
	    opt.append(optname);
	    return unrecognized_option(opt);
	}
	char ch;
	istrm->get(ch);
	// Don't need to check for istrm->eof() because at_whitespace did it.
	optname.append(1, ch);
	if (is_flag) {
	    const OptionInterpreter *optp = interp->flag_option(ch);
	    if (optp && optp->no_arg()) {
		opt.push(optp);
		flag_found = true;
	    }
	    else {
		non_flag_found = true;
	    }
	}
    }
}

// ===========================================================================
// This returns true to indicate that an unrecognized option is ignored,
// and treated as accepting no argument.  Giving an error for unrecognized
// options would put an intolerable burden on configuration file maintenance,
// because it's hard to list all options of even a single version of one
// compiler.  Copying unrecognized options in the translation has also
// proved useless in released configuration files.  However, we allow the
// config file to indicate either of these.
//
bool CommandLineConverter::unrecognized_option(const string &optname)
{
    TranslatedOutput *block = interp->unrecognized_receiver();
    if (block != NULL) {
	block->translation.push_back(optname);
    }
    if (interp->unrecognized_msg() < msg_sev(no_message_sev)) {
	msg("Unrecognized command line option: $1", interp->unrecognized_msg())
	    << optname << eoarg << eom;
    }
    return true;
}

// ===========================================================================
// Parses zero or more whitespace characters.
// Now that we use lists of strings, we're just looking to move from one
// arg to the next.
//
bool CommandLineConverter::whitespace() {
    if (at_whitespace()) {
        next_arg();
	return true;
    }
    return false;
}

// ===========================================================================
// Succeed if the current position is at the end of an arg string,
// or if it's at end of file.  Now that we use lists of strings, we're just
// checking whether we're at the end of an argument.
//
bool CommandLineConverter::at_whitespace() {
    if (fault) return false;
    char ch;
    if (istrm == NULL) {
        return true;  // at end of list
    }
    istrm->get(ch);
    if (istrm->eof()) {
	return true;
    }
    istrm->putback(ch);
    return false;
}

// ===========================================================================
// An argument consists of all characters up to the end of input argument.
// Whitespace characters may occur.  Nothing special with backslashes, either.
//
bool CommandLineConverter::argument(string &arg) {
    if (fault) return false;
    if (!at_argument()) {
	return false;
    }
    for (;;) {
	char ch;
	istrm->get(ch);
	if (istrm->eof()) {
	    return true;
	}
	arg.append(1, ch);
    }
}

// ===========================================================================
// Succeed if the current position immediately precedes an argument, i.e. is
// not at whitespace or eof.
//
bool CommandLineConverter::at_argument() {
    if (fault || istrm == NULL) return false;
    return true;
}

// ===========================================================================
// Returns whether the current position of the parse is at end-of-file
// (end of list)
//
bool CommandLineConverter::eof() {
    if (fault) return false;
    return istrm == NULL;
}

// ===========================================================================
// Advance to the next input argument, and fill the istrm char buffer.
//
bool CommandLineConverter::next_arg() {
    if (iargp == iargs.end()) {
        return false;
    }
    ++iargp;
    return fetch_arg();
}

// ===========================================================================
// Fill the istrm char buffer with the current argument.
//
bool CommandLineConverter::fetch_arg() {
    delete istrm;
    istrm = NULL;
    if (iargp != iargs.end()) {
#ifdef ISO_CPP_HEADERS
	istrm = new istringstream(*iargp);
#else
	istrm = new istrstream(iargp->c_str());
#endif
	return true;
    }
    return false;
}

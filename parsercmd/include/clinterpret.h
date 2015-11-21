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
// An object of this class is based on a particular
// compiler on a particular operating system. 
// Its presents the information from a file
// describing a particular compiler's command line options.
// A constructor for this class reads such a file in order
// to set its state.

// Since it encapsulates only compiler-specific information,
// it does not retain a parser state.  A command line parser
// has a CommandLineInterpreter.

#ifndef CLINTERPRET_H
#define CLINTERPRET_H

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
class ostrstream;
#endif
#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include "msg.h"
namespace std {};
using namespace std;

class CommandLineInterpreter;
class TranslatedOutput;

// An OptionInterpreter retains data on one option such as -I.
// Whether it accepts an argument, etc.
class OptionInterpreter {
public:
    OptionInterpreter(bool no_arg, 
		      bool immediate, 
		      bool separate,
		      bool swallows,
		      list<string> translation,
		      string name,
		      string *intro,
		      pair<string,bool> *swch,
		      CommandLineInterpreter &env,
		      TranslatedOutput &block);
    bool no_arg() const            { return takes_no_arg; }
    bool immediate_arg() const     { return takes_immediate_arg; }
    bool separate_arg() const      { return takes_separate_arg; }
    bool swallow_remaining() const { return swallower; }
    void interpret(string *arg) const;
    bool has_translation() const;
    void flip_switch() const;
    const string &diagnostic_name() const;
    const string &name() const;
    const string *intro() const;
private:
    bool takes_no_arg;
    bool takes_immediate_arg;
    bool takes_separate_arg;
    bool swallower;
    list<string> translation;
    string option_name;
    string *option_intro;
    pair<string,bool> *swch;
    CommandLineInterpreter &env;

    // the associated output section:
    TranslatedOutput &block;
};

// A SuffixInterpreter holds information on a file suffix.
class SuffixInterpreter {
public:
    SuffixInterpreter(string name,
		      bool fold,
		      TranslatedOutput &block);
    string name;         // name to generate when this suffix is found
    bool fold;           // whether to fold case to identify this suffix

    // the associated output section:
    TranslatedOutput &block;
};

// Specifications from the configuration file are
// ordered in a list of OutputBlocks,
// so that the configuration can dictate the sequence of the output.
class OutputBlock {
public:
    // Called once after the list of output blocks is created,
    // to set up the CommandLineInterpreter.  The interpreter
    // needs to be able to find the options and suffixes in
    // the translated output blocks, and to find the translated
    // output block (if any) where unrecognized options are
    // forwarded.
    virtual void configure(CommandLineInterpreter &,
                           vector<string> &envargs) = 0;

    // Called before parsing/translating each command line.
    virtual void clear() = 0;

    // Called after parsing/translating each command line
    // to get the contents of this output block.
    virtual list<string> get(CommandLineInterpreter &) = 0;
};

// Represents a block of fixed (default) output which does not
// vary with the input line.
// The condition is an optional expression which determines whether
// the block is active.
// While appending the block to the final output, one named flag
// may be enabled or diabled.
class FixedOutput : public OutputBlock {
public:
    FixedOutput(const list<string> &, bool do_environment,
		const string *, const pair<string,bool> *);
    ~FixedOutput();

    void configure(CommandLineInterpreter &, vector<string> &envargs);
    void clear();
    list<string> get(CommandLineInterpreter &);
    const string *condition;
    const pair<string, bool> *swch;
private:
    list<string> value;
    bool do_environment;    // whether to expand environment variables
			    // in the given value
};

// TranslatedOutput represents a block of output that's translated from the
// input line.  Normally there's just one of these, but there's support for
// multiples.  A translation block entails any number of flag and named option
// translations.  It can indicate the place to copy unrecognized
// command line options.  It can act on the suffix of a file name
// that's found in the input line.
class TranslatedOutput : public OutputBlock {
public:
    TranslatedOutput();

    void configure(CommandLineInterpreter &, vector<string> &envargs);
    void clear();
    list<string> get(CommandLineInterpreter &);

    typedef vector<OptionInterpreter *> an_option_list;
    typedef map<string, SuffixInterpreter> a_suffix_map;

    an_option_list flags;        // handlers for flag-based options
    an_option_list named_options;// handlers for name-based options
    a_suffix_map suffixes;	 // treatment of input file suffix
    bool forward_unrecognized;

    list<pair<string,bool> > swch_activations;
    list<string> translation;
};

// Holds the name options associated with a particular introductory
// character sequence.  A single compiler might accept options beginning
// with both "-" and "+".
class NameSelector {
public:
    typedef map<string, OptionInterpreter *> an_option_ptr_map;

    NameSelector(string intro);

    // Called upon parsing the config file, to set up this interpreter
    // with needed info:
    void note_named_option(OptionInterpreter &);
    
    // Called to guide the parse of a compiler command line,
    // and to translate options:
    const string &nameintro() const;
    const OptionInterpreter *named_option(const string &) const;

private:
    string name_prefix;         // introduces name-based options
    an_option_ptr_map named_options;
};

// Identifies an introductory character sequence that's equivalent to
// another.
class NameSelectorEquivalent {
public:
    NameSelectorEquivalent(string intro, NameSelector &equiv);

    const string &nameintro() const { return name_prefix; };
    NameSelector &selector() { return eq; }
private:
    string name_prefix;         // introduces name-based options
    NameSelector &eq;
};

// CommandLineInterpreter holds information from a compiler configuration file.
// Also accumulates translation information during the processing of
// a compiler command line.
class CommandLineInterpreter {
public:
    typedef map<string, OptionInterpreter *> an_option_ptr_map;
    typedef map<string, SuffixInterpreter *> a_suffix_ptr_map;
    typedef vector<OutputBlock *> a_list_of_blocks;
    typedef vector<NameSelector *> a_selector_list;
    typedef vector<NameSelectorEquivalent *> an_equiv_list;
    typedef vector<const char *> a_list_of_cstrs;

    CommandLineInterpreter();

    // Parse the given config file, and prepare this interpreter.
    // Called from main.
    bool configure(const char *config_file, vector<string> &envargs, string *&diagnostic, msg_sev unrec_msg);

    // Called upon parsing the config file, to set up this interpreter
    // with needed info:
    void note_named_option(OptionInterpreter &);
    void note_flag(OptionInterpreter &);
    void note_suffix(pair<const string, SuffixInterpreter> &);
    void note_receiver(TranslatedOutput *p);

    // Prepare data structures for parsing a new command line.
    void reset();
    
    // Called to guide the parse of a compiler command line,
    // and to translate options:
    const string *flagintro() const;
    const OptionInterpreter *flag_option(char) const;
    a_selector_list &name_selectors();
    an_equiv_list &name_equivs();
    bool suffixString(const string &suffix, string &result) const;
    const string *arg(int argnum);
    TranslatedOutput *unrecognized_receiver();
    a_list_of_blocks &block_sequence();
    msg_sev unrecognized_msg();

    void set_switch(string, bool);
    bool evaluate_switch_expr(string);

private:
    string *flag_prefix;         // introduces flag-based options
    vector<string> *envargs;     // arguments from parsercmd command line
    an_option_ptr_map flags;
    a_selector_list selectors;
    an_equiv_list equivs;
    a_list_of_cstrs name_intros; // First in the list is the default, if any.
    a_list_of_cstrs name_intro_equivs;
    NameSelector *default_selector;
    a_suffix_ptr_map suffixes;
    TranslatedOutput *unrec_receiver;
    msg_sev unrec_msg;

    set<string> enabled_switches;
    a_list_of_blocks blocks;     // sequence of blocks indicating a
				 // translated compiler command line
};

#endif // CLINTERPRET_H

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
// parse_config_file parses a file which describes how to translate
// the command line options of a particular compiler, to the
// corresponding options for aset_CCcc.
// It sets up the data structures used for the actual translation.
// The form of the file is XML-based.
// An example file:
/*

<cmdline nameintro="/">
--microsoft_bugs
  <translate>
    <filesuffix name="cxx" foldcase>ntclcpp</filesuffix>
    <option name="I" immarg separg>-I%</option>
    <option name="link" remarg/>
  </translate>
</cmdline>

*/

// The full structure is:

// The element "cmdline" is the overall container.
//    attribute "nameintro" indicates a character sequence that introduces name-based options.
//    attribute "flagintro" indicates a character sequence that introduces flag-based options.
//    attribute "unrecognized" indicates "error" to report any unrecognized options.
//    The immediately included text is interpreted to aset_CCcc args as the default args.
// The element "translate" contains a block of elements that translate the input command line.
//    attribute "unrecognized" enables the passing along of unrecognized options.
//   The element "option" describes one option.
//      The "name" attribute indicates a name-based option to be matched.
//      The "flag" attribute indicates a flag-based option to be matched.
//      The "intro" attribute of a name-based option specifies which "nameintro" applies.
//          (If "intro" is absent, then it defaults to the one specified in "cmdline".)
//      One or more argument policies must be indicated.
//          noarg: The option may be followed directly by another option.
//          immarg: The option may be followed immediately, with no intervening
//                  whitespace, by its argument.
//          separg: The option may be followed by a separate argument.
//          remarg: All remaining text after the option is swallowed up.
//      The immediately included text is interpreted to aset_CCcc args,
//          with an occurrence of % substituted with the argument.
//   The element "filesuffix" provides a result based on the source file name.
//      The "name" attribute indicates the suffix on which to act.
//      The "foldcase" attribute indicates that the suffix is to be interpreted
//          regardless of upper vs. lower case.
// The element "list" dictates the expansion of a list that is independent of input.
//    attribute "getenv" indicates an environment variable for list expansion.
//    attribute "separator" indicates where to chop the value into a list.
//    The immediately included text is expanded for each list item.
// The element "alternate" supplements the "cmdline" attributes.
//    attribute "nameintro" indicates a character sequence that introduces name-based options.
// The element "equivalent" supplements the "cmdline" attributes.
//    attribute "nameintro" indicates a character sequence that introduces name-based options,
//    that is interchangeable with the default nameintro.
// Generally, % expands to the option argument or list item depending on context.
//            %% is an escape sequence when % is needed in the output.
//            %n% expands to the n-th command line argument of this program (0-based).
// Text is parsed as a list of argument strings. Because this applies to unexpanded text,
//    escape sequences are not needed, and whitespace makes a good argument separator.
//    Expansion of % or %n% occurs within a string and does not multiply strings.

#ifndef CONFIGPAR_H
#define CONFIGPAR_H

#include <string>
#include <vector>
#include <utility>
#include "msg.h"

namespace std {};
using namespace std;

class OptionInterpreter;
class CommandLineInterpreter;
class OutputBlock;
bool parse_config_file(const char *config_file, string *&diagnostic, 
		       vector<const char *> &name_intros,
		       vector<const char *> &name_equivs,
		       string *&flag_prefix, 
		       vector<OutputBlock *> &output_seq,
		       msg_sev &unrec_msg,
		       CommandLineInterpreter &env);

enum { no_message_sev = unknown_sev + 1, unspecified_sev };
bool string_to_severity(const string &str, msg_sev &sev);

#endif // CONFIGPAR_H

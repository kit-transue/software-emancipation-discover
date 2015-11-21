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
// Implements the parser for a configuration file.
// The configuration file is structured as XML data.
// This parser uses a subset of IBM's SAX API, distributed in xml4c.
// SAX is a standard multi-language API for event-based parsing of XML.
//

#ifdef _WIN32
#pragma warning (disable:4786)
#endif

#include "configpar.h"
#include "configpar_internal.h"
#include "clinterpret.h"
#include "msg.h"
#include "minisax.h"
#include "startprocess.h"
#ifndef _WIN32
#include "unistd.h"
#endif

#include <string.h>
#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif
#include <list>

using namespace std;
using namespace MiniXML;
using namespace MBDriver;

typedef map<string, OptionInterpreter> an_option_map;
typedef map<string, SuffixInterpreter > a_suffix_map;


#if 0
// ===========================================================================
// Utility to help glue the SAX API to ASCII
//
string XMLsToString(const XMLCh *in) {
#ifdef ISO_CPP_HEADERS
    ostringstream result;
#else
    ostrstream result;
#endif
    if (in != NULL) {
        const XMLCh *p = in;
	while (*p != 0) {
	    result << (char)(*p);
	    p += 1;
	}
    }
#ifndef ISO_CPP_HEADERS
    result << ends;
#endif
    return result.str();
}
#endif

// ===========================================================================
// Abort if SAX calls back to a stub.
//
static void unimplemented(const char *name) {
    if (name)
	msg("Unexpected SAX callback $1.", catastrophe_sev) << name << eoarg << eom;
    else
	msg("Unexpected SAX callback.", catastrophe_sev) << eom;
    exit(1);
}

static void unimplemented() {
    unimplemented(0);
}

// ===========================================================================
//
ConfigErrorHandler::ConfigErrorHandler(
    string *&diagnostic,
    bool &have_warning,
    bool &have_error,
    bool &have_fatality) : diagnostic(diagnostic),
			   have_warning(have_warning),
			   have_error(have_error),
			   have_fatality(have_fatality)
{
}

// ===========================================================================
//
void ConfigErrorHandler::warning(const SAXParseException& exception) {
    if (!(have_warning || have_error || have_fatality)) {
	updateDiagnostic(warning_sev, "warning", exception);
    }
    have_warning = true;
}

// ===========================================================================
//
void ConfigErrorHandler::error(const SAXParseException& exception) {
    if (!(have_error || have_fatality)) {
	updateDiagnostic(error_sev, "error", exception);
    }
    have_error = true;
}

// ===========================================================================
//
void ConfigErrorHandler::fatalError(const SAXParseException& exception) {
    if (!(have_error || have_fatality)) {
	updateDiagnostic(catastrophe_sev, "fatal error", exception);
    }
    have_fatality = true;
}

// ===========================================================================
//
void ConfigErrorHandler::resetErrors() {
    have_warning  = false;
    have_error    = false;
    have_fatality = false;
    diagnostic    = NULL;
}

// ===========================================================================
//
void ConfigErrorHandler::updateDiagnostic(msg_sev sev, const char *kind, 
					  const SAXParseException& exception) {
    string message = XMLsToString(exception.getMessage());
#ifdef ISO_CPP_HEADERS
    ostringstream composed;
#else
    ostrstream composed;
#endif
    composed << kind << " on line " << exception.getLineNumber() << ": " << message;
#ifndef ISO_CPP_HEADERS
    composed << ends;
#endif
    if (diagnostic == NULL) {
	diagnostic = new string;
    }
    *diagnostic = composed.str();
    msg("flags translator configuration: $1, on line $2", sev)
	<< message << eoarg << exception.getLineNumber() << eoarg << eom;
}

// ===========================================================================
//
ConfigDocumentHandler::ConfigDocumentHandler(vector<const char *> &name_intros,
			  vector<const char *> &name_equivs,
			  string *&flag_prefix,
			  vector<OutputBlock *> &output_seq,
			  msg_sev &unrec_msg,
			  CommandLineInterpreter &env) 
  : name_intros(name_intros),
    name_equivs(name_equivs),
    flag_prefix(flag_prefix),
    output_seq(output_seq),
    unrec_msg(unrec_msg),
    in_cmdline(0),
    in_option(0),
    in_translate(0),
    in_alternate(0),
    in_equivalent(0),
    in_switch(0),
    env(env),
    translate(NULL),
    option_intro(NULL),
    condition(NULL),
    swch(NULL)
{
}

// ===========================================================================
// Most text from the XML file goes into a list of strings.
// It is broken into strings at whitespace.
// Simple double-quotes with backslash-escapes are supported, so that
// a string containing whitespace, quotes, or backslashes can be specified.
// 
// On Unix, environment variables of the form $xyz are expanded at this
// point.  (On Windows, %%xyz%% is converted by parsercmd to %xyz% and then
// returned very carefully.)
static void get_string_list(const string &in, list<string> &interpretation)
{
    interpretation.clear();
    const char *p = in.c_str();
    const char *start = NULL;
    string arg;
    bool in_quote = false;
    for (;;) {
	// p points to the current command line character being examined.
	// start indicates the beginning of the string which needs to be
	// copied to the argument.
        const char ch = *p;
	bool space = !in_quote && (isspace(ch) || ch == '\n' || ch == '\r');
#ifndef _WIN32
	if (start != NULL && start[0] == '$' && p > start + 1
			      && !(isalnum(ch) || ch == '_')) {
	    string name(start + 1, p - (start + 1));
	    char *val = getenv(name.c_str());
	    if (val != NULL) {
		arg.append(val, strlen(val));
		start = p;
	    }
	}
#endif
        if (start != NULL && (ch == '\0' || space)) {
	    arg.append(start, p - start);
	    interpretation.push_back(arg);
	    start = NULL;
	}
	if (ch == '\0') {
	    break;
	}
	if (start == NULL && !space) {
	    start = p;
	    arg = "";
	}
	if (start == NULL) {
	    // Wait for a non-whitespace character.
	}
	else if (ch == '\\' && (p[1] == '"' || p[1] == '\\')) {
	    arg.append(start, p - start);
	    start = p + 1;
	    p = start;
	}
	else if (ch == '\\') {
	    msg("flags translator configuration: lone backslash (\\)", error_sev)
		<< eom;
	}
	else if (ch == '"') {
	    in_quote = !in_quote;
	    arg.append(start, p - start);
	    start = p + 1;
	}
#ifndef _WIN32
	else if (ch == '$') {
	    arg.append(start, p - start);
	    start = p;
	}
#endif
	p += 1;
    }
}

// ===========================================================================
//
void ConfigDocumentHandler::characters (
    const   XMLCh* const    chars
    , const unsigned int    length)
{
    if (in_cmdline == 1) {
	string text = XMLsToString(chars);
	list<string> args;
	get_string_list(text, args);
	if (in_option == 0 && in_switch == 0) {
	    output_seq.push_back(new FixedOutput(args, true, NULL, NULL));
	}
	else if (in_option == 1 && in_switch == 0) {
	    translation.splice(translation.end(), args);
	}
	else if (in_switch == 1 && in_option == 0) {
	    output_seq.push_back(new FixedOutput(args, true,
		condition == NULL ? NULL : new string(*condition), NULL));
	}
    }
}

// ===========================================================================
//
void ConfigDocumentHandler::endDocument () {}

// ===========================================================================
//
void ConfigDocumentHandler::ignorableWhitespace (
    const XMLCh* const,
    const unsigned int
) {}

// ===========================================================================
//
void ConfigDocumentHandler::processingInstruction (
    const XMLCh* const,
    const XMLCh* const
) { unimplemented("ConfigDocumentHandler::processingInstruction"); }

// ===========================================================================
//
void ConfigDocumentHandler::resetDocument() {
    in_cmdline = 0;
    in_option = 0;
    in_switch = 0;
}

// ===========================================================================
// This function is called by the SAX parser to give us a Locator.  Since
// we do not plan to do any locating, we will just disregard it.
// 
void ConfigDocumentHandler::setDocumentLocator(const Locator * const) {}

// ===========================================================================
//
void ConfigDocumentHandler::startDocument() {}

// ==========================================================================
//
static void substitute(string &translation, string &arg) {
#ifdef ISO_CPP_HEADERS
    ostringstream result;
#else
    ostrstream result;
#endif
    int len = translation.length();
    for (int i = 0; i < len; i += 1) {
	char ch = translation[i];
	if (ch == '%') {
	    if (i+1 < len && translation[i+1] == '%') {
		result << '%';
		i += 1;
	    }
	    else {
		// insert the argument
		result << arg;
	    }
	}
	else {
	    result << ch;
	}
    }
#ifndef ISO_CPP_HEADERS
    result << ends;
#endif
    translation = result.str();
}

// ===========================================================================
// For an element like
//     <list separator=";" getenv="INCLUDE">-I"%"</list>
// get the environment variable and interpret it as a list using the given
// separator.  Expand the translation (-I"%") for each value in the list.
//
bool expand_list(const string &env_variable, list<string> &translation,
		 const char separator, list<string> &expansion)
{
    const char *listval = getenv(env_variable.c_str());
    if (listval == NULL) {
	return false;
    }
    const char *begin = listval;
    const char *p;
    expansion.clear();
    for (p = begin; ; p += 1) {
	if (*p == '\0' || *p == separator) {
	    if (p != begin) {
		string listelem = string(begin, p - begin);

		// Detect badness (infinite recursion) if listelem refers to env_variable.
		bool found_bad = false;
		{
		    size_t i = 0;
		    size_t j = 0;
		    size_t elem_len = listelem.length();
		    size_t var_len = env_variable.length();
		    for (i = 0; i < elem_len; i += 1) {
			if (listelem.at(i) == '%' && i+1+var_len < elem_len
                                      && listelem.at(i + 1 + var_len) == '%') {
			    bool match_so_far = true;
			    for (j = 0; j < var_len; j += 1) {
				match_so_far = match_so_far &&
				    toupper(env_variable.at(j)) == toupper(listelem.at(i+1+j));
			    }
			    found_bad = found_bad || match_so_far;
			}
		    }
		}
		if (!found_bad) {
		    list<string>::iterator iter;
		    for (iter = translation.begin(); iter != translation.end(); ++iter) {
			expansion.push_back(*iter);
			substitute(expansion.back(), listelem);
		    }
		}
	    }
	    if (*p == '\0') {
		break;
	    }
	    else {
		begin = p + 1;
	    }
	}
    }
    return true;
}

// ===========================================================================
//
void ConfigDocumentHandler::endElement(const XMLCh* const name) {
    if (charsEqual(name, "cmdline")) {
	in_cmdline -= 1;
    }
    else if (in_cmdline == 1) {
        if (charsEqual(name, "translate")) {
	    output_seq.push_back(translate);
	    translate = NULL;
	    in_translate -= 1;
        }
	else if (in_translate == 1) {
	    if (charsEqual(name, "option")) {
		if (in_option == 1 && (is_name || is_flag)) {
		    (is_name ? translate->named_options : translate->flags).push_back(
			new
			OptionInterpreter(no_arg, immediate, separate, swallows,
					  translation, name_or_flag, option_intro, swch,
					  env, *translate));
		}
		in_option -= 1;
	    }
	    if (charsEqual(name, "filesuffix")) {
		if (in_option == 1 && is_name && !translation.empty()) {
		    translate->suffixes.insert(pair<const string, SuffixInterpreter>
			(name_or_flag, 
			SuffixInterpreter(translation.back(), foldcase, *translate)));
		}
		in_option -= 1;
	    }
	}
	if (in_translate == 0 && charsEqual(name, "switch")) {
	    if (in_switch == 1) {
		output_seq.push_back(new FixedOutput(list<string>(), true,
		    condition == NULL ? NULL : new string(*condition),
		    swch == NULL ? NULL : new pair<string,bool>(*swch)));
	    }
	    in_switch -= 1;
	}
	if (charsEqual(name, "list")) {
	    if (in_option == 1 && is_list) {
		list<string> expansion;
		if (expand_list(name_or_flag, translation, separator, expansion)) {
		    output_seq.push_back(new FixedOutput(expansion, false, NULL, NULL));
		}
	    }
	    in_option -= 1;
	}
	if (charsEqual(name, "compilername")) {
	    // Ignore translation.  Compilername tag is for project editor.
	    in_option -= 1;
	}
	if (charsEqual(name, "alternate")) {
	    in_alternate -= 1;
	}
	if (charsEqual(name, "equivalent")) {
	    in_equivalent -= 1;
	}
    }
}

// ===========================================================================
// Sets sev if the string designates a known severity, otherwise sets
// sev to unknown_sev.  Returns true if the string designates a known
// severity or if it is "no_message".
//
bool string_to_severity(const string &str, msg_sev &sev) {
    sev = unknown_sev;
    if (str == "normal") {
	sev = normal_sev;
    }
    else if (str == "warning") {
	sev = warning_sev;
    }
    else if (str == "error") {
	sev = error_sev;
    }
    else if (str == "catastrophe") {
	sev = catastrophe_sev;
    }
    else if (str == "unknown") {
    }
    else if (str == "no_message") {
        sev = msg_sev(no_message_sev);
    }
    else {
	return false;
    }
    return true;
}

// ===========================================================================
//
void ConfigDocumentHandler::startElement (
    const   XMLCh* const    name
    ,       AttributeList&  attrs) 
{
    if (charsEqual(name, "cmdline")) {
	in_cmdline += 1;
	if (in_cmdline == 1) {
	    string s;
	    if (setStringFromAttr(s, flagintro_str, attrs)) {
		flag_prefix = new string(s);
	    }
	    if (setStringFromAttr(s, nameintro_str, attrs)) {
	        char *np = new char[s.length() + 1];
		strcpy(np, s.c_str());
	        name_intros.push_back(np);
	    }
	    else {
	        name_intros.push_back(NULL);
	    }
            if (setStringFromAttr(s, unrecognized_str, attrs)) {
	        string_to_severity(s, unrec_msg);
	    }
	}
    }
    else if (in_cmdline == 1) {
	if (charsEqual(name, "translate")) {
	    in_translate += 1;
	    translate = new TranslatedOutput();
	    translate->forward_unrecognized = getFlagFromAttrs(attrs, "unrecognized");
	}
	else if (in_translate == 1) {
	    if (charsEqual(name, "option")) {
		in_option += 1;
		if (in_option == 1) {
		    is_name = setStringFromAttr(name_or_flag, name_str, attrs);
		    if (is_name) {
			is_flag = false;
		    }
		    else {
			is_flag = setStringFromAttr(name_or_flag, flag_str, attrs);
		    }
		    string intro;
		    if (setStringFromAttr(intro, intro_str, attrs)) {
			option_intro = new string(intro);
		    }
		    else {
			option_intro = NULL;
		    }
		    string switch_name;
		    bool have_able = false;
		    if (setStringFromAttr(switch_name, enable_str, attrs)) {
		    }
		    if (setStringFromAttr(switch_name, disable_str, attrs)) {
			swch = new pair<string,bool>(switch_name, false);
			if (have_able) {
			    msg("flags translator configuration: enable and disable in the same option", error_sev)
				<< eom;
			}
		    }
		    else if (!have_able) {
		        swch = NULL;
		    }
		    getFlagsFromAttrs(attrs);
		    translation.clear();
		}
	    }
	    if (charsEqual(name, "filesuffix")) {
		in_option += 1;
		if (in_option == 1) {
		    is_name = setStringFromAttr(name_or_flag, name_str, attrs);
		    getFlagsFromAttrs(attrs);
		    translation.clear();
		}
	    }
	}
	if (in_translate == 0 && charsEqual(name, "switch")) {
	    in_switch += 1;
	    if (in_switch == 1) {
		string cond;
		if (setStringFromAttr(cond, when_str, attrs)) {
		    condition = new string(cond);
		}
		else {
		    condition = NULL;
		}
		string switch_name;
		bool have_able = false;
		if (setStringFromAttr(switch_name, enable_str, attrs)) {
		    swch = new pair<string,bool>(switch_name, true);
		    have_able = true;
		}
		if (setStringFromAttr(switch_name, disable_str, attrs)) {
		    swch = new pair<string,bool>(switch_name, false);
		    if (have_able) {
			msg("flags translator configuration: enable and disable in the same switch", error_sev)
			    << eom;
		    }
		}
		else if (!have_able) {
		    swch = NULL;
		}
	    }
	}
	if (charsEqual(name, "list")) {
	    in_option += 1;
	    if (in_option == 1) {
		string sepstring;
		is_list = setStringFromAttr(name_or_flag, getenv_str, attrs)
		    && setStringFromAttr(sepstring, separator_str, attrs)
		    && sepstring.length() == 1;
		if (is_list) {
		    separator = sepstring.at(0);
		}
		translation.clear();
	    }
	}
	if (charsEqual(name, "compilername")) {
	    in_option += 1;
	    if (in_option == 1) {
		translation.clear();
	    }
	}
	if (charsEqual(name, "alternate")) {
	    in_alternate += 1;
	    string s;
	    if (setStringFromAttr(s, nameintro_str, attrs)) {
	        char *np = new char[s.length() + 1];
		strcpy(np, s.c_str());
	        name_intros.push_back(np);
	    }
	}
	if (charsEqual(name, "equivalent")) {
	    in_equivalent += 1;
	    string s;
	    if (setStringFromAttr(s, nameintro_str, attrs)) {
	        char *np = new char[s.length() + 1];
		strcpy(np, s.c_str());
	        name_equivs.push_back(np);
	    }
	}
    }
}

// ===========================================================================
//
bool ConfigDocumentHandler::setStringFromAttr(string &str, const XMLCh *attr_name, 
					      AttributeList &attrs) {
    const XMLCh *val = attrs.getValue(attr_name);
    if (val != NULL) {
	str = XMLsToString(val);
	return true;
    }
    return false;
}

// ===========================================================================
//
map<string, ConfigDocumentHandler::attribute> *ConfigDocumentHandler::initFlagsForAttrs() {
    typedef map<string, attribute> map_type;
    map_type *attr_map = new map_type;
    attr_map->insert(map_type::value_type(string("remarg"), 
	                                  &ConfigDocumentHandler::swallows));    
    attr_map->insert(map_type::value_type(string("separg"), 
	                                  &ConfigDocumentHandler::separate));    
    attr_map->insert(map_type::value_type(string("immarg"), 
	                                  &ConfigDocumentHandler::immediate));    
    attr_map->insert(map_type::value_type(string("noarg"), 
	                                  &ConfigDocumentHandler::no_arg));   
    attr_map->insert(map_type::value_type(string("foldcase"), 
	                                  &ConfigDocumentHandler::foldcase));   
    return attr_map;
}

// ===========================================================================
//
void ConfigDocumentHandler::getFlagsFromAttrs(AttributeList &attrs) {
    typedef map<string, attribute> map_type;
    map_type *attr_map = initFlagsForAttrs();

    unsigned int n_attrs = attrs.getLength();
    swallows = false;
    separate = false;
    immediate = false;
    no_arg = false;
    foldcase = false;
    // This loop could be cleaned up.
    for (int i = 0; i < n_attrs; i += 1) {
	string name = XMLsToString(attrs.getName(i));
	map_type::iterator iter = attr_map->find(name);
	if (iter != attr_map->end()) {
	    this->*((*iter).second) = getFlagFromAttrs(attrs, name.c_str());
	}
    }
}

// ===========================================================================
//
bool ConfigDocumentHandler::getFlagFromAttrs(AttributeList &attrs,
					     const char *flag_name) {
    bool result = false;
    unsigned int n_attrs = attrs.getLength();
    for (int i = 0; i < n_attrs; i += 1) {
	string name = XMLsToString(attrs.getName(i));
	if (name == flag_name) {
	    string value = XMLsToString(attrs.getValue(i));
	    result = !(value == "0" || value == "false" || value == "FALSE");
	}
    }
    return result;
}

// ===========================================================================
//
const XMLCh ConfigDocumentHandler::nameintro_str[] = { 'n','a','m','e','i','n','t','r','o','\0' };
const XMLCh ConfigDocumentHandler::unrecognized_str[] = { 'u','n','r','e','c','o','g','n','i','z','e','d','\0' };
const XMLCh ConfigDocumentHandler::intro_str[] = { 'i','n','t','r','o','\0' };
const XMLCh ConfigDocumentHandler::flagintro_str[] = { 'f','l','a','g','i','n','t','r','o','\0' };
const XMLCh ConfigDocumentHandler::name_str[] = { 'n','a','m','e','\0' };
const XMLCh ConfigDocumentHandler::flag_str[] = { 'f','l','a','g','\0' };
const XMLCh ConfigDocumentHandler::getenv_str[] = { 'g','e','t','e','n','v','\0' };
const XMLCh ConfigDocumentHandler::separator_str[] = { 's','e','p','a','r','a','t','o','r','\0' };
const XMLCh ConfigDocumentHandler::enable_str[] = { 'e','n','a','b','l','e','\0' };
const XMLCh ConfigDocumentHandler::disable_str[] = { 'd','i','s','a','b','l','e','\0' };
const XMLCh ConfigDocumentHandler::when_str[] = { 'w','h','e','n','\0' };
#if 0
const XMLCh ConfigDocumentHandler::remarg_str[] = { 'r','e','m','a','r','g','\0' };
const XMLCh ConfigDocumentHandler::immarg_str[] = { 'i','m','m','a','r','g','\0' };
const XMLCh ConfigDocumentHandler::separg_str[] = { 's','e','p','a','r','g','\0' };
const XMLCh ConfigDocumentHandler::noarg_str[] = { 'n','o','a','r','g','\0' };
#endif

// ===========================================================================
//
bool ConfigDocumentHandler::charsEqual(const XMLCh *xmlchars, const char *chars)
{
    if (xmlchars == NULL || chars == NULL) {
	return false;
    }
    for (;;) {
	char ch = *chars;
	XMLCh xmlch = *xmlchars;
	if (ch == '\0' && xmlch == 0) {
	    return true;
	}
	else if ((unsigned char)ch != xmlch) {
	    return false;
	}
	chars += 1;
	xmlchars += 1;
    }
}

// ===========================================================================
ConfigParser::ConfigParser(const char *config_file, string *&diagnostic, 
		       vector<const char *> &name_intros,
		       vector<const char *> &name_equivs,
		       string *&flag_prefix, 
		       vector<OutputBlock *> &output_seq,
		       msg_sev &unrec_msg,
		       CommandLineInterpreter &env)

		       : config_file(config_file), diagnostic(diagnostic),
		       name_intros(name_intros),
		       name_equivs(name_equivs),
		       flag_prefix(flag_prefix),
		       output_seq(output_seq),
		       have_warning(false),
		       have_error(false),
		       have_fatality(false),
		       documentHandler(name_intros, name_equivs, flag_prefix, output_seq, unrec_msg, env),
		       errorHandler(diagnostic, have_warning, have_error, have_fatality)
{
}

// ===========================================================================
bool ConfigParser::success() { return !(have_error || have_fatality); }

// ===========================================================================
void ConfigParser::parse () {
    Parser *p = getParser();
    if (p != NULL ) {
	Parser &parser = *p;
	parser.setDocumentHandler(&documentHandler);
	parser.setErrorHandler(&errorHandler);
	parser.parse(config_file);
    }
}

// ===========================================================================
// ===========================================================================
// Parse a configuration file describing how a particular compiler handles its
// command line arguments, and how to translate those arguments for aset_CCcc.
// The file is in a simplified XML data format.
//
bool parse_config_file(const char *config_file, string *&diagnostic, 
		       vector<const char *> &name_intros,
		       vector<const char *> &name_equivs,
		       string *&flag_prefix, 
		       vector<OutputBlock *> &output_seq,
		       msg_sev &unrec_msg,
		       CommandLineInterpreter &env) {
    ConfigParser theParser(config_file, diagnostic, name_intros,
			   name_equivs, flag_prefix,
			   output_seq, unrec_msg, env);
    theParser.parse();
    return theParser.success();
}

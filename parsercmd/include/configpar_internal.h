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
#include "clinterpret.h"
#include "msg.h"

#include "minisax.h"


using namespace std;
using namespace MiniXML;

typedef map<string, OptionInterpreter> an_option_map;
typedef map<string, SuffixInterpreter> a_suffix_map;

// ===========================================================================
// Concrete class for the object which handles parse errors.
//
class ConfigErrorHandler : public ErrorHandler {
public:
    ConfigErrorHandler(
    string *&diagnostic,
    bool &have_warning,
    bool &have_error,
    bool &have_fatality);

    void warning(const SAXParseException& exception);
    void error(const SAXParseException& exception);
    void fatalError(const SAXParseException& exception);
    void resetErrors();
private:
    string *&diagnostic;
    bool &have_warning;
    bool &have_error;
    bool &have_fatality;

    void updateDiagnostic(msg_sev, const char *, const SAXParseException &exception);
};

// ===========================================================================
// Concrete class for the object which handles document-oriented parse events.
//
class ConfigDocumentHandler : public DocumentHandler {
public:
    ConfigDocumentHandler(vector<const char *> &name_intros,
			  vector<const char *> &name_equivs,
			  string *&flag_prefix,
			  vector<OutputBlock *> &output_seq,
			  msg_sev &unrec_msg,
			  CommandLineInterpreter &env);
    void characters (
        const   XMLCh* const    chars
        , const unsigned int    length
    );
    void endDocument ();
    void endElement(const XMLCh* const name);
    void ignorableWhitespace (
        const   XMLCh* const    chars
        , const unsigned int    length
    );
    void processingInstruction (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );
    void resetDocument();
    void setDocumentLocator(const Locator * const);
    void startDocument();
    void startElement (
        const   XMLCh* const    name
        ,       AttributeList&  attrs
    );
    typedef bool ConfigDocumentHandler::*attribute;

private:
    int in_cmdline;
    int in_option;
    int in_translate;
    int in_alternate;
    int in_equivalent;
    int in_switch;
    
    // Properties of the current option, filesuffix, or switch:
    bool is_name;
    bool is_flag;
    bool is_list;
    string name_or_flag;
    string *option_intro;
    char separator;
    bool no_arg;
    bool immediate; 
    bool separate;
    bool swallows;
    bool foldcase;
    string *condition;
    pair<string, bool> *swch;
    list<string> translation;
    TranslatedOutput *translate;

    vector<const char *> &name_intros;
    vector<const char *> &name_equivs;
    string *&flag_prefix;
    msg_sev &unrec_msg;
    vector<OutputBlock *> &output_seq;

    bool charsEqual(const XMLCh *xmlchars, const char *chars);
    bool setStringFromAttr(string &str, const XMLCh *attr_name, 
					      AttributeList &attrs);
    void getFlagsFromAttrs(AttributeList &attrs);
    bool getFlagFromAttrs(AttributeList &attrs, const char *flag_name);

    map<string, attribute> *initFlagsForAttrs();

    static const XMLCh nameintro_str[];
    static const XMLCh unrecognized_str[];
    static const XMLCh intro_str[];
    static const XMLCh flagintro_str[];
    static const XMLCh name_str[];
    static const XMLCh flag_str[];
    static const XMLCh getenv_str[];
    static const XMLCh separator_str[];
    static const XMLCh enable_str[];
    static const XMLCh disable_str[];
    static const XMLCh when_str[];
#if 0
    static const XMLCh remarg_str[];
    static const XMLCh immarg_str[];
    static const XMLCh separg_str[];
    static const XMLCh noarg_str[];
#endif

    CommandLineInterpreter &env;
};

// ===========================================================================
// Class to hold the state of parsing a configuration file.
//
class ConfigParser {
public:
    ConfigParser(const char *config_file, string *&diagnostic, 
		       vector<const char *> &name_intros,
		       vector<const char *> &name_equivs,
		       string *&flag_prefix, 
		       CommandLineInterpreter::a_list_of_blocks &blocks,
		       msg_sev &unrec_msg,
		       CommandLineInterpreter &env);
    void parse();
    bool success();

private:
    const char *config_file;
    string *&diagnostic; 
    vector<const char *> &name_intros;
    vector<const char *> &name_equivs;
    string *&flag_prefix;
    CommandLineInterpreter::a_list_of_blocks &output_seq;

    bool have_warning;
    bool have_error;
    bool have_fatality;

    ConfigErrorHandler errorHandler;
    ConfigDocumentHandler documentHandler;
};

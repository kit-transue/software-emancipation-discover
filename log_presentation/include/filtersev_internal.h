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

#ifdef ISO_CPP_HEADERS
#include <sstream>
#include <iostream>
#else
#include <strstream.h>
#include <iostream.h>
#endif
#include "filtersev.h"
#include "minisax.h"
#include <list>
#include <map>
#include "limits.h"

using namespace std;
using namespace MiniXML;

// ===========================================================================
// Data structures active during both configuration parsing and filtering.
//
// ===========================================================================
// The different kinds of conditions, corresponding to the concrete Condition
// classes.
//
typedef enum { message_id_ck, group_id_ck, severity_ck, is_group_ck, no_ck } condition_kind;

// ===========================================================================
// Abstract class representing a filter, the primary meaning of which is
// embodied in the 'filter' member function, which determines whether or not
// a message meets its criterion.
//
class Condition {
public:
    virtual bool filter(const string &, const lp_msg_sev, bool child_is_message_group) = 0;
    virtual condition_kind kind() = 0;

    // Id string associated with a filter.  Empty if filter has no id.
    virtual string getId();

    // Severity associated with a filter.  no_lp_sev if filter has no severity.
    virtual lp_msg_sev getSev();
};

// ===========================================================================
// Concrete class corresponding to <messageid> element in config file.
//
class MessageIdCondition : public Condition {
public:
    MessageIdCondition(string s);
    virtual bool filter(const string &s, const lp_msg_sev, bool child_is_message_group);
    virtual condition_kind kind();
    virtual string getId();
private:
    string id;
};

// ===========================================================================
// Concrete class corresponding to <severity> element in config file.
//
class SeverityCondition : public Condition {
public:
    SeverityCondition(lp_msg_sev sev);
    virtual bool filter(const string &, const lp_msg_sev sev, bool);
    virtual condition_kind kind();
    virtual lp_msg_sev getSev();
private:
    lp_msg_sev severity;
};

// ===========================================================================
// Concreate class corresponding to <groupid> element in config file.
//
class GroupIdCondition : public Condition {
public:
    GroupIdCondition(string s);
    virtual bool filter(const string &s, const lp_msg_sev, bool child_is_message_group);
    virtual condition_kind kind();
    virtual string getId();
private:
    string id;
};

// ===========================================================================
// Concrete class corresponding to <isgroup> element in config file.
//
class IsGroupCondition : public Condition {
public:
    IsGroupCondition();
    virtual bool filter(const string &s, const lp_msg_sev, bool child_is_message_group);
    virtual condition_kind kind();
};

// ===========================================================================
// This "Filter" is at a larger level than "Condition".  It represents a 
// <filter> element from the configuration file, with 'sev' to indicate the
// severity to substitute if the filter matches the message, 'conditions' to
// represent one or more Condition filters, and 'seq' to indicate sequence
// within the configuration file.
//
struct Filter {
    lp_msg_sev sev;
    list<Condition *> conditions;
    int seq;
};

bool operator < (Filter, Filter);
bool operator == (Filter, Filter);

// ===========================================================================
// map types
//
typedef map<string, list<Filter> > STRING2FILTERS;
typedef map<int, list<Filter> > INT2FILTERS;

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

    void updateDiagnostic(const char *, const SAXParseException &exception);
};

// ===========================================================================
// Helper classes for the parse process. Each concrete ElementHandler is
// specific to building the data structures for a particular element type.
//
// ===========================================================================
// Abstract class, made specific to handle elements.
//
class ElementHandler {
public:
    virtual const char *tag() = 0;

    // Called to make a child of this element, of the given tag name.
    // Returns NULL if the name fails to match an appropriate child object.
    virtual ElementHandler *makeChild(const XMLCh *name);

    // Called when characters from element's text are recognized by the
    // parser.
    virtual void characters(const string &s);

    // Called when the parser reaches the end of the element.
    virtual void endElement() = 0;

    // Called when the element is begun, giving the element handler its
    // opportunity to grab attributes.
    virtual void attributes(AttributeList &);
protected:
    string text;
};

// ===========================================================================
// Concreate handler for <messageid> elements.  Adds a condition
// to the list of the current Filter.
//
class MessageIdElementHandler : public ElementHandler {
public:
    MessageIdElementHandler(Filter &f);
    virtual const char *tag();
    virtual void endElement();
private:
    Filter &filter;
};

// ===========================================================================
// Concrete handler for <severity> elements. Adds a condition to the list
// of the current Filter.
//
class SeverityElementHandler : public ElementHandler {
public:
    SeverityElementHandler(Filter &f);
    virtual const char *tag();
    virtual void endElement();
    virtual void attributes(AttributeList &);
private:
    lp_msg_sev value;
    Filter &filter;
};

// ===========================================================================
// Concrete handler for <groupid> elements.  Adds a condition to the list
// of the current Filter.
//
class GroupIdElementHandler : public ElementHandler {
public:
    GroupIdElementHandler(Filter &f);
    virtual const char *tag();
    virtual void endElement();
private:
    Filter &filter;
};

// ===========================================================================
// Concrete handler for <isgroup> elements.  Adds a condition to the list
// of the current Filter.
//
class IsGroupElementHandler : public ElementHandler {
public:
    IsGroupElementHandler(Filter &f);
    virtual const char *tag();
    virtual void endElement();
private:
    Filter &filter;
};

// ===========================================================================
// Concrete handler for any unrecognized element.
//
class UnknownElementHandler : public ElementHandler {
public:
    UnknownElementHandler();
    virtual const char *tag();
    virtual void endElement();
};

// ===========================================================================
// Concrete handler for <filter> elements. Adds a Filter to the current
// list of Filters.
//
class FilterElementHandler : public ElementHandler {
public:
    FilterElementHandler(list<Filter> &fl);
    virtual const char *tag();
    virtual ElementHandler *makeChild(const XMLCh *);
    virtual void endElement();
    virtual void attributes(AttributeList &attrs);
private:
    list<Filter> &filterList;
    Filter filter;
};

// ===========================================================================
// Concrete handler for <name> elements.  Grabs the text string.
//
class NameElementHandler : public ElementHandler {
public:
    NameElementHandler(string &s);
    virtual const char *tag();
    virtual void endElement();
private:
    string &str;
};

// ===========================================================================
// Concrete handler for <group> elements.  Adds to the STRING2FILTERS map,
// an entry mapping the message group's id to its list of Filters.
//
class GroupElementHandler : public ElementHandler {
public:
    GroupElementHandler(STRING2FILTERS &gf);
    virtual const char *tag();
    virtual ElementHandler *makeChild(const XMLCh *);
    virtual void endElement();
private:
    STRING2FILTERS &groupFilters;
    string name;
    list<Filter> filterList;
    bool have_name;
};

// ===========================================================================
// Concrete class for the <percolate> element.
// Gathers data for the STRING2FILTERS map.
//
class PercolateElementHandler : public ElementHandler {
public:
    PercolateElementHandler(STRING2FILTERS &gf);
    virtual const char *tag();
    virtual ElementHandler *makeChild(const XMLCh *);
    virtual void endElement();
private:
    STRING2FILTERS &groupFilters;
};

// ===========================================================================
// ===========================================================================
// Concrete class for the object which handles document-oriented parse events.
//
class ConfigDocumentHandler : public DocumentHandler {
public:

    ConfigDocumentHandler(STRING2FILTERS &gf);
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
    void setDocumentLocator(Locator const* const locator);
    void startDocument();
    void startElement (
        const   XMLCh* const    name
        ,       AttributeList&  attrs
    );
    typedef bool ConfigDocumentHandler::*attribute;

private:
    list<ElementHandler *> nest;
    STRING2FILTERS &groupFilters;
};

// ===========================================================================
// Class to hold the state of parsing a configuration file.
//
class ConfigParser {
public:
    ConfigParser(const char *config_file, string *&diagnostic, 
		       STRING2FILTERS &groupFilters);
    void parse();
    bool success();

private:
    const char *config_file;
    STRING2FILTERS &groupFilters;

    bool have_warning;
    bool have_error;
    bool have_fatality;
    string*&diagnostic;

    ConfigErrorHandler errorHandler;
    ConfigDocumentHandler documentHandler;
};

// ===========================================================================
// For each message group name, when processing a message we don't want to
// look through its whole list of filters.  Instead during initialization
// the filter list is organized into the GroupData structure, for fast
// access.  The messageIdFilters can be looked up by message id, etc.
// Any filters which remain uncategorized are left in 'otherFilters'.
// Each <filter> from the config file is placed in just one of these
// categories.
//
struct GroupData {
    STRING2FILTERS messageIdFilters;
    INT2FILTERS severityFilters;
    STRING2FILTERS groupIdFilters;
    list<Filter> otherFilters;
};


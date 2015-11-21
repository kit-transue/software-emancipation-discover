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
// Implements 1. the parser for the configuration file, as well as
//            2. routines and classes to create data structures
//               for efficent message severity filtering
//            3. routines to enable message severity filtering
// The configuration file is structured as XML data.
//

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
#include <list>
#include <map>
#include "limits.h"
#include "filtersev_internal.h"

#include "minisax.h"
#include "msg.h"
using namespace std;
using namespace MiniXML;

// ===========================================================================
// ===========================================================================
// Utilities
// ===========================================================================
//
static void string2severity(lp_msg_sev &sev, const string &str)
{
    lp_msg_sev s = severity_from_string(str.c_str());
    if (s != no_lp_sev) {
	sev = s;
    }
}

// ===========================================================================
// Compare a string of XML characters to a string of chars for equality.
//
static bool charsEqual(const XMLCh *xmlchars, const char *chars)
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

string Condition::getId() { return ""; }
lp_msg_sev Condition::getSev() { return no_lp_sev; }

MessageIdCondition::MessageIdCondition(string s) : id(s) {}
bool MessageIdCondition::filter(const string &s, const lp_msg_sev, bool child_is_message_group) {
    return !child_is_message_group && id == s;
}
condition_kind MessageIdCondition::kind() { return message_id_ck; }
string MessageIdCondition::getId() { return id; }

SeverityCondition::SeverityCondition(lp_msg_sev sev) : severity(sev) {}
bool SeverityCondition::filter(const string &, const lp_msg_sev sev, bool) {
    return severity == sev;
}
condition_kind SeverityCondition::kind() { return severity_ck; }
lp_msg_sev SeverityCondition::getSev() { return severity; }

GroupIdCondition::GroupIdCondition(string s) : id(s) {}
bool GroupIdCondition::filter(const string &s, const lp_msg_sev, bool child_is_message_group) {
    return child_is_message_group && id == s;
}
condition_kind GroupIdCondition::kind() { return group_id_ck; }
string GroupIdCondition::getId() { return id; }

IsGroupCondition::IsGroupCondition() {}
bool IsGroupCondition::filter(const string &s, const lp_msg_sev, bool child_is_message_group) {
    return child_is_message_group;
}
condition_kind IsGroupCondition::kind() { return is_group_ck; }

bool operator < (Filter one, Filter two) {
    return one.conditions < two.conditions;
}

bool operator == (Filter one, Filter two) {
    return one.conditions == two.conditions;
}

// ===========================================================================
// Configuration file parser.
// This parser uses a subset of IBM's SAX API, distributed in xml4c.
// SAX is a standard multi-language API for event-based parsing of XML.
// ===========================================================================
// Abort if SAX calls back to a stub.
//
static void unimplemented(char const * name) {
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
	updateDiagnostic("warning", exception);
    }
    have_warning = true;
}

// ===========================================================================
//
void ConfigErrorHandler::error(const SAXParseException& exception) {
    if (!(have_error || have_fatality)) {
	updateDiagnostic("error", exception);
    }
    have_error = true;
}

// ===========================================================================
//
void ConfigErrorHandler::fatalError(const SAXParseException& exception) {
    if (!(have_error || have_fatality)) {
	updateDiagnostic("fatal error", exception);
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
void ConfigErrorHandler::updateDiagnostic(const char *kind, 
					  const SAXParseException& exception) {
    string message = XMLsToString(exception.getMessage());
#ifdef ISO_CPP_HEADERS
    ostringstream composed;
#else
    ostrstream composed;
#endif
    composed << kind << " on line " << exception.getLineNumber() << ": " << message;
    if (diagnostic == NULL) {
	diagnostic = new string;
    }
#ifndef ISO_CPP_HEADERS
    composed << ends;
#endif
    *diagnostic = composed.str();
}

// ===========================================================================
//
static bool setStringFromAttr(string &str, const XMLCh *attr_name, 
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
static const XMLCh sev_str[] = { 's','e','v','\0' };
static const XMLCh val_str[] = { 'v','a','l','\0' };

// ===========================================================================
//
// Called to make a child of this element, of the given tag name.
// Returns NULL if the name fails to match an appropriate child object.
ElementHandler *ElementHandler::makeChild(const XMLCh *name) {
    return NULL;
}

// Called when characters from element's text are recognized by the
// parser.
void ElementHandler::characters(const string &s) {
    text.append(s);
}

// Called when the element is begun, giving the element handler its
// opportunity to grab attributes.
void ElementHandler::attributes(AttributeList &) {}

// ===========================================================================
//
MessageIdElementHandler::MessageIdElementHandler(Filter &f) : filter(f) {}
const char *MessageIdElementHandler::tag() { return "messageid"; }
void MessageIdElementHandler::endElement() {
    filter.conditions.push_back(new MessageIdCondition(text));
}

// ===========================================================================
//
SeverityElementHandler::SeverityElementHandler(Filter &f) : filter(f) {}
const char *SeverityElementHandler::tag() { return "severity"; }
void SeverityElementHandler::endElement() {
    filter.conditions.push_back(new SeverityCondition(value));
    if (value == no_lp_sev) {
	cerr << "In file filtersev.xml, a <severity> element has a missing or invalid \"val\" attribute." << endl;
    }
}
void SeverityElementHandler::attributes(AttributeList &attrs) {
    string sevStr;
    setStringFromAttr(sevStr, val_str, attrs);
    value = no_lp_sev;
    string2severity(value, sevStr);
}

// ===========================================================================
//
GroupIdElementHandler::GroupIdElementHandler(Filter &f) : filter(f) {}
const char *GroupIdElementHandler::tag() { return "groupid"; }
void GroupIdElementHandler::endElement() {
    filter.conditions.push_back(new GroupIdCondition(text));
}

// ===========================================================================
//
IsGroupElementHandler::IsGroupElementHandler(Filter &f) : filter(f) {}
const char *IsGroupElementHandler::tag() { return "isgroup"; }
void IsGroupElementHandler::endElement() {
    filter.conditions.push_back(new IsGroupCondition());
}

// ===========================================================================
//
UnknownElementHandler::UnknownElementHandler() {}
const char *UnknownElementHandler::tag() { return ""; }
void UnknownElementHandler::endElement() {}

// ===========================================================================
//
FilterElementHandler::FilterElementHandler(list<Filter> &fl) : filterList(fl) {}
const char *FilterElementHandler::tag() { return "filter"; }
void FilterElementHandler::endElement() {
    if (filter.sev == no_lp_sev) {
	cerr << "In filtersev.xml, filter element is missing a \"sev\" attribute." << endl;
    }
    else {
	filterList.push_back(filter);
    }
}
void FilterElementHandler::attributes(AttributeList &attrs) {
    string sevStr;
    setStringFromAttr(sevStr, sev_str, attrs);
    filter.sev = no_lp_sev;
    string2severity(filter.sev, sevStr);
}
ElementHandler *FilterElementHandler::makeChild(const XMLCh *name) {
    if (charsEqual(name, "messageid")) {
	return new MessageIdElementHandler(filter);
    }
    else if (charsEqual(name, "severity")) {
	return new SeverityElementHandler(filter);
    }
    else if (charsEqual(name, "groupid")) {
	return new GroupIdElementHandler(filter);
    }
    else if (charsEqual(name, "isgroup")) {
	return new IsGroupElementHandler(filter);
    }
    return NULL;
}

// ===========================================================================
//
NameElementHandler::NameElementHandler(string &s) : str(s) {}
const char *NameElementHandler::tag() { return "name"; }
void NameElementHandler::endElement() {
    str = text;
}

// ===========================================================================
//
GroupElementHandler::GroupElementHandler(STRING2FILTERS &gf) : groupFilters(gf), have_name(false) {}
const char *GroupElementHandler::tag() { return "group"; }
void GroupElementHandler::endElement() {
    if (!have_name) {
	cerr << "In filtersev.xml, a message group has no name." << endl;
    }
    groupFilters.insert(STRING2FILTERS::value_type(name, filterList));
}
ElementHandler *GroupElementHandler::makeChild(const XMLCh *tag_name) {
    if (charsEqual(tag_name, "name")) {
	have_name = true;
        return new NameElementHandler(name);
    }
    else if (charsEqual(tag_name, "filter")) {
	return new FilterElementHandler(filterList);
    }
    return NULL;
}

// ===========================================================================
//
PercolateElementHandler::PercolateElementHandler(STRING2FILTERS &gf) : groupFilters(gf) {}
const char *PercolateElementHandler::tag() { return "percolate"; }
void PercolateElementHandler::endElement() {}
ElementHandler *PercolateElementHandler::makeChild(const XMLCh *name) {
    if (charsEqual(name, "group")) {
        return new GroupElementHandler(groupFilters);
    }
    return NULL;
}

// ===========================================================================
//
void ConfigDocumentHandler::endDocument () {}
void ConfigDocumentHandler::ignorableWhitespace (
        const   XMLCh* const    chars
        , const unsigned int    length
) {}
void ConfigDocumentHandler::processingInstruction (
        const   XMLCh* const    target
        , const XMLCh* const    data
) { unimplemented("ConfigDocumentHandler::processingInstruction"); }

void ConfigDocumentHandler::resetDocument() {}
// ===========================================================================
// This function is called by the SAX parser to give us a Locator.  Since
// we do not plan to do any locating, we will just disregard it.
// 
void ConfigDocumentHandler::setDocumentLocator(Locator const*) {}

void ConfigDocumentHandler::startDocument() {}

// ===========================================================================
//
ConfigDocumentHandler::ConfigDocumentHandler(STRING2FILTERS &gf)
: groupFilters(gf)
{
}

// ===========================================================================
//
void ConfigDocumentHandler::characters (
    const   XMLCh* const    chars
    , const unsigned int    length)
{
    if (!nest.empty()) {
	string str(XMLsToString(chars));
	nest.back()->characters(str);
    }
}

// ===========================================================================
//
void ConfigDocumentHandler::endElement(const XMLCh* const name) {
    if (!nest.empty()) {
	const char *start_tag = nest.back()->tag();
    	if (!charsEqual(name, start_tag)) {
	    string nmstr = XMLsToString(name);
	    if (strlen(start_tag) > 0) {
    		cerr << "Mismatched tags in filtersev.xml: " << nmstr.c_str()
		     << ", " << start_tag << endl;
    	    }
	    else {
		cerr << "Unexpected tag in filtersev.xml: " << nmstr.c_str() << endl;
	    }
	}
    	nest.back()->endElement();
    	nest.pop_back();
    }
}

// ===========================================================================
//
void ConfigDocumentHandler::startElement (
    const   XMLCh* const    name
    ,       AttributeList&  attrs) 
{
    ElementHandler *newtop = NULL;
    if (!nest.empty()) {
        newtop = nest.back()->makeChild(name);
    }
    else if (charsEqual(name, "percolate")) {
	newtop = new PercolateElementHandler(groupFilters);
    }
    if (newtop == NULL) {
	newtop = new UnknownElementHandler();
    }
    nest.push_back(newtop);

    newtop->attributes(attrs);
}

// ===========================================================================
ConfigParser::ConfigParser(const char *config_file, string *&diagnostic,
			   STRING2FILTERS &gf)

		       : config_file(config_file), diagnostic(diagnostic),
		       groupFilters(gf),
		       have_warning(false),
		       have_error(false),
		       have_fatality(false),
		       documentHandler(groupFilters),
		       errorHandler(diagnostic, have_warning, have_error, have_fatality)
{
}

// ===========================================================================
bool ConfigParser::success() { return !(have_error || have_fatality); }

// ===========================================================================
void ConfigParser::parse () {
    Parser *p = getParser();
    if (p != NULL) {
	Parser &parser = *p;
	parser.setDocumentHandler(&documentHandler);
	parser.setErrorHandler(&errorHandler);
	parser.parse(config_file);
    }
}

// ===========================================================================
// High-level configuration routines and their helper data.
//
// ===========================================================================
// Parse a configuration file describing how a particular compiler handles its
// command line arguments, and how to translate those arguments for aset_CCcc.
// The file is in a simplified XML data format.
//
static bool parse_config_file(const char *config_file, string *&diagnostic,
		       STRING2FILTERS &groupFilters) {
    ConfigParser theParser(config_file, diagnostic, groupFilters);
    theParser.parse();
    return theParser.success();
}

// whether this filtersev is initialized
static bool isInitialized = false;

typedef map<string, GroupData> STRING2GROUP;
static STRING2GROUP groupData;

// ===========================================================================
// Analyze the filters which emerge from the parsing, to create the groupData.
//
static void massage(STRING2FILTERS &groupFilters)
{
    // Set up groupData.
    groupData.clear();
    STRING2FILTERS::iterator i;
    for (i = groupFilters.begin(); i != groupFilters.end(); ++i) {
	// have a message group id and its associated filtering rules
	GroupData gd;
	int seq = 0;
	list<Filter>::iterator li;
	for (li = i->second.begin(); li != i->second.end(); ++li, ++seq) {
	    // have one filtering rule
	    // Place the filtering rule in an appropriate section of GroupData.
	    string id;
	    lp_msg_sev sev;
	    condition_kind kind = no_ck;
	    li->seq = seq;
	    list<Condition *> &conditions = li->conditions;
	    list<Condition *>::iterator ci;
	    for (ci = conditions.begin(); ci != conditions.end(); ++ci) {
		condition_kind this_kind = (*ci)->kind();
		if (this_kind < kind) {
		    kind = this_kind;
		    if (this_kind == message_id_ck || this_kind == group_id_ck) {
			id = (*ci)->getId();
		    }
		    else if (this_kind == severity_ck) {
			sev = (*ci)->getSev();
		    }
		}
	    }
	    list<Filter> *listObj = NULL;
	    if (kind == message_id_ck) {
		STRING2FILTERS::iterator f = gd.messageIdFilters.find(id);
		if (f == gd.messageIdFilters.end()) {
		    gd.messageIdFilters.insert(STRING2FILTERS::value_type(id, list<Filter>()));
		    f = gd.messageIdFilters.find(id);
		}
		listObj = &(f->second);
	    }
	    else if (kind == severity_ck) {
		INT2FILTERS::iterator f = gd.severityFilters.find(sev);
		if (f == gd.severityFilters.end()) {
		    gd.severityFilters.insert(INT2FILTERS::value_type(sev, list<Filter>()));
		    f = gd.severityFilters.find(sev);
		}
		listObj = &(f->second);
	    }
	    else if (kind == group_id_ck) {
		STRING2FILTERS::iterator f = gd.groupIdFilters.find(id);
		if (f == gd.groupIdFilters.end()) {
		    gd.groupIdFilters.insert(STRING2FILTERS::value_type(id, list<Filter>()));
		    f = gd.groupIdFilters.find(id);
		}
		listObj = &(f->second);
	    }
	    else {
		listObj = &gd.otherFilters;
	    }
	    listObj->push_back(*li);
	}
	groupData.insert(STRING2GROUP::value_type(i->first, gd));
    }
}

// ===========================================================================
// Called to initialize this filtersev module, this reads the configuration
// file filtersev.xml, and constructs the groupData.
//
bool initialize_filtersev()
{
    const char *home = getenv("PSETHOME");
    if (home == NULL) {
	return false;
    }
#ifdef ISO_CPP_HEADERS
    stringstream config_file_strm;
#else
    strstream config_file_strm;
#endif
    config_file_strm << home;
#ifdef _WIN32
    config_file_strm << "\\lib\\filtersev.xml";
#else
    config_file_strm << "/lib/filtersev.xml";
#endif
#ifndef ISO_CPP_HEADERS
    config_file_strm << ends;
#endif
    string config_file_str = config_file_strm.str();
    string *diagnostic = NULL;
    STRING2FILTERS groupFilters;
    bool success = parse_config_file(config_file_str.c_str(), 
				     diagnostic, groupFilters);
    if (!success) {
	cerr << "Error parsing filtersev.xml: "
	     << (diagnostic != NULL ? diagnostic->c_str() : "no diagnostic available")
	     << endl;
	return false;
    }
    massage(groupFilters);
    isInitialized = true;
    cerr << "did initialize filtersev" << endl;
    return isInitialized;
}

// ===========================================================================
// Filtering routines.
//
// ===========================================================================
// Finds the first filter in the given list, which matches a message having
// the characteristics 'child_id', 'child_sev', 'child_is_message_group'.
// Returns true if one is found, in which case 'sev' and 'seq' are set to
// the found filter's replacement severity and configuration sequence.
//
static bool find_filter(list<Filter> &filters, const string &child_id,
					       lp_msg_sev child_sev,
					       bool child_is_message_group,
					       lp_msg_sev &sev,
					       int &seq)
{
    list<Filter>::iterator fi;
    for (fi = filters.begin(); fi != filters.end(); ++fi) {
	list<Condition *> &conds = fi->conditions;
	list<Condition *>::iterator fi2;
	bool match = true;
	for (fi2 = conds.begin(); fi2 != conds.end(); ++fi2) {
	    if (!(*fi2)->filter(child_id, child_sev, child_is_message_group)) {
		match = false;
	    }
	}
	if (match) {
	    sev = fi->sev;
	    seq = fi->seq;
	    return true;
	}
    }
    return false;
}

// ===========================================================================
// Look for a filter which applies in the context of the given parent
// message group id, and child properties.  'child_id' is the message group
// id if the child is a message group; otherwise it is the message id.
// If no filter is found, it returns 'child_sev', otherwise returns the
// replacement severity.
//
lp_msg_sev filter_severity(const string &parent_group_id, 
			   const string &child_id, 
			   lp_msg_sev child_sev,
			   bool child_is_message_group)
{
    if (!isInitialized) {
	return child_sev;
    }
    STRING2GROUP::iterator atry = groupData.begin();
    STRING2GROUP::iterator theIterator = groupData.find(parent_group_id);
    if (theIterator == groupData.end()) {
	return child_sev;
    }
    GroupData &gd = (*theIterator).second;
    // Now go through the data, and find what to do.

    lp_msg_sev found_sev;
    lp_msg_sev new_sev = child_sev;
    int new_seq = INT_MAX;
    int found_seq;
    STRING2FILTERS *filters
	= child_is_message_group ? &gd.groupIdFilters : &gd.messageIdFilters;
    STRING2FILTERS::iterator f = filters->find(child_id);
    if (f != filters->end()
	&& find_filter(f->second, child_id, child_sev, child_is_message_group, found_sev, found_seq)) {

	if (found_seq < new_seq) {
	    new_seq = found_seq;
	    new_sev = found_sev;
	}
    }
    INT2FILTERS::iterator sf = gd.severityFilters.find(child_sev);
    if (sf != gd.severityFilters.end()
	&& find_filter(sf->second, child_id, child_sev, child_is_message_group, found_sev, found_seq)) {

	if (found_seq < new_seq) {
	    new_seq = found_seq;
	    new_sev = found_sev;
	}
    }
    if (find_filter(gd.otherFilters, child_id, child_sev, child_is_message_group, found_sev, found_seq)) {
	if (found_seq < new_seq) {
	    new_seq = found_seq;
	    new_sev = found_sev;
	}
    }
    return new_sev;
}

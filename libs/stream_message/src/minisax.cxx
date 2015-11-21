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
// Implements a subset SAX parser for XML.
// See minisax.h for an overview of the role of this module.
//
// This file contains:
//   utility functions
//   a concrete AttributeList class
//   a concrete Parser class
//   an ActualParser class which does the grunge work of parsing
//   definition of the getParser function
//   implementation of the SAXException class
//   implementation of the SAXParseException class

#ifdef _WIN32
#pragma warning (disable:4786)
#endif
int ttt;
#include "minisax.h"
#include "msg.h"
#include <stddef.h>
#ifdef ISO_CPP_HEADERS
#include <sstream>
#include <fstream>
#else
#include <strstream.h>
#include <fstream.h>
#endif
//#include <string.h>
#include "ctype.h"
#include <map>
#include <string>

using namespace std;
using namespace MiniXML;

// ===========================================================================
// ===========================================================================
// Utility Functions
// ===========================================================================
// Abort because an unimplemented part of SAX is used.
//
static void unimplemented() {
    msg("MiniSAX is not full SAX.", catastrophe_sev) << eom;
    exit(1);
}

// ===========================================================================
// ===========================================================================
// For the startElement callback, the SAX parser must supply an attribute
// list object.  ConcreteAttributeList is also set up to be
// fabricated on the fly, as the attribute list is parsed.
//
class ConcreteAttributeList : public AttributeList {
public:
    ConcreteAttributeList();
    ~ConcreteAttributeList();
    unsigned int getLength() const;
    const XMLCh* getName(const unsigned int i) const;
    const XMLCh* getType(const unsigned int i) const;
    const XMLCh* getValue(const unsigned int i) const;
    const XMLCh* getType(const XMLCh* const name) const;
    const XMLCh* getValue(const XMLCh* const name) const;
    
    typedef map<string, string*> attrs_type;
    attrs_type attrs;
    mutable const XMLCh *temp_XMLs;
    void update_temp(const char *s) const;
};

// ===========================================================================
// The destructor must delete the attribute value strings and the temp
// string.
//
ConcreteAttributeList::ConcreteAttributeList() : temp_XMLs(NULL) {
}

// ===========================================================================
// The destructor must delete the attribute value strings and the temp
// string.
//
ConcreteAttributeList::~ConcreteAttributeList() {
    attrs_type::iterator i;
    for (i = attrs.begin(); i != attrs.end(); ++i) {
	string *val = (*i).second;
	delete val;
    }
    update_temp(NULL);
}

// ===========================================================================
unsigned int ConcreteAttributeList::getLength() const {
    return attrs.size();
}

// ===========================================================================
const XMLCh* ConcreteAttributeList::getName(const unsigned int i) const {
    attrs_type::const_iterator iter = attrs.begin();
    advance(iter, i);
    update_temp((*iter).first.c_str());
    return temp_XMLs;
}

// ===========================================================================
const XMLCh* ConcreteAttributeList::getType(const unsigned int i) const {
    unimplemented();
    return NULL;
}

// ===========================================================================
const XMLCh* ConcreteAttributeList::getValue(const unsigned int i) const {
    attrs_type::const_iterator iter = attrs.begin();
    advance(iter, i);
    update_temp((*iter).second->c_str());
    return temp_XMLs;
}

// ===========================================================================
const XMLCh* ConcreteAttributeList::getType(const XMLCh* const name) const {
    unimplemented();
    return NULL;
}

// ===========================================================================
const XMLCh* ConcreteAttributeList::getValue(const XMLCh* const name) const {
    const char *s = XMLsToChars(name);
    string str = s;
    delete [] (char *)s;
    attrs_type::const_iterator iter = attrs.find(str);
    if (iter != attrs.end()) {
	string *valp = (*iter).second;
	update_temp(valp != NULL ? valp->c_str() : NULL);
	return temp_XMLs;
    }
    return NULL;
}

// ===========================================================================
// The temp_XMLs object serves to hold the requested string until the next
// such string is requested.  Probably this is not long enough for real SAX,
// but it enables our parser to do what it needs to do.
//
void ConcreteAttributeList::update_temp(const char *str) const {
    delete [] (XMLCh *)temp_XMLs;
    if (str == NULL) {
	temp_XMLs = NULL;
    }
    else {
	temp_XMLs = charsToXMLs(str);
    }
}

// ===========================================================================
// ===========================================================================
// This class belongs to a partial implementation of SAX. It's in an odd
// position, because the SAX API does not define a concrete class for its
// parser, and yet a parser object must exist in order to use SAX.
// ConcreteParser declares and implements only those methods of Parser
// which we require.
//
// The first bunch of member functions are demanded by SAX.
// The second bunch are called from the ActualParser class,
// as a convenient way to handle parse events.
//
class ConcreteParser : public Parser {
public:
    ConcreteParser();
    // void setEntityResolver(EntityResolver* const resolver);
    // void setDTDHandler(DTDHandler* const handler);
    void setDocumentHandler(DocumentHandler* const handler);
    void setErrorHandler(ErrorHandler* const handler);
    void parse (const char* const systemId, const bool = false);
    void parse (const char* const bufPtr, unsigned long bufLen);

    DocumentHandler *docHandler;
    ErrorHandler *errorHandler;

    void fatalError(const char *diagnostic, int line, int col);
    void error(const char *diagnostic, int line, int col);

#ifdef ISO_CPP_HEADERS
    void noteContent(ostringstream &str);
#else
    void noteContent(ostrstream &str);
#endif
    void noteStartElement(const string &tag_name, AttributeList &attrs);
    void noteEndElement(const string &tag_name);
    void noteBeginFile();
    void noteEndFile();
};

// ===========================================================================
// ===========================================================================
// ActualParser does the dirty work of tracking the input stream and the
// constructs of XML. It reports interesting events such as element constructs
// and parse errors via the SAXParser object.
//
class ActualParser {
public:
    ActualParser(ConcreteParser &);

private:
    ConcreteParser &SAXParser;

    int curLine;
    int curCol;
    istream *input;

    void accountChar(char);
    bool getChar(char &ch);
    bool matchChar(char ch);
    bool atChar(char ch);
    bool matchChar(bool (*test)(char), char &matched);
    bool getName(string &name);
    bool getQuotedValue(string &val);
    void skipWhitespace();
    void recoverFromGarbledTag();
    void getAttrs(ConcreteAttributeList &attrs);
    void getContent(string *containing_tag);
    void getElement();
    void getFile();

    void fatalError(const char *diagnostic);
    void error(const char *diagnostic);
public:
    void parse(const char *config_file);
    void parse(const char *bufPtr, unsigned long bufLen);

};

// ===========================================================================
ActualParser::ActualParser(ConcreteParser &par) : SAXParser(par)
{
}

// ===========================================================================
void ActualParser::accountChar(char inch) {
    if (inch == '\n') {
	curLine += 1;
	curCol = 0;
    }
    else {
	curCol += 1;
    }
}

// ===========================================================================
// Parse a particular character.
bool ActualParser::matchChar(char ch) {
    char inch;
    input->get(inch);
    if (input->eof()) {
	return false;
    }
    bool success = (inch == ch);
    if (success) {
	accountChar(inch);
    }
    else {
	input->putback(inch);
    }
    return success;
}

// ===========================================================================
// Test for a particular next character.
bool ActualParser::atChar(char ch) {
    char inch;
    input->get(inch);
    if (input->eof()) {
	return false;
    }
    bool success = (inch == ch);
    input->putback(inch);
    return success;
}

// ===========================================================================
// Parse a character that meets a test.
bool ActualParser::matchChar(bool (*test)(char), char &matched) {
    char inch;
    input->get(inch);
    if (input->eof()) {
	return false;
    }
    bool success = test(inch);
    if (!success) {
	input->putback(inch);
    }
    else {
	matched = inch;
	accountChar(inch);
    }
    return success;
}

// ===========================================================================
static bool is_name_char(char ch) {
    return isalpha(ch) != 0 || ch == '_';
}

// ===========================================================================
static bool is_whitespace_char(char ch) {
    return isspace(ch) != 0;
}

// ===========================================================================
static bool is_any_char(char ch) {
    return true;
}

// ===========================================================================
// Parse a character.
bool ActualParser::getChar(char &matched) {
    input->get(matched);
    if (input->eof()) {
	return false;
    }
    accountChar(matched);
    return true;
}

// ===========================================================================
// Parse an alphabetic name. Should be fixed eventually to correspond
// better to the valid tag names in XML.
//
bool ActualParser::getName(string &name) {
    char ch;
    if (!matchChar(&is_name_char, ch)) {
	return false;
    }
#ifdef ISO_CPP_HEADERS
    ostringstream nm;
#else
    ostrstream nm;
#endif
    nm << ch;
    for (;;) {
	if (!matchChar(&is_name_char, ch)) {
	    break;
	}
	nm << ch;
    }
#ifndef ISO_CPP_HEADERS
    nm << ends;
#endif
    name = nm.str();
    return true;
}

// ===========================================================================
// Parse a quoted string. Should be fixed eventually to correspond
// better to the attribute values in XML.
//
bool ActualParser::getQuotedValue(string &val_out) {
    if (!matchChar('"')) {
	return false;
    }
#ifdef ISO_CPP_HEADERS
    ostringstream val;
#else
    ostrstream val;
#endif
    char ch;
    for (;;) {
	if (!matchChar(&is_any_char, ch)) {
	    error("bad quoted attribute value");
	    return false;
	}
	if (ch == '"') {
	    break;
	}
	val << ch;
    }
#ifndef ISO_CPP_HEADERS
    val << ends;
#endif
    val_out = val.str();
    return true;

}

// ===========================================================================
// Skip over whitespace to the next non-whitespace character.
//
void ActualParser::skipWhitespace() {
    for(;;) {
	char ch;
	if (!matchChar(&is_whitespace_char, ch)) {
	    break;
	}
    }
}

// ===========================================================================
// skip to > or end of line
//
void ActualParser::recoverFromGarbledTag() {
    for (;;) {
	char ch;
	if (!getChar(ch)) {
	    break;
	}
	if (ch == '>' || ch == '\n') {
	    break;
	}
    }
}

// ===========================================================================
bool entity_match(const char *p, const char *name)
{
    while (*name != '\0') {
	if (*p != *name) {
	    return false;
	}
	name += 1;
	p += 1;
    }
    return (*p == ';');
}

// ===========================================================================
// A skimpy implementation that substitutes for
// &quot; etc.  It really should be handling lots more, and producing a
// stream of XMLCh.  In fact, its input should be XMLCh.
//
#ifdef ISO_CPP_HEADERS
void substitute_entities(ostringstream &strm, const char *bufPtr, unsigned long bufLen)
#else
void substitute_entities(ostrstream &strm, const char *bufPtr, unsigned long bufLen)
#endif
{
    int i = 0;
    while (i < bufLen) {
	char ch = bufPtr[i];
	if (ch == '&') {
	    const char *p = bufPtr + (i+1);
	    if (entity_match(p, "quot")) {
		strm << '"';
		i += 5;
	    }
	    else if (entity_match(p, "amp")) {
		strm << '&';
		i += 4;
	    }
	    else if (entity_match(p, "lt")) {
		strm << '<';
		i += 3;
	    }
	    else if (entity_match(p, "gt")) {
		strm << '>';
		i += 3;
	    }
	    else {
		strm << ch;
	    }
	}
	else {
	    strm << ch;
	}
	i += 1;
    }
}

// ===========================================================================
// Parse attributes of a begin tag, up to the > or />.
//
void ActualParser::getAttrs(ConcreteAttributeList &attrs) {
    bool garbled = false;
    for (;;) {
	skipWhitespace();
	string attr_name;
	if (getName(attr_name)) {
	    skipWhitespace();
	    if (matchChar('=')) {
		skipWhitespace();
		string attr_value;
		if (getQuotedValue(attr_value)) {
		}
		else {
#ifdef ISO_CPP_HEADERS
		    ostringstream d;
#else
		    ostrstream d;
#endif
		    d << "bad attribute " << attr_name;
#ifndef ISO_CPP_HEADERS
		    d << ends;
#endif
		    string d_str = d.str();
		    error(d_str.c_str());
		}
#ifdef ISO_CPP_HEADERS
		ostringstream value_stream;
#else
		ostrstream value_stream;
#endif
		substitute_entities(value_stream, attr_value.c_str(), attr_value.length());
#ifndef ISO_CPP_HEADERS
		value_stream << ends;
#endif
		attrs.attrs.insert(pair<const string,string*>(attr_name, new string(value_stream.str())));
	    }
	    else {
		attrs.attrs.insert(pair<const string,string*>(attr_name, NULL));
	    }
	}
	else if (atChar('>') || atChar('/')) {
	    if (garbled) {
		error("bad tag");
	    }
	    break;
	}
	else {
	    char ch;
	    garbled = true;
	    if (!getChar(ch)) {
		error ("bad tag");
		break;
	    }
	}
    }
}

// ===========================================================================
// Parse content of an element, up to the end of file or an
// end-of-element marker.
//
void ActualParser::getContent(string *containing_tag) {
#ifdef ISO_CPP_HEADERS
    ostringstream chars;
#else
    ostrstream chars;
#endif
    for (;;) {
        char ch;
	if (!getChar(ch)) {
	    if (containing_tag != NULL) {
		error("unmatching start tag");
	    }
    	    break;
        }
        if (ch == '<') {
	    SAXParser.noteContent(chars);
	    if (containing_tag != NULL && matchChar('/')) {
		skipWhitespace();
		string tag;
		if (getName(tag)) {
		    if (tag != *containing_tag) {
			error("unmatching end tag");
		    }
		    skipWhitespace();
		    if (!matchChar('>')) {
			error("bad end tag");
			recoverFromGarbledTag();
		    }
		}
		else {
		    error("bad end tag");
		}		
		break;
	    }
	    else if (matchChar('!')) {
			// could be a comment
			if (matchChar('-')) {
				// could be a comment.
				if (matchChar('-')) {
				// is a comment.  Look for -->
				int hyphen_count = 0;
				for (;;) {
					char ch;
					if (!getChar(ch)) {
					error("bad comment");
					break;
					}
					else if (hyphen_count >= 2 && ch == '>') {
					break;
					}
					else {
					if (ch == '-') {
						hyphen_count += 1;
					}
					else {
						hyphen_count = 0;
					}
					}
				}
				}
				else {
				error ("bad tag");
				recoverFromGarbledTag();
				}
			}
			else if (matchChar('D')) {
				// could be a DOCTYPE tag with DTD definition.
				char ch;
				char tagname [] = { 'O', 'C', 'T', 'Y', 'P', 'E' };
				for(int cntr = 0; cntr < 6; cntr++) {
					if (!getChar(ch)) {
						error("bad tag");
						break;
					}
					else {
					if (tagname[ cntr ] == ch) {
						continue;
					}
					else {
						error("bad tag");
						recoverFromGarbledTag();
						break;
					}
					}
				}
				// is a DOCTYPE tag Look for >
				for(;;) {
					if (!getChar(ch)) {
						error("bad tag");
						break;
					}
					else if('>' == ch) {
						break;
					}
				}

			}
			else {
			error ("bad tag");
			recoverFromGarbledTag();
			}
		}
		else if (matchChar('?')) {
		// is a xml header & encoding tag.  Look for ?>
			bool questionmark = false;
			char ch;
			for(;;) {
				if (!getChar(ch)) {
					error("bad tag");
					break;
				}
				else if(questionmark && '>' == ch) {
					break;
				}
				else {
				if ('?' == ch) {
					questionmark = true;
				}
				else {
					questionmark = false;
				}
				}
				
			}
		}
	    else {
		getElement();
	    }
        }
        else {
	    chars << ch;
        }
    }
    SAXParser.noteContent(chars);
}

// ===========================================================================
// Parse an element and its contents. The initial '<' is past.
//
void ActualParser::getElement() {
    if (matchChar('/')) {
	error("unmatching end tag");
	recoverFromGarbledTag();
	return;
    }
    skipWhitespace();
    string tag;
    if (getName(tag)) {
	ConcreteAttributeList attrs;
	getAttrs(attrs);
	char ch;
	bool have_char = getChar(ch);
	bool is_end = have_char && (ch == '/');
	if (is_end) {
	    have_char = getChar(ch);
	}
	if (have_char && ch == '>') {
	    SAXParser.noteStartElement(tag, attrs);
	    if (!is_end) {
		getContent(&tag);
	    }
	    SAXParser.noteEndElement(tag);
	}
	else {
	    is_end = false;
	    error("bad tag");
	    recoverFromGarbledTag();
	}
    }
    else {
        error("bad tag");
	recoverFromGarbledTag(); // skip to > or end of line
    }
}

// ===========================================================================
// Parse a file.
//
void ActualParser::getFile() {
    SAXParser.noteBeginFile();
    getContent(NULL);
    SAXParser.noteEndFile();
}

// ===========================================================================
void ActualParser::parse(const char *config_file) {
    ifstream i;
    i.open(config_file);
    if (i.fail()) {
#ifdef ISO_CPP_HEADERS
    	ostringstream d;
#else
    	ostrstream d;
#endif
    	d << "file " << config_file << " failed to open";
#ifndef ISO_CPP_HEADERS
	d << ends;
#endif
	string d_str = d.str();
	curLine = 0;
	curCol = 0;
	fatalError(d_str.c_str());
	return;
    }
    input = &i;
    curLine = 1;
    curCol = 0;
    getFile();
}

// ===========================================================================
void ActualParser::parse(const char *bufPtr, unsigned long bufLen) {
#ifdef ISO_CPP_HEADERS
    istringstream i(string(bufPtr, bufLen));
#else
    istrstream i(bufPtr, bufLen);
#endif
    if (i.fail()) {
#ifdef ISO_CPP_HEADERS
    	ostringstream d;
#else
    	ostrstream d;
#endif
    	d << "memory buffer of bytes failed";
#ifndef ISO_CPP_HEADERS
	d << ends;
#endif
	string d_str = d.str();
	curLine = 0;
	curCol = 0;
	fatalError(d_str.c_str());
	return;
    }
    input = &i;
    curLine = 1;
    curCol = 0;
    getFile();
}

// ===========================================================================
void ActualParser::fatalError(const char *diagnostic) {
    SAXParser.fatalError(diagnostic, curLine, curCol);
}

// ===========================================================================
void ActualParser::error(const char *diagnostic) {
    SAXParser.error(diagnostic, curLine, curCol);
}

// ===========================================================================
// ===========================================================================
// ConcreteParser member definitions
//
// ===========================================================================
ConcreteParser::ConcreteParser() : docHandler(NULL), errorHandler(NULL) {}
void ConcreteParser::setDocumentHandler(DocumentHandler* const handler) { 
    docHandler = handler; 
}

// ===========================================================================
void ConcreteParser::setErrorHandler(ErrorHandler* const handler) {
    errorHandler = handler;
}

// ===========================================================================
void ConcreteParser::fatalError(const char *diagnostic, int line, int col) {
    const XMLCh *diag = charsToXMLs(diagnostic);
    SAXParseException exception(diag, NULL, NULL, line, col);
    delete [] (XMLCh *)diag;
    if (errorHandler != NULL) {
	errorHandler->fatalError(exception);
    }
}

// ===========================================================================
void ConcreteParser::error(const char *diagnostic, int line, int col) {
    const XMLCh *diag = charsToXMLs(diagnostic);
    SAXParseException exception(diag, NULL, NULL, line, col);
    delete [] (XMLCh *)diag;
    if (errorHandler != NULL) {
	errorHandler->error(exception);
    }
}

// ===========================================================================
#ifdef ISO_CPP_HEADERS
void ConcreteParser::noteContent(ostringstream &str) {
#else
void ConcreteParser::noteContent(ostrstream &str) {
#endif
#if 0
    // This didn't work: a bug in MS VC6?
    const char *strp = str.str().c_str();
#else
#ifndef ISO_CPP_HEADERS
    str << ends;
#endif
    string strobj = str.str();
    const char *strp = strobj.c_str();
#endif

#ifdef ISO_CPP_HEADERS
    ostringstream strm;
#else
    ostrstream strm;
#endif
    substitute_entities(strm, strp, strlen(strp));
#ifndef ISO_CPP_HEADERS
    strm << ends;
#endif
    if (docHandler != NULL) {
	string tn_str = strm.str();
	const XMLCh *tn = charsToXMLs(tn_str.c_str());
	docHandler->characters(tn, XMLs_len(tn));
	delete [] (XMLCh *)tn;
    }
#ifdef ISO_CPP_HEADERS
#ifndef sun5
    // Reset the ostringstream.
    str.str(string());
#else
    // The above ran into a bug on Sun.
    // Hack: need to flush/clear/reinitialize the ostrstream at this point,
    // but flush/clear/init don't do the job.
    {
        str.ostringstream::~ostringstream();
        new (&str) ostringstream();
    }
#endif
#else
    // Hack: need to flush/clear/reinitialize the ostrstream at this point,
    // but flush/clear/init don't do the job.
    {
        str.ostrstream::~ostrstream();
        new (&str) ostrstream();
    }
#endif
}

// ===========================================================================
void ConcreteParser::noteStartElement(const string &tag_name, 
				      AttributeList &attrs) {
    if (docHandler != NULL) {
	const XMLCh *tn = charsToXMLs(tag_name.c_str());
	docHandler->startElement(tn, attrs);
	delete [] (XMLCh *)tn;
    }
}

// ===========================================================================
void ConcreteParser::noteEndElement(const string &tag_name) {
    if (docHandler != NULL) {
	const XMLCh *tn = charsToXMLs(tag_name.c_str());
	docHandler->endElement(tn);
	delete [] (XMLCh *)tn;
    }
}

// ===========================================================================
void ConcreteParser::noteBeginFile() {
    if (docHandler != NULL) {
	docHandler->startDocument();
    }
}

// ===========================================================================
void ConcreteParser::noteEndFile() {
    if (docHandler != NULL) {
	docHandler->endDocument();
    }
}

// ===========================================================================
//
void ConcreteParser::parse (const char* const config_file, const bool) {
    ActualParser parser(*this);
    parser.parse(config_file);
}

// ===========================================================================
//
void ConcreteParser::parse (const char* const bufPtr, unsigned long bufLen) {
    ActualParser parser(*this);
    parser.parse(bufPtr, bufLen);
}

// ===========================================================================
// ===========================================================================
Parser *MiniXML::getParser()
{
    return new ConcreteParser();
}

// ===========================================================================
// ===========================================================================
SAXException::SAXException()
  : message(NULL)
{
}

// ===========================================================================
SAXException::SAXException(const XMLCh* const msg)
  : message(replicate_XMLs(msg))
{
}

// ===========================================================================
SAXException::SAXException(const char* const msg)
  : message(charsToXMLs(msg))
{
}

// ===========================================================================
SAXException::~SAXException() {
    delete [] (XMLCh *)message;
}

// ===========================================================================
const XMLCh* SAXException::getMessage() const
{
    return message;
}

// ===========================================================================
SAXException::SAXException(const SAXException &that)
  : message(replicate_XMLs(that.message))
{
}

// ===========================================================================
SAXException & SAXException::operator=(const SAXException &that)
{
    if (this != &that) {
	delete [] (XMLCh *)message;
	message = replicate_XMLs(that.message);
    }
    return *this;
}

// ===========================================================================
// ===========================================================================
SAXParseException::SAXParseException(const XMLCh* const message, const Locator& locator)
  : SAXException(message),
    publicId(NULL),
    systemId(NULL),
    lineNumber(0),
    columnNumber(0)
{
    unimplemented();
}

// ===========================================================================
SAXParseException::SAXParseException(const XMLCh* const message
				    , const XMLCh* const publicId
				    , const XMLCh* const systemId
				    , const unsigned int lineNumber
				    , const unsigned int columnNumber)
  : SAXException(message),
    publicId(replicate_XMLs(publicId)),
    systemId(replicate_XMLs(systemId)),
    lineNumber(lineNumber),
    columnNumber(columnNumber)
{
}

// ===========================================================================
SAXParseException::SAXParseException(const SAXParseException& that)
  : SAXException(that.getMessage()),
    publicId(replicate_XMLs(that.publicId)),
    systemId(replicate_XMLs(that.systemId)),
    lineNumber(that.lineNumber),
    columnNumber(that.columnNumber)
{
}

// ===========================================================================
SAXParseException::~SAXParseException()
{
    delete [] (XMLCh *)publicId;
    delete [] (XMLCh *)systemId;
}

// ===========================================================================
SAXParseException& SAXParseException::operator=(const SAXParseException& that)
{
    if (this != &that) {
	SAXException::operator=(that.getMessage());
	delete [] (XMLCh *)publicId;
	delete [] (XMLCh *)systemId;
	publicId = replicate_XMLs(that.publicId);
	systemId = replicate_XMLs(that.systemId);
	lineNumber = that.lineNumber;
	columnNumber = that.columnNumber;
    }
    return *this;
}

// ===========================================================================
unsigned int SAXParseException::getColumnNumber() const
{
    return columnNumber;
}

// ===========================================================================
unsigned int SAXParseException::getLineNumber() const
{
    return lineNumber;
}

// ===========================================================================
const XMLCh* SAXParseException::getPublicId() const
{
    return publicId;
}

// ===========================================================================
const XMLCh* SAXParseException::getSystemId() const
{
    return systemId;
}

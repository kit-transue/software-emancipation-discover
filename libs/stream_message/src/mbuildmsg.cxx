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
#include "mbuildmsg.h"
#include "minisax.h"
#include "minidom.h"
#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <sstream>
#else
#include <iostream.h>
#include <strstream.h>
#endif
#include <deque>

using namespace MBDriver;
using namespace MiniXML;
using namespace std;

//----------------------------------------------------------------------------------

void Message::AddText(const DOMString &txt) {
    Message textElement;
    textElement.m_text = txt;
    m_children.Add(textElement);
}

void Message::Encode(DOMString &result) {
    if(m_elementName.length() == 0) {
	EncodeString(result, m_text);
    } else {
	result += chOpenAngle;
	result += m_elementName;
	for(int i = 0; i < m_attributes.GetSize(); i++) {
	    result += chSpace;
	    Attribute &attribute = m_attributes[i];
	    result += attribute.m_attributeName;
	    (result += chEqual) += chDoubleQuote;
	    EncodeString(result, attribute.m_attributeValue);
	    result += chDoubleQuote;
	}
	if(m_text.length() == 0 && m_children.GetSize() == 0) 
	    (result += chForwardSlash) += chCloseAngle;
	else {
	    result += chCloseAngle;
	    EncodeString(result, m_text);
	    for(int i = 0; i < m_children.GetSize(); i++)
		m_children[i].Encode(result);
	    (((result += chOpenAngle) += chForwardSlash) += m_elementName) += chCloseAngle;
	}
    }
}

bool Message::GetAttributeByName(const DOMString &attributeName, DOMString &value) const {
    for(int i = m_attributes.GetSize() - 1; i >= 0; i--)
	if(m_attributes[i].m_attributeName.equals(attributeName)) {
	    value = m_attributes[i].m_attributeValue;
	    return true;
	}
    return false;
}

bool Message::GetAttributeByName(const XMLCh *attributeName, DOMString &value) const {
    for(int i = m_attributes.GetSize() - 1; i >= 0; i--)
	if(m_attributes[i].m_attributeName.equals(attributeName)) {
	    value = m_attributes[i].m_attributeValue;
	    return true;
	}
    return false;
}

void Message::GetText(DOMString &value) const {
    value = DOMString();
    GetTextInternal(value);
}

void Message::GetTextInternal(DOMString &value) const {
    value += m_text;
    for(int i = 0; i < m_children.GetSize(); i++) {
	if (m_children[i].m_elementName.length() == 0) {
	    m_children[i].GetTextInternal(value);
	}
    }
}

bool Message::EnumerateChildren(int &enumerator) const {
    for(int i = 0; i < m_children.GetSize(); i++)
	if(m_children[i].m_elementName.length() != 0) {
	    enumerator = i;
	    return true;
	}
    return false;
}

bool Message::GetNextChild(int &enumerator, Message **msg) const {
    if(enumerator < m_children.GetSize()) {
	*msg = &m_children[enumerator++];
	for(int i = enumerator; i < m_children.GetSize(); i++)
	    if(m_children[i].m_elementName.length() != 0) {
		enumerator = i;
		return true;
	    }
	enumerator = m_children.GetSize();
	return true;
    }
    return false;
}

void Message::EncodeString(DOMString &result, DOMString &toWrite) {
    int		 length = toWrite.length();
    const XMLCh* chars	= toWrite.rawBuffer();

    int index;
    for (index = 0; index < length; index++) {
	switch (chars[index]) {
	case chAmpersand :
	    result += chAmpersand;
	    result += chLatin_a;
	    result += chLatin_m;
	    result += chLatin_p;
	    result += chSemiColon;
	    break;
	
	case chOpenAngle :
	    result += chAmpersand;
	    result += chLatin_l;
	    result += chLatin_t;
	    result += chSemiColon;
	    break;
	
	case chCloseAngle:
	    result += chAmpersand;
	    result += chLatin_g;
	    result += chLatin_t;
	    result += chSemiColon;
	    break;
	
	case chDoubleQuote :
	    result += chAmpersand;
	    result += chLatin_q;
	    result += chLatin_u;
	    result += chLatin_o;
	    result += chLatin_t;
	    result += chSemiColon;
	    break;
	
	default:
	    // If it is none of the special characters, print it as such
	    result += toWrite.charAt(index);
	    break;
	}
    }
}

// ===========================================================================
// ###########################################################################
// ===========================================================================
// ###########################################################################
// ===========================================================================

// Implements the parser for a message.
// Messages are structured as XML data.
// This parser uses a subset of IBM's SAX API, distributed in xml4c.
// SAX is a standard multi-language API for event-based parsing of XML.
//

// ===========================================================================
// Abort if SAX calls back to a stub.
//
static void unimplemented(const char *name) {
    if (name)
	cerr << "Unexpected SAX callback " << name << "." << endl;
    else
	cerr << "Unexpected SAX callback." << endl;
    exit(1);
}

#ifdef XXX_non_static_SAX_callbacks
static void unimplemented() {
    unimplemented(0);
}
#endif
// ===========================================================================
// Concrete class for the object which handles parse errors.
//
class MessageErrorHandler : public ErrorHandler {
public:
    MessageErrorHandler(
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
//
MessageErrorHandler::MessageErrorHandler(
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
void MessageErrorHandler::warning(const SAXParseException& exception) {
    if (!(have_warning || have_error || have_fatality)) {
	// updateDiagnostic("warning", exception);
    }
    have_warning = true;
}

// ===========================================================================
//
void MessageErrorHandler::error(const SAXParseException& exception) {
    if (!(have_error || have_fatality)) {
	updateDiagnostic("error", exception);
    }
    have_error = true;
}

// ===========================================================================
//
void MessageErrorHandler::fatalError(const SAXParseException& exception) {
    if (!have_fatality) {
	updateDiagnostic("fatal error", exception);
    }
    have_fatality = true;
}

// ===========================================================================
//
void MessageErrorHandler::resetErrors() {
    have_warning  = false;
    have_error    = false;
    have_fatality = false;
    diagnostic    = NULL;
}

// ===========================================================================
//
void MessageErrorHandler::updateDiagnostic(const char *kind, 
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
    cerr << "XML message parsing error: " << message << ", on line " <<  exception.getLineNumber() << endl;
}

class MessageParser;

// ===========================================================================
// Concrete class for the object which handles document-oriented parse events.
//
class MessageDocumentHandler : public DocumentHandler {
public:
    MessageDocumentHandler(MessageParser &);
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
    typedef bool MessageDocumentHandler::*attribute;
private:
    MessageParser &parser;
};

// ===========================================================================
// Class to hold the state of parsing a configuration file.
//
class MessageParser {
public:
    MessageParser(const char *bufPtr, unsigned long bufLen, string *&diagnostic);
    void parse();
    bool success();

    deque<Message *> messageStack;
    Message *finalRoot;

private:
    const char *bufPtr;
    unsigned long bufLen;
    string *&diagnostic; 

    bool have_warning;
    bool have_error;
    bool have_fatality;

    MessageErrorHandler errorHandler;
    MessageDocumentHandler documentHandler;
};

// ===========================================================================
//
MessageDocumentHandler::MessageDocumentHandler(MessageParser &parser)
  : parser(parser)
{
}

// ===========================================================================
//
void MessageDocumentHandler::characters (
    const   XMLCh* const    chars
    , const unsigned int    length)
{
    if (parser.messageStack.size() > 0) {
	Message *msg = parser.messageStack.back();
	msg->AddText(DOMString(chars, length));
    }
}

// ===========================================================================
//
void MessageDocumentHandler::endDocument () {}

// ===========================================================================
//
void MessageDocumentHandler::ignorableWhitespace (
    const XMLCh* const,
    const unsigned int
) {}

// ===========================================================================
//
void MessageDocumentHandler::processingInstruction (
    const XMLCh* const,
    const XMLCh* const
) { unimplemented("MessageDocumentHandler::processingInstruction"); }

// ===========================================================================
//
void MessageDocumentHandler::resetDocument() {
}

// ===========================================================================
// This function is called by the SAX parser to give us a Locator.  Since
// we do not plan to do any locating, we will just disregard it.
// 
void MessageDocumentHandler::setDocumentLocator(const Locator * const) {}

// ===========================================================================
//
void MessageDocumentHandler::startDocument() {}

// ===========================================================================
//
void MessageDocumentHandler::endElement(const XMLCh* const name) {
    Message *msg = parser.messageStack.back();
    parser.messageStack.pop_back();
    if (parser.messageStack.size() > 0) {
        Message *parent = parser.messageStack.back();
	parent->AddChild(*msg);
	delete msg;
    }
}

// ===========================================================================
//
void MessageDocumentHandler::startElement (
    const   XMLCh* const    name
    ,       AttributeList&  attrs) 
{
    Message *msg = new Message(DOMString(name));
    unsigned int nattrs = attrs.getLength();
    for (unsigned int i = 0; i < nattrs; i += 1) {
	DOMString name = attrs.getName(i);
	DOMString val = attrs.getValue(i);
        msg->AddAttribute(name, val);
    }
    if (parser.messageStack.size() == 0) {
	parser.finalRoot = msg;
    }
    parser.messageStack.push_back(msg);
}

// ===========================================================================
MessageParser::MessageParser(const char *bufPtr, unsigned long bufLen, string *&diagnostic
		       )

    : finalRoot(NULL),
      bufPtr(bufPtr), bufLen(bufLen), diagnostic(diagnostic),
      errorHandler(diagnostic, have_warning, have_error, have_fatality),
      documentHandler(*this)
{
    errorHandler.resetErrors();
}

// ===========================================================================
bool MessageParser::success() { return !(have_error || have_fatality); }

// ===========================================================================
void MessageParser::parse () {
    MiniXML::Parser *parser = MiniXML::getParser();
    if (parser != NULL) {
	parser->setDocumentHandler(&documentHandler);
	parser->setErrorHandler(&errorHandler);
	parser->parse(bufPtr, bufLen);
	delete parser;
    }
}

// ===========================================================================
// Parse a message.
// The message is in a simplified XML data format.
//
Message *Message::Decode(const char *bufPtr, unsigned long bufLen) {
    string *diagnostic = NULL;
    MessageParser theParser(bufPtr, bufLen, diagnostic);
    theParser.parse();
    Message *retVal = theParser.finalRoot;
    if (retVal == NULL && diagnostic != NULL) {
	cerr << diagnostic << endl;
    }
    delete diagnostic;
    return retVal;
}

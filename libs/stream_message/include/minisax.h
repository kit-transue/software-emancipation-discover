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
// Minisax provides the subset of the SAX API
// required by the config file parser.
// See www.megginson.com/SAX/index.html for general SAX info.
// The C++ binding of SAX is taken from IBM's XML for C.

// Besides the API restrictions evident below, Minisax also does not
// throw exceptions on fatal errors.  It will just return, and
// an error handler must be registered if the user is to know
// whether there was a fatal error.

#ifndef MINISAX_H
#define MINISAX_H

#include "minixml.h"

namespace MiniXML {

class Parser;
class DocumentHandler;
class ErrorHandler;

// The SAX API does not define a concrete class for its
// parser, and yet a parser object must exist in order to use SAX.
Parser *getParser();

class  Parser {
public:
    Parser() {}
    virtual ~Parser() {}
    virtual void setDocumentHandler(DocumentHandler* const handler) = 0;
    virtual void setErrorHandler(ErrorHandler* const handler) = 0;
    virtual void parse (const char* const systemId, const bool = false) = 0;
    virtual void parse (const char* const bufPtr, unsigned long bufLen) = 0;
};

class SAXException
{
public:
    SAXException();
    SAXException(const XMLCh* const msg);
    SAXException(const char* const msg);
    SAXException(const SAXException &);
    SAXException &operator=(const SAXException &);
    virtual ~SAXException();
    const XMLCh* getMessage() const;
private:
    const XMLCh *message;
};


class Locator; // not instantiated in Minisax

class SAXParseException : public SAXException
{
public:
    SAXParseException(const XMLCh* const message, const Locator& locator);
    SAXParseException
    (
        const   XMLCh* const    message
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const unsigned int    lineNumber
        , const unsigned int    columnNumber
    );

    SAXParseException(const SAXParseException& toCopy);
    ~SAXParseException();
    SAXParseException& operator=(const SAXParseException& toAssign);

    unsigned int getColumnNumber() const;
    unsigned int getLineNumber() const;
    const XMLCh* getPublicId() const;
    const XMLCh* getSystemId() const;
private:
    const XMLCh *publicId;
    const XMLCh *systemId;
    unsigned int lineNumber;
    unsigned int columnNumber;
};


class ErrorHandler
{
public:
    ErrorHandler() {}
    virtual ~ErrorHandler() {}
    virtual void warning(const SAXParseException& exception) = 0;
    virtual void error(const SAXParseException& exception) = 0;
    virtual void fatalError(const SAXParseException& exception) = 0;
};

class AttributeList
{
public:
    AttributeList() {}
    virtual ~AttributeList() {}
    virtual unsigned int getLength() const = 0;
    virtual const XMLCh* getName(const unsigned int i) const = 0;
    virtual const XMLCh* getType(const unsigned int i) const = 0;
    virtual const XMLCh* getValue(const unsigned int i) const = 0;
    virtual const XMLCh* getType(const XMLCh* const name) const = 0;
    virtual const XMLCh* getValue(const XMLCh* const name) const = 0;
};

class DocumentHandler
{
public:
    DocumentHandler() {}
    virtual ~DocumentHandler() {}
    virtual void characters (
        const   XMLCh* const    chars
        , const unsigned int    length
    ) = 0;
    virtual void endDocument () = 0;
    virtual void endElement(const XMLCh* const name) = 0;
    virtual void ignorableWhitespace (
        const   XMLCh* const    chars
        , const unsigned int    length
    ) = 0; // callback unimplemented
    virtual void processingInstruction (
        const   XMLCh* const    target
        , const XMLCh* const    data
    ) = 0; // callback unimplemented
    virtual void resetDocument() = 0; // callback unimplemented
    virtual void setDocumentLocator(const Locator* const locator) = 0; // callback unimplemented
    virtual void startDocument() = 0;
    virtual void startElement (
        const   XMLCh* const    name
        ,       AttributeList&  attrs
    ) = 0;
};

}

#endif // MINISAX_H

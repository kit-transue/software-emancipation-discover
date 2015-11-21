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

#include <stdio.h>
#include <list>

#include "minisax.h"
#include "Jpr.h"
#include "JprLex.h"

using namespace std;
using namespace MiniXML;

// ===========================================================================
// Class for the object which handles parse errors.
//
class LibraryParserErrorHandler : public ErrorHandler {
public:
    LibraryParserErrorHandler();
    void warning(const SAXParseException& exception);
    void error(const SAXParseException& exception);
    void fatalError(const SAXParseException& exception);
};

// ===========================================================================
// Class for the object which handles document-oriented parse events.
//
typedef list<string>	Paths;

class LibraryParserDocumentHandler : public DocumentHandler {
public:
	static const char* libraryTag;
	static const char* fullNameTag;
	static const char* classTag;
	static const char* pathTag;
public:
    LibraryParserDocumentHandler();
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
	const char*	getLibraryName() {return libraryName.c_str();};
	Paths	getPaths() {return libraryPaths;};
private:
	bool	inLibrarySection;
	bool	inNameSection;
	bool	inClassSection;
	bool	inPathSection;

	string	currentElement;
	string	libraryName;
	Paths	libraryPaths;
};

// ===========================================================================
// Class to hold the state of parsing a library configuration file.
//
class LibraryParser {
public:
    LibraryParser(const char *library_file);
    void parse(JBuilderProject *jbuilder_proj);
    bool success();

private:
    const char *library_file;

    LibraryParserErrorHandler errorHandler;
    LibraryParserDocumentHandler documentHandler;
};

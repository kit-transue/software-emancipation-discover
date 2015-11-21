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
#include <stdio.h>

#include <dom/DOMString.hpp>
#include <sax/SAXParseException.hpp>

#include "CmXmlErrorHandler.h"


void CmXmlErrorHandler::warning(const SAXParseException&) {
    //
    // Ignore all warnings.
    //
} //warning

void CmXmlErrorHandler::error(const SAXParseException& toCatch) {

	printMessage("Error", toCatch);

} //error

void CmXmlErrorHandler::fatalError(const SAXParseException& toCatch) {

	printMessage("Fatal error", toCatch);

} //fatalError

void CmXmlErrorHandler::resetErrors() {

    // No-op in this case

} //resetErrors

void CmXmlErrorHandler::printMessage(const char *prefix, const SAXParseException& toCatch) {

	printf("%s at file \"", prefix);

	DOMString *file_str = new DOMString(toCatch.getSystemId());
	file_str->print();
	delete file_str;

	printf("\", line %i, column %i\n", toCatch.getLineNumber(), toCatch.getColumnNumber());
	printf("   Message: ");
	DOMString *msg_str = new DOMString(toCatch.getMessage());
	msg_str->println();
	delete msg_str;

} //printMessage

//END OF FILE

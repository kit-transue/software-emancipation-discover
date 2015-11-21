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
#include "LibraryParser.h"

const char* LibraryParserDocumentHandler::libraryTag = "library";
const char* LibraryParserDocumentHandler::fullNameTag = "fullname";
const char* LibraryParserDocumentHandler::classTag = "class";
const char* LibraryParserDocumentHandler::pathTag = "path";

// ===========================================================================
//
LibraryParserErrorHandler::LibraryParserErrorHandler(){
}

// ===========================================================================
//
void LibraryParserErrorHandler::warning(const SAXParseException& exception) {
}

// ===========================================================================
//
void LibraryParserErrorHandler::error(const SAXParseException& exception) {
}

// ===========================================================================
//
void LibraryParserErrorHandler::fatalError(const SAXParseException& exception) {
}


// ===========================================================================
//
LibraryParserDocumentHandler::LibraryParserDocumentHandler() {
	inLibrarySection = false;
	inNameSection = false;
	inClassSection = false;
	inPathSection = false;
}

// ===========================================================================
//
void LibraryParserDocumentHandler::characters (
		const   XMLCh* const    chars, 
		const unsigned int    length) {
	if(inNameSection || inPathSection) {
		string text = XMLsToString(chars);
		bool is_all_whitespace = true;
		int len = text.length();
		int first_nonwhite = len;
		int last_nonwhite = 0;
		for (int i = 0; i < len; i += 1) {
			char ch = text.at(i);
			if (!(isspace(ch) || ch == '\n' || ch == '\r')) {
				is_all_whitespace = false;
				last_nonwhite = i + 1;
				if (i < first_nonwhite) {
					first_nonwhite = i;
				}
			}
		}
		if (!is_all_whitespace)  {
			string trimmed = text.substr(first_nonwhite, last_nonwhite - first_nonwhite);
			if (currentElement.length() > 0) {
				currentElement.append(string(" "));
			}
			currentElement.append(trimmed);
		}
	}
}

// ===========================================================================
//
void LibraryParserDocumentHandler::endDocument () {
}

// ===========================================================================
//
void LibraryParserDocumentHandler::ignorableWhitespace (
    const XMLCh* const,
    const unsigned int ) {
}

// ===========================================================================
//
void LibraryParserDocumentHandler::processingInstruction (
    const XMLCh* const,
    const XMLCh* const) { 
}

// ===========================================================================
//
void LibraryParserDocumentHandler::resetDocument() {
	inLibrarySection = false;
	inNameSection = false;
	inClassSection = false;
	inPathSection = false;
}

// ===========================================================================
// This function is called by the SAX parser to give us a Locator.  Since
// we do not plan to do any locating, we will just disregard it.
// 
void LibraryParserDocumentHandler::setDocumentLocator(const Locator * const) {
}

// ===========================================================================
//
void LibraryParserDocumentHandler::startDocument() {
	inLibrarySection = false;
	inNameSection = false;
	inClassSection = false;
	inPathSection = false;
}

// ===========================================================================
//
void LibraryParserDocumentHandler::endElement(const XMLCh* const name) {
	char* elementName = XMLsToChars(name);
	if(!strcmp(libraryTag,elementName)) {
		inLibrarySection = false;
		return;
	}
	if(!strcmp(fullNameTag,elementName)) {
		libraryName = currentElement;
		currentElement.erase();
		inNameSection = false;
		return;
	}
	if(!strcmp(classTag,elementName)) {
		inClassSection = false;
		return;
	}
	if(!strcmp(pathTag,elementName)) {
		inPathSection = false;
		libraryPaths.push_back(currentElement);
		currentElement.erase();
		return;
	}
}

// ===========================================================================
//
void LibraryParserDocumentHandler::startElement (
				const   XMLCh* const    name,       
				AttributeList&  attrs) {
	char* elementName = XMLsToChars(name);
	if(!inLibrarySection && !strcmp(libraryTag,elementName)) {
		inLibrarySection = true;
		return;
	}
	if(!inNameSection && !strcmp(fullNameTag,elementName)) {
		inNameSection = true;
		return;
	}
	if(inLibrarySection && !inNameSection && 
		!inClassSection && !strcmp(classTag,elementName)) {
		inClassSection = true;
		return;
	}
	if(inLibrarySection && !inNameSection && 
		inClassSection && !inPathSection && !strcmp(pathTag,elementName)) {
		inPathSection = true;
		return;
	}
}

// ===========================================================================
LibraryParser::LibraryParser(const char *library_file) : library_file(library_file),
												documentHandler(),
												errorHandler() {
}

// ===========================================================================
bool LibraryParser::success() { 
	return true; 
}

// ===========================================================================
void LibraryParser::parse(JBuilderProject *jbuilder_proj) {
    Parser *p = getParser();
    if (p != NULL ) {
	Parser &parser = *p;
	parser.setDocumentHandler(&documentHandler);
	parser.setErrorHandler(&errorHandler);
	parser.parse(library_file);
	if(success()) {
	    const char* libraryName = documentHandler.getLibraryName();
	    Paths   libraryPaths = documentHandler.getPaths();
	    
	    static const char* classPathName= "library.classpath.";
	    int	    name_len = strlen(classPathName)+strlen(libraryName)+1;
	    char*   var_name;

	    int	    value_len = 0;
	    char*   var_value = NULL;
	    
	    var_name = new char[name_len];
	    strcpy(var_name,classPathName);
	    strcat(var_name,libraryName);
	    JBuilderProjectVariable* variable = jbuilder_proj->GetVariable( var_name );
	    if( variable == NULL ) {
		variable = new JBuilderProjectVariable;
		variable->SetName( var_name );

		jbuilder_proj->AddVariable( variable );
	    }
	    
	    Paths::iterator pathsIterator;
	    for(pathsIterator=libraryPaths.begin();
		pathsIterator!=libraryPaths.end();
		pathsIterator++) {
		int len = (*pathsIterator).length();
		if(len > 0) {
		    value_len += len;
		    value_len++; // we need to increment value length to have enough
				// space to add paths separator ';'
		}
	    }
	    if(value_len > 0) {
		value_len++;  // to store a trailing end of string char
		var_value = new char[value_len];
		var_value[0]=0;
		for(pathsIterator=libraryPaths.begin();
		    pathsIterator!=libraryPaths.end();
		    pathsIterator++) {
		    int len = (*pathsIterator).length();
		    if(len>0) {
			strcat(var_value,(*pathsIterator).c_str());
			strcat(var_value,";");
		    }
		}
		char* lastChar = var_value + strlen(var_value)-1;
		if(*lastChar==';') *lastChar = 0;
	    }
	    JBuilderProjectVariableValue* variable_value = variable->GetValue( 0 );
	    if( variable_value == NULL )
	    {
		variable_value = new JBuilderProjectVariableValue;
		variable->SetValue( 0, variable_value );

		long    tokenStrSize=0;
		char*   tokenStr = NULL;
		char*   valuePtr = var_value;
		bool    prevEolSettings = _jpr_set_treat_as_string_to_eol( true );
		_jpr_get_token(valuePtr,&valuePtr,&tokenStr,&tokenStrSize);
		if(tokenStr)
		    variable_value->SetValue( tokenStr );
		_jpr_set_treat_as_string_to_eol( prevEolSettings );
		delete tokenStr;
	    }
	    else {
		// The classpath has already been set for this library, so don't
		// modify it.  For example, the same library may be both defined in
		// the JBuilder installation, and overridden in the user's profile.
		// The user's profile is processed first in JprParseMain.
	    }
	    delete var_name;
	    delete var_value;
	}
    }
}

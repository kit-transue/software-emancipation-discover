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
#ifndef CmXml_HEADER_GUARD_
#define CmXml_HEADER_GUARD_

#include <dom/DOM_Document.hpp>
#include <dom/DOM_Node.hpp>
#include <parsers/DOMParser.hpp>

#include "CmXmlCollection.h"
#include "CmXmlErrorHandler.h"
#include "CmXmlSystem.h"


class CmXml {

public:
    // -----------------------------------------------------------------------
    //  Constructor and Destructor
    // -----------------------------------------------------------------------
    CmXml(const char *fileName);
    virtual ~CmXml();

    // -----------------------------------------------------------------------
    //  Different queries
    // -----------------------------------------------------------------------
	void	checkConsistency();
	char*	getCmSystems();
	char*	getAttributes(char *cm_name);
	char*	getCommands(char *cm_name);
    bool    isAttribute(char *cm_name, char *attr_name);
	char*	getAttributeValue(char *cm_name, char *attr_name);
	bool	isCmSystem(char *cm_name);
    bool    isCommand(char *cm_name, char *cmd_name);
    bool    isReturnable(char *cm_name, char *cmd_name);
    char*   translateCommand(char *cm_name, char **args, int *files_start = NULL);
    char*   translateResult(char *cm_name, char *attr_name, char *result);

private:
	void		initConstants();
	DOM_Node	findCmRoot(DOM_Document doc);
    char*       getArgValue(char *arg_name, char **args, int *arg_ndx);
    char*       getAttrSpec(CmXmlSystem *cmXmlSys, DOM_Node parentNode);
	char*		getExecSpec(DOM_Node parentNode);
	char*		getListSpec(DOM_Node parentNode);
    char*       getNodeText(DOM_Node node);
    char*       translateList(char *src);
    int         compareString(const DOMString &domStr, const char* str);
    void        checkAutoCommands(char *cm_name);

// Data
private:
	char                *cmXmlFile;
    DOMParser           *cmXmlParser;
    CmXmlErrorHandler   *cmXmlErrorHandler;
	DOM_Document        cmDoc;
	DOM_Node            cmRoot;
	CmXmlCollection     cmSystems;

// Keywords of 'cm.xml' as usual readable strings
public:
	static const char *str_cm;
	static const char *str_cm_attr;
	static const char *str_cm_cmd;
	static const char *str_cm_exec;
	static const char *str_cm_li;
	static const char *str_cm_root;
	static const char *str_display;
	static const char *str_key;
	static const char *str_list;
	static const char *str_name;
	static const char *str_type;
	static const char *str_value;

// Keywords of 'cm.xml' as DOMStrings
private:
	DOMString dom_cm;
	DOMString dom_cm_attr;
	DOMString dom_cm_cmd;
	DOMString dom_cm_exec;
	DOMString dom_cm_li;
	DOMString dom_cm_root;
	DOMString dom_display;
	DOMString dom_key;
	DOMString dom_list;
	DOMString dom_name;
	DOMString dom_type;
	DOMString dom_value;

}; //CmXml


#endif //CmXml_HEADER_GUARD_

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <winbase.h>
#endif

#include <dom/DOM.hpp>
#include <parsers/DOMParser.hpp>
#include <util/PlatformUtils.hpp>
#include <framework/MemBufInputSource.hpp>

#include "debug.h"

#include "CmXml.h"
#include "CmXmlCollection.h"
#include "CmXmlErrorHandler.h"
#include "CmXmlException.h"
#include "CmXmlNode.h"
#include "CmXmlStringTokenizer.h"
#include "CmXmlSystem.h"


extern int		do_debug;
extern FILE*	log_fd;
extern char		dbgFileName[1024];


void _DBG_PROCESSING_NODE(const char *prefix, const DOM_Node &dom_node) {

  if(do_debug) {

    DOMString nodeName = dom_node.getNodeName();
    int nodeType = dom_node.getNodeType();
    char *s = nodeName.transcode();
    const char *ts = NULL;

    switch(nodeType) {
        case 1:  ts = "element";                break;
        case 2:  ts = "attribute";              break;
        case 3:  ts = "text";                   break;
        case 4:  ts = "cdata";                  break;
        case 5:  ts = "entity reference";       break;
        case 6:  ts = "entity";                 break;
        case 7:  ts = "processing instruction"; break;
        case 8:  ts = "comment";                break;
        case 9:  ts = "document";               break;
        case 10: ts = "document type";          break;
        case 11: ts = "document fragment";      break;
        case 12: ts = "notation";               break;
        case 13: ts = "xml decl";               break;
        default: ts = "???";                    break;
    }

    fprintf(log_fd, "%sProcessing tag <%s>, Len=%i, Type='%s'(%i) ...\n",
            prefix, s, strlen(s), ts, nodeType);
    delete [] s;
    fflush(log_fd);
  }

} //_DBG_PROCESSING_NODE


void _DBG_PROCESSING_ATTR(const char *prefix, const DOM_Node &dom_attr) {

  if(do_debug) { 
    DOMString attrName = dom_attr.getNodeName();
    char *s = attrName.transcode();
    fprintf(log_fd, "%sProcessing attribute '%s'...\n", prefix, s);
    delete [] s;
    fflush(log_fd);
  }

} //_DBG_PROCESSING_ATTR


const char *CmXml::str_cm         = "cm";
const char *CmXml::str_cm_attr    = "cm_attr";
const char *CmXml::str_cm_cmd     = "cm_cmd";
const char *CmXml::str_cm_exec    = "cm_exec";
const char *CmXml::str_cm_li      = "cm_li";
const char *CmXml::str_cm_root    = "cm_root";
const char *CmXml::str_display    = "display";
const char *CmXml::str_key        = "key";
const char *CmXml::str_list       = "list";
const char *CmXml::str_name       = "name";
const char *CmXml::str_type       = "type";
const char *CmXml::str_value      = "value";


CmXml::CmXml(const char *fileName) {

	_DBG(fprintf(log_fd, "\nINITIALIZING...\n"));

	// Let's find source XML file
	_DBG(fprintf(log_fd, "\nSearching for file '%s'...\n", fileName));
	FILE *f = fopen(fileName, "r");
	if(f == NULL) {

		char message[4096];
		sprintf(message, "Can't open file '%s'.\n", fileName);
		throw CmXmlException(message);
	}
	fclose(f);
	this->cmXmlFile = strdup(fileName);

	// Initialize the XML4C2 system
	_DBG(fprintf(log_fd, "\nInitializing Xerces-c...\n"));
    try {
        XMLPlatformUtils::Initialize();
    }
    catch(const XMLException& toCatch) {
		char message[4096];
		strcpy(message, "Error during Xerces-c initialization.\n");
        strcat(message, "  Exception message:\n");
        DOMString domStr(toCatch.getMessage());
		char *s = domStr.transcode();
		strcat(message, s);
		delete [] s;
		s = NULL;
		throw CmXmlException(message);
    }

	// Preparing XML parser
	_DBG(fprintf(log_fd, "Preparing XML parser...\n"));
    cmXmlParser = new DOMParser();
    cmXmlParser->setValidationScheme(DOMParser::Val_Auto);
    cmXmlParser->setDoNamespaces(false);
    cmXmlErrorHandler = new CmXmlErrorHandler();
    cmXmlParser->setErrorHandler(cmXmlErrorHandler);
    cmXmlParser->setExpandEntityReferences(false);
    cmXmlParser->setToCreateXMLDeclTypeNode(true);

    //  Parse the XML file, catching any XML exceptions that 
	//  might propogate out of it.
	_DBG(fprintf(log_fd, "Parsing \"%s\"...\n", cmXmlFile));
    try {
        cmXmlParser->parse(cmXmlFile);
    }
    catch(const XMLException& toCatch) {
		char message[4096];
        strcpy(message, "An error occured during parsing\n");
		strcat(message, "   Message: ");
		DOMString domStr(toCatch.getMessage());
		char *s = domStr.transcode();
		strcat(message, s);
		delete [] s;
		s = NULL;
		throw CmXmlException(message);
    }

	// Init constants
	initConstants();

	// Look at loaded DOM
	cmDoc = cmXmlParser->getDocument();
	cmRoot = findCmRoot(cmDoc);

} //constructor


CmXml::~CmXml() {

    free(this->cmXmlFile);
    this->cmXmlFile = NULL;
    delete this->cmXmlParser;
    this->cmXmlParser = NULL;
    delete this->cmXmlErrorHandler;
    this->cmXmlErrorHandler = NULL;

	// Terminating the XML4C2 system
	_DBG(fprintf(log_fd, "Terminating Xerces-c...\n"));
    XMLPlatformUtils::Terminate();

} //destructor


void CmXml::checkConsistency() {

	_DBG(fprintf(log_fd, "\nCHECKING CONSISTENCY...\n"));

	char *systems = NULL;
	char *attributes = NULL;
	char *commands = NULL;

	// Try to perform different queries
	try {

		// Try to get CM systems
		systems = getCmSystems();

		// Try to get supported attributes and commands for every CM system
        CmXmlStringTokenizer tokenizer(systems, "\t\n", CmXmlStringTokenizer::ONE_SIDE_TOKEN);
        char *sys = NULL;
        while((sys = tokenizer.nextToken()) != NULL) {

			// Try to get attributes and commands for the current CM system.
            // Attributes in the first because we need them when we will process commands!
			attributes = getAttributes(sys);
			commands = getCommands(sys);

            // Check auto-commands: 'autoco', 'autoci'.
            checkAutoCommands(sys);

			// Free resources
			delete [] attributes;
			attributes = NULL;
			delete [] commands;
			commands = NULL;

        } //while token
	}

	// If exception occures -> something wrong in 'cm.xml', so we can't proceed anymore.
	catch(const CmXmlException &ex) {

		// Free resources
		delete [] systems;
		systems = NULL;
		delete [] attributes;
		attributes = NULL;
		delete [] commands;
		commands = NULL;

		// Propagate exception to upper level.
		// We have to do it in so way because we must free resources here.
		const char *msg = ex.getMessage();
		throw CmXmlException(msg);
	}

	// Free resources
	delete [] systems;
	systems = NULL;
	delete [] attributes;
	attributes = NULL;
	delete [] commands;
	commands = NULL;

	_DBG(fprintf(log_fd, "\nDRIVER IS READY TO WORK!\n\n"));

} //checkConsistency


char* CmXml::getCmSystems() {

	_DBG(fprintf(log_fd, "\nGetting names of all supported CM systems...\n"));

	// If cache for CM systems' names doesn't exist -> construct it.
	if(this->cmSystems.size() == 0) {

		_DBG(fprintf(log_fd, "  Filling a cache...\n"));

		// Searching for nodes which correspond to CM systems.
		for(DOM_Node child = cmRoot.getFirstChild(); child != 0;
								child = child.getNextSibling()) {

			_DBG_PROCESSING_NODE("    ", child);

			if(child.getNodeType() == DOM_Node::ELEMENT_NODE) {

				DOMString childName = child.getNodeName();
				if(compareString(childName, str_cm) == 0) {

					// Get all attributes of current node
					DOM_NamedNodeMap attributes = child.getAttributes();
					int attrCount = attributes.getLength();

					// Searching for attributes which will say us about name of CM.
					int attrNdx = 0;
					for(attrNdx = 0; attrNdx < attrCount; attrNdx++)
					{
						DOM_Node attr = attributes.item(attrNdx);
						_DBG_PROCESSING_ATTR("      ", attr);

						DOMString attrName = attr.getNodeName();
						if(compareString(attrName, str_name) == 0) {

							// We finally found name of one of supported CM systems
							DOMString attrValue = attr.getNodeValue();
							char *s = attrValue.transcode();
							_DBG(fprintf(log_fd, "      Found name of supported CM system: '%s'\n", s));

							// Add found CM name to the collection
							CmXmlSystem *sys = new CmXmlSystem(child, s);
							this->cmSystems.add(sys);

							delete [] s;
                            s = NULL;

							// We don't need to find any more attributes in this node
							break;

						} //if attrName
					} //for i

					// May be we didn't find necessary attribute
					if(attrNdx >= attrCount) {
						char message[1024];
						sprintf(message, "Tag <%s> doesn't have mandatory attribute '%s'!\n", str_cm, str_name);
						throw CmXmlException(message);
					}

				} //if childName
			} //if childType
		} //for child
	} //if

	// Buffer for names of all supported CM systems. They will be divided by character '\n'.
	// It is not a smart solution to hard code buffer's size!!!
	//    So it is necessary to make it dynamic (alloc, realloc, ...)
	char *systems = new char[4096];
	systems[0] = 0;

	// Here we must have necessary information in the cache
	int sysArrSize = this->cmSystems.size();
	if(sysArrSize > 0) {

		_DBG(fprintf(log_fd, "  Loading necessary names from the cache...\n"));

		for(int i = 0; i < sysArrSize; i++) {
			CmXmlNode *sysNode = this->cmSystems.get(i);

			_DBG(fprintf(log_fd, "    CM system: '%s'\n", sysNode->getKey()));

			// If it is not first found CM system -> 
			//    so divide its name from others by character '\n'.
			if(systems[0] != 0) {
				strcat(systems, "\n");
			}
			strcat(systems, sysNode->getKey());
		} //for i
	} //if sysArrSize

	// Return pointer to buffer, which contains names of all supported CM systems.
	// Caller method is responsible for deleting memory.
	return systems;

} //getCmSystems


bool CmXml::isCmSystem(char *cm_name) {

	// Try to find CM system with the given name
	CmXmlNode *sysNode = this->cmSystems.get(cm_name);

	return (sysNode != NULL) ? true : false;

} //isCmSystem


char* CmXml::getAttributes(char *cm_name) {

    if(cm_name == NULL) {
		char message[1024];
		sprintf(message, "CM system is not specified!\n");
		throw CmXmlException(message);
    }

	_DBG(fprintf(log_fd, "\nGetting all supported attributes for CM system '%s'...\n", cm_name));

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return NULL;
	}

	// If cache of attributes of the givem CM system doesn't exist -> construct it.
	if(cmXmlSys->sizeAttr() == 0) {

		_DBG(fprintf(log_fd, "  Filling a cache...\n"));

		// This is CM system's root in the DOM
		DOM_Node sysRoot = cmXmlSys->getNode();
		_DBG_PROCESSING_NODE("    ", sysRoot);

		// Process all children and find all CM attributes
		for(DOM_Node attr = sysRoot.getFirstChild(); attr != 0;	attr = attr.getNextSibling()) {

			_DBG_PROCESSING_NODE("    ", attr);

			if(attr.getNodeType() == DOM_Node::ELEMENT_NODE) {

				DOMString attrName = attr.getNodeName();
				if(compareString(attrName, str_cm_attr) == 0) {
					// We found node in DOM, which describes one of the CM attributes

					// Here will be description of the current CM attribute
					// It is not a smart solution to hard code buffer's size!!!
					//    So it is necessary to make it dynamic (alloc, realloc, ...)
					char *attrDescr = new char[16384];
					attrDescr[0] = 0;

					// Here will be "shortcut" to the current attribute
					CmXmlNode *attrNode = NULL;

					// Get attribute's options
					DOM_NamedNodeMap options = attr.getAttributes();
					int optCount = options.getLength();

					// Looking at all options, and making from them
                    //    description of the current attribute.
					// Special case for options `key` and `type="list"`.
					int optNdx = 0;
					for(optNdx = 0; optNdx < optCount; optNdx++) {

						DOM_Node opt = options.item(optNdx);
						_DBG_PROCESSING_ATTR("      ", opt);

						// Get option's name and value
						DOMString optName = opt.getNodeName();
						DOMString optValue = opt.getNodeValue();

						// Translate option's name and value to "readable" form.
						char *n = optName.transcode();
						char *v = optValue.transcode();

						_DBG(fprintf(log_fd, "        %s=\"%s\"\n", n, v));

						// If it is not first found attribute's option -> 
						//    so divide this option from others by character '\t'.
						if(attrDescr[0] != 0) {
							strcat(attrDescr, "\t");
						}

						if(compareString(optName, str_type) == 0 && compareString(optValue, str_list) == 0) {

							// Special case for type='list'.
							// It is necessary to retrieve list items which are stored in children tags.

							char *listSpec = getListSpec(attr);

							strcat(attrDescr, n);
							strcat(attrDescr, "=\"");
						    strcat(attrDescr, listSpec);
							strcat(attrDescr, "\"");

							delete [] listSpec;
                            listSpec = NULL;

						} else {

							// Add description of the current option to the buffer
							strcat(attrDescr, n);
							strcat(attrDescr, "=\"");
							strcat(attrDescr, v);
							strcat(attrDescr, "\"");

							// For future fast access, we will store link 
							//   to the current attribute's node in the "cache".
							if(compareString(optName, str_key) == 0) {
								attrNode = new CmXmlNode(attr, v);
							}
						} //if optName

						// We must free memory that was allocated in the Xerces library
						delete [] n;
                        n = NULL;
						delete [] v;
                        v = NULL;

					} //for optNdx

					if(attrNode == NULL) {
						// We didn't obtain mandatory option 'key'.
						// So we can't process further.

						char message[1024];
						sprintf(message, "Tag <%s> doesn't have mandatory attribute '%s'!\n",
							    str_cm_attr, str_key);
						throw CmXmlException(message);
					}

					// Obtain information concerning executing external command
					char *execSpec = getExecSpec(attr);
                    if(execSpec != NULL && execSpec[0] != 0) {
					    strcat(attrDescr, "\t");
					    strcat(attrDescr, "ask=\"\"");
                    }
				    delete [] execSpec;
                    execSpec = NULL;

					// Store this CM attribute in the cache
					attrNode->setDescription(attrDescr);
					cmXmlSys->addAttr(attrNode);

					// Free memory, which was temporary allocated.
					delete [] attrDescr;
                    attrDescr = NULL;

				} //if attrName
			} //if attr
		} //for child
	} //if sizeAttr

	// Buffer for all supported attributes of the given CM system.
	//    Different attributes will be divided by character '\n'.
	//    Different attribute's options will be divided by character '\t'.
	// It is not a smart solution to hard code buffer's size!!!
	//    So it is necessary to make it dynamic (alloc, realloc, ...)
	char *attributesDescription = new char[65536];
	attributesDescription[0] = 0;

	// Here we must have necessary information in the cache.
    // So we only need to retrieve it from cache.
	int attrSize = cmXmlSys->sizeAttr();
	if(attrSize > 0) {

		_DBG(fprintf(log_fd, "  Loading necessary attributes' description from the cache...\n"));

		for(int attrNdx = 0; attrNdx < attrSize; attrNdx++) {

			CmXmlNode *attrNode = cmXmlSys->getAttr(attrNdx);
			char *attrDescr = attrNode->getDescription();

			if(attrDescr != NULL) {

				_DBG(fprintf(log_fd, "    %s\n", attrDescr));

				// If it is not first found attribute -> 
				//    so divide its description from others by character '\n'.
				if(attributesDescription[0] != 0) {
					strcat(attributesDescription, "\n");
				}
				strcat(attributesDescription, attrDescr);
			} //if attrDesr
		} //for attrNdx
	} //if attrSize

	// Return pointer to buffer, which contains description of all supported attributes.
	// Caller method is responsible for deleting memory.
	return attributesDescription;

} //getAttributes


char* CmXml::getCommands(char *cm_name) {

    if(cm_name == NULL) {
		char message[1024];
		sprintf(message, "CM system is not specified!\n");
		throw CmXmlException(message);
    }

	_DBG(fprintf(log_fd, "\nGetting all supported commands for CM system '%s'...\n", cm_name));

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return NULL;
	}

	// If the cache of commands of the givem CM system doesn't exist -> construct it.
	if(cmXmlSys->sizeCmd() == 0) {

		_DBG(fprintf(log_fd, "  Filling a cache...\n"));

		// This is CM system's root in the DOM
		DOM_Node sysRoot = cmXmlSys->getNode();
		_DBG_PROCESSING_NODE("    ", sysRoot);

		// Process all children and find all CM commands
		for(DOM_Node cmd = sysRoot.getFirstChild(); cmd != 0; cmd = cmd.getNextSibling()) {

			_DBG_PROCESSING_NODE("    ", cmd);

			if(cmd.getNodeType() == DOM_Node::ELEMENT_NODE) {

				DOMString cmdName = cmd.getNodeName();
				if(compareString(cmdName, str_cm_cmd) == 0) {
					// We found node in DOM, which describes one of the CM commands

					// Here will be description of the current CM command
					// It is not a smart solution to hard code buffer's size!!!
					//    So it is necessary to make it dynamic (alloc, realloc, ...)
					char *cmdDescr = new char[16384];
					cmdDescr[0] = 0;

					// Here will be "shortcut" to the current command
					CmXmlNode *cmdNode = NULL;

					// Get command's options
					DOM_NamedNodeMap options = cmd.getAttributes();
					int optCount = options.getLength();

					// Looking at all options, and making from them
                    //    description of the current command.
					int optNdx = 0;
					for(optNdx = 0; optNdx < optCount; optNdx++) {

						DOM_Node opt = options.item(optNdx);
						_DBG_PROCESSING_ATTR("      ", opt);

						// Get option's name and value
						DOMString optName = opt.getNodeName();
						DOMString optValue = opt.getNodeValue();

						// Translate option's name and value to "readable" form.
						char *n = optName.transcode();
						char *v = optValue.transcode();

						_DBG(fprintf(log_fd, "        %s=\"%s\"\n", n, v));

						// If it is not first found command's option -> 
						//    so divide this option from others by character '\t'.
						if(cmdDescr[0] != 0) {
							strcat(cmdDescr, "\t");
						}

						// Add description of the current option to the buffer
						strcat(cmdDescr, n);
						strcat(cmdDescr, "=\"");
						strcat(cmdDescr, v);
						strcat(cmdDescr, "\"");

						// For future fast access, we will store link 
						//   to the current command's node in the "cache".
						if(compareString(optName, str_key) == 0) {
							cmdNode = new CmXmlNode(cmd, v);
						}

						// We must free memory that was allocated in the Xerces library
						delete [] n;
                        n = NULL;
						delete [] v;
                        v = NULL;

                    } //for optNdx

					if(cmdNode == NULL) {
						// We didn't obtain mandatory option 'key'.
						// So we can't process further.

						char message[1024];
						sprintf(message, "Tag <%s> doesn't have mandatory attribute '%s'!\n",
							    str_cm_cmd, str_key);
						throw CmXmlException(message);
					}

					// Obtain information concerning command's attributes
					char *attrSpec = getAttrSpec(cmXmlSys, cmd);
                    if(attrSpec != NULL && attrSpec[0] != 0) {
					    strcat(cmdDescr, "\t");
					    strcat(cmdDescr, attrSpec);
                    }
				    delete [] attrSpec;
                    attrSpec = NULL;

					// Store this CM command in the cache
					cmdNode->setDescription(cmdDescr);
					cmXmlSys->addCmd(cmdNode);

					// Free memory, which was temporary allocated.
					delete [] cmdDescr;
                    cmdDescr = NULL;

                } //if cmdName
            } //if Type
        } //for cmd
    } //if sizeCmd

	// Buffer for all supported commands of the given CM system.
	//    Different commands will be divided by character '\n'.
	//    Different commands's options will be divided by character '\t'.
	// It is not a smart solution to hard code buffer's size!!!
	//    So it is necessary to make it dynamic (alloc, realloc, ...)
	char *commandsDescription = new char[65536];
	commandsDescription[0] = 0;

	// Here we must have necessary information in the cache.
    // So we only need to retrieve it from cache.
	int cmdSize = cmXmlSys->sizeCmd();
	if(cmdSize > 0) {

		_DBG(fprintf(log_fd, "  Loading necessary commands' description from the cache...\n"));

		for(int cmdNdx = 0; cmdNdx < cmdSize; cmdNdx++) {

			CmXmlNode *cmdNode = cmXmlSys->getCmd(cmdNdx);
			char *cmdDescr = cmdNode->getDescription();

			if(cmdDescr != NULL) {

				_DBG(fprintf(log_fd, "    %s\n", cmdDescr));

				// If it is not first found command -> 
				//    so divide its description from others by character '\n'.
				if(commandsDescription[0] != 0) {
					strcat(commandsDescription, "\n");
				}
				strcat(commandsDescription, cmdDescr);
			} //if cmdDesr
		} //for cmdNdx
	} //if cmdSize

	// Return pointer to buffer, which contains description of all supported commands.
	// Caller method is responsible for deleting memory.
	return commandsDescription;

} //getCommands


bool CmXml::isAttribute(char *cm_name, char *attr_name) {

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return false;
	}

    // Try to find the given attribute in the cache
    if(cmXmlSys->getAttr(attr_name) == NULL) {
        return false;
    }

    return true;

} //isAttribute


char* CmXml::getAttributeValue(char *cm_name, char *attr_name) {

    // Check parameters
    if(cm_name == NULL || cm_name[0] == 0 || attr_name == NULL || attr_name[0] == 0) {
        return NULL;
    }

	_DBG(fprintf(log_fd, "\nGetting value of the attribute '%s'...\n", attr_name));

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return NULL;
	}

    // Try to find the given attribute in the cache
    CmXmlNode *attr = cmXmlSys->getAttr(attr_name);
    if(attr == NULL) {
        return NULL;
    }
    DOM_Node attrNode = attr->getNode();
    if(attrNode == 0) {
        return NULL;
    }

	// Get attribute's options
	DOM_NamedNodeMap options = attrNode.getAttributes();
	int optCount = options.getLength();

	// Looking at all options, and find option 'value="..."'
	for(int optNdx = 0; optNdx < optCount; optNdx++) {

		DOM_Node opt = options.item(optNdx);

		// Get option's name
		DOMString optName = opt.getNodeName();
        if(compareString(optName, str_value) == 0) {

            // Get option's value
		    DOMString optValue = opt.getNodeValue();
		    char *v = optValue.transcode();
            return v;
        }
    }

    return NULL;

} //getAttributeValue


bool CmXml::isCommand(char *cm_name, char *cmd_name) {

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return false;
	}

    // Try to find the given command in the cache
    if(cmXmlSys->getCmd(cmd_name) == NULL) {
        return false;
    }

    return true;

} //isCommand


bool CmXml::isReturnable(char *cm_name, char *cmd_name) {

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return false;
	}

    // Try to find the given command in the cache
    CmXmlNode *cmd = cmXmlSys->getCmd(cmd_name);
	if(cmd == NULL) {
        return false;
    }

	// Try to find 'return="' in command's description
	char *cmdDescr = cmd->getDescription();
	if(cmdDescr == NULL) {
        return false;
    }
	if(strstr(cmdDescr, "return=\"") == NULL) {
        return false;
	}

    return true;

} //isReturnable


char* CmXml::translateCommand(char *cm_name, char **args, int *files_start) {

    // Check parameters
    if(cm_name == NULL || cm_name[0] == 0 || args == NULL || args[0] == NULL) {
        return NULL;
    }

    // Command name is passed as first element of <args>
    char *cmdName = args[0];

	_DBG(fprintf(log_fd, "\nTranslating command '%s'...\n", cmdName));

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return NULL;
	}

    // Special command 'get_attr_value'
    if(strcmp(cmdName, "get_attr_value") == 0) {

        // Try to find this attribute in the cache
        char *attrName = args[1];
        CmXmlNode *attrNode = cmXmlSys->getAttr(attrName);
        if(attrNode == NULL) {
            return NULL;
        }

        // Get command line specification
        char *cmdLine = getExecSpec(attrNode->getNode());
        int len = strlen(cmdLine);

        // Calculate possible length
        int expLen = len;
        for(int k = 0; k < len; k++) {
            if(cmdLine[k] == '$') {
                expLen += 1024;
            }
        }

        // Add extra space for expanding env. variables
        if(expLen > len) {
            char *expCmdLine = new char [expLen + 1];
            strcpy(expCmdLine, cmdLine);
            delete [] cmdLine;
            cmdLine = expCmdLine;
            expCmdLine = NULL;
        }

        return cmdLine;
    }

    // Try to find the given command in the cache
    CmXmlNode *cmdNode = cmXmlSys->getCmd(cmdName);
    if(cmdNode == NULL) {
        return NULL;
    }

    // Get exec specification for the given command.
    // It surely should be because we have checked it before.
    char *execSpec = getExecSpec(cmdNode->getNode());

    // Get the beginning of command line, and its length.
    char *startCmdLine = execSpec;
    int lenCmdLine = strlen(startCmdLine);

    // Find number of passed arguments
    int argNum = 0;
    while(args[argNum] != NULL) {
        argNum++;
    }

    // Here we will store flags, which say whether argument has been processed or not.
    bool *argProcessed = new bool [argNum];
    int i = 0;
    for(i = 1; i < argNum; i++) {
        argProcessed[i] = false;
    }
    argProcessed[0] = true; // first "argument" is command name

    // Calculate maximum possible length of the resulting command line.
    // It is upper estimation.
    int possibleLen = lenCmdLine + 1;
    for(i = 1; i < argNum; i++) {
        possibleLen += strlen(args[i]);
        possibleLen += 4; // additional space for possible double quotes
    }

    // Add extra space for expanding env. variables
    for(i = 0; i < lenCmdLine; i++) {
        if(startCmdLine[i] == '$') {
            possibleLen += 1024;
        }
    }

    // This buffer will contain the resulting command line.
    char *resBuf = new char[possibleLen];
    resBuf[0] = 0;

    // Single file (1), multi files (-1) or no files (0)
    int fileMode = 0;

    // Find all arguments in the exec specification and change them to their values.
    CmXmlStringTokenizer tokenizer(startCmdLine, "%", CmXmlStringTokenizer::BOTH_SIDES_TOKEN);
    char *argName = NULL; // here will be name of argument
    int argStart = -1;    // here will be index of first character of argument
    int fixedStart = 0;   // index of first character of fixed part of the command line

    while((argName = tokenizer.nextToken(&argStart)) != NULL) {

        // Copy fixed part of the command line
        int fixedLen = argStart - 1 - fixedStart;
        if(fixedLen > 0) {
            strncat(resBuf, startCmdLine + fixedStart, fixedLen);
        }

        // Special case for %file%, %files%, %dir% and %dirs%.
        if(strcmp(argName, "file") == 0 || strcmp(argName, "dir") == 0) {

            // Place "%s" instead of %file% or %dir%.
            // We will put actual file (directory) name in this place later.
            strcat(resBuf, "\"%s\"");
            fileMode = 1;

        } else if(strcmp(argName, "files") == 0 || strcmp(argName, "dirs") == 0) {

            // Place "%s" instead of %files% or %dirs%.
            // We will put actual file (directory) names in this place later.
            strcat(resBuf, "\"%s\"");
            fileMode = -1;

        } else {

            // Find value of argument and copy this value instead of argument's name
            int argNdx = -1;
            char *argValue = getArgValue(argName, args, &argNdx);

            // If we didn't receive value of this argument -> we cannot proceed further.
            if(argValue == NULL) {

                // Delete memory, which was temporary allocated.
                delete [] execSpec;
                execSpec = NULL;
                delete [] argProcessed;
                argProcessed = NULL;
                delete [] resBuf;
                resBuf = NULL;

                // Throw an exception to notify caller method
			    char message[1024];
                sprintf(message, "Client didn't pass value of argument '%s' for command '%s'!",
                        argName, cmdName);
			    throw CmXmlException(message);
            }

            // Copy value of the current argument
            strcat(resBuf, argValue);

            // Mark argument as processed
            argProcessed[argNdx] = true;

        } //if "file" ("dir")

        // Move to the next fixed part of the original command line
        fixedStart = argStart + strlen(argName) + 1;

    } //while argName

    // May be fixed part of the original command line is left at the end of string.
    // Copy it to the new command line.
    if(startCmdLine[fixedStart] != 0) {
        strcat(resBuf, startCmdLine + fixedStart);
    }

    // Find index of first file (directory) name in <args>
    int filesStart = argNum;
    for(i = argNum - 1; i >= 0; i--) {
        if(argProcessed[i]) {
            filesStart = i + 1;
            break;
        }
    }

    // Check whether all passed arguments were used in exec specification
    for(i = 1; i < filesStart; i++) {

        if(!argProcessed[i]) {
            // Delete memory, which was temporary allocated.
            delete [] execSpec;
            execSpec = NULL;
            delete [] argProcessed;
            argProcessed = NULL;
            delete [] resBuf;
            resBuf = NULL;

            // Throw an exception to notify caller method
			char message[1024];
            sprintf(message, "Client passed unknown statement '%s' for command '%s'!",
                    args[i], cmdName);
			throw CmXmlException(message);
        }
    }

    // Check how many files does this external command support
    if(fileMode == 0) {

        // No files are expected
        if(filesStart != argNum) {
            // Delete memory, which was temporary allocated.
            delete [] execSpec;
            execSpec = NULL;
            delete [] argProcessed;
            argProcessed = NULL;
            delete [] resBuf;
            resBuf = NULL;

            // Throw an exception to notify caller method
			char message[1024];
            sprintf(message, "Client passed file name(s) for command '%s', "  \
                             "which doesn't accept file(s) as argument(s)!", cmdName);
			throw CmXmlException(message);
        }

        filesStart = -1;

    } else {

        // At least one file is expected
        if(filesStart == argNum) {
            // Delete memory, which was temporary allocated.
            delete [] execSpec;
            execSpec = NULL;
            delete [] argProcessed;
            argProcessed = NULL;
            delete [] resBuf;
            resBuf = NULL;

            // Throw an exception to notify caller method
			char message[1024];
            sprintf(message, "Client didn't passed file name(s) for command '%s', "  \
                             "which can't be executed without file(s)!", cmdName);
			throw CmXmlException(message);
        }

        // Delete double quotes, which may bound file names.
        for(i = filesStart; i < argNum; i++) {

            char *fname = args[i];
            int flen = strlen(fname);

            if(fname[0] == '\"' && fname[flen - 1] == '\"') {
                memmove(fname, fname + 1, flen - 2);
                fname[flen - 2] = 0;
                fname[flen - 1] = 0;
            }
        }

        if(fileMode == -1) {

            // This buffer will contain all file (directory) names.
            char *fileBuf = new char[possibleLen];
            fileBuf[0] = 0;

            for(i = filesStart; i < argNum; i++) {
                if(i > filesStart) {
                    strcat(fileBuf, "\" \"");
                }
                strcat(fileBuf, args[i]);
            }

            // We actually have template in the resulting buffer
            char *templateBuf = resBuf;

            // Allocate new memory for result
            resBuf = new char[possibleLen];
            resBuf[0] = 0;

            // Assemble all we have
            sprintf(resBuf, templateBuf, fileBuf);

            // Delete memory, which was temporary allocated.
            delete [] fileBuf;
            fileBuf = NULL;
            delete [] templateBuf;
            templateBuf = NULL;

            // The caller method don't need to do any manipulation with files
            filesStart = -1;
        }

    } //fileMode

    if(files_start != NULL) {
        *files_start = filesStart;
    }

    // Delete memory, which was temporary allocated.
    delete [] execSpec;
    execSpec = NULL;
    delete [] argProcessed;
    argProcessed = NULL;

	_DBG(fprintf(log_fd, "   --> '%s'\n", resBuf));

	// Return pointer to buffer, which contains translated command line.
	// This command line is almost ready to execute (m.b. it is necessary 
	//    to include file (directory) name instead of %s).
	// Caller method is responsible for deleting memory.
    return resBuf;

} //translateCommand


char* CmXml::translateResult(char *cm_name, char *attr_name, char *result) {

    // Check parameters
    if(cm_name == NULL || attr_name == NULL || result == NULL) {
        return NULL;
    }

	_DBG(fprintf(log_fd, "\nTranslating result...\n"));

	// Get descriptor, which holds all information for the given CM system
	CmXmlSystem *cmXmlSys = (CmXmlSystem*)this->cmSystems.get(cm_name);
	if(cmXmlSys == NULL) {
		return NULL;
	}

    // Try to find the given attribute in the cache
    CmXmlNode *attr = cmXmlSys->getAttr(attr_name);
    if(attr == NULL) {
        return NULL;
    }
    DOM_Node attrNode = attr->getNode();
    if(attrNode == 0) {
        return NULL;
    }

	// Get attribute's options
	DOM_NamedNodeMap options = attrNode.getAttributes();
	int optCount = options.getLength();

	// Looking at all options, and find option 'type="..."'
    char *type = NULL;
	for(int optNdx = 0; optNdx < optCount; optNdx++) {

		DOM_Node opt = options.item(optNdx);

		// Get option's name
		DOMString optName = opt.getNodeName();
        if(compareString(optName, str_type) == 0) {

            // Get option's value, i.e. type itself
		    DOMString optValue = opt.getNodeValue();
		    type = optValue.transcode();
            break;
        }
    }

    // Here will be translated result
    char *trans = NULL;

    if(type == NULL || strcmp(type, "list") != 0) {

        // We don't have to do any translation for text, etc.
        int len = strlen(result);
        char *trans = new char [len + 1];
        strcpy(trans, result);

    } else {

        // For list we have to do real translation
        trans = translateList(result);
    }

    return trans;

} //translateResult


void CmXml::initConstants() {

	_DBG(fprintf(log_fd, "Initializing constants for searching in the DOM...\n"));

	dom_cm			= DOMString::transcode(str_cm);
	dom_cm_attr		= DOMString::transcode(str_cm_attr);
	dom_cm_cmd		= DOMString::transcode(str_cm_cmd);
	dom_cm_exec		= DOMString::transcode(str_cm_exec);
	dom_cm_li		= DOMString::transcode(str_cm_li);
	dom_cm_root		= DOMString::transcode(str_cm_root);
	dom_display		= DOMString::transcode(str_display);
	dom_key			= DOMString::transcode(str_key);
	dom_list		= DOMString::transcode(str_list);
	dom_name		= DOMString::transcode(str_name);
	dom_type		= DOMString::transcode(str_type);
	dom_value		= DOMString::transcode(str_value);

} //initConstants


DOM_Node CmXml::findCmRoot(DOM_Document doc) {

	_DBG(fprintf(log_fd, "Searching for <%s> in DOM...\n", str_cm_root));
	_DBG_PROCESSING_NODE("  ", doc);

	// Searching for appropriate node in the DOM
    for(DOM_Node child = doc.getFirstChild(); child != 0; child = child.getNextSibling()) {

		_DBG_PROCESSING_NODE("    ", child);

		if(child.getNodeType() == DOM_Node::ELEMENT_NODE) {
			DOMString childName = child.getNodeName();
			if(compareString(childName, str_cm_root) == 0) {
				return child;
			}
		}
	}

	// We couldn't find necessary element. We can't proceed further without having it.
	char message[1024];
	sprintf(message, "Unable to find <%s>!\n", str_cm_root);
	throw CmXmlException(message);

} //findCmRoot


char* CmXml::getArgValue(char *arg_name, char **args, int *arg_ndx) {

    // Here will be results
    char *argValue = NULL;
    int ndx = -1;

    // Make template for searching
    int templLen = strlen(arg_name) + 2;
    char *templ = new char [templLen + 1];
    strcpy(templ, arg_name);
    strcat(templ, "=\"");

    // Searching for the template
    for(int i = 1; args[i] != NULL; i++) {
        if(strncmp(args[i], templ, templLen) == 0) {
            ndx = i;
            break;
        }
    }

    delete [] templ;
    templ = NULL;

    // If found -> prepare result
    if(ndx != -1) {

        // Skip template
        argValue = args[ndx];
        argValue += templLen;

        // If value contains spaces -> we have to bound it with double quotes
        if(strchr(argValue, ' ') != NULL) {
            argValue--;
        } else {
            // Otherwise remove these double quotes
            int valueLen = strlen(argValue);
            argValue[valueLen - 1] = 0;
        }
    } //if ndx

    if(arg_ndx != NULL) {
        *arg_ndx = ndx;
    }

    return argValue;

} //getArgValue


char* CmXml::getAttrSpec(CmXmlSystem *cmXmlSys, DOM_Node parentNode) {

    // Get specification for executing external program for the given command
	char *execSpec = getExecSpec(parentNode);

    // May be nothing to search for
    if(execSpec == NULL || execSpec[0] == 0) {
        delete [] execSpec;
        execSpec = NULL;
        return NULL;
    }

	// This is buffer for storing attributes specification (i.e. what attributes are passed as arguments).
	// We will return it as a result of this method.
	char *attrSpec = new char[8192];
	attrSpec[0] = 0;

	_DBG(fprintf(log_fd, "      Searching for all attributes, which are used in command line...\n"));

    // Find all attributes, which are mentioned in the specification
    CmXmlStringTokenizer tokenizer(execSpec, "%", CmXmlStringTokenizer::BOTH_SIDES_TOKEN);
    char *attrName = NULL;
    while((attrName = tokenizer.nextToken()) != NULL) {

	    _DBG(fprintf(log_fd, "        attr=\"%s\"\n", attrName));

        // Try to find this attribute in the cache
        if(stricmp(attrName, "file") != 0 && stricmp(attrName, "files") != 0
					&& stricmp(attrName, "dir") != 0 && stricmp(attrName, "dirs") != 0
                    && cmXmlSys->getAttr(attrName) == NULL) {
            // Found attribute that has not been described.
            // There are only exceptions %file%, %files%, %dir% and %dirs%.

			char message[1024];
			sprintf(message,
                    "Attribute '%s' is used in command line specification,\n"    \
                    "but it has not previously described for CM system '%s'!\n",
					attrName, cmXmlSys->getKey());
			throw CmXmlException(message);
        }

		// If it is not first found attribute -> 
		//    divide its description from others by character '\t'.
		if(attrSpec[0] != 0) {
			strcat(attrSpec, "\t");
		}

        // Add to attributes specification
        strcat(attrSpec, "attr=\"");
        strcat(attrSpec, attrName);
        strcat(attrSpec, "\"");

    } //while attrName

    delete [] execSpec;
    execSpec = NULL;

	// Return pointer to buffer, which contains attributes specification.
	// Caller method is responsible for deleting memory.
    return attrSpec;

} //getAttrSpec


char* CmXml::getExecSpec(DOM_Node parentNode) {

	// This is buffer for storing exec specification (i.e. what to execute).
	// We will return it as a result of this method.
	char *execSpec = NULL;

	_DBG(fprintf(log_fd, "      Searching for tag <%s>...\n", str_cm_exec));

    for(DOM_Node childNode = parentNode.getFirstChild(); childNode != 0;
								childNode = childNode.getNextSibling()) {

		_DBG_PROCESSING_NODE("        ", childNode);

		if(childNode.getNodeType() == DOM_Node::ELEMENT_NODE) {

			DOMString childName = childNode.getNodeName();
            if(compareString(childName, str_cm_exec) == 0) {

                // Get command line specification
                execSpec = getNodeText(childNode);

                if(execSpec == NULL || execSpec[0] == 0) {
                    // Incorrect 'cm.xml'
                    delete [] execSpec;
					char message[1024];
					sprintf(message, "Tag <%s> doesn't have necessary command line specification!\n", str_cm_exec);
					throw CmXmlException(message);
                }

                // We don't need any more information
                break;

            } //if howExecute
        } //if Type
    } //for childNode

    if(execSpec != NULL && execSpec[0] != 0) {
	    _DBG(fprintf(log_fd, "        %s\n", execSpec));
    }

	// Return pointer to buffer, which contains exec specification.
	// Caller method is responsible for deleting memory.
	return execSpec;

} //getExecSpec


char* CmXml::getListSpec(DOM_Node parentNode) {

	// This is buffer for storing list specification.
	// We will return it as a result of this method.
	char *listSpec = new char[8192];
	listSpec[0] = '[';
	listSpec[1] = 0;

	_DBG(fprintf(log_fd, "        Searching for list items...\n"));

    for(DOM_Node childNode = parentNode.getFirstChild(); childNode != 0;
								childNode = childNode.getNextSibling()) {

		_DBG_PROCESSING_NODE("          ", childNode);

		if(childNode.getNodeType() == DOM_Node::ELEMENT_NODE) {

			DOMString childName = childNode.getNodeName();
			if(compareString(childName, str_cm_li) == 0) {
				// Found node, which contains information about list item

				// Here will be item's key and displayable name
				char *liKey = NULL;
				char *liDisplay = NULL;

				// Get node's attributes
				DOM_NamedNodeMap attributes = childNode.getAttributes();
				int attrCount = attributes.getLength();

				// Looking at all attributes, and getting 'key' and 'display' only.
				for(int attrNdx = 0; attrNdx < attrCount; attrNdx++) {

					DOM_Node attr = attributes.item(attrNdx);
					_DBG_PROCESSING_ATTR("            ", attr);

					DOMString attrName = attr.getNodeName();
					DOMString attrValue = attr.getNodeValue();

					if(compareString(attrName, str_key) == 0) {
						liKey = attrValue.transcode();
					} else if(compareString(attrName, str_display) == 0) {
						liDisplay = attrValue.transcode();
					}
				} //for attrNdx

				// If not all information was specified
				if(liKey == NULL) {
					char message[1024];
					sprintf(message,
							"Tag <%s> doesn't have the mandatory attribute '%s'!\n",
							str_cm_li, str_key);
					throw CmXmlException(message);
				}

				// If it is not first found list item -> 
				//    divide its description from others by character ';'.
				if(listSpec[1] != 0) {
					strcat(listSpec, ";");
				}

				// Add description of this list item to the specification
				strcat(listSpec, liKey);
                if(liDisplay != NULL) {
				    strcat(listSpec, ",");
				    strcat(listSpec, liDisplay);
                }

				delete [] liKey;
                liKey = NULL;
				delete [] liDisplay;
                liDisplay = NULL;

			} //if ChildName
		} //if Type
	} //for childNode

    strcat(listSpec, "]");
	_DBG(fprintf(log_fd, "        %s=\"%s\"\n", str_type, listSpec));

	// Return pointer to buffer, which contains specification of this list.
	// Caller method is responsible for deleting memory.
	return listSpec;

} //getListSpec


char* CmXml::getNodeText(DOM_Node node) {

	_DBG(fprintf(log_fd, "          Searching for text...\n"));

    // Text is stored as child node with type TEXT_NODE.
    // So no children -> no text.
    if(!node.hasChildNodes()) {
        return NULL;
    }

    // Here will be desirable text.
    char *text = NULL;

    // Searching for TEXT_NODE
    for(DOM_Node child = node.getFirstChild(); child != 0; child = child.getNextSibling()) {

		_DBG_PROCESSING_NODE("            ", child);

        if(child.getNodeType() == DOM_Node::TEXT_NODE) {

            DOMString childValue = child.getNodeValue();
            text = childValue.transcode();
           
            // We found all we need.
            break;

        } //if TEXT_NODE
    } //for child

    // Caller method is responsible for deleting this memory.
    return text;

} //getNodeText


char* CmXml::translateList(char *src) {

	_DBG(fprintf(log_fd, "  Translating list specification...\n"));

    // Make source for parser
    int len = strlen(src);
    DOMString id = DOMString::transcode("CmExecXml");
    MemBufInputSource source((const XMLByte*)src, len, id.rawBuffer(), false);

    // Here we will store translated list
    char *listSpec = NULL;

    try {
        cmXmlParser->parse(source);

    	DOM_Document doc = cmXmlParser->getDocument();
	    _DBG_PROCESSING_NODE("    ", doc);
        for(DOM_Node child = doc.getFirstChild(); child != 0; child = child.getNextSibling()) {

		    _DBG_PROCESSING_NODE("      ", child);

		    if(child.getNodeType() == DOM_Node::ELEMENT_NODE) {
			    DOMString childName = child.getNodeName();
			    if(compareString(childName, str_cm_attr) == 0) {
                    listSpec = getListSpec(child);
                    break;
			    }
		    }
        } //for child
    } //try

    catch(const XMLException& toCatch) {
		char message[4096];
        strcpy(message, "An error occured during parsing\n");
		strcat(message, "   Message: ");
		DOMString domStr(toCatch.getMessage());
		char *s = domStr.transcode();
		strcat(message, s);
		delete [] s;
		s = NULL;
		throw CmXmlException(message);
    }

    return listSpec;

} //translateList


int CmXml::compareString(const DOMString &domStr, const char* str) {

    int ret = 0;
    char *s = domStr.transcode();

    if(s == NULL || str == NULL) {
        ret = (s == str) ? 0 : 1;
    } else {
        ret = strcmp(s, str);
    }

    delete [] s;

    return ret;

} //compareString


void CmXml::checkAutoCommands(char *cm_name) {

    // Try to find the command 'autoco' for the given CM system.
	_DBG(fprintf(log_fd,
                 "\nSearching for command 'Automatically Check Out' for " \
                 "CM system '%s'...\n", cm_name));

    if(!isCommand(cm_name, "autoco")) {

		char message[1024];
		sprintf(message,
                "CM system '%s' must have 'autoco' " \
                "(Automatically Check Out) command!\n",
                cm_name);
		throw CmXmlException(message);
    }

    // Try to find the command 'autoci' for the given CM system.
	_DBG(fprintf(log_fd,
                 "\nSearching for command 'Automatically Check In' for " \
                 "CM system '%s'...\n", cm_name));

    if(!isCommand(cm_name, "autoci")) {

		char message[1024];
		sprintf(message,
                "CM system '%s' must have 'autoci' " \
                "(Automatically Check In) command!\n",
                cm_name);
		throw CmXmlException(message);
    }

} //checkAutoCommands


// END OF FILE

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

#include "CmXmlNode.h"
#include "CmXmlSystem.h"
#include "CmXmlCollection.h"


CmXmlSystem::CmXmlSystem(DOM_Node node, char *key) : CmXmlNode(node, key) {

	this->attributes = new CmXmlCollection();
	this->commands = new CmXmlCollection();

} //constructor


CmXmlSystem::~CmXmlSystem() {

	delete this->attributes;
	delete this->commands;

} //desctuctor


void CmXmlSystem::addAttr(CmXmlNode *attrNode) {

	this->attributes->add(attrNode);

} //addAttr


void CmXmlSystem::addCmd(CmXmlNode *cmdNode) {

	this->commands->add(cmdNode);

} //addCmd


CmXmlNode* CmXmlSystem::getAttr(int index) {

	CmXmlNode *attrNode = this->attributes->get(index);
	return attrNode;

} //getAttr


CmXmlNode* CmXmlSystem::getAttr(char *key) {

	CmXmlNode *attrNode = this->attributes->get(key);
	return attrNode;

} //getAttr


CmXmlNode* CmXmlSystem::getCmd(int index) {

	CmXmlNode *cmdNode = this->commands->get(index);
	return cmdNode;

} //getCmd


CmXmlNode* CmXmlSystem::getCmd(char *key) {

	CmXmlNode *cmdNode = this->commands->get(key);
	return cmdNode;

} //getCmd


int CmXmlSystem::sizeAttr() {

	return this->attributes->size();

} //sizeAttr


int CmXmlSystem::sizeCmd() {

	return this->commands->size();

} //sizeCmd


// END OF FILE

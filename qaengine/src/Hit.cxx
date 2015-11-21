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
// Hit.cpp: implementation of the Hit class.
//
//////////////////////////////////////////////////////////////////////

#include "Hit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Hit::Hit() {
	m_Name="";
	m_Line=-1;
	m_AstPath="";
	m_Module = "";
	m_Index = 0;

}

Hit::Hit(string name, string module,int line,string path) {
	m_Name=name;
	m_Line=line;
	m_AstPath=path;
	m_Module = module;
}


Hit::~Hit()
{

}

string Hit::getName() {
	return m_Name;

}

void Hit::setName(string name) {
	m_Name = name;

}

int Hit::getLine() {
	return m_Line;

}

void Hit::setLine(int line) {
	m_Line = line;
}

string Hit::getAstPath() {
	return m_AstPath;
}

void Hit::setAstPath(string path) {
	m_AstPath = path;
}

string Hit::getModule() {
	return m_Module;
}

void Hit::setModule(string module) {
	m_Module = module;
}


string Hit::getAttribute(string key) {
	PropertiesMap::iterator i = m_Attributes.find(key);
	if(i!=m_Attributes.end()) return i->second;
	return "";
}

void Hit::setAttribute(string key, string value) {
	m_Attributes[key]=value;
}

PropertiesMap* Hit::getAttributes() {
	return &m_Attributes;
}


int Hit::getIndex() {
	return m_Index;
}

void Hit::setIndex(int index) {
	m_Index = index;
}

string* Hit::getAstPathPtr() {
	return &m_AstPath;
}

string Hit::toString() {
char buf[100];
string str="";
    sprintf(buf,"%d",getIndex());
	str+=buf;
	str+="\t";
	str+="Kind";
	str+="\t";
	str+="hit";
	str+="\t";
	str+="Name";
	str+="\t";
	str+=getName();
	str+="\t";
	str+="Module";
	str+="\t";
	str+=getModule();
	str+="\t";
	str+="Line";
	str+="\t";
	sprintf(buf,"%d",getLine());
  	str+=buf;
    PropertiesMap::iterator i;
    for(i=m_Attributes.begin();i!=m_Attributes.end();i++) {
       str+="\t";
       str+=i->first;
	   str+="\t";
	   str+=i->second;
	}
	return str;
}

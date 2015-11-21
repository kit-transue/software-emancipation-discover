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
#include "EntityInfo.h"

#include "symbolArr.h"

#include <TclList.h>
#include <Interpreter.h>

#include <ddKind.h>

extern const char* ATT_etag(symbolPtr& sym);
extern const char* ATT_kind(symbolPtr& sym);
extern const char* ATT_java_kind(symbolPtr& sym);
extern const char* ATT_attribute(symbolPtr& any_sym);
extern const char* ATT_language( symbolPtr& sym);


string CEntityInfo::name(symbolPtr* pSym) {
	string result;
	string command = "printformat \"%s\" name;cname " + prepareQuery(etag(pSym));
	exec(command,result);
	return result;
}

string CEntityInfo::entityName(symbolPtr* pSym) {
    string result;
    string command = "printformat \"%s\" name;name " + prepareQuery(etag(pSym));
    exec(command,result);
    return result;
}

string CEntityInfo::kind(symbolPtr* pSym) { 
	string language = ATT_language(*pSym);
	if(language=="JAVA") return ATT_java_kind(*pSym);
	return ATT_kind(*pSym);	
}

string CEntityInfo::language(symbolPtr* pSym) { 
	string language = ATT_language(*pSym);
	
	for(int i = 0; i < language.length(); i++) { 
        	language[i] = toupper(language[i]); 
    	} 
	return language;
}

string CEntityInfo::etag(symbolPtr* pSym) {
  return ATT_etag(*pSym);
}

bool CEntityInfo::IsProtected(symbolPtr* pSym) {
  string symAttrs = ATT_attribute(*pSym);
  return symAttrs.find("PROTECTED")!=string::npos;
}

bool CEntityInfo::IsPrivate(symbolPtr* pSym) {
  string symAttrs = ATT_attribute(*pSym);
  return symAttrs.find("PRIVATE")!=string::npos;
}

bool CEntityInfo::IsPublic(symbolPtr* pSym) {
  return !(IsProtected(pSym) || IsPrivate(pSym));
}

string CEntityInfo::getChangeBodyCommand(symbolPtr* pSym) {
  string command;
  string selectTag = prepareQuery(etag(pSym));
  switch(pSym->get_kind()) {
  case DD_FUNC_DECL:  
    command = "Impact:ChangeFunctionBody " + selectTag;
    break;
  case DD_MACRO:
    command = "Impact:ChangeMacroBody " + selectTag;
    break;
  case DD_TEMPLATE:
    command = "Impact:ChangeTemplateBody " + selectTag;
    break;
  }
  return command;
}

string CEntityInfo::getChangeTypeCommand(symbolPtr* pSym,string newtype) {
  string command;
  string selectTag = prepareQuery(etag(pSym));

  switch(pSym->get_kind()) {
  case DD_FIELD: 
    command = "Impact:ChangeVariableType " + selectTag + " " +
                          "{" + newtype + "}";
    break;
  case DD_FUNC_DECL:  
    command = "Impact:ChangeFunctionType " + selectTag + " " +
                          "{" + newtype + "}";
    break;
  case DD_VAR_DECL:
    command = "Impact:ChangeVariableType " + selectTag + " " +
                          "{" + newtype + "}";
    break;
  }
  return command;
}

string CEntityInfo::prepareQuery(const string& query) {
	int nLen = query.size();
	if(nLen==0) return "";
	string result="{";
	for(int j=0;j<nLen;j++) {
		switch(query[j]) {
			case '{' : result.append("\\{");break;
			case '}' : result.append("\\}");break;
			default: result+=query[j];break;
		}
	}
	result.append("}");
	return result;
}


string CEntityInfo::getRenameCommand(symbolPtr* pSym,string newname) {
  string command;
  string selectTag = prepareQuery(etag(pSym));
  string permissionCode = "1";
  
  if(IsPublic(pSym)) permissionCode = "0";
  else if(IsProtected(pSym)) permissionCode = "2";
  else if(IsPrivate(pSym))   permissionCode = "3";
  
  switch(pSym->get_kind()) {
  case DD_MODULE:
    command="Impact:ChangeFilename " + selectTag + 
                          " {" + newname + "}";
    break;
  case DD_INTERFACE:
  case DD_CLASS:


    command = "Impact:ChangeClassname " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_ENUM:
  case DD_UNION:    command = "Impact:ChangeUnionEnumStructName " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_FIELD: 
    command = "Impact:ChangeFieldName " + selectTag +
                          " {" + newname + "} " + permissionCode;
    break;
  case DD_FUNC_DECL:  
    command = "Impact:ChangeFunctionName " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_TYPEDEF:
    command = "Impact:ChangeTypedefName " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_MACRO:
    command = "Impact:ChangeMacroName " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_TEMPLATE:
    command = "Impact:ChangeTemplateName " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_VAR_DECL:
    command = "Impact:ChangeVariableName " + selectTag +
                          " {" + newname + "}";
    break;
  case DD_PACKAGE:
    command ="Impact:RenamePackage " + selectTag +
			 " { " + newname + "}";
  }
  return command;
}

extern int cli_eval_string(const char* cmd, Interpreter *i);

bool CEntityInfo::exec(string cmd,string& result) {
  ostream*  m_pOldStream = NULL;
  strstream* m_pStream = new strstream();
  Interpreter* m_pInterpreter = NULL; 
  m_pInterpreter = GetActiveInterpreter();
  m_pOldStream = &m_pInterpreter->GetOutputStream();
  m_pInterpreter->SetOutputStream(*m_pStream);

  printf("Exec.Command: %s\n",cmd.c_str());

  genString command(cmd.c_str());
  genString genResult;
  cli_eval_string(command,m_pInterpreter);

  m_pInterpreter->PrintResult();
   
#if defined(irix6) || defined(hp10)
  m_pStream->rdbuf()->freeze(1);
  int pcount = m_pStream->rdbuf()->pcount();
#else // sun
  m_pStream->freeze(true);
  int pcount = m_pStream->pcount();
#endif // irix || hp10
 
  // bcoutch (2003/03/21)
  // Inherent problem with m_pStream->str():
  //
  // From MSDN Documentation
  //
  //    strstreambuf::str
  //    char *str();
  //    The member function calls freeze(), then returns a pointer to the beginning of the controlled sequence.
  //    (Note that no terminating null element exists, unless you insert one explicitly.)

  char* pRes = m_pStream->str();
  if(pRes!=NULL && pRes[0]!=0)
  {
    pRes[ pcount ] = '\0'; // Add a null terminator
  	result = pRes;
  }
  else {
      result = "";
  }

  // reset state of the stream
#if defined(irix6) || defined(hp10)
  m_pStream->rdbuf()->freeze(0);
#else // sun
  m_pStream->freeze(false);
#endif // irix || hp10
  
  delete m_pStream;
  m_pStream = new strstream(); 
  m_pInterpreter->SetOutputStream(*m_pStream);
  
  printf("Exec.RESULT: %s\n",result.c_str());

  m_pInterpreter = GetActiveInterpreter();
  m_pInterpreter->SetOutputStream(*m_pOldStream);
  m_pInterpreter = NULL;
  delete m_pStream;
  return result.length()>0;
}


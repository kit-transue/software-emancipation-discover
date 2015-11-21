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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// scriptfile :   abstract base class for various concrete scripting languages.
//
// Author: Gregory Bronner
// Date: 2-20-98
//
//
// The scriptfile is inteneded to abstract the notions of scripting operations in a platform and language independent way.
// It is intended to relieve its clients of the responsibility of converting slashes and backslashes in path names.
//
// All commands get parsed by the scriptfile and have their slashes replaced by the native sep string of the subclass
// So you can pass it C:/discover\bin/stupidprogram - eat c:\asdf/asdf\asdf 
// and it will turn out ok
// if you want to prevent argument expansion, use Command(NULL, x)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _ScriptFile_h
#define _ScriptFile_h
#include <iostream.h>
#include "cstring.h"



class ScriptFile
{
public:
	ScriptFile();  //constructor
	virtual ~ScriptFile(); //destructor
	
	virtual void EchoCommands(int state); //1 means start 0 means stop. Not implemented for all subclasses
	virtual void SetEnvironmentVariable(const char * name, const char * value)=0;   //sets name= value and exports it to the OS
	void Comment(const char *text);	//writes a comment in the script
	void Command(const char *subscommand, const char *nonsubscommand);  //replaces slashes in the first part, but not in the second...
	void Command(const char * command);	 //replaces slashes throughout
	virtual void Echo(const char *text)=0;	 //prints a command to the screen
	friend ostream& operator<<(ostream&, const ScriptFile &); //dumps contents to a file or cout

protected:
	virtual  cstring &ProtectSlashes(cstring &);
	cstring& ConvertSlashes(cstring &);

	cstring NL;	 //New line.
	cstring MSG1; // introductory message
	const char * mpSepStrings; //array of chars which are used as directory separators
	cstring mEchoString;	   //command used to echo commands
	cstring mCharData;		   //the actual data
	cstring  mCommentString;   // string
	const char * mpMySepString; //subclass defined separator strings.
	const char * mpMyProtectString; //protects arguments.
};

#endif
